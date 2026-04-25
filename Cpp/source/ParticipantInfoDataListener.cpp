/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2026 Lennart Andersson.
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

#include "ParticipantInfoDataListener.h"

#include "DataNotifier.h"
#include "ErrorService.h"
#include "BasicError.h"
#include "NetworkSupport.h"
#include "ParticipantInfoData.h"
#include "SendDataHandler.h"
#include "ReceiveDataHandler.h"
#include "Subscriber.h"

namespace ops
{

	ParticipantInfoDataListener::ParticipantInfoDataListener(ErrorService& errorSvc, const ObjectName_T& domId):
		errorService(errorSvc),
		domainId(domId)
    {
    }

	void ParticipantInfoDataListener::setup(const Topic& top)
	{
		partInfoTopic = top;
	}

    void ParticipantInfoDataListener::handle(ParticipantInfoData* const partInfo)
    {
        const SafeLock lock(mutex);
        if (partInfo->mc_udp_port != 0) {
            for (const auto& x : partInfo->subscribeTopics) {
                if (x.transport == Topic::TRANSPORT_UDP) {
                    // Lookup topic in map. If found call handler
                    const auto result = sendDataHandlers.find(x.name);
                    if (result != sendDataHandlers.end()) {
                        result->second.get()->addSink(x.name, partInfo->ip, partInfo->mc_udp_port);
                    }
                }
            }
        }
        for (const auto& x : partInfo->publishTopics) {
            if (x.transport == Topic::TRANSPORT_TCP) {
                // Lookup topic in map. If found call handler
                const auto result = rcvDataHandlers.find(x.name);
                if (result != rcvDataHandlers.end()) {
                    result->second.get()->AddReceiveChannel(x.name, x.address, x.port);
                }
            }
        }
    }

	void ParticipantInfoDataListener::cleanup()
	{
		const SafeLock lock(mutex);
		// We can't remove the Subscriber in the destructor, since the delete of the Subscriber
		// requires objects that already has been deleted when the participant delete us
		// (for the case when user has subscribers left when deleting the participant. 
		// Normally we have matching calls to connect & disconnect, so subscriber is already deleted)
		removeSubscriber();
	}

	bool ParticipantInfoDataListener::setupSubscriber()
	{
		// Check that user hasn't disabled the meta data
		if (partInfoTopic.getPort() == 0) {
			return false;
		}

		partInfoSub = std::make_unique<Subscriber>(partInfoTopic);
		partInfoSub->addDataListener([this](DataNotifier* const )
			{
				ParticipantInfoData* const partInfo = dynamic_cast<ParticipantInfoData*> (partInfoSub->getMessage()->getData());
				if (partInfo != nullptr) {
					// Is it on our domain?
					if (partInfo->domain == domainId) {
						handle(partInfo);
					}
				}
				else {
					BasicError err("ParticipantInfoDataListener", "onNewData", "Illegal data detected", BasicError::ILLEGAL_DATA);
					errorService.report(&err);
				}
			}
		);
		partInfoSub->start();

		return true;
	}

	void ParticipantInfoDataListener::removeSubscriber()
	{
		partInfoSub.reset();
	}

	void ParticipantInfoDataListener::connectSDH(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
	{
        const ObjectName_T key = top.getName();
        const SafeLock lock(mutex);
		if (partInfoSub.get() == nullptr) {
			if (!setupSubscriber()) {
				if (!isValidNodeAddress(top.getDomainAddress())) {
					// Generate an error message if we come here with domain->getMetaDataMcPort() == 0,
					// it means that we have UDP topics that require meta data but user has disabled it.
					ErrorMessage_T msg("Send topic '");
					msg += key;
					msg += "' won't work since Meta Data disabled in config-file";
					BasicError err("ParticipantInfoDataListener", "connectSDH", msg, BasicError::CONFIG_ERROR);
					errorService.report(&err);
				}
			}
		}

        // Add to map if not already there
        if (sendDataHandlers.find(key) != sendDataHandlers.end()) {
            const std::shared_ptr<SendDataHandler> sdh = sendDataHandlers[key];
            if (sdh.get() != handler.get()) {
                ErrorMessage_T msg("Send topic '");
                msg += key;
                msg += "' already registered for another SDH";
                BasicError err("ParticipantInfoDataListener", "connectSDH", msg, BasicError::ALREADY_INUSE);
				errorService.report(&err);
                return;
            }
        } else {
            sendDataHandlers[key] = handler;
        }
    }

	void ParticipantInfoDataListener::disconnectSDH(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
	{
		const SafeLock lock(mutex);

        // Remove from map
        const ObjectName_T key = top.getName();
        const auto result = sendDataHandlers.find(key);
        if (result != sendDataHandlers.end()) {
            const std::shared_ptr<SendDataHandler> sdh = sendDataHandlers[key];
            if (sdh.get() != handler.get()) {
                ErrorMessage_T msg("Send topic '");
                msg += key;
                msg += "' atempt to remove topic for another SDH";
                BasicError err("ParticipantInfoDataListener", "disconnectSDH", msg, BasicError::NO_MATCH);
				errorService.report(&err);
                return;
            }
            sendDataHandlers.erase(result);

            if (sendDataHandlers.size() == 0) {
                if (rcvDataHandlers.size() == 0) {
                    removeSubscriber();
                }
            }
        }
    }

	void ParticipantInfoDataListener::connectRDH(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> const handler)
	{
		const SafeLock lock(mutex);
		if (partInfoSub.get() == nullptr) {
			if (!setupSubscriber()) {
				// Generate an error message if we come here with domain->getMetaDataMcPort() == 0,
				// it means that we have TCP topics that require meta data but user has disabled it.
				ErrorMessage_T msg("Receive topic '");
				msg += top;
				msg += "' won't work since Meta Data disabled in config-file";
				BasicError err("ParticipantInfoDataListener", "connectRDH", msg, BasicError::CONFIG_ERROR);
				errorService.report(&err);
				return;
			}
		}
		
		// Add to map if not already there
		if (rcvDataHandlers.find(top) != rcvDataHandlers.end()) {
            const std::shared_ptr<ReceiveDataHandler> rdh = rcvDataHandlers[top];
			if (rdh.get() != handler.get()) {
				ErrorMessage_T msg("Receive topic '");
				msg += top;
				msg += "' already registered for another RDH";
				BasicError err("ParticipantInfoDataListener", "connectRDH", msg, BasicError::ALREADY_INUSE);
				errorService.report(&err);
				return;
			}
		} else {
			rcvDataHandlers[top] = handler;
		}
	}

	void ParticipantInfoDataListener::disconnectRDH(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> const handler)
	{
		const SafeLock lock(mutex);

		// Remove from map
		const auto result = rcvDataHandlers.find(top);
		if (result != rcvDataHandlers.end()) {
            const std::shared_ptr<ReceiveDataHandler> rdh = rcvDataHandlers[top];
			if (rdh.get() != handler.get()) {
				ErrorMessage_T msg("Receive topic '");
				msg += top;
				msg += "' atempt to remove topic for another RDH";
				BasicError err("ParticipantInfoDataListener", "disconnectRDH", msg, BasicError::NO_MATCH);
				errorService.report(&err);
				return;
			}
			rcvDataHandlers.erase(result);

			if (rcvDataHandlers.size() == 0) {
                if (sendDataHandlers.size() == 0) {
                    removeSubscriber();
				}
			}
		}
	}

}
