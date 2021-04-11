/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
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

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include "CTcpTransportServer.h"
#include "BridgeLog.h"

namespace opsbridge {

CTcpTransportServer::CTcpTransportServer(uint16_t const localPort):
	m_localPort(localPort), m_listenSocket(INVALID_SOCKET)
#ifdef _WIN32
	, m_WsaInitialized(false)
#endif
{
#ifdef _WIN32
	WSADATA wsaData;
	// Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    m_WsaInitialized = (iResult == NO_ERROR);
    if (!m_WsaInitialized)  {
		BL_ERROR("# [ TcpServer ] WSAStartup failed with error: %d\n", iResult);
    }
#endif
}

CTcpTransportServer::~CTcpTransportServer()
{
	Terminate();
	WaitFor();
#ifdef _WIN32
	if (m_WsaInitialized) WSACleanup();
#endif
}

void CTcpTransportServer::Terminate()
{
	CSocketTransport::Terminate();

	CloseServer();
}

void CTcpTransportServer::CloseServer()
{
	if (m_listenSocket != INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(m_listenSocket);
#else
		close(m_listenSocket);
#endif
	}
	m_listenSocket = INVALID_SOCKET;
}

void CTcpTransportServer::Run()
{
    int				nRet;           
	sockaddr_in		saServer{ 0 };
	sockaddr_in		SockAddr{ 0 };

	// Create socket
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET) {
		BL_ERROR("# [ TcpServer ] socket() creation failed with error: %d\n", GetError());
		return;
	}

	// Bind to server address/port
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY; 
	saServer.sin_port = htons(m_localPort);

	nRet = bind(m_listenSocket, (sockaddr*)&saServer, sizeof(saServer));
	if (nRet == SOCKET_ERROR) {
		BL_ERROR("# [ TcpServer ] bind() failed with error: %d\n", GetError());
		CloseServer();
		return;
	}
	
    // Listen for clients
	nRet = listen(m_listenSocket, SOMAXCONN);
    if (nRet == SOCKET_ERROR) {
		BL_ERROR("# [ TcpServer ] listen() failed with error: %d\n", GetError());
		CloseServer();
		return;
	}

#ifdef _WIN32
	typedef int socklen_t;
#endif

	//
	while (!terminated()) {
		// Connect phase
		while (!terminated()) {
		    // Accept a client
			socklen_t nLen = (socklen_t)sizeof(SockAddr);
			m_socketCom = accept(m_listenSocket, (sockaddr*)&SockAddr, &nLen);
			if (m_socketCom != INVALID_SOCKET) { break; }
			BL_ERROR("# [ TcpServer ] accept() failed with error: %d\n", GetError());

			// Wait a short while and then try again
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		if (!terminated()) {
			// Connected
			std::string remoteIP(inet_ntoa(SockAddr.sin_addr));
			BL_INFO("# [ TcpServer ] connected to: %s, Port: %d\n", remoteIP.c_str(), SockAddr.sin_port);

			int const optVal = 1;
			int const optLen = sizeof(optVal);
			setsockopt(m_socketCom, IPPROTO_TCP, TCP_NODELAY, (const char*)&optVal, optLen);

			m_Connected = true;
			if (m_user != nullptr) { m_user->onConnect(this); }

			// Handle data from other side while connected
			HandleData();

			m_Connected = false;
			if (m_user != nullptr) { m_user->onDisconnect(this); }

			BL_INFO("# [ TcpServer ] disconnected from: %s, Port: %d\n", remoteIP.c_str(), SockAddr.sin_port);
		}

		// Disconnect
		Close();
	}

	// Free all socket parts
	CloseServer();
}

}
