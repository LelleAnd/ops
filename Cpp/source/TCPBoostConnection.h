/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2020 Lennart Andersson.
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

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#include "OPSTypeDefs.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "TCPConnection.h"

namespace ops
{
	// Helper class for a connected TCP socket
	class TCPBoostConnection : public TCPConnection
	{
	protected:
		boost::asio::ip::tcp::socket* _sock;
		Address_T _remoteAddress;
        uint32_t _remoteAddressHost = 0;
		uint16_t _remotePort = 0;

		// Used for a connection created by TCPClient
		explicit TCPBoostConnection(TCPConnectionCallbacks* client) :
			TCPConnection(client),
			_sock(nullptr)
		{
		}

	public:
		// Used for connections created by TCPServer
		// In this case the socket is already connected
		explicit TCPBoostConnection(TCPConnectionCallbacks* client, boost::asio::ip::tcp::socket* sock, int outSocketBufferSize) :
			TCPConnection(client),
			_sock(sock)
		{
			_connected = true;
			setOutSize(outSocketBufferSize);
			disableNagleAlg();
		}

		virtual ~TCPBoostConnection()
		{
			close();
			if (_sock != nullptr) delete _sock;
		}

		void close() override
		{
			if (_sock != nullptr) _sock->close();
		}

		void setOutSize(int size)
		{
			if ((_sock != nullptr) && (size > 0)) {
				boost::asio::socket_base::send_buffer_size option(size);
				boost::system::error_code ec;
				ec = _sock->set_option(option, ec);
				_sock->get_option(option);
				if (ec || option.value() != size) {
					ErrorMessage_T msg("Socket buffer size ");
					msg += NumberToString(size);
					msg += " could not be set. Used value: ";
					msg += NumberToString(option.value());
					BasicWarning err("TCPBoostConnection", "setOutSize", msg);
					Participant::reportStaticError(&err);
				}
			}
		}

		void setInSize(int size)
		{
			if ((_sock != nullptr) && (size > 0)) {
				boost::asio::socket_base::receive_buffer_size option(size);
				boost::system::error_code ec;
				ec = _sock->set_option(option, ec);
				_sock->get_option(option);
				if (ec || option.value() != size) {
					ErrorMessage_T msg("Socket buffer size ");
					msg += NumberToString(size);
					msg += " could not be set. Used value: ";
					msg += NumberToString(option.value());
					ops::BasicWarning err("TCPBoostConnection", "setInSize", msg);
					Participant::reportStaticError(&err);
				}
			}
		}

		void disableNagleAlg()
		{
			if (_sock != nullptr) {
				const boost::asio::ip::tcp::no_delay option(true);
				boost::system::error_code error;
				_sock->set_option(option, error);
				if (error) {
					ops::BasicWarning warn("TCPBoostConnection", "disableNagleAlg", "Failed to disable Nagle algorithm.");
					Participant::reportStaticError(&warn);
				}
			}
		}

		void getRemoteEndPoint()
		{
			if (_connected && (_sock != nullptr)) {
				boost::system::error_code error;
				boost::asio::ip::tcp::endpoint sendingEndPoint;
				sendingEndPoint = _sock->remote_endpoint(error);
				if (!error) {
                    _remoteAddressHost = sendingEndPoint.address().to_v4().to_ulong();
					_remoteAddress = sendingEndPoint.address().to_string().c_str();
					_remotePort = sendingEndPoint.port();
				}
			}
		}

		void startAsyncRead(char* bytes, uint32_t size) override
		{
			if (_connected) {
				OPS_TCP_TRACE("Conn: startAsyncRead(), size " << size << '\n');
				std::shared_ptr<TCPConnection> self = shared_from_this();
				_sock->async_receive(
					boost::asio::buffer(bytes, size),
					[self](const boost::system::error_code& error, size_t nrBytesReceived) {
						std::dynamic_pointer_cast<TCPBoostConnection>(self)->handleReceivedData(error.value(), (uint32_t)nrBytesReceived);
					}
				);
			}
		}

        size_t bytesAvailable() override
        {
            boost::asio::socket_base::bytes_readable command(true);
            _sock->io_control(command);
            return command.get();
        }

        int send(const char* buf, const uint32_t size) override
		{
			try {
				// Send the data
				return (int)boost::asio::write(*_sock, boost::asio::buffer(buf, size));
			} catch (const std::exception& e) {
				ErrorMessage_T msg("Exception: ");
				msg += e.what();
				BasicError err("TCPBoostConnection", "send", msg);
				Participant::reportStaticError(&err);
			}
			return -1;
		}

		void getRemote(Address_T& address, uint16_t& port) override
		{
			// We cache the address and port for TCP, since it is the same until disconnected
			if (_remotePort == 0) {
				getRemoteEndPoint();
			}
			address = _remoteAddress;
			port = _remotePort;
		}

        void getRemote(uint32_t& address, uint16_t& port) override
        {
            // We cache the address and port for TCP, since it is the same until disconnected
            if (_remotePort == 0) {
                getRemoteEndPoint();
            }
            address = _remoteAddressHost;
            port = _remotePort;
        }

        void getLocal(Address_T& address, uint16_t& port) override
		{
			boost::system::error_code error;
			boost::asio::ip::tcp::endpoint localEndPoint;
			localEndPoint = _sock->local_endpoint(error);
			address = localEndPoint.address().to_string().c_str();
			port = localEndPoint.port();
		}
	};
}
