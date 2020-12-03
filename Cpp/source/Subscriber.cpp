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

#include <chrono>

#include "OPSTypeDefs.h"
#include "Subscriber.h"
#include "Publisher.h"
#include "TimeHelper.h"
#include "Participant.h"
#include "BasicError.h" 
#include "opsidls/OPSConstants.h"
#include "opsidls/SendAckPatternData.h"
#include "ConfigException.h"

namespace ops
{

    SubscriberBase::SubscriberBase(Topic const t) :
        topic(t),
        deadlineTimeout(TimeHelper::infinite)
    {
        participant = Participant::getInstance(topic.getDomainID(), topic.getParticipantID());
        deadlineTimer = DeadlineTimer::create(participant->getIOService());
    }

    SubscriberBase::~SubscriberBase()
    {
        // Make sure subscriber is stopped and no more notifications can call us
        stop();

        delete deadlineTimer;
    }

    void SubscriberBase::start()
    {
        if (started) { return; }

        receiveDataHandler = participant->getReceiveDataHandler(topic);
        receiveDataHandler->addListener(this, topic);
        receiveDataHandler->Notifier<ConnectStatus>::addListener(this);
        deadlineTimer->addListener(this);
        deadlineTimer->start(deadlineTimeout);
        started = true;
    }

    void SubscriberBase::stop()
    {
        if (!started) { return; }

        // Note that the receiveDataHandler messageLock is held while we are removed from its list.
        // This ensures that the receive thread can't be in our onNewEvent() or be calling us anymore
        // when we return from the removeListener() call.
        receiveDataHandler->Notifier<ConnectStatus>::removeListener(this);
        receiveDataHandler->removeListener(this, topic);
        receiveDataHandler.reset();
        participant->releaseReceiveDataHandler(topic);
        deadlineTimer->removeListener(this);
        deadlineTimer->cancel();
        started = false;
    }

    Topic SubscriberBase::getTopic() const
    {
        return topic;
    }

    bool SubscriberBase::aquireMessageLock()
    {
        return receiveDataHandler->aquireMessageLock();
    }

    void SubscriberBase::releaseMessageLock()
    {
        receiveDataHandler->releaseMessageLock();
    }

    /// =======================================================================================
    
    // Helper class that handles sending of ACK's if that is enabled in the Topic
    struct ACKFilter : public FilterQoSPolicy
    {
        explicit ACKFilter(const Topic& ackTopic)
        {
            // Create Publisher for sending REGISTER, ACK, UNREGISTER
            _pub = new Publisher(ackTopic);
        }

        ~ACKFilter()
        {
            delete _pub;
        }

        // Not used, but must be implemeted
        bool applyFilter(const OPSObject* const) override
        {
            return true;
        }

        // Called from Subscriber callback (i.e. messagelock is held)
        // Applies a filter in the receiving process in Subscribers.
        // Returning false from a filter indicates that this data sample (OPSObject)
        // shall not be propagated to the application layer.
        bool applyFilter(const OPSMessage* const mess, const OPSObject* const) override
        {
            // Send ACK with Pub ID counter
            int64_t pubId = mess->getPublicationID();
            mess->getSource((uint32_t&)_data.sourceIP, (uint16_t&)_data.sourcePort);
            _data.publicationID = pubId;
            _data.messageType = opsidls::SendAckPatternData::MType::ACK;
            _pub->writeOPSObject(&_data);

            _expectedPub[mess->getPublisherName()] = true;

            MyKey_t pubKey((uint32_t)_data.sourceIP, (uint16_t)_data.sourcePort);

            auto it = _lastReceivedPubId.find(pubKey);
            if (it != _lastReceivedPubId.end()) {
                // If the same Pub ID counter from Publisher, skip message (we have already seen it, it's a resend)
                if (it->second == pubId) { return false; }

                // Save current number
                it->second = pubId;
            } else {
                // If a new Pub ID counter, save value and give message to User
                _lastReceivedPubId[pubKey] = pubId;
            }
            _gotMessage = true;
            return true;
        }

        // Need to hold Messagelock when called to synchronize with callback/applyFilter above
        void send(opsidls::SendAckPatternData::MType req)
        {
            _data.messageType = req;
            _pub->writeOPSObject(&_data);
        }

        typedef std::pair<uint32_t, uint16_t> MyKey_t;

        Publisher* _pub{ nullptr };
        opsidls::SendAckPatternData _data;
        std::map<MyKey_t, int64_t> _lastReceivedPubId;
        std::map<ObjectName_T, bool> _expectedPub;
        bool _gotMessage{ false };
    };

    /// =======================================================================================

    Subscriber::Subscriber(Topic const t) :
        SubscriberBase(t)
	{
        timeLastData = TimeHelper::currentTimeMillis();

        if (topic.getUseAck()) {
            // Enable ACK's
            // Check that SampleMaxSize is <= 60000-14
            if (topic.getSampleMaxSize() > opsidls::OPSConstants::USABLE_SEGMENT_SIZE) {
                throw ConfigException("SampleMaxSize to big for SubscriberWithAck");
            }

            // Add a filter that will handle ACK sending and remove duplicates
            _ackFilter = new ACKFilter(Topic::CreateAckTopic(topic));
        }

#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.RegisterSub(this, topic.getName());
#endif
	}

