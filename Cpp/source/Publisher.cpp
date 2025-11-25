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

#include "Publisher.h"
#include "Participant.h"
#include "Domain.h"
#include "OPSArchiverOut.h"
#include "Subscriber.h"
#include "opsidls/SendAckPatternData.h"
#include "opsidls/OPSConstants.h"
#include "TimeHelper.h"
#include "DataSegmentPool.h"
#include "ConfigException.h"

namespace ops
{
    // A subscriber for ACK messages
    struct Publisher::AckSubscriber : public ops::SubscriberBase
    {
        using ops::SubscriberBase::onNewEvent;

        explicit AckSubscriber(const ops::Topic& top) :
            SubscriberBase(top)
        {
        }

#if defined(_MSC_VER) && (_MSC_VER == 1900)
        // VS2015 gives a warning on onNewEvent() "... previous versions of the compiler did not override when ...", which later versions don't do.
        // We want it to override so skip warning
#pragma warning( disable : 4373 )
#endif
        // Note that the receiveDataHandler messageLock is held while executing this method
        virtual void onNewEvent(ops::Notifier<ops::OPSMessage*>*, ops::OPSMessage* const mess) override
        {
            //Check that this message is delivered on the same topic as this Subscriber use
            if (mess->getTopicName() != topic.getName()) {
                // This is a normal case when several Topics use the same port
                return;
            }
            opsidls::SendAckPatternData* const ackData = dynamic_cast<opsidls::SendAckPatternData*>(mess->getData());
            if (ackData == nullptr) { return; }

            uint32_t IP;
            uint16_t port;
            mess->getSource(IP, port);
            const MyKey_t subKey(IP, port);

            const ObjectName_T subName = mess->getPublisherName();

            switch (ackData->messageType)
            {
            case opsidls::SendAckPatternData::MType::ACK:
                // Got message (ACK) on Subscriber
                // Check that ACK number is correct
                if ((_sourceIP == (uint32_t)ackData->sourceIP) && (_sourcePort == (uint16_t)ackData->sourcePort)) {
                    if (ackData->publicationID == _lastSentPubId) {
                        // If so, mark Subscriber as "acked"
                        _map[subKey].ack();
                        // If it's an expected subscribers, set mark
                        const auto ent = _expectedSub.find(subName);
                        if (ent != _expectedSub.end()) {
                            _expectedSub[subName].ack();
                        }
                        OPS_ACK_TRACE(topic.getName() << ": CORRECT ACK from subscriber " << subName << "\n");
                    } else {
                        OPS_ACK_TRACE(topic.getName() << ": ACK on wrong number from subscriber " << subName << "\n");
                    }
                } else {
                    //OPS_ACK_TRACE(topic.getName() << ": Got an ACK on data from another publisher. Skip.\n");
                }
                break;

            case opsidls::SendAckPatternData::MType::REGISTER:
                // Got message (REGISTER) on Subscriber
                // Add publisher to expected ACK senders (and publish latest data if any)
                OPS_ACK_TRACE(topic.getName() << ": REGISTER from subscriber " << subName << "\n");
                if (_map.find(subKey) == _map.end()) {
                    // Only set false if it's a new one, we don't want to clear an ACK mark
                    _map[subKey].acked = false;
                }
                if (_map[subKey].acked == false) {
                    if (_owner != nullptr) {
                        _owner->resendLatest();
                    }
                }
                break;

            case opsidls::SendAckPatternData::MType::UNREGISTER:
                // Got message (UNREGISTER) on Subscriber
                // Remove publisher from expected ACK senders
                OPS_ACK_TRACE(topic.getName() << ": UNREGISTER from subscriber " << subName << "\n");
                _map.erase(subKey);
                break;

            default:
                break;
            }
        }

        // Callback for deadline timeouts, Needed but not used
        virtual void onNewEvent(ops::Notifier<int>*, int) override {}

        typedef std::pair<uint32_t, uint16_t> MyKey_t;
        struct entry_t {
            bool valid{ true };
            bool acked{ false };
            int32_t failed{ 0 };
            void ack() { acked = true; }
        };
        std::map<ObjectName_T, entry_t> _expectedSub;
        std::map<MyKey_t, entry_t> _map;
        int64_t _lastSentPubId{ -1 };
        Publisher* _owner{ nullptr };
        uint32_t _sourceIP{ 0 };
        uint16_t _sourcePort{ 0 };
    };

