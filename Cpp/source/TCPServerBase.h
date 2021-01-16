/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2021 Lennart Andersson.
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

/* 
 * File:   TCPServerBase.h
 * Author: Anton Gravestam
 *
 * Created on den 22 oktober 2008, 20:01
 */

#pragma once

#include <vector>
#include <memory>
#include <atomic>

#include "OPSTypeDefs.h"

#include "DeadlineTimer.h"
#include "Notifier.h"
#include "Lockable.h"
#include "Sender.h"
#include "ConnectStatus.h"
#include "TCPConnection.h"

namespace ops
{
	struct TCPServerCallbacks
	{
		// Called from server when a new connection is accepted
		// A call to conn.setProtocol(...) should be done
		// Could be used to call conn.asynchRead(buffer, size)
		virtual void onConnect(TCPConnection& conn, ConnectStatus status) = 0;

		// Called from server when data has been filled into given buffer
		// A new call to conn.asynchRead(buffer, size) need to be done to continue to read
		virtual void onEvent(TCPConnection& conn, BytesSizePair arg) = 0;

		// Called from server when a connection has been deleted
		// Ev. buffer used in asynchRead() is no longer in use
		virtual void onDisconnect(TCPConnection& conn, ConnectStatus status) = 0;

		virtual ~TCPServerCallbacks() = default;
	};

	class TCPServerBase : public Sender, Listener<int>, public TCPConnectionCallbacks
    {
    public:
		TCPServerBase(TCPServerCallbacks* client, IOService* ioServ) :
			_client(client)
		{
			_timer = std::unique_ptr<DeadlineTimer>(DeadlineTimer::create(ioServ));
			_timer->addListener(this);
			_timer->start(period);
		}

		virtual ~TCPServerBase()
		{
			close();
		}

		void close() override
		{
			SafeLock lck(_mtx);
			OPS_TCP_TRACE("Server: Close() #connected: " << _connectedSockets.size() << '\n');
			for (int i = (int)_connectedSockets.size() - 1; i >= 0; i--) {
				deleteConnection(i);
			}
			_connectedSockets.clear();
			_connectedStatus.clear();
			OPS_TCP_TRACE("Server: Close() connected cleared\n");
		}

		///Sends buf to all Receivers connected to this Sender, ip and port are discarded and can be left blank.
		/// buf == nullptr or size == 0, is handled by connection/protocol
        bool sendTo(const char* buf, const int size, const Address_T&, const uint16_t) override
		{
			SafeLock lck(_mtx);
            sendToInternal(buf, size);
            if (_doPeriodic) {
                _doPeriodic = false;
                // We use a size 0 to force a periodic check (heartbeat handling in protocol)
                sendToInternal(nullptr, 0);
                OPS_TCP_ERROR("Server: Deferred Periodic check excuted\n");
            }
            return true;
		}

		int numConnected()
		{
			SafeLock lck(_mtx);
			return (int)_connectedSockets.size();
		}

	protected:
		// Called from derived classes
		void AddSocket(std::shared_ptr<TCPConnection> sock)
		{
			SafeLock lck(_mtx);
			_connectedSockets.push_back(sock);
			OPS_TCP_TRACE("Server: New socket connected. Total: " << _connectedSockets.size() << '\n');
			// Connected status callback with new address::port and total sockets connected
			ConnectStatus st(true, (int)_connectedSockets.size());
			sock->getRemote(st.addr, st.port);
			_connectedStatus.push_back(st);
			_client->onConnect(*sock, st);
		}

		void deleteConnection(int i)
		{
			// Make sure ev. callbacks are finished and no new ones can be called
			_connectedSockets[i]->clearCallbacks();
			// Connected status callback with deleted address::port and total sockets connected
			_connectedStatus[i].connected = false;
			_connectedStatus[i].totalNo = (int)_connectedSockets.size() - 1;
			// Close connection
			_connectedSockets[i]->close();
			_client->onDisconnect(*_connectedSockets[i], _connectedStatus[i]);
			// Delete connection
			_connectedSockets[i].reset();
		}

		// Called from TCPConnection()
		void onEvent(TCPConnection& conn, BytesSizePair arg) override
		{
			_client->onEvent(conn, arg);
		}

		// Called from TCPConnection()
		void onReceiveError(TCPConnection& conn) override
		{
			// The protocol/connection can't receive any more. For a server connection
			// we only close the connection. The TCP Client need to connect again.
			conn.close();

			// Since we are called from within the connection we can't delete it here.
			// It will be deleted at next sendTo() call.
		}
		
		// Called from periodic timer
		void onNewEvent(Notifier<int>*, int) override
		{
            _doPeriodic = true;
            SafeTryLock lck(_mtx);
            if (lck.isLocked()) {
                _doPeriodic = false;
                // We use a size 0 to force a periodic check (heartbeat handling in protocol)
                sendToInternal(nullptr, 0);
            } else {
                OPS_TCP_ERROR("Server: Periodic check deferred (due to LOCK taken)\n");
            }
			_timer->start(period);
		}

        ///Sends buf to all Receivers connected to this Sender
        /// buf == nullptr or size == 0, is handled by connection/protocol
        /// _mtx must be held when calling this method !!!
        bool sendToInternal(const char* buf, const int size)
        {
            // Send to anyone connected. Loop backwards to avoid problems when removing broken sockets
            for (int i = (int)_connectedSockets.size() - 1; i >= 0; i--) {
                // Send the data
                if (_connectedSockets[i]->sendData(buf, size) < 0) {
                    // Failed to send to this socket
                    deleteConnection(i);
                    // Remove it from our lists
                    auto it = _connectedSockets.begin();
                    it += i;
                    _connectedSockets.erase(it);
                    auto it2 = _connectedStatus.begin();
                    it2 += i;
                    _connectedStatus.erase(it2);
                    OPS_TCP_TRACE("Server: Connection deleted. Total: " << _connectedSockets.size() << '\n');
                }
            }
            return true;
        }

		// All connected sockets
		std::vector<std::shared_ptr<TCPConnection>> _connectedSockets;
		std::vector<ConnectStatus> _connectedStatus;
		Lockable _mtx;
        std::unique_ptr<DeadlineTimer> _timer;
		const int64_t period = 1000;

	private:
		TCPServerCallbacks* _client;
        std::atomic_bool _doPeriodic{ false };
	};
}
