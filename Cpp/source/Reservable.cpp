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

#include <assert.h>

#include "OPSTypeDefs.h"
#include "Reservable.h"
#include "ReferenceHandler.h"

namespace ops
{

	Reservable::Reservable(const Reservable&)
	{
		// Should not copy our data
	}
	Reservable& Reservable::operator= (const Reservable& l)
	{
        if (this != &l) {
            // Should not copy our data
        }
		return *this;
	}
	
	void Reservable::setReferenceHandler(ReferenceHandler* const refHandler)
	{
#ifndef OPS_REMOVE_ASSERT
		// Referencehandler should not be changed if already reserved
		assert(nrOfReservations == 0);
#endif
		referenceHandler = refHandler;
	}
	ReferenceHandler* Reservable::getReferenceHandler() const noexcept
	{
		return referenceHandler;
	}

	void Reservable::reserve()
	{
#ifndef OPS_REMOVE_ASSERT
		// Should not be used if no referenceHandler is assigned
		assert(referenceHandler != nullptr);
#endif
		nrOfReservations++;
		if(referenceHandler != nullptr)
		{
			referenceHandler->onNewEvent(this, ReserveInfo(this, nrOfReservations));
		}
	}
	void Reservable::unreserve()
	{
#ifndef OPS_REMOVE_ASSERT
		// Should not be used if no referenceHandler is assigned
		assert(referenceHandler != nullptr);
#endif
		nrOfReservations--;
		if(referenceHandler != nullptr)
		{
			referenceHandler->onNewEvent(this, ReserveInfo(this, nrOfReservations));
		}
	}
	int Reservable::getNrOfReservations() const noexcept
	{
		return nrOfReservations;
	}

	Reservable::~Reservable()
	{
#ifndef OPS_REMOVE_ASSERT
		assert(nrOfReservations == 0);
#endif
	}

}
