/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2026 Lennart Andersson.
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
#include <thread>
#include <algorithm>
#ifdef _WIN32
#include <process.h>
#include <winsock.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include "OPSTypeDefs.h"
#include "Participant.h"
#include "opsidls/OPSConstants.h"
#include "ReceiveDataHandler.h"
#include "ReceiveDataHandlerFactory.h"
#include "SendDataHandlerFactory.h"
#include "OPSObjectFactoryImpl.h"
#include "ConfigException.h"
#include "CommException.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "BasicError.h"
#include "NetworkSupport.h"
#include "ThreadSupport.h"
#include "TimeHelper.h"
#include "InProcDistributor.h"

namespace ops
{
    using namespace opsidls;

	//static
	std::map<ParticipantKey_T, Participant*> Participant::instances;
	Lockable Participant::creationMutex;

	// Compile signature
	InternalString_T Participant::LibraryCompileSignature()
	{
		return InternalString_T(OPS_COMPILESIGNATURE) + NumberToString(fixed_string_length_check_value);
	}

	// --------------------------------------------------------------------------------
	// A static error service that user could create, by calling getStaticErrorService(), and connect to.
	// If it exist, "reportStaticError()" will use this instead of using all participants errorservices
	// which leads to duplicated error messages when several participants exist.
	// This static errorservice also has the advantage that errors during Participant creation can be logged.
	static ErrorService* staticErrorService = nullptr;

	ErrorService* Participant::getStaticErrorService()
	{
		const SafeLock lock(creationMutex);
		if (!staticErrorService) {
			staticErrorService = new ErrorService();
		}
		return staticErrorService;
	}


	// --------------------------------------------------------------------------------

	static ParticipantKey_T getKey(const ObjectName_T& domainID_, const ObjectName_T& participantID)
	{
		ParticipantKey_T key = domainID_;
		key += "::";
		key += participantID;
		return key;
	}

	Participant* Participant::getInstanceInternal(ObjectName_T domainID_, ObjectName_T participantID, FileName_T configFile, execution_policy::Enum policy)
	{
        if (participantID == "") { participantID = OPSConstants::DEFAULT_PARTICIPANT_ID(); }
		const ParticipantKey_T key = getKey(domainID_, participantID);
		const SafeLock lock(creationMutex);
		if (instances.find(key) == instances.end()) {
			try
			{
				Participant* const newInst = new Participant(domainID_, participantID, configFile, policy);
				instances[key] = newInst;
			}
			catch(ops::ConfigException& ex)
			{
				ErrorMessage_T msg("Exception: ");
				msg += ex.what();
				BasicError err("Participant", "Participant", msg);
				reportStaticError(&err);
				return nullptr;
			}
			catch (ops::exceptions::CommException& ex)
			{
				BasicError err("Participant", "Participant", ex.what());
				reportStaticError(&err);
				return nullptr;
			}
			catch(...)
			{
				BasicError err("Participant", "Participant", "Unknown Exception");
				reportStaticError(&err);
				return nullptr;
			}
		}
		return instances[key];
	}

	///Remove this instance from the static instance map
	void Participant::RemoveInstance()
	{
		const ParticipantKey_T key = getKey(domainID, participantID);
		const SafeLock lock(creationMutex);
		instances.erase(key);
	}

	Participant::Participant(ObjectName_T const domainID_, ObjectName_T const participantID_, FileName_T const configFile_, execution_policy::Enum const policy):
#ifdef OPS_ENABLE_DEBUG_HANDLER
		debugHandler(*this),
#endif
		_policy(policy),
		domainID(domainID_),
		participantID(participantID_),
		metaDataHnd(errorService, participantID_, domainID_)
	{
        ioService = IOService::create();

		if(!ioService)
		{
			//Error, should never happen, throw?
            throw exceptions::CommException("No config on rundirectory");
		}

		//Should trow?
		if (configFile_ == "") {
			config = OPSConfig::getConfig();
		} else {
			// Note that the getDomain() call below returns a reference to an object internally in config.
			config = OPSConfig::getConfig(configFile_);
		}
		if(!config)
		{
			throw ops::ConfigException("No config on rundirectory?");
		}

		//Get the domain from config. Note should not be deleted, owned by config.
		domain = config->getDomain(domainID);
		if(!domain)
		{
			ExceptionMessage_T msg("Domain '");
			msg += domainID;
			msg += "' missing in config-file";
			throw exceptions::CommException(msg);
		}

		//Create a factory instance for each participant
		objectFactory = std::make_unique<OPSObjectFactoryImpl>();

		//-----------Create delegate helper classes---
		receiveDataHandlerFactory = std::make_unique<ReceiveDataHandlerFactory>();
		sendDataHandlerFactory = std::make_unique<SendDataHandlerFactory>();
		inProcDistributor = std::make_shared<InProcDistributor>();
		//--------------------------------------------

		//------------Create timer for periodic events-
		aliveDeadlineTimer = DeadlineTimer::creat(ioService.get());
		aliveDeadlineTimer->addListener(this);

		// Create the meta data publisher if user hasn't disabled it for the domain.
		try {
			if (domain->getMetaDataMcPort() > 0) { metaDataHnd.setup(createParticipantInfoTopic()); }
		} catch (std::exception& ex)
		{
			ErrorMessage_T errMessage = "Failed to create publisher for ParticipantInfoTopic. Check localInterface and metaDataMcPort in configuration file.";
			errMessage += " Exception: ";
			errMessage += ex.what();
			BasicError err("Participant", "constructor", errMessage);
			reportStaticError(&err);
		}

		// Now start the timer. Calls onNewEvent(Notifier<int>* , int ) on timeout
		aliveDeadlineTimer->start(aliveTimeout);
		//--------------------------------------------

		//------------Create thread pool--------------
		if (_policy == execution_policy::threading) {
			threadPool = thread_support::CreateThreadPool();
			threadPool->addRunnable(this);
			threadPool->start();
		}
	}

