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

#include "OPSTypeDefs.h"
#include "Subscriber.h"
#include "TimeHelper.h"
#include "Participant.h"
#include "BasicError.h" 

#include <chrono>

namespace ops
{

    Subscriber::Subscriber(Topic const t) :
		topic(t),
		deadlineTimeout(TimeHelper::infinite)
	{
        participant = Participant::getInstance(topic.getDomainID(), topic.getParticipantID());
        deadlineTimer = DeadlineTimer::create(participant->getIOService());
        
        timeLastData = TimeHelper::currentTimeMillis();

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

        delete deadlineTimer;

		while (messageBuffer.size() > 0) {
			messageBuffer.back()->unreserve();
			messageBuffer.pop_back();
		}
	}

    void Subscriber::start()
    {
        if (started) { return; }

        receiveDataHandler = participant->getReceiveDataHandler(topic);
        receiveDataHandler->addListener(this, topic);
		receiveDataHandler->Notifier<ConnectStatus>::addListener(this);
        deadlineTimer->addListener(this);
        deadlineTimer->start(deadlineTimeout);
        started = true;
    }

    void Subscriber::stop()
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

    OPSObject* Subscriber::getData() noexcept
    {
        hasUnreadData = false;
        return data;
    }

    Topic Subscriber::getTopic() const
    {
        return topic;
    }

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

    void Subscriber::checkAndNotifyDeadlineMissed()
    {
        if (isDeadlineMissed())
        {
            //printf("DeadlineMissed timeLastData = %d, currTime = %d, deadlineTimeout = %d\n", timeLastData, currTime, deadlineTimeout)
            deadlineMissedEvent.notifyDeadlineMissed();
            timeLastData = TimeHelper::currentTimeMillis();
        }
    }

    int64_t Subscriber::getTimeBasedFilterQoS() const noexcept
    {
        return timeBaseMinSeparationTime;
    }

    void Subscriber::setTimeBasedFilterQoS(int64_t const timeBaseMinSeparationMillis) noexcept
    {
        timeBaseMinSeparationTime = timeBaseMinSeparationMillis;
    }

    bool Subscriber::waitForNewData(int const timeoutMs)
    {
        if (hasUnreadData) {
            return true;
        }
        return newDataEvent.waitFor(std::chrono::milliseconds(timeoutMs));
    }

	ObjectName_T Subscriber::getName() const noexcept
    {
        return name;
    }

    void Subscriber::setName(ObjectName_T const name) noexcept
    {
        this->name = name;
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

    bool Subscriber::aquireMessageLock()
    {
        return receiveDataHandler->aquireMessageLock();
    }

    void Subscriber::releaseMessageLock()
    {
        receiveDataHandler->releaseMessageLock();
    }

    OPSMessage* Subscriber::getMessage() noexcept
    {
        hasUnreadData = false;
        return message;
    }

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
