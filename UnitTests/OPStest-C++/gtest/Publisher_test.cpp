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

#include <thread>

#include "gtest/gtest.h"

#include "SetupOPSConfig.h"
#include "Participant.h"
#include "Publisher.h"
#include "MocSendReceiveDataHelpers.h"

TEST(Test_Publisher, TestNewDeleteLoop) {

	SetupOPSConfig soc;

	ops::Participant* part = ops::Participant::getInstance("GTestDomain", "Kalle");
	ASSERT_NE(part, nullptr) << "Failed to create participant";

	ops::Topic top = part->createTopic("TestTopic");

	for (int i = 0; i < 1000; i++) {
		ops::Publisher* pub = new ops::Publisher(top);
		delete pub;
	}

	delete part;
}

TEST(Test_Publisher, TestBasics) {

    // Install function to create our "mocked" SendDataHandler
    RAII_SDHF z;

    SetupOPSConfig soc;

    std::unique_ptr<ops::Participant> part( ops::Participant::getInstance("GTestDomain", "Kalle2") );
    ASSERT_NE(part, nullptr) << "Failed to create participant";

    {
        ops::Topic top = part->createTopic("TestTopic2");
        top.setTransport("pub-moc");

        ops::Publisher pub(top);
        std::shared_ptr<MocSendDataHandler> sdh = z.SDHS[top.getName()];

        ASSERT_NE(sdh, nullptr) << "Failed to create MOC SDH";
        
        EXPECT_EQ(sdh->oc_cnt, 1);
        pub.start();
        EXPECT_EQ(sdh->oc_cnt, 1);
        pub.stop();
        EXPECT_EQ(sdh->oc_cnt, 0);
        pub.start();
        EXPECT_EQ(sdh->oc_cnt, 1);

        //ops::Topic top2 = pub.getTopic();
        //TODO

        EXPECT_STREQ(pub.getName().c_str(), "");
        pub.setName("Kalle");
        EXPECT_STREQ(pub.getName().c_str(), "Kalle");

        EXPECT_STREQ(pub.getKey().c_str(), "");
        pub.setKey("sun");
        EXPECT_STREQ(pub.getKey().c_str(), "sun");

        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::init);

        ops::OPSObject obj;
        EXPECT_EQ(sdh->send_cnt, 0);
        EXPECT_TRUE(pub.writeOPSObject(&obj));
        EXPECT_EQ(sdh->send_cnt, 1);

        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::acked);
    }
}

TEST(Test_Publisher, TestResends) {

    // Install function to create our "mocked" Send/Receive DataHandlers
    RAII_SDHF z1;
    RAII_RDHF z2;

    SetupOPSConfig soc;

    std::unique_ptr<ops::Participant> part(ops::Participant::getInstance("GTestDomain", "Kalle2"));
    ASSERT_NE(part, nullptr) << "Failed to create participant";

    {
        ops::OPSObject obj;
        ops::Topic top = part->createTopic("TestTopic2");
        top.setTransport("pub-moc");
        top.setUseAck(true);
        EXPECT_EQ(top.getNumResends(), 5);      // Configured value in SetupOPSConfig

        ops::Publisher pub(top);
        std::shared_ptr<MocSendDataHandler> sdh = z1.SDHS["TestTopic2"];
        std::shared_ptr<MocReceiveDataHandler> rdh = z2.RDHS["TestTopic2#ack"];

        ASSERT_NE(sdh, nullptr) << "Failed to create MOC SDH";
        ASSERT_NE(rdh, nullptr) << "Failed to create MOC RDH";

        EXPECT_EQ(rdh->ss_cnt, 1);
        EXPECT_STREQ(rdh->topic.getName().c_str(), "TestTopic2#ack");

        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::init);

        // Send data with no expected ACK's
        EXPECT_EQ(sdh->send_cnt, 0);
        EXPECT_TRUE(pub.writeOPSObject(&obj));
        EXPECT_EQ(sdh->send_cnt, 1);

        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            pub.Activate();
        }
        EXPECT_EQ(sdh->send_cnt, 1);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::acked);

        // Check without any added 
        EXPECT_FALSE(pub.CheckAckSender("sub-one"));
        EXPECT_TRUE(pub.CheckAckSender(""));

        // Add expected ACK's
        pub.AddExpectedAckSender("sub-one");

        EXPECT_FALSE(pub.CheckAckSender("sub-one"));
        EXPECT_FALSE(pub.CheckAckSender(""));

        // Send data with expected ACK's
        EXPECT_TRUE(pub.writeOPSObject(&obj));
        EXPECT_EQ(sdh->send_cnt, 2);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            pub.Activate();
        }

        EXPECT_EQ(sdh->send_cnt, 7);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::failed);

        EXPECT_FALSE(pub.CheckAckSender("sub-one"));
        EXPECT_FALSE(pub.CheckAckSender(""));

        pub.RemoveExpectedAckSender("sub-one");
        EXPECT_FALSE(pub.CheckAckSender("sub-one"));
        EXPECT_TRUE(pub.CheckAckSender(""));

        // Send data with expected ACK's using Activate with lamda
        pub.AddExpectedAckSender("sub-one");
        EXPECT_TRUE(pub.writeOPSObject(&obj));
        EXPECT_EQ(sdh->send_cnt, 8);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub.Activate([](const ops::ObjectName_T& subname, int32_t totfail) { return ops::Publisher::ResendAlternative_T::yes; });

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub.Activate([](const ops::ObjectName_T& subname, int32_t totfail) { return ops::Publisher::ResendAlternative_T::yes; });

        EXPECT_EQ(sdh->send_cnt, 10);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub.Activate([](const ops::ObjectName_T& subname, int32_t totfail) { return ops::Publisher::ResendAlternative_T::no; });

        EXPECT_EQ(sdh->send_cnt, 10);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::acked);

        EXPECT_TRUE(pub.writeOPSObject(&obj));
        EXPECT_EQ(sdh->send_cnt, 11);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub.Activate([](const ops::ObjectName_T& subname, int32_t totfail) { return ops::Publisher::ResendAlternative_T::yes; });

        EXPECT_EQ(sdh->send_cnt, 12);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::sending);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pub.Activate([](const ops::ObjectName_T& subname, int32_t totfail) { return ops::Publisher::ResendAlternative_T::no_remove; });

        EXPECT_EQ(sdh->send_cnt, 12);
        EXPECT_EQ(pub.getSendState(), ops::Publisher::SendState::acked);

        EXPECT_FALSE(pub.CheckAckSender("sub-one"));
        EXPECT_TRUE(pub.CheckAckSender(""));
    }
}

