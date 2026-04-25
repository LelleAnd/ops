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

#ifndef ops_MetaDataHandlerH
#define	ops_MetaDataHandlerH

#include <memory>

#include "OPSTypeDefs.h"
#include "Lockable.h"
#include "ParticipantInfoData.h"
#include "ParticipantInfoDataListener.h"
#include "Topic.h"

namespace ops
{
    // Forward declarations
    class ErrorService;
    class Publisher;
    class ReceiveDataHandler;
    class SendDataHandler;

    // Participant helper for handling meta-data distrubution
    class MetaDataHandler
    {
    public:
        static InternalString_T getPid();

        MetaDataHandler(ErrorService& errorSvc, const ObjectName_T& partId, const ObjectName_T& domId);

        // Meta-data topic to publish on and listen to
        void setup(const Topic& top);

        // Cleanup. Need to be called early in Participant's destructor (while factories and ioservice are still working)
        // since publishers and subscribers will be deleted
        void cleanup();

        // Methods for updating meta-data to be published
        void setUdpTransportInfo(const Address_T& ip, const int port);
        void addSubTopic(const Topic& top);
        void removeSubTopic(const Topic& top);
        void addPubTopic(const Topic& top);
        void removePubTopic(const Topic& top);

        // Query methods
        InternalString_T getPartInfoName() const;
        bool hasPublisherOn(const ObjectName_T& topicName);
        bool hasSubscriberOn(const ObjectName_T& topicName);

        // Called to publish current meta-data
        void publish();

        // Methods for requesting updates when meta-data is received
        void registerReceiveTopic(const Topic& top, std::shared_ptr<ReceiveDataHandler> const handler);
        void unregisterReceiveTopic(const Topic& top, std::shared_ptr<ReceiveDataHandler> const handler);
        void registerSendTopic(const Topic& top, std::shared_ptr<SendDataHandler> const handler);
        void unregisterSendTopic(const Topic& top, std::shared_ptr<SendDataHandler> const handler);

    private:
        // The ParticipantInfoData that should be published periodically
        ParticipantInfoData partInfoData;
        Lockable partInfoDataLock;
        bool metaDataChanged{ true };

        // Meta-data topic to listen to (initialized as empty)
        Topic partInfoTopic;

        // Publisher of ParticipantInfoData
        std::unique_ptr<Publisher> partInfoPub;

        // Listener and handler for ParticipantInfoData
        ParticipantInfoDataListener partInfoListener;
    };

}
#endif
