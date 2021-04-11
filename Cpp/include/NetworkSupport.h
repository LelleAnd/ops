/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2021 Lennart Andersson.
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
#include "IOService.h"

namespace ops
{
	extern uint32_t IPString2Addr(const Address_T addr);
	extern Address_T IPAddr2String(const uint32_t addr);

	extern bool isValidMCAddress(const Address_T addr);
	extern bool isValidNodeAddress(const Address_T addr);
	extern bool isMyNodeAddress(const Address_T addr, IOService* ioServ);

	extern Address_T doSubnetTranslation(const Address_T addr, IOService* ioServ);

	extern InternalString_T GetHostName();
}
