/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2021 Lennart Andersson.
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

#ifndef ops_UDPSenderH
#define	ops_UDPSenderH

#include "Sender.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "Notifier.h"

namespace ops
{
    ///A sender implementation that dispatches messages over ip based UDP.
    class UDPSender : public Sender
    {
    public:
        ///Costructs a new UDPSender an binds its underlying socket to local host
        ///and a dynamically allocated local port.
		///This class accepts synchronous write operations through sendTo().

        UDPSender(IOService* ioServ, Address_T localInterface = "0.0.0.0", int ttl = 1, int outSocketBufferSize = 16000000, bool multicastSocket = false);
        ~UDPSender();
        
		virtual bool open() override;
		virtual void close() override;

        virtual bool sendTo(const char* buf, const int size, const Address_T& ip, const uint16_t port) override;
        virtual uint16_t getLocalPort() override {return socket->local_endpoint().port();};
        virtual Address_T getLocalAddress() override {return socket->local_endpoint().address().to_string().c_str();};
        virtual uint32_t getLocalAddressHost() override { return (uint32_t)socket->local_endpoint().address().to_v4().to_ulong(); };

    private:
        ///This UDPSender wraps boost socket functionality.
        ///These are the required boost members to perform operations required.
		boost::asio::ip::address ipAddr;
		boost::asio::ip::udp::endpoint localEndpoint;   //<-- The local port to bind to.
        std::unique_ptr<boost::asio::ip::udp::socket> socket;//<-- The socket that sends data.
        boost::asio::io_service* io_service;            //<-- Required for boost sockets.

		Address_T _localInterface;
		int _ttl;
		int _outSocketBufferSize;
		bool _multicastSocket;
    };
}
#endif	
