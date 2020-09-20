/**
* 
* Copyright (C) 2016-2020 Lennart Andersson.
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
#include "Transport.h"

namespace ops
{
    Transport::Transport()
        : channelID("")
    {
        appendType(TypeId_T("Transport"));
    }

    void Transport::serialize(ArchiverInOut* const archiver)
    {
        OPSObject::serialize(archiver);
        if (idlVersionMask != 0) {
            archiver->inout("Transport_version", Transport_version);
        } else {
            Transport_version = 0;
        }
        archiver->inout("channelID", channelID);
        archiver->inout("topics", topics);
    }

}
