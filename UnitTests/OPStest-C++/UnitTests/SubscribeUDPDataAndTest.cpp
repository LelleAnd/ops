#include "SubscribeDataAndTest.h"

#undef USE_MESSAGE_HEADER

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

template <class DataType>
class CHelperListener
{
public:
	virtual void onData(ops::Subscriber* sub, DataType* data) = 0;
	virtual ~CHelperListener() {}
  CHelperListener() {}
  CHelperListener(const CHelperListener&) = delete;
  CHelperListener(CHelperListener&&) = delete;
  CHelperListener& operator=(const CHelperListener&) = delete;
  CHelperListener& operator=(CHelperListener&&) = delete;
};

class IHelper
{
public:
	virtual void CreateSubscriber(ops::Participant* part, std::string topicName) = 0;
	virtual void DeleteSubscriber(bool doLog = true) = 0;
	virtual void StartSubscriber() = 0;
	virtual void StopSubscriber() = 0;
	virtual ~IHelper() {};
  IHelper() {}
  IHelper(const IHelper&) = delete;
  IHelper(IHelper&&) = delete;
  IHelper& operator=(const IHelper&) = delete;
  IHelper& operator=(IHelper&&) = delete;
};

template <class DataType, class DataTypePublisher, class DataTypeSubscriber>
class CHelper : public IHelper, ops::DataListener, ops::DeadlineMissedListener
{
public:
	DataType data;

	CHelper(CHelperListener<DataType>* client_):
		client(client_), pub(nullptr), sub(nullptr), expectedPubId(-1)
	{
	}

	virtual ~CHelper()
	{
		DeleteSubscriber(false);
	}

	CHelper() = delete;
	CHelper(const CHelper& r) = delete;
	CHelper& operator= (const CHelper& l) = delete;
	CHelper(CHelper&&) = delete;
	CHelper& operator =(CHelper&&) = delete;

	virtual void CreateSubscriber(ops::Participant* part, std::string topicName) override
	{
		if (sub != nullptr) {
			std::cout << "Subscriber already exist for topic " << sub->getTopic().getName() << std::endl;
		} else {
			try {
				//Create topic, might throw ops::NoSuchTopicException
				ops::Topic topic = part->createTopic(topicName.c_str());

				//Create a subscriber on that topic.
				sub = new DataTypeSubscriber(topic);
				sub->addDataListener(this);
				sub->deadlineMissedEvent.addDeadlineMissedListener(this);

				sub->start();
			}
			catch (...) {
				std::cout << "Requested topic '" << topicName << "' does not exist!!" << std::endl;
			}
		}
	}

	virtual void DeleteSubscriber(bool doLog = true) override
	{
		if (sub != nullptr) {
			std::cout << "Deleting subscriber for topic " << sub->getTopic().getName() << std::endl;
			sub->stop();
			delete sub;
			sub = nullptr;
		} else {
			if (doLog) { std::cout << "Subscriber must be created first!!" << std::endl; }
		}
	}

	virtual void StartSubscriber() override
	{
		if (sub != nullptr) {
			std::cout << "Starting subscriber for topic " << sub->getTopic().getName() << std::endl;
			sub->start();
		} else {
			std::cout << "Subscriber must be created first!!" << std::endl;
		}

	}

	virtual void StopSubscriber() override
	{
		if (sub != nullptr) {
			std::cout << "Stoping subscriber for topic " << sub->getTopic().getName() << std::endl;
			sub->stop();
		} else {
			std::cout << "Subscriber must be created first!!" << std::endl;
		}
	}
	void SetDeadlineQos(int64_t timeoutMs)
	{
		if (sub != nullptr) {
			std::cout << "Setting deadlineQos to " << timeoutMs << " [ms] for topic " << sub->getTopic().getName() << std::endl;
			sub->setDeadlineQoS(timeoutMs);
		} else {
			std::cout << "Subscriber must be created first!!" << std::endl;
		}
	}

	///Override from ops::DataListener, called whenever new data arrives.
	virtual void onNewData(ops::DataNotifier* subscriber) override
	{
		if(subscriber == sub)
		{
			// Check if we have lost any messages. We use the publicationID and that works as long as
			// it is the same publisher sending us messages.
			ops::OPSMessage* newMess = sub->getMessage();

			if (expectedPubId >= 0) {
				if (expectedPubId != newMess->getPublicationID()) {
					std::cout << ">>>>> Lost message for topic " << sub->getTopic().getName() <<
						". Exp.pubid: " << expectedPubId << " got: " << newMess->getPublicationID() << std::endl;
				}
			}
			expectedPubId = newMess->getPublicationID() + 1;
			client->onData(sub, (DataType*)newMess->getData());
		}
	}

