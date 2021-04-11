/**
* 
* Copyright (C) 2016-2021 Lennart Andersson.
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

#ifndef ops_transport_h
#define ops_transport_h

#include "OPSTypeDefs.h"
#include "OPSObject.h"

namespace ops
{
    class Transport : public OPSObject
    {
    public:
        char Transport_version = Transport_idlVersion;

        static const char Transport_idlVersion = 0;
        ChannelId_T channelID;
        std::vector<ObjectName_T> topics;

        Transport();
        Transport(const Transport& other) = default;
        Transport(Transport&& other) = default;
        Transport& operator= (const Transport& other) = default;
        Transport& operator= (Transport&& other) = default;
        virtual ~Transport() = default;

        // Returns a newely allocated deep copy/clone of this object.
        virtual Transport* clone() override;

        // Fills the parameter obj with all values from this object.
        void fillClone(Transport* obj) const;

        void serialize(ArchiverInOut* archiver) override;
    };
}
#endif
