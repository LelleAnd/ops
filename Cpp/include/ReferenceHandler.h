/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019 Lennart Andersson.
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

#ifndef ops_ReferenceHandler_h
#define ops_ReferenceHandler_h

#include <vector>
#include "Reservable.h"
#include "ReserveInfo.h"
#include "Lockable.h"

namespace ops
{

	class ReferenceHandler : Lockable
	{
	public:
		ReferenceHandler() noexcept
		{
		}

		void addReservable(Reservable* res)
		{
			SafeLock lock(this);
			references.push_back(res);
			res->setReferenceHandler(this);
		}

		void onNewEvent(Reservable* notifier, ReserveInfo reserveInfo)
		{
			if(reserveInfo.nrOfReservations == 0)
			{
				removeReference(notifier);
			}
		}

		int size() const noexcept
		{
			return (int)references.size();
		}

	private:
		std::vector<Reservable*> references;

		void removeReference(const Reservable* reservable)
		{
			SafeLock lock(this);
			for(unsigned int i = 0; i < references.size(); i++)
			{
				if(references[i] == reservable)
				{
					if(references[i])
					{
						delete references[i];
						references[i] = nullptr;
					}

					std::vector<Reservable*>::iterator p = references.begin();
					p += i;
					references.erase(p);
					return;
				}
			}
		}
	};
}
#endif
