/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2021 Lennart Andersson.
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

#ifndef ops_ParticipantH
#define	ops_ParticipantH

#include <map>
#include <exception>
#include <memory>

#include "OPSTypeDefs.h"
#include "ThreadPool.h"
#include "Runnable.h"
#include "IOService.h"
#include "SerializableFactory.h"
#include "Topic.h"
#include "OPSConfig.h"
#include "OPSObjectFactory.h"
#include "DeadlineTimer.h"
#include "Error.h"
#include "ErrorService.h"
#include "ParticipantInfoData.h"
#include "ParticipantInfoDataListener.h"
#include "SendDataHandler.h"
#include "DebugHandler.h"

namespace ops
{
	//Forward declaration..
	class ReceiveDataHandler;
	class UDPReceiveDataHandler;
	class ReceiveDataHandlerFactory;
	class SendDataHandlerFactory;
	class Domain;
    class Publisher;

	namespace execution_policy {
		enum Enum {
			threading,		// The Participant instance has its own thread that handles sockets, timeouts and 
							// performs the callbacks to user code.
			polling			// The user code need to "drive" the Participant instance by calling the Poll() method.
							// During the execution of Poll() the Participant will handle work that is ready.
		};
	}

	class Participant : Runnable, Listener<int>
	{
		friend class SubscriberBase;
		friend class Publisher;
		friend class UDPReceiveDataHandler;
		friend class TCPReceiveDataHandler;
		friend class ReceiveDataHandlerFactory;
		friend class SendDataHandlerFactory;
		friend class ParticipantInfoDataListener;
	public:
		// -------------------------------------------------------------------
		// Mismatched header and library detection
		struct mismatched_headers_and_library : public std::exception {
			const char* what() const noexcept override { return "Mismatched headers and compiled library"; }
		};
		static InternalString_T LibraryCompileSignature();
		static InternalString_T HeaderCompileSignature() { return InternalString_T(OPS_COMPILESIGNATURE) + NumberToString(fixed_string_length_check_value); }
		static bool CheckCompileSignature() { return LibraryCompileSignature() == HeaderCompileSignature(); }

		// -------------------------------------------------------------------
		///By Singelton, one Participant per 'domainID::participantID'
		static std::map<ParticipantKey_T, Participant*> instances;

		//NOTE: Since Participants are singletons per 'domainID::participantID', the execution_policy only has effect 
		//at the first call when the Participant is created.
		static Participant* getInstance(ObjectName_T domainID, execution_policy::Enum policy = execution_policy::threading) 
		{
			return getInstance(domainID, "", policy);
		}
		static Participant* getInstance(ObjectName_T domainID, ObjectName_T participantID, execution_policy::Enum policy = execution_policy::threading)
		{
			return getInstance(domainID, participantID, "", policy);
		}
		static Participant* getInstance(ObjectName_T domainID, ObjectName_T participantID, FileName_T configFile, execution_policy::Enum policy = execution_policy::threading)
		{
			if (!CheckCompileSignature()) throw mismatched_headers_and_library();
			return getInstanceInternal(domainID, participantID, configFile, policy);
		}
		
		// -------------------------------------------------------------------
		//Report an error via all participants ErrorServices or the static ErrorService if it exists
		static void reportStaticError(Error* err);

#ifdef OPS_ENABLE_DEBUG_HANDLER
		//Create a Topic for subscribing or publishing on DebugRequest
		ops::Topic createDebugTopic() const;
#endif

		//Create a Topic for subscribing or publishing on ParticipantInfoData
		ops::Topic createParticipantInfoTopic() const;

		//Get the name that this participant has set in its ParticipantInfoData
		InternalString_T getPartInfoName() const {
			return partInfoData.name;
		}

		//Add a SerializableFactory which has support for data types (i.e. OPSObject derivatives you want this Participant to understand)
		//Takes over ownership of the object and it will be deleted with the participant
		void addTypeSupport(ops::SerializableFactory* typeSupport);

		//Create a From the ops config. See config below.
		Topic createTopic(ObjectName_T name);

		void run() override;

		//Make this participant report an Error, which will be delivered to all ErrorService listeners
		void reportError(Error* err);

		///Deadline listener callback
		void onNewEvent(Notifier<int>* sender, int message) override;
		
		///Cleans up ReceiveDataHandlers
		//void cleanUpReceiveDataHandlers();

		// Get a pointer to the underlying IOService.
        // Note: Only valid while the participant exist
		IOService* getIOService() const
		{
			return ioService.get();
		}
        // Note: Only valid while the participant exist
        OPSConfig* getConfig() const
		{
			return config.get();
		}
		
		ErrorService* getErrorService() const
		{
			return errorService.get();
		}

