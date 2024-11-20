/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2024 Lennart Andersson.
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
#include "TCPReceiveDataHandler.h"
#include "TCPReceiveDataChannel.h"
#include "Participant.h"

namespace ops
{
    TCPReceiveDataHandler::TCPReceiveDataHandler(Topic top, Participant& part) :
		ReceiveDataHandler(part, nullptr),
        topic(top)
    {
		// Handle TCP channels specified with an address and port
		if ((top.getTransport() == Topic::TRANSPORT_TCP) && (top.getPort() != 0)) {
			ReceiveDataChannel* const rdc_ = new TCPReceiveDataChannel(top, part);
            rdc_->connect(this);
            sampleMaxSize = rdc_->getSampleMaxSize();
            rdc.push_back(rdc_);
			usingPartInfo = false;
		} else {
			// Since we use the same "topic" parameters for all created RDC's, we can set the sampleMaxSize here
			sampleMaxSize = ReceiveDataChannel::calcSampleMaxSize(top);
		}
	}

	void TCPReceiveDataHandler::AddReceiveChannel(const ObjectName_T& topicName, const Address_T& ip, int const port)
	{
        UNUSED(topicName);
		OPS_PIFO_TRACE("Partinfo: name: " << topicName << ", ip: " << ip << ", port: " << port << "\n");

		if (port == 0) { return; }

		// We need to check if a new publisher has emerged that we need to connect to
		InternalKey_T key(ip);
		key += "::";
		key += NumberToString(port);

		// Look for it in rdc, if not there, create one
		bool found = false;
		for (auto const x : rdc) {
			if (x->key == key) {
				found = true;
				break;
			}
		}
		if (!found) {
			OPS_PIFO_TRACE("Partinfo: CREATED name: " << topicName << ", ip: " << ip << ", port: " << port << "\n");
			topic.setDomainAddress(ip);
			topic.setPort(port);
			try {
				ReceiveDataChannel* const rdc_ = new TCPReceiveDataChannel(topic, participant);
				rdc_->key = key;
				rdc_->connect(this);

				const SafeLock lock(messageLock);
				rdc.push_back(rdc_);

				if (Notifier<OPSMessage*>::getNrOfListeners() > 0) {
					rdc_->start();
				}
			} catch (const std::exception &e) {
				ExceptionMessage_T msg("Unknown exception: ");
				msg += e.what();
				ops::BasicError err("TCPReceiveDataHandler", "AddReceiveChannel", msg);
				Participant::reportStaticError(&err);
			}
		}
	}

	void TCPReceiveDataHandler::topicUsage(const Topic& top, bool const used)
	{
        if (usingPartInfo) {
            const SafeLock lock(topicsLock);
            // We should only register unique topics
			const auto it = topics.find(top.getName());
			int32_t count = 0;
			if (it != topics.end()) {
				count = topics[top.getName()];
			}
			// Register topic with participant info data handler/listener to get callbacks to handler above
			if (used) {
				++count;
				if (count == 1) {
					participant.registerTcpTopic(top.getName(), shared_from_this());
				}
			} else {
				--count;
				if (count == 0) {
					participant.unregisterTcpTopic(top.getName(), shared_from_this());
				}
			}
			topics[top.getName()] = count;
		}
	}

}
