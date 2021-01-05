/**
*
* Copyright (C) 2018-2020 Lennart Andersson.
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

#include "TopicInfoData.h"
#include "ParticipantInfoData.h"
#include "XMLArchiverIn.h"

using namespace ops;

///=========================================================================

class TopicInfoDataTestFixture : public ::testing::Test {
protected:

	void SetUp() {}

	void TearDown() {}

	TopicInfoData f(TopicInfoData o)
	{
		return o;
	}

};

TEST_F(TopicInfoDataTestFixture, Test) {

	// Default constructed
	TopicInfoData obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "TopicInfoData ");
	EXPECT_STREQ(obj1.name.c_str(), "");
	EXPECT_STREQ(obj1.type.c_str(), "");
	EXPECT_STREQ(obj1.transport.c_str(), "");
	EXPECT_STREQ(obj1.address.c_str(), "");
	EXPECT_EQ(obj1.port, 0);
	EXPECT_EQ(obj1.keys.size(), (size_t)0);

	// Set some values for copy test below
	obj1.name = "Kalle";
	obj1.type = "Test";
	obj1.transport = "mail";
	obj1.address = "127";
	obj1.port = 6789;
	obj1.keys.push_back("hejhopp");

	// Copy constructed
	TopicInfoData const obj2(obj1);
	EXPECT_STREQ(obj2.getTypeString().c_str(), "TopicInfoData ");
	EXPECT_STREQ(obj2.name.c_str(), "Kalle");
	EXPECT_STREQ(obj2.type.c_str(), "Test");
	EXPECT_STREQ(obj2.transport.c_str(), "mail");
	EXPECT_STREQ(obj2.address.c_str(), "127");
	EXPECT_EQ(obj2.port, 6789);
	ASSERT_EQ(obj2.keys.size(), (size_t)1);
	EXPECT_STREQ(obj2.keys[0].c_str(), "hejhopp");

	// Copy Assignment
	TopicInfoData obj3;
	obj3 = obj2;
	EXPECT_STREQ(obj3.getTypeString().c_str(), "TopicInfoData ");
	EXPECT_STREQ(obj3.name.c_str(), "Kalle");
	ASSERT_EQ(obj3.keys.size(), (size_t)1);
	EXPECT_STREQ(obj3.keys[0].c_str(), "hejhopp");

	// Move constructed
	TopicInfoData obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	EXPECT_STREQ(obj4.getTypeString().c_str(), "TopicInfoData ");
	EXPECT_STREQ(obj4.name.c_str(), "Kalle");
	ASSERT_EQ(obj4.keys.size(), (size_t)1);
	EXPECT_STREQ(obj4.keys[0].c_str(), "hejhopp");

	// Move assignment
	TopicInfoData obj5;
	obj5 = std::move(obj3);
	EXPECT_STREQ(obj5.getTypeString().c_str(), "TopicInfoData ");
	EXPECT_STREQ(obj5.name.c_str(), "Kalle");
	ASSERT_EQ(obj5.keys.size(), (size_t)1);
	EXPECT_STREQ(obj5.keys[0].c_str(), "hejhopp");
}

TEST_F(TopicInfoDataTestFixture, Test_Clone) {

	TopicInfoData obj1;
	obj1.name = "John";
	obj1.type = "Simon";
	obj1.transport = "udp";
	obj1.address = "127.0.0.1";
	obj1.port = 8888;
	obj1.keys.push_back("Adam");

	std::unique_ptr<TopicInfoData> obj2(obj1.clone());

	EXPECT_STREQ(obj2->name.c_str(), "John");
	EXPECT_STREQ(obj2->type.c_str(), "Simon");
	EXPECT_STREQ(obj2->transport.c_str(), "udp");
	EXPECT_STREQ(obj2->address.c_str(), "127.0.0.1");
	EXPECT_EQ(obj2->port, 8888);
	ASSERT_EQ(obj2->keys.size(), (size_t)1);
	EXPECT_STREQ(obj2->keys[0].c_str(), "Adam");
}

TEST_F(TopicInfoDataTestFixture, Test_Serialize) {

	TopicInfoData obj1;
	{
		std::string content(
			" <root>"
			"	<name>TestTopic</name>"
			"	<type>GTest.TestData</type>"
			"   <transport></transport>"
			"	<address>127.0.0.1</address>"
			"	<port>6689</port>"
			"   <keys>"
			"     <element>key1</element>"
			"     <element>key2</element>"
			"     <element>key3</element>"
			"   </keys>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.name.c_str(), "TestTopic");
		EXPECT_STREQ(obj1.type.c_str(), "GTest.TestData");
		EXPECT_STREQ(obj1.transport.c_str(), "");
		EXPECT_STREQ(obj1.address.c_str(), "127.0.0.1");
		EXPECT_EQ(obj1.port, 6689);
		ASSERT_EQ(obj1.keys.size(), (size_t)3);
		EXPECT_STREQ(obj1.keys[0].c_str(), "key1");
		EXPECT_STREQ(obj1.keys[1].c_str(), "key2");
		EXPECT_STREQ(obj1.keys[2].c_str(), "key3");
	}
}

///=========================================================================

class ParticipantInfoDataTestFixture : public ::testing::Test {
protected:

	void SetUp() {}

	void TearDown()	{}

	ParticipantInfoData f(ParticipantInfoData o)
	{
		return o;
	}

};

TEST_F(ParticipantInfoDataTestFixture, Test) {

#if defined(DEBUG_OPSOBJECT_COUNTER)
	uint32_t start_value = ops::OPSObject::NumOpsObjects();
#endif
	{
		// Default constructed
		ParticipantInfoData obj1;
		EXPECT_STREQ(obj1.name.c_str(), "");
		EXPECT_STREQ(obj1.id.c_str(), "");
		EXPECT_STREQ(obj1.domain.c_str(), "");
		EXPECT_STREQ(obj1.ip.c_str(), "");
		EXPECT_STREQ(obj1.languageImplementation.c_str(), "");
		EXPECT_STREQ(obj1.opsVersion.c_str(), "");
		EXPECT_EQ(obj1.mc_udp_port, 0);
		EXPECT_EQ(obj1.mc_tcp_port, 0);
		EXPECT_EQ(obj1.subscribeTopics.size(), (size_t)0);
		EXPECT_EQ(obj1.publishTopics.size(), (size_t)0);
		EXPECT_EQ(obj1.knownTypes.size(), (size_t)0);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 1);
#endif

		// Add some data
		Topic top;
		top.setPort(25000);
		top.setDomainAddress("127.0.0.1");
		obj1.subscribeTopics.push_back(TopicInfoData(top));
		top.setPort(26000);
		obj1.publishTopics.push_back(TopicInfoData(top));
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 4);
#endif

		// Copy constructed
		ParticipantInfoData const obj2(obj1);
		ASSERT_EQ(obj2.subscribeTopics.size(), (size_t)1);
		EXPECT_EQ(obj2.subscribeTopics[0].port, 25000);
		ASSERT_EQ(obj2.publishTopics.size(), (size_t)1);
		EXPECT_EQ(obj2.publishTopics[0].port, 26000);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 7);
#endif

		// Copy Assignment
		ParticipantInfoData obj3;
		obj3 = obj2;
		ASSERT_EQ(obj3.subscribeTopics.size(), (size_t)1);
		EXPECT_EQ(obj3.subscribeTopics[0].port, 25000);
		ASSERT_EQ(obj3.publishTopics.size(), (size_t)1);
		EXPECT_EQ(obj3.publishTopics[0].port, 26000);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 10);
#endif

		// Move constructed
		ParticipantInfoData obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
		ASSERT_EQ(obj4.subscribeTopics.size(), (size_t)1);
		EXPECT_EQ(obj4.subscribeTopics[0].port, 25000);
		ASSERT_EQ(obj4.publishTopics.size(), (size_t)1);
		EXPECT_EQ(obj4.publishTopics[0].port, 26000);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 13);
#endif

		// Move assignment
		ParticipantInfoData obj5;
		obj5 = std::move(obj3);
		ASSERT_EQ(obj5.subscribeTopics.size(), (size_t)1);
		EXPECT_EQ(obj5.subscribeTopics[0].port, 25000);
		ASSERT_EQ(obj5.publishTopics.size(), (size_t)1);
		EXPECT_EQ(obj5.publishTopics[0].port, 26000);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value + 14);
#endif
	}
#if defined(DEBUG_OPSOBJECT_COUNTER)
	EXPECT_EQ(ops::OPSObject::NumOpsObjects(), start_value);
#endif
}

TEST_F(ParticipantInfoDataTestFixture, Test_Clone) {

	ParticipantInfoData obj1;
	obj1.name = "A";
	obj1.id = "B";
	obj1.domain = "C";
	obj1.ip = "D";
	obj1.languageImplementation = "E";
	obj1.opsVersion = "F";
	obj1.mc_udp_port = 7777;
	obj1.mc_tcp_port = 9999;

	Topic top;
	top.setPort(77);
	obj1.subscribeTopics.push_back(TopicInfoData(top));
	top.setPort(88);
	obj1.publishTopics.push_back(TopicInfoData(top));
	obj1.knownTypes.push_back("myType");

	std::unique_ptr<ParticipantInfoData> obj2(obj1.clone());
	EXPECT_STREQ(obj2->name.c_str(), "A");
	EXPECT_STREQ(obj2->id.c_str(), "B");
	EXPECT_STREQ(obj2->domain.c_str(), "C");
	EXPECT_STREQ(obj2->ip.c_str(), "D");
	EXPECT_STREQ(obj2->languageImplementation.c_str(), "E");
	EXPECT_STREQ(obj2->opsVersion.c_str(), "F");
	EXPECT_EQ(obj2->mc_udp_port, 7777);
	EXPECT_EQ(obj2->mc_tcp_port, 9999);
	ASSERT_EQ(obj2->subscribeTopics.size(), (size_t)1);
	EXPECT_EQ(obj2->subscribeTopics[0].port, 77);
	ASSERT_EQ(obj2->publishTopics.size(), (size_t)1);
	EXPECT_EQ(obj2->publishTopics[0].port, 88);
	ASSERT_EQ(obj2->knownTypes.size(), (size_t)1);
	EXPECT_STREQ(obj2->knownTypes[0].c_str(), "myType");
}

TEST_F(ParticipantInfoDataTestFixture, Test_Serialize) {

	ParticipantInfoData obj1;

	std::string content(
		" <root>"
		"	<name>Test</name>"
		"	<id>nnnn</id>"
		"	<domain>Testdomain</domain>"
		"	<ip>127.0.0.1</ip>"
		"	<languageImplementation>C++</languageImplementation>"
		"	<opsVersion>4</opsVersion>"
		"	<mc_udp_port>11111</mc_udp_port>"
		"	<mc_tcp_port>22222</mc_tcp_port>"
		"	<subscribeTopics>"
		"		<element type = \"TopicInfoData\">"
		"	        <name>TestTopic</name>"
		"	        <type>GTest.TestData</type>"
		"           <transport>tcp</transport>"
		"	        <address>127.0.0.1</address>"
		"	        <port>6689</port>"
		"		</element>"
		"	</subscribeTopics>"
		"	<publishTopics>"
		"		<element type = \"TopicInfoData\">"
		"	        <name>TestTopic2</name>"
		"	        <type>GTest.TestData</type>"
		"           <transport>udp</transport>"
		"	        <address>127.0.0.1</address>"
		"	        <port>6670</port>"
		"		</element>"
		"	</publishTopics>"
		"   <knownTypes>"
		"     <element>type1</element>"
		"     <element>type2</element>"
		"     <element>type3</element>"
		"   </knownTypes>"
		" </root>"
		" "
	);
	std::istringstream is(content);
	XMLArchiverIn arcIn(is, "root", nullptr);
	obj1.serialize(&arcIn);
	EXPECT_STREQ(obj1.name.c_str(), "Test");
	EXPECT_STREQ(obj1.id.c_str(), "nnnn");
	EXPECT_STREQ(obj1.domain.c_str(), "Testdomain");
	EXPECT_STREQ(obj1.ip.c_str(), "127.0.0.1");
	EXPECT_STREQ(obj1.languageImplementation.c_str(), "C++");
	EXPECT_STREQ(obj1.opsVersion.c_str(), "4");
	EXPECT_EQ(obj1.mc_udp_port, 11111);
	EXPECT_EQ(obj1.mc_tcp_port, 22222);

	// subscribetopics
	ASSERT_EQ(obj1.subscribeTopics.size(), (size_t)1);
	EXPECT_STREQ(obj1.subscribeTopics[0].name.c_str(), "TestTopic");
	EXPECT_STREQ(obj1.subscribeTopics[0].type.c_str(), "GTest.TestData");
	EXPECT_STREQ(obj1.subscribeTopics[0].transport.c_str(), "tcp");
	EXPECT_STREQ(obj1.subscribeTopics[0].address.c_str(), "127.0.0.1");
	EXPECT_EQ(obj1.subscribeTopics[0].port, 6689);

	// publishtopics
	ASSERT_EQ(obj1.publishTopics.size(), (size_t)1);
	EXPECT_STREQ(obj1.publishTopics[0].name.c_str(), "TestTopic2");
	EXPECT_STREQ(obj1.publishTopics[0].type.c_str(), "GTest.TestData");
	EXPECT_STREQ(obj1.publishTopics[0].transport.c_str(), "udp");
	EXPECT_STREQ(obj1.publishTopics[0].address.c_str(), "127.0.0.1");
	EXPECT_EQ(obj1.publishTopics[0].port, 6670);

	// knowntypes
	ASSERT_EQ(obj1.knownTypes.size(), (size_t)3);
	EXPECT_STREQ(obj1.knownTypes[0].c_str(), "type1");
	EXPECT_STREQ(obj1.knownTypes[1].c_str(), "type2");
	EXPECT_STREQ(obj1.knownTypes[2].c_str(), "type3");
}
