/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
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
#include "OPSTypeDefs.h"
#include "Receiver.h"

#ifndef REPLACE_TRANSPORT_LAYER

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "MulticastReceiver.h"
#include "TCPClient.h"
#include "UDPReceiver.h"

namespace ops
{
	std::unique_ptr<Receiver> Receiver::createMCReceiver(Address_T ip, int bindPort, IOService* ioService, Address_T localInterface, int64_t inSocketBufferSize)
	{
		return std::make_unique<MulticastReceiver>(ip, (uint16_t)bindPort, ioService, localInterface, (int)inSocketBufferSize);
	}
	std::unique_ptr<Receiver> Receiver::createTCPClient(TCPClientCallbacks* client, Address_T ip, int port, IOService* ioService, int64_t inSocketBufferSize)
	{
		return std::make_unique<TCPClient>(client, ip, (uint16_t)port, ioService, (int)inSocketBufferSize);
	}
	std::unique_ptr<Receiver> Receiver::createUDPReceiver(int port, IOService* ioService, Address_T localInterface, int64_t inSocketBufferSize)
	{
		return std::make_unique<UDPReceiver>((uint16_t)port, ioService, localInterface, (int)inSocketBufferSize);
	}

}
#endif // REPLACE_TRANSPORT_LAYER
