/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2021 Lennart Andersson.
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
        topic(t)
    {
        participant = Participant::getInstance(topic.getDomainID(), topic.getParticipantID());
    }

    SubscriberBase::~SubscriberBase()
    {
        // Make sure subscriber is stopped and no more notifications can call us
        stop();
    }

    void SubscriberBase::setupDeadlineTimer()
    {
        if (deadlineTimer == nullptr) {
            deadlineTimer = std::unique_ptr<DeadlineTimer>(DeadlineTimer::create(participant->getIOService()));
            deadlineTimer->addListener(this);
        }
    }

    void SubscriberBase::start()
    {
        if (m_started) { return; }

        receiveDataHandler = participant->getReceiveDataHandler(topic);
        receiveDataHandler->addListener(this, topic);
        receiveDataHandler->Notifier<ConnectStatus>::addListener(this);
        if ((deadlineTimer != nullptr) && deadlineUsed) {
            deadlineTimer->start(deadlineTimeout);
        }
        m_started = true;
    }

    void SubscriberBase::stop()
    {
        if (!m_started) { return; }

        // Note that the receiveDataHandler messageLock is held while we are removed from its list.
        // This ensures that the receive thread can't be in our onNewEvent() or be calling us anymore
        // when we return from the removeListener() call.
        receiveDataHandler->Notifier<ConnectStatus>::removeListener(this);
        receiveDataHandler->removeListener(this, topic);
        receiveDataHandler.reset();
        participant->releaseReceiveDataHandler(topic);
        if (deadlineTimer != nullptr) {
            deadlineTimer->cancel();
        }
        m_started = false;
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

    // =======================================================================================
    
    /// Helper class to the Subscriber for handling of sending ACK's if that is enabled in the Topic
    struct Subscriber::ACKFilter : public FilterQoSPolicy
    {
        explicit ACKFilter(const Topic& ackTopic) :
            _pub(new Publisher(ackTopic))   // Create Publisher for sending REGISTER, ACK, UNREGISTER
        {
        }

        // Not used, but must be implemeted
        virtual bool applyFilter(const OPSObject* const) override
        {
            return true;
        }

        // Called from Subscriber callback (i.e. messagelock is held)
        // Applies a filter in the receiving process in Subscribers.
        // Returning false from a filter indicates that this data sample (OPSObject)
        // shall not be propagated to the application layer.
        virtual bool applyFilter(const OPSMessage* const mess, const OPSObject* const) override
        {
            // Send ACK with Pub ID counter
            const int64_t pubId = mess->getPublicationID();
            mess->getSource((uint32_t&)_data.sourceIP, (uint16_t&)_data.sourcePort);
            _data.publicationID = pubId;
            _data.messageType = opsidls::SendAckPatternData::MType::ACK;
            _pub->writeOPSObject(&_data);

            _expectedPub[mess->getPublisherName()] = true;

            const MyKey_t pubKey((uint32_t)_data.sourceIP, (uint16_t)_data.sourcePort);

            const auto it = _lastReceivedPubId.find(pubKey);
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
        void send(const opsidls::SendAckPatternData::MType req)
        {
            _data.messageType = req;
            _pub->writeOPSObject(&_data);
        }

        typedef std::pair<uint32_t, uint16_t> MyKey_t;

        std::unique_ptr<Publisher> _pub;
        opsidls::SendAckPatternData _data;
        std::map<MyKey_t, int64_t> _lastReceivedPubId;
        std::map<ObjectName_T, bool> _expectedPub;
        bool _gotMessage{ false };
    };

    /// =======================================================================================

    Subscriber::Subscriber(Topic const t) :
        SubscriberBase(t)
	{
        timeLastData = ops_clock::now();

        if (topic.getUseAck()) {
            // Enable ACK's
            // Check that SampleMaxSize is <= 60000-14
            if (topic.getSampleMaxSize() > opsidls::OPSConstants::USABLE_SEGMENT_SIZE) {
                throw ConfigException("SampleMaxSize to big for SubscriberWithAck");
            }

            // Add a filter that will handle ACK sending and remove duplicates
            _ackFilter = std::unique_ptr<ACKFilter>(new ACKFilter(Topic::CreateAckTopic(topic)));
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
            _ackFilter->send(opsidls::SendAckPatternData::MType::UNREGISTER);
            _ackFilter.reset();
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
        const MessageLock lck(*this);
        _ackFilter->_expectedPub[pubname] = false;
    }

    // nullptr check all
    bool Subscriber::CheckPublisher(const ObjectName_T& pubname)
    {
        if (_ackFilter == nullptr) { return true; }
        const MessageLock lck(*this);
        if (pubname == "") {
            for (const auto& x : _ackFilter->_expectedPub) {
                if (x.second == false) { return false; }
            }
            return true;
        }
        else {
            for (const auto& x : _ackFilter->_expectedPub) {
                if (x.first == pubname) {
                    if (x.second == true) { return true; }
                }
            }
            return false;
        }
    }

    void Subscriber::RemoveExpectedPublisher(const ObjectName_T& pubname)
    {
        if (_ackFilter == nullptr) { return; }
        const MessageLock lck(*this);
        _ackFilter->_expectedPub.erase(pubname);
    }

    // Activate (need to be called periodically)
    ///TBD controllable if called by user or timer???
    void Subscriber::Activate()
    {
        if ((_ackFilter == nullptr) || (!isStarted())) { return; }

        // Send REGISTER (~1Hz)
        const ops_clock::time_point now = ops_clock::now();
        if (_nextRegisterTime > now) { return; }

        bool sendRegister = false;

        const MessageLock lck(*this);

        // Only until we got a message from all expected publishers
        for (const auto& x : _ackFilter->_expectedPub) {
            if (x.second == false) {
                sendRegister = true;
                break;
            }
        }
        // or if no expected until we get a message
        if (sendRegister == false) {
            sendRegister = !_ackFilter->_gotMessage;
        }
        if (sendRegister) {
            _ackFilter->send(opsidls::SendAckPatternData::MType::REGISTER);
            _nextRegisterTime = now + std::chrono::milliseconds(topic.getRegisterTimeMs());
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
			const SafeLock lck(_dbgLock);
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
            if ((!minSeparationUsed) || ((ops_clock::now() - timeLastDataForTimeBase) > timeBaseMinSeparationTime)) {
                firstDataReceived = true;
                
                addToBuffer(message);
                m_message = message;
                data = o;

                // Notify all registered listeners
                notifyNewData();

                // Signal any waiting thread
                hasUnreadData = true;
                newDataEvent.signal();

				// Update deadline variables
                timeLastData = timeLastDataForTimeBase = ops_clock::now();

                if (deadlineUsed) {
                    deadlineTimer->start(deadlineTimeout);
                }
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

    bool Subscriber::waitForNewData(const std::chrono::milliseconds& timeout)
    {
        if (hasUnreadData) {
            return true;
        }
        return newDataEvent.waitFor(timeout);
    }

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
        return m_message;
    }

    /// ----------------------------------------------------------

    ObjectName_T Subscriber::getName() const noexcept
    {
        return m_name;
    }

    void Subscriber::setName(ObjectName_T const name) noexcept
    {
        m_name = name;
        if (_ackFilter != nullptr) {
            _ackFilter->_pub->setName(name);
        }
    }

    /// ----------------------------------------------------------

    void Subscriber::addFilterQoSPolicy(FilterQoSPolicy* const fqos)
    {
        const SafeLock lock(filterQoSPolicyMutex);
        filterQoSPolicies.push_back(fqos);
    }

    void Subscriber::removeFilterQoSPolicy(FilterQoSPolicy* const fqos)
    {
        const SafeLock lock(filterQoSPolicyMutex);
        filterQoSPolicies.remove(fqos);
    }

    bool Subscriber::applyFilterQoSPolicies(const OPSMessage* const message, const OPSObject* const obj)
    {
        const SafeLock lock(filterQoSPolicyMutex);
        for (const auto filter : filterQoSPolicies) {
            if (filter->applyFilter(message, obj) == false) {
                return false;
            }
        }
        return true;
    }

    void Subscriber::setDeadlineQoS(int64_t const millis)
    {
        setDeadline(std::chrono::milliseconds(millis));
    }

    void Subscriber::setDeadline(const std::chrono::milliseconds& millis)
    {
        if (millis <= std::chrono::milliseconds(0)) {
            deadlineTimeout = std::chrono::milliseconds(0);
            deadlineUsed = false;
            if (deadlineTimer != nullptr) {
                deadlineTimer->cancel();
            }
        } else {
            if (millis > std::chrono::milliseconds(TimeHelper::infinite)) {
                deadlineTimeout = std::chrono::milliseconds(TimeHelper::infinite);
            } else {
                deadlineTimeout = millis;
            }
            deadlineUsed = true;
            setupDeadlineTimer();
            if (deadlineTimer != nullptr) {
                deadlineTimer->start(deadlineTimeout);	// Restart with new timeout
            }
        }
    }

    int64_t Subscriber::getDeadlineQoS() const noexcept
    {
        return deadlineTimeout.count();
    }

    std::chrono::milliseconds Subscriber::getDeadline() const noexcept
    {
        return deadlineTimeout;
    }

    int64_t Subscriber::getTimeBasedFilterQoS() const noexcept
    {
        return timeBaseMinSeparationTime.count();
    }

    void Subscriber::setTimeBasedFilterQoS(int64_t const timeBaseMinSeparationMillis) noexcept
    {
        timeBaseMinSeparationTime = std::chrono::milliseconds(timeBaseMinSeparationMillis);
        minSeparationUsed = timeBaseMinSeparationTime != std::chrono::milliseconds(0);
    }

    void Subscriber::setTimeBasedFilter(const std::chrono::milliseconds& minSeparation) noexcept
    {
        timeBaseMinSeparationTime = minSeparation;
        minSeparationUsed = timeBaseMinSeparationTime != std::chrono::milliseconds(0);
    }

    std::chrono::milliseconds Subscriber::getTimeBasedFilter() const noexcept
    {
        return timeBaseMinSeparationTime;
    }

    /// ----------------------------------------------------------

    void Subscriber::checkAndNotifyDeadlineMissed()
    {
        if (isDeadlineMissed()) {
            deadlineMissedEvent.notifyDeadlineMissed();
            timeLastData = ops_clock::now();
        }
    }

    bool Subscriber::isDeadlineMissed() noexcept
    {
        const ops_clock::time_point currTime = ops_clock::now();
        if (currTime - timeLastData > deadlineTimeout) {
            return true;
        }
        return false;
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
		const SafeLock lck(_dbgLock);
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
