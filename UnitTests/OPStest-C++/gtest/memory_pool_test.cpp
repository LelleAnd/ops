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

//
// Note: Doesn't work together with code/objects that uses memory_pool_nd since they
// require memory_pools_nd to be declared static at file level which will create a
// manager and update its variables. This will fail some tests in this file.
//

#include "gtest/gtest.h"

#include "memory_pool.h"

using namespace ops;
using namespace memory_pools;

// ===============================
// Helper objects

class memory_pool_stub : public memory_pool_abs
{
private:
	int _value;
public:
	memory_pool_stub(int value) : _value(value) {}

	void PrintStat(std::ostream& os) override
	{
		os << _value;
	}
};

void CheckMemoryPools(memory_pool_manager& mgr, std::string expected)
{
	std::stringstream ss;
	mgr.PrintStat(ss, true /* skip_header */);
	EXPECT_STREQ(ss.str().c_str(), expected.c_str());
}

// ===============================

TEST(Test_memory_pools, Test_manager) {

	memory_pool_manager& mgr = memory_pool_manager::Instance();
	EXPECT_EQ(mgr.numPools(), 0);
	CheckMemoryPools(mgr, "");

	{
		const memory_pool_stub mp1(11);
		const memory_pool_stub mp2(22);
		const memory_pool_stub mp3(33);

		EXPECT_EQ(mgr.numPools(), 3);
		CheckMemoryPools(mgr, "112233");
	}

	{
		const memory_pool_stub mp1(11);
		memory_pool_stub* mp2 = new memory_pool_stub(44);
		const memory_pool_stub mp3(33);

		EXPECT_EQ(mgr.numPools(), 3);
		CheckMemoryPools(mgr, "114433");

		delete mp2; mp2 = nullptr;

		EXPECT_EQ(mgr.numPools(), 2);
		CheckMemoryPools(mgr, "1133");
	}

	memory_pool_stub* mp2 = nullptr;
	{
		const memory_pool_stub mp1(11);
		mp2 = new memory_pool_stub(99);
		const memory_pool_stub mp3(33);

		EXPECT_EQ(mgr.numPools(), 3);
		CheckMemoryPools(mgr, "119933");
	}

	EXPECT_EQ(mgr.numPools(), 1);
	CheckMemoryPools(mgr, "99");

	delete mp2;
	mp2 = nullptr;

	EXPECT_EQ(mgr.numPools(), 0);
	CheckMemoryPools(mgr, "");
}

// ===============================
// Helper objects
class mem_pool_obj1 {
public:
	mem_pool_obj1() {}

};

TEST(Test_memory_pools, Test_memory_pool) {

	memory_pool_manager& mgr = memory_pool_manager::Instance();
	EXPECT_EQ(mgr.numPools(), 0);
	CheckMemoryPools(mgr, "");

	EXPECT_THROW(memory_pool<mem_pool_obj1> mp1(0), memory_pool<mem_pool_obj1>::out_of_space);
	EXPECT_EQ(mgr.numPools(), 0);

	memory_pool<mem_pool_obj1> mp1(2);
	EXPECT_EQ(mgr.numPools(), 1);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)2);

	mem_pool_obj1* o1 = mp1.getEntry();
	EXPECT_NE(o1, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	mem_pool_obj1* o2 = mp1.getEntry();
	EXPECT_NE(o2, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)0);

	EXPECT_THROW(mp1.getEntry(), memory_pool<mem_pool_obj1>::out_of_space);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)0);

	mem_pool_obj1* cpy = o1;

	mp1.returnEntry(o1);
	EXPECT_EQ(o1, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	EXPECT_THROW(mp1.returnEntry(cpy), memory_pool<mem_pool_obj1>::pool_corruption);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

#ifdef OPS_MEMORY_POOL_INTEGRITY_CHECK
	EXPECT_THROW(mp1.returnEntry(o1), memory_pool<mem_pool_obj1>::illegal_ref);
	EXPECT_EQ(o1, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	o1 = (mem_pool_obj1*)(-1);
	EXPECT_THROW(mp1.returnEntry(o1), memory_pool<mem_pool_obj1>::illegal_ref);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	// Try to destroy marker for check of pool corruption detection
	void* rawptr = (void*)o2;
	memset(rawptr, 1, sizeof(mem_pool_obj1) + 4);
	EXPECT_THROW(mp1.returnEntry(o2), memory_pool<mem_pool_obj1>::pool_corruption);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);
#endif
}

TEST(Test_memory_pools, Test_memory_pool_exp) {

	memory_pool_manager& mgr = memory_pool_manager::Instance();
	EXPECT_EQ(mgr.numPools(), 0);
	CheckMemoryPools(mgr, "");

	memory_pool_exp<128> mp1(2);
	EXPECT_EQ(mgr.numPools(), 1);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)2);

	char* o1 = mp1.getEntry();
	EXPECT_NE(o1, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	char* o2 = mp1.getEntry();
	EXPECT_NE(o2, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)0);

	char* o3 = mp1.getEntry();
	EXPECT_NE(o3, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)0);

	mp1.returnEntry(o1);
	EXPECT_EQ(o1, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)1);

	mp1.returnEntry(o2);
	EXPECT_EQ(o2, nullptr);
	EXPECT_EQ(mp1.capacity(), (size_t)2);
	EXPECT_EQ(mp1.size(), (size_t)2);

	mp1.returnEntry(o3);
	EXPECT_EQ(o3, nullptr);
	EXPECT_GE(mp1.capacity(), (size_t)3);
	EXPECT_EQ(mp1.size(), (size_t)3);

	EXPECT_THROW(mp1.returnEntry(o3), memory_pool_exp<128>::illegal_ref);
	EXPECT_GE(mp1.capacity(), (size_t)3);
	EXPECT_EQ(mp1.size(), (size_t)3);

}
