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

#include "gtest/gtest.h"

#include "OPSObject.h"
#include "OPSArchiverOut.h"

using namespace ops;

// ===============================
// Helper classes

class MyOpsObject : public OPSObject
{
public:
	MyOpsObject(): OPSObject()
	{ 
		appendType("MyOpsObject"); 
	}
	virtual ~MyOpsObject() = default;

    virtual MyOpsObject* clone() override
    {
        MyOpsObject* const obj = new MyOpsObject();
        fillClone(obj);
        return obj;
    }
    void fillClone(MyOpsObject* const obj) const
    {
        ops::OPSObject::fillClone(obj);
    }
    
    MyOpsObject(const MyOpsObject& r) = delete;
	MyOpsObject& operator= (const MyOpsObject& l) = delete;
	MyOpsObject(MyOpsObject&&) = delete;
	MyOpsObject& operator =(MyOpsObject&&) = delete;
};

// ===============================

TEST(Test_OPSObject, Test) {

	// Default constructed 
	OPSObject obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "");
	EXPECT_STREQ(obj1.getKey().c_str(), "");

	obj1.setKey("Kalle");
	EXPECT_STREQ(obj1.getKey().c_str(), "Kalle");

	// Test appendType
	MyOpsObject obj2;
	EXPECT_STREQ(obj2.getTypeString().c_str(), "MyOpsObject ");
	EXPECT_STREQ(obj2.getKey().c_str(), "");

	obj2.setKey("Pelle");
	EXPECT_STREQ(obj2.getKey().c_str(), "Pelle");

	// Test Clone, and indirectly fillClone()
	obj2.spareBytes.push_back('a');
	obj2.spareBytes.push_back('b');
	obj2.spareBytes.push_back('c');
	obj2.spareBytes.push_back('d');
	obj2.spareBytes.push_back('\0');

	OPSObject* const obj3 = obj2.clone();
	ASSERT_NE(obj3, nullptr);
	EXPECT_STREQ(obj3->getTypeString().c_str(), "MyOpsObject ");
	EXPECT_STREQ(obj3->getKey().c_str(), "Pelle");

	EXPECT_EQ(obj3->spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj3->spareBytes[0], "abcd");

    OPSObject* const obj4 = obj3->clone();
    ASSERT_NE(obj4, nullptr);
    EXPECT_STREQ(obj4->getTypeString().c_str(), "MyOpsObject ");
    EXPECT_STREQ(obj4->getKey().c_str(), "Pelle");

    EXPECT_EQ(obj4->spareBytes.size(), (size_t)5);
    EXPECT_STREQ((char*)&obj4->spareBytes[0], "abcd");

    delete obj4;
	delete obj3;

    MyOpsObject* const obj5 = obj2.clone();
    ASSERT_NE(obj5, nullptr);
    EXPECT_STREQ(obj5->getTypeString().c_str(), "MyOpsObject ");
    EXPECT_STREQ(obj5->getKey().c_str(), "Pelle");

    EXPECT_EQ(obj5->spareBytes.size(), (size_t)5);
    EXPECT_STREQ((char*)&obj5->spareBytes[0], "abcd");

    delete obj5;

	// Test Serialize
	// Note, Only key of OPSObject is serialized and MyOpsObject don't have any fields that is serialized
	MemoryMap map(1, 60);
	ByteBuffer buf(map);
	OPSArchiverOut arc(buf, false);
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	obj2.serialize(&arc);
	EXPECT_EQ(buf.GetSize(), 9);
	EXPECT_EQ(buf.GetIndex(), 9);

	buf.ResetIndex();
	EXPECT_STREQ(buf.ReadString().c_str(), "Pelle");
}

static OPSObject f(OPSObject o)
{
	return o;
}

TEST(Test_OPSObject, TestCopyMove) {

#if defined(DEBUG_OPSOBJECT_COUNTER)
	uint32_t start_value = ops::OPSObject::NumOpsObjects();
#endif

	{
		// Default constructed 
		MyOpsObject obj1;
		obj1.setKey("Kalle");
		obj1.spareBytes.push_back('a');
		obj1.spareBytes.push_back('b');
		obj1.spareBytes.push_back('c');
		obj1.spareBytes.push_back('d');
		obj1.spareBytes.push_back('\0');

		EXPECT_STREQ(obj1.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj1.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj1.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj1.spareBytes[0], "abcd");
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 1);
#endif

		// Copy constructed
		OPSObject obj2(obj1);
		EXPECT_STREQ(obj2.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj2.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj2.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj2.spareBytes[0], "abcd");
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 2);
#endif

		// Copy assignment
		OPSObject obj3;
		obj3 = obj2;
		EXPECT_STREQ(obj3.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj3.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj3.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj3.spareBytes[0], "abcd");
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 3);
#endif

		// Move constructor
		OPSObject obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
		EXPECT_STREQ(obj4.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj4.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj4.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj4.spareBytes[0], "abcd");

		EXPECT_STREQ(obj3.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj3.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj3.spareBytes.size(), (size_t)5);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 4);
#endif

		// Move assignment
		OPSObject obj5;
		obj5 = std::move(obj3);
		EXPECT_STREQ(obj5.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj5.getTypeString().c_str(), "MyOpsObject ");
		EXPECT_EQ(obj5.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj5.spareBytes[0], "abcd");

		// From cplusplus.com reference:
		// ... the value of the moved-from object should only be destroyed or assigned a new value; 
		// accessing it otherwise yields an unspecified value.
		// So skip tests on obj3

#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 5);
#endif
	}

#if defined(DEBUG_OPSOBJECT_COUNTER)
	EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value);
#endif
}
