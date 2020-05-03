/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020 Lennart Andersson.
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

#include <sstream>

#include "OPSTypeDefs.h"
#include "SendDataHandlerFactory.h"
#include "SendDataHandler.h"
#include "Participant.h"
#include "McSendDataHandler.h"
#include "McUdpSendDataHandler.h"
#include "TCPSendDataHandler.h"
#include "Domain.h"
#include "NetworkSupport.h"

namespace ops
{

	SendDataHandlerFactory::SendDataHandlerFactory()
	{
		// There is only one McUdpSendDataHandler for each participant
	}

	SendDataHandlerFactory::~SendDataHandlerFactory()
	{
		// All SendDataHandlers should have been released before this instance is deleted,
		// so it should be OK to delete all objects in the map.
		for (std::map<InternalKey_T, SendDataHandler*>::iterator it = sendDataHandlers.begin();
			it != sendDataHandlers.end(); ++it)
		{
			delete it->second;
		}
	}
	
	InternalKey_T getKey(Topic& top, Address_T& localIf)
	{
		// We need to store SendDataHandlers with more than just the name as key.
		// Since topics can use the same port, we need to return the same SendDataHandler.
		// Make a key with the transport info that uniquely defines the receiver.
		InternalKey_T key = top.getTransport();
		key += "::";
        if (top.getTransport() == Topic::TRANSPORT_UDP) {
            key += localIf;
            key += "::";
        }
		if ((top.getTransport() == Topic::TRANSPORT_TCP) && (top.getPort() == 0)) {
			// We add the channel name so different channels get different TCP Servers
			key += top.getChannelId();
			key += "::";
		}
		key += top.getDomainAddress();
        if (top.getTransport() != Topic::TRANSPORT_UDP) {
            key += "::";
            key += NumberToString(top.getPort());
        }
		return key;
	}

    void SendDataHandlerFactory::PostSetup(Topic& top, Participant& participant, SendDataHandler* sdh)
    {
        if (top.getTransport() == Topic::TRANSPORT_UDP) {
            // If topic specifies a valid node address, add that as a static destination address for topic
            if (isValidNodeAddress(top.getDomainAddress())) {
                ObjectName_T topName = top.getName();
                Address_T destAddress = top.getDomainAddress();
                int destPort = top.getPort();
                ((McUdpSendDataHandler*)sdh)->addSink(topName, destAddress, destPort, true);
            } else {
                // Setup a listener on the participant info data published by participants on our domain.
                // We use the information for topics with UDP as transport, to know the destination for UDP sends
                // ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
                // Note: need to call connectUdp/disconnectUdp equal number of times
                participant.partInfoListener->connectUdp(top, sdh);
            }
        }
    }

	SendDataHandler* SendDataHandlerFactory::getSendDataHandler(Topic& top, Participant& participant)
	{
        Address_T localIf = doSubnetTranslation(top.getLocalInterface(), participant.getIOService());
        InternalKey_T key = getKey(top, localIf);

		SafeLock lock(&mutex);

        // Check if a suitable SendDataHandler already exist
        if (sendDataHandlers.find(key) != sendDataHandlers.end()) {
            SendDataHandler* sdh = sendDataHandlers[key];
            PostSetup(top, participant, sdh);
            return sdh;
        }

		int ttl = top.getTimeToLive();
        SendDataHandler* sdh = nullptr;

		if (top.getTransport() == Topic::TRANSPORT_MC)
		{
			sdh = new McSendDataHandler(participant.getIOService(), top, localIf, ttl);
		}
		else if (top.getTransport() == Topic::TRANSPORT_UDP)
		{
            // We have only one sender for all topics on the same interface, so use the domain value for buffer size
            sdh = new McUdpSendDataHandler(participant.getIOService(), localIf,
                                           participant.getDomain()->getOutSocketBufferSize());
            PostSetup(top, participant, sdh);
		}
		else if (top.getTransport() == Topic::TRANSPORT_TCP)
		{
			sdh = new TCPSendDataHandler(participant.getIOService(), top);
		}
        if (sdh != nullptr) {
            sendDataHandlers[key] = sdh;
        }
        return sdh;
    }

	void SendDataHandlerFactory::releaseSendDataHandler(Topic& top, Participant& participant)
	{
        Address_T localIf = doSubnetTranslation(top.getLocalInterface(), participant.getIOService());
        InternalKey_T key = getKey(top, localIf);

		SafeLock lock(&mutex);
		
        if (top.getTransport() == Topic::TRANSPORT_UDP) {
            if (sendDataHandlers.find(key) != sendDataHandlers.end()) {
                SendDataHandler* sdh = sendDataHandlers[key];
                if (!isValidNodeAddress(top.getDomainAddress())) {
                    participant.partInfoListener->disconnectUdp(top, sdh);
                }
            }
        }
    }

}
