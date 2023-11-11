/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2023 Lennart Andersson.
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

#ifndef ops_MulticastReceiverH
#define ops_MulticastReceiverH

#include <iostream>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "Participant.h"
#include "Receiver.h"
#include "ByteBuffer.h"
#include "BoostIOServiceImpl.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "TimeHelper.h"

namespace ops
{
	using boost::asio::ip::udp;

	class MulticastReceiver : public Receiver
	{
	public:
		MulticastReceiver(Address_T mcAddress, uint16_t bindPort, IOService* ioServ, Address_T localInterface = "0.0.0.0", int inSocketBufferSizent = 16000000):
		  _ipaddress(mcAddress),
		  _localInterface(localInterface),
		  _inSocketBufferSizent(inSocketBufferSizent)
		{
			boost::asio::io_service* ioService = BoostIOServiceImpl::get(ioServ);
			//udp::resolver resolver(*ioService);
			//udp::resolver::query query(boost::asio::ip::host_name(),"");
			//udp::resolver::iterator it=resolver.resolve(query);
			//boost::asio::ip::address addr=(it++)->endpoint().address();

            // Linux needs INADDR_ANY here, for Windows it works with INADDR_ANY or localInterface
            const boost::asio::ip::address ipAddr(boost::asio::ip::address_v4::from_string("0.0.0.0"));
            //boost::asio::ip::address ipAddr(boost::asio::ip::address_v4::from_string(localInterface));

			localEndpoint = new boost::asio::ip::udp::endpoint(ipAddr, bindPort);

			sock = new boost::asio::ip::udp::socket(*ioService);
		}

        ///Override from Receiver
        virtual bool start() override
        {
            boost::system::error_code ec;
            sock->open(localEndpoint->protocol(), ec);
            if (ec) {
                ErrorMessage_T msg("Open failed with error: ");
                msg += ec.message();
                msg += ", port: ";
                msg += NumberToString(localEndpoint->port());
                ops::BasicError err("MulticastReceiver", "Start", msg);
                Participant::reportStaticError(&err);
                return false;
            }

			if (_inSocketBufferSizent > 0) {
				boost::asio::socket_base::receive_buffer_size option(_inSocketBufferSizent);
				boost::system::error_code ec1, ec2;
				ec1 = sock->set_option(option, ec1);
				sock->get_option(option, ec2);
				if (ec1 || ec2 || option.value() != _inSocketBufferSizent) {
					ErrorMessage_T msg("Socket buffer size ");
					msg += NumberToString(_inSocketBufferSizent);
					msg += " could not be set. Used value: ";
					msg += NumberToString(option.value());
					ops::BasicWarning err("MulticastReceiver", "Start", msg);
					Participant::reportStaticError(&err);
				}
			}

            sock->set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
            if (ec) {
                ErrorMessage_T msg("Set reuse address failed with error: ");
                msg += ec.message();
                ops::BasicWarning err("MulticastReceiver", "Start", msg);
                Participant::reportStaticError(&err);
                ec.clear();
            }
            sock->bind(*localEndpoint, ec);
            if (ec) {
                ErrorMessage_T msg("Bind failed with error: ");
                msg += ec.message();
                ops::BasicError err("MulticastReceiver", "Start", msg);
                Participant::reportStaticError(&err);
                sock->close();
                return false;
            }

            // Join the multicast group.
            const boost::asio::ip::address_v4 multicastAddress = boost::asio::ip::address_v4::from_string(_ipaddress.c_str());
            const boost::asio::ip::address_v4 networkInterface(boost::asio::ip::address_v4::from_string(_localInterface.c_str()));
            sock->set_option(boost::asio::ip::multicast::join_group(multicastAddress, networkInterface), ec);
            if (ec) {
                ErrorMessage_T msg("Join MC group failed with error: ");
                msg += ec.message();
                msg += ", MC Address: ";
                msg += _ipaddress;
                msg += ", Interface: ";
                msg += _localInterface;
                ops::BasicError err("MulticastReceiver", "Start", msg);
                Participant::reportStaticError(&err);
                sock->close();
                return false;
            }

#ifndef _WIN32
            // IP_MULTICAST_ALL (since Linux 2.6.31)
            // This option can be used to modify the delivery policy of multicast messages to sockets bound
            // to the wildcard INADDR_ANY address. The argument is a boolean integer (defaults to 1).
            // If set to 1, the socket will receive messages from all the groups that have been joined
            // globally on the whole system. Otherwise, it will deliver messages only from the groups that
            // have been explicitly joined (for example via the IP_ADD_MEMBERSHIP option) on this particular socket.
#if BOOST_VERSION > 106000
            int nsock = sock->native_handle();
#else
            int nsock = sock->native();
#endif
            if (nsock >= 0) {
                int mc_all = 0;
                if ((setsockopt(nsock, IPPROTO_IP, IP_MULTICAST_ALL, (void*)&mc_all, sizeof(mc_all))) < 0) {
                    perror("setsockopt() failed");
                }
            }
#endif

            _port = sock->local_endpoint().port();
            return true;
		}

