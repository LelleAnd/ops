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

#include <cstdio>
#include <fstream>

#include "gtest/gtest.h"

#include "opsidls/OPSConstants.h"
#include "Channel.h"
#include "Topic.h"
#include "Transport.h"
#include "Domain.h"
#include "DefaultOPSConfigImpl.h"

#include "XMLArchiverIn.h"
#include "ConfigException.h"
#include "NoSuchTopicException.h"
#include "OPSObjectFactory.h"
#include "OPSObjectFactoryImpl.h"

#include "CreateTempOpsConfigFile.h"

using namespace ops;
using namespace opsidls;

///=========================================================================

class TopicTestFixture : public ::testing::Test {
protected:
	std::string gcontent{
		" <root>"
		"	<name>TestTopic</name>"
		"	<dataType>GTest.TestData</dataType>"
		"	<port>6689</port>"
		"	<address>127.0.0.1</address>"
		"   <outSocketBufferSize>100000</outSocketBufferSize>"
		"   <inSocketBufferSize>200000</inSocketBufferSize>"
		"   <sampleMaxSize>99999</sampleMaxSize>"
		"   <transport></transport>"
		" </root>"
		" "
	};

	void SetUp() {}

	void TearDown() {}

	void getFromString(const std::string& content, Topic& obj)
	{
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj.serialize(&arcIn);
	}

	Topic f(Topic o)
	{
		return o;
	}
};

TEST_F(TopicTestFixture, Test) {

    const int SAMPLE_MAX_SIZE = OPSConstants::USABLE_SEGMENT_SIZE;

	// Default constructed
	Topic obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj1.getName().c_str(), "");
	EXPECT_EQ(obj1.getPort(), 0);
	EXPECT_EQ(obj1.getTimeToLive(), -1);
	EXPECT_STREQ(obj1.getTypeID().c_str(), "");
	EXPECT_STREQ(obj1.getDomainAddress().c_str(), "");
	EXPECT_STREQ(obj1.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj1.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj1.getDomainID().c_str(), "");
	EXPECT_EQ(obj1.getSampleMaxSize(), SAMPLE_MAX_SIZE);
	EXPECT_STREQ(obj1.getTransport().c_str(), "");
	EXPECT_EQ(obj1.getOutSocketBufferSize(), (int64_t)-1);
	EXPECT_EQ(obj1.getInSocketBufferSize(), (int64_t)-1);
	EXPECT_FALSE(obj1.getUseAck());
	EXPECT_FALSE(obj1.getOptNonVirt());
	EXPECT_EQ(obj1.getHeartbeatPeriod(), 0);
	EXPECT_EQ(obj1.getHeartbeatTimeout(), 0);
	EXPECT_EQ(obj1.getNumResends(), -1);
	EXPECT_EQ(obj1.getResendTimeMs(), -1);
	EXPECT_EQ(obj1.getRegisterTimeMs(), -1);
	EXPECT_STREQ(obj1.getChannelId().c_str(), "");

	// SetSampleMaxSize
	obj1.setSampleMaxSize(789789);
	EXPECT_EQ(obj1.getSampleMaxSize(), 789789);
	obj1.setSampleMaxSize(3333);
	EXPECT_EQ(obj1.getSampleMaxSize(), 3333);

	// Copy constructed
	Topic obj2(obj1);
	EXPECT_EQ(obj2.getSampleMaxSize(), 3333);
	EXPECT_FALSE(obj2.getUseAck());

	// Copy Assignment
	Topic obj3;
	obj3 = obj2;
	EXPECT_EQ(obj3.getSampleMaxSize(), 3333);
	EXPECT_FALSE(obj3.getUseAck());

	// Move constructed
	Topic obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	EXPECT_EQ(obj4.getSampleMaxSize(), 3333);
	EXPECT_FALSE(obj4.getUseAck());

	// Move assignment
	Topic obj5;
	obj5 = std::move(obj3);
	EXPECT_EQ(obj5.getSampleMaxSize(), 3333);
	EXPECT_FALSE(obj5.getUseAck());

	// Other constructors
	Topic obj6("Kalle", 9999, "MinType", "localhost");
	EXPECT_STREQ(obj6.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj6.getName().c_str(), "Kalle");
	EXPECT_EQ(obj6.getPort(), 9999);
	EXPECT_EQ(obj6.getTimeToLive(), -1);
	EXPECT_STREQ(obj6.getTypeID().c_str(), "MinType");
	EXPECT_STREQ(obj6.getDomainAddress().c_str(), "localhost");
	EXPECT_STREQ(obj6.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj6.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj6.getDomainID().c_str(), "");
	EXPECT_EQ(obj6.getSampleMaxSize(), SAMPLE_MAX_SIZE);
	EXPECT_STREQ(obj6.getTransport().c_str(), "");
	EXPECT_EQ(obj6.getOutSocketBufferSize(), (int64_t)-1);
	EXPECT_EQ(obj6.getInSocketBufferSize(), (int64_t)-1);
	EXPECT_FALSE(obj6.getUseAck());

	Topic obj7("Kalle2", "MinType2", 4444, true, obj6);
	EXPECT_STREQ(obj7.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj7.getName().c_str(), "Kalle2");
	EXPECT_EQ(obj7.getPort(), 9999);
	EXPECT_EQ(obj7.getTimeToLive(), -1);
	EXPECT_STREQ(obj7.getTypeID().c_str(), "MinType2");
	EXPECT_STREQ(obj7.getDomainAddress().c_str(), "localhost");
	EXPECT_STREQ(obj7.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj7.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj7.getDomainID().c_str(), "");
	EXPECT_EQ(obj7.getSampleMaxSize(), 4444);
	EXPECT_STREQ(obj7.getTransport().c_str(), "");
	EXPECT_EQ(obj7.getOutSocketBufferSize(), (int64_t)-1);
	EXPECT_EQ(obj7.getInSocketBufferSize(), (int64_t)-1);
	EXPECT_TRUE(obj7.getUseAck());

	// Create specialized topic
	Topic obj8 = Topic::CreateAckTopic(obj7);
	EXPECT_STREQ(obj8.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj8.getName().c_str(), "Kalle2#ack");
	EXPECT_EQ(obj8.getPort(), 9999);
	EXPECT_EQ(obj8.getTimeToLive(), -1);
	EXPECT_STREQ(obj8.getTypeID().c_str(), opsidls::SendAckPatternData::getTypeName().c_str());
	EXPECT_STREQ(obj8.getDomainAddress().c_str(), "localhost");
	EXPECT_STREQ(obj8.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj8.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj8.getDomainID().c_str(), "");
	EXPECT_EQ(obj8.getSampleMaxSize(), 1024);
	EXPECT_STREQ(obj8.getTransport().c_str(), "");
	EXPECT_EQ(obj8.getOutSocketBufferSize(), (int64_t)-1);
	EXPECT_EQ(obj8.getInSocketBufferSize(), (int64_t)-1);
	EXPECT_FALSE(obj8.getUseAck());
}

