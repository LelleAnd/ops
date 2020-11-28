/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2020 Lennart Andersson.
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

#pragma once

#include <map>

#include "opsidls/OPSConstants.h"
#include "SendDataHandler.h"
#include "Sender.h"
#include "IOService.h"
#include "TCPServerBase.h"
#include "TCPOpsProtocol.h"
#include "NetworkSupport.h"

namespace ops
{
    using namespace opsidls;

	///TODO
	/// We want to send:
	///   "Info about which publishers"
	///      @ connect (actually @ 1st timer tick when we detected it's a version 2 client)
	///      pub added/removed
	/// We receive:
    ///   "Info about which subscribers"
	///      Store info in map so that publishers can ask if any subscriber
	///      @ disconnect, clear subscribers from that connection

    class TCPSendDataHandler : public SendDataHandler, TCPServerCallbacks
    {
		IOService* _ioService;
		std::map<ObjectName_T, int> _topics;
		int _heartbeatPeriod, _heartbeatTimeout;

		struct Connection_t : TCPUserBase
		{
			// For now static buffer, later from pool
			char buffer[OPSConstants::PACKET_MAX_SIZE];
			bool sentAtConnect = false;

			///TODO @ creation get buffer from DataSegmentPool
			///TODO @ deletion return buffer to DataSegmentPool
		};

	public:
        TCPSendDataHandler(IOService* ioService, Topic& topic) :
			_ioService(ioService), _heartbeatPeriod(topic.getHeartbeatPeriod()), _heartbeatTimeout(topic.getHeartbeatTimeout())
        {
			sender = Sender::createTCPServer(this, ioService, topic.getDomainAddress(), topic.getPort(), topic.getOutSocketBufferSize());
        }

        bool sendData(char* buf, int bufSize, Topic& topic)
        {
            UNUSED(topic);
            SafeLock lock(&mutex);
            //We dont "sendTo" but rather lets the server (sender) send to all connected clients.
            const bool result = sender->sendTo(buf, bufSize, "", 0);
            return result;
        }

		// At least one publisher must be added to us for this call to work correct
		// ie. sender must be opened for this to be correct
		virtual void updateTransportInfo(Topic& top) override
		{
			// Set port to the one actually used (for tcp server where OS defines port)
			top.setPort(getLocalPort());
			if (!isValidNodeAddress(top.getDomainAddress())) {
				top.setDomainAddress(doSubnetTranslation(top.getLocalInterface(), _ioService));
			}
		}

		// Tell derived classes which topics that are active
		void topicUsage(Topic& top, bool used) override
		{
			bool needSend = false;
			// Keep a list of all used topics, with count
			std::map<ObjectName_T, int>::iterator it = _topics.find(top.getName());
			if (used) {
				if (it == _topics.end()) {
					_topics[top.getName()] = 1;
					needSend = true;
				} else {
					it->second = it->second + 1;
				}
			} else {
				if (it != _topics.end()) {
					it->second = it->second - 1;
					if (it->second > 0) return;
					_topics.erase(it);
					needSend = true;
				}
			}
			// If a new topic is added or an old one deleted, send updates
			if (needSend) {
				///TODO send topic info (if version >= 2)
			}
		}

		// Called from server when a new connection is accepted
		// A call to conn.setProtocol(...) should be done
		// Could be used to call conn.asynchWait(buffer, size)
		void onConnect(TCPConnection& conn, ConnectStatus status) override
		{
			// Create parameters that we need for each connection, will be deleted by protocol
			Connection_t* ct = new Connection_t();
			ct->sentAtConnect = false;

			TCPOpsProtocol* prot = new TCPOpsProtocol(TimeHelper::currentTimeMillis, _heartbeatPeriod, _heartbeatTimeout);
			prot->userData = ct;
			InternalString_T dbgId(status.addr);
			dbgId += "::";
			dbgId += NumberToString(status.port);
			prot->setDebugId(dbgId);

			conn.setProtocol(prot);
			conn.asynchWait(ct->buffer, sizeof(ct->buffer));

			// Notify parent
			SendDataHandler::onNewEvent(nullptr, status);
		}

		// Called from server when data has been filled into given buffer
		// A new call to conn.asynchWait(buffer, size) need to be done to continue to read
		void onEvent(TCPConnection& conn, BytesSizePair) override
		{
			Connection_t* ct = dynamic_cast<Connection_t*>(conn.getProtocol()->userData);
			if (!ct->sentAtConnect) {
				///TODO send topic info (if version >= 2)
				ct->sentAtConnect = true;
			}

			// Handle received data
			// Keep a list of subscribers for each connection
			///TODO

			// Start a new read
			conn.asynchWait(ct->buffer, sizeof(ct->buffer));
		}

		// Called from server when a connection is going to be deleted
		// Ev. buffer used in asynchRead() is no longer in use
		void onDisconnect(TCPConnection&, ConnectStatus status) override
		{
			// All subscribers from this connection disapeared, update list
			///TODO

			// Notify parent
			SendDataHandler::onNewEvent(nullptr, status);
		}

        virtual ~TCPSendDataHandler()
        {
			OPS_DES_TRACE("SDH: Destructor()\n");
            SafeLock lock(&mutex);
			delete sender;
        }
    };

}
