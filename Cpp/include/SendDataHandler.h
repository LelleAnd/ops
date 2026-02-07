/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2026 Lennart Andersson.
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

#include <memory>
#include <algorithm>

#include "OPSMessage.h"
#include "Topic.h"
#include "Sender.h"
#include "Notifier.h"
#include "Lockable.h"
#include "ConnectStatus.h"
#include "Telemetry.h"

namespace ops
{
	class SendDataHandler :
        protected Listener<ConnectStatus>, public Notifier<ConnectStatus>,
        public std::enable_shared_from_this<SendDataHandler>
	{
	public:
		virtual ~SendDataHandler() = default;

		virtual bool sendData(char* buf, int bufSize, const Topic& topic) = 0;

		// Used for inprocess transport
		virtual bool sendMessage(const Topic&, const OPSMessage&) { return false; }

		virtual void addPublisher(void* client, Topic& top)
		{
            SafeLock lock(mutex);
			// Check that it isn't already in the list
			auto it = std::find(publishers.begin(), publishers.end(), client);
			if (it != publishers.end()) return;

			// Save client in the list
			publishers.push_back(client);
			// For the first client, we open the sender
			if (publishers.size() == 1) sender->open();
			topicUsage(top , true);
		}

		virtual void removePublisher(void* client, Topic& top)
		{
            SafeLock lock(mutex);
			// Remove it from the list
			auto it = std::find(publishers.begin(), publishers.end(), client);
			if (it != publishers.end()) {
				publishers.erase(it);
				topicUsage(top, false);
			}
			if (publishers.size() == 0) sender->close();
		}

		uint16_t getLocalPort()
		{
			return sender->getLocalPort();
		}

		Address_T getLocalAddress()
		{
			return sender->getLocalAddress();
		}

        uint32_t getLocalAddressHost()
        {
            return sender->getLocalAddressHost();
        }

        // At least one publisher must be added to us for this call to work correct
		virtual void updateTransportInfo(Topic&)
		{
		}

		// Get some collected telemetry from the sender
		virtual Telemetry getTelemetry()
		{
			return Telemetry();
		}

	protected:
		std::unique_ptr<Sender> sender;
        Lockable mutex;

		// Called from senders (TCPServer)
		virtual void onNewEvent(Notifier<ConnectStatus>* sndr, ConnectStatus arg) override
		{
			UNUSED(sndr)
			// Forward status to all connected publishers
			notifyNewEvent(arg);
		}

		// Tell derived classes which topics that are active
		virtual void topicUsage(const Topic& top, bool used)
		{
			UNUSED(top); UNUSED(used);
		}

	private:
		std::vector<void*> publishers;
	};
}
