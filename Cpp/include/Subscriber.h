/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2025 Lennart Andersson.
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
#include <chrono>

#include "OPSTypeDefs.h"
#include "Topic.h"
#ifdef OPS_ENABLE_DEBUG_HANDLER
#include "Lockable.h"
#endif
#include "OPSObject.h"
#include "DataNotifier.h"
#include "DeadlineMissedListener.h"
#include "FilterQoSPolicy.h"
#include "Listener.h"
#include "OPSMessage.h"
#include "ReceiveDataHandler.h"
#include "DeadlineTimer.h"
#include "PubIdChecker.h"
#include "DebugHandler.h"
#include "OPSEvent.h"
#include "TimeHelper.h"
#include "Validation.h"

namespace ops
{
    /// Abstract base class for Subscribers
    // Need to implement:
    //  *  Callback for received messages
    //     // Note that the receiveDataHandler messageLock is held while executing this method
    //     // void Subscriber::onNewEvent(Notifier<OPSMessage*>*, OPSMessage* const message)
    //
    //  *  Callback for deadline timeouts
    //     // void Subscriber::onNewEvent(Notifier<int>* , int )

    class SubscriberBase : public DataNotifier,
        public Listener<OPSMessage*>, public Listener<int>,
        protected Listener<ConnectStatus>, public Notifier<ConnectStatus>
    {
    public:
        //
        // Methods marked with "(CB)" can be used from the callback.
        // Other methods should not be used from the callback.
        //

        SubscriberBase(const Topic& t);
        virtual ~SubscriberBase();

        ///Starts subscription (starts listening to data)
        void start();

        ///Stops subscription (stops listening to data)
        void stop();

        bool isStarted() const noexcept { return m_started; }

        ///Returns a copy of this subscribers Topic.
        Topic getTopic() const;                                                      // (CB)

        ///Acquires the MessageLock
        // You should preferably use the MessageLock() class when using the lock.
        // NOTE: The MessageLock is held by the subscriber while in the "New Message" callback.
        bool aquireMessageLock();
        ///Releases the MessageLock
        void releaseMessageLock();

        ///Returns the number of reserved messages in the underlying ReceiveDataHandler
        // This value is the total nr for this topic on this participant not only
        // for this subscriber.
        int numReservedMessages() const                                              // (CB)
        {
            return receiveDataHandler->numReservedMessages();
        }

	    // Validation
        void defineValidation(ValidationStrategy strat, ValidationSubCallback cb = {});

      protected:
        ///The Topic this Subscriber subscribes to.
        Topic topic;

        ///The Participant to which this Subscriber belongs.
        Participant* participant{ nullptr };

        ///ReceiveDataHandler delivering new data samples to this Subscriber
        std::shared_ptr<ReceiveDataHandler> receiveDataHandler{ nullptr };

        std::unique_ptr<DeadlineTimer> deadlineTimer;
        std::chrono::milliseconds deadlineTimeout{ 0 };
        bool deadlineUsed{ false };

        void setupDeadlineTimer();

        using Listener<OPSMessage*>::onNewEvent;
        using Listener<int>::onNewEvent;

        // Called from ReceiveDataHandler (TCPClient)
        virtual void onNewEvent(Notifier<ConnectStatus>*, ConnectStatus arg) override
        {
            // Forward status to all connected
            notifyNewEvent(arg);
        }

        ValidationStrategy valStrat{ValidationStrategy::None};
        ValidationSubCallback valCall{};

      private:
        bool m_started{ false };
    };

    // =======================================================================================

