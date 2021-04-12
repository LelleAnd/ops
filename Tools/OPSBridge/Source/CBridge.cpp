/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

#include <iostream>
#include <inttypes.h>

#include "SEHException.h"

#include <Subscriber.h>
#include <OPSUtilities.h>

#include "BridgeTypeFactory.h"				// Special hand-coded

#include "BridgeLog.h"
#include "BridgeConfig.h"
#include "CBridge.h"
#include "BridgeMessages.h"

namespace opsbridge {

	using namespace ops;

CBridge::CBridge(std::string name, int64_t const maxBufferSize, int64_t const iMinPubTime, CTransport* const transport) :
	m_numRecvMess(0), m_numSentMess(0), m_myQueued(0), m_otherQueued(0), m_bytesBuffered(0),
	m_myName(name), m_maxBufferSize(maxBufferSize),
	m_minPubTime(iMinPubTime),
	m_isPausedChanged(false), m_ResendSend(false), m_TrigSend(false),
	m_transport(transport), m_raw(this),
	m_isConnected(false), m_skipMessages(false), m_isPaused(false), m_isEpPaused(false), 
	m_AckCounter(0),
	m_MaxLimitReached(false)
{
	m_savedMess.mess = nullptr;
	m_savedMess.topicName = "";
	m_savedMess.destTopicName = "";
	m_savedMess.priority = 0;
	m_savedMess.AckNumber = 0;

	m_transport->SetListener(this);
}

CBridge::~CBridge()
{
	TerminateAndWaitFor();		// Kill our own thread

	m_transport->Terminate();

	removeAllSubscribers();

	clearQueuedMessages();

	m_transport->WaitFor();
	delete m_transport;
}

// =================================================================================
void CBridge::setupSubscriber(ops::Topic& recTopic, BridgeConfig::TTopicConfig& tc)
{
	ops::Subscriber* const sub = new ops::Subscriber(recTopic);
	sub->addDataListener(this);

	// Time based filter
	if (tc.iMinTime_ms > 0) {
		sub->setTimeBasedFilter(std::chrono::milliseconds(tc.iMinTime_ms));
	}

	// Key filter
	if (tc.sKey != "") {
		sub->addFilterQoSPolicy(new ops::KeyFilterQoSPolicy(tc.sKey));
	}

	ops::SafeLock const lock(m_subscriberLock);
	TSubscriberEntry ent;
	ent.tc = tc;
	ent.sub = sub;
	ent.domainName = recTopic.getDomainID();
	ent.topicName = recTopic.getName();
	ent.dataType = recTopic.getTypeID();		// Original type
	ent.numMessages = 0;
	ent.numSkippedMessages = 0;
	ent.stopCounter = 0;
	ent.started = true;
	m_subscribers.push_back(ent);

	// Now that everything is setup, we can start the subscription
	sub->start();
}

void CBridge::stopSubscriber(ops::ObjectName_T const topic)
{
	ops::ObjectName_T domainName, topicName;
	ops::utilities::splitTopicName(topic, domainName, topicName);

	ops::SafeLock const lock(m_subscriberLock);

	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		if ((m_subscribers[i].domainName == domainName) && (m_subscribers[i].topicName == topicName)) {
			if (m_subscribers[i].started) {
				m_subscribers[i].sub->stop();
				m_subscribers[i].started = false;
			}
			m_subscribers[i].stopCounter++;
		}
	}
}

void CBridge::stopAllSubscribers()
{
	ops::SafeLock const lock(m_subscriberLock);
	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		if (m_subscribers[i].started) {
			m_subscribers[i].sub->stop();
			m_subscribers[i].started = false;
		}
		m_subscribers[i].stopCounter++;
	}
}

void CBridge::startSubscriber(ops::ObjectName_T const topic)
{
	ops::ObjectName_T domainName, topicName;
	ops::utilities::splitTopicName(topic, domainName, topicName);

	ops::SafeLock const lock(m_subscriberLock);

	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		if ((m_subscribers[i].domainName == domainName) && (m_subscribers[i].topicName == topicName)) {
			if (m_subscribers[i].stopCounter > 0) { m_subscribers[i].stopCounter--; }
			if (!m_subscribers[i].started && (m_subscribers[i].stopCounter == 0)) {
				m_subscribers[i].sub->start();
				m_subscribers[i].started = true;
			}
		}
	}
}

