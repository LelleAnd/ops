/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020-2025 Lennart Andersson.
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

#include <map>

#include "Sender.h"

#ifndef REPLACE_TRANSPORT_LAYER

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "UDPSender.h"
#include "TCPServer.h"

namespace ops
{

    std::unique_ptr<Sender> Sender::create(IOService* ioService, Address_T localInterface, int ttl, int64_t outSocketBufferSize)
    {
        return std::make_unique<UDPSender>(ioService, localInterface, ttl, (int)outSocketBufferSize, true);
    }

    std::unique_ptr<Sender> Sender::createUDPSender(IOService* ioService, Address_T localInterface, int ttl, int64_t outSocketBufferSize)
    {
        return std::make_unique<UDPSender>(ioService, localInterface, ttl, (int)outSocketBufferSize, false);
    }

    std::unique_ptr<Sender> Sender::createTCPServer(TCPServerCallbacks* client, IOService* ioService, Address_T ip, int port, int64_t outSocketBufferSize)
    {
        return std::make_unique<TCPServer>(client, ioService, ip, (uint16_t)port, (int)outSocketBufferSize);
    }

}
#endif // REPLACE_TRANSPORT_LAYER
