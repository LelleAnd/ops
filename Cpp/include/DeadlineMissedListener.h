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

#include <vector>
#include <functional>

#include "OPSTypeDefs.h"

namespace ops
{
//Forward declaration
class DeadlineMissedEvent;

class DeadlineMissedListener
{
public:
	virtual void onDeadlineMissed(DeadlineMissedEvent* e) = 0;
	virtual ~DeadlineMissedListener() = default;
};

class DeadlineMissedEvent
{
private:
	std::vector<DeadlineMissedListener*> listeners;
	std::vector<std::function<void(ops::DeadlineMissedEvent* sender)>> closureListeners;
public:
	void addDeadlineMissedListener(DeadlineMissedListener* listener)
	{
		listeners.push_back(listener);
	}
	void addDeadlineMissedListener(std::function<void(ops::DeadlineMissedEvent* sender)> callback)
	{
		closureListeners.push_back(callback);
	}
	void notifyDeadlineMissed()
	{
		for(unsigned int i = 0; i < listeners.size() ; i++) {
			listeners[i]->onDeadlineMissed(this);
		}
		for (unsigned int i = 0; i < closureListeners.size(); i++) {
			closureListeners[i](this);
		}
	}
};

}
