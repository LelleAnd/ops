/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020 Lennart Andersson.
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

#ifndef ops_UDPReceiverH
#define ops_UDPReceiverH

#include <iostream>

#include "Participant.h"
#include "Receiver.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <boost/asio.hpp>
#include "boost/bind.hpp"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "ByteBuffer.h"
#include "BoostIOServiceImpl.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "Compatibility.h"

namespace ops
{
    using boost::asio::ip::udp;

    class UDPReceiver : public Receiver
    {
    public:
        UDPReceiver(uint16_t bindPort, IOService* ioServ, Address_T localInterface = "0.0.0.0", int inSocketBufferSizent = 16000000)
        {
            boost::asio::io_service* ioService = BoostIOServiceImpl::get(ioServ);

            if (localInterface == "0.0.0.0") {
                udp::resolver resolver(*ioService);
                udp::resolver::query query(boost::asio::ip::host_name(), "");
                udp::resolver::iterator it = resolver.resolve(query);
                udp::resolver::iterator end;
                while (it != end) {
                    const boost::asio::ip::address addr = it->endpoint().address();
                    if (addr.is_v4()) {
                        ipaddress = addr.to_string().c_str();
                        localEndpoint = new udp::endpoint(addr, bindPort);
                        break;
                    }
                    ++it;
                }
            } else {
                const boost::asio::ip::address ipAddr(boost::asio::ip::address_v4::from_string(localInterface.c_str()));
                localEndpoint = new boost::asio::ip::udp::endpoint(ipAddr, bindPort);
				ipaddress = localInterface;
            }

            sock = new boost::asio::ip::udp::socket(*ioService);

            sock->open(localEndpoint->protocol());

            if (inSocketBufferSizent > 0) {
                boost::asio::socket_base::receive_buffer_size option(inSocketBufferSizent);
                boost::system::error_code ec;
                ec = sock->set_option(option, ec);
                sock->get_option(option);
                if (ec || option.value() != inSocketBufferSizent) {
					ErrorMessage_T msg("Socket buffer size ");
					msg += NumberToString(inSocketBufferSizent);
					msg += " could not be set. Used value: ";
					msg += NumberToString(option.value());
					ops::BasicWarning err("UDPReceiver", "UDPReceiver", msg);
                    Participant::reportStaticError(&err);
                }
            }

			try {
	            //Note, takes address even if in use.
		        sock->set_option(boost::asio::ip::udp::socket::reuse_address(true));
			    sock->bind(*localEndpoint);

				//ipaddress = localEndpoint->address().to_string();
	            port = sock->local_endpoint().port();
			} catch (...) {
				ops::BasicError err("UDPReceiver", "UDPReceiver", "Failed to setup UDP socket. Check address");
				Participant::reportStaticError(&err);
				stop();
			}
        }

