/**
*
* Copyright (C) 2020-2026 Lennart Andersson.
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

#include <thread>

#include "SetupOPSConfig.h"
#include "Participant.h"
#include "Subscriber.h"
#include "Publisher.h"
#include "MocSendReceiveDataHelpers.h"
#include "listener_helpers.h"

using namespace ops;

namespace GTest {
    class TestData : public ops::OPSObject
    {
    public:
        static ops::TypeId_T getTypeName() { return ops::TypeId_T("GTest.TestData"); }
#ifdef OPS_C17_DETECTED
        TestData() : ops::OPSObject(getTypeName())
        {
        }
        TestData(const TestData& __c) : ops::OPSObject(getTypeName())
        {
            __c.fillClone(this);
        }
#else
        TestData() : ops::OPSObject()
        {
            OPSObject::appendType(ops::TypeId_T("GTest.TestData"));
        }
        TestData(const TestData& __c) : ops::OPSObject()
        {
            OPSObject::appendType(ops::TypeId_T("GTest.TestData"));
            __c.fillClone(this);
        }
#endif
        TestData& operator = (const TestData& other)
        {
            other.fillClone(this);
            return *this;
        }
        void serialize(ops::ArchiverInOut* archive)
        {
            ops::OPSObject::serialize(archive);
        }
        virtual ops::OPSObject* clone()
        {
            TestData* ret = new TestData;
            fillClone(ret);
            return ret;
        }
        void fillClone(TestData* obj) const
        {
            ops::OPSObject::fillClone(obj);
        }
        virtual ~TestData(void) {}
    };

    class PubSubFactory_1 : public SerializableFactory
    {
    public:
        virtual Serializable* create(CreateType_T type) override
        {
            if (type == "GTest.TestData") { return new GTest::TestData(); }
            return nullptr;
        }
        PubSubFactory_1() = default;
        virtual ~PubSubFactory_1() = default;
        PubSubFactory_1(const PubSubFactory_1& r) = delete;
        PubSubFactory_1& operator= (const PubSubFactory_1& l) = delete;
        PubSubFactory_1(PubSubFactory_1&&) = delete;
        PubSubFactory_1& operator =(PubSubFactory_1&&) = delete;
    };

}

// Fixture class for GoogleTest
class Test_PubSub : public ::testing::Test
{
protected:
    void SetUp() override
    {
        part = std::unique_ptr<ops::Participant>(ops::Participant::getInstance("GTestDomain", "Kalle"));
        ASSERT_NE(part, nullptr) << "Failed to create participant";

        part->addTypeSupport(new GTest::PubSubFactory_1());

        Topic top = part->createTopic("TestTopic");
        top.setUseAck(true);
        EXPECT_EQ(top.getNumResends(), 5);      // Configured value in SetupOPSConfig
        EXPECT_EQ(top.getRegisterTimeMs(), 100);

        // Setup Subscriber
        top.setTransport("sub-moc");
        sub = std::unique_ptr<Subscriber>(new Subscriber(top));
        sub->setName("sub-one");
        sub->addDataListener([&](ops::DataNotifier* /*subscriber*/) { dataNotifications++; });
        sub->start();

        sub_rdh = z2.RDHS[top.getName()];
        sub_sdh = z1.SDHS["TestTopic#ack"];

        ASSERT_NE(sub_rdh, nullptr) << "Failed to create MOC RDH";
        EXPECT_EQ(sub_rdh->ss_cnt, 1);
        ASSERT_NE(sub_sdh, nullptr) << "Failed to create MOC SDH";
        EXPECT_EQ(sub_sdh->oc_cnt, 1);
        EXPECT_STREQ(sub_sdh->topic.getName().c_str(), "TestTopic#ack");

        // Setup publisher
        top.setTransport("pub-moc");
        pub = std::unique_ptr<Publisher>(new Publisher(top));
        pub->setName("pub-one");
        pub->AddExpectedAckSender("sub-one");
        pub_sdh = z1.SDHS[top.getName()];
        pub_rdh = z2.RDHS["TestTopic#ack"];

        ASSERT_NE(pub_sdh, nullptr) << "Failed to create MOC SDH";
        EXPECT_EQ(pub_sdh->oc_cnt, 1);
        ASSERT_NE(pub_rdh, nullptr) << "Failed to create MOC RDH";
        EXPECT_EQ(pub_rdh->ss_cnt, 1);
        EXPECT_STREQ(pub_rdh->topic.getName().c_str(), "TestTopic#ack");
        EXPECT_FALSE(pub->CheckAckSender("sub-one"));

        // Connect sdh and rdh for TestTopic and the acknowledge
        pub_sdh->rdh = sub_rdh;
        sub_sdh->rdh = pub_rdh;
    }

    void TearDown() override 
    {
    }

    // Install function to create our "mocked" Send/Receive DataHandlers
    RAII_SDHF z1;
    RAII_RDHF z2;
    std::shared_ptr<MocReceiveDataHandler> sub_rdh;
    std::shared_ptr<MocSendDataHandler> sub_sdh;
    std::shared_ptr<MocSendDataHandler> pub_sdh;
    std::shared_ptr<MocReceiveDataHandler> pub_rdh;

    SetupOPSConfig soc;

    std::unique_ptr<Participant> part;
    std::unique_ptr<Subscriber> sub;
    std::unique_ptr<Publisher> pub;

    GTest::TestData obj;
    int dataNotifications = 0;
};