TEST_F(TopicTestFixture, Test_Clone) {

	Topic obj1;

	getFromString(gcontent, obj1);
	obj1.setTimeToLive(2);
	obj1.setLocalInterface("192.1.1.1");
	obj1.setParticipantID("Hi");
	obj1.setDomainID("Bye");
	obj1.setUseAck(true);

	EXPECT_STREQ(obj1.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj1.getName().c_str(), "TestTopic");
	EXPECT_EQ(obj1.getPort(), 6689);
	EXPECT_EQ(obj1.getTimeToLive(), 2);
	EXPECT_STREQ(obj1.getTypeID().c_str(), "GTest.TestData");
	EXPECT_STREQ(obj1.getDomainAddress().c_str(), "127.0.0.1");
	EXPECT_STREQ(obj1.getLocalInterface().c_str(), "192.1.1.1");
	EXPECT_STREQ(obj1.getParticipantID().c_str(), "Hi");
	EXPECT_STREQ(obj1.getDomainID().c_str(), "Bye");
	EXPECT_EQ(obj1.getSampleMaxSize(), 99999);
	EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_MC.c_str());
	EXPECT_EQ(obj1.getOutSocketBufferSize(), 100000);
	EXPECT_EQ(obj1.getInSocketBufferSize(), 200000);
	EXPECT_TRUE(obj1.getUseAck());
	EXPECT_FALSE(obj1.getOptNonVirt());
	EXPECT_EQ(obj1.getHeartbeatPeriod(), 0);
	EXPECT_EQ(obj1.getHeartbeatTimeout(), 0);
	EXPECT_EQ(obj1.getNumResends(), -1);
	EXPECT_EQ(obj1.getResendTimeMs(), -1);
	EXPECT_EQ(obj1.getRegisterTimeMs(), -1);
	EXPECT_STREQ(obj1.getChannelId().c_str(), "");

	std::unique_ptr<Topic> obj2(obj1.clone());

	EXPECT_STREQ(obj2->getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj2->getName().c_str(), "TestTopic");
	EXPECT_EQ(obj2->getPort(), 6689);
	EXPECT_EQ(obj2->getTimeToLive(), 2);
	EXPECT_STREQ(obj2->getTypeID().c_str(), "GTest.TestData");
	EXPECT_STREQ(obj2->getDomainAddress().c_str(), "127.0.0.1");
	EXPECT_STREQ(obj2->getLocalInterface().c_str(), "192.1.1.1");
	EXPECT_STREQ(obj2->getParticipantID().c_str(), "Hi");
	EXPECT_STREQ(obj2->getDomainID().c_str(), "Bye");
	EXPECT_EQ(obj2->getSampleMaxSize(), 99999);
	EXPECT_STREQ(obj2->getTransport().c_str(), Topic::TRANSPORT_MC.c_str());
	EXPECT_EQ(obj2->getOutSocketBufferSize(), 100000);
	EXPECT_EQ(obj2->getInSocketBufferSize(), 200000);
	EXPECT_TRUE(obj2->getUseAck());
	EXPECT_FALSE(obj2->getOptNonVirt());
	EXPECT_EQ(obj2->getHeartbeatPeriod(), 0);
	EXPECT_EQ(obj2->getHeartbeatTimeout(), 0);
	EXPECT_EQ(obj2->getNumResends(), -1);
	EXPECT_EQ(obj2->getResendTimeMs(), -1);
	EXPECT_EQ(obj2->getRegisterTimeMs(), -1);
	EXPECT_STREQ(obj2->getChannelId().c_str(), "");
}

