/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2024 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "TCPReceiveDataChannel.h"
#include "OPSArchiverIn.h"
#include "BasicError.h" 
#include "Domain.h"
#include "Participant.h"
#include "ReceiverFactory.h"
#include "CommException.h"
#include "TimeHelper.h"

namespace ops
{
	///TODO
	/// We want to send:
	///   "Info about which subscribers"
	///      @ connect
	///      sub added/removed
	/// We receive:
	///   "OPS Messages"
	///      Normal handling
	///   "Info about which publishers"
	///      Store info in map so that subscribers can ask if any publisher
	///      @ disconnect, clear publishers

    TCPReceiveDataChannel::TCPReceiveDataChannel(const Topic& top, Participant& part) :
		ReceiveDataChannel(top, part, 
			Receiver::createTCPClient(
				this, top.getDomainAddress(), top.getPort(), 
				part.getIOService(), top.getInSocketBufferSize())),
		_heartbeatPeriod(top.getHeartbeatPeriod()),
		_heartbeatTimeout(top.getHeartbeatTimeout())
    {
	}

    TCPReceiveDataChannel::~TCPReceiveDataChannel()
    {
    }

	// Tell derived classes which topics that are active
	void TCPReceiveDataChannel::topicUsage(const Topic& , bool )
	{
		///TODO Keep a list of all used topics, with count
		/// If a new topic is added or an old one deleted, send updates if connected
	}

    ///Override from Listener
    ///Called whenever the receiver has new data.
    void TCPReceiveDataChannel::onNewEvent(Notifier<BytesSizePair>* const sender, BytesSizePair const byteSizePair)
    {
		/// Here we got some data in our buffer
		///TODO Check if it is an internal TCP Transport info and if so handle it and start a new asynch read

		/// Otherwise let parent handle it (normal OPS trafic)
		ReceiveDataChannel::onNewEvent(sender, byteSizePair);
	}

	// Called from client when a connection is made
	void TCPReceiveDataChannel::onConnect(TCPConnection& conn, ConnectStatus status)
	{
		OPS_TCP_TRACE("RDC: onConnect()\n");
		// If first time, set the protocol to use
		if (conn.getProtocol() == nullptr) {
			conn.setProtocol(new TCPOpsProtocol(ops_clock::now, _heartbeatPeriod, _heartbeatTimeout));
		}
		TCPOpsProtocol* const prot = dynamic_cast<TCPOpsProtocol*>(conn.getProtocol());
		if (prot != nullptr) {
			InternalString_T dbgId(status.addr);
			dbgId += "::";
			dbgId += NumberToString(status.port);
			prot->setDebugId(dbgId);

			// Trig sending of "ops protocol probe" to see if server know new features.
			prot->sendProbe();
		}

        if (_hasBeenConnected && !_isConnected) {
            // Note that this will only happen for TCP Channels with configured fixed ports
            // Dynamic ports will have a new RDC for each connection
            BasicError err("ReceiveDataChannel", "onConnect", "Connection was lost but is now reconnected.");
            participant.reportError(&err);
        }
        _hasBeenConnected = true;
        _isConnected = true;

        ReceiveDataChannel::onNewEvent(nullptr, status);

        // We also need to start the asynch receiving
        conn.asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
    }

	// Called from client when a connection is closed
	// Ev. buffer used in asynchRead() is no longer in use
	void TCPReceiveDataChannel::onDisconnect(TCPConnection& conn, ConnectStatus const status)
	{
		OPS_TCP_TRACE("RDC: onDisconnect()\n");
        _isConnected = false;
        // Need to reset protocol state in case the next connection is to a server with another version
		TCPOpsProtocol* const prot = dynamic_cast<TCPOpsProtocol*>(conn.getProtocol());
		if (prot != nullptr) {
			prot->resetProtocol();
		}
		ReceiveDataChannel::onNewEvent(nullptr, status);
	}
}
