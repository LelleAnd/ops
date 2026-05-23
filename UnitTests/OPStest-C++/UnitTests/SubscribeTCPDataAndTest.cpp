#include "SubscribeDataAndTest.h"


#ifndef _WIN32
#include <time.h>

int64_t getNow()
{
    timespec ts;
    memset(&ts, 0, sizeof(ts));
    //clock_gettime(CLOCK_REALTIME, &ts)
    return ((1000 * ts.tv_sec) + (ts.tv_nsec / 1000000));
}

#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, nullptr);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#else
int64_t getNow()
{
    return (int64_t)timeGetTime();
}
#endif

std::vector<pizza::special::ExtraAllt> receivedPizzaVec;

typedef CHelper<pizza::special::ExtraAllt, pizza::special::ExtraAlltSubscriber> TExtraAlltHelper;

ItemInfo* itemInfo = nullptr;

class MyListener : public CHelperListener<pizza::special::ExtraAllt>
{
public:
	virtual void onData(ops::Subscriber* , int64_t pubId, pizza::special::ExtraAllt* data) override {
		receivedPizzaVec.push_back(*data);
		std::cout << "GOT EXTRA_ALLT TCP DATA " << pubId << ", size = " << receivedPizzaVec.size() << std::endl;
	}
	MyListener() = default;
	virtual ~MyListener() = default;
	MyListener(const MyListener& r) = delete;
	MyListener& operator= (const MyListener& l) = delete;
	MyListener(MyListener&&) = delete;
	MyListener& operator =(MyListener&&) = delete;
};


//test class
class Test_OPS_Publisher_And_Subscriber : public testing::Test
{
public:
	Test_OPS_Publisher_And_Subscriber() {}
	virtual ~Test_OPS_Publisher_And_Subscriber() {}
	Test_OPS_Publisher_And_Subscriber(const Test_OPS_Publisher_And_Subscriber& r) = delete;
	Test_OPS_Publisher_And_Subscriber& operator= (const Test_OPS_Publisher_And_Subscriber& l) = delete;
	Test_OPS_Publisher_And_Subscriber(Test_OPS_Publisher_And_Subscriber&&) = delete;
	Test_OPS_Publisher_And_Subscriber& operator =(Test_OPS_Publisher_And_Subscriber&&) = delete;
};


int main(int argc, char**argv)
{

	// --------------------------------------------------------------------
	MyListener myListener;

	itemInfo = new ItemInfo("PizzaDomain", "TcpExtraAlltTopic", "pizza.special.ExtraAllt");
	// Setup the OPS static error service (common for all participants, reports errors during participant creation)
	ops::ErrorWriter* const errorWriterStatic = new ops::ErrorWriter(std::cout);
	ops::Participant::getStaticErrorService()->addListener(errorWriterStatic);

	// Create participants
	// NOTE that the second parameter (participantID) must be different for the two participant instances
	ops::Participant* const participant = ops::Participant::getInstance("PizzaDomain", "PizzaDomain", "UnitTests/OPStest-C++/ops_config.xml");
	if (participant == nullptr) {
	    std::cout << "Failed to create Participant. Missing ops_config.xml ??" << std::endl;
		exit(-1);
	}
	participant->addTypeSupport(new PizzaProject::PizzaProjectTypeFactory());

	// Add error writers to catch internal ops errors
	ops::ErrorWriter* errorWriter = new ops::ErrorWriter(std::cout);
	participant->getErrorService()->addListener(errorWriter);

	// Finish up our ItemInfo's
	itemInfo->helper = (new TExtraAlltHelper(&myListener));
	itemInfo->part = participant;
	itemInfo->selected = true;
	itemInfo->helper->CreateSubscriber(itemInfo->part, itemInfo->TopicName);
	itemInfo->helper->StartSubscriber();
	std::cout << "start linstening on TCP" << std::endl;
	ops::TimeHelper::sleep(std::chrono::seconds(24)); //listen on data for 24 seconds

	//delete objects
	delete itemInfo->helper;
	itemInfo->helper = nullptr;
	itemInfo->part = nullptr;
	delete itemInfo;
	itemInfo = nullptr;

	participant->getErrorService()->removeListener(errorWriter);

	delete errorWriter; errorWriter = nullptr;

	///TODO this should be done by asking Participant to delete instances??
	delete participant;

    //run unit tests
    ::testing::InitGoogleTest(&argc, argv);
    int const result = RUN_ALL_TESTS();

    return result;
}


TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_NORMAL_EXTRA_ALLT_TCP) {
	EXPECT_EQ(receivedPizzaVec.size(), (size_t)22);
	test::testExtraAlltNormal(receivedPizzaVec.at(0), TCP);
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_LARGE_EXTRA_ALLT_TCP) {
	test::testExtraAlltLarge(receivedPizzaVec.at(1), TCP);
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_BRUST_NORMAL_EXTRA_ALLT_TCP) {
	for(int i = 2; i < 12; ++i) {
		test::testExtraAlltNormal(receivedPizzaVec.at(i), TCP);
	}
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_BURST_LARGE_EXTRA_ALLT_TCP) {
	for(int i = 12; i < 22; ++i) {
		test::testExtraAlltLarge(receivedPizzaVec.at(i), TCP);
	}
}
