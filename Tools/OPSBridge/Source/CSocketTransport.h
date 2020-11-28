/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>

	#define SOCKET int
#endif

#include "Lockable.h"

#include "CTransport.h"

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

namespace opsbridge {

	class CSocketTransport : public CTransport
	{
	public:
		CSocketTransport();
		~CSocketTransport();

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
		bool writeOpsMessage(ops::OPSObject* mess,
			const ops::ObjectName_T publisherName,
			const ops::ObjectName_T topicName,
			const uint64_t ackCounter) override;
		bool writeAckNak(uint64_t ackCounter, uint32_t errorCode) override;

		// The .Head field in cmd and status will be overwritten
		bool writeCommand(TCommandMessage& cmd) override;
		bool writeStatus(TStatusMessage& status) override;

		// The .Head field in UdpMc will be overwritten
		bool writeUdpMcMessage(TUdpMcMessage& udpMc, const char* data) override;

		void disconnect();

		void Terminate();

	protected:
		// Handle for read/write
		SOCKET m_socketCom;
		bool m_Connected;		// connected or not

		bool ReadData(char* buffer, uint32_t numBytes);
		void HandleData();

		int GetError();
		void Close();

	private:
		ops::Lockable m_writeLock;
		ops::SerializableInheritingTypeFactory m_factory;

		bool write(const char* const buffer, uint32_t const length);
	};

}
