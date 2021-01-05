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

#include "OPSTypeDefs.h"
#include "Domain.h"
#include "NoSuchTopicException.h"
#include "XMLArchiverIn.h"
#include "ConfigException.h"

namespace ops
{

Domain::Domain()
{
	appendType(TypeId_T("Domain"));
}

Domain::Domain(const Domain& other)
{
	other.fillClone(this);
}

Domain& Domain::operator= (const Domain& other)
{
	if (this != &other) {
		other.fillClone(this);
	}
	return *this;
}

// Returns a newely allocated deep copy/clone of this object.
Domain* Domain::clone()
{
	Domain* ret = new Domain;
	fillClone(ret);
	return ret;
}

// Fills the parameter obj with all values from this object.
void Domain::fillClone(Domain* obj) const
{
	if (this == obj) { return; }
	ops::OPSObject::fillClone(obj);
	obj->Domain_version = Domain_version;
	obj->domainAddress = domainAddress;
	obj->timeToLive = timeToLive;
	obj->localInterface = localInterface;
	obj->inSocketBufferSize = inSocketBufferSize;
	obj->outSocketBufferSize = outSocketBufferSize;
	obj->domainID = domainID;
	obj->metaDataMcPort = metaDataMcPort;
	obj->debugMcPort = debugMcPort;
	obj->optNonVirt = optNonVirt;
	obj->heartbeatPeriod = heartbeatPeriod;
	obj->heartbeatTimeout = heartbeatTimeout;
	obj->resendNum = resendNum;
	obj->resendTimeMs = resendTimeMs;
	obj->registerTimeMs = registerTimeMs;

	cloneVectorPtr<Topic>(obj->topics, topics);
	cloneVectorPtr<Channel>(obj->channels, channels);
	cloneVectorPtr<Transport>(obj->transports, transports);
}

Address_T Domain::getDomainAddress() const noexcept
{
	return domainAddress;
}

void Domain::checkTopicValues(Topic* const top) const
{
	if (top->getDomainAddress() == "") { top->setDomainAddress(domainAddress); }
	if (top->getLocalInterface() == "") { top->setLocalInterface(localInterface); }
	if (top->getTimeToLive() < 0) { top->setTimeToLive(timeToLive); }
	if (top->getInSocketBufferSize() < 0) { top->setInSocketBufferSize(inSocketBufferSize); }
	if (top->getOutSocketBufferSize() < 0) { top->setOutSocketBufferSize(outSocketBufferSize); }
    if (top->resendNum < 0) { top->resendNum = resendNum; }
    if (top->resendTimeMs < 0) { top->resendTimeMs = resendTimeMs; }
    if (top->registerTimeMs < 0) { top->registerTimeMs = registerTimeMs; }
    top->optNonVirt = optNonVirt;
	top->heartbeatPeriod = heartbeatPeriod;
	top->heartbeatTimeout = heartbeatTimeout;
}

std::vector<Topic* > Domain::getTopics() const
{
	for (auto& x : topics) {
		checkTopicValues(x);
	}
	return topics;
}

Topic Domain::getTopic(ObjectName_T const name) const
{
	for (auto& x : topics) {
		if (x->getName() == name) {
			checkTopicValues(x);
			return *x;
		}
	}
	ExceptionMessage_T msg = "Topic ";
	msg += name;
	msg += " does not exist in ops config file.";
	throw NoSuchTopicException(msg);
}

bool Domain::existsTopic(ObjectName_T const name) const
{
	for (auto& x : topics) {
		if (x->getName() == name) { return true; }
	}
	return false;
}

ObjectName_T Domain::getDomainID() const noexcept
{
	return domainID;
}

void Domain::serialize(ArchiverInOut* const archiver)
{
	OPSObject::serialize(archiver);
    if (idlVersionMask != 0) {
        archiver->inout("Domain_version", Domain_version);
        ValidateVersion("Domain", Domain_version, Domain_idlVersion);
    } else {
        Domain_version = 0;
    }
    archiver->inout("domainID", domainID);
	archiver->inout<Topic>("topics", topics);
	archiver->inout("domainAddress", domainAddress);
	archiver->inout("localInterface", localInterface);
	archiver->inout("timeToLive", timeToLive);
	archiver->inout("inSocketBufferSize", inSocketBufferSize);
	archiver->inout("outSocketBufferSize", outSocketBufferSize);
	archiver->inout("metaDataMcPort", metaDataMcPort);

	// To not break binary compatibility we only do this when we know we are
	// reading from an XML-file
	if (dynamic_cast<XMLArchiverIn*>(archiver) != nullptr) { 
		archiver->inout<Channel>("channels", channels);
		archiver->inout<Transport>("transports", transports);
		archiver->inout("debugMcPort", debugMcPort);
		archiver->inout("optNonVirt", optNonVirt);
		archiver->inout("heartbeatPeriod", heartbeatPeriod);
		archiver->inout("heartbeatTimeout", heartbeatTimeout);
        archiver->inout("resendNum", resendNum);
        archiver->inout("resendTimeMs", resendTimeMs);
        archiver->inout("registerTimeMs", registerTimeMs);
        checkTransports();
	}
}

Channel* Domain::findChannel(ChannelId_T const id) const
{
	if (id != "") {
		for (auto& x : channels) {
			if (id == x->channelID) { return x; }
		}
	}
	return nullptr;
}

Topic* Domain::findTopic(ObjectName_T const id) const
{
	if (id != "") {
		for (auto& x : topics) {
			if (id == x->getName()) { return x; }
		}
	}
	return nullptr;
}

void Domain::checkTransports() const
{
	// Now update topics with values from the transports and channels
	// Loop over all transports and for each topic, see if it needs parameters from the channel
	for (auto& trans : transports) {
		// Get channel
		const Channel* channel = findChannel(trans->channelID);
		if (channel == nullptr) {
			ExceptionMessage_T msg("Non existing channelID: '");
			msg += trans->channelID;
			msg += "' used in transport specification.";
			throw ops::ConfigException(msg);
		} else {
			for (auto& topname : trans->topics) {
				Topic* const top = findTopic(topname);
				if (top == nullptr) {
					ExceptionMessage_T msg("Non existing topicID: '");
					msg += topname;
					msg += "' used in transport specification.";
					throw ops::ConfigException(msg);
				} else {
					channel->populateTopic(*top);
				}
			}
		}
	}
}

int Domain::getTimeToLive() const noexcept
{
	return timeToLive;
}

Address_T Domain::getLocalInterface() const noexcept
{
	return localInterface;
}

int Domain::getInSocketBufferSize() const noexcept
{
	return inSocketBufferSize;
}

int Domain::getOutSocketBufferSize() const noexcept
{
	return outSocketBufferSize;
}

int Domain::getMetaDataMcPort() const noexcept
{
	return metaDataMcPort;
}

int Domain::getDebugMcPort() const noexcept
{
	return debugMcPort;
}

bool Domain::getOptNonVirt() const noexcept
{
	return optNonVirt;
}
int Domain::getHeartbeatPeriod() const noexcept
{
	return heartbeatPeriod;
}
int Domain::getHeartbeatTimeout() const noexcept
{
	return heartbeatTimeout;
}

Domain::~Domain()
{
	for (auto& x : topics) { delete x; }
	topics.clear();
	for (auto& x : channels) { delete x; }
	channels.clear();
	for (auto& x : transports) { delete x; }
	transports.clear();
}

}
