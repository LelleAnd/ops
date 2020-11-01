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

#include "SetupOPSConfig.h"
#include "Participant.h"
#include "Subscriber.h"
#include "MocSendReceiveDataHelpers.h"
#include "listener_helpers.h"

using namespace ops;

TEST(Test_Subscriber, TestNewDeleteLoop) {

	SetupOPSConfig soc;

	Participant* part = Participant::getInstance("GTestDomain", "Kalle");
	ASSERT_NE(part, nullptr) << "Failed to create participant";

	Topic top = part->createTopic("TestTopic");

	for (int i = 0; i < 1000; i++) {
		Subscriber* sub = new Subscriber(top);
		sub->start();
		delete sub;
	}

	delete part;
}

// ======================================================================

struct MySubscriberBase : public ops::SubscriberBase
{
    MySubscriberBase(ops::Topic top) : ops::SubscriberBase(top) {}

    // Callback for received messages
    void onNewEvent(Notifier<OPSMessage*>*, OPSMessage* const message)
    {
    }
    // Callback for deadline timeouts
    void onNewEvent(Notifier<int>*, int)
    {
    }
};

TEST(Test_Subscriber, TestBase) {

    RAII_RDHF z2;
    
    SetupOPSConfig soc;

    std::unique_ptr<ops::Participant> part(ops::Participant::getInstance("GTestDomain", "Kalle"));
	ASSERT_NE(part, nullptr) << "Failed to create participant";
    {
        Topic top = part->createTopic("TestTopic");
        top.setTransport("sub-moc");

        MyConnectListener mcl("");

        MySubscriberBase sub(top);
        sub.addListener(&mcl);
        sub.start();

        std::shared_ptr<MocReceiveDataHandler> rdh = z2.RDHS[top.getName()];

        ASSERT_NE(rdh, nullptr) << "Failed to create MOC RDH";
        EXPECT_EQ(rdh->ss_cnt, 1);
        sub.stop();
        EXPECT_EQ(rdh->ss_cnt, 0);

        sub.start();
        // After a stop and new start we need to get the new RDH
        rdh = z2.RDHS[top.getName()];
        EXPECT_EQ(rdh->ss_cnt, 1);

        // getTopic()

        EXPECT_EQ(sub.numReservedMessages(), 0);

        EXPECT_FALSE(mcl.cst.connected);

        ops::ConnectStatus cst(true, 1);
        rdh->connectStatus(cst);
        EXPECT_TRUE(mcl.cst.connected);
        EXPECT_EQ(mcl.cst.totalNo, 1);
    }
}

// ======================================================================

TEST(Test_Subscriber, TestBasics) {

    RAII_RDHF z2;

    SetupOPSConfig soc;

    std::unique_ptr<ops::Participant> part(ops::Participant::getInstance("GTestDomain", "Kalle"));
    ASSERT_NE(part, nullptr) << "Failed to create participant";
    {
        Topic top = part->createTopic("TestTopic");
        top.setTransport("sub-moc");

        Subscriber sub(top);
        sub.start();

        std::shared_ptr<MocReceiveDataHandler> rdh = z2.RDHS[top.getName()];

        ASSERT_NE(rdh, nullptr) << "Failed to create MOC RDH";
        EXPECT_EQ(rdh->ss_cnt, 1);

        EXPECT_STREQ(sub.getName().c_str(), "");
        sub.setName("sub1");
        EXPECT_STREQ(sub.getName().c_str(), "sub1");

        // deadlineMissedEvent
        // DeadlineQos
        // Deadline listener callback
        // FilterQoS
        // timebased filter
        // History
        // PubIdChecker

        // new data, wait for new data,
        // getDataRef, getMessage
    }
}

// ======================================================================

TEST(Test_Subscriber, TestConnection) {

    // Install function to create our "mocked" Send/Receive DataHandlers
    RAII_SDHF z1;
    RAII_RDHF z2;

    SetupOPSConfig soc;

    std::unique_ptr<ops::Participant> part(ops::Participant::getInstance("GTestDomain", "Kalle"));
    ASSERT_NE(part, nullptr) << "Failed to create participant";
    {
        Topic top = part->createTopic("TestTopic");
        top.setTransport("sub-moc");
        top.setUseAck(true);

        Subscriber sub(top);
        sub.start();

        std::shared_ptr<MocReceiveDataHandler> rdh = z2.RDHS[top.getName()];
        std::shared_ptr<MocSendDataHandler> sdh = z1.SDHS["TestTopic#ack"];

        ASSERT_NE(rdh, nullptr) << "Failed to create MOC RDH";
        EXPECT_EQ(rdh->ss_cnt, 1);
        ASSERT_NE(sdh, nullptr) << "Failed to create MOC SDH";
        EXPECT_EQ(sdh->oc_cnt, 1);
        EXPECT_STREQ(sdh->topic.getName().c_str(), "TestTopic#ack");
        EXPECT_TRUE(sub.CheckPublisher(nullptr));

        EXPECT_EQ(top.getRegisterTimeMs(), 100);

        // Activate
        for (int i = 0; i < 3; i++) {
            sub.Activate();
            std::this_thread::sleep_for(std::chrono::milliseconds(120));
        }
        // Check that a number of REGISTER has been sent
        EXPECT_EQ(sdh->send_cnt, 3);
        EXPECT_TRUE(sub.CheckPublisher(nullptr));

        sub.AddExpectedPublisher("pub-one");
        EXPECT_FALSE(sub.CheckPublisher("pub-one"));
        EXPECT_FALSE(sub.CheckPublisher(nullptr));

        sub.RemoveExpectedPublisher("pub-one");
        EXPECT_FALSE(sub.CheckPublisher("pub-one"));
        EXPECT_TRUE(sub.CheckPublisher(nullptr));
    }

    // Check that an UNREGISTER has been sent
    std::shared_ptr<MocSendDataHandler> sdh = z1.SDHS["TestTopic#ack"];
    EXPECT_EQ(sdh->send_cnt, 4);
}
