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

// ===============================

TEST(Test_OPSConfigObjects, TestTopic) {

    const int SAMPLE_MAX_SIZE = OPSConstants::USABLE_SEGMENT_SIZE;

	// Default constructed
	Topic obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj1.getName().c_str(), "");
	EXPECT_EQ(obj1.getPort(), 0);
	//timetolive
	EXPECT_STREQ(obj1.getTypeID().c_str(), "");
	EXPECT_STREQ(obj1.getDomainAddress().c_str(), "");
	EXPECT_STREQ(obj1.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj1.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj1.getDomainID().c_str(), "");
	EXPECT_EQ(obj1.getSampleMaxSize(), SAMPLE_MAX_SIZE);
	EXPECT_STREQ(obj1.getTransport().c_str(), "");
	//outsocketbuffersize
	//insocketbuffersize
///	EXPECT_EQ(obj1.getParticipant(), nullptr);

	// SetSampleMaxSize
	obj1.setSampleMaxSize(789789);
	EXPECT_EQ(obj1.getSampleMaxSize(), 789789);
	obj1.setSampleMaxSize(3333);
	EXPECT_EQ(obj1.getSampleMaxSize(), 3333);

	// Other constructor
	Topic obj2("Kalle", 9999, "MinType", "localhost");
	EXPECT_STREQ(obj2.getTypeString().c_str(), "Topic ");
	EXPECT_STREQ(obj2.getName().c_str(), "Kalle");
	EXPECT_EQ(obj2.getPort(), 9999);
	//timetolive
	EXPECT_STREQ(obj2.getTypeID().c_str(), "MinType");
	EXPECT_STREQ(obj2.getDomainAddress().c_str(), "localhost");
	EXPECT_STREQ(obj2.getLocalInterface().c_str(), "");
	EXPECT_STREQ(obj2.getParticipantID().c_str(), OPSConstants::DEFAULT_PARTICIPANT_ID());
	EXPECT_STREQ(obj2.getDomainID().c_str(), "");
	EXPECT_EQ(obj2.getSampleMaxSize(), SAMPLE_MAX_SIZE);
	EXPECT_STREQ(obj2.getTransport().c_str(), "");
	//outsocketbuffersize
	//insocketbuffersize
///	EXPECT_EQ(obj2.getParticipant(), nullptr);
}

