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

#include <ops.h>
#include <ByteBuffer.h>
#include <MemoryMap.h>
#include <OPSArchiverOut.h>
#include <OPSArchiverIn.h>

#include "BridgeTypeFactory.h"
#include "CSocketTransport.h"
#include "BridgeMessages.h"
#include "BridgeLog.h"

namespace opsbridge {

CSocketTransport::CSocketTransport():
	m_socketCom(INVALID_SOCKET), m_Connected(false)
{
	m_factory.add( new BridgeTypeFactory() );
}


CSocketTransport::~CSocketTransport()
{
}

void CSocketTransport::Terminate()
{
	CTransport::Terminate();
	disconnect();
}

void CSocketTransport::disconnect()
{
	Close();
	m_Connected = false;
}

int CSocketTransport::GetError()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

void CSocketTransport::Close()
{
#ifdef _WIN32
	::shutdown(m_socketCom, SD_BOTH);
	::closesocket(m_socketCom);
#else
	::shutdown(m_socketCom, SHUT_RDWR);
	::close(m_socketCom);
#endif
	m_socketCom = INVALID_SOCKET;
}

bool CSocketTransport::write(const char* const buffer, uint32_t const length)
{
	if (!m_Connected) { return false; }
	
	int const iResult = (int const)send(m_socketCom, buffer, length, 0);

	if (iResult == 0) {
		BL_TRACE("# [ SocketTransport ] send() returned 0\n");
		return false;

	} else if (iResult == SOCKET_ERROR) {
		BL_ERROR("# [ SocketTransport ] send() failed with error: %d\n", GetError());
		return false;
	}

	return true;
}

bool CSocketTransport::writeOpsMessage(ops::OPSObject* const mess, 
									   const ops::ObjectName_T publisherName,
									   const ops::ObjectName_T topicName,
									   const uint64_t ackCounter)
{
	ops::MemoryMap memMap(1, 65536);
	ops::ByteBuffer buf(memMap);
	ops::OPSArchiverOut archive(buf, false);

	// Build header
	//   First a TOpsMessage structure
	TOpsMessage head;
	head.Head.Length = 0;	// replaced below with real length
	head.Head.Type = mtOps;
	head.PartNo = 1;
	head.TotalParts = 1;
	head.DataLength = (uint32_t)mess->spareBytes.size();
	head.AckCounter = ackCounter;

	//   Followed by TopicName & PublisherName
	buf.WriteString(topicName);
	buf.WriteString(publisherName);

	//   Followed by Message body. Serialize OPS message into a data buffer (excl. sparebytes)
	archive.inout("data", mess);

	//	 Rest of data from mess.spareBytes 
	//   Direcly in write call below.

	// Update header length with real size of header
	head.Head.Length = (uint32_t)sizeof(head) + buf.GetSize();

	// Send on transport
	// We need to lock the writes since the complete message requires 3 writes
	ops::SafeLock const lock(m_writeLock);
	if (!write((char *)&head, sizeof(head))) { return false; }
	if (!write(memMap.getSegment(0), buf.GetSize())) { return false; }
	return write(&mess->spareBytes[0], head.DataLength);
}

bool CSocketTransport::writeAckNak(uint64_t const ackCounter, uint32_t const errorCode)
{
	TAckNakMessage AckNak;
	AckNak.Head.Type = mtAckNak;
	AckNak.Head.Length = sizeof(AckNak);
	AckNak.AckCounter = ackCounter;
	AckNak.ErrorCode = errorCode;
	
	// Send on transport
	// We need to lock the writes
	ops::SafeLock const lock(m_writeLock);
	return write((char *)&AckNak, sizeof(AckNak));
}

// The .Head field in cmd and status will be overwritten
bool CSocketTransport::writeCommand(TCommandMessage& cmd)
{
	ops::MemoryMap memMap(1, 1000);
	ops::ByteBuffer buf(memMap);

	cmd.Head.Type = mtCommand;
	cmd.Head.Length = 0;	// replaced below with real length

	int const tmp = cmd.Command;
	buf.WriteInt(tmp);
	buf.WriteString(cmd.DestTopicName);
	buf.WriteString(cmd.DataType);
	buf.WriteLong(cmd.AckCounter);

	// Update header length with real size of header
	cmd.Head.Length = (uint32_t)sizeof(cmd.Head) + buf.GetSize();

	// Send on transport
	// We need to lock the writes
	ops::SafeLock const lock(m_writeLock);
	if (!write((char *)&cmd.Head, sizeof(cmd.Head))) { return false; }
	return write(memMap.getSegment(0), buf.GetSize());
}

bool CSocketTransport::writeStatus(TStatusMessage& status)
{
	status.Head.Type = mtStatus;
	status.Head.Length = sizeof(status);
	// Send on transport
	// We need to lock the writes
	ops::SafeLock const lock(m_writeLock);
	return write((char *)&status, sizeof(status));
}

// The .Head field in UdpMc will be overwritten
bool CSocketTransport::writeUdpMcMessage(TUdpMcMessage& udpMc, const char* const data)
{
	udpMc.Head.Type = mtUdpMc;
	udpMc.Head.Length = sizeof(udpMc);

	// Send on transport
	// We need to lock the writes
	ops::SafeLock const lock(m_writeLock);
	if (!write((const char *)&udpMc, sizeof(udpMc))) { return false; }
	return write(data, udpMc.DataLength);
}

bool CSocketTransport::ReadData(char* buffer, uint32_t const numBytes)
{
	uint32_t numRead = 0;

    // Loop until the requested number of bytes have been read
	while (numRead < numBytes) {
		int const iResult = (int const)recv(m_socketCom, &buffer[numRead], numBytes - numRead, 0);
		
		if (iResult > 0) {
			numRead += iResult;

		} else if (iResult == 0) {
			BL_TRACE("# [ SocketTransport ] recv() returned 0 (DISCONNECT)\n");
			return false;

		} else {
			BL_ERROR("# [ SocketTransport ] recv() failed with error: %d\n", GetError());
			return false;
		}
	}
	return true;
}

// Called from the transport thread when we are connected
void CSocketTransport::HandleData()
{
	ops::MemoryMap memMap(1, 65536);
	char *Ptr;
	std::string topicName;
	std::string publisherName;
	THeader Head;
	TOpsMessage OpsMess;
	TAckNakMessage AckNakMess;
	TCommandMessage CmdMess;
	TStatusMessage StatusMess;
	TUdpMcMessage UdpMcMess;

	while (!terminated()) {
		// All messages start with a THeader with Length and Type
		// Read data into Head and validate
		if (!ReadData((char*)&Head, sizeof(Head))) { return; }
		if (terminated()) { return; }
		if ((Head.Length < sizeof(Head)) || (Head.Length > 2048)) {
			BL_ERROR("# [ SocketTransport ] HandleData() Head.Length invalid (%u)\n", Head.Length);
			return;
		}

		switch (Head.Type) {
		case mtOps:
		{
			ops::OPSObject* messHead = nullptr;
			ops::ByteBuffer buf(memMap);
			ops::OPSArchiverIn archive(buf, &m_factory);

			// Read rest of data for TOpsMessage
			OpsMess.Head = Head;
			Ptr = (char*)&OpsMess;
			Ptr += sizeof(Head);

			if (!ReadData(Ptr, sizeof(OpsMess) - sizeof(Head))) { return; }
			if (terminated()) { return; }

			if ((Head.Length < sizeof(OpsMess)) || 
				((Head.Length - sizeof(OpsMess)) > (uint32_t)memMap.getSegmentSize()))
			{
				BL_ERROR("# [ SocketTransport ] HandleData(OPS) Head.Length invalid (%u)\n", Head.Length);
				return;
			}

			// Read rest of header into buffer for deserializing
			if (!ReadData(memMap.getSegment(0), Head.Length - (uint32_t)sizeof(OpsMess))) { return; }
			if (terminated()) { return; }

			topicName = buf.ReadString();
			publisherName = buf.ReadString();

			// Create OPS Object
			messHead = dynamic_cast<ops::OPSObject*>(archive.inout("data", messHead));
			if (messHead == nullptr) {
				BL_ERROR("# [ SocketTransport ] HandleData() Failed to create OPS object\n");
				return;
			}

			///TODO validate OpsMess.DataLength

			// Read rest of data into sparebytes
			messHead->spareBytes.resize(OpsMess.DataLength);
			if (ReadData((char*)&messHead->spareBytes[0], OpsMess.DataLength)) {
				if (!terminated()) {
					if (m_user != nullptr) { m_user->onOpsMessage(this, messHead, publisherName, topicName, OpsMess.AckCounter); }
				}
			}
			delete messHead;
		}
		break;

		case mtAckNak:
		{
			// Read rest of data for TAckNakMessage
			AckNakMess.Head = Head;
			Ptr = (char*)&AckNakMess;
			Ptr += sizeof(Head);

			if (Head.Length != sizeof(AckNakMess)) {
				BL_ERROR("# [ SocketTransport ] HandleData(AckNak) Head.Length invalid (%u)\n", Head.Length);
				return;
			}

			if (!ReadData(Ptr, Head.Length - (uint32_t)sizeof(Head))) { return; }
			if (terminated()) { return; }

			if (m_user != nullptr) { m_user->onAckNakMessage(this, AckNakMess); }
		}
		break;

		case mtCommand:
		{
			ops::ByteBuffer buf(memMap);

			// Read rest of data for TCommandMessage
			CmdMess.Head = Head;

			// Read rest of header into buffer for deserializing
			if (!ReadData(memMap.getSegment(0), Head.Length - (uint32_t)sizeof(CmdMess.Head))) { return; }
			if (terminated()) { return; }

			CmdMess.Command = (TCommandType)buf.ReadInt();
			CmdMess.DestTopicName = buf.ReadString();
			CmdMess.DataType = buf.ReadString();
			CmdMess.AckCounter = buf.ReadLong();

			if (m_user != nullptr) { m_user->onCommandMessage(this, CmdMess); }
		}
		break;

		case mtStatus:
		{
			// Read rest of data for TStatusMessage
			StatusMess.Head = Head;
			Ptr = (char*)&StatusMess;
			Ptr += sizeof(Head);

			if (Head.Length != sizeof(StatusMess)) {
				BL_ERROR("# [ SocketTransport ] HandleData(Status) Head.Length invalid (%u)\n", Head.Length);
				return;
			}

			if (!ReadData(Ptr, Head.Length - (uint32_t)sizeof(Head))) { return; }
			if (terminated()) { return; }

			if (m_user != nullptr) { m_user->onStatusMessage(this, StatusMess); }
		}
		break;

		case mtUdpMc:
		{
			// Read rest of data for TUdpMcMessage
			UdpMcMess.Head = Head;
			Ptr = (char*)&UdpMcMess;
			Ptr += sizeof(Head);

			if (Head.Length != sizeof(UdpMcMess)) {
				BL_ERROR("# [ SocketTransport ] HandleData(UdpMc) Head.Length invalid (%u)\n", Head.Length);
				return;
			}

			if (!ReadData(Ptr, Head.Length - (uint32_t)sizeof(Head))) { return; }
			if (terminated()) { return; }

			if (UdpMcMess.DataLength > (uint32_t)memMap.getSegmentSize()) {
				BL_ERROR("# [ SocketTransport ] HandleData(UdpMc) DataLength invalid (%u)\n", UdpMcMess.DataLength);
				return;
			}

			// Read actual data
			if (!ReadData(memMap.getSegment(0), UdpMcMess.DataLength)) { return; }
			if (terminated()) { return; }

			if (m_user != nullptr) { m_user->onUdpMcMessage(this, UdpMcMess, memMap.getSegment(0)); }
		}
		break;

		default:
			// This is an error so return from this routine
			BL_ERROR("# [ SocketTransport ] HandleData() UNKNOWN DATA received\n");
			return;
            break;
		}
	}
}

}

