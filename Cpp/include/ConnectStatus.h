/**
* 
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

#include "OPSTypeDefs.h"

namespace ops
{
	// Information provided in notifications to listeners when:
	//   - for TCP Server, 
	//     - a new connection is accepted,
	//     - a connection fails and is closed.
	//
	// NOTE: Lock(s) are held while in notification callback. Avoid calling OPS methods
	//       while in the callback, since that may lead to a deadlock.
	struct ConnectStatus
	{
		Address_T addr;
		uint16_t port;
		bool connected;
		int totalNo;
		ConnectStatus(bool c, int no) noexcept : port(0), connected(c), totalNo(no) { }
	};

}
