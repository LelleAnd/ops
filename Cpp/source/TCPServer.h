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

/* 
 * File:   TCPServer.h
 * Author: Anton Gravestam
 *
 * Created on den 22 oktober 2008, 20:01
 */

#pragma once

#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#include "Lockable.h"
#include "TCPServerBase.h"
#include "IOService.h" 
#include "BytesSizePair.h"
#include "BoostIOServiceImpl.h"
#include "Participant.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "TCPBoostConnection.h"

namespace ops
{

	class TCPServer : public TCPServerBase
    {

		// Internal helper class that handles the server socket
		class impl : public std::enable_shared_from_this<impl>
		{
			Lockable _ownerMtx;
			TCPServer* _owner = nullptr;
			boost::asio::io_service* _ioService = nullptr;
			boost::asio::ip::tcp::socket* _sock = nullptr;				// The socket that handles next accept.
			boost::asio::ip::tcp::acceptor* _acceptor = nullptr;
			volatile bool _canceled = false;
			int _outSocketBufferSize = 0;

		public:
			impl(TCPServer* owner, boost::asio::io_service* ioService) : _owner(owner), _ioService(ioService)
			{
				_sock = new boost::asio::ip::tcp::socket(*_ioService);
				// This constructor opens, sets reuse_address, binds and listens to the given endpoint.
				_acceptor = new boost::asio::ip::tcp::acceptor(*_ioService, *_owner->_endpoint);
				_outSocketBufferSize = _owner->_outSocketBufferSize;
			}

			~impl()
			{
				if (_acceptor) delete _acceptor;
				if (_sock) delete _sock;
			}

			void clearCallbacks()
			{
				// By holding the mutex while clearing _owner, we ensure that we can't 
				// be in a callback while clearing it. This also means that when this method returns
				// we can't call the _owner anymore and it's OK for the owner to vanish.
				SafeLock lck(&_ownerMtx);
				_owner = nullptr;
				OPS_TCP_TRACE("Server: Callbacks Cleared\n");
			}

			void start_accept()
			{
				OPS_TCP_TRACE("Server: start_accept()\n");
				std::shared_ptr<impl> self = shared_from_this();
				_acceptor->async_accept(*_sock,
					[self](const boost::system::error_code& error) {
						self->handleAccept(error);
					}
				);
			}

			void cancel()
			{
				OPS_TCP_TRACE("Server: cancel()\n");
				clearCallbacks();
				_canceled = true;
				_acceptor->close();
			}

			void handleAccept(const boost::system::error_code& error)
			{
				OPS_TCP_TRACE("Server: handleAccept(), error: " << error << '\n');
				if (!_canceled) {
					if (!error) {
						// By holding the mutex while in the callback, we are synchronized with clearCallbacks()
						SafeLock lck(&_ownerMtx);
						if (_owner) _owner->AddSocket(std::make_shared<TCPBoostConnection>(_owner, _sock, _outSocketBufferSize));
						_sock = new boost::asio::ip::tcp::socket(*_ioService);
					}
					start_accept();
				}
			}

			void getLocal(Address_T& address, uint32_t& addressHost, uint16_t& port)
			{
				boost::system::error_code error;
				boost::asio::ip::tcp::endpoint localEndPoint;
				localEndPoint = _acceptor->local_endpoint(error);
				address = localEndPoint.address().to_string().c_str();
                addressHost = (uint32_t)localEndPoint.address().to_v4().to_ulong();
				port = localEndPoint.port();
			}
		};

    public:
		TCPServer(TCPServerCallbacks* client, IOService* ioServ, Address_T serverIP, uint16_t serverPort, int outSocketBufferSize = 16000000) :
			TCPServerBase(client, ioServ),
			_serverPort(serverPort), _serverIP(serverIP), _outSocketBufferSize(outSocketBufferSize)
		{
			_ioService = BoostIOServiceImpl::get(ioServ);
			//boost::asio::ip::address ipAddr(boost::asio::ip::address_v4::from_string(serverIP));
			_endpoint = new boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverPort);
		}
		
		virtual ~TCPServer()
		{
			OPS_TCP_TRACE("Server: Destructor...\n");
			close();
			if (_endpoint) delete _endpoint;
			OPS_TCP_TRACE("Server: Destructor finished\n");
		}

		bool open() override
		{
			if (_server != nullptr) close();
			_server = std::make_shared<impl>(this, _ioService);
			_server->start_accept();

			Address_T addr;
            uint16_t port;
			_server->getLocal(addr, _serverIPHost, port);
			OPS_PIFO_TRACE("TCP Server IP: " << addr << ", Port: " << port << "\n");
            return true;
		}

		void close() override
		{
			if (_server != nullptr) {
				_server->cancel();
				_server.reset();
			}
			TCPServerBase::close();
		}

		uint16_t getLocalPort() override
		{
			if ((_serverPort == 0) && (_server != nullptr)) {
				Address_T addr;
				uint16_t port;
				_server->getLocal(addr, _serverIPHost, port);
				return port;
			}
			return _serverPort;
		}
        
		Address_T getLocalAddress() noexcept override
		{
			return _serverIP;
		}

        uint32_t getLocalAddressHost() noexcept override
        {
            if ((_serverIPHost == 0) && (_server != nullptr)) {
                Address_T addr;
                uint16_t port;
                _server->getLocal(addr, _serverIPHost, port);
            }
            return _serverIPHost;
        }

    private:
        uint32_t _serverIPHost{ 0 };
        uint16_t _serverPort{ 0 };
		Address_T _serverIP;
        int _outSocketBufferSize{ 0 };
        boost::asio::ip::tcp::endpoint* _endpoint{ nullptr };   // The local port to bind to.
        boost::asio::io_service* _ioService{ nullptr };         // Boost io_service handles the asynchronous operations on the sockets

		std::shared_ptr<impl> _server;
    };
}

