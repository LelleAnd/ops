/**
* 
* Copyright (C) 2016-2025 Lennart Andersson.
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

#ifndef ops_channel_h
#define ops_channel_h

#include "OPSTypeDefs.h"
#include "Topic.h"
#include "OPSObject.h"

namespace ops
{
    class Channel : public OPSObject
    {
    public:
        static constexpr uint8_t Channel_idlVersion = 0;
        uint8_t Channel_version = Channel_idlVersion;

        ChannelId_T channelID;
        Transport_T linktype;
        Address_T localInterface;     // If multicast, this specifies interface to use
        Address_T domainAddress;
        int timeToLive{ -1 };         // if multicast, this specifies the ttl parameter
        int port{ 0 };
        int64_t outSocketBufferSize{ -1 };
        int64_t inSocketBufferSize{ -1 };
        int resendNum{ -1 };
        int resendTimeMs{ -1 };
        int registerTimeMs{ -1 };
        int sampleMaxSize{ -1 };

        Channel();
        Channel(const Channel& other) = default;
        Channel(Channel&& other)= default;
        Channel& operator= (const Channel& other) = default;
        Channel& operator= (Channel&& other) = default;
        virtual ~Channel() = default;

        // Returns a newely allocated deep copy/clone of this object.
        virtual Channel* clone() override;

        // Fills the parameter obj with all values from this object.
        void fillClone(Channel* obj) const;

        void serialize(ArchiverInOut* archiver) override;

        void populateTopic(Topic& top) const;

        static Transport_T LINKTYPE_MC;
        static Transport_T LINKTYPE_TCP;
        static Transport_T LINKTYPE_UDP;
        static Transport_T LINKTYPE_INPROC;
    };
}
#endif
