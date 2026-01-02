/**
* 
* Copyright (C) 2024-2025 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "ReceiveDataHandler.h"
#include "InProcDistributor.h"

namespace ops
{

	class InProcReceiveDataHandler : public ReceiveDataHandler
	{
	private:
		std::shared_ptr<InProcDistributor> distributor;
		std::map<ObjectName_T, int32_t> topics;
		Lockable topicsLock;

	public:
		InProcReceiveDataHandler(Topic top, Participant& part, std::shared_ptr<InProcDistributor> dist) :
			ReceiveDataHandler(part, std::make_unique<ReceiveDataChannelBase>(top)), distributor(dist)
		{
		}

		virtual void topicUsage(const Topic& topic, bool used) override
		{
			const SafeLock lock(topicsLock);
			// We should only register unique topics
			const auto it = topics.find(topic.getName());
			int32_t count = 0;
			if (it != topics.end()) {
				count = topics[topic.getName()];
			}
			if (used) {
				++count;
				if (count == 1) {
					// register topic in distributor with a lambda callback
					distributor->regInProcReceiver(topic, [=](OPSMessage* msg)
						{
							msg->setDataOwner(true);
							onMessage(*rdcs[0], msg);
						}
					);
				}
			} else {
				--count;
				if (count == 0) {
					// unregister topic in distributor
					distributor->unregInProcReceiver(topic);
				}
			}
			topics[topic.getName()] = count;
		}
	};
	
}
