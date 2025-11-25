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

#include "OPSTypeDefs.h"
#include "Channel.h"
#include "XMLArchiverIn.h"
#include "ConfigException.h"

namespace ops
{
    Channel::Channel()
    {
        appendType(TypeId_T("Channel"));
    }

    // Returns a deep copy of this object.
    Channel* Channel::clone()
    {
        Channel* const ret = new Channel;
        fillClone(ret);
        return ret;
    }

    void Channel::fillClone(Channel* const obj) const
    {
        if (this == obj) { return; }
        ops::OPSObject::fillClone(obj);
        obj->Channel_version = Channel_version;
        obj->channelID = channelID;
        obj->linktype = linktype;
        obj->localInterface = localInterface;
        obj->domainAddress = domainAddress;
        obj->timeToLive = timeToLive;
        obj->port = port;
        obj->outSocketBufferSize = outSocketBufferSize;
        obj->inSocketBufferSize = inSocketBufferSize;
        obj->resendNum = resendNum;
        obj->resendTimeMs = resendTimeMs;
        obj->registerTimeMs = registerTimeMs;
        obj->sampleMaxSize = sampleMaxSize;
    }

    void Channel::serialize(ArchiverInOut* const archiver)
    {
        OPSObject::serialize(archiver);
        if (idlVersionMask != 0) {
            archiver->inout("Channel_version", Channel_version);
            ValidateVersion("Channel", Channel_version, Channel_idlVersion);
        } else {
            Channel_version = 0;
        }
        archiver->inout("name", channelID);
        archiver->inout("linktype", linktype);
        archiver->inout("localInterface", localInterface);
        archiver->inout("address", domainAddress);
        archiver->inout("timeToLive", timeToLive);
        archiver->inout("port", port);
        archiver->inout("outSocketBufferSize", outSocketBufferSize);
        archiver->inout("inSocketBufferSize", inSocketBufferSize);
    
        if (linktype == "")
        {
            linktype = LINKTYPE_MC;
        } else if (linktype != LINKTYPE_MC && linktype != LINKTYPE_TCP && linktype != LINKTYPE_UDP &&
                   linktype != LINKTYPE_INPROC && linktype != LINKTYPE_SHMEM)
        {
			ExceptionMessage_T msg("Illegal linktype: '");
			msg += linktype;
			msg += "'. Linktype for Channel must be either 'multicast', 'tcp', 'udp', 'inprocess', 'shmem' or left blank( = multicast)";
			throw ops::ConfigException(msg);
        }

        // To not break binary compatibility we only do this when we know we are
        // reading from an XML-file
        if (dynamic_cast<XMLArchiverIn*>(archiver) != nullptr) {
            archiver->inout("sampleMaxSize", sampleMaxSize);
            archiver->inout("resendNum", resendNum);
            archiver->inout("resendTimeMs", resendTimeMs);
            archiver->inout("registerTimeMs", registerTimeMs);
        }
    }

    void Channel::populateTopic(Topic& top) const
    {
        // If Topic doesn't specify a transport it will default to 'multicast', therefore
        // we can't just check for an empty 'top.getTransport()' to know when to replace value.
        // Therfore, if a topic is listed in a 'Transport/Channel', we assume it shall
        // use the channel values, so replace all values.
        top.setTransport(linktype);
        top.setLocalInterface(localInterface);
        top.setDomainAddress(domainAddress);
        top.setPort(port);
        top.setOutSocketBufferSize(outSocketBufferSize);
        top.setInSocketBufferSize(inSocketBufferSize);
        top.setTimeToLive(timeToLive);
        top.channelID = channelID;
        top.resendNum = resendNum;
        top.resendTimeMs = resendTimeMs;
        top.registerTimeMs = registerTimeMs;

        // sampleMaxSize is only replaced in topics if it is specified for the channel
        if (sampleMaxSize >= 0) {
            top.setSampleMaxSize(sampleMaxSize);
        }
    }

	Transport_T Channel::LINKTYPE_MC = "multicast";
	Transport_T Channel::LINKTYPE_TCP = "tcp";
	Transport_T Channel::LINKTYPE_UDP = "udp";
    Transport_T Channel::LINKTYPE_INPROC = "inprocess";
    Transport_T Channel::LINKTYPE_SHMEM = "shmem";
} // namespace ops