		// A static error service that user could create, by calling getStaticErrorService(), and connect to. 
		// If it exist, "reportStaticError()" will use this instead of using all participants errorservices
		// which leads to duplicated error messages when several participants exist.
		// This static errorservice also has the advantage that errors during Participant creation can be logged.
		static ErrorService* getStaticErrorService();

		Domain* getDomain() const
		{
			return domain;
		}

		///Get a pointer to the data type factory used in this Participant. 
		//TODO: Rename?
		OPSObjectFactory* getObjectFactory() const
		{
			return objectFactory.get();
		}

		///Deprecated, use getErrorService()->addListener instead. Add a listener for OPS core reported Errors
		void addListener(Listener<Error*>* listener);
		///Deprecated, use getErrorService()->removeListener instead. Remove a listener for OPS core reported Errors
		void removeListener(Listener<Error*>* listener);

		virtual ~Participant();

		// Method to "drive" the Participant when the execution_policy is "polling"
		bool Poll();

		execution_policy::Enum GetExecutionPolicy() const noexcept { return _policy; }

        // Check under laying transports if there is any data not processed
        bool dataAvailable();

#ifdef OPS_ENABLE_DEBUG_HANDLER
		DebugHandler debugHandler;
#endif
	private:
		execution_policy::Enum _policy;

		static Participant* getInstanceInternal(ObjectName_T domainID, ObjectName_T participantID, FileName_T configFile, execution_policy::Enum policy);

		///Constructor is private instance are acquired through getInstance()
		Participant(ObjectName_T domainID_, ObjectName_T participantID_, FileName_T configFile_, execution_policy::Enum policy);

		///Remove this instance from the static instance map
		void RemoveInstance();

		///The IOService used for this participant, it handles communication and timers for all receivers, subscribers and member timers of this Participant.
		std::unique_ptr<IOService> ioService;
		std::shared_ptr<OPSConfig> config;

		///The ErrorService
        std::unique_ptr<ErrorService> errorService;

		///The threadPool drives ioService. By default Participant use a SingleThreadPool i.e. only one thread drives ioService.
        std::unique_ptr<ThreadPool> threadPool;

		///A timer that fires with a certain periodicity, it keeps this Participant alive in the system by publishing ParticipantInfoData
        std::unique_ptr<DeadlineTimer> aliveDeadlineTimer;

		//------------------------------------------------------------------------
		///A publisher of ParticipantInfoData
        std::unique_ptr<Publisher> partInfoPub;
                
		///The ParticipantInfoData that partInfoPub will publish periodically
		ParticipantInfoData partInfoData;
		Lockable partInfoDataMutex;

		//Visible to friends only
		void setUdpTransportInfo(Address_T ip, int port);
		void registerTcpTopic(const ObjectName_T topicName, std::shared_ptr<ReceiveDataHandler> handler);
		void unregisterTcpTopic(const ObjectName_T topicName, std::shared_ptr<ReceiveDataHandler> handler);
		bool hasPublisherOn(const ObjectName_T& topicName);
		bool hasSubscriberOn(const ObjectName_T&topicName);

        Domain* domain{ nullptr };

		//------------------------------------------------------------------------
		///A listener and handler for ParticipantInfoData
        std::unique_ptr<ParticipantInfoDataListener> partInfoListener;

		//------------------------------------------------------------------------
		//
        std::unique_ptr<ReceiveDataHandlerFactory> receiveDataHandlerFactory;
        std::unique_ptr<SendDataHandlerFactory> sendDataHandlerFactory;

		//Visible to friends only
		//TODO: Deprecate and delegate to receiveDataHandlerFactory???
        std::shared_ptr<ReceiveDataHandler> getReceiveDataHandler(Topic top);
		void releaseReceiveDataHandler(Topic top);

		///Visible to friends only
		//TODO: Deprecate and delegate to sendDataHandlerFactory???
		std::shared_ptr<SendDataHandler> getSendDataHandler(Topic top);
		void releaseSendDataHandler(const Topic top);		
		void updateSendPartInfo(const Topic top);

		///Mutex for ioService, used to shutdown safely
		Lockable serviceMutex;

		///The domainID for this Participant
		ObjectName_T domainID;
		///The id of this participant, must be unique in process
		ObjectName_T participantID;

		///As long this is true, we keep on running this participant
        volatile bool keepRunning{ true };

		///The interval with which this Participant publishes ParticipantInfoData
        int64_t aliveTimeout{ 1000 };

		///The data type factory used in this Participant. 
        std::unique_ptr<OPSObjectFactory> objectFactory;

		///Static Mutex used by factory methods getInstance()
		static Lockable creationMutex;
	};

}
#endif
