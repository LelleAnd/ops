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

#include "OPSTypeDefs.h"
#include "ReceiveDataHandlerFactory.h"
#include "opsidls/OPSConstants.h"
#include "ReceiveDataHandler.h"
#include "MCReceiveDataHandler.h"
#include "UDPReceiveDataHandler.h"
#include "TCPReceiveDataHandler.h"
#include "Participant.h"
#include "BasicError.h"
#include "NetworkSupport.h"

namespace ops
{
    using namespace opsidls;

    ReceiveDataHandlerFactory::ReceiveDataHandlerFactory()
    {
    }

	InternalKey_T ReceiveDataHandlerFactory::makeKey(Topic& top, IOService* const ioServ)
	{
		// Since topics can use the same port for transports multicast & tcp, or 
		// use transport udp which in most cases use a single ReceiveDataHandler, 
		// we need to return the same ReceiveDataHandler in these cases.
		// Make a key with the transport info that uniquely defines the receiver.
		InternalKey_T key(top.getTransport());
		if (top.getTransport() == Topic::TRANSPORT_UDP) {
			if (!isMyNodeAddress(top.getDomainAddress(), ioServ)) {
				return key;
			}
		} 
		key += "::";
		key += top.getDomainAddress();
		key += "::";
		key += NumberToString(top.getPort());
		return key;
	}

    std::shared_ptr<ReceiveDataHandler> ReceiveDataHandlerFactory::getReceiveDataHandler(Topic& top, Participant& participant)
    {
		// Make a key with the transport info that uniquely defines the receiver.
		InternalKey_T key = makeKey(top, participant.getIOService());

        const SafeLock lock(&garbageLock);
        if (receiveDataHandlerInstances.find(key) != receiveDataHandlerInstances.end())
        {
            // If we already have a ReceiveDataHandler for this topic, use it.
            std::shared_ptr<ReceiveDataHandler> rdh = receiveDataHandlerInstances[key];

            // Check if any of the topics have a sample size larger than MAX_SEGMENT_SIZE
            // This will lead to a problem when using the same port or using UDP, if samples becomes > MAX_SEGMENT_SIZE
			if ((rdh->getSampleMaxSize() > OPSConstants::PACKET_MAX_SIZE) || (top.getSampleMaxSize() > OPSConstants::PACKET_MAX_SIZE))
            {
				ErrorMessage_T msg;
				if (top.getTransport() == Topic::TRANSPORT_UDP) {
					msg = "Warning: UDP Transport is used with Topics with 'sampleMaxSize' > ";
					msg += NumberToString(OPSConstants::PACKET_MAX_SIZE);
				} else {
					msg += "Warning: Same port (";
					msg += NumberToString(top.getPort());
					msg += ") is used with Topics with 'sampleMaxSize' > ";
					msg += NumberToString(OPSConstants::PACKET_MAX_SIZE);
				}
				BasicError err("ReceiveDataHandlerFactory", "getReceiveDataHandler", msg);
				participant.reportError(&err);
            }
            return rdh;
        }
        else if (top.getTransport() == Topic::TRANSPORT_MC)
        {
            std::shared_ptr<ReceiveDataHandler> rdh = std::make_shared<MCReceiveDataHandler>(top, participant);
            receiveDataHandlerInstances[key] = rdh;
            return rdh;
        }
		else if (top.getTransport() == Topic::TRANSPORT_TCP)
		{
            std::shared_ptr<ReceiveDataHandler> rdh = std::make_shared<TCPReceiveDataHandler>(top, participant);
            receiveDataHandlerInstances[key] = rdh;
            return rdh;
        }
		else if (top.getTransport() == Topic::TRANSPORT_UDP)
        {
            bool commonReceiver = (key == Topic::TRANSPORT_UDP);
            std::shared_ptr<ReceiveDataHandler> rdh = std::make_shared<UDPReceiveDataHandler>(top, participant, commonReceiver);
			receiveDataHandlerInstances[key] = rdh;
            return rdh;
        }
        else //For now we can not handle more transports
        {
			//Signal an error by returning nullptr.
			ErrorMessage_T msg = "Unknown transport for Topic: ";
			msg += top.getName();
			BasicError err("ReceiveDataHandlerFactory", "getReceiveDataHandler", msg);
			participant.reportError(&err);
            return nullptr;
        }
    }

    void ReceiveDataHandlerFactory::releaseReceiveDataHandler(Topic& top, Participant& participant)
    {
		// Make a key with the transport info that uniquely defines the receiver.
		InternalKey_T key = makeKey(top, participant.getIOService());

		const SafeLock lock(&garbageLock);
        if (receiveDataHandlerInstances.find(key) != receiveDataHandlerInstances.end())
        {
            const std::shared_ptr<ReceiveDataHandler> rdh = receiveDataHandlerInstances[key];
            if (rdh->Notifier<OPSMessage*>::getNrOfListeners() == 0)
            {
                //Time to mark this receiveDataHandler as garbage.
                receiveDataHandlerInstances.erase(receiveDataHandlerInstances.find(key));

                rdh->clear();

				if (key == Topic::TRANSPORT_UDP) {
					participant.setUdpTransportInfo("", 0);
				}

                garbageReceiveDataHandlers.push_back(rdh);
            }
        }
    }

    void ReceiveDataHandlerFactory::cleanUpReceiveDataHandlers()
    {
        const SafeLock lock(&garbageLock);
        
        for (int i = (int)garbageReceiveDataHandlers.size() - 1; i >= 0; i--)
        {
            if ((garbageReceiveDataHandlers[i]->numReservedMessages() == 0) &&
                (garbageReceiveDataHandlers[i]->asyncFinished()))
            {
                garbageReceiveDataHandlers[i].reset();
                const auto iter = garbageReceiveDataHandlers.begin() + i;
                garbageReceiveDataHandlers.erase(iter);
            }
        }
    }

	bool ReceiveDataHandlerFactory::cleanUpDone()
	{
        const SafeLock lock(&garbageLock);
		return garbageReceiveDataHandlers.size() == 0;
	}

    bool ReceiveDataHandlerFactory::dataAvailable()
    {
        const SafeLock lock(&garbageLock);
        for (auto& rdh : receiveDataHandlerInstances) {
            if (rdh.second->dataAvailable()) return true;
        }
        return false;
    }

    ReceiveDataHandlerFactory::~ReceiveDataHandlerFactory()
    {
    }

}