void CBridge::startAllSubscribers()
{
	ops::SafeLock const lock(m_subscriberLock);
	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		if (m_subscribers[i].stopCounter > 0) { m_subscribers[i].stopCounter--; }
		if (!m_subscribers[i].started && (m_subscribers[i].stopCounter == 0)) {
			m_subscribers[i].sub->start();
			m_subscribers[i].started = true;
		}
	}
}

void CBridge::removeAllSubscribers()
{
	ops::SafeLock const lock(m_subscriberLock);

	// Debug
	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		BL_TRACE("# [ Bridge (%s) ] %s::%s  %u saved messages, %u skipped messages\n",
			m_myName.c_str(),
			m_subscribers[i].domainName.c_str(), 
			m_subscribers[i].topicName.c_str(), 
			m_subscribers[i].numMessages,
			m_subscribers[i].numSkippedMessages);
	}

	for (std::vector<TSubscriberEntry>::size_type i = 0; i < m_subscribers.size(); i++) {
		// Stop subscriber first. This is synchronized so that the BOOST thread can't 
		// be in onNewData() for this subscriber. But it can be for other subscribers.
		if (m_subscribers[i].started) { m_subscribers[i].sub->stop(); }

		// Therfore we don't delete the entry, we just set it to nullptr since we don't
		// want to delete indexes in case onNewData() loops on the list.
		Subscriber* const sub = m_subscribers[i].sub;
		m_subscribers[i].sub = nullptr;

		// Now we can delete the subscriber
		delete sub;
	}

	// Now that all subscribers are deleted, we can also clear the list
	m_subscribers.clear();
}

// =================================================================================
// Method used for temporarily skipping of messages
// Used if the application starts to "eat" memory
void CBridge::setSkipMessage(bool const flag)
{
	m_skipMessages = flag;
}

// =================================================================================
// Override from ops::DataListener, called whenever new data arrives.
// NOTE called from the reader thread (in OPS (actually BOOST))
void CBridge::onNewData(ops::DataNotifier* const subscriber) 
{
	try {
		// Find subscriber in list
		int idx = -1;
		for (std::vector<TSubscriberEntry>::size_type i = 0; i < m_subscribers.size(); i++) {
			if (subscriber == m_subscribers[i].sub) {
				idx = (int)i;
				break;
			}
		}
		if (idx < 0) { return; }
		if (m_subscribers[idx].sub == nullptr) { return; }		// In case removed above

		// Check if we should skip message 
		if (m_skipMessages) {
			m_subscribers[idx].numSkippedMessages++;	// Debug
			return;
		}

		// Get message 
		ops::OPSMessage* const mess =	m_subscribers[idx].sub->getMessage();
		m_subscribers[idx].numMessages++;	// Debug

		// Queue message for sending to other endpoint, use priority
		queueMessage(mess, m_subscribers[idx]);

		// Make sure there is a send "active"
		m_TrigSend = true;
		Notify();
	}
#ifdef _WIN32
	catch (utils::SEHException E) {
		BL_ERROR("### Exception in CBridge::onNewData() %s (0x%x)\n", E.what(), E.GetExceptionID());
	}
#endif
	catch (...) {
		BL_ERROR("### Exception in CBridge::onNewData()\n");
	}
}

