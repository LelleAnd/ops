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
#include <memory>

#include "Topic.h"
#include "OPSMessage.h"
#include "Notifier.h"
#include "Listener.h"
#include "ReferenceHandler.h"
#include "ReceiveDataChannel.h"
#include "ConnectStatus.h"

namespace ops
{
	//Forward declaration
	class Participant;
	
	class ReceiveDataHandler : 
		protected Notifier<OPSMessage*>, 
		public Notifier<ConnectStatus>, public ReceiveDataChannelCallbacks,
        public std::enable_shared_from_this<ReceiveDataHandler>
	{
		friend class ReceiveDataHandlerFactory;
		using Notifier<OPSMessage*>::addListener;
		using Notifier<OPSMessage*>::removeListener;

	public:
		using Notifier<ConnectStatus>::addListener;
		using Notifier<ConnectStatus>::removeListener;

		ReceiveDataHandler(Participant& part, std::unique_ptr<ReceiveDataChannelBase> rdc = nullptr);
		virtual ~ReceiveDataHandler() = default;

		bool aquireMessageLock();
		void releaseMessageLock();

		void clear();

		// Add/Remove listener to received messages
		void addListener(Listener<OPSMessage*>* listener, Topic& top);
		void removeListener(Listener<OPSMessage*>* listener, Topic& top);

		int numReservedMessages() const noexcept
		{
			return messageReferenceHandler.size();
		}

		bool asyncFinished()
		{
			bool finished = true;
			SafeLock lock(messageLock);
			for (auto& rdc : rdcs) {
				finished &= rdc->asyncFinished();
			}
			return finished;
		}

		int getSampleMaxSize() const noexcept
        {
            return sampleMaxSize;
        }

        bool dataAvailable()
        {
            SafeLock lock(messageLock);
            for (auto& rdc : rdcs) {
                if (rdc->bytesAvailable() > 0) { return true; }
            }
            return false;
        }

    protected:
		Lockable messageLock;

		//The receiver channel(s) used for this transport channel. 
        std::vector<std::unique_ptr<ReceiveDataChannelBase>> rdcs;

		//The Participant to which this ReceiveDataHandler belongs.
		Participant& participant;

		// Tell derived classes which topics that are active
		virtual void topicUsage(const Topic& top, bool used) 
		{
			UNUSED(top); UNUSED(used);
		}

		virtual void onMessage(ReceiveDataChannelBase& rdc, OPSMessage* mess) override;
		virtual void onStatusChange(ReceiveDataChannelBase& rdc, ConnectStatus& status) override
		{
			UNUSED(rdc);
			Notifier<ConnectStatus>::notifyNewEvent(status);
		}

        int sampleMaxSize{ 0 };

    private:
		///Current OPSMessage, valid until next sample arrives.
        OPSMessage* message{ nullptr };

		///ReferenceHandler that keeps track of object created on reception and deletes them when no one is interested anymore.
		ReferenceHandler messageReferenceHandler;
	};
	
}
