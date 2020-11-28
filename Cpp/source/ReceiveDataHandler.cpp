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
#include "ReceiveDataHandler.h"
#include "OPSArchiverIn.h"
#include "BasicError.h" 
#include "Domain.h"
#include "Participant.h"
#include "ReceiverFactory.h"
#include "CommException.h"
#include "TCPClientBase.h"
#include "DataSegmentPool.h"

namespace ops
{
    ReceiveDataHandler::ReceiveDataHandler(Participant& part, ReceiveDataChannel* const rdc_) :
		participant(part)
    {
		if (rdc_ != nullptr) {
            sampleMaxSize = rdc_->getSampleMaxSize();
			rdc.push_back(rdc_);
			rdc_->connect(this);
		}
    }

    ReceiveDataHandler::~ReceiveDataHandler()
    {
		for (auto const x : rdc) {
			delete x;
		}
    }

	// Overridden from Notifier<OPSMessage*>
	void ReceiveDataHandler::addListener(Listener<OPSMessage*>* const listener, Topic& top)
    {
        {
            const SafeLock lock(&messageLock);
            Notifier<OPSMessage*>::addListener(listener);
            if (Notifier<OPSMessage*>::getNrOfListeners() == 1) {
                for (auto const x : rdc) {
                    x->start();
                }
            }
        }
		topicUsage(top, true);
	}

	// Overridden from Notifier<OPSMessage*>
    void ReceiveDataHandler::removeListener(Listener<OPSMessage*>* const listener, Topic& top)
    {
        topicUsage(top, false);

        const SafeLock lock(&messageLock);
		Notifier<OPSMessage*>::removeListener(listener);
		if (Notifier<OPSMessage*>::getNrOfListeners() == 0) {
			for (auto const x : rdc) {
				x->stop();
			}
		}
	}

	///Called whenever the receiver has new data.
	void ReceiveDataHandler::onMessage(ReceiveDataChannel&, OPSMessage* const mess)
	{
		const SafeLock lock(&messageLock);

		OPSMessage* const oldMessage = message;
		message = mess;

		//Add message to a reference handler that will keep the message until it is no longer needed.
		messageReferenceHandler.addReservable(message);
		message->reserve();

		//Send it to Subscribers
		Notifier<OPSMessage*>::notifyNewEvent(message);

		//This will delete this message if no one reserved it in the application layer.
		if (oldMessage != nullptr) {
			oldMessage->unreserve();
		}
	}

	bool ReceiveDataHandler::aquireMessageLock()
	{
		return messageLock.lock();
	}

	void ReceiveDataHandler::releaseMessageLock()
	{
		messageLock.unlock();
	}

	// Called when there are no more listeners and we are about to be put on the garbage-list for later removal
    void ReceiveDataHandler::clear()
    {
		for (auto const x : rdc) {
			x->clear();
		}

		// Need to release the last message we received, if any.
		// (We always keep a reference to the last message received)
		// If we don't, the garbage-cleaner won't delete us.
		if (message != nullptr) { message->unreserve(); }
		message = nullptr;
    }

}