// =================================================================================
// NOTE called from the reader thread (in OPS (actually BOOST))
void CBridge::queueMessage(ops::OPSMessage* const mess, TSubscriberEntry& subEntry)
{
	BridgeConfig::THandlingType const ht = (m_isConnected) ? subEntry.tc.eConnectedHandling : subEntry.tc.eDisconnectedHandling;

	if (ht == BridgeConfig::discard) { return; }

///TODO Check buffersize and decide if that should alter the handling type
	if ((uint64_t)m_bytesBuffered > m_maxBufferSize) {
		if (!m_MaxLimitReached) { 	
			BL_ERROR("# [ Bridge (%s) ] MAX BUFFER LIMIT REACHED !!!\n", m_myName.c_str());
			m_MaxLimitReached = true;
		}

		///TODO Kanske skall vi ta bort de äldsta av samma topic tills utrymme finns?
		/// Eller ta bort de äldsta av lägsta prioritet?
		/// Eller annat sätt?
	}
	if (m_MaxLimitReached && ((uint64_t)m_bytesBuffered < ((m_maxBufferSize * 9) / 10)) ) {
		BL_INFO("# [ Bridge (%s) ] Below 90%% of MAX BUFFER LIMIT\n", m_myName.c_str());
		m_MaxLimitReached = false;
	}
	if (m_MaxLimitReached) {
		// For now we skip messages until room
		subEntry.numSkippedMessages++;			// Debug
		return;
	}

	if (ht == BridgeConfig::keepLatest) {
		// There should only be one of this type, so remove any found samples.
		// We will add a sample at the end of the method
		ops::SafeLock const lock(m_queueLock);
		std::deque<TMessageEntry>* const Ptr = &m_queue[subEntry.tc.iPriority];

		// Algorithm: Find the last entry for topic and clear it.
		//            Draw back is that it may be starved and not sent at all.
		//            But it preserves the order of messages

		ops::ObjectName_T const topicName = subEntry.tc.sTopicName;
		std::deque<TMessageEntry>::reverse_iterator riter;
		for (riter = Ptr->rbegin(); riter != Ptr->rend(); riter++) {
			if ((*riter).topicName == topicName) {
				if ((*riter).mess != nullptr) {
					// Calculate approx. buffered size
					uint32_t const bytes = (uint32_t)(*riter).mess->getData()->spareBytes.size();
					m_bytesBuffered -= bytes;
					(*riter).mess->unreserve();
				}
				(*riter).mess = nullptr;
				break;
			}
		}
	}

	// We should now keep the new message
	// Mark message to be saved, so OPS doesn't delete it when we return
	mess->reserve();

	// Calculate approx. buffered size
	uint32_t const bytes = (uint32_t)mess->getData()->spareBytes.size();
	m_bytesBuffered += bytes;

	// Queue it, sorted on priority
	TMessageEntry me;
	me.mess = mess;
	me.topicName = subEntry.tc.sTopicName;
	me.destTopicName = subEntry.tc.sDestTopicName;	// Used as destination topic on the other side
	me.priority = subEntry.tc.iPriority;			// Priority may be needed at disconnect
	me.AckNumber = 0;								// Initialized to NOT SET

	ops::SafeLock const lock(m_queueLock);
	m_queue[subEntry.tc.iPriority].push_back(me);
}

bool CBridge::getHighestPrioMessage(TMessageEntry& me)
{
	ops::SafeLock const lock(m_queueLock);

	for (int prio = HIGHEST_PRIO; prio >= 0; prio--) {
		while (m_queue[prio].size() > 0) {
			me = m_queue[prio].front();
			m_queue[prio].pop_front();
			if (me.mess != nullptr) { return true; }
		}
	}
	me.mess = nullptr;
	me.topicName = "";
	me.destTopicName = "";
	me.priority = 0;
	me.AckNumber = 0;
	return false;
}

void CBridge::clearQueuedMessages()
{
	ops::SafeLock const lock(m_queueLock);

	for (int prio = HIGHEST_PRIO; prio >= 0; prio--) {
		while (m_queue[prio].size() > 0) {
			TMessageEntry const me = m_queue[prio].front();
			m_queue[prio].pop_front();
			if (me.mess != nullptr) { me.mess->unreserve(); }
		}
	}
	m_myQueued = 0;
	m_bytesBuffered = 0;
}

// =================================================================================
void CBridge::Pause()
{
	m_isPaused = true;
	m_isPausedChanged = true;
	Notify();
}

void CBridge::Continue()
{
	m_isPaused = false;
	m_isPausedChanged = true;
	Notify();
}

// =================================================================================
// Update time used as heartbeat check
// NOTE Called from transport thread
void CBridge::UpdateReceiveTime()
{
	ops::SafeLock const lock(m_timeLock);
	m_recvTime = ops::ops_clock::now();
}