TEST_F(TopicTestFixture, Test_Serialize) {

	Topic obj1;
	{
		getFromString(gcontent, obj1);
		EXPECT_STREQ(obj1.getName().c_str(), "TestTopic");
		EXPECT_EQ(obj1.getPort(), 6689);
		EXPECT_STREQ(obj1.getTypeID().c_str(), "GTest.TestData");
		EXPECT_STREQ(obj1.getDomainAddress().c_str(), "127.0.0.1");
		EXPECT_EQ(obj1.getSampleMaxSize(), 99999);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_MC.c_str());
		EXPECT_EQ(obj1.getOutSocketBufferSize(), 100000);
		EXPECT_EQ(obj1.getInSocketBufferSize(), 200000);
	}
	{
		std::string content(
			" <root>"
			"   <useAck>true</useAck>"
			" </root>"
			" "
		);
		getFromString(content, obj1);
		EXPECT_TRUE(obj1.getUseAck());
	}
	{
		std::string content(
			" <root>"
			"   <useAck>false</useAck>"
			" </root>"
			" "
		);
		getFromString(content, obj1);
		EXPECT_FALSE(obj1.getUseAck());
	}
	{
		std::string content(
			" <root>"
			"   <transport>tcp</transport>"
			" </root>"
			" "
		);
		getFromString(content, obj1);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_TCP.c_str());
	}
	{
		std::string content(
			" <root>"
			"   <transport>multicast</transport>"
			" </root>"
			" "
		);
		getFromString(content, obj1);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_MC.c_str());
	}
	{
		std::string content(
			" <root>"
			"   <transport>udp</transport>"
			" </root>"
			" "
		);
		getFromString(content, obj1);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
	}
	{
		std::string content(
			" <root>"
			"   <transport>hello</transport>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		EXPECT_THROW(obj1.serialize(&arcIn), ConfigException);
	}
}

///=========================================================================

class ChannelTestFixture : public ::testing::Test {
protected:
	std::string gcontent{
		" <root>"
		"	<name>Channel-A</name>"
		"	<localInterface>127.0.0.1</localInterface>"
		"	<address>236.5.6.7</address>"
		"	<timeToLive>3</timeToLive>"
		"	<port>6689</port>"
		"   <outSocketBufferSize>100000</outSocketBufferSize>"
		"   <inSocketBufferSize>200000</inSocketBufferSize>"
		"   <sampleMaxSize>50000</sampleMaxSize>"
		"   <resendNum>4</resendNum>"
		"   <resendTimeMs>25</resendTimeMs>"
		"   <registerTimeMs>2000</registerTimeMs>"
		" </root>"
		" "
	};
	std::string content_linkudp{
		" <root>"
		"	<linkType>udp</linkType>"
		" </root>"
		" "
	};

	void SetUp() {}

	void TearDown() {}
	
	void getFromString(const std::string& content, Channel& obj)
	{
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj.serialize(&arcIn);
	}

	Channel f(Channel o)
	{
		return o;
	}
};

TEST_F(ChannelTestFixture, Test) {

	// Default constructed 
	Channel obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Channel ");
	EXPECT_STREQ(obj1.channelID.c_str(), "");
	EXPECT_STREQ(obj1.linktype.c_str(), "");
	EXPECT_STREQ(obj1.localInterface.c_str(), "");
	EXPECT_STREQ(obj1.domainAddress.c_str(), "");
	EXPECT_EQ(obj1.timeToLive, -1);
	EXPECT_EQ(obj1.port, 0);
	EXPECT_EQ(obj1.outSocketBufferSize, -1);
	EXPECT_EQ(obj1.inSocketBufferSize, -1);
	EXPECT_EQ(obj1.sampleMaxSize, -1);
	EXPECT_EQ(obj1.resendNum, -1);
	EXPECT_EQ(obj1.resendTimeMs, -1);
	EXPECT_EQ(obj1.registerTimeMs, -1);

	// Add some data
	obj1.channelID = "Test";
	obj1.spareBytes.push_back('a');
	obj1.spareBytes.push_back('b');
	obj1.spareBytes.push_back('c');
	obj1.spareBytes.push_back('d');
	obj1.spareBytes.push_back('\0');

	// Copy constructed
	Channel obj2(obj1);
	EXPECT_STREQ(obj2.channelID.c_str(), "Test");
	EXPECT_EQ(obj2.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj2.spareBytes[0], "abcd");

	// Copy Assignment
	Channel obj3;
	obj3 = obj2;
	EXPECT_STREQ(obj3.channelID.c_str(), "Test");
	EXPECT_EQ(obj3.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj3.spareBytes[0], "abcd");

	// Move constructed
	Channel obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	EXPECT_STREQ(obj4.channelID.c_str(), "Test");
	EXPECT_EQ(obj4.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj4.spareBytes[0], "abcd");

	// Move assignment
	Channel obj5;
	obj5 = std::move(obj3);
	EXPECT_STREQ(obj5.channelID.c_str(), "Test");
	EXPECT_EQ(obj5.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj5.spareBytes[0], "abcd");
}

