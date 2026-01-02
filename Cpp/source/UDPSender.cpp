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

#include <iostream>

#include "OPSTypeDefs.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "boost/asio/basic_datagram_socket.hpp"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "UDPSender.h"
#include "Participant.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "BoostIOServiceImpl.h"

namespace ops
{
    using boost::asio::ip::udp;
	UDPSender::UDPSender(IOService* const ioServ, const Address_T localInterface, const int ttl, const int outSocketBufferSize, const bool multicastSocket):
		ipAddr(boost::asio::ip::address_v4::from_string(localInterface.c_str())),
		localEndpoint(ipAddr, 0),
		io_service(BoostIOServiceImpl::get(ioServ)),
		_localInterface(localInterface), _ttl(ttl), _outSocketBufferSize(outSocketBufferSize), _multicastSocket(multicastSocket)
    {
        UDPSender::open();
    }

    UDPSender::~UDPSender()
    {
        UDPSender::close();
    }

	bool UDPSender::open()
	{
        if (socket != nullptr) { return true; }

        boost::system::error_code ec;
        socket = std::make_unique<boost::asio::ip::udp::socket>(*io_service);
        socket->open(localEndpoint.protocol(), ec);
        if (ec.value() != 0) {
            ErrorMessage_T msg("Open failed with error: ");
            msg += ec.message();
            msg += ", port: ";
            msg += NumberToString(localEndpoint.port());
            ops::BasicError err("UDPSender", "Open", msg);
            Participant::reportStaticError(&err);
            socket.reset();
            return false;
        }

        if (_outSocketBufferSize > 0) {
            boost::asio::socket_base::send_buffer_size option(_outSocketBufferSize);
            boost::system::error_code ec1, ec2;
            ec1 = socket->set_option(option, ec1);
            socket->get_option(option, ec2);
            if ((ec1.value() != 0) || (ec2.value() != 0) || option.value() != _outSocketBufferSize)
            {
                ErrorMessage_T msg("Socket buffer size ");
                msg += NumberToString(_outSocketBufferSize);
                msg += " could not be set. Used value: ";
                msg += NumberToString(option.value());
                ops::BasicWarning err("UDPSender", "Open", msg);
                Participant::reportStaticError(&err);
            }
        }

        if (_multicastSocket) {
            const boost::asio::ip::multicast::hops ttlOption(_ttl);
            socket->set_option(ttlOption, ec);
            if (ec.value() != 0) {
                ErrorMessage_T msg("Set TTL failed with error: ");
                msg += ec.message();
                ops::BasicWarning err("UDPSender", "Open", msg);
                Participant::reportStaticError(&err);
                ec.clear();
            }

            const boost::asio::ip::address_v4 local_interface = boost::asio::ip::address_v4::from_string(_localInterface.c_str());
            const boost::asio::ip::multicast::outbound_interface ifOption(local_interface);
            socket->set_option(ifOption, ec);
            if (ec.value() != 0) {
                ErrorMessage_T msg("Set MC Interface failed with error: ");
                msg += ec.message();
                ops::BasicWarning err("UDPSender", "Open", msg);
                Participant::reportStaticError(&err);
                ec.clear();
            }
        }

#if BOOST_VERSION > 106500
        socket->non_blocking(true, ec);
#else
        boost::asio::socket_base::non_blocking_io command(true);
        socket->io_control(command, ec);
#endif
        if (ec.value() != 0) {
            ErrorMessage_T msg("Set non-blocking failed with error: ");
            msg += ec.message();
            ops::BasicWarning err("UDPSender", "Open", msg);
            Participant::reportStaticError(&err);
            ec.clear();
        }

        socket->bind(localEndpoint, ec);
        if (ec.value() != 0) {
            ErrorMessage_T msg("Bind failed with error: ");
            msg += ec.message();
            ops::BasicError err("UDPSender", "Open", msg);
            Participant::reportStaticError(&err);
            close();
            return false;
        }
        return true;
	}

	void UDPSender::close()
	{
		if (socket != nullptr) {
	        socket->close();
			socket.reset();
		}
	}

    bool UDPSender::sendTo(const char* const buf, const int size, const Address_T& ip, const uint16_t port)
    {
		if (socket == nullptr) { return false; }
        try
        {
            const boost::asio::ip::address ipaddress = boost::asio::ip::address::from_string(ip.c_str());
            const boost::asio::ip::udp::endpoint endpoint(ipaddress, port);
            const std::size_t res = socket->send_to(boost::asio::buffer(buf, size), endpoint);
			if (res != (std::size_t)size) {
				OPS_UDP_ERROR("UDPSender: sendTo(), Error: Failed to write message (" << size << "), res: " << res << "]\n");
				return false;
			}
            return true;
        }
		catch (const std::exception& ex)
        {
			ErrorMessage_T msg("Error when sending udp message: ");
			msg += ex.what();
			msg += " Params: buf = ";
			msg += NumberToString((size_t)buf);
			msg += ", size = ";
			msg += NumberToString(size);
			msg += ", ip = ";
			msg += ip;
			msg += ", port = ";
			msg += NumberToString(port);
			ops::BasicError err("UDPSender", "sendTo", msg);
			Participant::reportStaticError(&err);
            return false;
        }
        catch (...)
        {
			ErrorMessage_T msg("Error when sending udp message: Params: buf = ");
			msg += NumberToString((size_t)buf);
			msg += ", size = ";
			msg += NumberToString(size);
			msg += ", ip = ";
			msg += ip;
			msg += ", port = ";
			msg += NumberToString(port);
			ops::BasicError err("UDPSender", "sendTo", msg);
			Participant::reportStaticError(&err);
            return false;
        }
    }

}

