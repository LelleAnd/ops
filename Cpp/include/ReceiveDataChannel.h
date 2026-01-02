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

#include <map>
#include <iostream>

#include "Topic.h"
#include "ByteBuffer.h"
#include "Receiver.h"
#include "OPSMessage.h"
#include "Listener.h"
#include "BytesSizePair.h"
#include "ConnectStatus.h"
#include "opsidls/OPSConstants.h"

namespace ops
{
	//Forward declaration
	class Participant;
	
	class ReceiveDataChannelBase;

	struct ReceiveDataChannelCallbacks
	{
		virtual void onMessage(ReceiveDataChannelBase& rdc, OPSMessage* mess) = 0;
		virtual void onStatusChange(ReceiveDataChannelBase& rdc, ConnectStatus& status) = 0;
		virtual ~ReceiveDataChannelCallbacks() = default;
	};

	class ReceiveDataChannelBase
	{
	public:
		ReceiveDataChannelBase(const Topic& top) :
			sampleMaxSize(calcSampleMaxSize(top))
		{
		}
		virtual ~ReceiveDataChannelBase() = default;

		void connect(ReceiveDataChannelCallbacks* client) noexcept {
			_client = client;
		}
		virtual void clear() { }

		virtual void start() { }
		virtual void stop() { }

		int getSampleMaxSize() const noexcept { return sampleMaxSize; }

		static int calcSampleMaxSize(const Topic& top)
		{
			return top.getSampleMaxSize() > opsidls::OPSConstants::USABLE_SEGMENT_SIZE ?
				top.getSampleMaxSize() :
				opsidls::OPSConstants::USABLE_SEGMENT_SIZE;
		}

		virtual bool asyncFinished() { return true; }
		virtual size_t bytesAvailable() { return 0; }

		virtual uint16_t getLocalPort() { return 0; }
		virtual Address_T getLocalAddress() { return "0.0.0.0"; }

		// Key used to identify channel when several channels are keept in a container
		InternalKey_T key;

	protected:
		ReceiveDataChannelCallbacks* _client{ nullptr };
		int sampleMaxSize{ 0 };
	};

	class ReceiveDataChannel : public ReceiveDataChannelBase,
		protected SingleListener<BytesSizePair>,
		public Listener<ConnectStatus>
	{
	public:
		ReceiveDataChannel(const Topic& top, Participant& part, std::unique_ptr<Receiver> recv = nullptr);
		virtual ~ReceiveDataChannel() = default;

		virtual void clear() override;

		virtual void start() override;
		virtual void stop() override;

		virtual bool asyncFinished() override
		{
			return receiver->asyncFinished();
		}

		virtual size_t bytesAvailable() override
		{
			return receiver->bytesAvailable();
		}

		virtual uint16_t getLocalPort() override
		{
			return receiver->getLocalPort();
		}

		virtual Address_T getLocalAddress() override
		{
			return receiver->getLocalAddress();
		}

		virtual void topicUsage(const Topic& top, bool used)
		{
			UNUSED(top); UNUSED(used);
		}

	protected:
		///Override from Listener
		///Called whenever the receiver has new data.
		void onNewEvent(SingleNotifier<BytesSizePair>* sender, BytesSizePair byteSizePair) override;
		bool calculateAndSetSpareBytes(ByteBuffer &buf, OPSObject* obj, int segmentPaddingSize);

        void onNewEvent(Notifier<ConnectStatus>*, ConnectStatus arg) override;

        ///Preallocated MemoryMap for receiving data
        MemoryMap memMap;
		int expectedSegment{ 0 };

        //The Participant to which this ReceiveDataChannel belongs.
        Participant& participant;

		///The receiver used for this channel
		std::unique_ptr<Receiver> receiver;

	private:
		///The accumulated size in bytes of the current message
		int currentMessageSize{ 0 };

		bool firstReceived{ false };
	};
	
}