TEST(Test_OPSConfigObjects, TestTopic_Serialize) {

	Topic obj1;
	{
		std::string content(
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
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
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
			"   <transport>tcp</transport>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_TCP.c_str());
	}
	{
		std::string content(
			" <root>"
			"   <transport>multicast</transport>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.getTransport().c_str(), Topic::TRANSPORT_MC.c_str());
	}
	{
		std::string content(
			" <root>"
			"   <transport>udp</transport>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
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

TEST(Test_OPSConfigObjects, TestChannel) {

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
}

TEST(Test_OPSConfigObjects, TestChannel_Serialize) {

	Channel obj1;
	{
		std::string content(
			" <root>"
			"	<name>Channel-A</name>"
			"	<localInterface>127.0.0.1</localInterface>"
			"	<address>236.5.6.7</address>"
			"	<timeToLive>3</timeToLive>"
			"	<port>6689</port>"
			"   <outSocketBufferSize>100000</outSocketBufferSize>"
			"   <inSocketBufferSize>200000</inSocketBufferSize>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_MC.c_str());
		EXPECT_STREQ(obj1.localInterface.c_str(), "127.0.0.1");
		EXPECT_STREQ(obj1.domainAddress.c_str(), "236.5.6.7");
		EXPECT_EQ(obj1.timeToLive, 3);
		EXPECT_EQ(obj1.port, 6689);
		EXPECT_EQ(obj1.outSocketBufferSize, 100000);
		EXPECT_EQ(obj1.inSocketBufferSize, 200000);
	}
	{
		std::string content(
			" <root>"
			"	<linkType>multicast</linkType>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_MC.c_str());
	}
	{
		std::string content(
			" <root>"
			"	<linkType>tcp</linkType>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_TCP.c_str());
	}
	{
		std::string content(
			" <root>"
			"	<linkType>udp</linkType>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	}
	{
		std::string content(
			" <root>"
			"	<linkType>world</linkType>"
			" </root>"
			" "
		);
		std::istringstream is(content);
		XMLArchiverIn arcIn(is, "root", nullptr);
		EXPECT_THROW(obj1.serialize(&arcIn), ConfigException);
	}
}

TEST(Test_OPSConfigObjects, TestChannel_Populate) {

	Channel obj1;

	std::string content(
		" <root>"
		"	<name>Channel-A</name>"
		"	<localInterface>127.0.0.1</localInterface>"
		"	<address>236.5.6.7</address>"
		"	<linkType>udp</linkType>"
		"	<timeToLive>3</timeToLive>"
		"	<port>6689</port>"
		"   <outSocketBufferSize>100000</outSocketBufferSize>"
		"   <inSocketBufferSize>200000</inSocketBufferSize>"
		" </root>"
		" "
	);
	std::istringstream is(content);
	XMLArchiverIn arcIn(is, "root", nullptr);
	obj1.serialize(&arcIn);
	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	EXPECT_STREQ(obj1.localInterface.c_str(), "127.0.0.1");
	EXPECT_STREQ(obj1.domainAddress.c_str(), "236.5.6.7");
	EXPECT_STREQ(obj1.linktype.c_str(), Channel::LINKTYPE_UDP.c_str());
	EXPECT_EQ(obj1.timeToLive, 3);
	EXPECT_EQ(obj1.port, 6689);
	EXPECT_EQ(obj1.outSocketBufferSize, 100000);
	EXPECT_EQ(obj1.inSocketBufferSize, 200000);

	Topic top;
	obj1.populateTopic(top);
	EXPECT_STREQ(top.getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
	EXPECT_STREQ(top.getLocalInterface().c_str(), "127.0.0.1");
	EXPECT_STREQ(top.getDomainAddress().c_str(), "236.5.6.7");
	EXPECT_EQ(top.getPort(), 6689);
	EXPECT_EQ(top.getOutSocketBufferSize(), 100000);
	EXPECT_EQ(top.getInSocketBufferSize(), 200000);
	EXPECT_EQ(top.getTimeToLive(), 3);
}

TEST(Test_OPSConfigObjects, TestTransport) {

	// Default constructed 
	Transport obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Transport ");
	EXPECT_STREQ(obj1.channelID.c_str(), "");
	EXPECT_EQ(obj1.topics.size(), (size_t)0);
}

TEST(Test_OPSConfigObjects, TestTransport_Serialize) {

	Transport obj1;

	std::string content(
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
	);
	std::istringstream is(content);
	XMLArchiverIn arcIn(is, "root", nullptr);
	obj1.serialize(&arcIn);
	EXPECT_STREQ(obj1.channelID.c_str(), "Channel-A");
	ASSERT_EQ(obj1.topics.size(), (size_t)4);

	EXPECT_STREQ(obj1.topics[0].c_str(), "Topic1");
	EXPECT_STREQ(obj1.topics[1].c_str(), "Topic2");
	EXPECT_STREQ(obj1.topics[2].c_str(), "Topic3");
	EXPECT_STREQ(obj1.topics[3].c_str(), "Topic4");
}

// ===============================
// Helper classes
class RAII_FactoryHelper_Builtin
{
public:
	OPSObjectFactoryImpl fact1;
	SerializableInheritingTypeFactory fact;
	RAII_FactoryHelper_Builtin()
	{
		fact.add((SerializableFactory*)&fact1);
	}
	~RAII_FactoryHelper_Builtin()
	{
		// Note must remove fact1 since they are static above
		// otherwise fact tries to delete it at exit!!
		EXPECT_TRUE(fact.remove((SerializableFactory*)&fact1));
	}
	RAII_FactoryHelper_Builtin(const RAII_FactoryHelper_Builtin& r) = delete;
	RAII_FactoryHelper_Builtin& operator= (const RAII_FactoryHelper_Builtin& l) = delete;
	RAII_FactoryHelper_Builtin(RAII_FactoryHelper_Builtin&&) = delete;
	RAII_FactoryHelper_Builtin& operator =(RAII_FactoryHelper_Builtin&&) = delete;
};

TEST(Test_OPSConfigObjects, TestDomain) {

	// Default constructed 
	Domain obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "Domain ");
	EXPECT_STREQ(obj1.getDomainAddress().c_str(), "");
	EXPECT_STREQ(obj1.getDomainID().c_str(), "");
	EXPECT_STREQ(obj1.getLocalInterface().c_str(), "0.0.0.0");
	EXPECT_EQ(obj1.getDebugMcPort(), 0);
	EXPECT_EQ(obj1.getMetaDataMcPort(), 9494);
	EXPECT_EQ(obj1.getTimeToLive(), 1);
	EXPECT_EQ(obj1.getOutSocketBufferSize(), -1);
	EXPECT_EQ(obj1.getInSocketBufferSize(), -1);
	EXPECT_EQ(obj1.getTopics().size(), (size_t)0);
	EXPECT_THROW(obj1.getTopic("TestTopic"), NoSuchTopicException);
	EXPECT_FALSE(obj1.existsTopic("TestTopic"));
}

TEST(Test_OPSConfigObjects, TestDomain_Serialize) {

	RAII_FactoryHelper_Builtin factory;

	// Test without channels and transports
	{
		Domain obj1;
		std::string content(
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
		);
		std::istringstream is(content);

		XMLArchiverIn arcIn(is, "root", (SerializableInheritingTypeFactory*)&factory);
		obj1.serialize(&arcIn);
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
		for (unsigned int i = 0; i < vec.size(); i++) {
			EXPECT_STREQ(vec[i]->getDomainAddress().c_str(), "236.7.8.9");
			EXPECT_STREQ(vec[i]->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_EQ(vec[i]->getTimeToLive(), 4);
			EXPECT_EQ(vec[i]->getInSocketBufferSize(), 100000);
			EXPECT_EQ(vec[i]->getOutSocketBufferSize(), 200000);
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
		std::string content(
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
		);
		std::istringstream is(content);

		XMLArchiverIn arcIn(is, "root", (SerializableInheritingTypeFactory*)&factory);
		obj1.serialize(&arcIn);
		EXPECT_STREQ(obj1.getDomainID().c_str(), "TestDomain");
		EXPECT_STREQ(obj1.getDomainAddress().c_str(), "236.7.8.9");
		std::vector<Topic*> vec = obj1.getTopics();
		EXPECT_EQ(vec.size(), (size_t)3);

		// These should be set by Domain for all topics
		for (unsigned int i = 0; i < vec.size(); i++) {
			EXPECT_STREQ(vec[i]->getDomainAddress().c_str(), "236.5.6.7");
			EXPECT_STREQ(vec[i]->getLocalInterface().c_str(), "127.0.0.1");
			EXPECT_STREQ(vec[i]->getTransport().c_str(), Topic::TRANSPORT_UDP.c_str());
			EXPECT_EQ(vec[i]->getPort(), 6689);
			EXPECT_EQ(vec[i]->getTimeToLive(), 3);
			EXPECT_EQ(vec[i]->getOutSocketBufferSize(), 100000);
			EXPECT_EQ(vec[i]->getInSocketBufferSize(), 200000);
		}

		EXPECT_TRUE(obj1.existsTopic("PizzaTopic"));
		EXPECT_TRUE(obj1.existsTopic("VessuvioTopic"));
		EXPECT_TRUE(obj1.existsTopic("ExtraAlltTopic"));
		EXPECT_FALSE(obj1.existsTopic("TestTopic"));
	}

}

TEST(Test_OPSConfigObjects, TestOPSConfig) {

	DefaultOPSConfigImpl obj1;
	EXPECT_STREQ(obj1.getTypeString().c_str(), "DefaultOPSConfigImpl ");
	EXPECT_EQ(obj1.getRefToDomains().size(), (size_t)0);
	EXPECT_EQ(obj1.getDomains().size(), (size_t)0);
	EXPECT_EQ(obj1.getDomain("Test"), nullptr);
}

TEST(Test_OPSConfigObjects, TestOPSConfig_Serialize) {

	RAII_FactoryHelper_Builtin factory;

	{
		DefaultOPSConfigImpl obj1;
		std::string content(
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
		);
		std::istringstream is(content);

		XMLArchiverIn arcIn(is, "root", (SerializableInheritingTypeFactory*)&factory);
		obj1.serialize(&arcIn);
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

TEST(Test_OPSConfigObjects, TestOPSConfig_File) {

	// OPSConfig::getConfig(filename) --> uses OPSConfig::getConfig(stream)

	std::string ops_config;
	RAII_FileRemover remover;

	// Remove warnings for tmpnam with VC++
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

	// Try first with a non existing file
	ops_config = std::tmpnam(nullptr);
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

