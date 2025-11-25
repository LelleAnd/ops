/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2025 Lennart Andersson.
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

#ifndef ops_ReceiverH
#define ops_ReceiverH

#include "OPSTypeDefs.h"
#include "Notifier.h"
#include "IOService.h"
#include "BytesSizePair.h"
#include "Telemetry.h"

namespace ops
{
	struct TCPClientCallbacks;

	class Receiver : public Notifier<BytesSizePair>
	{
	public:
		virtual ~Receiver() = default;

		static std::unique_ptr<Receiver> createMCReceiver(Address_T ip, int bindPort, IOService* ioService, Address_T localInterface = "0.0.0.0", int64_t inSocketBufferSize = 16000000);
		static std::unique_ptr<Receiver> createTCPClient(TCPClientCallbacks* client, Address_T ip, int port, IOService* ioService, int64_t inSocketBufferSize = 16000000);
		static std::unique_ptr<Receiver> createUDPReceiver(int port, IOService* ioService, Address_T localInterface = "0.0.0.0", int64_t inSocketBufferSize = 16000000);
		
		// Set the receive buffer to use
		virtual void asynchWait(char* bytes, int size) = 0;
		virtual bool start() = 0;           // Starts asynchrounous work
		virtual void stop() = 0;            // Request the asynchronous work to stop
		virtual bool asyncFinished() = 0;   // Returns true if all asynchronous work has finished
        virtual size_t bytesAvailable() = 0;

		virtual uint16_t getLocalPort() = 0;
		virtual Address_T getLocalAddress() = 0;

		// Get some collected telemetry from the reciever (if implemented)
		virtual Telemetry getTelemetry()
		{
			return Telemetry();
		}

		// Used to get the sender IP and port for a received message
		// Only safe to call in callback, before a new asynchWait() is called.
		virtual void getSource(Address_T& address, uint16_t& port) = 0;
        virtual void getSource(uint32_t& address, uint16_t& port) = 0;
	};
}
#endif