TEST_F(ChannelTestFixture, Test_Clone) {

	Channel obj1;

	getFromString(gcontent, obj1);
	getFromString(content_linkudp, obj1);
	obj1.spareBytes.push_back('a');

	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	EXPECT_STREQ(obj1.localInterface.c_str(), "127.0.0.1");
	EXPECT_STREQ(obj1.domainAddress.c_str(), "236.5.6.7");
	EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	EXPECT_EQ(obj1.timeToLive, 3);
	EXPECT_EQ(obj1.port, 6689);
	EXPECT_EQ(obj1.outSocketBufferSize, 100000);
	EXPECT_EQ(obj1.inSocketBufferSize, 200000);
	EXPECT_EQ(obj1.sampleMaxSize, 50000);
	EXPECT_EQ(obj1.resendNum, 4);
	EXPECT_EQ(obj1.resendTimeMs, 25);
	EXPECT_EQ(obj1.registerTimeMs, 2000);
	EXPECT_EQ(obj1.spareBytes.size(), (size_t)1);

	std::unique_ptr<Channel> obj2(obj1.clone());

	EXPECT_STREQ(obj2->channelID.c_str(), "Channel-A");
	EXPECT_STREQ(obj2->localInterface.c_str(), "127.0.0.1");
	EXPECT_STREQ(obj2->domainAddress.c_str(), "236.5.6.7");
	EXPECT_STREQ(obj2->linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	EXPECT_EQ(obj2->timeToLive, 3);
	EXPECT_EQ(obj2->port, 6689);
	EXPECT_EQ(obj2->outSocketBufferSize, 100000);
	EXPECT_EQ(obj2->inSocketBufferSize, 200000);
	EXPECT_EQ(obj2->sampleMaxSize, 50000);
	EXPECT_EQ(obj2->resendNum, 4);
	EXPECT_EQ(obj2->resendTimeMs, 25);
	EXPECT_EQ(obj2->registerTimeMs, 2000);
	EXPECT_EQ(obj1.spareBytes.size(), (size_t)1);
}

TEST_F(ChannelTestFixture, Test_Serialize) {

	Channel obj1;
	{
		getFromString(gcontent, obj1);
		EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_MC.c_str());
		EXPECT_STREQ(obj1.localInterface.c_str(), "127.0.0.1");
		EXPECT_STREQ(obj1.domainAddress.c_str(), "236.5.6.7");
		EXPECT_EQ(obj1.timeToLive, 3);
		EXPECT_EQ(obj1.port, 6689);
		EXPECT_EQ(obj1.outSocketBufferSize, 100000);
		EXPECT_EQ(obj1.inSocketBufferSize, 200000);
		EXPECT_EQ(obj1.sampleMaxSize, 50000);
		EXPECT_EQ(obj1.resendNum, 4);
		EXPECT_EQ(obj1.resendTimeMs, 25);
		EXPECT_EQ(obj1.registerTimeMs, 2000);
	}
	{
		std::string content1(
			" <root>"
			"	<linkType>multicast</linkType>"
			" </root>"
			" "
		);
		getFromString(content1, obj1);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_MC.c_str());
	}
	{
		std::string content1(
			" <root>"
			"	<linkType>tcp</linkType>"
			" </root>"
			" "
		);
		getFromString(content1, obj1);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_TCP.c_str());
	}
	{
		getFromString(content_linkudp, obj1);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	}
	{
		std::string content1(
			" <root>"
			"	<linkType>world</linkType>"
			" </root>"
			" "
		);
		std::istringstream is(content1);
		XMLArchiverIn arcIn(is, "root", nullptr);
		EXPECT_THROW(obj1.serialize(&arcIn), ConfigException);
	}
}

