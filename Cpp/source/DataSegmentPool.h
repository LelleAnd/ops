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

#pragma once

#include "MemoryMap.h"

namespace ops {

	// Pool with fixed sized data segments (OPSConstants::PACKET_MAX_SIZE)
	class DataSegmentPool {
	public:
		static DataSegmentPool& Instance() noexcept;

		char* getEntry();
		void returnEntry(char*& ptr);

		DataSegmentPool(DataSegmentPool const&) = delete;
		DataSegmentPool(DataSegmentPool&&) = delete;
		DataSegmentPool& operator =(DataSegmentPool&&) = delete;
		DataSegmentPool& operator =(DataSegmentPool const&) = delete;

	private:
		DataSegmentPool() = default;
	};

	class DataSegmentAllocator : MemoryMapAllocator {
	public:
		static MemoryMapAllocator& Instance() noexcept;

		virtual char* Allocate(unsigned int size) override;
		virtual void Deallocate(char*& ptr) override;

		DataSegmentAllocator(DataSegmentAllocator const&) = delete;
		DataSegmentAllocator(DataSegmentAllocator&&) = delete;
		DataSegmentAllocator& operator =(DataSegmentAllocator&&) = delete;
		DataSegmentAllocator& operator =(DataSegmentAllocator const&) = delete;

	private:
		DataSegmentAllocator() = default;
	};

}
