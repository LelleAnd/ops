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

#pragma once

#include <chrono>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <boost/asio.hpp>
#include <boost/array.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "TCPClientBase.h"
#include "IOService.h"
#include "BoostIOServiceImpl.h"
#include "Participant.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "TCPBoostConnection.h"

namespace ops
{

    class TCPClient : public TCPClientBase
    {

		// Internal helper class that handles the socket connection
		class TCPBoostConnectionWConnect : public TCPBoostConnection
		{
			TCPClient* _owner;
			volatile bool _tryToConnect;
			std::unique_ptr<boost::asio::ip::tcp::endpoint> _endpoint;
			int _inBufferSize;
			boost::asio::steady_timer _timer;

		public:
			TCPBoostConnectionWConnect(TCPClient* owner, Address_T serverIP, uint16_t serverPort, IOService* ioServ, int inBufferSize) :
				TCPBoostConnection(owner),
				_owner(owner), _tryToConnect(false), _inBufferSize(inBufferSize),
				_timer(*BoostIOServiceImpl::get(ioServ))
			{
				boost::asio::io_service* ioService = BoostIOServiceImpl::get(ioServ);
				const boost::asio::ip::address ipAddr(boost::asio::ip::address_v4::from_string(serverIP.c_str()));
				_endpoint = std::make_unique<boost::asio::ip::tcp::endpoint>(ipAddr, serverPort);
				_sock = std::make_unique<boost::asio::ip::tcp::socket>(*ioService);
			}

			~TCPBoostConnectionWConnect() = default;

			void start() override
			{
				_tryToConnect = true;
				start_async_connect();
			}

			void start_async_connect()
			{
				OPS_TCP_TRACE("Client: start_async_connect()\n");
				_connected = false;
				_remotePort = 0;

				std::shared_ptr<TCPConnection> self = shared_from_this();
				_sock->async_connect(
					*_endpoint,
					[self](const boost::system::error_code& error) {
						std::dynamic_pointer_cast<TCPBoostConnectionWConnect>(self)->handleConnect(error);
					}
				);
			}

			void stop() override
			{
				if (_tryToConnect) {
					_tryToConnect = false;
					_connected = false;
					_remotePort = 0;
					_timer.cancel();
					if (_sock) _sock->close();
					_owner->connected(false);
				}
			}

			void handleConnect(const boost::system::error_code& error)
			{
				OPS_TCP_TRACE("Client: handleConnect(), error: " << error << '\n');
				try {
					if (_tryToConnect) {
						if (error) {
							_owner->connected(false);

							// Delay new connect attempt
							std::shared_ptr<TCPConnection> self = shared_from_this();
							_timer.cancel();
							_timer.expires_from_now(std::chrono::milliseconds(100));
							_timer.async_wait([self](const boost::system::error_code& e) {
								OPS_TCP_TRACE("Client: handleConnect(), delay error: " << e << '\n');
								if (e != boost::asio::error::operation_aborted) {
									// Timer was not cancelled, connect again
									std::dynamic_pointer_cast<TCPBoostConnectionWConnect>(self)->start_async_connect();
								}
							});

						} else {
							_connected = true;

							setInSize(_inBufferSize);
							disableNagleAlg();
							getRemoteEndPoint();

							_owner->connected(true);
						}
					}
				} catch (const std::exception& e) {
					ExceptionMessage_T msg("Unknown exception: ");
					msg += e.what();
					ops::BasicWarning err("TCPClient", "handleConnect", msg);
					Participant::reportStaticError(&err);
				}
			}
		};

	public:
        TCPClient(TCPClientCallbacks* client, Address_T serverIP, uint16_t serverPort, IOService* ioServ, int inSocketBufferSizent = 16000000) :
			TCPClientBase(client, ioServ, std::make_shared<TCPBoostConnectionWConnect>(this, serverIP, serverPort, ioServ, inSocketBufferSizent))
        {}

		bool start() override
		{
			const bool res = TCPClientBase::start();
			std::shared_ptr<TCPBoostConnectionWConnect> sp = std::dynamic_pointer_cast<TCPBoostConnectionWConnect>(_connection);
			sp->start();
            return res;
		}

        void stop() override
        {
			TCPClientBase::stop();
			std::shared_ptr<TCPBoostConnectionWConnect> sp = std::dynamic_pointer_cast<TCPBoostConnectionWConnect>(_connection);
			sp->stop();
		}

		// Used to get the sender IP and port for a received message
		// Only safe to call in callback, before a new asynchWait() is called.
		void getSource(Address_T& address, uint16_t& port) override
		{
			_connection->getRemote(address, port);
		}
        virtual void getSource(uint32_t& address, uint16_t& port) override
        {
            _connection->getRemote(address, port);
        }


		virtual bool asyncFinished() override
		{
			return true; ///TODO to be removed when other transports also fixed
		}

		virtual ~TCPClient()
		{
			// Make sure socket is closed
			TCPClient::stop();
        }

		bool isConnected() override
		{ 
			return _connection->isConnected();
		}

		uint16_t getLocalPort() override
		{
			Address_T address;
			uint16_t port;
			_connection->getLocal(address, port);
			return port;
		}

		Address_T getLocalAddress() override
		{
			Address_T address;
			uint16_t port;
			_connection->getLocal(address, port);
			return address;
		}

    };
}
