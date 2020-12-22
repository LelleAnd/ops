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

#include "Receiver.h"
#include "BytesSizePair.h"
#include "DeadlineTimer.h"
#include "Participant.h"
#include "BasicError.h"
#include "TCPProtocol.h"
#include "ConnectStatus.h"
#include "TCPConnection.h"

namespace ops
{
	struct TCPClientCallbacks
	{
		// Called from client when a connection is made
		virtual void onConnect(TCPConnection& conn, ConnectStatus status) = 0;

		// Called from client when a connection is closed
		// Ev. buffer used in asynchRead() is no longer in use
		virtual void onDisconnect(TCPConnection& conn, ConnectStatus status) = 0;

		virtual ~TCPClientCallbacks() = default;
	};

    class TCPClientBase : public Receiver, Listener<int>, protected TCPConnectionCallbacks
    {
    public:
        TCPClientBase(TCPClientCallbacks* client, IOService* ioServ, std::shared_ptr<TCPConnection> connection) :
			_connection(connection), _client(client), _cs(false, 0), _started(false)
        {
			_timer = DeadlineTimer::create(ioServ);
			_timer->addListener(this);
			_timer->start(period);
		}

		virtual ~TCPClientBase()
		{
			delete _timer;
			// Make sure ev. callbacks are finished and no new ones can be called
			_connection->clearCallbacks();
			// Delete connection
			_connection.reset();
		}

        void asynchWait(char* bytes, int size) override
        {
			_connection->asynchWait(bytes, size);
        }

        size_t bytesAvailable() override
        {
            return _connection->bytesAvailable();
        }

        int sendTo(const char* buf, int size)
		{
			return _connection->sendData(buf, size);
		}

		virtual bool isConnected() = 0;

	protected:
		std::shared_ptr<TCPConnection> _connection;

		// Should be called by the derived classes
		void connected(bool value)
		{
			const bool doNotify = _cs.connected != value;
			_cs.connected = value;
			_cs.totalNo = value ? 1 : 0;
			if (value) getSource(_cs.addr, _cs.port);
			if (doNotify) {
				if (_cs.connected) {
					_client->onConnect(*_connection, _cs);
				} else {
					_client->onDisconnect(*_connection, _cs);
				}
			}
		}

		// Called from TCPConnection()
		void onEvent(TCPConnection&, BytesSizePair arg) override
		{
			///TODO this could be simplified to a simple callback (when all Receiver's updated)
			///Notifier has a list and takes a lock which isn't necessary, we can only have 1 listener
			///and a lock is already taken in TCPConnection() calling us.
			Notifier<BytesSizePair>::notifyNewEvent(arg);
		}

		// Called from TCPConnection()
		void onReceiveError(TCPConnection& conn) override
		{
			//Report error
			ops::BasicError err("TCPClient", "handle_received_data", "Error in receive.");
			Participant::reportStaticError(&err);

			//Close the socket
			conn.stop();

			//Notify our user
			Notifier<BytesSizePair>::notifyNewEvent(BytesSizePair(nullptr, -1));

			if (_started) {
				//Try to connect again
				conn.start();
			}
		}

		// Called from periodic timer
		void onNewEvent(Notifier<int>*, int) override
		{
			if (_connection->isConnected()) {
				if (!_connection->getProtocol()->periodicCheck()) {
					OPS_TCP_TRACE("Client: Error from periodicCheck()\n");
					// Disconnect. Since there is a read ongoing, it will restart when finished.
					_connection->stop();
				}
			}
			_timer->start(period);
		}

		bool start() override
		{
			_started = true;
            return true;
		}

		void stop() override
		{
			_started = false;
		}

	private:
		TCPClientCallbacks* _client;
		ConnectStatus _cs;
		volatile bool _started;
		DeadlineTimer* _timer;
		const int64_t period = 1000;
	};
}
