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

#include "OPSTypeDefs.h"
#include "SingleThreadPool.h"
#include "ThreadSupport.h"

#ifdef _WIN32
	#include <windows.h> 
#else
	#include <pthread.h>
#endif

namespace ops {
namespace thread_support {

	// Called from Participant to create a thread pool on which to execute the Participant Run() method.
	std::unique_ptr<ThreadPool> CreateThreadPool()
	{
		return std::make_unique<SingleThreadPool>();
	}

#ifdef _WIN32
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
	#pragma pack(push,8)  
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.  
		LPCSTR szName; // Pointer to name (in user addr space).  
		DWORD dwThreadID; // Thread ID (-1=caller thread).  
		DWORD dwFlags; // Reserved for future use, must be zero.  
	} THREADNAME_INFO;
	#pragma pack(pop)  
#endif

	// Called from within the thread to be named
	void SetThreadName(const char* const name)
	{
#ifdef _WIN32
		THREADNAME_INFO info{};
		info.dwType = 0x1000;
		info.szName = name;
		info.dwThreadID = (DWORD)-1;
		info.dwFlags = 0;
		#pragma warning(push)  
		#pragma warning(disable: 6320 6322)  
		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		} __except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		#pragma warning(pop) 
#else
		pthread_setname_np(pthread_self(), name);
#endif
	}
}
}

