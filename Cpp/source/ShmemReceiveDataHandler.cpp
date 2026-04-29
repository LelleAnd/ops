/**
* 
* Copyright (C) 2025-2026 Lennart Andersson.
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

#include "ShmemReceiveDataHandler.h"
#include "ShmemReceiver.h"

namespace ops
{

	ShmemReceiveDataHandler::ShmemReceiveDataHandler(const Topic& top, Participant& part, const InternalKey_T& name) :
		ReceiveDataHandler(part, nullptr), topic(top), baseName(name)
	{
#ifndef OPS_NO_SHMEM_TRANSPORT
		// Handle SHMEM channels specified with or without an extra process ID as address
		if ((top.getTransport() == Topic::TRANSPORT_SHMEM) && (top.getDomainAddress() == "")) {
			// No process ID as address, so create RDH directly, no meta-data used
			std::unique_ptr<ReceiveDataChannelBase> rdc = std::make_unique<ReceiveDataChannel>(top, part,
				std::make_unique<ShmemReceiver>(name, ""));
			rdc->connect(this);
			sampleMaxSize = rdc->getSampleMaxSize();
			rdcs.push_back(std::move(rdc));
			usingPartInfo = false;
		}
		else {
			// Got a process ID as address, so we are using meta-data to get actual name
			// Since we use the same "topic" parameters for all created RDC's, we can set the sampleMaxSize here
			sampleMaxSize = ReceiveDataChannel::calcSampleMaxSize(top);
		}
#endif
	}

#ifndef OPS_NO_SHMEM_TRANSPORT
	void ShmemReceiveDataHandler::AddReceiveChannel(const ObjectName_T& topicName, const Address_T& ip, int const port)
	{
		UNUSED(topicName)
		UNUSED(port)

		OPS_PIFO_TRACE("SHMEM, Partinfo: name: " << topicName << ", address: " << ip << "\n");

		// We need to check if a new publisher has emerged that we need to connect to
		InternalKey_T key(baseName);
		key += "-";
		key += ip;

		// Look for it in rdc, if not there, create one
		bool found = false;
		for (auto const& rdc : rdcs) {
			if (rdc->key == key) {
				found = true;
				break;
			}
		}
		if (!found) {
			OPS_PIFO_TRACE("SHMEM, Partinfo: CREATED name: " << key << "\n");
			topic.setDomainAddress(ip);
			try {
				std::unique_ptr<ReceiveDataChannelBase> rdc = std::make_unique<ReceiveDataChannel>(topic, participant,
					std::make_unique<ShmemReceiver>(key, ip));
				rdc->key = key;
				rdc->connect(this);

				const SafeLock lock(messageLock);
				if (Notifier<OPSMessage*>::getNrOfListeners() > 0) { rdc->start(); }
				rdcs.push_back(std::move(rdc));

			}
			catch (const std::exception& e) {
				ExceptionMessage_T msg("Unknown exception: ");
				msg += e.what();
				ops::BasicError err("ShmemReceiveDataHandler", "AddReceiveChannel", msg);
				Participant::reportStaticError(&err);
			}
		}
	}

	void ShmemReceiveDataHandler::topicUsage(const Topic& top, bool const used)
	{
		if (usingPartInfo) {
			const SafeLock lock(topicsLock);
			int32_t count = topics.update(top.getName(), used);

			// Register topic with participant info data handler/listener to get callbacks to handler above
			if (used && (count == 1)) {
				participant.metaDataHnd.registerReceiveTopic(top, shared_from_this());

			}
			else if (count == 0) {
				participant.metaDataHnd.unregisterReceiveTopic(top, shared_from_this());
			}
		}
	}
#endif
	
}