    ///Base class for subscribing to Topics
    class Subscriber : public SubscriberBase
#ifdef OPS_ENABLE_DEBUG_HANDLER
		, DebugNotifyInterface
#endif
	{
    public:
        //
        // Methods marked with "(CB)" can be used from the callback.
        // Other methods should not be used from the callback.
        //

        Subscriber(const Topic& t);
        virtual ~Subscriber();

        ObjectName_T getName() const noexcept;                                       // (CB)
        virtual void setName(ObjectName_T name) noexcept;                            // (CB)

        DeadlineMissedEvent deadlineMissedEvent;

        ///Sets the deadline timeout for this subscriber (== 0 --> no deadline timeout (default)).
        ///If no message is received within deadline,
        ///listeners to deadlineMissedEvent will be notified
        static constexpr int64_t MAX_DEADLINE_TIMEOUT = TimeHelper::infinite;

        [[deprecated("Deprecated. Replaced by setDeadline() taking chrono duration")]]
        void setDeadlineQoS(int64_t deadlineT);

        [[deprecated("Deprecated. Replaced by getDeadline() returning chrono duration")]]
        int64_t getDeadlineQoS() const noexcept;

        void setDeadline(const std::chrono::milliseconds& deadlineT);
        std::chrono::milliseconds getDeadline() const noexcept;

        bool isDeadlineMissed() noexcept;

        // Note that the subscriber just borrows the reference. The caller still owns it and 
        // need to keep it around at least as long as the subscriber use it.
        void addFilterQoSPolicy(FilterQoSPolicy* fqos);
        void removeFilterQoSPolicy(FilterQoSPolicy* fqos);

        ///Sets the minimum time separation between to consecutive messages.
        ///Received messages in between will be ignored by this Subscriber
        [[deprecated("Deprecated. Replaced by setTimeBasedFilter() taking chrono duration")]]
        void setTimeBasedFilterQoS(int64_t timeBaseMinSeparationMillis) noexcept;   // (CB)

        [[deprecated("Deprecated. Replaced by getTimeBasedFilter() returning chrono duration")]]
        int64_t getTimeBasedFilterQoS() const noexcept;                             // (CB)
        void setTimeBasedFilter(const std::chrono::milliseconds& minSeparation) noexcept;  // (CB)
        std::chrono::milliseconds getTimeBasedFilter() const noexcept;              // (CB)                // (CB)

        ///Waits for new data to arrive or timeout.
        ///Returns: true if new data (i.e. unread data) exist.
        [[deprecated("Deprecated. Replaced by waitForNewData() taking chrono duration")]]
        bool waitForNewData(int timeoutMs);
        bool waitForNewData(const std::chrono::milliseconds& timeout);

        ///Checks if new data exist (same as 'waitForNewData(0)' but faster) 
        ///Returns: true if new data (i.e. unread data) exist.
        bool newDataExist() const noexcept {                                         // (CB)
            return hasUnreadData;
        }

        ///Returns a reference to the latest received data object.
        ///Clears the "new data" flag.
        ///NOTE: MessageLock should be held while working with the data object, to prevent a
        ///new incoming message to delete the current one while in use. 
        virtual OPSObject* getDataReference() noexcept                               // (CB)
        {
            hasUnreadData = false;
            return data;
        }

        ///Returns a reference to the latest received OPSMessage (including the latest data object).
        ///Clears the "new data" flag.
        ///NOTE: MessageLock should be held while working with the message, to prevent a
        ///new incoming message to delete the current one while in use.
        OPSMessage* getMessage() noexcept;                                           // (CB)

        void setHistoryMaxSize(size_t s) noexcept;

        ///Returns the latest received messages (upto the set maximum number).
        ///Messages MUST NOT be deleted, they are still in the Subscriber history.
        ///Use clearHistory() method below to delete messages
        ///NOTE: MessageLock should be held while working with the history, to prevent 
        ///new incoming messages to delete the ones in the history buffer while in use.
        std::deque<OPSMessage*> getHistory();

        ///Delete all or part of the history
        ///NOTE: MessageLock should be held while clearing the history, to prevent 
        ///new incoming messages to be deleted.
        void clearHistory(size_t keep = 0);

        using Listener<ConnectStatus>::onNewEvent;

        //Message listener callback
        void onNewEvent(Notifier<OPSMessage*>* sender, OPSMessage* message) override;

        //Deadline listener callback
        void onNewEvent(Notifier<int>* sender, int message) override;

        // Ack specific
        void AddExpectedPublisher(const ObjectName_T& pubname);
        bool CheckPublisher(const ObjectName_T& pubname);    // "" --> check all
        void RemoveExpectedPublisher(const ObjectName_T& pubname);

        // Activate (need to be called periodically)
        ///TBD controllable if called by user or timer???
        void Activate();

		//Default nullptr. Create a PublicationIdChecker if you want OPS to perform Publication Id checking.
		//The check is performed before any QoS filtering, so it sees all messages.
		//Add listerner(s) to the checker and you will be notified when:
		// - A new publisher is detected (Ip & Port of publisher is used)
		// - A Sequence Error is detected for an existing publisher
        PublicationIdChecker* pubIdChecker{ nullptr };

		//User data field that the owner of the subscriber can use for any purpose. Not used by OPS.
        uint64_t userData{ 0 };
		//User data field that the owner of the subscriber can use for any purpose. Not used by OPS.
        void* userPtr{ nullptr };

      protected:
        void checkAndNotifyDeadlineMissed();

        OPSMessage* m_message{ nullptr };

        OPSObject* data{ nullptr };
        OPSObject* getData() noexcept;
        bool firstDataReceived{ false };
        bool hasUnreadData{ false };

	private:
        ///Name of this subscriber
		ObjectName_T m_name;

        /// Receiver side filters that will be applied to data from receiveDataHandler before delivery to application layer.
        std::list<FilterQoSPolicy*> filterQoSPolicies;

        std::deque<OPSMessage*> messageBuffer;
        size_t messageBufferMaxSize{ 1 };
        void addToBuffer(OPSMessage* mess);

        Lockable filterQoSPolicyMutex;
        Event newDataEvent;

        ops_clock::time_point timeLastData;
        ops_clock::time_point timeLastDataForTimeBase;
        std::chrono::milliseconds timeBaseMinSeparationTime{ 0 };
        bool minSeparationUsed{ false };

        bool applyFilterQoSPolicies(const OPSMessage* message, const OPSObject* o);

        // ACK handling
        struct ACKFilter;
        std::unique_ptr<ACKFilter> _ackFilter;
        ops_clock::time_point _nextRegisterTime;

#ifdef OPS_ENABLE_DEBUG_HANDLER
        volatile int64_t _dbgSkip{ 0 };
        volatile int64_t _numReceived{ 0 };
		Lockable _dbgLock;
		virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp) override;
#endif
	};

	///RAII helper for aquire/release of the MessageLock
	class MessageLock
	{
		SubscriberBase& _sub;
	public:
		MessageLock(SubscriberBase& sub) : _sub(sub) { _sub.aquireMessageLock(); }
		~MessageLock() { _sub.releaseMessageLock(); }
	};
}
