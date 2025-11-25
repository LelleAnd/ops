/**
*
* Copyright (C) 2018-2025 Lennart Andersson.
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

#include <cinttypes>

// For compilers not defining this for C++
#ifndef PRId64
	#define PRId64 "ld"
#endif

// 1 --> Trace and above
// 2
// 3
// 4 --> Debug and above
// 5
// 6
// 7 --> Info and above
// 8 --> Warn and above
// 9 --> Error and above
extern int BL_log_level;

#define BL_TRACE3(...) if (BL_log_level <= 1) { printf(__VA_ARGS__); }
#define BL_TRACE2(...) if (BL_log_level <= 2) { printf(__VA_ARGS__); }
#define BL_TRACE(...)  if (BL_log_level <= 3) { printf(__VA_ARGS__); }
#define BL_DEBUG3(...) if (BL_log_level <= 4) { printf(__VA_ARGS__); }
#define BL_DEBUG2(...) if (BL_log_level <= 5) { printf(__VA_ARGS__); }
#define BL_DEBUG(...)  if (BL_log_level <= 6) { printf(__VA_ARGS__); }
#define BL_INFO(...)   if (BL_log_level <= 7) { printf(__VA_ARGS__); }
#define BL_WARN(...)   if (BL_log_level <= 8) { printf(__VA_ARGS__); }
#define BL_ERROR(...)  if (BL_log_level <= 9) { printf(__VA_ARGS__); }