    // ==============================================================================

    using namespace opsidls;

    Publisher::Publisher(Topic t) :
        topic(t),
        memMap(1 + ((t.getSampleMaxSize() - 1) / OPSConstants::USABLE_SEGMENT_SIZE),
            (t.getSampleMaxSize() >= OPSConstants::USABLE_SEGMENT_SIZE) ? OPSConstants::PACKET_MAX_SIZE : t.getSampleMaxSize() + OPSConstants::SEGMENT_HEADER_SIZE,
            &DataSegmentAllocator::Instance()),
        buf(memMap),
        _ackTimeoutInc(t.getResendTimeMs() < 0 ? 0 : t.getResendTimeMs())
    {
        participant = Participant::getInstance(topic.getDomainID(), topic.getParticipantID());
        valStrat = participant->valPubStrat;
        valCall = participant->valPubCall;

        sendDataHandler = participant->getSendDataHandler(topic);
        if (!sendDataHandler) {
            throw ConfigException(ExceptionMessage_T("Failed to create sender for topic ") + ExceptionMessage_T(topic.getName()));
        }

		message.setKey("");
        message.setPublisherName(m_name);
        message.setTopicName(topic.getName());
        message.setDataOwner(false);
		
		start();

		// If we let the OS define the port, the transport info isn't available until after start()
		sendDataHandler->updateTransportInfo(topic);
		participant->updateSendPartInfo(topic);

        useInProc = (topic.getTransport() == Topic::TRANSPORT_INPROC);

        if (topic.getUseAck() && (!useInProc)) {
            // Enable ACK's
            // Check that SampleMaxSize is <= 60000-14
            if (topic.getSampleMaxSize() > OPSConstants::USABLE_SEGMENT_SIZE) {
                throw ConfigException("SampleMaxSize to big for Publisher With Ack");
            }

            // Create Subscriber for ACK's
            _ackSub = std::unique_ptr<AckSubscriber>(new AckSubscriber(Topic::CreateAckTopic(topic)));
            _ackSub->_sourceIP = sendDataHandler->getLocalAddressHost();
            _ackSub->_sourcePort = sendDataHandler->getLocalPort();
            _ackSub->start();
        }

#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.RegisterPub(this, topic.getName());
#endif
	}

    Publisher::~Publisher()
    {
		OPS_DES_TRACE("Pub: Destructor()...\n");
#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.UnregisterPub(this, topic.getName());
		for (unsigned int i = 0; i < _replace.size(); i++) {
			if (_replace[i] != nullptr) { delete _replace[i]; }
		}
#endif
		stop();
        _ackSub.reset();
		participant->releaseSendDataHandler(topic);
		OPS_DES_TRACE("Pub: Destructor() finished\n");
	}

	// Validation
    void Publisher::defineValidation(ValidationStrategy strat, ValidationPubCallback cb)
    {
        if (strat == ValidationStrategy::Default) {
            valStrat = participant->valPubStrat;
            valCall = participant->valPubCall;
        } else {
            valStrat = strat;
            valCall = cb;
        }
    }

    void Publisher::start()
	{
        if (!started) {
            sendDataHandler->addListener(this);
            sendDataHandler->addPublisher(this, topic);
            started = true;
        }
	}

	void Publisher::stop()
	{
        if (started) {
            sendDataHandler->removeListener(this);
            sendDataHandler->removePublisher(this, topic);
            started = false;
        }
    }

    Topic Publisher::getTopic() const
    {
        return topic;
    }

    void Publisher::setName(ObjectName_T const name)
    {
        m_name = name;
		message.setPublisherName(name);
	}

    void Publisher::setKey(ObjectKey_T const key) noexcept
    {
        m_key = key;
    }

    ObjectKey_T Publisher::getKey() const noexcept
    {
        return m_key;
    }

	ObjectName_T Publisher::getName() const noexcept
    {
        return m_name;
    }

    bool Publisher::writeOPSObject(OPSObject* const obj)
    {
        return write(obj);
    }

