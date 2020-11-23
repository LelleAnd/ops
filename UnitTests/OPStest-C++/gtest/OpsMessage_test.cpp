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

#include "OPSMessage.h"
#include "XMLArchiverIn.h"

using namespace ops;

// ===============================

TEST(Test_OPSMessage, Test) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "ops.protocol.OPSMessage ");
	EXPECT_STREQ(obj1.getKey().c_str(), "");
	EXPECT_TRUE(obj1.isDataOwner());

    uint32_t addrRaw;
	Address_T addr;
	uint16_t port;
	obj1.getSource(addrRaw, port);
	EXPECT_EQ(port, 0);
	EXPECT_EQ(addrRaw, 0u);

    obj1.getSource(addr, port);
    EXPECT_EQ(port, 0);
    EXPECT_STREQ(addr.c_str(), "0.0.0.0");
    
    EXPECT_EQ(obj1.getPublicationID(), 0);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "");
	EXPECT_EQ(obj1.getData(), nullptr);

	// Test set/get
	obj1.setPublicationID(8888);
	obj1.setPublisherName("GTest");
	obj1.setTopicName("TestTopic");
	EXPECT_EQ(obj1.getPublicationID(), 8888);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "GTest");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "TestTopic");

	obj1.setSource(0x7F000001, 8765);
	obj1.getSource(addr, port);
	EXPECT_EQ(port, 8765);
	EXPECT_STREQ(addr.c_str(), "127.0.0.1");
}

// ===============================
// Helper classes

static int OpsObject_MessageTest_Cnt = 0;

class OpsObject_MessageTest : public OPSObject
{
public:
	std::string Message;

	OpsObject_MessageTest() : OPSObject()
	{
		appendType("OpsObject_MessageTest");
		++OpsObject_MessageTest_Cnt;
	}
	virtual ~OpsObject_MessageTest()
	{
		--OpsObject_MessageTest_Cnt;
	}
	virtual void serialize(ArchiverInOut* archive) override
	{
		OPSObject::serialize(archive);
		archive->inout("Message", Message);
	}

	virtual OpsObject_MessageTest* clone() override
	{ 
		OpsObject_MessageTest* const obj = new OpsObject_MessageTest();
		fillClone(obj);
		return obj; 
	}
	void fillClone(OpsObject_MessageTest* const obj) const
	{ 
		ops::OPSObject::fillClone(obj);
		obj->Message = Message;
	}

	OpsObject_MessageTest(const OpsObject_MessageTest& r) = delete;
	OpsObject_MessageTest& operator= (const OpsObject_MessageTest& l) = delete;
	OpsObject_MessageTest(OpsObject_MessageTest&&) = delete;
	OpsObject_MessageTest& operator =(OpsObject_MessageTest&&) = delete;
};

class OpsObject_MessageTest_Factory : public SerializableFactory
{
public:
	virtual Serializable* create(const TypeId_T& type) override
	{
		if (type == "OpsObject_MessageTest") { return new OpsObject_MessageTest(); }
		return nullptr;
	}

	OpsObject_MessageTest_Factory() = default;
	virtual ~OpsObject_MessageTest_Factory() = default;

	OpsObject_MessageTest_Factory(const OpsObject_MessageTest_Factory& r) = delete;
	OpsObject_MessageTest_Factory& operator= (const OpsObject_MessageTest_Factory& l) = delete;
	OpsObject_MessageTest_Factory(OpsObject_MessageTest_Factory&&) = delete;
	OpsObject_MessageTest_Factory& operator =(OpsObject_MessageTest_Factory&&) = delete;
};

class RAII_MessageTest_FactoryHelper
{
public:
	OpsObject_MessageTest_Factory fact1;
	SerializableInheritingTypeFactory fact;
	RAII_MessageTest_FactoryHelper()
	{
		fact.add(&fact1);
	}
	~RAII_MessageTest_FactoryHelper()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(fact.remove(&fact1));
	}
	RAII_MessageTest_FactoryHelper(const RAII_MessageTest_FactoryHelper& r) = delete;
	RAII_MessageTest_FactoryHelper& operator= (const RAII_MessageTest_FactoryHelper& l) = delete;
	RAII_MessageTest_FactoryHelper(RAII_MessageTest_FactoryHelper&&) = delete;
	RAII_MessageTest_FactoryHelper& operator =(RAII_MessageTest_FactoryHelper&&) = delete;
};

// ===============================
// Check dataOwner handling incl. getData/setData

