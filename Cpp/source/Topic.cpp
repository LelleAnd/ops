/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2025 Lennart Andersson.
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

#include "Topic.h"
#include "opsidls/OPSConstants.h"
#include "XMLArchiverIn.h"
#include "ConfigException.h"
#include "opsidls/SendAckPatternData.h"

namespace ops
{
    using namespace opsidls;

    static_assert(OPSConstants::PACKET_MAX_SIZE - OPSConstants::SEGMENT_HEADER_SIZE == OPSConstants::USABLE_SEGMENT_SIZE, "Sizes don't compute");

    Topic::Topic() :
        participantID(OPSConstants::DEFAULT_PARTICIPANT_ID()),
        sampleMaxSize(OPSConstants::USABLE_SEGMENT_SIZE)
    {
        appendType(TypeId_T("Topic"));
    }

    Topic::Topic(const ObjectName_T namee, int const portt, const TypeId_T typeIDd, const Address_T domainAddresss) :
		name(namee), 
		port(portt), 
		typeID(typeIDd), 
		domainAddress(domainAddresss),
		participantID(OPSConstants::DEFAULT_PARTICIPANT_ID()),
		sampleMaxSize(OPSConstants::USABLE_SEGMENT_SIZE)
	{
		appendType(TypeId_T("Topic"));
	}

    Topic::Topic(const ObjectName_T namee, const TypeId_T typeIDd, const int sampleMaxSizee, const bool useAckk, const Topic& base) :
        name(namee),
        port(base.port),
        timeToLive(base.timeToLive),
        typeID(typeIDd),
        domainAddress(base.domainAddress),
        localInterface(base.localInterface),
        participantID(base.participantID),
        domainID(base.domainID),
        sampleMaxSize(sampleMaxSizee),
        transport(base.transport),
        outSocketBufferSize(base.outSocketBufferSize),
        inSocketBufferSize(base.inSocketBufferSize),
        useAck(useAckk),
        optNonVirt(base.optNonVirt),
        heartbeatPeriod(base.heartbeatPeriod),
        heartbeatTimeout(base.heartbeatTimeout),
        resendNum(base.resendNum),
        resendTimeMs(base.resendTimeMs),
        registerTimeMs(base.registerTimeMs),
        channelID(base.channelID)
    {
        appendType(TypeId_T("Topic"));
    }

    // Create ACK topic based on given topic
    Topic Topic::CreateAckTopic(const Topic& base)
    {
        ObjectName_T ackName(base.getName());
        ackName += "#ack";
        constexpr int ackSampleMaxSize = 1024;
        return Topic(ackName, opsidls::SendAckPatternData::getTypeName(), ackSampleMaxSize, false, base);
    }

	// Returns a newely allocated deep copy/clone of this object.
	Topic* Topic::clone()
	{
		Topic* const ret = new Topic;
		fillClone(ret);
		return ret;
	}

	// Fills the parameter obj with all values from this object.
	void Topic::fillClone(Topic* const obj) const
	{
		if (this == obj) { return; }
		ops::OPSObject::fillClone(obj);
		obj->Topic_version = Topic_version;
		obj->name = name;
		obj->port = port;
		obj->timeToLive = timeToLive;
		obj->typeID = typeID;
		obj->domainAddress = domainAddress;
		obj->localInterface = localInterface;
		obj->participantID = participantID;
		obj->domainID = domainID;
		obj->sampleMaxSize = sampleMaxSize;
		obj->transport = transport;
		obj->outSocketBufferSize = outSocketBufferSize;
		obj->inSocketBufferSize = inSocketBufferSize;
		obj->useAck = useAck;
		obj->optNonVirt = optNonVirt;
		obj->heartbeatPeriod = heartbeatPeriod;
		obj->heartbeatTimeout = heartbeatTimeout;
		obj->resendNum = resendNum;
		obj->resendTimeMs = resendTimeMs;
		obj->registerTimeMs = registerTimeMs;
		obj->channelID = channelID;
	}

	void Topic::setParticipantID(ObjectName_T const partID) noexcept
	{
		participantID = partID;
	}

	ObjectName_T Topic::getParticipantID() const noexcept
	{
		return participantID;
	}