	///Override from ops::DeadlineMissedListener, called if no new data has arrived within deadlineQoS.
	virtual void onDeadlineMissed(ops::DeadlineMissedEvent* evt) override
	{
		UNUSED(evt)
		std::cout << "Deadline Missed for topic " << sub->getTopic().getName() << std::endl;
	}

private:
	CHelperListener<DataType>* client;
	ops::Publisher* pub;
	ops::Subscriber* sub;
	int64_t expectedPubId;
};


std::vector<pizza::special::ExtraAllt> receivedPizzaVec;

typedef CHelper<pizza::special::ExtraAllt, pizza::special::ExtraAlltPublisher, pizza::special::ExtraAlltSubscriber> TExtraAlltHelper;

struct ItemInfo {
	std::string Domain;
	std::string TopicName;
	std::string TypeName;

	bool selected = false;
	IHelper* helper = nullptr;
	ops::Participant* part = nullptr;

	ItemInfo(std::string const dom, std::string const top, std::string const typ):
		Domain(dom), TopicName(top), TypeName(typ)
	{
	}

	ItemInfo() = delete;
	~ItemInfo() = default;
	ItemInfo(const ItemInfo& r) = delete;
	ItemInfo& operator= (const ItemInfo& l) = delete;
	ItemInfo(ItemInfo&&) = delete;
	ItemInfo& operator =(ItemInfo&&) = delete;
};
ItemInfo* itemInfo = nullptr;


class MyListener : public CHelperListener<pizza::special::ExtraAllt>
{
public:
	virtual void onData(ops::Subscriber* sub, pizza::special::ExtraAllt* data) override {
		UNUSED(sub)
		receivedPizzaVec.push_back(*data);
		std::cout << "GETS EXTRA_ALLT UDP DATA, size = " << receivedPizzaVec.size() << std::endl;
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

	itemInfo = new ItemInfo("PizzaDomain", "UdpExtraAlltTopic", "pizza.special.ExtraAllt");
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
	ops::Participant* const otherParticipant = ops::Participant::getInstance("OtherPizzaDomain", "OtherPizzaDomain", "UnitTests/OPStest-C++/ops_config.xml");
	if (otherParticipant == nullptr) {
		std::cout << "Failed to create Participant. Missing ops_config.xml ??" << std::endl;
        exit(-1);
	}
	otherParticipant->addTypeSupport(new PizzaProject::PizzaProjectTypeFactory());

	// Add error writers to catch internal ops errors
	ops::ErrorWriter* errorWriter = new ops::ErrorWriter(std::cout);
	participant->getErrorService()->addListener(errorWriter);
	ops::ErrorWriter* errorWriter2 = new ops::ErrorWriter(std::cout);
	otherParticipant->getErrorService()->addListener(errorWriter2);

	// Finish up our ItemInfo's
	itemInfo->helper = (new TExtraAlltHelper(&myListener));
	itemInfo->part = participant;
	itemInfo->selected = true;
	itemInfo->helper->CreateSubscriber(itemInfo->part, itemInfo->TopicName);
	itemInfo->helper->StartSubscriber();
	std::cout << "start linstening on udp" << std::endl;
	ops::TimeHelper::sleep(std::chrono::seconds(26)); //listen on data 26 seconds

	//delete objects

	delete itemInfo->helper;
	itemInfo->helper = nullptr;
	itemInfo->part = nullptr;
	delete itemInfo;
	itemInfo = nullptr;

	participant->getErrorService()->removeListener(errorWriter);
	otherParticipant->getErrorService()->removeListener(errorWriter2);

	delete errorWriter; errorWriter = nullptr;
	delete errorWriter2; errorWriter2 = nullptr;

	///TODO this should be done by asking Participant to delete instances??
	delete participant;

    //run unit tests
    ::testing::InitGoogleTest(&argc, argv);
    int const result = RUN_ALL_TESTS();

    return result;
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_NORMAL_EXTRA_ALLT_UDP) {
	EXPECT_EQ(receivedPizzaVec.size(), (size_t)22);
	test::testExtraAlltNormal(receivedPizzaVec.at(0), UDP);
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_LARGE_EXTRA_ALLT_UDP) {
	test::testExtraAlltLarge(receivedPizzaVec.at(1), UDP);
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_BRUST_NORMAL_EXTRA_ALLT_UDP) {
	for(int i = 2; i < 12; ++i) {
		test::testExtraAlltNormal(receivedPizzaVec.at(i), UDP);
	}
}

TEST_F(Test_OPS_Publisher_And_Subscriber, TESTING_BURST_LARGE_EXTRA_ALLT_UDP) {
	for(int i = 12; i < 22; ++i) {
		test::testExtraAlltLarge(receivedPizzaVec.at(i), UDP);
	}
}
