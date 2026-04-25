/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2026 Lennart Andersson.
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
#ifndef ops_ParticipantInfoDataListener_h
#define	ops_ParticipantInfoDataListener_h

#include <map>

#include "OPSTypeDefs.h"
#include "DataListener.h"
#include "Lockable.h"
#include "Topic.h"

namespace ops
{
	// Forward declarations
	class ErrorService;
	class ParticipantInfoData;
	class ReceiveDataHandler;
	class SendDataHandler;
	class Subscriber;

	// Listener for Participant meta-data
	class ParticipantInfoDataListener
	{
	public:
		ParticipantInfoDataListener(ErrorService& errorSvc, const ObjectName_T& domId);

		// Define topic to listen to
		void setup(const Topic& top);

		void cleanup();

		void connectSDH(const Topic& top, std::shared_ptr<SendDataHandler> handler);
		void disconnectSDH(const Topic& top, std::shared_ptr<SendDataHandler> handler);

		void connectRDH(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> handler);
		void disconnectRDH(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> handler);

	private:
		ErrorService& errorService;
		ObjectName_T domainId;

		// Meta-data topic to listen to (initialized as empty)
		Topic partInfoTopic;

		Lockable mutex;
		std::unique_ptr<Subscriber> partInfoSub;

        std::map<ObjectName_T, std::shared_ptr<SendDataHandler>> sendDataHandlers;
		std::map<ObjectName_T, std::shared_ptr<ReceiveDataHandler>> rcvDataHandlers;

		bool setupSubscriber();
		void removeSubscriber();

        void handle(ParticipantInfoData* partInfo);
	};
}
#endif
