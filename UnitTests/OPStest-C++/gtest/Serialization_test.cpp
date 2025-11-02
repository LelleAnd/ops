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

// TODO: Not all exception cases are tested. 
//

#include <sstream>

#include "gtest/gtest.h"

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include "OPSArchiverIn.h"
#include "OPSArchiverOut.h"
#include "XMLArchiverIn.h"
#include "XMLArchiverOut.h"

#include "SerDesObjects.h"

using namespace ops;

// ===============================
// Helper classes

int InitObject(SerDesObject_Core& obj)
{
	int size = 4 + (int)obj.getKey().size();     // From OPSObject()
	obj.bo = true; size += 1;
	obj.ch = 'm'; size += 1;
	obj.u8 = 233; size += 1;
	obj.i16 = -456; size += 2;
	obj.i32 = 3223; size += 4;
	obj.i64 = 99999; size += 8;
	obj.f32 = 678.98f; size += 4;
	obj.d64 = 123456789.0; size += 8;
	obj.str = "Test std::string"; size += 4 + (int)obj.str.size();
	obj.fstr = "Test strings:fixed_string..."; size += 4 + (int)obj.fstr.size();
	for (int i = 0; i < (int)sizeof(obj.buffer); i++) { obj.buffer[i] = (char)i; }
	size += (int)sizeof(obj.buffer);
	return size;
}

void ExpectObjects_EQ(SerDesObject_Core& obj1, SerDesObject_Core& obj2, const std::string msg)
{
	EXPECT_EQ(obj1.bo, obj2.bo) << msg;
	EXPECT_EQ(obj1.ch, obj2.ch) << msg;
	EXPECT_EQ(obj1.u8, obj2.u8) << msg;
	EXPECT_EQ(obj1.i16, obj2.i16) << msg;
	EXPECT_EQ(obj1.i32, obj2.i32) << msg;
	EXPECT_EQ(obj1.i64, obj2.i64) << msg;
	EXPECT_EQ(obj1.f32, obj2.f32) << msg;
	EXPECT_EQ(obj1.d64, obj2.d64) << msg;
	EXPECT_STREQ(obj1.str.c_str(), obj2.str.c_str()) << msg;
	EXPECT_STREQ(obj1.fstr.c_str(), obj2.fstr.c_str()) << msg;
	for (int i = 0; i < (int)sizeof(obj1.buffer); i++) {
		EXPECT_EQ(obj1.buffer[i], obj2.buffer[i]) << msg;
	}
}