// NOTE Called from transport thread
void CBridge::SendPreparePublishers()
{
	// Subscriberlist contains all destination topics and datatypes
	ops::SafeLock const lock(m_subscriberLock);
	TCommandMessage cmd;
	cmd.Command = ctPreparePub;
	cmd.AckCounter = m_AckCounter;

	for (uint32_t i = 0; i < m_subscribers.size(); i++) {
		cmd.DestTopicName = m_subscribers[i].tc.sDestTopicName;
		cmd.DataType = m_subscribers[i].dataType;
		BL_TRACE("# [ CBridge (%s) ] Send Prepare Pub: %s\n", m_myName.c_str(), cmd.DestTopicName.c_str());
		m_transport->writeCommand(cmd);
	}
}

// NOTE Called from transport thread
void CBridge::onConnect(CTransport* const sender)
{
    UNUSED(sender);
	BL_INFO("# [ CBridge (%s) ] CONNECTED...\n", m_myName.c_str());

	// Update time so we don't disconnect before data has started to arrive
	UpdateReceiveTime();

	if (m_savedMess.mess != nullptr) {
		BL_WARN("# [ CBridge (%s) ] MESS SHOULD NOT EXIST !!!!\n", m_myName.c_str());
	}

	// Send command preparePub for all destination topics
	SendPreparePublishers();

	// Set this after we have sent prepare publishers, so that an eventual trigg from queueMessage()
	// doesn't start sending to early
	m_isConnected = true;

	// Try to start a sending 
	m_TrigSend = true;
	Notify();
}

// NOTE Called from transport thread
void CBridge::onDisconnect(CTransport* const sender)
{
    UNUSED(sender);
    BL_INFO("# [ CBridge (%s) ] DISCONNECTED...\n", m_myName.c_str());

	// Save state
	m_isConnected = false;

	// First lock the sends 
	ops::SafeLock const lock1(m_sendLock);

	// Then lock the queue
	ops::SafeLock const lock2(m_queueLock);

	// Handle ev. message in process of sending but not acknowledged
	if (m_savedMess.mess != nullptr) {
		// Put it back first in queue on the correct priority
		m_queue[m_savedMess.priority].push_front(m_savedMess);
		m_savedMess.topicName = "";
		m_savedMess.destTopicName = "";
		m_savedMess.mess = nullptr;
		m_savedMess.priority = 0;
		m_savedMess.AckNumber = 0;
	}

	uint32_t SumBytes = 0;

	// Loop over all subscribers
	// Eventually free queued messages, dependent on topic's disconnect mode
	for (std::vector<TSubscriberEntry>::size_type idx = 0; idx < m_subscribers.size(); idx++) {
		if (m_subscribers[idx].tc.eDisconnectedHandling == BridgeConfig::keepAll) { continue; }

		ops::ObjectName_T const topicName = m_subscribers[idx].tc.sTopicName;
		std::deque<TMessageEntry>* const Ptr = &m_queue[m_subscribers[idx].tc.iPriority];

		// if keepLatest then keep one sample and remove all others
		bool Keep = false;
		if (m_subscribers[idx].tc.eDisconnectedHandling == BridgeConfig::keepLatest) { Keep = true; }
			
		// Loop backwards
		std::deque<TMessageEntry>::reverse_iterator riter;
		for (riter = Ptr->rbegin(); riter != Ptr->rend(); riter++) {
			// Find a sample
			if ((*riter).topicName == topicName) {
				if (!Keep) {
					if ((*riter).mess != nullptr) {
						SumBytes += (uint32_t)(*riter).mess->getData()->spareBytes.size();
						(*riter).mess->unreserve();
					}
					(*riter).mess = nullptr;
				}
				Keep = false;
			}
		}
	}

	// Calculate approx. buffered size
	m_bytesBuffered -= SumBytes;
}

