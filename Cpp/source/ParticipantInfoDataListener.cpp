/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2021 Lennart Andersson.
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
#include "OPSTypeDefs.h"
#include "ParticipantInfoDataListener.h"
#include "McUdpSendDataHandler.h"
#include "TCPReceiveDataHandler.h"
#include "Participant.h"
#include "BasicError.h"
#include "NetworkSupport.h"

namespace ops
{

	ParticipantInfoDataListener::ParticipantInfoDataListener(Participant& part):
		participant(part)
    {
    }

    void ParticipantInfoDataListener::handle(ParticipantInfoData* partInfo)
    {
        const SafeLock lock(&mutex);
        if (partInfo->mc_udp_port != 0) {
            for (auto x : partInfo->subscribeTopics) {
                if ((x.transport == Topic::TRANSPORT_UDP) && participant.hasPublisherOn(x.name)) {
                    // Lookup topic in map. If found call handler
                    const auto result = sendDataHandlers.find(x.name);
                    if (result != sendDataHandlers.end()) {
                        dynamic_cast<McUdpSendDataHandler*>(result->second.get())->addSink(x.name, partInfo->ip, partInfo->mc_udp_port);
                    }
                }
            }
        }
        for (const auto& x : partInfo->publishTopics) {
            if ((x.transport == Topic::TRANSPORT_TCP) && participant.hasSubscriberOn(x.name)) {
                // Lookup topic in map. If found call handler
                const auto result = rcvDataHandlers.find(x.name);
                if (result != rcvDataHandlers.end()) {
                    dynamic_cast<TCPReceiveDataHandler*>(result->second.get())->AddReceiveChannel(x.name, x.address, x.port);
                }
            }
        }
    }

	///Called when a new message is received. Running on the boost thread.
    void ParticipantInfoDataListener::onNewData(DataNotifier* const notifier)
    {
        Subscriber* const sub = dynamic_cast<Subscriber*> (notifier);
        if (sub != nullptr) {
            ParticipantInfoData* const partInfo = dynamic_cast<ParticipantInfoData*> (sub->getMessage()->getData());
            if (partInfo != nullptr) {
				// Is it on our domain?
				if (partInfo->domain == participant.domainID) {
                    handle(partInfo);
				}
            } else {
				BasicError err("ParticipantInfoDataListener", "onNewData", "Data could not be cast as expected.");
                participant.reportError(&err);
            }
        } else {
			BasicError err("ParticipantInfoDataListener", "onNewData", "Subscriber could not be cast as expected.");
            participant.reportError(&err);
        }
    }

    ParticipantInfoDataListener::~ParticipantInfoDataListener()
    {
	}

	void ParticipantInfoDataListener::prepareForDelete()
	{
		const SafeLock lock(&mutex);
		// We can't remove the Subscriber in the destructor, since the delete of the Subscriber
		// requires objects that already has been deleted when the participant delete us
		// (for the case when user has subscribers left when deleting the participant. 
		// Normally we have matching calls to connect & disconnect, so subscriber is already deleted)
		removeSubscriber();
	}

	bool ParticipantInfoDataListener::setupSubscriber()
	{
		// Check that user hasn't disabled the meta data
		if (participant.getDomain()->getMetaDataMcPort() == 0) {
			return false;
		}

		partInfoSub = std::unique_ptr<Subscriber>(new Subscriber(participant.createParticipantInfoTopic()));
		partInfoSub->addDataListener(this);
		partInfoSub->start();

		return true;
	}

	void ParticipantInfoDataListener::removeSubscriber()
	{
		partInfoSub.reset();
	}

	void ParticipantInfoDataListener::connectUdp(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
	{
        const ObjectName_T key = top.getName();
        const SafeLock lock(&mutex);
		if (partInfoSub.get() == nullptr) {
			if (!setupSubscriber()) {
				if (!isValidNodeAddress(top.getDomainAddress())) {
					// Generate an error message if we come here with domain->getMetaDataMcPort() == 0,
					// it means that we have UDP topics that require meta data but user has disabled it.
					ErrorMessage_T msg("UDP topic '");
					msg += key;
					msg += "' won't work since Meta Data disabled in config-file";
					BasicError err("ParticipantInfoDataListener", "connectUdp", msg);
					participant.reportError(&err);
				}
			}
		}

        // Add to map if not already there
        if (sendDataHandlers.find(key) != sendDataHandlers.end()) {
            const std::shared_ptr<SendDataHandler> sdh = sendDataHandlers[key];
            if (sdh.get() != handler.get()) {
                ErrorMessage_T msg("UDP topic '");
                msg += key;
                msg += "' already registered for another SDH";
                BasicError err("ParticipantInfoDataListener", "connectUdp", msg);
                participant.reportError(&err);
                return;
            }
        } else {
            sendDataHandlers[key] = handler;
        }
    }

	void ParticipantInfoDataListener::disconnectUdp(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
	{
		const SafeLock lock(&mutex);

        // Remove from map
        const ObjectName_T key = top.getName();
        const auto result = sendDataHandlers.find(key);
        if (result != sendDataHandlers.end()) {
            const std::shared_ptr<SendDataHandler> sdh = sendDataHandlers[key];
            if (sdh.get() != handler.get()) {
                ErrorMessage_T msg("UDP topic '");
                msg += key;
                msg += "' atempt to remove topic for another SDH";
                BasicError err("ParticipantInfoDataListener", "disconnectUdp", msg);
                participant.reportError(&err);
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

	void ParticipantInfoDataListener::connectTcp(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> const handler)
	{
		const SafeLock lock(&mutex);
		if (partInfoSub.get() == nullptr) {
			if (!setupSubscriber()) {
				// Generate an error message if we come here with domain->getMetaDataMcPort() == 0,
				// it means that we have TCP topics that require meta data but user has disabled it.
				ErrorMessage_T msg("TCP topic '");
				msg += top;
				msg += "' won't work since Meta Data disabled in config-file";
				BasicError err("ParticipantInfoDataListener", "connectTcp", msg);
				participant.reportError(&err);
				return;
			}
		}
		
		// Add to map if not already there
		if (rcvDataHandlers.find(top) != rcvDataHandlers.end()) {
            const std::shared_ptr<ReceiveDataHandler> rdh = rcvDataHandlers[top];
			if (rdh.get() != handler.get()) {
				ErrorMessage_T msg("TCP topic '");
				msg += top;
				msg += "' already registered for another RDH";
				BasicError err("ParticipantInfoDataListener", "connectTcp", msg);
				participant.reportError(&err);
				return;
			}
		} else {
			rcvDataHandlers[top] = handler;
		}
	}

	void ParticipantInfoDataListener::disconnectTcp(const ObjectName_T& top, std::shared_ptr<ReceiveDataHandler> const handler)
	{
		const SafeLock lock(&mutex);

		// Remove from map
		const auto result = rcvDataHandlers.find(top);
		if (result != rcvDataHandlers.end()) {
            const std::shared_ptr<ReceiveDataHandler> rdh = rcvDataHandlers[top];
			if (rdh.get() != handler.get()) {
				ErrorMessage_T msg("TCP topic '");
				msg += top;
				msg += "' atempt to remove topic for another RDH";
				BasicError err("ParticipantInfoDataListener", "disconnectTcp", msg);
				participant.reportError(&err);
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