	bool Publisher::write(OPSObject* const data)
	{
#ifdef OPS_ENABLE_DEBUG_HANDLER
        bool sendOK = true;
        const SafeLock lck(_dbgLock);
		if (_dbgSkip > 0) {
			_dbgSkip--;
		} else {
			if (_replace.size() > 0) {
				sendOK = internalWrite(_replace[_replace.size()-1]);
				delete _replace[_replace.size() - 1];
				_replace.pop_back();
			} else {
				sendOK = internalWrite(data);
			}
		}
        return sendOK;
	}

	bool Publisher::internalWrite(OPSObject* const data)
	{
#endif
        if (m_key != "") {
            data->setKey(m_key);
        }
        if (valStrat != ValidationStrategy::None) {
            if (!data->isValid()) {
                ValidationAction act = ValidationAction::Pass;
                if (valStrat == ValidationStrategy::Callback) {
                    act = valCall(this, data);
                    if (act == ValidationAction::Skip) { return false; }
                }
                if ((valStrat == ValidationStrategy::Exception) || (act == ValidationAction::Throw)) {
                    throw ValidationFailed("Message validation failed");
                }
            }
        }

        if (_ackSub != nullptr) {
            // Clear ACK flag for each expected ACK sender, but not failed counter
            const ops::MessageLock lck(*_ackSub);
            for (auto& element : _ackSub->_map) {
                element.second.acked = false;
            }
            for (auto& element : _ackSub->_expectedSub) {
                element.second.acked = false;
            }
            _ackSub->_owner = this;
            _ackSub->_lastSentPubId = currentPublicationID;   // Will be sent with this number below
            _sendState = SendState::sending;
        } else {
            _sendState = SendState::acked;
        }

        const SafeLock lck(_pubLock);
        _resendsLeft = topic.getNumResends();

        message.setData(data);
        message.setPublicationID(currentPublicationID);
        currentPublicationID++;

        if (useInProc) {
            return sendDataHandler->sendMessage(topic, message);

        } else {
            // Serialize message
            buf.Reset();

            buf.writeNewSegment();

            OPSArchiverOut archive(buf, topic.getOptNonVirt());

            archive.inout("message", &message);

            //If data has spare bytes, write them to the end of the buf
            if (message.getData()->spareBytes.size() > 0) {
                buf.WriteChars(&(message.getData()->spareBytes[0]), (int)message.getData()->spareBytes.size());
            }

            buf.finish();

            // Send serialized message
            return writeSerializedBuffer();
        }
    }

    bool Publisher::writeSerializedBuffer()
    {
        bool sendOK = true;
        for (int i = 0; i < buf.getNrOfSegments(); i++) {
            const int segSize = buf.getSegmentSize(i);
            // We need to continue even if a send fails, since it may work to some destinations
            sendOK &= sendDataHandler->sendData(buf.getSegment(i), segSize, topic);
            // Check if we should back off for a while to distribute large messages over time
            if ((i > 0) && (i % sleepEverySendPacket == 0)) {
                TimeHelper::sleep(sendSleepTime);
            }
        }
        // Since time_point can't be stored in an atomic, we use a duration from the epoch start
        _ackTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(ops_clock::now().time_since_epoch()) + _ackTimeoutInc;
        return sendOK;
    }

    bool Publisher::resendLatest()
    {
        if (_ackSub == nullptr) { return true; }
        if (_ackSub->_lastSentPubId < 0) { return true; }
        const SafeLock lck(_pubLock);
        return writeSerializedBuffer();
    }

    // One way for us to know which ACK's to expect
    void Publisher::AddExpectedAckSender(const ObjectName_T& subname)
    {
        if (_ackSub == nullptr) { return; }
        // Add Subscriber to expected ACK senders
        const MessageLock lck(*_ackSub);
        _ackSub->_expectedSub[subname].acked = false;
        _ackSub->_expectedSub[subname].valid = true;
    }