		///Override from Receiver
		virtual void stop() override
		{
			cancelled = true;
            if (sock != nullptr) { sock->close(); }
		}

		// Returns true if all asynchronous work has finished
		virtual bool asyncFinished() noexcept override
		{
			return !m_working;
		}

		virtual void asynchWait(char* bytes, int size) override
		{
			data  = bytes;
			max_length = size;
			// Set variables indicating that we are "active"
			m_working = true;
			m_asyncCallActive = true;
			using namespace boost::placeholders;
			sock->async_receive_from(
				boost::asio::buffer(data, max_length),
				sendingEndPoint,
				boost::bind(&MulticastReceiver::handle_receive_from, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
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
            address = (uint32_t)sendingEndPoint.address().to_v4().to_ulong();
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
                    //printf("Data receivedm in multicast receiver\n");
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
			ErrorMessage_T message("Error: ");
			message += NumberToString(error.value());
			ops::BasicError err("MulticastReceiver", "handleReadError", message);
			Participant::reportStaticError(&err);

			//WSAEFAULT (10014) "Illegal buffer address" is fatal, happens e.g. if a too small buffer is given and
			// it probably wont go away by calling the same again, so just report error and then exit without
			// starting a new async_receive().
#ifdef _WIN32
            if (error.value() == WSAEFAULT) { return; }
#else
///TODO LINUX			if (error.value() == WSAEFAULT) return;
#endif

			asynchWait(data, max_length);
		}

		~MulticastReceiver()
		{
			// Make sure socket is closed
			MulticastReceiver::stop();

			/// We must handle asynchronous callbacks that haven't finished yet.
			/// This approach works, but the recommended boost way is to use a shared pointer to the instance object
			/// between the "normal" code and the callbacks, so the callbacks can check if the object exists.
			while (m_working) {
				TimeHelper::sleep(std::chrono::milliseconds(1));
			}

            if (sock != nullptr) { delete sock; }
            if (localEndpoint != nullptr) { delete localEndpoint; }
		}

		int receive(char* buf, int size)
		{
			try {
				size_t nReceived = sock->receive_from(boost::asio::buffer(buf, size), lastEndpoint);
				return (int)nReceived;
			} catch(...) {
				ops::BasicError err("MulticastReceiver", "receive", "Exception in MulticastReceiver::receive()");
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
					OPS_UDP_ERROR("MulticastReceiver: sendReply(), Error: Failed to write message (" << size << "), res: " << res << "]\n");
					return false;
				}
				return true;
			} catch (...) {
				return false;
			}
		}

		virtual uint16_t getLocalPort() noexcept override
		{
			return _port;
		}

		virtual Address_T getLocalAddress() noexcept override
		{
			return _ipaddress;
		}

	private:
		uint16_t _port = 0;
		Address_T _ipaddress;
		Address_T _localInterface;
		int _inSocketBufferSizent = 0;
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
