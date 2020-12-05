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

#include "TCPProtocol.h"

namespace ops
{
	// Original TCP ptotocol (version 1)
	// ---------------------------------
	// The OPS TCP Protocol has a leading header of 22 bytes with
	//    0..17    "opsp_tcp_size_info"
	//   18..21    uint32_t in little-endian format giving size of data that follows
	//
	// TCP protocol version 2
	// ----------------------
	// The above is still valid for ordinary transport of OPS messages (used by old clients and server).
	// Version 2 adds probe messages (probe and heartbeat).
	//    0..17    "opsprobeNNNN______"
	//   18..21    uint32_t in little-endian format giving size of data that follows
	//               0  : heartbeat no additional data
	//               1  : probe additional data, 1 byte with value 0
	//

	// Function that returns current time in ms.
	typedef int64_t(*timeFunc)();

	class TCPOpsProtocol : public TCPProtocol
	{
		const uint32_t protocol_version = 2;
		const uint32_t headersize = 22;
		const uint32_t minbufsize = headersize + 1;
	protected:
		timeFunc _timeFuncMs;
		char _sizeInfoBuffer[32];
		char _probeBuffer[32];
		bool _readingHeader = true;
		int64_t _timeRcv = 0;
		int64_t _timeSnd = 0;
		int _detectedVersion = 1;
		bool _hbSent = false;
		int _heartbeatPeriod = 1000;
		int _heartbeatTimeout = 3000;

		// _data[0 .. _accumulatedSize-1] contains data (_accumulatedSize == _expectedSize)
		// returns false if error
		virtual bool handleData() override
		{
			bool errorDetected = false;
			_timeRcv = _timeFuncMs();
			if (_readingHeader) {
				// Get size of data packet from the received size packet
				uint32_t sizeInfo = *((uint32_t*)(_data + 18));
				if (sizeInfo > _maxLength) {
					// This is an error, we are not able to receive more than _maxLength bytes (the buffer size)
					errorDetected = true;
					OPS_TCP_ERROR("OpsProt: handleData(), Error: sizeInfo too large: " << sizeInfo << ", [" << _debugId << "]\n");
					OPS_DUMP_MEMORY("TCP", _data, 23);
				} else if (sizeInfo < 2) {
					// Check that it is a probe/heartbeat message using 8 first bytes
					if (memcmp(_probeBuffer, _data, 8) != 0) {
						errorDetected = true;
						OPS_TCP_ERROR("OpsProt: handleData(), Error: Not a probe/heartbeat, [" << _debugId << "]\n");
						OPS_DUMP_MEMORY("TCP", _data, 23);
					} else {
						_detectedVersion = 2;
						if (sizeInfo == 0) {
							OPS_TCP_TRACE("Heartbeat received\n");
							// Heartbeats, if used, are sent with size 0. Restart read of header
							errorDetected = !startAsyncRead(headersize);
							if (errorDetected) {
								OPS_TCP_ERROR("OpsProt: handleData(), Error: Failed to start async read, [" << _debugId << "]\n");
							}
						} else {
							// size 1 is used for the leading Probe message from a client
							if (_expectedSize == headersize) {
								// We have not gotten all data yet, Read one more byte after header
								_expectedSize = headersize + 1;
								contAsyncRead();
							} else {
								OPS_TCP_TRACE("Probe received\n");
								// Restart read of header
								errorDetected = !startAsyncRead(headersize);
								if (errorDetected) {
									OPS_TCP_ERROR("OpsProt: handleData(), Error: Failed to start async read, [" << _debugId << "]\n");
								}
							}
						}
					}
				} else {
					_readingHeader = false;
					errorDetected = !startAsyncRead(sizeInfo);
					if (errorDetected) {
						OPS_TCP_ERROR("OpsProt: handleData(), Error: Failed to start async read, [" << _debugId << "]\n");
					}
				}
			} else {
				// Got a complete data packet
				_client->onEvent(*this, BytesSizePair(_data, _expectedSize));
			}
			return !errorDetected;
		}