	Participant::~Participant()
	{
		OPS_DES_TRACE("Part: Destructor()...");

		// We assume that the user has deleted all publishers and subscribers connected to this Participant.
		// We also assume that the user has cancelled eventual deadlinetimers etc. connected to the ioService.
		// We also assume that the user has unreserved() all messages that he has reserved().

		// Remove this instance from the static instance map
		RemoveInstance();

		{
			const SafeLock lock(serviceMutex);

			// Indicate that shutdown is in progress
			keepRunning = false;

			// We have indicated shutdown in progress. Cleanup the meta-data handler.
			// Note that this uses our sendDataHandlerFactory, receiveDataHandlerFactory and requires ioService to be running.
			metaDataHnd.cleanup();

#ifdef OPS_ENABLE_DEBUG_HANDLER
			debugHandler.Stop();
#endif
		}

		// Now delete our send factory
		sendDataHandlerFactory.reset();

		// Our timer is required for ReceiveDataHandlers to be cleaned up so it shouldn't be stopped
		// before receiveDataHandlerFactory is finished.
		// Wait until receiveDataHandlerFactory has no more cleanup to do
		try {
			while (!receiveDataHandlerFactory->cleanUpDone()) {
				if (_policy == execution_policy::polling) { Poll(); }	// Need to drive timer in case the user forget
				TimeHelper::sleep(std::chrono::milliseconds(1));
			}
		}
		catch (...)
		{}

		// Now stop and delete our timer (NOTE requires ioService to be running).
		// If the timer is in the callback, the delete will wait for it to finish and then the object is deleted.
		aliveDeadlineTimer.reset();

		// Now time to delete our receive factory
		receiveDataHandlerFactory.reset();

		// There should now not be anything left requiring the ioService to be running.

		// Then we request the IO Service to stop the processing (it's running on the threadpool).
		// The stop() call will not block, it just signals that we want it to finish as soon as possible.
		if (ioService != nullptr) { ioService->stop(); }

		// Now we delete the threadpool, which will wait for the thread(s) to finish
		threadPool.reset();

		// Now when the threads are gone, it's safe to delete the rest of our objects
		objectFactory.reset();
		config.reset();
		// All objects connected to our ioservice should now be deleted, so it should be safe to delete it
        ioService.reset();

		OPS_DES_TRACE("Part: Destructor() Finished");
	}

	ops::Topic Participant::createParticipantInfoTopic() const
	{
		ops::Topic infoTopic("ops.bit.ParticipantInfoTopic", domain->getMetaDataMcPort(), "ops.ParticipantInfoData", domain->getDomainAddress());
        Address_T addr = GetAddrFromNameEx(domain->getLocalInterface(), ioService.get());
        infoTopic.setLocalInterface(addr);
		infoTopic.setTimeToLive(domain->getTimeToLive());
		infoTopic.setDomainID(domainID);
		infoTopic.setParticipantID(participantID);
		infoTopic.setTransport(Topic::TRANSPORT_MC);
		return infoTopic;
	}

#ifdef OPS_ENABLE_DEBUG_HANDLER
	ops::Topic Participant::createDebugTopic() const
	{
		ops::Topic debugTopic("ops.DebugTopic", domain->getDebugMcPort(), opsidls::DebugRequestResponseData::getTypeName(), domain->getDomainAddress());
        Address_T addr = GetAddrFromNameEx(domain->getLocalInterface(), ioService.get());
        debugTopic.setLocalInterface(addr);
		debugTopic.setTimeToLive(domain->getTimeToLive());
		debugTopic.setDomainID(domainID);
		debugTopic.setParticipantID(participantID);
		debugTopic.setTransport(Topic::TRANSPORT_MC);
		return debugTopic;
	}
#endif

	// Report an error via the participants ErrorService
	void Participant::reportError(Error* const err)
	{
		errorService.report(err);
	}

	// Report an error via all participants ErrorServices
	void Participant::reportStaticError(Error* const err)
	{
		if (staticErrorService != nullptr) {
			staticErrorService->report(err);

		} else {
			std::map<ParticipantKey_T, Participant*>::iterator it = instances.begin();
			while(it !=instances.end())
			{
				it->second->getErrorService()->report(err);
				++it;
			}
		}
	}