	void Topic::setDomainID(ObjectName_T const domID) noexcept
	{
		domainID = domID;
	}
	ObjectName_T Topic::getDomainID() const noexcept
	{
		return domainID;
	}
	ObjectName_T Topic::getName() const noexcept
	{
		return name;
	}
	TypeId_T Topic::getTypeID() const noexcept
	{
		return typeID;
	}
	void Topic::setDomainAddress(Address_T const domainAddr) noexcept
	{
		domainAddress = domainAddr;
	}
	void Topic::setTransport(Transport_T const transp) noexcept
	{
		transport = transp;
	}
	Address_T Topic::getDomainAddress() const noexcept
	{
		return domainAddress;
	}
	void Topic::setLocalInterface(Address_T const localIf) noexcept
	{
		localInterface = localIf;
	}
	Address_T Topic::getLocalInterface() const noexcept
	{
		return localInterface;
	}
	int Topic::getSampleMaxSize() const noexcept
	{
		return sampleMaxSize;
	}
	void Topic::setSampleMaxSize(int const size) noexcept
	{
		sampleMaxSize = size;
	}
	int Topic::getPort() const noexcept
	{
		return port;
	}
	void Topic::setPort(int const pt) noexcept
	{
		this->port = pt;
	}
	void Topic::setTimeToLive(int const ttl) noexcept
	{
		timeToLive = ttl;
	}
	int Topic::getTimeToLive() const noexcept
	{
		return timeToLive;
	}
	Transport_T Topic::getTransport() const noexcept
	{
		return transport;
	}

	int64_t Topic::getOutSocketBufferSize() const noexcept
	{
		return outSocketBufferSize;
	}
	void Topic::setOutSocketBufferSize(int64_t const size) noexcept
	{
		outSocketBufferSize = size;
	}
	int64_t Topic::getInSocketBufferSize() const noexcept
	{
		return inSocketBufferSize;
	}
	void Topic::setInSocketBufferSize(int64_t const size) noexcept
	{
		inSocketBufferSize = size;
	}

	bool Topic::getOptNonVirt() const noexcept
	{
		return optNonVirt;
	}

	int Topic::getHeartbeatPeriod() const noexcept
	{
		return heartbeatPeriod;
	}
	int Topic::getHeartbeatTimeout() const noexcept
	{
		return heartbeatTimeout;
	}

    bool Topic::getUseAck() const noexcept
    {
        return useAck;
    }
    void Topic::setUseAck(const bool value) noexcept
    {
        useAck = value;
    }

    int Topic::getNumResends() const noexcept
    {
        return resendNum;
    }
    int Topic::getResendTimeMs() const noexcept
    {
        return resendTimeMs;
    }
    int Topic::getRegisterTimeMs() const noexcept
    {
        return registerTimeMs;
    }

	ChannelId_T Topic::getChannelId() const noexcept
	{
		return channelID;
	}

	void Topic::serialize(ArchiverInOut* const archiver)
	{
		OPSObject::serialize(archiver);
        if (idlVersionMask != 0) {
            archiver->inout("Topic_version", Topic_version);
            ValidateVersion("Topic", Topic_version, Topic_idlVersion);
        } else {
            Topic_version = 0;
        }
        archiver->inout("name", name);
		archiver->inout("dataType", typeID);
		archiver->inout("port", port);		
		archiver->inout("address", domainAddress);

		archiver->inout("outSocketBufferSize", outSocketBufferSize);
		archiver->inout("inSocketBufferSize", inSocketBufferSize);
	
		archiver->inout("sampleMaxSize", sampleMaxSize);

		archiver->inout("transport", transport);
		if(transport == "")
		{
			transport = TRANSPORT_MC;
		} else if (transport != TRANSPORT_MC && transport != TRANSPORT_TCP && transport != TRANSPORT_UDP &&
                   transport != TRANSPORT_INPROC && transport != TRANSPORT_SHMEM)
		{
			ExceptionMessage_T msg("Illegal transport: '");
			msg += transport;
			msg += "'. Transport for topic must be either 'multicast', 'tcp', 'udp', 'inprocess', 'shmem' or left blank( = multicast)";
			throw ops::ConfigException(msg);
		}

        // To not break binary compatibility we only do this when we know we are
        // reading from an XML-file
        if (dynamic_cast<XMLArchiverIn*>(archiver) != nullptr) {
            archiver->inout("useAck", useAck);
        }
    }

	Transport_T Topic::TRANSPORT_MC = "multicast";
	Transport_T Topic::TRANSPORT_TCP = "tcp";
	Transport_T Topic::TRANSPORT_UDP = "udp";
	Transport_T Topic::TRANSPORT_INPROC = "inprocess";
    Transport_T Topic::TRANSPORT_SHMEM = "shmem";

}