TEST_F(Test_PubSub, TestNormal) {

    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    // --------------------------------------------------------------------------------
    // Write normal 
    // Publish some data, should result in an ack from the subscriber
    obj.setKey("test0");
    pub->writeOPSObject(&obj);

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 1);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Check work in publisher
    for (int i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub->Activate();
    }
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);

    // Check data incl. PubId
    ASSERT_NE(sub->getMessage(), nullptr);
    EXPECT_EQ(sub->getMessage()->getPublicationID(), 0);
    EXPECT_STREQ(sub->getDataReference()->getKey().c_str(), "test0");
    EXPECT_EQ(dataNotifications, 1);
}

TEST_F(Test_PubSub, TestLostData) {

    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    // --------------------------------------------------------------------------------
    // Write simulating lost publications
    pub_sdh->forward = false;

    obj.setKey("test1");
    pub->writeOPSObject(&obj);

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 0);
    EXPECT_EQ(sub_sdh->send_cnt, 0);
    EXPECT_EQ(pub_rdh->rcv_cnt, 0);
    EXPECT_FALSE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Each call to Activate should be a resend (time > 10 ms default resendTimeout)
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 3);
    EXPECT_EQ(sub_rdh->rcv_cnt, 0);
    EXPECT_EQ(sub_sdh->send_cnt, 0);
    EXPECT_EQ(pub_rdh->rcv_cnt, 0);
    EXPECT_FALSE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Connect data again
    pub_sdh->forward = true;

    // First call to Activate should be a resend and second should not be anything to do 
    // (time > 10 ms default resendTimeout)
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 4);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 1);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 4);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 1);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);

    // Check data incl. PubId
    ASSERT_NE(sub->getMessage(), nullptr);
    EXPECT_EQ(sub->getMessage()->getPublicationID(), 0);
    EXPECT_STREQ(sub->getDataReference()->getKey().c_str(), "test1");
    EXPECT_EQ(dataNotifications, 1);
}