TEST_F(ChannelTestFixture, Test_Populate) {

	Channel obj1;

	getFromString(gcontent, obj1);
	getFromString(content_linkudp, obj1);

	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	EXPECT_STREQ(obj1.localInterface.c_str(), "127.0.0.1");
	EXPECT_STREQ(obj1.domainAddress.c_str(), "236.5.6.7");
	EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	EXPECT_EQ(obj1.timeToLive, 3);
	EXPECT_EQ(obj1.port, 6689);
	EXPECT_EQ(obj1.outSocketBufferSize, 100000);
	EXPECT_EQ(obj1.inSocketBufferSize, 200000);
	EXPECT_EQ(obj1.sampleMaxSize, 50000);
	EXPECT_EQ(obj1.resendNum, 4);
	EXPECT_EQ(obj1.resendTimeMs, 25);
	EXPECT_EQ(obj1.registerTimeMs, 2000);

	Topic top;
	obj1.populateTopic(top);
	EXPECT_STREQ(top.getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
	EXPECT_STREQ(top.getLocalInterface().c_str(), "127.0.0.1");
	EXPECT_STREQ(top.getDomainAddress().c_str(), "236.5.6.7");
	EXPECT_STREQ(top.getChannelId().c_str(), "Channel-A");
	EXPECT_EQ(top.getPort(), 6689);
	EXPECT_EQ(top.getOutSocketBufferSize(), 100000);
	EXPECT_EQ(top.getInSocketBufferSize(), 200000);
	EXPECT_EQ(top.getTimeToLive(), 3);
	EXPECT_EQ(top.getSampleMaxSize(), 50000);
	EXPECT_EQ(top.getNumResends(), 4);
	EXPECT_EQ(top.getResendTimeMs(), 25);
	EXPECT_EQ(top.getRegisterTimeMs(), 2000);
}

///=========================================================================

class TransportTestFixture : public ::testing::Test {
protected:
	std::string gcontent{
		" <root>"
		"	<channelID>Channel-A</channelID>"
		"	<topics>"
		"		<element>Topic1</element>"
		"		<element>Topic2</element>"
		"		<element>Topic3</element>"
		"		<element>Topic4</element>"
		"   </topics>"
		" </root>"
		" "
	};

	void SetUp() {}

	void TearDown() {}

	void getFromString(const std::string& content, Transport& obj)
	{
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj.serialize(&arcIn);
	}

	Transport f(Transport o)
	{
		return o;
	}
};

TEST_F(TransportTestFixture, Test) {

	// Default constructed 
	Transport obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Transport ");
	EXPECT_STREQ(obj1.channelID.c_str(), "");
	EXPECT_EQ(obj1.topics.size(), (size_t)0);

	// Add some data
	obj1.topics.push_back("Test1");
	obj1.topics.push_back("Test2");
	obj1.spareBytes.push_back('a');
	obj1.spareBytes.push_back('b');
	obj1.spareBytes.push_back('c');
	obj1.spareBytes.push_back('d');
	obj1.spareBytes.push_back('\0');

	// Copy constructed
	Transport obj2(obj1);
	ASSERT_EQ(obj2.topics.size(), (size_t)2);
	EXPECT_STREQ(obj2.topics[0].c_str(), "Test1");
	EXPECT_STREQ(obj2.topics[1].c_str(), "Test2");
	EXPECT_EQ(obj2.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj2.spareBytes[0], "abcd");

	// Copy Assignment
	Transport obj3;
	obj3 = obj2;
	ASSERT_EQ(obj3.topics.size(), (size_t)2);
	EXPECT_STREQ(obj3.topics[0].c_str(), "Test1");
	EXPECT_STREQ(obj3.topics[1].c_str(), "Test2");
	EXPECT_EQ(obj3.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj3.spareBytes[0], "abcd");

	// Move constructed
	Transport obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	ASSERT_EQ(obj4.topics.size(), (size_t)2);
	EXPECT_STREQ(obj4.topics[0].c_str(), "Test1");
	EXPECT_STREQ(obj4.topics[1].c_str(), "Test2");
	EXPECT_EQ(obj4.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj4.spareBytes[0], "abcd");

	// Move assignment
	Transport obj5;
	obj5 = std::move(obj3);
	ASSERT_EQ(obj5.topics.size(), (size_t)2);
	EXPECT_STREQ(obj5.topics[0].c_str(), "Test1");
	EXPECT_STREQ(obj5.topics[1].c_str(), "Test2");
	EXPECT_EQ(obj5.spareBytes.size(), (size_t)5);
	EXPECT_STREQ((char*)&obj5.spareBytes[0], "abcd");

}

TEST_F(TransportTestFixture, Test_Clone) {

	Transport obj1;

	getFromString(gcontent, obj1);
	obj1.spareBytes.push_back('a');

	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	ASSERT_EQ(obj1.topics.size(), (size_t)4);

	EXPECT_STREQ(obj1.topics[0].c_str(), "Topic1");
	EXPECT_STREQ(obj1.topics[1].c_str(), "Topic2");
	EXPECT_STREQ(obj1.topics[2].c_str(), "Topic3");
	EXPECT_STREQ(obj1.topics[3].c_str(), "Topic4");
	EXPECT_EQ(obj1.spareBytes.size(), (size_t)1);

	std::unique_ptr<Transport> obj2(obj1.clone());

	EXPECT_STREQ(obj2->channelID.c_str(), "Channel-A");
	ASSERT_EQ(obj2->topics.size(), (size_t)4);

	EXPECT_STREQ(obj2->topics[0].c_str(), "Topic1");
	EXPECT_STREQ(obj2->topics[1].c_str(), "Topic2");
	EXPECT_STREQ(obj2->topics[2].c_str(), "Topic3");
	EXPECT_STREQ(obj2->topics[3].c_str(), "Topic4");
	EXPECT_EQ(obj1.spareBytes.size(), (size_t)1);
}

TEST_F(TransportTestFixture, Test_Serialize) {

	Transport obj1;

	getFromString(gcontent, obj1);

	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	ASSERT_EQ(obj1.topics.size(), (size_t)4);

	EXPECT_STREQ(obj1.topics[0].c_str(), "Topic1");
	EXPECT_STREQ(obj1.topics[1].c_str(), "Topic2");
	EXPECT_STREQ(obj1.topics[2].c_str(), "Topic3");
	EXPECT_STREQ(obj1.topics[3].c_str(), "Topic4");
}

///=========================================================================

class DomainTestFixture : public ::testing::Test {
protected:
	OPSObjectFactoryImpl fact1;
	SerializableInheritingTypeFactory factory;

	std::string content_no_ch_tr{
		" <root>"
		"	<domainID>TestDomain</domainID>"
		"	<domainAddress>236.7.8.9</domainAddress>"
		"	<localInterface>127.0.0.1</localInterface>"
		"	<timeToLive>4</timeToLive>"
		"	<inSocketBufferSize>100000</inSocketBufferSize>"
		"	<outSocketBufferSize>200000</outSocketBufferSize>"
		"	<metaDataMcPort>7877</metaDataMcPort>"
		"	<debugMcPort>9999</debugMcPort>"
		"	<topics>"
		"		<element type = \"Topic\">"
		"			<name>PizzaTopic</name>"
		"			<port>6689</port>"
		"			<dataType>pizza.PizzaData</dataType>"
		"		</element>"
		"		<element type = \"Topic\">"
		"			<name>VessuvioTopic</name>"
		"			<port>6690</port>"
		"			<dataType>pizza.VessuvioData</dataType>"
		"		</element>"
		"		<element type = \"Topic\">"
		"			<name>ExtraAlltTopic</name>"
		"			<port>6691</port>"
		"			<dataType>pizza.special.ExtraAllt</dataType>"
		"		</element>"
		"   </topics>"
		" </root>"
		" "
	};
	std::string content_with_ch_tr {
		" <root>"
		"	<domainID>TestDomain</domainID>"
		"	<domainAddress>236.7.8.9</domainAddress>"
		"	<channels>"
		"		<element type = \"Channel\">"
		"			<name>Channel-A</name>"
		"			<localInterface>127.0.0.1</localInterface>"
		"			<address>236.5.6.7</address>"
		"			<linkType>udp</linkType>"
		"			<timeToLive>3</timeToLive>"
		"			<port>6689</port>"
		"			<outSocketBufferSize>100000</outSocketBufferSize>"
		"			<inSocketBufferSize>200000</inSocketBufferSize>"
		"           <resendNum>17</resendNum>"
		"           <resendTimeMs>53</resendTimeMs>"
		"           <registerTimeMs>777</registerTimeMs>"
		"		</element>"
		"	</channels>"
		"	<transports>"
		"		<element type = \"Transport\">"
		"			<channelID>Channel-A</channelID>"
		"			<topics>"
		"				<element>PizzaTopic</element>"
		"				<element>VessuvioTopic</element>"
		"				<element>ExtraAlltTopic</element>"
		"			</topics>"
		"		</element>"
		"	</transports>"
		"	<topics>"
		"		<element type = \"Topic\">"
		"			<name>PizzaTopic</name>"
		"			<dataType>pizza.PizzaData</dataType>"
		"		</element>"
		"		<element type = \"Topic\">"
		"			<name>VessuvioTopic</name>"
		"			<dataType>pizza.VessuvioData</dataType>"
		"		</element>"
		"		<element type = \"Topic\">"
		"			<name>ExtraAlltTopic</name>"
		"			<dataType>pizza.special.ExtraAllt</dataType>"
		"		</element>"
		"   </topics>"
		" </root>"
		" "
	};

	void SetUp()
	{
		factory.add((SerializableFactory*)&fact1);
	}

	void TearDown()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(factory.remove((SerializableFactory*)&fact1));
	}

	void getFromString(const std::string& content, Domain& obj)
	{
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", &factory);
		obj.serialize(&arcIn);
	}

	Domain f(Domain o)
	{
		return o;
	}
};

