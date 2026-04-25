/**
*
* Copyright (C) 2026 Lennart Andersson.
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

#include "SetupOPSConfig.h"
#include "BasicError.h"
#include "ErrorService.h"
#include "Listener.h"
#include "Participant.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "ReceiveDataHandler.h"
#include "SendDataHandler.h"

namespace GTest {
    struct MocRDH : public ops::ReceiveDataHandler
    {
        int dataNotifications{ 0 };
        ops::ObjectName_T m_topicName;
        ops::Address_T m_ip;
        int m_port{ 0 };

        MocRDH(ops::Participant& part) : ReceiveDataHandler(part) {}

        void AddReceiveChannel(const ops::ObjectName_T& topicName, const ops::Address_T& ip, int port) override
        {
            ++dataNotifications;
            m_topicName = topicName;
            m_ip = ip;
            m_port = port;
        }
    };

    struct MocSDH : public ops::SendDataHandler
    {
        int dataNotifications{ 0 };
        ops::ObjectName_T m_topicName;
        ops::Address_T m_ip;
        int m_port{ 0 };

        virtual void addSink(const ops::ObjectName_T& topic, const ops::Address_T& ip, const int& port, bool staticRoute = false)
        {
            UNUSED(staticRoute);
            ++dataNotifications;
            m_topicName = topic;
            m_ip = ip;
            m_port = port;
        }

        virtual bool sendData(char* , int , const ops::Topic& ) override
        {
            return false;
        }

    };

    struct ErrorListener : public ops::Listener<ops::Error*>
    {
        int errorCode{ 0 };

        void onNewEvent(ops::Notifier<ops::Error*>* , ops::Error* error) override
        {
            errorCode = error->getErrorCode();
        }
    };
}

// Fixture class for GoogleTest
class Test_MetaData : public ::testing::Test
{
public:
    Test_MetaData() : soc(SetupOPSConfig::configType::meta)
    {
    }
protected:
    void SetUp() override
    {
        part = std::unique_ptr<ops::Participant>(ops::Participant::getInstance("MetaDomain"));
        ASSERT_NE(part, nullptr) << "Failed to create participant";

        ops::Topic top = part->createTopic("MetaTopic");

        // Setup a Subscriber for receiving messages from the test object
        sub = std::unique_ptr<ops::Subscriber>(new ops::Subscriber(top));
        sub->addDataListener([&](ops::DataNotifier* /*subscriber*/) 
            { 
                ++dataNotifications;
                infoData = *dynamic_cast<ops::ParticipantInfoData*>(sub->getMessage()->getData());
            }
        );
        sub->start();

        // Setup publisher
        pub = std::unique_ptr<ops::Publisher>(new ops::Publisher(top));

        mocRDH  = std::make_shared<GTest::MocRDH>(*part);
        mocRDH2 = std::make_shared<GTest::MocRDH>(*part);
        mocSDH  = std::make_shared<GTest::MocSDH>();
        mocSDH2 = std::make_shared<GTest::MocSDH>();
        errSvc.addListener(&errors);
    }

    void TearDown() override
    {
    }

    SetupOPSConfig soc;
    GTest::ErrorListener errors;
    ops::ErrorService errSvc;
    std::unique_ptr<ops::Participant> part;
    std::unique_ptr<ops::Subscriber> sub;

    std::shared_ptr<GTest::MocRDH> mocRDH;
    std::shared_ptr<GTest::MocRDH> mocRDH2;

    std::shared_ptr<GTest::MocSDH> mocSDH;
    std::shared_ptr<GTest::MocSDH> mocSDH2;

    std::unique_ptr<ops::Publisher> pub;

    ops::ParticipantInfoData infoData;

    int dataNotifications{ 0 };
};