TEST_F(Test_PubSub, TestLostAck) {

    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    // --------------------------------------------------------------------------------
    // Write simulating lost ACK's
    sub_sdh->forward = false;

    obj.setKey("test2");
    pub->writeOPSObject(&obj);

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 1);
    EXPECT_EQ(pub_rdh->rcv_cnt, 0);
    EXPECT_FALSE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Each call to Activate should be a resend (time > 10 ms default resendTimeout)
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 3);
    EXPECT_EQ(sub_rdh->rcv_cnt, 3);
    EXPECT_EQ(sub_sdh->send_cnt, 3);
    EXPECT_EQ(pub_rdh->rcv_cnt, 0);
    EXPECT_FALSE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Connect ACK's again
    sub_sdh->forward = true;

    // First call to Activate should be a resend and second should not be anything to do 
    // (time > 10 ms default resendTimeout)
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 4);
    EXPECT_EQ(sub_rdh->rcv_cnt, 4);
    EXPECT_EQ(sub_sdh->send_cnt, 4);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 4);
    EXPECT_EQ(sub_rdh->rcv_cnt, 4);
    EXPECT_EQ(sub_sdh->send_cnt, 4);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);

    // Check data incl. PubId
    ASSERT_NE(sub->getMessage(), nullptr);
    EXPECT_EQ(sub->getMessage()->getPublicationID(), 0);
    EXPECT_STREQ(sub->getDataReference()->getKey().c_str(), "test2");
    EXPECT_EQ(dataNotifications, 1);
}

TEST_F(Test_PubSub, TestResendLatest1) {

    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    // Send a REGISTER message
    sub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 0);
    EXPECT_EQ(sub_rdh->rcv_cnt, 0);
    EXPECT_EQ(sub_sdh->send_cnt, 1);
    EXPECT_EQ(pub_rdh->rcv_cnt, 1);
    EXPECT_FALSE(sub->CheckPublisher("pub-one"));
    EXPECT_TRUE(sub->CheckPublisher(""));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    obj.setKey("test3");
    pub->writeOPSObject(&obj);

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 2);
    EXPECT_EQ(pub_rdh->rcv_cnt, 2);
    EXPECT_TRUE(sub->CheckPublisher("pub-one"));
    EXPECT_TRUE(sub->CheckPublisher(""));
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Should not be any messages sent by this
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    sub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 2);
    EXPECT_EQ(pub_rdh->rcv_cnt, 2);

    pub->Activate();
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);

    // Adding another expected publisher should result in a REGISTER message sent
    // but no response since the subscriber is already known by the publisher.
    sub->AddExpectedPublisher("test");

    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    sub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 3);
    EXPECT_EQ(pub_rdh->rcv_cnt, 3);
    EXPECT_FALSE(sub->CheckPublisher("test"));
    EXPECT_TRUE(sub->CheckPublisher("pub-one"));
    EXPECT_FALSE(sub->CheckPublisher(""));
    EXPECT_EQ(dataNotifications, 1);
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);
}

TEST_F(Test_PubSub, TestResendLatest2) {

    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::init);

    // Write simulating lost publications just to have a message for resending
    pub_sdh->forward = false;

    obj.setKey("test4");
    pub->writeOPSObject(&obj);

    EXPECT_EQ(pub_sdh->send_cnt, 1);
    EXPECT_EQ(sub_rdh->rcv_cnt, 0);
    EXPECT_EQ(sub_sdh->send_cnt, 0);
    EXPECT_EQ(pub_rdh->rcv_cnt, 0);
    EXPECT_FALSE(pub->CheckAckSender("sub-one"));
    EXPECT_EQ(dataNotifications, 0);
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::sending);

    // Connect sender and receiver again
    pub_sdh->forward = true;

    // Send a REGISTER message, should result in a resend of the latest
    sub->Activate();

    EXPECT_EQ(pub_sdh->send_cnt, 2);
    EXPECT_EQ(sub_rdh->rcv_cnt, 1);
    EXPECT_EQ(sub_sdh->send_cnt, 2);
    EXPECT_EQ(pub_rdh->rcv_cnt, 2);
    EXPECT_TRUE(pub->CheckAckSender("sub-one"));
    EXPECT_TRUE(sub->CheckPublisher("pub-one"));
    EXPECT_TRUE(sub->CheckPublisher(""));
    EXPECT_EQ(dataNotifications, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    pub->Activate();
    EXPECT_EQ(pub->getSendState(), ops::Publisher::SendState::acked);
}
