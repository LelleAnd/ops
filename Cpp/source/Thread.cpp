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

#include "Thread.h"

namespace ops
{
    Thread::Thread() noexcept
    {
    }
    
    Thread::~Thread()
    {
        stop();
        join();
    }
    
    int Thread::start()
    {
        if (thread.get() == nullptr) {
			thread = std::unique_ptr<std::thread>(new std::thread(&Thread::EntryPoint, this));
		}
        return 0;
    }

    bool Thread::join()
    {
        if (thread.get() != nullptr) {
            // Wait for thread to exit before we delete it
            thread->join();
            thread.reset();
        }
        return true;
    }
    
    void Thread::stop()
    {
	}
    
    void Thread::EntryPoint(Thread* const pt)
    {
        if (pt != nullptr) { pt->run(); }
    }
}
