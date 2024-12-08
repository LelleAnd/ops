
//
// Google Test demo of unit testing of class 'ClassUnderTest' that is using OPS Publishers and Subscribers.
//
// By using transport 'inprocess' (configurable in ops_config.xml), we can set up our own Subscriber and Publisher
// here in the test code and have them directly communicate with those in ClassUnderTest.
//

#include "gtest/gtest.h"

#include "ClassUnderTest.h"


class CutTestFixture : public ::testing::Test {
protected:
	TestAll::ChildData data;
	TestAll::ChildDataSubscriber* sub{ nullptr };
	TestAll::ChildDataPublisher* pub{ nullptr };

	ops::Topic cutPublishTopic;
	ops::Topic cutSubscribeTopic;


	void SetUp()
	{
		ops::Participant* const participant = ops::Participant::getInstance("TestAllDomain");

		cutPublishTopic = participant->createTopic("AA");
		cutSubscribeTopic = participant->createTopic("BB");

		pub = new TestAll::ChildDataPublisher(cutSubscribeTopic);
		
		sub = new TestAll::ChildDataSubscriber(cutPublishTopic);
		sub->start();
	}

	void TearDown()
	{
		delete sub;
		sub = nullptr;

		delete pub;
		pub = nullptr;
	}

};

TEST_F(CutTestFixture, Test)
{
	ClassUnderTest cut(cutSubscribeTopic, cutPublishTopic);

	// Check init values
	auto val = cut.getValue();
	EXPECT_FALSE(val.first);
	EXPECT_EQ(val.second, -1);

	// Send data to cut via OPS
	data.l = 42;
	pub->write(data);

	// Check received values
	val = cut.getValue();
	EXPECT_TRUE(val.first);
	EXPECT_EQ(val.second, 42);

	// Let cut send data via OPS
	cut.update(17);

	// Verify what was sent
	EXPECT_TRUE(sub->newDataExist());
	TestAll::ChildData chdata;
	sub->getData(chdata);
	EXPECT_EQ(chdata.l, 17);

	// Let cut send data via OPS
	cut.update(99);

	// Verify what was sent
	EXPECT_TRUE(sub->newDataExist());
	sub->getData(chdata);
	EXPECT_EQ(chdata.l, 99);

}
