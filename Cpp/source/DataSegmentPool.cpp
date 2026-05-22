/**
*
* Copyright (C) 2018-2026 Lennart Andersson.
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
#include "opsidls/OPSConstants.h"
#include "memory_pool.h"
#include "DataSegmentPool.h"

namespace ops {

    using namespace opsidls;

	///TODO add an allocator as parameter 
	static memory_pools::memory_pool_exp<OPSConstants::PACKET_MAX_SIZE> rcvPool(20);

	DataSegmentPool& DataSegmentPool::Instance() noexcept
	{
		static DataSegmentPool inst;
		return inst;
	};

	char* DataSegmentPool::getEntry()
	{
		return rcvPool.getEntry();
	}

	void DataSegmentPool::returnEntry(char*& ptr)
	{
		rcvPool.returnEntry(ptr);
	}

#ifndef REPLACE_NETWORK_ALLOC
	MemoryMapAllocator& DataSegmentAllocator::Instance() noexcept
	{
		static DataSegmentAllocator inst;
		return inst;
	}

	char* DataSegmentAllocator::Allocate(unsigned int const size)
	{
		return new char[size];
	}

	void DataSegmentAllocator::Deallocate(char*& ptr)
	{
		delete[] ptr;
		ptr = nullptr;
	}
#endif

}
