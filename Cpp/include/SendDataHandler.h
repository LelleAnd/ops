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

#pragma once

#include <memory>

#include "Topic.h"
#include "Sender.h"
#include "Notifier.h"
#include "Lockable.h"
#include "ConnectStatus.h"

namespace ops
{
	class SendDataHandler :
        protected Listener<ConnectStatus>, public Notifier<ConnectStatus>,
        public std::enable_shared_from_this<SendDataHandler>
	{
	public:
		virtual ~SendDataHandler() = default;

		virtual bool sendData(char* buf, int bufSize, Topic& topic) = 0;

		virtual void addPublisher(void* client, Topic& top)
		{
            SafeLock lock(&mutex);
			// Check that it isn't already in the list
			for (unsigned int i = 0; i < publishers.size(); i++) {
				if (publishers[i] == client) return;
			}
			// Save client in the list
			publishers.push_back(client);
			// For the first client, we open the sender
			if (publishers.size() == 1) sender->open();
			topicUsage(top , true);
		}

		virtual void removePublisher(void* client, Topic& top)
		{
            SafeLock lock(&mutex);
			// Remove it from the list
			std::vector<void*>::iterator Iter;
			for (Iter = publishers.begin(); Iter != publishers.end(); Iter++) {
				if (*Iter == client) {
					publishers.erase(Iter);
					topicUsage(top, false);
					break;
				}
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

	protected:
        Sender* sender = nullptr;
        Lockable mutex;

		// Called from senders (TCPServer)
		virtual void onNewEvent(Notifier<ConnectStatus>* sndr, ConnectStatus arg) override
		{
			UNUSED(sndr)
			// Forward status to all connected publishers
			notifyNewEvent(arg);
		}

		// Tell derived classes which topics that are active
		virtual void topicUsage(Topic& top, bool used)
		{
			UNUSED(top); UNUSED(used);
		}

	private:
		std::vector<void*> publishers;
	};
}
