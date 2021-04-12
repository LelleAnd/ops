// PizzaTest.cpp : Defines the entry point for the console application.
//
#include <chrono>

#include "PublishData.h"

#undef USE_MESSAGE_HEADER

#ifndef _WIN32
#include <time.h>



int64_t getNow()
{
    timespec ts;
    memset(&ts, 0, sizeof(ts));
    //clock_gettime(CLOCK_REALTIME, &ts);
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
	virtual void CreatePublisher(ops::Participant* part, std::string topicName) = 0;
	virtual void DeletePublisher(bool doLog = true) = 0;
	virtual void StartPublisher() = 0;
	virtual void StopPublisher() = 0;
	virtual void Write() = 0;
	virtual void SetDeadlineQos(int64_t timeoutMs) = 0;
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
		DeletePublisher(false);
	}

	CHelper() = delete;
	CHelper(const CHelper& r) = delete;
	CHelper& operator= (const CHelper& l) = delete;
	CHelper(CHelper&&) = delete;
	CHelper& operator =(CHelper&&) = delete;

	virtual void CreatePublisher(ops::Participant* part, std::string topicName) override
	{
		if (pub != nullptr) {
			std::cout << "Publisher already exist for topic " << pub->getTopic().getName() << std::endl;
		} else {
			try {
				//Create topic, might throw ops::NoSuchTopicException
				ops::Topic topic = part->createTopic(topicName.c_str());

				//Create a publisher on that topic
				pub = new DataTypePublisher(topic);

				std::ostringstream myStream;
#ifdef _WIN32
				myStream << " Win(" << _getpid() << ")" << std::ends;
#else
				myStream << " Linux(" << getpid() << ")" << std::ends;
#endif
				pub->setName(std::string("C++Test " + myStream.str()).c_str());
			}
			catch (...) {
				std::cout << "Requested topic '" << topicName << "' does not exist!!" << std::endl;
			}
		}
	}

	virtual void DeletePublisher(bool doLog = true) override
	{
		if (pub != nullptr) {
			std::cout << "Deleting publisher for topic " << pub->getTopic().getName() << std::endl;
			//pub->stop();
			delete pub;
			pub = nullptr;
		} else {
			if (doLog) { std::cout << "Publisher must be created first!!" << std::endl; }
		}
	}

	virtual void StartPublisher() override
	{
		if (pub != nullptr) {
			pub->start();
		} else {
			std::cout << "Publisher must be created first!!" << std::endl;
		}
	}

	virtual void StopPublisher() override
	{
		if (pub != nullptr) {
			pub->stop();
		} else {
			std::cout << "Publisher must be created first!!" << std::endl;
		}
	}

	virtual void Write() override
	{
		if (pub != nullptr) {
			pub->writeOPSObject(&data);
		} else {
			std::cout << "Publisher must be created first!!" << std::endl;
		}
	}

	virtual void SetDeadlineQos(int64_t timeoutMs) override
	{
		if (sub != nullptr) {
			std::cout << "Setting deadlineQos to " << timeoutMs << " [ms] for topic " << sub->getTopic().getName() << std::endl;
			sub->setDeadline(std::chrono::milliseconds(timeoutMs));
		} else {
			std::cout << "Subscriber must be created first!!" << std::endl;
		}
	}

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

typedef CHelper<pizza::PizzaData, pizza::PizzaDataPublisher, pizza::PizzaDataSubscriber> TPizzaHelper;
typedef CHelper<pizza::VessuvioData, pizza::VessuvioDataPublisher, pizza::VessuvioDataSubscriber> TVessuvioHelper;
typedef CHelper<pizza::special::ExtraAllt, pizza::special::ExtraAlltPublisher, pizza::special::ExtraAlltSubscriber> TExtraAlltHelper;

struct ItemInfo {
	std::string Domain;
	std::string TopicName;
	std::string TypeName;

	bool selected = false;
	IHelper* helper = nullptr;
	ops::Participant* part = nullptr;