// NOTE Called from transport thread
// mess will be deleted by transport thread (i.e. the caller)
void CBridge::onOpsMessage(CTransport* const sender, ops::OPSObject* const mess, 
						   ops::ObjectName_T const publisherName,
						   ops::ObjectName_T const topicName, uint64_t const AckCounter)
{
    UNUSED(sender);
    UpdateReceiveTime();
	m_numRecvMess++;

	uint32_t errorCode = EC_NO_ERROR;

	// Find publisher to use from the topicName, i.e. destination
	ops::Publisher* pub = nullptr;

	// no need to take publisher lock since only transport thread is using 

	// Find publisher for this destination topic
	// (topicName is the destination name including domain to use for publishing)
	std::vector<TPublisherData>::iterator iter;
	for ( iter = m_publishers.begin(); iter != m_publishers.end(); iter++) {
		if ((*iter).topicName == topicName) {
			pub = (*iter).pub;
			break;
		}
	}
	if (pub == nullptr) { errorCode = EC_MISSING_PUB; }

	// Publish message, if not already done
	if ( (errorCode == EC_NO_ERROR) && (pub != nullptr) ) {
		try {
			// If we haven't already sent this one
			if (AckCounter > (*iter).AckNumber) {
				// Check min time beween samples
				std::chrono::milliseconds const diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(ops::ops_clock::now() - (*iter).pubTime);
				if (diffTime < m_minPubTime) {
					std::this_thread::sleep_for(m_minPubTime - diffTime);
				}

				// Publish message via OPS
				pub->setName(publisherName);
				pub->writeOPSObject(mess);

				// Save time
				(*iter).pubTime = ops::ops_clock::now();

				// Update saved AckNumber for publisher
				(*iter).AckNumber = AckCounter;
			}
		}
		catch (...) {
			errorCode = EC_PUBLISH_FAILED;
		}
	}

	// Send a Message acknowledge back to other endpoint
	m_transport->writeAckNak(AckCounter, errorCode);
}

// NOTE Called from transport thread
void CBridge::onAckNakMessage(CTransport* const sender, TAckNakMessage& ackNak)
{
    UNUSED(sender);
    UpdateReceiveTime();

	// if NAK logg error
	if (ackNak.ErrorCode != EC_NO_ERROR) {
		BL_TRACE("# [ CBridge (%s) ] AckNak Error: %d\n", m_myName.c_str(), ackNak.ErrorCode);
	}

	// Take lock
	ops::SafeLock const lock(m_sendLock);

	if (m_savedMess.AckNumber == ackNak.AckCounter) {
		// Calculate approx. buffered size
		uint32_t const bytes = (uint32_t)m_savedMess.mess->getData()->spareBytes.size();
		m_bytesBuffered -= bytes;
		
		// Release message now that we have sent it
		m_savedMess.mess->unreserve();
		m_savedMess.topicName = "";
		m_savedMess.destTopicName = "";
		m_savedMess.mess = nullptr;
		m_savedMess.priority = 0;
		m_savedMess.AckNumber = 0;

		// Send next message
		m_TrigSend = true;
		Notify();
	} else {
		// Wrong ackcounter, logg error
		BL_TRACE("# [ CBridge (%s) ] Ack Counter Error: %" PRId64 " (%" PRId64 ")\n", m_myName.c_str(), ackNak.AckCounter, m_savedMess.AckNumber);

		// Resend this message
		m_ResendSend = true;
		Notify();
	}
}

// NOTE Called from transport thread
ops::Publisher* CBridge::setupPublisher(ops::Topic& destTopic)
{
	TPublisherData pd;
	pd.topicName = ops::utilities::fullTopicName(destTopic.getDomainID(), destTopic.getName());

	// no need to take publisher lock since only transport thread is using 

	// Check if publisher already exist
	for (std::vector<TPublisherData>::iterator iter = m_publishers.begin(); iter != m_publishers.end(); iter++) {
		if ((*iter).topicName == pd.topicName) { return (*iter).pub; }

	}

	pd.pub = new ops::Publisher(destTopic);
	pd.pub->setName("Ops_Bridge");
	pd.AckNumber = 0;

	m_publishers.push_back(pd);

	BL_TRACE("# [ CBridge (%s) ] >>>> Created Publisher for topic: %s\n", m_myName.c_str(), pd.topicName.c_str());

	return pd.pub;
}