TEST_F(DomainTestFixture, Test) {

	// Default constructed
	Domain obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Domain ");
	EXPECT_STREQ(obj1.getDomainAddress().c_str(), "");
	EXPECT_EQ(obj1.getTimeToLive(), 1);
	EXPECT_STREQ(obj1.getLocalInterface().c_str(), "0.0.0.0");
	EXPECT_EQ(obj1.getInSocketBufferSize(), -1);
	EXPECT_EQ(obj1.getOutSocketBufferSize(), -1);
	EXPECT_EQ(obj1.getTopics().size(), (size_t)0);
	EXPECT_STREQ(obj1.getDomainID().c_str(), "");
	EXPECT_EQ(obj1.getMetaDataMcPort(), 9494);
	EXPECT_EQ(obj1.getDebugMcPort(), 0);
	EXPECT_FALSE(obj1.getOptNonVirt());
	EXPECT_EQ(obj1.getHeartbeatPeriod(), 1000);
	EXPECT_EQ(obj1.getHeartbeatTimeout(), 3000);

	EXPECT_THROW(obj1.getTopic("TestTopic"), NoSuchTopicException);
	EXPECT_FALSE(obj1.existsTopic("TestTopic"));

	// Add some data
	getFromString(content_with_ch_tr, obj1);
	{
		std::vector<Topic*> vec = obj1.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_EQ(x->getPort(), 6689);
		}
	}

	// Copy constructed
	Domain obj2(obj1);
	{
		std::vector<Topic*> vec = obj2.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_EQ(x->getPort(), 6689);
		}
	}

	// Copy Assignment
	Domain obj3;
	obj3 = obj2;
	{
		std::vector<Topic*> vec = obj3.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_EQ(x->getPort(), 6689);
		}
	}

	// Move constructed
	Domain obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	{
		std::vector<Topic*> vec = obj4.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_EQ(x->getPort(), 6689);
		}
	}

	// Move assignment
	Domain obj5;
	obj5 = std::move(obj3);
	{
		std::vector<Topic*> vec = obj5.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_EQ(x->getPort(), 6689);
		}
	}
}

