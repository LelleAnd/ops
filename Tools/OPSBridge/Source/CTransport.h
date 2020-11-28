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

#include <ops.h>

#include "CWorkerThread.h"
#include "BridgeMessages.h"

namespace opsbridge {

	class CTransport;

	class CTransportListener
	{
	public:
		virtual void onConnect(CTransport* sender) = 0;
		virtual void onDisconnect(CTransport* sender) = 0;

		virtual void onOpsMessage(CTransport* sender, 
			ops::OPSObject* mess, 
			ops::ObjectName_T publisherName,
			ops::ObjectName_T topicName,
			uint64_t AckCounter) = 0;
		virtual void onAckNakMessage(CTransport* sender, TAckNakMessage& ackNak) = 0;

		virtual void onCommandMessage(CTransport* sender, TCommandMessage& cmd) = 0;
		virtual void onStatusMessage(CTransport* sender, const TStatusMessage& status) = 0;

		virtual void onUdpMcMessage(CTransport* sender, const TUdpMcMessage& udpMc, const char* data) = 0;
	};

	class CTransport : public CWorkItemEx
	{
	public:
		CTransport();
		virtual ~CTransport();

		void SetListener(CTransportListener* user) {
			m_user = user;
		}

		virtual bool writeOpsMessage(ops::OPSObject* mess,
			ops::ObjectName_T publisherName,
			ops::ObjectName_T topicName,
			uint64_t ackCounter) = 0;
		virtual bool writeAckNak(uint64_t ackCounter, uint32_t errorCode) = 0;
		
		// The .Head field in cmd and status will be overwritten
		virtual bool writeCommand(TCommandMessage& cmd) = 0;
		virtual bool writeStatus(TStatusMessage& status) = 0;

		// The .Head field in UdpMc will be overwritten
		virtual bool writeUdpMcMessage(TUdpMcMessage& udpMc, const char* data) = 0;

		virtual void disconnect() = 0;

	protected:
		CTransportListener* m_user;
	};

}
