#pragma once

/**
*
* Copyright (C) 2017-2025 Lennart Andersson.
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

#include <string>
#include "ThreadPool.h"

namespace ops {
	namespace thread_support {
		// Called from Participant to create a thread pool on which to execute the Participant Run() method.
		std::unique_ptr<ThreadPool> CreateThreadPool();

		// Called from within the thread to be named
		void SetThreadName(const char* name);
	}
}