TEST(Test_Serialization, TestCoreTypes) {

	SerDesObject_Core obj1, obj2;
	int numSerBytes = InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	MemoryMap map(1, 1000);
	ByteBuffer buf(map);
	OPSArchiverOut arcOut(buf, false);
	EXPECT_TRUE(arcOut.isOut());
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	obj1.serialize(&arcOut);
	EXPECT_EQ(buf.GetSize(), numSerBytes);
	EXPECT_EQ(buf.GetIndex(), numSerBytes);

	buf.ResetIndex();

	OPSArchiverIn arcIn(buf, nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

TEST(Test_Serialization, TestCoreTypesXml) {

	SerDesObject_Core obj1, obj2;
	InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	std::ostringstream os;

	XMLArchiverOut arcOut(os, "root");
	EXPECT_TRUE(arcOut.isOut());

	obj1.serialize(&arcOut);
	arcOut.close();

	//std::cout << "[>>]" << os.str() << "[<<]\n";

	std::istringstream is(os.str());

	XMLArchiverIn arcIn(is, "root", nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

// ===============================
// Helper classes

int InitObject(SerDesObject_Vectors& obj)
{
	int size = 4 + (int)obj.getKey().size();     // From OPSObject()
	obj.bo = { true, false, false, true }; size += 4 + (4 * 1);
	obj.ch = { 'm', 's' }; size += 4 + (2 * 1);
	obj.u8 = { 22, 199, 111 }; size += 4 + (3 * 1);
	obj.i16 = { -456 }; size += 4 + (1 * 2);
	obj.i32 = { 3223, -987, 123 }; size += 4 + (3 * 4);
	obj.i64 = { 99999, -7777 }; size += 4 + (2 * 8);
	obj.f32 = { 678.98f, -4.654f }; size += 4 + (2 * 4);
	obj.d64 = { 123456789.0, -999.87654 }; size += 4 + (2 * 8);
	obj.str = { "Test", "std::string" }; size += 4 + (4 + (int)obj.str[0].size()) + (4 + (int)obj.str[1].size());
	obj.fstr = { "Test", "strings:fixed_string..." }; size += 4 + (4 + (int)obj.fstr[0].size()) + (4 + (int)obj.fstr[1].size());
	return size;
}

void ExpectObjects_EQ(SerDesObject_Vectors& obj1, SerDesObject_Vectors& obj2, const std::string msg)
{
	EXPECT_EQ(obj1.bo, obj2.bo) << msg;
	EXPECT_EQ(obj1.ch, obj2.ch) << msg;
	EXPECT_EQ(obj1.u8, obj2.u8) << msg;
	EXPECT_EQ(obj1.i16, obj2.i16) << msg;
	EXPECT_EQ(obj1.i32, obj2.i32) << msg;
	EXPECT_EQ(obj1.i64, obj2.i64) << msg;
	EXPECT_EQ(obj1.f32, obj2.f32) << msg;
	EXPECT_EQ(obj1.d64, obj2.d64) << msg;
	EXPECT_EQ(obj1.str, obj2.str) << msg;
	for (unsigned int i = 0; i < obj1.fstr.size(); i++) {
		EXPECT_STREQ(obj1.fstr[i].c_str(), obj2.fstr[i].c_str()) << msg;
	}
}


TEST(Test_Serialization, TestVectorTypes) {

	SerDesObject_Vectors obj1, obj2;
	int numSerBytes = InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	MemoryMap map(1, 1000);
	ByteBuffer buf(map);
	OPSArchiverOut arcOut(buf, false);
	EXPECT_TRUE(arcOut.isOut());
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	obj1.serialize(&arcOut);
	EXPECT_EQ(buf.GetSize(), numSerBytes);
	EXPECT_EQ(buf.GetIndex(), numSerBytes);

	buf.ResetIndex();

	OPSArchiverIn arcIn(buf, nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

TEST(Test_Serialization, TestVectorTypesXml) {

	SerDesObject_Vectors obj1, obj2;
	InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	std::ostringstream os;

	XMLArchiverOut arcOut(os, "root");
	EXPECT_TRUE(arcOut.isOut());
	
	obj1.serialize(&arcOut);
	arcOut.close();

	//std::cout << "[>>]" << os.str() << "[<<]\n";

	std::istringstream is(os.str());

	XMLArchiverIn arcIn(is, "root", nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

// ===============================
// Helper classes

int InitObject(SerDesObject_Fixarrays& obj)
{
	int size = 4 + (int)obj.getKey().size();     // From OPSObject()
	obj.bo[0] = true;  obj.bo[1] = false;  obj.bo[2] = false; obj.bo[3] = true;
	size += 4 + (4 * 1);

	obj.ch[0] = 'm';  obj.ch[1] = 's';
	size += 4 + (2 * 1);
	
	obj.u8[0] = 22;  obj.u8[1] = 190; obj.u8[2] = 45;
	size += 4 + (3 * 1);

	obj.i16[0] = -456;
	size += 4 + (1 * 2);
	
	obj.i32[0] = 3223;  obj.i32[1] = -987;  obj.i32[2] = 123;
	size += 4 + (3 * 4);

	obj.i64[0] = 99999;  obj.i64[1] = -7777;
	size += 4 + (2 * 8);

	obj.f32[0] = 678.98f;  obj.f32[1] = -4.654f;
	size += 4 + (2 * 4);

	obj.d64[0] = 123456789.0;  obj.d64[1] = -999.87654;
	size += 4 + (2 * 8);

	obj.str[0] = "Test";  obj.str[1] = "std::string";
	size += 4 + (4 + (int)obj.str[0].size()) + (4 + (int)obj.str[1].size());

	obj.fstr[0] = "Test";
	obj.fstr[1] = "strings:fixed_string...";
    size += 4 + (4 + (int)obj.fstr[0].size()) + (4 + (int)obj.fstr[1].size());
	return size;
}

void ExpectObjects_EQ(SerDesObject_Fixarrays& obj1, SerDesObject_Fixarrays& obj2, const std::string msg)
{
	for (unsigned int i = 0; i < 4; i++) { 
		EXPECT_EQ(obj1.bo[i], obj2.bo[i]) << msg; 
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_EQ(obj1.ch[i], obj2.ch[i]) << msg;
	}
	for (unsigned int i = 0; i < 3; i++) {
		EXPECT_EQ(obj1.u8[i], obj2.u8[i]) << msg;
	}
	for (unsigned int i = 0; i < 1; i++) {
		EXPECT_EQ(obj1.i16[i], obj2.i16[i]) << msg;
	}
	for (unsigned int i = 0; i < 3; i++) {
		EXPECT_EQ(obj1.i32[i], obj2.i32[i]) << msg;
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_EQ(obj1.i64[i], obj2.i64[i]) << msg;
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_EQ(obj1.f32[i], obj2.f32[i]) << msg;
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_EQ(obj1.d64[i], obj2.d64[i]) << msg;
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_STREQ(obj1.str[i].c_str(), obj2.str[i].c_str()) << msg;
	}
	for (unsigned int i = 0; i < 2; i++) {
		EXPECT_STREQ(obj1.fstr[i].c_str(), obj2.fstr[i].c_str()) << msg;
	}
}

TEST(Test_Serialization, TestFixedArrays) {

	SerDesObject_Fixarrays obj1, obj2;
	int numSerBytes = InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	MemoryMap map(1, 1000);
	ByteBuffer buf(map);
	OPSArchiverOut arcOut(buf, false);
	EXPECT_TRUE(arcOut.isOut());
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	obj1.serialize(&arcOut);
	EXPECT_EQ(buf.GetSize(), numSerBytes);
	EXPECT_EQ(buf.GetIndex(), numSerBytes);

	buf.ResetIndex();

	OPSArchiverIn arcIn(buf, nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

TEST(Test_Serialization, TestFixedArraysXml) {

	SerDesObject_Fixarrays obj1, obj2;
	InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	std::ostringstream os;

	XMLArchiverOut arcOut(os, "root");
	EXPECT_TRUE(arcOut.isOut());

	obj1.serialize(&arcOut);
	arcOut.close();

	std::istringstream is(os.str());

	XMLArchiverIn arcIn(is, "root", nullptr);		// No factory needed since we only have core types
	EXPECT_FALSE(arcIn.isOut());

	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");
}

// ===============================
// Helper classes

class ObjectFactory_SerDesObjects : public SerializableFactory
{
public:
	virtual Serializable* create(const TypeId_T& type) override
	{
		if (type == SerDesObject_Core::getTypeName()) { return new SerDesObject_Core(); }
		return nullptr;
	}
	ObjectFactory_SerDesObjects() = default;
	virtual ~ObjectFactory_SerDesObjects() = default;

	ObjectFactory_SerDesObjects(const ObjectFactory_SerDesObjects& r) = delete;
	ObjectFactory_SerDesObjects& operator= (const ObjectFactory_SerDesObjects& l) = delete;
	ObjectFactory_SerDesObjects(ObjectFactory_SerDesObjects&&) = delete;
	ObjectFactory_SerDesObjects& operator =(ObjectFactory_SerDesObjects&&) = delete;
};

class RAII_FactoryHelper
{
public:
	ObjectFactory_SerDesObjects fact1;
	SerializableInheritingTypeFactory fact;
	RAII_FactoryHelper()
	{
		fact.add(&fact1);
	}
	~RAII_FactoryHelper()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(fact.remove(&fact1));
	}

    RAII_FactoryHelper(const RAII_FactoryHelper&) = delete;
    RAII_FactoryHelper(RAII_FactoryHelper&&) = delete;
    RAII_FactoryHelper& operator=(const RAII_FactoryHelper&) = delete;
    RAII_FactoryHelper& operator=(RAII_FactoryHelper&&) = delete;
};

int InitObject(SerDesObject_Serializables& obj)
{
	int size = 4 + (int)obj.getKey().size();				// From OPSObject()
	
	size += 4 + (int)obj.obj1.getTypeString().size();	// Objects have their type first
	size += InitObject(obj.obj1);

	size += 4 + (int)obj.obj2->getTypeString().size();	// Objects have their type first
	size += InitObject(*obj.obj2);

	size += 4 + (int)obj.obj3->getTypeString().size();	// Objects have their type first
	size += InitObject(*obj.obj3);

	obj.vobj1.resize(2);
	size += 4;											// Vector size
	size += 4 + (int)obj.vobj1[0].getTypeString().size();	// Objects have their type first
	size += InitObject(obj.vobj1[0]);
	size += 4 + (int)obj.vobj1[1].getTypeString().size();	// Objects have their type first
	size += InitObject(obj.vobj1[1]);

	obj.vobj2.push_back(new SerDesObject_Core());
	size += 4;											// Vector size
	size += 4 + (int)obj.vobj2[0]->getTypeString().size();	// Objects have their type first
	size += InitObject(*obj.vobj2[0]);

	size += 4;											// Vector size
	size += 4 + (int)obj.fixarr1[0].getTypeString().size();	// Objects have their type first
	size += InitObject(obj.fixarr1[0]);
	size += 4 + (int)obj.fixarr1[1].getTypeString().size();	// Objects have their type first
	size += InitObject(obj.fixarr1[1]);

	size += 4;											// Vector size
	size += 4 + (int)obj.fixarr2[0]->getTypeString().size();	// Objects have their type first
	size += InitObject(*obj.fixarr2[0]);
	size += 4 + (int)obj.fixarr2[1]->getTypeString().size();	// Objects have their type first
	size += InitObject(*obj.fixarr2[1]);

	return size;
}

void ExpectObjects_EQ(SerDesObject_Serializables& obj1, SerDesObject_Serializables& obj2, const std::string msg)
{
	ExpectObjects_EQ(obj1.obj1, obj2.obj1, msg);

	EXPECT_NE(obj1.obj2, nullptr) << msg;
	EXPECT_NE(obj2.obj2, nullptr) << msg;
	ExpectObjects_EQ(*obj1.obj2, *obj2.obj2, msg);

	EXPECT_NE(obj1.obj3, nullptr) << msg;
	EXPECT_NE(obj2.obj3, nullptr) << msg;
	ExpectObjects_EQ(*obj1.obj3, *obj2.obj3, msg);

	EXPECT_EQ(obj1.vobj1.size(), obj2.vobj1.size()) << msg;
	for (unsigned int i = 0; i < obj1.vobj1.size(); i++) {
		ExpectObjects_EQ(obj1.vobj1[i], obj2.vobj1[i], msg);
	}

	EXPECT_EQ(obj1.vobj2.size(), obj2.vobj2.size()) << msg;
	for (unsigned int i = 0; i < obj1.vobj2.size(); i++) {
		ExpectObjects_EQ(*obj1.vobj2[i], *obj2.vobj2[i], msg);
	}

	ExpectObjects_EQ(obj1.fixarr1[0], obj2.fixarr1[0], msg);
	ExpectObjects_EQ(obj1.fixarr1[1], obj2.fixarr1[1], msg);

	EXPECT_NE(obj1.fixarr2[0], nullptr) << msg;
	EXPECT_NE(obj2.fixarr2[0], nullptr) << msg;
	ExpectObjects_EQ(*obj1.fixarr2[0], *obj2.fixarr2[0], msg);
	EXPECT_NE(obj1.fixarr2[1], nullptr) << msg;
	EXPECT_NE(obj2.fixarr2[1], nullptr) << msg;
	ExpectObjects_EQ(*obj1.fixarr2[1], *obj2.fixarr2[1], msg);
}

TEST(Test_Serialization, TestObjects) {

	SerDesObject_Serializables obj1, obj2;
	int numSerBytes = InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	MemoryMap map(1, 4000);
	ByteBuffer buf(map);
	OPSArchiverOut arcOut(buf, false);
	EXPECT_TRUE(arcOut.isOut());
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	obj1.serialize(&arcOut);
	EXPECT_EQ(buf.GetSize(), numSerBytes);
	EXPECT_EQ(buf.GetIndex(), numSerBytes);

	RAII_FactoryHelper factory;
	OPSArchiverIn arcIn(buf, &factory.fact);
	EXPECT_FALSE(arcIn.isOut());

	// Test Exceptions
	buf.ResetIndex();
	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");

	buf.ResetIndex();
	obj2.testException1 = 1;
	EXPECT_THROW(obj2.serialize(&arcIn), ArchiverException);

	buf.ResetIndex();
	obj2.testException1 = 0;
	EXPECT_NO_THROW(obj2.serialize(&arcIn));
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");

	buf.ResetIndex();
	obj2.testException2 = 1;
	EXPECT_THROW(obj2.serialize(&arcIn), ArchiverException);
}

TEST(Test_Serialization, TestObjectsXml) {

	SerDesObject_Serializables obj1, obj2;
	InitObject(obj1);
	ExpectObjects_EQ(obj1, obj1, "Comparing object with itself");

	std::ostringstream os;

	XMLArchiverOut arcOut(os, "root");
	EXPECT_TRUE(arcOut.isOut());

	obj1.serialize(&arcOut);
	arcOut.close();

	std::istringstream is(os.str());

	RAII_FactoryHelper factory;
	XMLArchiverIn arcIn(is, "root", &factory.fact);
	EXPECT_FALSE(arcIn.isOut());

	// Test Exceptions
	obj2.serialize(&arcIn);
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");

	arcIn.reset();
	obj2.testException1 = 1;
	EXPECT_THROW(obj2.serialize(&arcIn), ArchiverException);

	arcIn.reset();
	obj2.testException1 = 0;
	EXPECT_NO_THROW(obj2.serialize(&arcIn));
	ExpectObjects_EQ(obj1, obj2, "Comparing serialized object");

	arcIn.reset();
	obj2.testException2 = 1;
	EXPECT_THROW(obj2.serialize(&arcIn), ArchiverException);
}