	ItemInfo(std::string const dom, std::string const top, std::string const typ) :
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

std::vector<ItemInfo*> ItemInfoList;


class MyListener :
		public CHelperListener<pizza::VessuvioData>,
		public CHelperListener<pizza::special::ExtraAllt>
{
public:
	virtual void onData(ops::Subscriber* , pizza::special::ExtraAllt* ) override
	{
	}
	virtual void onData(ops::Subscriber* , pizza::VessuvioData* ) override
	{
	}
	MyListener() = default;
	virtual ~MyListener() = default;
	MyListener(const MyListener& r) = delete;
	MyListener& operator= (const MyListener& l) = delete;
	MyListener(MyListener&&) = delete;
	MyListener& operator =(MyListener&&) = delete;
};

int main(const int argc, const char**argv)
{
	UNUSED(argc)
	UNUSED(argv)
	// --------------------------------------------------------------------
	MyListener myListener;

	ItemInfoList.push_back(new ItemInfo("PizzaDomain", "ExtraAlltTopic"	  , "pizza.special.ExtraAllt"));
	ItemInfoList.push_back(new ItemInfo("PizzaDomain", "TcpExtraAlltTopic", "pizza.special.ExtraAllt"));
	ItemInfoList.push_back(new ItemInfo("PizzaDomain", "UdpExtraAlltTopic", "pizza.special.ExtraAllt"));

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




	//init pizza data to be sent

	//normal
	pizza::special::ExtraAllt extraAlltLarge;
	init::initExtraAlltLarge(extraAlltLarge);
	pizza::special::ExtraAllt extraAlltNormal;
	init::initExtraAlltNormal(extraAlltNormal);

	//TCP
	pizza::special::ExtraAllt extraAlltLargeTCP;
	init::initExtraAlltLargeTCP(extraAlltLargeTCP);
	pizza::special::ExtraAllt extraAlltNormalTCP;
	init::initExtraAlltNormalTCP(extraAlltNormalTCP);

	//UDP
	pizza::special::ExtraAllt extraAlltNormalUDP;
	init::initExtraAlltNormalUDP(extraAlltNormalUDP);
	pizza::special::ExtraAllt extraAlltLargeUDP;
	init::initExtraAlltLargeUDP(extraAlltLargeUDP);

	TExtraAlltHelper* hlpExtra = nullptr;


	// Finish up our ItemInfo's
	for(unsigned int i = 0; i < ItemInfoList.size(); ++i) {
		ItemInfo* const itemInfo = ItemInfoList[i];
		itemInfo->helper = new TExtraAlltHelper(&myListener);

		itemInfo->part = participant;
		itemInfo->selected = true;

		itemInfo->helper->CreatePublisher(itemInfo->part, itemInfo->TopicName);
		itemInfo->helper->StartPublisher();

		if (i == 0) { std::cout << "skickar extra allt" << std::endl; }
		else if (i == 1) { std::cout << "skickar extra allt TCP" << std::endl; }
		else if (i == 2) { std::cout << "skickar extra allt UDP" << std::endl; }

		hlpExtra = (TExtraAlltHelper*)itemInfo->helper;
		if (i == 0) { hlpExtra->data = extraAlltNormal; }
		else if (i == 1) { hlpExtra->data = extraAlltNormalTCP; }
		else if (i == 2) { hlpExtra->data = extraAlltNormalUDP; }

		ops::TimeHelper::sleep(std::chrono::milliseconds(2000));

		itemInfo->helper->Write();
		std::cout << "should write first data" << std::endl;
		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));

		if (i == 0) { hlpExtra->data = extraAlltLarge; }
		else if (i == 1) { hlpExtra->data = extraAlltLargeTCP; }
		else if (i == 2) { hlpExtra->data = extraAlltLargeUDP; }

		itemInfo->helper->Write();

		std::cout << "should write second data" << std::endl;
		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));

		if (i == 0) { hlpExtra->data = extraAlltNormal; }
		else if (i == 1) { hlpExtra->data = extraAlltNormalTCP; }
		else if (i == 2) { hlpExtra->data = extraAlltNormalUDP; }


		std::cout << "should write 3-12 data" << std::endl;
		for(int j = 0 ; j < 10; ++j){
			itemInfo->helper->Write();
			ops::TimeHelper::sleep(std::chrono::milliseconds(10));
		}

		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));

		if (i == 0) { hlpExtra->data = extraAlltLarge; }
		else if (i == 1) { hlpExtra->data = extraAlltLargeTCP; }
		else if (i == 2) { hlpExtra->data = extraAlltLargeUDP; }

		std::cout << "should write 13-22 data" << std::endl;
		for(int k = 0 ; k < 10; ++k){
			itemInfo->helper->Write();
			ops::TimeHelper::sleep(std::chrono::milliseconds(100));
		}
	}

	for (unsigned int idx = 0; idx < ItemInfoList.size(); ++idx) {
		ItemInfo* const ii = ItemInfoList[idx];
		if (ii->helper != nullptr) { delete ii->helper; }
		ii->helper = nullptr;
		ii->part = nullptr;
		delete ItemInfoList[idx];
		ItemInfoList[idx] = nullptr;
	}

	participant->getErrorService()->removeListener(errorWriter);
	otherParticipant->getErrorService()->removeListener(errorWriter2);
	delete errorWriter; errorWriter = nullptr;
	delete errorWriter2; errorWriter2 = nullptr;
	///TODO this should be done by asking Participant to delete instances??
	delete participant;
}