    Subscriber::~Subscriber()
    {
#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.UnregisterSub(this, topic.getName());
#endif
		// Make sure subscriber is stopped and no more notifications can call us
        stop();

        if (_ackFilter != nullptr) {
            // Send UNREGISTER
            ((ACKFilter*)_ackFilter)->send(opsidls::SendAckPatternData::MType::UNREGISTER);
            delete _ackFilter;
        }
        
        while (messageBuffer.size() > 0) {
			messageBuffer.back()->unreserve();
			messageBuffer.pop_back();
		}
	}

    // One way for us to know which publishers to expect
    void Subscriber::AddExpectedPublisher(const ObjectName_T& pubname)
    {
        if (_ackFilter == nullptr) { return; }
        MessageLock lck(*this);
        ((ACKFilter*)_ackFilter)->_expectedPub[pubname] = false;
    }

    // nullptr check all
    bool Subscriber::CheckPublisher(const ObjectName_T& pubname)
    {
        if (_ackFilter == nullptr) { return true; }
        MessageLock lck(*this);
        if (pubname == "") {
            for (auto& x : ((ACKFilter*)_ackFilter)->_expectedPub) {
                if (x.second == false) return false;
            }
            return true;
        }
        else {
            for (auto& x : ((ACKFilter*)_ackFilter)->_expectedPub) {
                if (x.first == pubname) {
                    if (x.second == true) return true;
                }
            }
            return false;
        }
    }

    void Subscriber::RemoveExpectedPublisher(const ObjectName_T& pubname)
    {
        if (_ackFilter == nullptr) { return; }
        MessageLock lck(*this);
        ((ACKFilter*)_ackFilter)->_expectedPub.erase(pubname);
    }

    // Activate (need to be called periodically)
    ///TBD controllable if called by user or timer???
    void Subscriber::Activate()
    {
        if (_ackFilter == nullptr) { return; }

        // Send REGISTER (~1Hz)
        int64_t now = ops::TimeHelper::currentTimeMillis();
        if (_nextRegisterTime > now) { return; }

        bool sendRegister = false;

        MessageLock lck(*this);

        // Only until we got a message from all expected publishers
        for (auto& x : ((ACKFilter*)_ackFilter)->_expectedPub) {
            if (x.second == false) {
                sendRegister = true;
                break;
            }
        }
        // or if no expected until we get a message
        if (sendRegister == false) {
            sendRegister = !((ACKFilter*)_ackFilter)->_gotMessage;
        }
        if (sendRegister) {
            ((ACKFilter*)_ackFilter)->send(opsidls::SendAckPatternData::MType::REGISTER);
            _nextRegisterTime = now + topic.getRegisterTimeMs();
        }
    }

    /// ----------------------------------------------------------

    // Note that the receiveDataHandler messageLock is held while executing this method
    void Subscriber::onNewEvent(Notifier<OPSMessage*>* , OPSMessage* const message)
    {
        //Perform a number of checks on incomming data to be sure we want to deliver it to the application layer
        //Check that this message is delivered on the same topic as this Subscriber use
        if (message->getTopicName() != topic.getName())
        {
            // This is a normal case when several Topics use the same port
            return;
        }
        //Check that the type of the delivered data can be interpreted as the type we expect in this Subscriber
        else if (message->getData()->getTypeString().find(topic.getTypeID()) == TypeId_T::npos)
        {
			ErrorMessage_T msg("Received message with wrong data type for Topic: ");
			msg += topic.getName();
			msg += "\nExpected type: ";
			msg += topic.getTypeID();
			msg += "\nGot type: ";
			msg += message->getData()->getTypeString();
			BasicError err("Subscriber", "onNewEvent", msg);
            participant->reportError(&err);
            return;
        }
#ifdef OPS_ENABLE_DEBUG_HANDLER
		{
			const SafeLock lck(&_dbgLock);
			if (_dbgSkip > 0) {
				_dbgSkip--;
				return;
			}
			_numReceived++;
		}
#endif

		//OK, we passed the basic checks

        // Check if ACK should be sent and resends filtered out
        if (_ackFilter != nullptr) {
            if (_ackFilter->applyFilter(message, nullptr) == false) { return; }
        }

		//If we have a publication ID checker, call it
		if (pubIdChecker != nullptr) { pubIdChecker->Check(message); }

		//Now lets go on and filter on data content...

        OPSObject* const o = message->getData();
        if (applyFilterQoSPolicies(message, o))
        {
            if (timeBaseMinSeparationTime == 0 || TimeHelper::currentTimeMillis() - timeLastDataForTimeBase > timeBaseMinSeparationTime)
            {
                firstDataReceived = true;
                
                addToBuffer(message);
                this->message = message;
                data = o;

                // Notify all registered listeners
                notifyNewData();

                // Signal any waiting thread
                hasUnreadData = true;
                newDataEvent.signal();

				// Update deadline variables
                timeLastData = timeLastDataForTimeBase = TimeHelper::currentTimeMillis();
                deadlineMissed = false;

                deadlineTimer->start(deadlineTimeout);
            }
        }
    }