        virtual void asynchWait(char* bytes, int size) override
        {
            data = bytes;
            max_length = size;
			// Set variables indicating that we are "active"
			m_working = true;
			m_asyncCallActive = true;
            sock->async_receive_from(
                    boost::asio::buffer(data, max_length),
					sendingEndPoint,
                    boost::bind(&UDPReceiver::handle_receive_from, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }

		// Used to get the sender IP and port for a received message
		// Only safe to call in callback, before a new asynchWait() is called.
		virtual void getSource(Address_T& address, uint16_t& port) override
		{
			address = sendingEndPoint.address().to_string().c_str();
			port = sendingEndPoint.port();
		}

        virtual void getSource(uint32_t& address, uint16_t& port) override
        {
            address = sendingEndPoint.address().to_v4().to_ulong();
            port = sendingEndPoint.port();
        }

        size_t bytesAvailable() override
        {
            boost::asio::socket_base::bytes_readable command(true);
            sock->io_control(command);
            return command.get();
        }

        void handle_receive_from(const boost::system::error_code& error, size_t nrBytesReceived)
        {
			m_asyncCallActive = false;	// No longer a call active, thou we may start a new one below
            if (!cancelled) {
                if (!error && nrBytesReceived > 0) {
                    notifyNewEvent(BytesSizePair(data, (int)nrBytesReceived));

                } else {
			        handleReadError(error);
				}
			}
			// We update the "m_working" flag as the last thing in the callback, so we don't access the object any more 
			// in case the destructor has been called and waiting for us to be finished.
			// If we haven't started a new async call above, this will clear the flag.
			m_working = m_asyncCallActive;
        }

        void handleReadError(const boost::system::error_code& error)
        {
			ops::BasicError err("UDPReceiver", "handleReadError", "Error");
            Participant::reportStaticError(&err);

			//WSAEFAULT (10014) "Illegal buffer address" is fatal, happens e.g. if a too small buffer is given and
			// it probably wont go away by calling the same again, so just report error and then exit without 
			// starting a new async_receive().
#ifdef _WIN32
            if (error.value() == WSAEFAULT) { return; }
#else
            UNUSED(error);
///TODO LINUX			if (error.value() == WSAEFAULT) return;
#endif

			asynchWait(data, max_length);
        }

        ~UDPReceiver()
        {
			// Make sure socket is closed
            cancelled = true;
            if (sock != nullptr) { sock->close(); }

			/// We must handle asynchronous callbacks that haven't finished yet.
			/// This approach works, but the recommended boost way is to use a shared pointer to the instance object
			/// between the "normal" code and the callbacks, so the callbacks can check if the object exists.
			while (m_working) { 
				Sleep(1);
			}

            if (sock != nullptr) { delete sock; }
            if (localEndpoint != nullptr) { delete localEndpoint; }
        }

        int receive(char* buf, int size)
        {
            try {
                size_t nReceived = sock->receive_from(boost::asio::buffer(buf, size), lastEndpoint);
                return (int)nReceived;
            } catch (...) {
				ops::BasicError err("UDPReceiver", "receive", "Exception");
                Participant::reportStaticError(&err);
                return -1;
            }
        }

        size_t available() const
        {
            return sock->available();
        }

        bool sendReply(char* buf, int size)
        {
            try {
                std::size_t res = sock->send_to(boost::asio::buffer(buf, size), lastEndpoint);
				if (res != (std::size_t)size) {
					OPS_UDP_ERROR("UDPReceiver: sendReply(), Error: Failed to write message (" << size << "), res: " << res << "]\n");
					return false;
				}
				return true;
            } catch (...) {
                return false;
            }
        }

        virtual uint16_t getLocalPort() noexcept override
        {
            return port;
        }

		virtual Address_T getLocalAddress() noexcept override
        {
            return ipaddress;
        }

        ///Override from Receiver
		virtual bool start() override
		{
			/// The UDP Receiver is open the whole life time
            cancelled = false;
            return true;
		}

        virtual void stop() override
        {
			/// The UDP Receiver is open the whole life time
			// Cancel the asynch_receive()
            cancelled = true;
			//sock->cancel(); ///FAILS ON WIN XP
            if (sock) { sock->close(); }

			/// We must handle asynchronous callbacks that haven't finished yet.
			/// This approach works, but the recommended boost way is to use a shared pointer to the instance object
			/// between the "normal" code and the callbacks, so the callbacks can check if the object exists.
			while (m_working) { 
				Sleep(1);
			}
        }

		// Returns true if all asynchronous work has finished
		virtual bool asyncFinished() override
		{
			return !m_working;
		}

    private:
        uint16_t port = 0;
		Address_T ipaddress;
        boost::asio::ip::udp::socket* sock = nullptr;
        boost::asio::ip::udp::endpoint* localEndpoint = nullptr;
        boost::asio::ip::udp::endpoint lastEndpoint;
		boost::asio::ip::udp::endpoint sendingEndPoint;

        int max_length = 65535; 
        char* data = nullptr;

        bool cancelled = false;

		// Variables to keep track of our outstanding requests, that will result in callbacks to us
		volatile bool m_asyncCallActive = false;
		volatile bool m_working = false;
    };
}
#endif
