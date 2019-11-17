/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
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

#include <map>
#include <iostream>

#include "Topic.h"
#include "ByteBuffer.h"
#include "OPSConstants.h"
#include "Receiver.h"
#include "OPSMessage.h"
#include "Listener.h"
#include "BytesSizePair.h"
#include "OPSExport.h"
#include "ConnectStatus.h"

namespace ops
{
	//Forward declaration
	class Participant;
	
	class ReceiveDataChannel;

	struct ReceiveDataChannelCallbacks
	{
		virtual void onMessage(ReceiveDataChannel& rdc, OPSMessage* mess) = 0;
		virtual void onStatusChange(ReceiveDataChannel& rdc, ConnectStatus& status) = 0;
	};

	class OPS_EXPORT ReceiveDataChannel : 
		protected Listener<BytesSizePair>, 
		public Listener<ConnectStatus>
	{
	public:
		ReceiveDataChannel(Topic top, Participant& part, Receiver* recv = nullptr);
		virtual ~ReceiveDataChannel();

		void connect(ReceiveDataChannelCallbacks* client) {
			_client = client;
		}

		void clear();

		void start();
		void stop();

		int getSampleMaxSize() const
        {
            return sampleMaxSize;
        }

		Receiver* getReceiver() const
		{
			return receiver;
		}

		virtual void topicUsage(Topic& top, bool used)
		{
			UNUSED(top); UNUSED(used);
		}

		// Key used to identify channel when several channels are keept in a container
		InternalKey_T key;

	protected:
		///Override from Listener
		///Called whenever the receiver has new data.
		void onNewEvent(Notifier<BytesSizePair>* sender, BytesSizePair byteSizePair) override;
		bool calculateAndSetSpareBytes(ByteBuffer &buf, OPSObject* obj, int segmentPaddingSize);

		void onNewEvent(Notifier<ConnectStatus>*, ConnectStatus arg) override
		{
			if (_client != nullptr) {
				_client->onStatusChange(*this, arg);
			}
		}

	private:
		ReceiveDataChannelCallbacks* _client;

		///The receiver used for this topic. 
		Receiver* receiver;

		///Preallocated MemoryMap for receiving data
		MemoryMap memMap;
		int sampleMaxSize;

		//The Participant to which this ReceiveDataChannel belongs.
		Participant& participant;

		///The accumulated size in bytes of the current message
		int currentMessageSize;

		int expectedSegment;
		bool firstReceived;
	};
	
}