TEST_F(Test_MetaData, TestQuery) {

    ops::MetaDataHandler mdh(errSvc, "partId", "domId");

    // Query for non-existing topic names
    EXPECT_FALSE(mdh.hasPublisherOn("kalle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("kalle"));

    // Add some topics
    ops::Topic kalle("kalle", 77, "type.kalle", "addr.kalle");
    kalle.setTransport("kalle.tr");

    ops::Topic pelle("pelle", 12345, "type.pelle", "addr.pelle");
    kalle.setTransport("pelle.tr");

    mdh.addPubTopic(kalle);
    mdh.addSubTopic(kalle);
    mdh.addPubTopic(pelle);
    mdh.addSubTopic(pelle);
    mdh.addPubTopic(kalle); //duplicate
    mdh.addSubTopic(kalle); //duplicate

    // Query for non-existing and existing topic names
    EXPECT_FALSE(mdh.hasPublisherOn("olle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("olle"));
    EXPECT_TRUE(mdh.hasPublisherOn("kalle"));
    EXPECT_TRUE(mdh.hasSubscriberOn("kalle"));
    EXPECT_TRUE(mdh.hasPublisherOn("pelle"));
    EXPECT_TRUE(mdh.hasSubscriberOn("pelle"));

    // Remove some topics
    mdh.removePubTopic(kalle);
    mdh.removeSubTopic(kalle);

    // Query for non-existing and existing topic names
    EXPECT_FALSE(mdh.hasPublisherOn("olle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("olle"));
    EXPECT_TRUE(mdh.hasPublisherOn("kalle"));
    EXPECT_TRUE(mdh.hasSubscriberOn("kalle"));
    EXPECT_TRUE(mdh.hasPublisherOn("pelle"));
    EXPECT_TRUE(mdh.hasSubscriberOn("pelle"));

    // Remove rest of topics
    mdh.removePubTopic(kalle);
    mdh.removeSubTopic(kalle);
    mdh.removePubTopic(pelle);
    mdh.removeSubTopic(pelle);

    // Query for non-existing topic names
    EXPECT_FALSE(mdh.hasPublisherOn("olle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("olle"));
    EXPECT_FALSE(mdh.hasPublisherOn("kalle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("kalle"));
    EXPECT_FALSE(mdh.hasPublisherOn("pelle"));
    EXPECT_FALSE(mdh.hasSubscriberOn("pelle"));
}

TEST_F(Test_MetaData, TestPublish) {

    ops::MetaDataHandler mdh(errSvc, "partId", "domId");
    mdh.setup(part->createTopic("MetaTopic"));

    // No messages received yet
    EXPECT_EQ(dataNotifications, 0);
    EXPECT_NE(mdh.getPartInfoName(), infoData.name);

    // Publish default
    mdh.publish();
    EXPECT_EQ(dataNotifications, 1);
    EXPECT_EQ(mdh.getPartInfoName(), infoData.name);
    EXPECT_EQ("C++", infoData.languageImplementation);
    EXPECT_EQ("partId", infoData.id);
    EXPECT_EQ("domId", infoData.domain);
    EXPECT_EQ("", infoData.ip);
    EXPECT_EQ(0, infoData.mc_udp_port);
    EXPECT_EQ(0u, infoData.publishTopics.size());
    EXPECT_EQ(0u, infoData.subscribeTopics.size());
    EXPECT_EQ(0u, infoData.knownTypes.size());

    // Update udp transport info
    mdh.setUdpTransportInfo("4.5.6.7", 99);
    mdh.publish();
    EXPECT_EQ(dataNotifications, 2);
    EXPECT_EQ("4.5.6.7", infoData.ip);
    EXPECT_EQ(99, infoData.mc_udp_port);

    // Add some topics and publish
    ops::Topic kalle("kalle", 77, "type.kalle", "addr.kalle");
    kalle.setTransport("kalle.tr");

    ops::Topic pelle("pelle", 12345, "type.pelle", "addr.pelle");
    kalle.setTransport("pelle.tr");

    mdh.addPubTopic(kalle);
    mdh.addPubTopic(pelle);
    mdh.addPubTopic(kalle); //duplicate
    mdh.addSubTopic(kalle);
    mdh.addSubTopic(pelle);
    mdh.addSubTopic(pelle); //duplicate
    mdh.addSubTopic(pelle); //duplicate
    mdh.publish();
    EXPECT_EQ(dataNotifications, 3);
    EXPECT_EQ(3u, infoData.publishTopics.size());
    EXPECT_EQ(4u, infoData.subscribeTopics.size());

    EXPECT_EQ("kalle", infoData.publishTopics[0].name);
    EXPECT_EQ("pelle", infoData.publishTopics[1].name);
    EXPECT_EQ("kalle", infoData.publishTopics[2].name);

    EXPECT_EQ("kalle", infoData.subscribeTopics[0].name);
    EXPECT_EQ("pelle", infoData.subscribeTopics[1].name);
    EXPECT_EQ("pelle", infoData.subscribeTopics[2].name);
    EXPECT_EQ("pelle", infoData.subscribeTopics[3].name);

    mdh.cleanup();
}

TEST_F(Test_MetaData, TestConfigError) {

    ops::MetaDataHandler mdh(errSvc, "partId", "domId");
    ops::Topic top = part->createTopic("MetaTopic");
    top.setPort(0);
    mdh.setup(top);
    EXPECT_EQ(errors.errorCode, 0);

    ops::Topic kalle("kalle", 77, "type.kalle", "");
    kalle.setTransport(ops::Topic::TRANSPORT_TCP);

    // Try to register receive topic
    mdh.registerReceiveTopic(kalle, mocRDH);
    EXPECT_EQ(errors.errorCode, ops::BasicError::CONFIG_ERROR);
    errors.errorCode = 0;

    // Try to register send topic
    kalle.setTransport(ops::Topic::TRANSPORT_UDP);
    mdh.registerSendTopic(kalle, mocSDH);
    EXPECT_EQ(errors.errorCode, ops::BasicError::CONFIG_ERROR);
    errors.errorCode = 0;

    mdh.cleanup();
}

TEST_F(Test_MetaData, TestPubTopics) {

    ops::MetaDataHandler mdh(errSvc, "partId", "domId");
    mdh.setup(part->createTopic("MetaTopic"));

    // No messages received yet
    EXPECT_EQ(dataNotifications, 0);
    EXPECT_NE(mdh.getPartInfoName(), infoData.name);
    EXPECT_EQ(mocRDH->dataNotifications, 0);
    EXPECT_EQ(errors.errorCode, 0);

    // Create topics
    ops::Topic kalle("kalle", 77, "type.kalle", "addr.kalle");
    kalle.setTransport(ops::Topic::TRANSPORT_TCP);
    ops::Topic olle("olle", 77, "type.olle", "addr.olle");
    olle.setTransport(ops::Topic::TRANSPORT_UDP);

    // Indicate that there is someone publishing on the topics
    mdh.addPubTopic(kalle);
    mdh.addPubTopic(olle);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 1);
    EXPECT_EQ(mocRDH->dataNotifications, 0);    // No callbacks expected
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    // Register receive topics
    mdh.registerReceiveTopic(kalle, mocRDH);
    EXPECT_EQ(errors.errorCode, 0);
    mdh.registerReceiveTopic(olle, mocRDH2);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 2);
    EXPECT_EQ(mocRDH->dataNotifications, 1);    // Callback expected
    EXPECT_EQ(mocRDH->m_topicName, "kalle");
    EXPECT_EQ(mocRDH->m_port, 77);
    EXPECT_EQ(mocRDH->m_ip, "addr.kalle");
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    // Publish the same but with another domainid
    infoData.domain = "zzzzz";
    pub->writeOPSObject(&infoData);
    EXPECT_EQ(dataNotifications, 3);
    EXPECT_EQ(mocRDH->dataNotifications, 1);    // No callback expected

    // Try register a second time with the same RDH
    mdh.registerReceiveTopic(kalle, mocRDH);
    EXPECT_EQ(errors.errorCode, 0);

    // Try register same topic again with another RDH
    mdh.registerReceiveTopic(kalle, mocRDH2);
    EXPECT_EQ(errors.errorCode, ops::BasicError::ALREADY_INUSE);
    errors.errorCode = 0;

    // Try unregister with wrong RDH
    mdh.unregisterReceiveTopic(kalle, mocRDH2);
    EXPECT_EQ(errors.errorCode, ops::BasicError::NO_MATCH);
    errors.errorCode = 0;

    mdh.publish();
    EXPECT_EQ(dataNotifications, 4);
    EXPECT_EQ(mocRDH->dataNotifications, 2);    // Callback expected
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    // Add duplicate
    mdh.addPubTopic(kalle);
    mdh.publish();
    EXPECT_EQ(dataNotifications, 5);
    EXPECT_EQ(mocRDH->dataNotifications, 4);    // 2x callbacks expected
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    // Unregister with correct RDH
    mdh.unregisterReceiveTopic(kalle, mocRDH);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 6);
    EXPECT_EQ(mocRDH->dataNotifications, 4);    // No callback expected
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    mdh.unregisterReceiveTopic(olle, mocRDH2);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 7);
    EXPECT_EQ(mocRDH->dataNotifications, 4);    // No callback expected
    EXPECT_EQ(mocRDH2->dataNotifications, 0);   // No callbacks expected

    mdh.cleanup();
}

TEST_F(Test_MetaData, TestSubTopics) {

    ops::MetaDataHandler mdh(errSvc, "partId", "domId");
    mdh.setup(part->createTopic("MetaTopic"));

    // No messages received yet
    EXPECT_EQ(dataNotifications, 0);
    EXPECT_NE(mdh.getPartInfoName(), infoData.name);
    EXPECT_EQ(mocSDH->dataNotifications, 0);
    EXPECT_EQ(errors.errorCode, 0);

    // Create topics
    ops::Topic kalle("kalle", 77, "type.kalle", "addr.kalle");
    kalle.setTransport(ops::Topic::TRANSPORT_UDP);
    ops::Topic olle("olle", 77, "type.olle", "addr.olle");
    olle.setTransport(ops::Topic::TRANSPORT_TCP);

    // Indicate that there are some subscribers on the topics, and their socket address
    mdh.addSubTopic(kalle);
    mdh.addSubTopic(olle);
    mdh.setUdpTransportInfo("4.5.6.7", 99);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 1);
    EXPECT_EQ(mocSDH->dataNotifications, 0);    // No callbacks expected
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    // Register send topics
    mdh.registerSendTopic(kalle, mocSDH);
    EXPECT_EQ(errors.errorCode, 0);
    mdh.registerSendTopic(olle, mocSDH2);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 2);
    EXPECT_EQ(mocSDH->dataNotifications, 1);    // Callback expected
    EXPECT_EQ(mocSDH->m_topicName, "kalle");
    EXPECT_EQ(mocSDH->m_port, 99);
    EXPECT_EQ(mocSDH->m_ip, "4.5.6.7");
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    // Try register a second time with the same SDH
    mdh.registerSendTopic(kalle, mocSDH);
    EXPECT_EQ(errors.errorCode, 0);

    // Try register same topic again with another SDH
    mdh.registerSendTopic(kalle, mocSDH2);
    EXPECT_EQ(errors.errorCode, ops::BasicError::ALREADY_INUSE);
    errors.errorCode = 0;

    // Try unregister with wrong RDH
    mdh.unregisterSendTopic(kalle, mocSDH2);
    EXPECT_EQ(errors.errorCode, ops::BasicError::NO_MATCH);
    errors.errorCode = 0;

    mdh.publish();
    EXPECT_EQ(dataNotifications, 3);
    EXPECT_EQ(mocSDH->dataNotifications, 2);    // Callback expected
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    // Add duplicate
    mdh.addSubTopic(kalle);
    mdh.publish();
    EXPECT_EQ(dataNotifications, 4);
    EXPECT_EQ(mocSDH->dataNotifications, 4);    // 2x callbacks expected
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    // Unregister with correct SDH
    mdh.unregisterSendTopic(kalle, mocSDH);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 5);
    EXPECT_EQ(mocSDH->dataNotifications, 4);    // No callback expected
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    mdh.unregisterSendTopic(olle, mocSDH2);
    EXPECT_EQ(errors.errorCode, 0);

    mdh.publish();
    EXPECT_EQ(dataNotifications, 6);
    EXPECT_EQ(mocSDH->dataNotifications, 4);    // No callback expected
    EXPECT_EQ(mocSDH2->dataNotifications, 0);   // No callbacks expected

    mdh.cleanup();
}
