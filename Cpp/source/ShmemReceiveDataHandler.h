/**
* 
* Copyright (C) 2025-2026 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "Lockable.h"
#include "ReceiveDataHandler.h"
#include "TopicsCounter.h"

namespace ops
{

	class Participant;

	class ShmemReceiveDataHandler : public ReceiveDataHandler
	{
	public:
		ShmemReceiveDataHandler(const Topic& top, Participant& part, const InternalKey_T& name);

#ifndef OPS_NO_SHMEM_TRANSPORT
		void AddReceiveChannel(const ObjectName_T& topicName, const Address_T& ip, int port) override;

	private:
		Topic topic;

		InternalKey_T baseName;
		TopicsCounter topics;
		Lockable topicsLock;
		bool usingPartInfo{ true };

		// Tell derived classes which topics that are active
		void topicUsage(const Topic& top, bool used) override;
#endif
	};

}