    // "" check all
    bool Publisher::CheckAckSender(const ObjectName_T& subname)
    {
        if (_ackSub == nullptr) { return true; }
        const MessageLock lck(*_ackSub);
        if (subname == "") {
            for (const auto& x : _ackSub->_expectedSub) {
                if (!x.second.acked && x.second.valid) { return false; }
            }
            return true;
        } else {
            for (const auto& x : _ackSub->_expectedSub) {
                if ((x.first == subname) && x.second.valid) {
                    if (x.second.acked) { return true; }
                }
            }
            return false;
        }
    }

    void Publisher::RemoveExpectedAckSender(const ObjectName_T& subname)
    {
        if (_ackSub == nullptr) { return; }
        // Remove Subscriber from expected ACK senders
        const MessageLock lck(*_ackSub);
        _ackSub->_expectedSub.erase(subname);
    }

    void Publisher::Activate()
    {
        Activate([](const ObjectName_T&, int32_t) { return ResendAlternative_T::yes; });
    }

    // Need to be called periodically
    void Publisher::Activate(ShouldResendFunc_T const shouldResend)
    {
        if (_ackSub == nullptr) { return; }

        //If we haven't sent anything yet or all is ACKED/no more resends, return
        if (_resendsLeft < 0) { return; }

        if (_ackTimeout.load() > std::chrono::duration_cast<std::chrono::milliseconds>(ops_clock::now().time_since_epoch())) { return; }

        // If message isn't ACK'ed from all expected ACK senders, resend message with same Pub Id Counter
        bool resend = false;
        {
            const MessageLock lck(*_ackSub);
            // Check if any of the expected hasn't responded to us
            for (auto& element : _ackSub->_expectedSub) {
                AckSubscriber::entry_t& ent = element.second;
                if (!ent.acked && ent.valid) {
                    ++ent.failed;
                    // Verify with owner that 'element.first' is still an expected AckSender.
                    switch (shouldResend(element.first, ent.failed)) {
                    case ResendAlternative_T::no:
                        break;
                    case ResendAlternative_T::no_remove:
                        ent.valid = false;
                        break;
                    case ResendAlternative_T::yes:
                        resend = true;
                        break;
                    default:
                        break;
                    }
                }
            }
            // Also check all that has registered with us
            for (auto& element : _ackSub->_map) {
                AckSubscriber::entry_t& ent = element.second;
                if (!ent.acked) {
                    ++ent.failed;
                    resend = true;
                }
            }
        }
        if (resend) {
            if (_resendsLeft == 0) {
                // Failed to get ACK from all, within number of configured ACK's
                _sendState = SendState::failed;
                OPS_ACK_TRACE("ACK FAILED (no more resends)\n");
            } else {
                resendLatest();
                OPS_ACK_TRACE("RESEND\n");
            }
            --_resendsLeft;
        } else {
            _sendState = SendState::acked;
            _resendsLeft = -1;
            OPS_ACK_TRACE("DATA FULLY ACKED (no more resends)\n");
        }
    }

#ifdef OPS_ENABLE_DEBUG_HANDLER
	void Publisher::onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp)
	{
		const SafeLock lck(_dbgLock);
		switch (req.Command) {
		case 1: // Request status
			break;
		case 2: // Enable
			_dbgSkip = (req.Param1 == 1) ? 0 : LLONG_MAX;
			break;
		case 3: // PubId
			currentPublicationID += req.Param1;
			break;
		case 4: // Skip next x sends
			_dbgSkip = req.Param1;
			break;
		case 5: // Send directly
            if (req.Objs.size() == 0) { break; }
			if (req.Objs[0] != nullptr) { internalWrite(req.Objs[0]); }
			break;
		case 6: // Send instead of next x ordinary
			// Free ev. objects stored since earlier 
			for (unsigned int i = 0; i < _replace.size(); i++) {
				if (_replace[i] != nullptr) { delete _replace[i]; }
			}
			_replace.clear();
			// Store backwards in _replace vector
			for (auto i = req.Objs.size(); i > 0; i--) {
				_replace.push_back(req.Objs[i-1]);
			}
			// We have now taken over ownership
			req.Objs.clear();
			break;
		default:;
			break;
		}

		// Fill in status
		resp.Enabled = _dbgSkip == 0;
		resp.Result1 = currentPublicationID;
		resp.Result2 = _dbgSkip;
		resp.Result3 = (_replace.size() > 0);
	}
#endif
}
