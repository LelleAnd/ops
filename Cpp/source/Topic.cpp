/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2020 Lennart Andersson.
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
#include "ConfigException.h"

namespace ops
{
    using namespace opsidls;

	Topic::Topic(ObjectName_T namee, int const portt, TypeId_T typeIDd, Address_T domainAddresss) :
		name(namee), 
		port(portt), 
		typeID(typeIDd), 
		domainAddress(domainAddresss),
		participantID(OPSConstants::DEFAULT_PARTICIPANT_ID()),
		sampleMaxSize(OPSConstants::PACKET_MAX_SIZE)
	{
		appendType(TypeId_T("Topic"));
	}
	Topic::Topic() :
		participantID(OPSConstants::DEFAULT_PARTICIPANT_ID()),
		sampleMaxSize(OPSConstants::PACKET_MAX_SIZE)
	{
		appendType(TypeId_T("Topic"));
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
	void Topic::setPort(int const port) noexcept
	{
		this->port = port;
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
		}
		else if (transport != TRANSPORT_MC && transport != TRANSPORT_TCP && transport != TRANSPORT_UDP)
		{
			ExceptionMessage_T msg("Illegal transport: '");
			msg += transport;
			msg += "'. Transport for topic must be either 'multicast', 'tcp', 'udp' or left blank( = multicast)";
			throw ops::ConfigException(msg);
		}
	}

	Transport_T Topic::TRANSPORT_MC = "multicast";
	Transport_T Topic::TRANSPORT_TCP = "tcp";
	Transport_T Topic::TRANSPORT_UDP = "udp";

}
