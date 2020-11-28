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

#include "PubIdChecker.h"

using namespace ops;

// ===============================
// Helper classes

class MyPubIdNotifyListener : public Listener<PublicationIdNotification_T>
{
public:
	int Counter;
	PublicationIdNotification_T value;
	MyPubIdNotifyListener() : Counter(0) {}
	~MyPubIdNotifyListener() = default;

	virtual void onNewEvent(Notifier<PublicationIdNotification_T>* , PublicationIdNotification_T const arg) override
	{ 
		value = arg;
		++Counter;
	}

	MyPubIdNotifyListener(const MyPubIdNotifyListener& r) = delete;
	MyPubIdNotifyListener& operator= (const MyPubIdNotifyListener& l) = delete;
	MyPubIdNotifyListener(MyPubIdNotifyListener&&) = delete;
	MyPubIdNotifyListener& operator =(MyPubIdNotifyListener&&) = delete;
};

static void NextId(OPSMessage& m)
{
	m.setPublicationID(m.getPublicationID() + 1);
}

TEST(Test_PubIdChecker, Test) {

	PublicationIdChecker pic;
	MyPubIdNotifyListener myl;
	pic.addListener(&myl);
	EXPECT_EQ(myl.Counter, 0);

	// Setup OPSMessages
	OPSMessage m1, m2, m3;

	m1.setSource("192.168.0.1", 6666);
	m2.setSource("192.168.0.1", 7777);
	m3.setSource("192.168.0.2", 7777);

	m1.setPublicationID(17);
	m2.setPublicationID(988);
	m3.setPublicationID(1);

	// Expect new publisher notification
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 1);
	EXPECT_TRUE(myl.value.newPublisher);
	EXPECT_EQ(myl.value.mess, &m1);
	EXPECT_EQ(myl.value.expectedPubID, 17);

	// Expect no notification
	NextId(m1);
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 1);

	NextId(m1);
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 1);

	// Expect notification, Same number as before
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 2);
	EXPECT_FALSE(myl.value.newPublisher);
	EXPECT_EQ(myl.value.mess, &m1);
	EXPECT_EQ(myl.value.expectedPubID, 20);

	// Expect no notification
	NextId(m1);
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 2);

	// Expect notification, Missed number
	NextId(m1);
	NextId(m1);
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 3);
	EXPECT_FALSE(myl.value.newPublisher);
	EXPECT_EQ(myl.value.mess, &m1);
	EXPECT_EQ(myl.value.expectedPubID, 21);

	// Expect new publisher notification (due to new port)
	pic.Check(&m2);
	EXPECT_EQ(myl.Counter, 4);
	EXPECT_TRUE(myl.value.newPublisher);
	EXPECT_EQ(myl.value.mess, &m2);
	EXPECT_EQ(myl.value.expectedPubID, 988);

	// Expect new publisher notification (due to new address)
	pic.Check(&m3);
	EXPECT_EQ(myl.Counter, 5);
	EXPECT_TRUE(myl.value.newPublisher);
	EXPECT_EQ(myl.value.mess, &m3);
	EXPECT_EQ(myl.value.expectedPubID, 1);

	// Expect no notification
	NextId(m1);
	pic.Check(&m1);
	EXPECT_EQ(myl.Counter, 5);

	NextId(m2);
	pic.Check(&m2);
	EXPECT_EQ(myl.Counter, 5);

	NextId(m3);
	pic.Check(&m3);
	EXPECT_EQ(myl.Counter, 5);
}