TEST_F(DomainTestFixture, Test_Clone) {

	Domain obj1;
	getFromString(content_with_ch_tr, obj1);
	{
		EXPECT_STREQ(obj1.getTypeString().c_str(), "Domain ");
		EXPECT_STREQ(obj1.getDomainAddress().c_str(), "236.7.8.9");
		std::vector<Topic*> vec = obj1.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);

		// These should be set by Domain for all topics
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_STREQ(x->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_STREQ(x->getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
			EXPECT_EQ(x->getPort(), 6689);
			EXPECT_EQ(x->getTimeToLive(), 3);
			EXPECT_EQ(x->getOutSocketBufferSize(), 100000);
			EXPECT_EQ(x->getInSocketBufferSize(), 200000);
		}
	}

	std::unique_ptr<Domain> obj2(obj1.clone());
	{
		EXPECT_STREQ(obj2->getTypeString().c_str(), "Domain ");
		EXPECT_STREQ(obj2->getDomainAddress().c_str(), "236.7.8.9");
		std::vector<Topic*> vec = obj2->getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);

		// These should be set by Domain for all topics
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_STREQ(x->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_STREQ(x->getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
			EXPECT_EQ(x->getPort(), 6689);
			EXPECT_EQ(x->getTimeToLive(), 3);
			EXPECT_EQ(x->getOutSocketBufferSize(), 100000);
			EXPECT_EQ(x->getInSocketBufferSize(), 200000);
		}
	}
}

TEST_F(DomainTestFixture, Test_Serialize) {

	// Test without channels and transports
	{
		Domain obj1;
		getFromString(content_no_ch_tr, obj1);
		EXPECT_STREQ(obj1.getDomainID().c_str(), "TestDomain");
		EXPECT_STREQ(obj1.getDomainAddress().c_str(), "236.7.8.9");
		EXPECT_STREQ(obj1.getLocalInterface().c_str(), "127.0.0.1");
		EXPECT_EQ(obj1.getDebugMcPort(), 9999);
		EXPECT_EQ(obj1.getMetaDataMcPort(), 7877);
		EXPECT_EQ(obj1.getTimeToLive(), 4);
		EXPECT_EQ(obj1.getOutSocketBufferSize(), 200000);
		EXPECT_EQ(obj1.getInSocketBufferSize(), 100000);
		std::vector<Topic*> vec = obj1.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);

		// These should be set by Domain for all topics
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.7.8.9");
			EXPECT_STREQ(x->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_EQ(x->getTimeToLive(), 4);
			EXPECT_EQ(x->getInSocketBufferSize(), 100000);
			EXPECT_EQ(x->getOutSocketBufferSize(), 200000);
			EXPECT_FALSE(x->getOptNonVirt());
			EXPECT_EQ(x->getHeartbeatPeriod(), 1000);
			EXPECT_EQ(x->getHeartbeatTimeout(), 3000);
			EXPECT_EQ(x->getNumResends(), 0);
			EXPECT_EQ(x->getResendTimeMs(), 10);
			EXPECT_EQ(x->getRegisterTimeMs(), 1000);
			EXPECT_STREQ(x->getChannelId().c_str(), "");
		}

		EXPECT_TRUE(obj1.existsTopic("PizzaTopic"));
		EXPECT_TRUE(obj1.existsTopic("VessuvioTopic"));
		EXPECT_TRUE(obj1.existsTopic("ExtraAlltTopic"));
		EXPECT_FALSE(obj1.existsTopic("TestTopic"));

		EXPECT_THROW(obj1.getTopic("TestTopic"), NoSuchTopicException);
		EXPECT_NO_THROW(obj1.getTopic("PizzaTopic"));
	}

	// Test with channels and transports
	{
		Domain obj1;
		getFromString(content_with_ch_tr, obj1);
		EXPECT_STREQ(obj1.getDomainID().c_str(), "TestDomain");
		EXPECT_STREQ(obj1.getDomainAddress().c_str(), "236.7.8.9");
		std::vector<Topic*> vec = obj1.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);

		// These should be set by Domain for all topics
		for (auto& x : vec) {
			EXPECT_STREQ(x->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_STREQ(x->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_STREQ(x->getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
			EXPECT_EQ(x->getPort(), 6689);
			EXPECT_EQ(x->getTimeToLive(), 3);
			EXPECT_EQ(x->getOutSocketBufferSize(), 100000);
			EXPECT_EQ(x->getInSocketBufferSize(), 200000);
			EXPECT_EQ(x->getHeartbeatPeriod(), 1000);
			EXPECT_EQ(x->getHeartbeatTimeout(), 3000);
			EXPECT_EQ(x->getNumResends(), 17);
			EXPECT_EQ(x->getResendTimeMs(), 53);
			EXPECT_EQ(x->getRegisterTimeMs(), 777);
			EXPECT_STREQ(x->getChannelId().c_str(), "Channel-A");
		}

		EXPECT_TRUE(obj1.existsTopic("PizzaTopic"));
		EXPECT_TRUE(obj1.existsTopic("VessuvioTopic"));
		EXPECT_TRUE(obj1.existsTopic("ExtraAlltTopic"));
		EXPECT_FALSE(obj1.existsTopic("TestTopic"));
	}
}

///=========================================================================

class OpsConfigTestFixture : public ::testing::Test {
protected:
	OPSObjectFactoryImpl fact1;
	SerializableInheritingTypeFactory factory;

	std::string gcontent{
		" <root>"
		"   <domains>"
		"	  <element type = \"Domain\">"
		"	    <domainID>TestDomain</domainID>"
		"	    <domainAddress>236.7.8.9</domainAddress>"
		"	    <localInterface>127.0.0.1</localInterface>"
		"	    <timeToLive>4</timeToLive>"
		"	    <inSocketBufferSize>100000</inSocketBufferSize>"
		"	    <outSocketBufferSize>200000</outSocketBufferSize>"
		"	    <metaDataMcPort>7877</metaDataMcPort>"
		"	    <debugMcPort>9999</debugMcPort>"
		"	    <topics>"
		"	      <element type = \"Topic\">"
		"           <name>PizzaTopic</name>"
		"           <port>6689</port>"
		"           <dataType>pizza.PizzaData</dataType>"
		"	      </element>"
		"       </topics>"
		"     </element>"
		"	  <element type = \"Domain\">"
		"	    <domainID>DummyDomain</domainID>"
		"	    <domainAddress>236.9.9.9</domainAddress>"
		"	    <localInterface>127.0.0.1</localInterface>"
		"	    <timeToLive>4</timeToLive>"
		"	    <metaDataMcPort>7878</metaDataMcPort>"
		"	    <debugMcPort>9991</debugMcPort>"
		"	    <topics>"
		"	      <element type = \"Topic\">"
		"           <name>PizzaTopic2</name>"
		"           <port>6690</port>"
		"           <dataType>pizza.PizzaData</dataType>"
		"	      </element>"
		"       </topics>"
		"     </element>"
		"   </domains>"
		" </root>"
		" "
	};

	void SetUp()
	{
		factory.add((SerializableFactory*)&fact1);
	}

	void TearDown()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(factory.remove((SerializableFactory*)&fact1));
	}

	void getFromString(const std::string& content, DefaultOPSConfigImpl& obj)
	{
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", &factory);
		obj.serialize(&arcIn);
	}

	DefaultOPSConfigImpl f(DefaultOPSConfigImpl o)
	{
		return o;
	}
};