void CBridge::HandlePreparePubCommand(TCommandMessage& cmd)
{
    // cmd.DestTopicName contains Domain::TopicName to publish on
    // cmd.DataType contains the expected type for the topic

    // Check if publisher already created
    ops::Publisher* pub = nullptr;
    std::vector<TPublisherData>::iterator iter;
    for (iter = m_publishers.begin(); iter != m_publishers.end(); iter++) {
        if ((*iter).topicName == cmd.DestTopicName) {
            pub = (*iter).pub;
            break;
        }
    }
    if (pub != nullptr) {
        BL_TRACE("# [ CBridge (%s) ] Prepare Pub (EXISTED): %s\n", m_myName.c_str(), cmd.DestTopicName.c_str());
        if (cmd.AckCounter < (*iter).AckNumber) {
            // Other side has probably been restarted, so we need to clear our saved AckCounter's.
            // Otherwise we will skip publish of messages until AckCounter becomes large enough
            (*iter).AckNumber = 0;
        }
        return;
    }

    BL_TRACE("# [ CBridge (%s) ] Prepare Pub: %s\n", m_myName.c_str(), cmd.DestTopicName.c_str());

    ops::ObjectName_T dom, top;
    ops::utilities::splitTopicName(cmd.DestTopicName, dom, top);

    // Get participant
    ops::ObjectName_T partID(dom);
    partID += "PubSpec";
    ops::Participant* const part = ops::Participant::getInstance(dom, partID);
    if (part != nullptr) {
        // Check if we already has handled this participant
        bool found = false;
        for (unsigned int i = 0; i < m_vPubPart.size(); i++) {
            if (part == m_vPubPart[i]) { found = true; }
        }
        if (!found) {
            // If not add the special factory
            part->addTypeSupport(new BridgeTypeFactory());
            ///TODO add an error listener???
            m_vPubPart.push_back(part);
        }

        // Check that topic exists
        if (part->getConfig() != nullptr) {
            bool exist = false;
            try {
                ops::Domain* const domain = part->getConfig()->getDomain(dom);
                if (domain != nullptr) { exist = domain->existsTopic(top); }
            }
            catch (...) {
                exist = false;
            }
            if (!exist) {
                BL_TRACE("##### Topic: %s doesn't exist\n", cmd.DestTopicName.c_str());
            }
            else {
                // Create topic
                ops::Topic t = part->createTopic(top);

                // Check data type
                if (t.getTypeID() == cmd.DataType) {
                    setupPublisher(t);
                }
                else {
                    BL_TRACE("##### Topic: %s has wrong datatype\n", cmd.DestTopicName.c_str());
                }
            }
        }
    }
}

// NOTE Called from transport thread
void CBridge::onCommandMessage(CTransport* , TCommandMessage& cmd)
{
    UpdateReceiveTime();

	// Commands
	switch (cmd.Command) {
		case ctPreparePub:
            ///TODO
            /// We should probably send over the result if it is possible to publish
            /// This would save bandwidth 
            try {
                HandlePreparePubCommand(cmd);
            } catch (...) {
            }
			break;

		case ctPause: 
			// Should pause sending of ops messages
			m_isEpPaused = true;
			break;

		case ctContinue: 
			// Should continue sending of ops messages
			m_isEpPaused = false;
			break;
	}
}

// NOTE Called from transport thread
void CBridge::onStatusMessage(CTransport* const sender, const TStatusMessage& status)
{
    UNUSED(sender);
    UpdateReceiveTime();

	//  - Buffer status (cyclic, heartbeat)
	m_otherQueued = status.NumQueuedMessages;

	// Check status.Paused (other sides m_isEpPaused)
	if (status.Paused != m_isPaused) {
		// Other side has different state than we, so update other side.
		m_isPausedChanged = true;
		Notify();
	}

	// If there isn't an "active" send, try to start one
	m_TrigSend = true;
	Notify();
}

// =================================================================================
// NOTE Called from transport thread
void CBridge::onUdpMcMessage(CTransport* , const TUdpMcMessage& udpMc, const char* const data)
{
    // We don't buffer Raw UDP/MC messages
	m_raw.Write(udpMc, data);
}

// NOTE Called from RawMcUdp thread
void CBridge::onUdpMcMessage(RawMcUdp* , TUdpMcMessage& mess, const char* const data)
{
    // We don't buffer Raw UDP/MC messages
	m_transport->writeUdpMcMessage(mess, data);
}

// =================================================================================
// Status published on this side
void CBridge::getStatus(opsbridge::OpsBridgeStatusData& status)
{
	///TODO need any lock???
	// Status for command Pause/Continue, 0 ==> Running, <> 0 ==> Paused 
	status.Paused = (m_isPaused || m_isEpPaused) ? 1 : 0;

	status.NumQueuedSend = m_myQueued;
	status.NumQueuedRecv = m_otherQueued;
	status.NumBufferedBytes = m_bytesBuffered;
}

