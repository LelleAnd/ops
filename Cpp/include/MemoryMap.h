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

#ifndef ops_MemoryMap_h
#define	ops_MemoryMap_h

#include <exception>

#include "OPSTypeDefs.h"
#include "OPSExport.h"

namespace ops
{

class MemoryMapException : public std::exception
{
private:
	ExceptionMessage_T message;
public:
	MemoryMapException(ExceptionMessage_T m)
	{
		message = "MemoryMapException: ";
		message += m;
	}
	const char* what() const noexcept override { return message.c_str(); }
};

// TODO: Replace this with a C++ allocator??
struct MemoryMapAllocator {
	virtual char* Allocate(unsigned int size) = 0;
	virtual void Deallocate(char*& ptr) = 0;
	virtual ~MemoryMapAllocator() = default;
};

///A memory buffer that together with the ByteBuffer class is used for [de]serializing OPS messages
class OPS_EXPORT MemoryMap
{
public:
	MemoryMap()
	{
		bytes = small_width_vector;
		bytes[0] = nullptr;
	}
	MemoryMap(int no_segs, int seg_size, MemoryMapAllocator* mma = nullptr):
		segment_allocator(mma),
		no_of_segments(no_segs),
		segment_size(seg_size),
		dataCreator(true)
	{
		if (no_of_segments <= smallWidthOpt) {
			// For small maps, use our internal vector as segment pointer array
			bytes = small_width_vector;
		} else {
			// For larger maps, allocate segment pointer array
			bytes = new char*[no_of_segments];
		}
		// Allocate all segments in one chunk
		if (segment_allocator != nullptr) {
			bytes[0] = segment_allocator->Allocate(no_of_segments*segment_size);
		} else {
			bytes[0] = new char[no_of_segments*segment_size];
		}
		// Setup segment pointers to point into the allocated chunk
		for(int i = 1; i < no_of_segments; i++) {
			bytes[i] = bytes[i-1] + segment_size;
		}
	}
	MemoryMap(char* segment, int size) noexcept :
		  no_of_segments(1), 
		  segment_size(size)
	{
		bytes = small_width_vector;
		bytes[0] = segment;
	}
	~MemoryMap()
	{
		if (dataCreator) {
			if (segment_allocator != nullptr) {
				segment_allocator->Deallocate(bytes[0]);
			} else {
				delete[] bytes[0];
			}
		}
		if (no_of_segments > smallWidthOpt) { delete[] bytes; }
	}
	MemoryMap(MemoryMap const&) = delete;
	MemoryMap(MemoryMap&&) = delete;
	MemoryMap& operator =(MemoryMap&&) = delete;
	MemoryMap& operator =(MemoryMap const&) = delete;

	// Method to initialize/re-initialize object
	// Fails if object is created as a data owner
	bool set(char* segment, int size) noexcept
	{
		if ((no_of_segments > 1) || dataCreator) return false;
		no_of_segments = 1;
		segment_size = size;
		bytes[0] = segment;
		return true;
	}

	char* getSegment(int i) const
	{
		if (i >= no_of_segments) throw MemoryMapException("Allocated MemoryMap too small!!!");
		return bytes[i];
	}
	int getSegmentSize() const noexcept
	{
		return segment_size;
	}
	int getNrOfSegments() const noexcept
	{
		return no_of_segments;
	}
	int getTotalSize() const noexcept
	{
		return no_of_segments*segment_size;
	}

	///Makes a copy of the content of this memory to dest. startIndex and endIndex are memory map relative. 
	bool copyToBytes(char* dest, int startIndex, int endIndex) const
	{
		const int bytesToCopy = endIndex - startIndex + 1;

		if ( (startIndex < 0) || (endIndex >= getTotalSize()) || (bytesToCopy <= 0) ) return false;

		// Since all segments are in one chunk after each other we can do a simple memcpy()
		memcpy(dest, bytes[0] + startIndex, bytesToCopy);
		return true;
	}

private:
    MemoryMapAllocator* segment_allocator{ nullptr };
    int no_of_segments{ 0 };
    int segment_size{ 0 };
    bool dataCreator{ false };
	char** bytes;
	static const int smallWidthOpt = 4;
    char* small_width_vector[smallWidthOpt]{ nullptr };
};

}
#endif
