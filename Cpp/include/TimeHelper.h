/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2021-2025 Lennart Andersson.
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

#ifndef ops_TimeHelperH
#define	ops_TimeHelperH

#include <chrono>
#include <string>

#include "OPSTypeDefs.h"

namespace ops
{
	using ops_clock = std::chrono::system_clock;

	class TimeHelper
	{
	public:
		///Returns the current time as a number of milliseconds since Epoch 1970-01-01.
		[[deprecated("Deprecated. Replaced by ops::ops_clock::now() returning a time_point")]]
		static int64_t currentTimeMillis() noexcept;

		///Sleeps the given duration
		static void sleep(const std::chrono::milliseconds& sleep_duration);
		static void sleep(const std::chrono::seconds& sleep_duration);

		[[deprecated("Deprecated. Replaced by sleep() taking chrono duration")]]
		///Sleeps the given number of milliseconds (millis).
	    static void sleep(int64_t millis);

		///Returns current system time as a string to be used as user output, file names etc...
	    static std::string getTimeToString();
		///Returns the current time as a number of milliseconds since Epoch 1970-01-01.
	    static int64_t getEpochTime() noexcept;

		static constexpr int64_t infinite = 0xffffffffLL;
	};

}
#endif