    /// ----------------------------------------------------------

    void Subscriber::addToBuffer(OPSMessage* const mess)
    {
        mess->reserve();
        messageBuffer.push_front(mess);
        while (messageBuffer.size() > messageBufferMaxSize)
        {
            messageBuffer.back()->unreserve();
            messageBuffer.pop_back();
        }
    }

    void Subscriber::setHistoryMaxSize(int const s) noexcept
    {
        messageBufferMaxSize = s;
    }

    std::deque<OPSMessage*> Subscriber::getHistory()
    {
        return messageBuffer;
    }

    /// ----------------------------------------------------------

    bool Subscriber::waitForNewData(int const timeoutMs)
    {
        if (hasUnreadData) {
            return true;
        }
        return newDataEvent.waitFor(std::chrono::milliseconds(timeoutMs));
    }

    OPSObject* Subscriber::getData() noexcept
    {
        hasUnreadData = false;
        return data;
    }

    OPSMessage* Subscriber::getMessage() noexcept
    {
        hasUnreadData = false;
        return message;
    }

    /// ----------------------------------------------------------

    ObjectName_T Subscriber::getName() const noexcept
    {
        return name;
    }

    void Subscriber::setName(ObjectName_T const name) noexcept
    {
        this->name = name;
        if (_ackFilter != nullptr) {
            ((ACKFilter*)_ackFilter)->_pub->setName(name);
        }
    }

    /// ----------------------------------------------------------

    void Subscriber::addFilterQoSPolicy(FilterQoSPolicy* const fqos)
    {
        const SafeLock lock(&filterQoSPolicyMutex);
        filterQoSPolicies.push_back(fqos);
    }

    void Subscriber::removeFilterQoSPolicy(FilterQoSPolicy* const fqos)
    {
        const SafeLock lock(&filterQoSPolicyMutex);
        filterQoSPolicies.remove(fqos);
    }

    bool Subscriber::applyFilterQoSPolicies(OPSMessage* const message, OPSObject* const obj)
    {
        const SafeLock lock(&filterQoSPolicyMutex);
        for (auto filter : filterQoSPolicies) {
            if (filter->applyFilter(message, obj) == false) {
                return false;
            }
        }
        return true;
    }

    void Subscriber::setDeadlineQoS(int64_t const millis)
    {
		if (millis == 0) {
		    deadlineTimeout = TimeHelper::infinite;
		} else {
	        deadlineTimeout = millis;
		}
		cancelDeadlineTimeouts();	// Restart with new timeout
    }

    int64_t Subscriber::getDeadlineQoS() const noexcept
    {
        return deadlineTimeout;
    }

    int64_t Subscriber::getTimeBasedFilterQoS() const noexcept
    {
        return timeBaseMinSeparationTime;
    }

    void Subscriber::setTimeBasedFilterQoS(int64_t const timeBaseMinSeparationMillis) noexcept
    {
        timeBaseMinSeparationTime = timeBaseMinSeparationMillis;
    }

    /// ----------------------------------------------------------

    void Subscriber::checkAndNotifyDeadlineMissed()
    {
        if (isDeadlineMissed())
        {
            //printf("DeadlineMissed timeLastData = %d, currTime = %d, deadlineTimeout = %d\n", timeLastData, currTime, deadlineTimeout)
            deadlineMissedEvent.notifyDeadlineMissed();
            timeLastData = TimeHelper::currentTimeMillis();
        }
    }

    bool Subscriber::isDeadlineMissed()
    {
        const int64_t currTime = TimeHelper::currentTimeMillis();
        if (currTime - timeLastData > deadlineTimeout)
        {
            deadlineMissed = true;
            return deadlineMissed;
        }
        return false;
    }

    void Subscriber::cancelDeadlineTimeouts()
    {
        deadlineTimer->start(deadlineTimeout);
    }

    void Subscriber::onNewEvent(Notifier<int>* , int )
    {
        deadlineMissedEvent.notifyDeadlineMissed();
        deadlineTimer->start(deadlineTimeout);
    }

    /// ----------------------------------------------------------

#ifdef OPS_ENABLE_DEBUG_HANDLER
	void Subscriber::onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp)
	{
		const SafeLock lck(&_dbgLock);
		switch (req.Command) {
		case 1: // Request
			break;
		case 2: // Enable
			_dbgSkip = (req.Param1 == 1) ? 0 : LLONG_MAX;
			break;
		case 3: // Filter
			///TODO
			break;
		case 4: // Skip next x receives
			_dbgSkip = req.Param1;
			break;
		default:;
			break;
		}

		// Fill in status
		resp.Enabled = (_dbgSkip == 0);
		resp.Result1 = _numReceived;
		resp.Result2 = _dbgSkip;
		resp.Result3 = false;
	}
#endif
}