	public:
		TCPOpsProtocol(timeFunc func, int heartbeatPeriod, int heartbeatTimeout) :
			_timeFuncMs(func),
			_sizeInfoBuffer("opsp_tcp_size_info"),
			_probeBuffer   ("opsprobeNNNN______"),
			_heartbeatPeriod(heartbeatPeriod),
			_heartbeatTimeout(heartbeatTimeout)
		{
			// In _probeBuffer the NNNN is the current TCP protocol version
			*((uint32_t*)(_probeBuffer + 8)) = protocol_version;
		}

		int detectedVersion() const noexcept
		{
			return _detectedVersion;
		}

		virtual void resetProtocol() noexcept override
		{
			_detectedVersion = 1;
			_hbSent = false;
		}

		// Returning false if unable to start
		virtual bool startReceive(char* bytes, uint32_t size) override
		{
			// Always start by reading the packet size when using tcp
			// We use the users provided buffer for receiving the size info
			if (size < minbufsize) return false;
			_data = bytes;
			_maxLength = size;
			_readingHeader = true;
			return startAsyncRead(headersize);
		}

		// Returns number of bytes written or < 0 if an error
		// size == 0, trigs a periodic check
		virtual int sendData(const char* bytes, const uint32_t size) override
		{
			if (size == 0) {
				if (!periodicCheck()) {
					return -1;	// Error detected
				} else {
					return 0;	// No error
				}
			} else {
				_timeSnd = _timeFuncMs();

				// Fill in header
				*((uint32_t*)(_sizeInfoBuffer + 18)) = size;

				// Send header
				int res = _client->sendBuffer(*this, _sizeInfoBuffer, headersize);
				if (res != (int)headersize) {
					OPS_TCP_ERROR("OpsProt: sendData(), Error: Failed to write header (" << headersize << "), res: " << res << ", [" << _debugId << "]\n");
					res = -1;
				}
				if (res < 0) return res;

				// Send data
				res = _client->sendBuffer(*this, bytes, size);
				if (res != (int)size) {
					OPS_TCP_ERROR("OpsProt: sendData(), Error: Failed to write data (" << size << "), res: " << res << ", [" << _debugId << "]\n");
					res = -1;
				}
				return res;
			}
		}

		// Should only be called by TCP clients
		// Returns number of bytes written or < 0 if an error
		int sendProbe()
		{
			OPS_TCP_TRACE("Send Probe\n");
			_timeSnd = _timeFuncMs();

			// Fill in protocol probe header
			*((uint32_t*)(_probeBuffer + 18)) = 1;
			

			// One byte of real data is needed to not get a disconnect if the message is received in an old client.
			// This will be acceptable by old servers/clients. Clients log an error code and continue to read.
			// Old servers wont notice since they don't read. This should not be a problem, a small amount of
			// bytes will be transfered and stored in the stack.
			// New TCP Servers notice and enable the sending of heartbeat/pubinfo and other new functionality.
			_probeBuffer[headersize] = 0;

			// Send header + 1 byte
			return _client->sendBuffer(*this, _probeBuffer, headersize + 1);
		}

		// Returns number of bytes written or < 0 if an error
		int sendHeartbeat()
		{
			if (_detectedVersion > 1) {
				OPS_TCP_TRACE("Send Heartbeat\n");
				_timeSnd = _timeFuncMs();
				_hbSent = true;

				// Fill in protocol probe header
				*((uint32_t*)(_probeBuffer + 18)) = 0;

				// Send only a header
				return _client->sendBuffer(*this, _probeBuffer, headersize);
			}
			return 0;
		}

		// Returns false if an error is detected
		virtual bool periodicCheck() override
		{
			bool errorDetected = false;

			if ((_detectedVersion > 1) && (_heartbeatPeriod > 0)) {
				// Check if we need to send a heartbeat (at least one during a connection)
				if ((!_hbSent) || ((_timeFuncMs() - _timeSnd) >= _heartbeatPeriod)) {
					if (sendHeartbeat() < 0) errorDetected = true;
				}

				// Check that we got data from other side
				if ((_timeFuncMs() - _timeRcv) >= _heartbeatTimeout) {
					OPS_TCP_TRACE("Receive timeout\n");
					// A long time since any data
					errorDetected = true;
				}
			}
			return !errorDetected;
		}

	};
}
