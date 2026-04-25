/**
*
* Copyright (C) 2026 Lennart Andersson.
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

#include <algorithm>
#ifdef _WIN32
#include <process.h>
#include <winsock.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "MetaDataHandler.h"

#include "ErrorService.h"
#include "Publisher.h"
#include "NetworkSupport.h"
#include "ReceiveDataHandler.h"
#include "SendDataHandler.h"
#include "Subscriber.h"
#include "Topic.h"

namespace ops
{

    MetaDataHandler::MetaDataHandler(ErrorService& errorSvc, const ObjectName_T& partId, const ObjectName_T& domId):
        partInfoListener(errorSvc, domId)
    {
        // Initialize static data in partInfoData (other methods will set some more fields)
        InternalString_T Name = GetHostName();
        Name += " (";
        Name += getPid();
        Name += ")";
        partInfoData.name = Name;
        partInfoData.languageImplementation = "C++";
        partInfoData.id = partId;
        partInfoData.domain = domId;
    }

    InternalString_T MetaDataHandler::getPid()
    {
#ifdef _WIN32
        return NumberToString(_getpid());
#else
        return NumberToString(getpid());
#endif
    }

    InternalString_T MetaDataHandler::getPartInfoName() const
    {
        return partInfoData.name;
    }

    void MetaDataHandler::setUdpTransportInfo(const Address_T& ip, const int port)
    {
        const SafeLock lock(partInfoDataLock);
        partInfoData.ip = ip;
        partInfoData.mc_udp_port = port;
        metaDataChanged = true;
    }

    void MetaDataHandler::addSubTopic(const Topic& top)
    {
        const SafeLock lock(partInfoDataLock);
        // Add topic to partInfoData.subscribeTopics (we store ev. duplicates)
        partInfoData.subscribeTopics.push_back(TopicInfoData(top));
        metaDataChanged = true;
    }

    void MetaDataHandler::removeSubTopic(const Topic& top)
    {
        ObjectName_T topicName{ top.getName() };

        const SafeLock lock(partInfoDataLock);
        // Remove topic from partInfoData.subscribeTopics (we store ev. duplicates)
        auto it = std::find_if(partInfoData.subscribeTopics.begin(), partInfoData.subscribeTopics.end(),
            [&](TopicInfoData const& td) { return td.name == topicName; });
        if (it != partInfoData.subscribeTopics.end()) {
            partInfoData.subscribeTopics.erase(it);
            metaDataChanged = true;
        }
    }

    void MetaDataHandler::addPubTopic(const Topic& top)
    {
        const SafeLock lock(partInfoDataLock);
        // Add topic to partInfoData.publisTopics (we store ev. duplicates)
        partInfoData.publishTopics.push_back(TopicInfoData(top));
        metaDataChanged = true;
    }

    void MetaDataHandler::removePubTopic(const Topic& top)
    {
        ObjectName_T topicName{ top.getName() };

        const SafeLock lock(partInfoDataLock);
        // Remove topic from partInfoData.publishTopics (we store ev. duplicates)
        auto it = std::find_if(partInfoData.publishTopics.begin(), partInfoData.publishTopics.end(),
            [&](TopicInfoData const& td) { return td.name == topicName; });
        if (it != partInfoData.publishTopics.end()) {
            partInfoData.publishTopics.erase(it);
            metaDataChanged = true;
        }
    }

    bool MetaDataHandler::hasPublisherOn(const ObjectName_T& topicName)
    {
        const SafeLock lock(partInfoDataLock);
        // Check if topic exist in partInfoData.publishTopics
        for (auto const& td : partInfoData.publishTopics) {
            if (td.name == topicName) { return true; }
        }
        return false;
    }

    bool MetaDataHandler::hasSubscriberOn(const ObjectName_T& topicName)
    {
        const SafeLock lock(partInfoDataLock);
        // Check if topic exist in partInfoData.subscribeTopics
        for (auto const& td : partInfoData.subscribeTopics) {
            if (td.name == topicName) { return true; }
        }
        return false;
    }

    void MetaDataHandler::setup(const Topic& top)
    {
        partInfoTopic = top;
        partInfoListener.setup(top);
    }

    void MetaDataHandler::publish()
    {
        const SafeLock lock(partInfoDataLock);
        if (partInfoPub == nullptr) {
            partInfoPub = std::make_unique<Publisher>(partInfoTopic);
        }
        if (partInfoPub != nullptr) {
            if (metaDataChanged) {
                // Set key to the pubID with changed data (ie. next write)
                partInfoData.setKey(NumberToString(partInfoPub->getPublicationID()));
            }
            partInfoPub->writeOPSObject(&partInfoData);
            metaDataChanged = false;
        }
    }

    void MetaDataHandler::cleanup()
    {
        const SafeLock lock(partInfoDataLock);
        partInfoPub.reset();

        // Stop the subscriber for partInfoData. This requires ioService to be running.
        // Note that the Subscriber uses the Participant's receiveDataHandlerFactory.
        partInfoListener.cleanup();
    }

    void MetaDataHandler::registerReceiveTopic(const Topic& top, std::shared_ptr<ReceiveDataHandler> const handler)
    {
        partInfoListener.connectRDH(top.getName(), handler);
    }

    void MetaDataHandler::unregisterReceiveTopic(const Topic& top, std::shared_ptr<ReceiveDataHandler> const handler)
    {
        partInfoListener.disconnectRDH(top.getName(), handler);
    }

    void MetaDataHandler::registerSendTopic(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
    {
        partInfoListener.connectSDH(top, handler);
    }

    void MetaDataHandler::unregisterSendTopic(const Topic& top, std::shared_ptr<SendDataHandler> const handler)
    {
        partInfoListener.disconnectSDH(top, handler);
    }

}
