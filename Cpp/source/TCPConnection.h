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

#include <memory>

#include "Lockable.h"
#include "OPSTypeDefs.h"
#include "TCPProtocol.h"

namespace ops
{
	class TCPConnection;

	struct TCPConnectionCallbacks
	{
		virtual void onEvent(TCPConnection& prot, BytesSizePair arg) = 0;
		virtual void onReceiveError(TCPConnection& prot) = 0;
		virtual ~TCPConnectionCallbacks() = default;
	};

	// Helper class for a connected TCP socket
	class TCPConnection : TCPProtocolCallbacks, public std::enable_shared_from_this<TCPConnection>
	{
	public:
		TCPConnection(TCPConnectionCallbacks* client) :
			_client(client)
		{}
		
		virtual ~TCPConnection()
		{
			if (_protocol) delete _protocol;
		}

		void clearCallbacks()
		{
			// By holding the mutex while clearing _client, we ensure that we can't 
			// be in a callback while clearing it. This also means that when this method returns
			// the connection can't call the _client anymore and it's OK for the client to vanish.
			SafeLock lck(_clientMtx);
			_client = nullptr;
		}

		void asynchWait(char* bytes, int size)
		{
			if (_connected) {
				_protocol->startReceive(bytes, size);
			}
		}

        virtual size_t bytesAvailable() = 0;

        int sendData(const char* buf, const int size)
		{
			OPS_TCP_TRACE("Conn: sendData(), #bytes: " << size << ", [" << _protocol->getDebugId() << "]\n");
			return _protocol->sendData(buf, size);
		}

		bool isConnected() const noexcept
		{
			return _connected;
		}
		
		virtual void getLocal(Address_T& address, uint16_t& port) = 0;
        virtual void getLocal(uint32_t& address, uint16_t& port) = 0;
        virtual void getRemote(Address_T& address, uint16_t& port) = 0;
        virtual void getRemote(uint32_t& address, uint16_t& port) = 0;

		virtual void start() {};
		virtual void stop() {};
		virtual void close() = 0;

		void setProtocol(TCPProtocol* prot)
		{
			_protocol = prot;
			_protocol->connect(this);
		}

		TCPProtocol* getProtocol() noexcept
		{
			return _protocol;
		}

	protected:
        volatile bool _connected{ false };

		virtual int send(const char* buf, const uint32_t size) = 0;
		virtual void startAsyncRead(char* bytes, uint32_t size) = 0;

		// Should be called by the derived classes
		virtual void handleReceivedData(int error, uint32_t nrBytesReceived)
		{
			if (error) {
				OPS_TCP_ERROR("Conn: handleReceivedData(), error: " << error << ", #bytes: " << nrBytesReceived << ", [" << _protocol->getDebugId() << "]\n");
			} else {
				OPS_TCP_TRACE("Conn: handleReceivedData(), error: " << error << ", #bytes: " << nrBytesReceived << ", [" << _protocol->getDebugId() << "]\n");
			}
			if (!_protocol->handleReceivedData(error, nrBytesReceived)) {
				// If we come here the protocol can't receive any longer and we need to disconnect
				// By holding the mutex while in the callback, we are synchronized with clearCallbacks()
				SafeLock lck(_clientMtx);
				if (_client) _client->onReceiveError(*this);
			}
		}

	private:
		friend class TCPServerBase;
		friend class TCPClientBase;

		Lockable _clientMtx;
        TCPConnectionCallbacks* _client{ nullptr };
        TCPProtocol* _protocol{ nullptr };

		// Needed by protocol
		bool isConnected(TCPProtocol&) noexcept override
		{
			return _connected;
		}

		// Needed by protocol
		void startAsyncRead(TCPProtocol&, char* bytes, uint32_t size) override
		{
			startAsyncRead(bytes, size);
		}

		// Needed by protocol
		void onEvent(TCPProtocol&, BytesSizePair arg) override
		{
			// By holding the mutex while in the callback, we are synchronized with clearCallbacks()
			SafeLock lck(_clientMtx);
			if (_client) _client->onEvent(*this, arg);
		}

		// Needed by protocol
		virtual int sendBuffer(TCPProtocol&, const char* bytes, const uint32_t size) override
		{
			return send(bytes, size);
		}
	};
}
