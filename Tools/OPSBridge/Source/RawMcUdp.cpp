/**
*
* Copyright (C) 2018-2025 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "NetworkSupport.h"

#include "BridgeLog.h"
#include "RawMcUdp.h"

namespace opsbridge {

	RawMcUdp::RawMcUdp(RawMcUdpListener* const client) : _client(client)
	{
		_ioService = ops::IOService::create();

		// Create thread running the ioService
		_threadPool = new ops::SingleThreadPool();
		_threadPool->addRunnable(this);
	}

	RawMcUdp::~RawMcUdp()
	{
		_keepRunning = false;

		// Stop and delete all Senders
		for (auto& x : _senders) {
			x.second.sender.reset();
		}

		// Stop and delete all Receivers
		for (auto& x : _receivers) {
			x.second.receiver.reset();
		}

		// Then we request the IO Service to stop the processing (it's running on the threadpool).
		// The stop() call will not block, it just signals that we want it to finish as soon as possible.
		if (_ioService != nullptr) { _ioService->stop(); }

		// Now we delete the threadpool, which will wait for the thread(s) to finish
		if (_threadPool != nullptr) { delete _threadPool; }
		_threadPool = nullptr;

		// All objects connected to this ioservice should now be deleted, so it should be safe to delete it
		_ioService.reset();
	}

	// This will be called by our threadpool
	void RawMcUdp::run()
	{
		_ioService->run();
	}

	bool RawMcUdp::AddReceiver(ops::Address_T ip, uint16_t const port, ops::Address_T const ifc)
	{
		ops::InternalString_T key(ip);
		key += "::";
		key += ops::NumberToString(port);

		///TODO Check if ifc need to be in key
		/// Seem to be necessary for MC atleast

		// If receiver already set up, return false
		if (_receivers.find(key) != _receivers.end()) { return false; }

		// Create UdpReceiver or MulticastReceiver
		uint32_t const addr = ops::IPString2Addr(ip);
		if (addr >= 0xE0000000) {
			// Multicast and above
			_receivers[key].receiver = ops::Receiver::createMCReceiver(ip, port, _ioService.get(), ifc);
		} else {
			// Use UDP
			_receivers[key].receiver = ops::Receiver::createUDPReceiver(port, _ioService.get(), ip);
		}
		_receivers[key].receiver->connect(this);

		BL_TRACE("RawMcUdp::AddReceiver(): Created receiver for %s\n", key.c_str());

		// Start async receive
		_receivers[key].receiver->start();
		_receivers[key].receiver->asynchWait(_receivers[key].buffer, 65536);

		if (!_started) {
			_started = true;
			_threadPool->start();
		}

		return true;
	}

	void RawMcUdp::onNewEvent(ops::SingleNotifier<ops::BytesSizePair>* const sender, ops::BytesSizePair byteSizePair)
	{
		ops::Receiver* const receiver = dynamic_cast<ops::Receiver*>(sender);

		ops::Address_T localAddr = receiver->getLocalAddress();
		uint16_t localPort = receiver->getLocalPort();
		ops::Address_T srcAddr;
		uint16_t srcPort;
		receiver->getSource(srcAddr, srcPort);

		// if srcPort is one of our own sender sockets, then skip data to avoid unwanted feedback
		if (_senderPorts.find(srcPort) != _senderPorts.end()) {
			BL_TRACE("RawMcUdp::onNewEvent() [%s::%d] Skipping data from %s::%d, size %d\n",
				localAddr.c_str(), localPort,
				srcAddr.c_str(), srcPort, byteSizePair.size);
		
		} else {
			BL_TRACE("RawMcUdp::onNewEvent() [%s::%d] Received data from %s::%d, size %d\n",
				localAddr.c_str(), localPort,
				srcAddr.c_str(), srcPort, byteSizePair.size);

			TUdpMcMessage mess;
			mess.Head.Type = mtUdpMc;
			mess.Head.Length = sizeof(mess);
			mess.DstMcIP = ops::IPString2Addr(localAddr);
			mess.DstMcPort = localPort;
			mess.SrcIP = ops::IPString2Addr(srcAddr);
			mess.SrcPort = srcPort;
			mess.DataLength = byteSizePair.size;

			// notify client
			if (_client != nullptr) { _client->onUdpMcMessage(this, mess, byteSizePair.bytes); }
		}

		if (_keepRunning) {
			// Start a new read
			receiver->asynchWait(byteSizePair.bytes, 65536);
		}
	}

	void RawMcUdp::Write(const TUdpMcMessage& mess, const char* const data)
	{
		// For each srcIP,srcPort we create a unique sender (these are the original sender's IP and port).
		// This enables the receiver of our sent MC and UDP packages, to differentiate
		// messages using the sockets source IP and Port
		ops::InternalString_T key(ops::NumberToString(mess.SrcIP));
		key += "::";
		key += ops::NumberToString(mess.SrcPort);

		// If sender is missing, set up one
		if (_senders.find(key) == _senders.end()) {
			// Lookup ev. translation
			// DstMcIp,DstMcPort --> translation key
			ops::InternalString_T translationKey(ops::NumberToString(mess.DstMcIP));
			translationKey += "::";
			translationKey += ops::NumberToString(mess.DstMcPort);

			translation_t t;								// Default Ifc and ttl is defined in type
			t.dstIp = ops::IPAddr2String(mess.DstMcIP);		// Default address is the same as for receive
			t.dstPort = mess.DstMcPort;						// Default port is the same as for receive

			// See if we need to translate some parameters
			if (_translations.find(translationKey) != _translations.end()) {
				translation_t const t2 = _translations[translationKey];
				if (t2.dstIp != "") { t.dstIp = t2.dstIp; }
				if (t2.dstPort != 0) { t.dstPort = t2.dstPort; }
				t.Ifc = t2.Ifc;
				t.ttl = t2.ttl;
			}
			_senders[key].t = t;

			BL_TRACE("RawMcUdp::write() mess.DstMcIP=%X\n", mess.DstMcIP);
			BL_TRACE("RawMcUdp::write() t.Ifc=%s\n", t.Ifc.c_str());
			BL_TRACE("RawMcUdp::write() t.ttl=%d\n", t.ttl);

			// Create MC or UDP sender
			if (mess.DstMcIP >= 0xE0000000) {
				_senders[key].sender = ops::Sender::create(_ioService.get(), t.Ifc, t.ttl);
			} else {
				_senders[key].sender = ops::Sender::createUDPSender(_ioService.get());
			}

			if (_senders[key].sender != nullptr) {
				// Store local ports for filtering on receive side above
				int localPort = _senders[key].sender->getLocalPort();
				if (_senderPorts.find(localPort) == _senderPorts.end()) {
					_senderPorts.insert(localPort);
				}
				BL_TRACE("RawMcUdp::write() Created sender to %s::%d using local port: %d (If:%s)\n", 
					_senders[key].t.dstIp.c_str(), _senders[key].t.dstPort,
					localPort, t.Ifc.c_str());
			}
		}
		if (_senders[key].sender != nullptr) {
			BL_TRACE("RawMcUdp::write() to %s::%d\n", _senders[key].t.dstIp.c_str(), _senders[key].t.dstPort);
			_senders[key].sender->sendTo(data, mess.DataLength, _senders[key].t.dstIp.c_str(), (uint16_t)_senders[key].t.dstPort);
		}
	}

	void RawMcUdp::AddTranslation(ops::Address_T ip, int const port, ops::Address_T const newIp, int const newPort, ops::Address_T const ifc, int const ttl)
	{
		ops::InternalString_T translationKey(ops::NumberToString(ops::IPString2Addr(ip)));
		translationKey += "::";
		translationKey += ops::NumberToString(port);

		BL_TRACE("Translationkey: %s\n", translationKey.c_str());

		translation_t t;
		t.dstIp = newIp;
		t.dstPort = newPort;
		t.Ifc = ifc;
		t.ttl = ttl;
		_translations[translationKey] = t;
	}
}