TEST_F(OpsConfigTestFixture, Test) {

	// Default constructor
	DefaultOPSConfigImpl obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "DefaultOPSConfigImpl ");
	EXPECT_EQ(obj1.getRefToDomains().size(), (size_t)0);
	EXPECT_EQ(obj1.getDomains().size(), (size_t)0);
	EXPECT_EQ(obj1.getDomain("Test"), nullptr);

	// Add some data
	getFromString(gcontent, obj1);
	Domain* dom = obj1.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj1.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);

	// Copy constructor
	DefaultOPSConfigImpl obj2(obj1);
	dom = obj2.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj2.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);

	// Copy assignment
	DefaultOPSConfigImpl obj3;
	obj3 = obj2;
	dom = obj3.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj3.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);

	// Move constructor
	DefaultOPSConfigImpl obj4 = f(obj3);	// Makes a copy of obj3 which is given to f(), which is moved from
	dom = obj4.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj4.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);

	// Move assignment
	DefaultOPSConfigImpl obj5;
	obj5 = std::move(obj3);
	dom = obj5.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj5.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);
}

TEST_F(OpsConfigTestFixture, Test_Clone) {

	DefaultOPSConfigImpl obj1;
	getFromString(gcontent, obj1);
	Domain* dom = obj1.getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj1.getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);

	std::unique_ptr<DefaultOPSConfigImpl> obj2(obj1.clone());
	dom = obj2->getDomain("TestDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9999);
	dom = obj2->getDomain("DummyDomain");
	ASSERT_NE(dom, nullptr);
	EXPECT_EQ(dom->getDebugMcPort(), 9991);
}

TEST_F(OpsConfigTestFixture, Test_Serialize) {

	{
		DefaultOPSConfigImpl obj1;
		getFromString(gcontent, obj1);

		EXPECT_STREQ(obj1.getTypeString().c_str(), "DefaultOPSConfigImpl ");
		EXPECT_EQ(obj1.getRefToDomains().size(), (size_t)2);
		EXPECT_EQ(obj1.getDomains().size(), (size_t)2);
		EXPECT_EQ(obj1.getDomain("Test"), nullptr);
		Domain* dom = obj1.getDomain("TestDomain");
		ASSERT_NE(dom, nullptr);
		EXPECT_EQ(dom->getDebugMcPort(), 9999);
		dom = obj1.getDomain("DummyDomain");
		ASSERT_NE(dom, nullptr);
		EXPECT_EQ(dom->getDebugMcPort(), 9991);
	}
}

TEST_F(OpsConfigTestFixture, Test_File) {

	// OPSConfig::getConfig(filename) --> uses OPSConfig::getConfig(stream)

	std::string ops_config;
	RAII_FileRemover remover;

	// Remove warnings for tmpnam with VC++
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

	// Try first with a non existing file
	char* ptr = std::tmpnam(nullptr);
	ASSERT_NE(ptr, nullptr);
	if (ptr != nullptr) {
		ops_config = ptr;
		EXPECT_EQ(OPSConfig::getConfig(ops_config), nullptr);

		// Try with an existing file
		ASSERT_TRUE(CreateTempOpsConfigFile(ops_config));
		remover.Add(ops_config);
		std::shared_ptr<OPSConfig> const cfg = OPSConfig::getConfig(ops_config);
		ASSERT_NE(cfg, nullptr);

		EXPECT_EQ(cfg->getRefToDomains().size(), (size_t)2);
		EXPECT_EQ(cfg->getDomains().size(), (size_t)2);
		EXPECT_EQ(cfg->getDomain("Test"), nullptr);
		Domain* dom = cfg->getDomain("TestDomain");
		ASSERT_NE(dom, nullptr);
		EXPECT_EQ(dom->getDebugMcPort(), 9999);
		dom = cfg->getDomain("DummyDomain");
		ASSERT_NE(dom, nullptr);
		EXPECT_EQ(dom->getDebugMcPort(), 9991);
	}
}