// =================================================================================
// Called from our thread 
void CBridge::TriggSendOPSMessage()
{
	// Take lock
	ops::SafeLock const lock(m_sendLock);

	// We must be connected 
	if (!m_isConnected) { return; }

	// if there already is a send "active" exit
	if (m_savedMess.mess != nullptr) { return; }

	// No "active" send, so try and start one
	SendOPSMessage();
}

// NOTE m_sendLock must be taken before call to this routine
void CBridge::SendOPSMessage()
{
	// Find and save highest prio message
	if (m_savedMess.mess == nullptr) {
		getHighestPrioMessage(m_savedMess);
	}
	if (m_savedMess.mess == nullptr) { return; }

	// Get object to send
	ops::OPSObject* const messHead = m_savedMess.mess->getData();
	ops::ObjectName_T const pubName = m_savedMess.mess->getPublisherName();

	// Define AckNumber for this message (a re-send will use the same number again so the receiver knows)
	if (m_savedMess.AckNumber == 0) {
		m_savedMess.AckNumber = ++m_AckCounter;
	}

	// Send on transport
	m_transport->writeOpsMessage(messHead, pubName, m_savedMess.destTopicName, m_savedMess.AckNumber);
	m_numSentMess++;
}

// 
void CBridge::Run()
{
	ops::ops_clock::time_point PeriodicalTime;
	ops::ops_clock::time_point StatusTime;

	m_transport->Start();

	while (!terminated()) {
		bool const notified = WaitForNotifyWithTimeout(100);

		// =========================================================================
		if (terminated()) { break; }

		// =========================================================================
		// Timeout
		if (!notified) {
		}

		// =========================================================================
		// Pause / Continue
		else if (m_isPausedChanged) {
			m_isPausedChanged = false;
			if (m_isConnected) {
				// Need to tell other side
				TCommandMessage cmd;
				cmd.Command = (m_isPaused) ? ctPause : ctContinue;
				cmd.DestTopicName = "";
				cmd.DataType = "";
				m_transport->writeCommand(cmd);
			}
		}

		// =========================================================================
		// Do Send 
		else if (m_ResendSend) {
			m_ResendSend = false;
			if (m_isConnected) {
				// If we or other side want to pause, don't send any OPS messages
				if (! (m_isPaused || m_isEpPaused) ) {
					ops::SafeLock const lock(m_sendLock);
					SendOPSMessage();
				}
			}
		}

		// =========================================================================
		// Trigg sending
		else if (m_TrigSend) {
			m_TrigSend = false;
			// If we or other side want to pause, don't send any OPS messages
			if (!(m_isPaused || m_isEpPaused)) {
				// Make sure there is a send "active"
				TriggSendOPSMessage();
			}
		}

		// =========================================================================
		// Periodical things

		// if connected, send heartbeat with status periodically
		if (m_isConnected && ((ops::ops_clock::now() - StatusTime) >= std::chrono::milliseconds(1000))) {
			TStatusMessage stat;
			stat.NumQueuedMessages = m_myQueued;
			stat.Paused = m_isEpPaused;			// Tell other side what they have ordered
			m_transport->writeStatus(stat);
			StatusTime = ops::ops_clock::now();
		}

		if ((ops::ops_clock::now() - PeriodicalTime) >= std::chrono::milliseconds(1000)) {
			// Calculate number of queued messages
			uint32_t NumQueuedMessages = 0;
			{
				ops::SafeLock const lock(m_queueLock);
				for (int prio = HIGHEST_PRIO; prio >= 0; prio--) {
					NumQueuedMessages += (uint32_t)m_queue[prio].size();
				}
			}
			m_myQueued = NumQueuedMessages;

			// if no data from other side (endpoint) in 5000 [ms], force a disconnect
			if (m_isConnected) {
				ops::SafeLock const lock(m_timeLock);
				if ((ops::ops_clock::now() - m_recvTime) > std::chrono::milliseconds(5000)) {
					BL_TRACE("# [ CBridge (%s) ] NO DATA. Forcing DISCONNECT\n", m_myName.c_str());
					m_transport->disconnect();
				}
			}

			PeriodicalTime = ops::ops_clock::now();
		}
	}
}

}
