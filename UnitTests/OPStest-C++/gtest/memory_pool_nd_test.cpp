/**
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

// Note: Doesn't work together with code/objects that uses memory_pool or memory_pool_manager
// since tests in this file have side effects in memory_pool_manager.
// Therefore this file need to be built into a separate binary

#include "gtest/gtest.h"

#include <memory>

#include "memory_pool.h"

using namespace ops;
using namespace memory_pools;

// ===============================
// Helper objects
class mem_pool_obj2 {
public:
	mem_pool_obj2() {}

	typedef ops::memory_pools::memory_pool_nd<mem_pool_obj2> memory_pool_type;

	void* operator new(size_t size) { return _pool.getEntry(size); }
	void operator delete(void *p) {	_pool.returnEntry(p); }

	static memory_pool_type _pool;

	~mem_pool_obj2() = default;
	mem_pool_obj2(const mem_pool_obj2& other) = delete;
	mem_pool_obj2& operator= (const mem_pool_obj2& other) = delete;
	mem_pool_obj2(mem_pool_obj2&& other) = delete;
	mem_pool_obj2& operator =(mem_pool_obj2&& other) = delete;
};

mem_pool_obj2::memory_pool_type mem_pool_obj2::_pool(2);

class mem_pool_log : public memory_pool_logger
{
public:
	virtual void Log(const char* const message, std::exception& e) override
	{
		std::cout << "[" << message << "] memory_pool Exception: " << e.what() << std::endl;
	}

	mem_pool_log() = default;
	~mem_pool_log() = default;
	mem_pool_log(const mem_pool_log& other) = delete;
	mem_pool_log& operator= (const mem_pool_log& other) = delete;
	mem_pool_log(mem_pool_log&& other) = delete;
	mem_pool_log& operator =(mem_pool_log&& other) = delete;
};
mem_pool_log mpl;

TEST(Test_memory_pools, Test_memory_pool_nd) {

	memory_pool_manager& mgr = memory_pool_manager::Instance();
	mgr.setLogger(&mpl);

	EXPECT_EQ(mgr.numPools(), 1);
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)2);

	EXPECT_THROW(mem_pool_obj2::_pool.getEntry(sizeof(mem_pool_obj2)+1), mem_pool_obj2::memory_pool_type::illegal_size);

//	mgr.PrintStat(std::cout)

	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)2);

	mem_pool_obj2* a = new mem_pool_obj2();
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)1);

	{
		std::unique_ptr<mem_pool_obj2> const b( new mem_pool_obj2());
		EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
		EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)0);

		EXPECT_THROW(new mem_pool_obj2(), mem_pool_obj2::memory_pool_type::out_of_space);
	}

	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)1);

	EXPECT_NO_THROW(mem_pool_obj2::_pool.checkException());

#ifdef OPS_MEMORY_POOL_INTEGRITY_CHECK
	mem_pool_obj2* dp = a;
	delete a;
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)2);

	delete dp;
	EXPECT_THROW(mem_pool_obj2::_pool.checkException(), mem_pool_obj2::memory_pool_type::pool_corruption);
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)2);

	EXPECT_NO_THROW(mem_pool_obj2::_pool.checkException());

	dp = (mem_pool_obj2*)(-1);
	delete dp;
	EXPECT_THROW(mem_pool_obj2::_pool.checkException(), mem_pool_obj2::memory_pool_type::illegal_ref);
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)2);

	a = new mem_pool_obj2();
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)1);

	// Try to destroy marker for check of pool corruption detection
	void* const rawptr = (void*)a;
	memset(rawptr, 1, sizeof(mem_pool_obj2) + 4);
	delete a;
	EXPECT_THROW(mem_pool_obj2::_pool.checkException(), mem_pool_obj2::memory_pool_type::pool_corruption);
	EXPECT_EQ(mem_pool_obj2::_pool.capacity(), (size_t)2);
	EXPECT_EQ(mem_pool_obj2::_pool.size(), (size_t)1);

	// Pool may now be in a corrupted state, Should not be used anymore.
#endif
}
