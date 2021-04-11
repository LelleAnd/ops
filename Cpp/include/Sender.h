/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020-2021 Lennart Andersson.
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
 * File:   Sender.h
 * Author: Anton Gravestam
 *
 * Created on den 22 oktober 2008, 20:01
 */

#ifndef ops_SenderH
#define	ops_SenderH

#include "OPSTypeDefs.h"
#include "ByteBuffer.h"
#include "IOService.h" 

namespace ops
{
	struct TCPServerCallbacks;

	///Interface used to send data
    class Sender
    {
    public:
        virtual bool sendTo(const char* buf, const int size, const Address_T& ip, const uint16_t port) = 0;
		virtual bool open() = 0;
		virtual void close() = 0;

		virtual uint16_t getLocalPort() = 0;
		virtual Address_T getLocalAddress() = 0;
        virtual uint32_t getLocalAddressHost() = 0;

        static std::unique_ptr<Sender> create(IOService* ioService, Address_T localInterface = "0.0.0.0", int ttl = 1, int64_t outSocketBufferSize = 16000000);
        static std::unique_ptr<Sender> createUDPSender(IOService* ioService, Address_T localInterface = "0.0.0.0", int ttl = 1, int64_t outSocketBufferSize = 16000000);
        static std::unique_ptr<Sender> createTCPServer(TCPServerCallbacks* client, IOService* ioService, Address_T ip, int port, int64_t outSocketBufferSize = 16000000);

        virtual ~Sender() = default;
    };

}

#endif
