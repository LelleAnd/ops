/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2020 Lennart Andersson.
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

#pragma once

#include <list>
#include <deque>
#include <climits>

#include "OPSTypeDefs.h"
#include "Topic.h"
#include "Lockable.h"
#include "OPSObject.h"
#include "DataNotifier.h"
#include "DeadlineMissedListener.h"
#include "FilterQoSPolicy.h"
#include "Listener.h"
#include "OPSMessage.h"
#include "ReceiveDataHandler.h"
#include "DeadlineTimer.h"
#include "OPSExport.h"
#include "PubIdChecker.h"
#include "DebugHandler.h"
#include "OPSEvent.h"

namespace ops
{

    class OPS_EXPORT Subscriber : public DataNotifier, public Lockable, public Listener<OPSMessage*>, public Listener<int>,
		protected Listener<ConnectStatus>, public Notifier<ConnectStatus>
#ifdef OPS_ENABLE_DEBUG_HANDLER
		, DebugNotifyInterface
#endif
	{
    public:
        //
        // Methods marked with "(CB)" can be used from the callback.
        // Other methods should not be used from the callback.
        //

        Subscriber(Topic t);
        virtual ~Subscriber();

        ///Starts communication.
        void start();

        ///Stops communication, unsubscribe this subscriber to data.
        void stop();

        DeadlineMissedEvent deadlineMissedEvent;

        ///Sets the deadline timout for this subscriber.
        ///If no message is received within deadline,
        ///listeners to deadlineMissedEvent will be notified
        const static int64_t MAX_DEADLINE_TIMEOUT = LLONG_MAX;
        void setDeadlineQoS(int64_t deadlineT);
        int64_t getDeadlineQoS() const noexcept;

        // Note that the subscriber just borrows the reference. The caller still owns it and 
        // need to keep it around at least as long as the subscriber use it.
        void addFilterQoSPolicy(FilterQoSPolicy* fqos);
        void removeFilterQoSPolicy(FilterQoSPolicy* fqos);

        int64_t getTimeBasedFilterQoS() const noexcept;                              // (CB)
        ///Sets the minimum time separation between to consecutive messages.
        ///Received messages in between will be ignored by this Subscriber
        void setTimeBasedFilterQoS(int64_t timeBaseMinSeparationMillis) noexcept;    // (CB)

        ///Returns a copy of this subscribers Topic.
        Topic getTopic() const;                                             // (CB)

        ///Waits for new data to arrive or timeout.
        ///Returns: true if new data (i.e. unread data) exist.
        bool waitForNewData(int timeoutMs);

        ///Checks if new data exist (same as 'waitForNewData(0)' but faster) 
        ///Returns: true if new data (i.e. unread data) exist.
        bool newDataExist() const noexcept {                                         // (CB)
            return hasUnreadData;
        }

        ///Acquires/Releases the MessageLock.
        ///You should preferably use the MessageLock() class when using the lock.
        ///NOTE: The MessageLock is held by the subscriber while in the "New Message" callback.
        bool aquireMessageLock();
        void releaseMessageLock();

        ///Returns a reference to the latest received OPSMessage (including the latest data object).
        ///Clears the "new data" flag.
        ///NOTE: MessageLock should be held while working with the message, to prevent a
        ///new incomming message to delete the current one while in use.
        OPSMessage* getMessage() noexcept;                                           // (CB)

        ///Returns the number of reserved messages in the underlying ReceiveDataHandler
        ///This value is the total nr for this topic on this participant not only
        ///for this subscriber.
        int numReservedMessages() const                                     // (CB)
        {
            return receiveDataHandler->numReservedMessages();
        }

        ObjectName_T getName() const noexcept;                                       // (CB)
        void setName(ObjectName_T name) noexcept;                                    // (CB)

        bool isDeadlineMissed();

        void setHistoryMaxSize(int s) noexcept;
        std::deque<OPSMessage*> getHistory();

        ///Returns a reference to the latest received data object.
        ///Clears the "new data" flag.
        ///NOTE: MessageLock should be held while working with the data object, to prevent a
        ///new incomming message to delete the current one while in use. 
        virtual OPSObject* getDataReference() noexcept                               // (CB)
        {
            hasUnreadData = false;
            return data;
        }

        //Message listener callback
        void onNewEvent(Notifier<OPSMessage*>* sender, OPSMessage* message) override;

        //Deadline listener callback
        void onNewEvent(Notifier<int>* sender, int message) override;

		//Default nullptr. Create a PublicationIdChecker if you want OPS to perform Publication Id checking.
		//The check is performed before any QoS filtering, so it sees all messages.
		//Add listerner(s) to the checker and you will be notified when:
		// - A new publisher is detected (Ip & Port of publisher is used)
		// - A Sequence Error is detected for an existing publisher
		PublicationIdChecker* pubIdChecker = nullptr;

		//User data field that the owner of the subscriber can use for any purpose. Not used by OPS.
		uint64_t userData = 0;
		//User data field that the owner of the subscriber can use for any purpose. Not used by OPS.
		void* userPtr = nullptr;

	protected:
        void checkAndNotifyDeadlineMissed();

        OPSMessage* message = nullptr;

        OPSObject* data = nullptr;
        OPSObject* getData() noexcept;
        bool firstDataReceived = false;
        bool hasUnreadData = false;

		// Called from ReceiveDataHandler (TCPClient)
		virtual void onNewEvent(Notifier<ConnectStatus>* sender, ConnectStatus arg) override
		{
			UNUSED(sender);
			// Forward status to all connected
			notifyNewEvent(arg);
		}

	private:
        ///The Participant to which this Subscriber belongs.
        Participant* participant = nullptr;

        ///ReceiveDataHandler delivering new data samples to this Subscriber
        std::shared_ptr<ReceiveDataHandler> receiveDataHandler = nullptr;

        ///The Topic this Subscriber subscribes to.
        Topic topic;

        ///Name of this subscriber
		ObjectName_T name;

        ///Receiver side filters that will be applied to data from receiveDataHandler before delivery to application layer.
        std::list<FilterQoSPolicy*> filterQoSPolicies;

        std::deque<OPSMessage*> messageBuffer;
        unsigned int messageBufferMaxSize = 1;
        void addToBuffer(OPSMessage* mess);

        Lockable filterQoSPolicyMutex;
        Event newDataEvent;

        int64_t timeLastData = 0;
        int64_t timeLastDataForTimeBase = 0;
        int64_t timeBaseMinSeparationTime = 0;
        int64_t deadlineTimeout = 0;

        bool applyFilterQoSPolicies(OPSObject* o);

        void cancelDeadlineTimeouts();

        bool deadlineMissed = false;

        bool started = false;

        DeadlineTimer* deadlineTimer = nullptr;

#ifdef OPS_ENABLE_DEBUG_HANDLER
		volatile int64_t _dbgSkip = 0;
		volatile int64_t _numReceived = 0;
		Lockable _dbgLock;
		virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp) override;
#endif
	};

	// RAII helper for aquire/release of the MessageLock
	class MessageLock
	{
		Subscriber& _sub;
	public:
		MessageLock(Subscriber& sub) : _sub(sub) { _sub.aquireMessageLock(); }
		~MessageLock() { _sub.releaseMessageLock(); }
	};
}
