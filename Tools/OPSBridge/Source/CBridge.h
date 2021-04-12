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

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <atomic>

#include <ops.h>
#include <Publisher.h>
#include <Lockable.h>
#include <TimeHelper.h>

#include "opsbridge/OpsBridgeStatusData.h"

#include "CWorkerThread.h"
#include "BridgeConfig.h"
#include "CTransport.h"
#include "RawMcUdp.h"

namespace opsbridge {

	class CBridge : public CWorkItemEx, CTransportListener, ops::DataListener, RawMcUdpListener
	{
	public:
		CBridge() = delete;

		CBridge(std::string name, 
			int64_t maxBufferSize, int64_t iMinPubTime,
			CTransport* transport);
		~CBridge();

		std::string getName() {return m_myName;}

		// Start a subscriber for the given topic
		void setupSubscriber(ops::Topic& recTopic, BridgeConfig::TTopicConfig& tc);

		void removeAllSubscribers();

		RawMcUdp& getRef() { return m_raw; }

		// Start bridge. Should be called after all subscribers have been setup
		// void Start(); is defined in base class CWorkItemEx

		// Methods for temporarily stop/start one or more subscribers
		void stopSubscriber(ops::ObjectName_T topic);
		void stopAllSubscribers();
		void startSubscriber(ops::ObjectName_T topic);
		void startAllSubscribers();

		// Method used for temporarily skipping of messages
		// Used if the application starts to "eat" memory 
		void setSkipMessage(bool flag);

		// ---------------------------------------------------------------
		void Pause();
		void Continue();

		// ---------------------------------------------------------------
		// Statistics & debug
		volatile uint32_t m_numRecvMess;
		volatile uint32_t m_numSentMess;
		volatile uint32_t m_myQueued;
		volatile uint32_t m_otherQueued;
		std::atomic<uint32_t> m_bytesBuffered;

		void getStatus(opsbridge::OpsBridgeStatusData& status);

	protected:
		// Override from ops::DataListener, called whenever new data arrives.
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
		virtual void onNewData(ops::DataNotifier* const subscriber) override;

		// Override from CTransportListener
		void onConnect(CTransport* const sender) override;
		void onDisconnect(CTransport* const sender) override;
		void onOpsMessage(CTransport* const sender, 
			ops::OPSObject* const mess, 
			ops::ObjectName_T const publisherName,
			ops::ObjectName_T const topicName,
			uint64_t const AckCounter) override;
		void onAckNakMessage(CTransport* const sender, TAckNakMessage& ackNak) override;
		void onCommandMessage(CTransport* const sender, TCommandMessage& cmd) override;
		void onStatusMessage(CTransport* const sender, const TStatusMessage& status) override;
		void onUdpMcMessage(CTransport* const sender, const TUdpMcMessage& udpMc, const char* data) override;

		// Override from RawMcUdpListener
		void onUdpMcMessage(RawMcUdp* const sender, TUdpMcMessage& mess, const char* data) override;

        // ---------------------------------------------------------------
        void HandlePreparePubCommand(TCommandMessage& cmd);
		void Run();

	private:
		// ---------------------------------------------------------------
		std::string m_myName;
		uint64_t m_maxBufferSize;
		std::chrono::milliseconds m_minPubTime{ 0 };

		volatile bool m_isPausedChanged;
		bool m_ResendSend;
		bool m_TrigSend;

		CTransport* m_transport;
		RawMcUdp m_raw;

		volatile bool m_isConnected;
		volatile bool m_skipMessages;

		volatile bool m_isPaused;
		volatile bool m_isEpPaused;

		// ---------------------------------------------------------------
		//
		typedef struct {
			// Subscriber
			ops::Subscriber* sub;
			// 
			ops::ObjectName_T domainName;
			ops::ObjectName_T topicName;
			ops::TypeId_T dataType;
			// Start/stop handling
			bool started;
			uint32_t stopCounter;
			// Statistics
			uint32_t numMessages;
			uint32_t numSkippedMessages;
			// Configuration values
			BridgeConfig::TTopicConfig tc;
		} TSubscriberEntry;

		// List with subscribers for data
		std::vector<TSubscriberEntry> m_subscribers;
		ops::Lockable m_subscriberLock;

		// List with participants for publishing
		std::vector<ops::Participant*> m_vPubPart;

		// ---------------------------------------------------------------
		// List with publishers for data 
		struct TPublisherData {
			ops::Publisher* pub{ nullptr };
			ops::ObjectName_T topicName;
			// Last AckNumber we published (init to 0, updated at each publish)
			uint64_t AckNumber{ 0 };
			// Publish time, used to provide a minimum distance between publishes
			ops::ops_clock::time_point pubTime;
		};

		std::vector<TPublisherData> m_publishers;
		// not needed since only transport thread is using 
		//ops::Lockable m_publishersLock;

		ops::Publisher* setupPublisher(ops::Topic& destTopic);

		// ---------------------------------------------------------------
		// Message queues and lock
		typedef struct {
			// Identification of message
			ops::ObjectName_T topicName;
			ops::ObjectName_T destTopicName;
			// Reference to message being written
			ops::OPSMessage* mess;
			// Priority, used if we need to put it back on queue
			int priority;
			// AckNumber used for this message (init to 0, set at first send)
			uint64_t AckNumber;
		} TMessageEntry;

		std::deque<TMessageEntry> m_queue[HIGHEST_PRIO+1];		//prio = 0..HIGHEST_PRIO
		ops::Lockable m_queueLock;

		void queueMessage(ops::OPSMessage* mess, TSubscriberEntry& subEntry);
		bool getHighestPrioMessage(TMessageEntry& me);
		void clearQueuedMessages();

		// ---------------------------------------------------------------
		//
		TMessageEntry m_savedMess;
		ops::Lockable m_sendLock;
		uint64_t m_AckCounter;

		void SendOPSMessage();
		void TriggSendOPSMessage();

		// 
		void SendPreparePublishers();

		// ---------------------------------------------------------------
		/// saved time
		ops::ops_clock::time_point m_recvTime;
		ops::Lockable m_timeLock;

		void UpdateReceiveTime();

		// ---------------------------------------------------------------
		bool m_MaxLimitReached;

	};

}
