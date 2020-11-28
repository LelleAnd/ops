/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2019 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "BytesSizePair.h"

namespace ops
{
	class TCPProtocol;

	struct TCPProtocolCallbacks
	{
		virtual bool isConnected(TCPProtocol& prot) = 0;
		virtual void startAsyncRead(TCPProtocol& prot, char* bytes, uint32_t size) = 0;
		virtual void onEvent(TCPProtocol& prot, BytesSizePair arg) = 0;
		virtual int sendBuffer(TCPProtocol& prot, const char* bytes, const uint32_t size) = 0;
	};

	struct TCPUserBase
	{
		virtual ~TCPUserBase() {}
	};

	class TCPProtocol
	{
	protected:
        TCPProtocolCallbacks* _client{ nullptr };
        char* _data{ nullptr };
        uint32_t _maxLength{ 0 };
        uint32_t _accumulatedSize{ 0 };
        uint32_t _expectedSize{ 0 };
		InternalString_T _debugId;

		// Returns false on error
		bool startAsyncRead(uint32_t size)
		{
            if (_client == nullptr) { return false; }
			_accumulatedSize = 0;
			_expectedSize = size;
			_client->startAsyncRead(*this, _data, _expectedSize);
			return true;
		}

		void contAsyncRead()
		{
			_client->startAsyncRead(*this, _data + _accumulatedSize, _expectedSize - _accumulatedSize);
		}

		// Called when _expectedSize of bytes has been received
		// Returns false if an error is detected
		virtual bool handleData() = 0;

	public:
		TCPUserBase* userData = nullptr;

		TCPProtocol() {}

		virtual ~TCPProtocol()
		{
            if (userData != nullptr) { delete userData; }
		}

		// Connect a client
		void connect(TCPProtocolCallbacks* client) noexcept { _client = client; }

		void setDebugId(const InternalString_T& dbgId) noexcept { _debugId = dbgId; }
		const InternalString_T& getDebugId() noexcept { return _debugId; }

		// Called to reset protocol internal state
		// Normaly used at connect
		virtual void resetProtocol() noexcept {}

		// Returning false if unable to start
		virtual bool startReceive(char* bytes, uint32_t size) = 0;

		// Returns number of bytes written or < 0 if an error
		virtual int sendData(const char* buf, const uint32_t size) = 0;

		// Returning false if an error is detected (should disconnect)
		virtual bool handleReceivedData(int error, uint32_t nrBytesReceived)
		{
			bool errorDetected = true;
			if (_client) {
				if (!_client->isConnected(*this)) {
					_client->onEvent(*this, BytesSizePair(nullptr, -2));

				} else {
					if (!error && nrBytesReceived > 0) {
						_accumulatedSize += nrBytesReceived;
						if (_accumulatedSize < _expectedSize) {
							// We have not gotten all data yet, Read more
							contAsyncRead();
							errorDetected = false;

						} else {
							// We have got requested amount (_expectedSize) of bytes
							errorDetected = !handleData();
						}
					}
				}
			}
			return !errorDetected;
		}

		// Returns false if an error is detected (should diconnect)
		virtual bool periodicCheck() = 0;
	};
}