TEST(Test_OPSMessage, Test_Data) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_TRUE(obj1.isDataOwner());
	EXPECT_EQ(obj1.getData(), nullptr);

	OpsObject_MessageTest* data = new OpsObject_MessageTest();
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting the same data again should not do anything
	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting another data should delete the original one
	obj1.setData(nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);
	EXPECT_EQ(obj1.getData(), nullptr);

	obj1.setDataOwner(false);
	EXPECT_FALSE(obj1.isDataOwner());

	data = new OpsObject_MessageTest();
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting the same data again should not do anything
	obj1.setData(data);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), data);

	// Setting another data should not delete the original since dataOwner is FALSE
	obj1.setData(nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
	EXPECT_EQ(obj1.getData(), nullptr);

	// Test that destructor doesn't delete object when message isn't a dataOwner
	{
		OPSMessage obj2;
		obj2.setDataOwner(false);
		obj2.setData(data);
		EXPECT_FALSE(obj2.isDataOwner());
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
		EXPECT_EQ(obj2.getData(), data);
	}
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

	// Test that destructor deletes object when message is a dataOwner
	{
		OPSMessage obj2;
		obj2.setData(data);
		EXPECT_TRUE(obj2.isDataOwner());
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
		EXPECT_EQ(obj2.getData(), data);
	}
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);
}

TEST(Test_OPSMessage, Test_Serialize) {

	// Default constructed 
	OPSMessage obj1;
	EXPECT_TRUE(obj1.isDataOwner());
	EXPECT_EQ(obj1.getData(), nullptr);
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);

	std::string content(
		" <root>"
		"	<messageType>A</messageType>"
		"	<publisherPriority>B</publisherPriority>"
		"   <publicationID>123456789</publicationID>"
		"	<publisherName>GTestPub</publisherName>"
		"	<topicName>TestTopic</topicName>"
		"	<topLevelKey>Key1</topLevelKey>"
		"   <data type = \"OpsObject_MessageTest\">"
		"     <Message>Kalle</Message>"
		"   </data>"
		" </root>"
		" "
	);
	std::istringstream is(content);

	RAII_MessageTest_FactoryHelper factory;
	XMLArchiverIn arcIn(is, "root", &factory.fact);
	obj1.serialize(&arcIn);
	EXPECT_STREQ(obj1.getTypeString().c_str(), "ops.protocol.OPSMessage ");
	EXPECT_STREQ(obj1.getKey().c_str(), "");
	EXPECT_EQ(obj1.getPublicationID(), 123456789);
	EXPECT_STREQ(obj1.getPublisherName().c_str(), "GTestPub");
	EXPECT_STREQ(obj1.getTopicName().c_str(), "TestTopic");
	ASSERT_NE(obj1.getData(), nullptr);

	OpsObject_MessageTest* const data = dynamic_cast<OpsObject_MessageTest*>(obj1.getData());
	ASSERT_NE(data, nullptr);
	EXPECT_STREQ(data->Message.c_str(), "Kalle");
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
}

TEST(Test_OPSMessage, Test_CopyMove) {

	{
		// Default constructed 
		OPSMessage obj1;
		obj1.setKey("Kalle");
		obj1.spareBytes.push_back('a');
		obj1.spareBytes.push_back('b');
		obj1.spareBytes.push_back('c');
		obj1.spareBytes.push_back('d');
		obj1.spareBytes.push_back('\0');

		EXPECT_STREQ(obj1.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj1.getTypeString().c_str(), "ops.protocol.OPSMessage ");
		EXPECT_EQ(obj1.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj1.spareBytes[0], "abcd");
		EXPECT_EQ(obj1.getData(), nullptr);

		// Copy constructed with no data
		OPSMessage obj2(obj1);
		EXPECT_STREQ(obj2.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj2.getTypeString().c_str(), "ops.protocol.OPSMessage ");
		EXPECT_EQ(obj2.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj2.spareBytes[0], "abcd");
		EXPECT_EQ(obj2.getData(), nullptr);

		// Create some data
		OpsObject_MessageTest* const data = new OpsObject_MessageTest();
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);

		obj1.setData(data);
		EXPECT_EQ(OpsObject_MessageTest_Cnt, 1);
		EXPECT_EQ(obj1.getData(), data);

		// Copy constructed with data
		OPSMessage obj3(obj1);
		EXPECT_STREQ(obj3.getKey().c_str(), "Kalle");
		EXPECT_STREQ(obj3.getTypeString().c_str(), "ops.protocol.OPSMessage ");
		EXPECT_EQ(obj3.spareBytes.size(), (size_t)5);
		EXPECT_STREQ((char*)&obj3.spareBytes[0], "abcd");
		EXPECT_NE(obj3.getData(), nullptr);

		EXPECT_EQ(OpsObject_MessageTest_Cnt, 2);
	}
	EXPECT_EQ(OpsObject_MessageTest_Cnt, 0);
}