    // Check under laying transports if there is any data not processed
    bool Participant::dataAvailable()
    {
        return receiveDataHandlerFactory->dataAvailable();
    }

	// Method to "drive" the Participant when the execution_policy is "polling"
	bool Participant::Poll()
	{
        if (_policy != execution_policy::polling) { return false; }
		ioService->poll();
		return true;
	}

	// This will be called by our threadpool (started in the constructor())
	void Participant::run()
	{
        if (_policy != execution_policy::threading) { return; }
		// Set name of current thread for debug purpose
		InternalString_T name("OPSP_");
		name += domainID;
		thread_support::SetThreadName(name.c_str());
		ioService->run();
	}

	// Called on aliveDeadlineTimer timeouts
	void Participant::onNewEvent(Notifier<int>* , int )
	{
		const SafeLock lock(serviceMutex);
		receiveDataHandlerFactory->cleanUpReceiveDataHandlers();

		if (keepRunning) {
			try {
				if (domain->getMetaDataMcPort() > 0) {
					metaDataHnd.publish();
				}
			} catch (std::exception& ex)
			{
				ErrorMessage_T errMessage = "Failed to publish ParticipantInfoTopic data.";
				errMessage += " Exception: ";
				errMessage += ex.what();
				BasicError err("Participant", "onNewEvent", errMessage);
				reportStaticError(&err);
			}

#ifdef OPS_ENABLE_DEBUG_HANDLER
			if (domain->getDebugMcPort() != 0) {
				debugHandler.Start();
			}
#endif
		}

		// Start a new timeout
		aliveDeadlineTimer->start(aliveTimeout);
	}

	void Participant::addTypeSupport(ops::SerializableFactory* const typeSupport)
	{
		objectFactory->add(typeSupport);
	}

	Topic Participant::createTopic(ObjectName_T const name)
	{
		Topic topic = domain->getTopic(name);
		topic.setParticipantID(participantID);
		topic.setDomainID(domainID);
//		topic.participant = this;

		Address_T addr = GetAddrFromNameEx(topic.getLocalInterface(), ioService.get());
		if (addr != "") { topic.setLocalInterface(addr); }

		if ((topic.getTransport() == Topic::TRANSPORT_TCP) || ((topic.getTransport() == Topic::TRANSPORT_UDP))) {
			addr = GetAddrFromName(topic.getDomainAddress(), ioService.get());
			if (addr != "") { topic.setDomainAddress(addr); }
		}
		else if (topic.getTransport() == Topic::TRANSPORT_SHMEM) {
			// Update info for shared memory connect
			if (domain->getMetaDataMcPort() == 0) {
				// No process ID added to shared memory name (ie. One-2-Many transport)
				topic.setDomainAddress("");
			}
			else {
				// Process ID to be added to shared memory name (ie. Many-2-Many transport)
				topic.setDomainAddress(metaDataHnd.getPid());
			}
		}

		return topic;
	}

	///Deprecated, use getErrorService()->addListener instead. Add a listener for OPS core reported Errors
	void Participant::addListener(Listener<Error*>* const listener)
	{
		errorService.addListener(listener);
	}

	///Deprecated, use getErrorService()->removeListener instead. Remove a listener for OPS core reported Errors
	void Participant::removeListener(Listener<Error*>* const listener)
	{
		errorService.removeListener(listener);
	}

	bool Participant::hasPublisherOn(const ObjectName_T& topicName)
	{
		return metaDataHnd.hasPublisherOn(topicName);
	}

	bool Participant::hasSubscriberOn(const ObjectName_T& topicName)
	{
		return metaDataHnd.hasSubscriberOn(topicName);
	}

	std::shared_ptr<ReceiveDataHandler> Participant::getReceiveDataHandler(const Topic& top)
	{
        std::shared_ptr<ReceiveDataHandler> result = receiveDataHandlerFactory->getReceiveDataHandler(top, *this);
		if (result != nullptr) {
			metaDataHnd.addSubTopic(top);
		}
		return result;
	}

	void Participant::releaseReceiveDataHandler(const Topic& top)
	{
		receiveDataHandlerFactory->releaseReceiveDataHandler(top, *this);
		metaDataHnd.removeSubTopic(top);
	}

	std::shared_ptr<SendDataHandler> Participant::getSendDataHandler(const Topic& top)
	{
		const std::shared_ptr<SendDataHandler> result = sendDataHandlerFactory->getSendDataHandler(top, *this);
		// We can't update Participant Info here, delayed until updateSendPartInfo()
		return result;
	}

	void Participant::updateSendPartInfo(const Topic& top, Action action)
	{
		if (action == Action::add) {
			metaDataHnd.addPubTopic(top);
		}
		else {
			metaDataHnd.removePubTopic(top);
		}
	}

	void Participant::releaseSendDataHandler(const Topic& top)
	{
		sendDataHandlerFactory->releaseSendDataHandler(top, *this);
	}

}
