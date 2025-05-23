// PizzaTest.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <process.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include <ops.h>
#include <Trace.h>
#include <TraceStream.h>

#include "pizza/PizzaData.h"
#include "pizza/PizzaDataSubscriber.h"
#include "pizza/PizzaDataPublisher.h"
#include "pizza/VessuvioData.h"
#include "pizza/VessuvioDataSubscriber.h"
#include "pizza/VessuvioDataPublisher.h"
#include "pizza/special/ExtraAllt.h"
#include "pizza/special/ExtraAlltSubscriber.h"
#include "pizza/special/ExtraAlltPublisher.h"

#include "PizzaProject/PizzaProjectTypeFactory.h"

#include "OPSConfigRepository.h"
#include "OPSUtilities.h"
#include "PubIdChecker.h"
#include "PrintArchiverOut.h"
#include "ChecksumArchiver.h"
#include "NetworkSupport.h"
#include "ValidationTest.h"

///----- Configuration -----
#define USE_LAMDAS
#undef USE_MEMORY_POOLS
#undef USE_MESSAGE_HEADER



#ifdef USE_MESSAGE_HEADER
#include "SdsSystemTime.h"
#endif

#include "../ConfigFileHelper.h"


#ifdef USE_MEMORY_POOLS
#ifdef USE_MESSAGE_HEADER
sds::MessageHeaderData::memory_pool_type sds::MessageHeaderData::_pool(1);
#endif
pizza::PizzaData::memory_pool_type pizza::PizzaData::_pool(20);
pizza::VessuvioData::memory_pool_type pizza::VessuvioData::_pool(20);
pizza::CapricosaData::memory_pool_type pizza::CapricosaData::_pool(10);
pizza::special::Cheese::memory_pool_type pizza::special::Cheese::_pool(10);
pizza::special::LHCData::memory_pool_type pizza::special::LHCData::_pool(10);
pizza::special::ExtraAllt::memory_pool_type pizza::special::ExtraAllt::_pool(20);
#endif

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int64_t getNow()
{
    timespec ts;
    memset(&ts, 0, sizeof(ts));
    clock_gettime(CLOCK_REALTIME, &ts);
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

static bool gDoRcvDelay = false;

template <class DataType>
class CHelperListener
{
public:
	virtual void onData(ops::Subscriber* sub, DataType* data) = 0;
	virtual ~CHelperListener() {};
	CHelperListener() = default;
	CHelperListener(CHelperListener const&) = delete;
	CHelperListener(CHelperListener&&) = delete;
	CHelperListener& operator =(CHelperListener&&) = delete;
	CHelperListener& operator =(CHelperListener const&) = delete;
};

class IHelper
{
public:
	virtual bool HasPublisher() = 0;
	virtual bool HasSubscriber() = 0;
	virtual void CreatePublisher(ops::Participant* part, const ops::ObjectName_T& topicName) = 0;
	virtual void DeletePublisher(bool doLog = true) = 0;
	virtual void StartPublisher() = 0;
	virtual void StopPublisher() = 0;
	virtual bool Write(const std::string message, ops::OPSObject* const other = nullptr) = 0;
	virtual void CreateSubscriber(ops::Participant* part, const ops::ObjectName_T& topicName) = 0;
	virtual void DeleteSubscriber(bool doLog = true) = 0;
	virtual void StartSubscriber() = 0;
	virtual void StopSubscriber() = 0;
	virtual void SetDeadlineQos(int64_t timeoutMs) = 0;
    virtual void Activate() = 0;
    virtual void CheckValid() = 0;
	virtual ~IHelper() {};
	IHelper() = default;
	IHelper(IHelper const&) = delete;
	IHelper(IHelper&&) = delete;
	IHelper& operator =(IHelper&&) = delete;
	IHelper& operator =(IHelper const&) = delete;
};

static int NumVessuvioBytes = 0;
static std::string FillerStr("");
int64_t sendPeriod = 1000;
int PD_version = (int)pizza::PizzaData::PizzaData_idlVersion;
unsigned int numBurst = 1;
ops::ObjectKey_T gKey;
int PD_Kalle = 0;	// Used for validation tests

void WriteUpdate(pizza::PizzaData& data, const std::string& message)
{
    if (PD_version < 0) {
        data.setVersionMask(0);
    } else {
        data.setVersionMask(1);
        data.PizzaData_version = (char)(PD_version & 0xFF);
    }
    data.cheese = "Pizza from C++: " + message;
    data.tomatoSauce = "Tomato";
    data.Kalle = PD_Kalle;
#ifdef USE_MESSAGE_HEADER
    data.systemTime = sds::sdsSystemTime();
#endif
}

void WriteUpdate(pizza::VessuvioData& data, const std::string& message)
{
    data.cheese = "Vessuvio from C++: " + message;
    data.ham = FillerStr;
#ifdef USE_MESSAGE_HEADER
    data.systemTime = sds::sdsSystemTime();
#endif
}

void WriteUpdate(pizza::special::ExtraAllt& data, const std::string& message)
{
    data.cheese = "ExtraAllt from C++: " + message;
    if (data.strings.size() == 0) {
        for (int k = 0; k < 1000; k++) { data.strings.push_back("hej"); }
    }
    data.description = "TLA";
    data.sh = -7;
    if (data.shs.size() == 0) {
        data.shs.push_back(17);
        data.shs.push_back(42);
        data.shs.push_back(-63);
    }
#ifdef USE_MESSAGE_HEADER
    data.systemTime = sds::sdsSystemTime();
#endif
}

template <class DataType, class DataTypePublisher, class DataTypeSubscriber>
class CHelper : 
	public IHelper, 
#ifndef USE_LAMDAS
	ops::DataListener,
	ops::DeadlineMissedListener,
#endif
	ops::Listener<ops::PublicationIdNotification_T>,
	ops::Listener<ops::ConnectStatus>
{
public:
	DataType data;

	CHelper(CHelperListener<DataType>* cli):
		client(cli), pub(nullptr), sub(nullptr)
	{
	}

	virtual ~CHelper()
	{
		DeletePublisher(false);
		DeleteSubscriber(false);
	}

	CHelper() = default;
	CHelper(CHelper const&) = delete;
	CHelper(CHelper&&) = delete;
	CHelper& operator =(CHelper&&) = delete;
	CHelper& operator =(CHelper const&) = delete;

	virtual void CheckValid() override
    {
        WriteUpdate(data, "");	// Ensure data object updated for validation
        bool valid = data.isValid();
        ops::ObjectName_T topName = "";
		if (pub != nullptr) {
            topName = pub->getTopic().getName();
        }
        if (valid) {
            std::cout << "Data for topic '" << topName << "' is valid" << std::endl;
        } else {
            std::cout << "Data for topic '" << topName << "' is NOT valid" << std::endl;
        }
    }

	virtual bool HasPublisher() override { return pub != nullptr; }
	virtual bool HasSubscriber() override { return sub != nullptr; }

	virtual void CreatePublisher(ops::Participant* part, const ops::ObjectName_T& topicName) override
	{
		if (pub != nullptr) {
			std::cout << "Publisher already exist for topic " << pub->getTopic().getName() << std::endl;
		} else {
			try {
				//Create topic, might throw ops::NoSuchTopicException
				ops::Topic topic = part->createTopic(topicName);

				std::cout << "Created topic " << topic.getName() <<
					" [" << topic.getTransport() <<
					"::" << topic.getDomainAddress() <<
					"::" << topic.getPort() <<
					"] " << std::endl;

				std::cout <<
					"  InSocketBufferSize: " << topic.getInSocketBufferSize() << std::endl <<
					"  OutSocketBufferSize: " << topic.getOutSocketBufferSize() << std::endl <<
					"  SampleMaxSize: " << topic.getSampleMaxSize() << std::endl <<
					"  LocalInterface: " << topic.getLocalInterface() << std::endl
					;

				//Create a publisher on that topic
                pub = new DataTypePublisher(topic);
				pub->addListener(this);

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

    virtual bool Write(const std::string message, ops::OPSObject* const other = nullptr) override
    {
        bool res = false;
        if (pub != nullptr) {
            try {
                if (other == nullptr) {
					data.setKey(gKey);
                    WriteUpdate(data, message);
#ifdef TRUE
                    res = pub->writeOPSObject(&data);     // Write using pointer
#else
                    res = pub->write(data);               // Write using ref
#endif
                } else {
                    res = pub->writeOPSObject(other);
                }
                if (!res) {
                    std::cout << "Write(): failed" << std::endl;
                }
            }
            catch (std::exception& e) {
                std::cout << "Write(): Got exception: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Publisher must be created first!!" << std::endl;
        }
        return res;
    }

	virtual void CreateSubscriber(ops::Participant* part, const ops::ObjectName_T& topicName) override 
	{
		if (sub != nullptr) {
			std::cout << "Subscriber already exist for topic " << sub->getTopic().getName() << std::endl;
		} else {
			try {
				//Create topic, might throw ops::NoSuchTopicException
				ops::Topic topic = part->createTopic(topicName);

				std::cout << "Created topic " << topic.getName() <<
					" [" << topic.getTransport() <<
					"::" << topic.getDomainAddress() <<
					"::" << topic.getPort() <<
					"] " << std::endl;

				std::cout <<
					"  InSocketBufferSize: " << topic.getInSocketBufferSize() << std::endl <<
					"  OutSocketBufferSize: " << topic.getOutSocketBufferSize() << std::endl <<
					"  SampleMaxSize: " << topic.getSampleMaxSize() << std::endl <<
					"  LocalInterface: " << topic.getLocalInterface() << std::endl
					;

				//Create a subscriber on that topic.
				sub = new DataTypeSubscriber(topic);

                std::ostringstream myStream;
#ifdef _WIN32
                myStream << " Win(" << _getpid() << ")" << std::ends;
#else
                myStream << " Linux(" << getpid() << ")" << std::ends;
#endif
                sub->setName(std::string("C++Test " + myStream.str()).c_str());

				// Setup listeners
#ifdef USE_LAMDAS
				sub->addDataListener(
					[&](ops::DataNotifier* /*subscriber*/) {
						//std::cout << "Lambda[] for topic '" << sub->getTopic().getName() << "'" << std::endl;
						ops::OPSMessage* newMess = sub->getMessage();
						client->onData(sub, dynamic_cast<DataType*>(newMess->getData()));
                    }
				);
				sub->deadlineMissedEvent.addDeadlineMissedListener(
					[&](ops::DeadlineMissedEvent* /*sender*/) {
						//std::cout << "Deadline Lambda[] for topic '" << sub->getTopic().getName() << "'" << std::endl;
						std::cout << "Deadline Missed for topic " << sub->getTopic().getName() << std::endl;
					}
				);
#else
				sub->addDataListener(this);
				sub->deadlineMissedEvent.addDeadlineMissedListener(this);
#endif
				sub->addListener(this);

				// Add a publication ID Checker
				sub->pubIdChecker = new ops::PublicationIdChecker;
				sub->pubIdChecker->addListener(this);

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

	virtual void SetDeadlineQos(int64_t timeoutMs) override
	{
        if (sub != nullptr) {
            std::cout << "Setting deadlineQos to " << timeoutMs << " [ms] for topic " << sub->getTopic().getName() << std::endl;
			sub->setDeadline(std::chrono::milliseconds(timeoutMs));
			std::cout << "Actual deadlineQos: " << sub->getDeadline().count() << std::endl;
		} else {
			std::cout << "Subscriber must be created first!!" << std::endl;
		}
	}

	virtual void onNewEvent(ops::Notifier<ops::PublicationIdNotification_T>* sender, ops::PublicationIdNotification_T arg) override
	{
		UNUSED(sender);
		ops::Address_T address;
		uint16_t port;
		arg.mess->getSource(address, port);

		std::string newPub = (arg.newPublisher) ? "NEW Publisher" : "SEQ ERROR";

		std::cout << "PubIdChecker(): " << newPub << 
			", Addr: " << address <<
			", Port: " << port <<
			", Expected: " << arg.expectedPubID <<
			", Got: " << arg.mess->getPublicationID() <<
			std::endl;
	}

    virtual void onNewEvent(ops::Notifier<ops::ConnectStatus>* sender, ops::ConnectStatus arg) override
	{
		ops::Publisher* pb = dynamic_cast<ops::Publisher*>(sender);
		if (pb != nullptr) {
			std::cout << "[Publisher on " << pb->getTopic().getName() << "] ";
		}
		ops::Subscriber* sb = dynamic_cast<ops::Subscriber*>(sender);
		if (sb != nullptr) {
			std::cout << "[Subscriber on " << sb->getTopic().getName() << "] ";
		}
		std::cout << "IP: " << arg.addr << "::" << arg.port;
		if (arg.connected) {
			std::cout << " Connected.";
		} else {
			std::cout << " Disconnected.";
		}
		std::cout << " Total: " << arg.totalNo << std::endl;
	}
	
#ifndef USE_LAMDAS
	///Override from ops::DataListener, called whenever new data arrives.
	virtual void onNewData(ops::DataNotifier* subscriber) override
	{
		if(subscriber == sub)
		{
			// Check if we have lost any messages. We use the publicationID and that works as long as
			// it is the same publisher sending us messages.
			ops::OPSMessage* newMess = sub->getMessage();

#ifdef NOT_USED_FOR_NOW
            // Check is done with the OPS built-in PublicationIDChecker()
			if (expectedPubId >= 0) {
				if (expectedPubId != newMess->getPublicationID()) {
					std::cout << ">>>>> Lost message for topic " << sub->getTopic().getName() <<
						". Exp.pubid: " << expectedPubId << " got: " << newMess->getPublicationID() << std::endl;
				}
			}
			expectedPubId = newMess->getPublicationID() + 1;
#endif

			client->onData(sub, dynamic_cast<DataType*>(newMess->getData()));
		}
	}
#endif

#ifndef USE_LAMDAS
	///Override from ops::DeadlineMissedListener, called if no new data has arrived within deadlineQoS.
	virtual void onDeadlineMissed(ops::DeadlineMissedEvent* evt) override
	{
		UNUSED(evt)
		std::cout << "Deadline Missed for topic " << sub->getTopic().getName() << std::endl;
	}
#endif

    virtual void Activate()
    {
        if (pub != nullptr) { pub->Activate(); }
        if (sub != nullptr) { sub->Activate(); }
    }

private:
	CHelperListener<DataType>* client;
	DataTypePublisher* pub;
    ops::Subscriber* sub;
};

typedef CHelper<pizza::PizzaData, pizza::PizzaDataPublisher, pizza::PizzaDataSubscriber> TPizzaHelper;
typedef CHelper<pizza::VessuvioData, pizza::VessuvioDataPublisher, pizza::VessuvioDataSubscriber> TVessuvioHelper;
typedef CHelper<pizza::special::ExtraAllt, pizza::special::ExtraAlltPublisher, pizza::special::ExtraAlltSubscriber> TExtraAlltHelper;

struct ItemInfo {
	ops::ObjectName_T Domain;
	ops::ObjectName_T TopicName;
	ops::TypeId_T TypeName;
	ops::Topic topic;

    bool selected{ false };
    IHelper* helper{ nullptr };
    ops::Participant* part{ nullptr };

	ItemInfo(ops::ObjectName_T const dom, ops::ObjectName_T const top, ops::TypeId_T const typ):
		Domain(dom), TopicName(top), TypeName(typ)
	{
	}
    ItemInfo() = delete;
	ItemInfo(ItemInfo const&) = delete;
	ItemInfo(ItemInfo&&) = delete;
	ItemInfo& operator =(ItemInfo&&) = delete;
	ItemInfo& operator =(ItemInfo const&) = delete;
    ~ItemInfo() {}
};
std::vector<ItemInfo*> ItemInfoList;

static bool beQuite = false;


class MyListener :
	public CHelperListener<pizza::PizzaData>,
	public CHelperListener<pizza::VessuvioData>,
	public CHelperListener<pizza::special::ExtraAllt>
{
public:
	MyListener() = default;
	virtual ~MyListener() = default;
	MyListener(MyListener const&) = delete;
	MyListener(MyListener&&) = delete;
	MyListener& operator =(MyListener&&) = delete;
	MyListener& operator =(MyListener const&) = delete;

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	virtual void onData(ops::Subscriber* const sub, pizza::PizzaData* const data) override
	{
        // test for sending derived objects on same topic
		if (dynamic_cast<pizza::VessuvioData*>(data) != nullptr) {
			onData(sub, dynamic_cast<pizza::VessuvioData*>(data));
			return;
		}

		ops::Address_T addr = "";
		uint16_t port = 0;
		sub->getMessage()->getSource(addr, port);

        if (!beQuite && (data != nullptr)) {
            std::cout <<
                "[Topic: " << sub->getTopic().getName() <<
                "] (From " << addr << ":" << port <<
                ") PizzaData(v";
            if (data->getVersionMask() != 0) {
                std::cout << (int)data->PizzaData_version;
            } else {
                std::cout << "-";
            }
            std::cout << "):: Cheese: " << data->cheese <<
				", Tomato sauce: " << data->tomatoSauce << 
				", spareBytes: " << data->spareBytes.size() << 
				std::endl;
#if defined(DEBUG_OPSOBJECT_COUNTER)
			std::cout << std::endl << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl << std::endl;
#endif
		}

#ifdef NOT_USED_FOR_NOW
		pizza::PizzaData* ttt = data->clone();
		std::cout <<
			"Clone PizzaData:: Cheese: " << ttt->cheese <<
			", Tomato sauce: " << ttt->tomatoSauce << 
			", spareBytes: " << ttt->spareBytes.size() << 
			std::endl;
		delete ttt;
#endif
	}

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	virtual void onData(ops::Subscriber* const sub, pizza::VessuvioData* const data) override
	{
		// test for sending derived objects on same topic
		if (dynamic_cast<pizza::special::ExtraAllt*>(data) != nullptr) {
			onData(sub, dynamic_cast<pizza::special::ExtraAllt*>(data));
			return;
		}

		ops::Address_T addr = "";
		uint16_t port = 0;
		sub->getMessage()->getSource(addr, port);

		if (!beQuite && (data != nullptr)) {
			std::cout <<
				"[Topic: " << sub->getTopic().getName() <<
				"] (From " << addr << ":" << port <<
				") VessuvioData:: Cheese: " << data->cheese <<
				", Tomato sauce: " << data->tomatoSauce <<
				", Ham length: " << data->ham.size() << std::endl;
		}
        if (gDoRcvDelay) {
            ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
        }
    }

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	virtual void onData(ops::Subscriber* const sub, pizza::special::ExtraAllt* const data) override
	{
		ops::Address_T addr = "";
		uint16_t port = 0;
		sub->getMessage()->getSource(addr, port);

		if (!beQuite) {
			std::ostringstream ss;
			if (data->shs.size() > 1) {
				ss << ", shs[1]: " << data->shs.at(1) << std::ends;
			} else {
				ss << "" << std::ends;
			}

			std::cout <<
				"[Topic: " << sub->getTopic().getName() <<
				"] (From " << addr << ":" << port <<
				") ExtraAllt:: Cheese: " << data->cheese <<
				ss.str() <<
				", Tomato sauce: " << data->tomatoSauce <<
				", Num strings: " << data->strings.size() << std::endl;
		}
	}
};

#ifdef OPS_ENABLE_DEBUG_HANDLER
class MyDebugNotifyInterface : public ops::DebugNotifyInterface
{
public:
	MyDebugNotifyInterface() = default;
	~MyDebugNotifyInterface() = default;
	MyDebugNotifyInterface(MyDebugNotifyInterface const&) = delete;
	MyDebugNotifyInterface(MyDebugNotifyInterface&&) = delete;
	MyDebugNotifyInterface& operator =(MyDebugNotifyInterface&&) = delete;
	MyDebugNotifyInterface& operator =(MyDebugNotifyInterface const&) = delete;

private:
	virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp) override
	{
		std::cout << 
			"DebugNotifyInterface::onRequest()"  <<
			", Param1: " << req.Param1 <<
			", Param3.size(): " << req.Param3.size() <<
			std::endl;

		resp.Param3.push_back("Test Debug Response from PizzaTest");
	}
};
#endif

void ActivateAll()
{
    for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
        ItemInfo* const info = ItemInfoList[i];
        info->helper->Activate();
    }
}

void WriteToAllSelected(unsigned int nBurst, ops::OPSObject* const other = nullptr)
{
	static int64_t Counter = 0;

	if (nBurst < 1) { nBurst = 1; }

	for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
		ItemInfo* const info = ItemInfoList[i];
		if (!info->selected) { continue; }
		for (unsigned int b = 0; b < nBurst; b++) {
			std::stringstream str;
			str << Counter;
			const std::string CounterStr(str.str());
			Counter++;
			info->helper->Write(CounterStr, other);
		}
	}
}

void printDomainInfo(const ops::Participant& part)
{
	const ops::Domain* const dom = part.getDomain();
	std::cout << std::endl <<
		"  DomainID: " << dom->getDomainID() << std::endl <<
		"  DomainAddress: " << dom->getDomainAddress() << std::endl <<
		"  InSocketBufferSize: " << dom->getInSocketBufferSize() << std::endl <<
		"  OutSocketBufferSize: " << dom->getOutSocketBufferSize() << std::endl <<
		"  MetaDataMcPort: " << dom->getMetaDataMcPort() << std::endl <<
		"  DebugMcPort: " << dom->getDebugMcPort() << std::endl <<
		"  LocalInterface: " << dom->getLocalInterface() << std::endl <<
		"  TimeToLive: " <<	dom->getTimeToLive() << std::endl;

	ops::Topic top = part.createParticipantInfoTopic();

	std::cout << std::endl <<
		"  TopicName: " << top.getName() << std::endl <<
		"  TypeID: " << top.getTypeID() << std::endl <<
		"  ParticipantID: " << top.getParticipantID() << std::endl <<
		"  DomainID: " << top.getDomainID() << std::endl <<
		"  DomainAddress: " << top.getDomainAddress() << std::endl <<
		"  LocalInterface: " << top.getLocalInterface() << std::endl <<
		"  Transport: " << top.getTransport() << std::endl <<
		"  TimeToLive: " << top.getTimeToLive() << std::endl <<
		"  SampleMaxSize: " << top.getSampleMaxSize() << std::endl <<
		"  Port: " << top.getPort() << std::endl <<
		"  InSocketBufferSize: " << top.getInSocketBufferSize() << std::endl <<
		"  OutSocketBufferSize: " << top.getOutSocketBufferSize() << std::endl;

	std::cout << std::endl << "Dump of configuration" << std::endl;
	{
		ops::PrintArchiverOut prt(std::cout);

		prt.printObject("ops_config", part.getDomain());
	}
	std::cout << std::endl << "Dump of configuration Finished " << std::endl;

    ops::ChecksumArchiver<ops::example::calculator_xor_8> chk8;
    ops::ChecksumArchiver<ops::example::calculator_xor_64> chk64;

    chk8.calc.sum = 0;
    top.serialize(&chk8);
    std::cout << "Checksum byte xor: " << (int)chk8.calc.sum << "\n";

    chk64.calc.sum = 0;
    top.serialize(&chk64);
    std::cout << "Checksum 64-bit xor: " << (uint64_t)chk64.calc.sum << "\n";
}


struct OPS_Trace_Sink : public ops::trace::Sink
{
    bool Enabled = true;

	void Log(const ops::trace::level_t level, const char* grp, const char* msg) noexcept override
	{
        if (Enabled) {
            std::cout << "[mySink:" << std::setw(5) << std::left << level << "] " << grp << ": " << msg;
        }
	}
};

OPS_Trace_Sink mySink;

void menu()
{
	std::cout << "" << std::endl;
	for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
		ItemInfo* const ii = ItemInfoList[i];
		std::string fullName = ii->Domain.c_str(); fullName += "::"; fullName += ii->TopicName.c_str();
		std::string xferInfo = ii->topic.getChannelId().c_str();
		if (xferInfo == "") {
			xferInfo = ii->topic.getTransport().c_str();
			if (xferInfo != "inprocess") {
				xferInfo += "::";
				xferInfo += ii->topic.getDomainAddress().c_str();
				xferInfo += "::";
				xferInfo += ops::NumberToString<int>(ii->topic.getPort()).c_str();
			}
		}

		std::cout << "\t " << std::setw(2) << i <<
			" " <<
		(ii->helper->HasPublisher() ? "P" : " ") <<
		(ii->helper->HasSubscriber() ? "S" : " ") <<
		(ii->selected ? "*" : " ") <<
			" " <<
			std::setw(40) << fullName <<
			"  " << xferInfo <<
			std::endl;
	}

	std::cout << "" << std::endl;
	std::cout << "\t PC      Create Publishers" << std::endl;
	std::cout << "\t PD      Delete Publishers" << std::endl;
	std::cout << "\t PS      Start Publishers" << std::endl;
	std::cout << "\t PT      Stop Publishers" << std::endl;
	std::cout << "\t SC      Create Subscriber" << std::endl;
	std::cout << "\t SD      Delete Subscriber" << std::endl;
	std::cout << "\t SS      Start Subscriber" << std::endl;
	std::cout << "\t ST      Stop Subscriber" << std::endl;
	std::cout << "\t B num   Set number of messages to burst [" << numBurst << "]" << std::endl;
	std::cout << "\t L num   Set num Vessuvio Bytes [" << NumVessuvioBytes << "]" << std::endl;
	std::cout << "\t T ms    Set deadline timeout [ms]" << std::endl;
	std::cout << "\t V ms    Set send period [ms] [" << sendPeriod << "]" << std::endl;
    std::cout << "\t Cx      Validation (x=? isValid()|i set invalid|v set valid|n none|e exception|c[stp] callback" << std::endl;
	std::cout << "\t A       Start/Stop periodical Write with set period" << std::endl;
	std::cout << "\t K       Set key [" << gKey << "]" << std::endl;
	std::cout << "\t M ver   Set Pizzadata version [" << PD_version << "]" << std::endl;
	std::cout << "\t W       Write data" << std::endl;
	std::cout << "\t I name  Lookup host name" << std::endl;
	std::cout << "\t Q       Quite (minimize program output)" << std::endl;
	std::cout << "\t D       Toggle trace" << std::endl;
	std::cout << "\t X       Exit program" << std::endl;

#if defined(DEBUG_OPSOBJECT_COUNTER)
	std::cout << std::endl << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl << std::endl;
#endif
}

int main(const int argc, const char* argv[])
{
	ops::ObjectName_T debugKey = "Pizza";
	ops::execution_policy::Enum policy = ops::execution_policy::threading;

	if (argc > 1) {
		std::string const arg = argv[1];
		if (arg == "polling") {
			policy = ops::execution_policy::polling;
		} else {
			debugKey = arg;
		}
	}

	// Install sink for trace logging from OPS
	InstallSink(&mySink);

	// --------------------------------------------------------------------
	// Test ops trace streams
	ops::trace::tracestream<500> mystream;

	mystream << ops::trace::grp("grp_A") << "Test of trace" << std::endl;
	ops::trace::os_info << "Test of trace\n\n" << std::flush;
	mystream << "Test of trace" << std::endl;
	mystream << ops::trace::grp("grp_C") << "Test of trace" << std::endl;
	ops::trace::os_error << "Test of trace" << std::endl << std::endl;

	std::cout << ops::trace::grp("kalle") << "Hostname: " << ops::GetHostName() << std::endl << std::endl;

#ifdef _WIN32
	// --------------------------------------------------------------------
	// Try to set timer resolution to 1 ms
	UINT const TARGET_RESOLUTION = 1;         // 1-millisecond target resolution

	TIMECAPS tc;
	UINT     wTimerRes = TARGET_RESOLUTION;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR) {
		wTimerRes = min(max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
	}
	timeBeginPeriod(wTimerRes);

  #ifdef USE_MESSAGE_HEADER
    sds::sdsSystemTimeInit();
  #endif
#endif

	// Setup the OPS static error service (common for all participants, reports errors during participant creation)
	ops::ErrorWriter* const errorWriterStatic = new ops::ErrorWriter(std::cout);
	ops::Participant::getStaticErrorService()->addListener(errorWriterStatic);

    // Add all Domain's from given file(s)
    setup_alt_config("Examples/OPSIdls/PizzaProject/ops_config.xml");

	// --------------------------------------------------------------------
	// Setup the InfoItem list
	std::shared_ptr<ops::OPSConfig> cfg = ops::OPSConfigRepository::Instance()->getConfig();
	if (cfg != nullptr) {
		const std::vector<ops::Domain*> doms = cfg->getDomains();
		for (const auto& dom : doms) {
			const ops::ObjectName_T domID = dom->getDomainID();
			const std::vector<ops::Topic*> tops = dom->getTopics();
			for (const auto& top : tops) {
				ItemInfoList.push_back(new ItemInfo(domID, top->getName(), top->getTypeID()));
			}
		}
	}

	// Add an extra entry with a duplicate topic
	ItemInfoList.push_back(new ItemInfo("PizzaDomain", "PizzaTopic", "pizza.PizzaData"));

	// --------------------------------------------------------------------
	MyListener myListener;

	// Create participants
	ops::Participant* const participant = ops::Participant::getInstance("PizzaDomain", "PizzaDomain", policy);
    if (participant == nullptr) {
	    std::cout << "Failed to create Participant. Missing ops_config.xml ??" << std::endl;
		return -1;
    }
	participant->addTypeSupport(new PizzaProject::PizzaProjectTypeFactory());
	printDomainInfo(*participant);
	
#ifdef OPS_ENABLE_DEBUG_HANDLER
	ops::DebugHandler::SetKey(debugKey);
	MyDebugNotifyInterface mdni;
	participant->debugHandler.SetAppCallback(&mdni);
#endif

	ops::Participant* const otherParticipant = ops::Participant::getInstance("OtherPizzaDomain", "OtherPizzaDomain", policy);
	if (otherParticipant == nullptr) {
		std::cout << "Failed to create Participant. Missing ops_config.xml ??" << std::endl;
        return -1;
	}
	otherParticipant->addTypeSupport(new PizzaProject::PizzaProjectTypeFactory());
	printDomainInfo(*otherParticipant);

	// Add error writers to catch internal ops errors
	ops::ErrorWriter* errorWriter = new ops::ErrorWriter(std::cout);
	participant->getErrorService()->addListener(errorWriter);

	ops::ErrorWriter* errorWriter2 = new ops::ErrorWriter(std::cout);
	otherParticipant->getErrorService()->addListener(errorWriter2);

	// Finish up our ItemInfo's
	for (unsigned int idx = 0; idx < ItemInfoList.size(); idx++) {
		ItemInfo* const info = ItemInfoList[idx];

		// Create helper
		if (info->TypeName == pizza::PizzaData::getTypeName()) {
			info->helper = new TPizzaHelper(&myListener);
		}
		if (info->TypeName == pizza::VessuvioData::getTypeName()) {
			info->helper = new TVessuvioHelper(&myListener);
		}
		if (info->TypeName == pizza::special::ExtraAllt::getTypeName()) {
			info->helper = new TExtraAlltHelper(&myListener);
		}

		// Set participant
		if (info->Domain == "PizzaDomain") {
			info->part = participant;
		}
		if (info->Domain == "OtherPizzaDomain") {
			info->part = otherParticipant;
		}

		//Create topic to get topics fully populated, might throw ops::NoSuchTopicException
		info->topic = info->part->createTopic(info->TopicName);
	}

	// --------------------------------------------------------------------
	ItemInfo* ii = ItemInfoList[0];
	ii->selected = true;

	bool doExit = false;
	bool doPeriodicSend = false;

	menu();

	bool doPartPolling = false;

    std::cout << std::endl;
	if (participant->GetExecutionPolicy() == ops::execution_policy::polling) {
		std::cout << "'participant' is using 'Polling' policy" << std::endl;
		doPartPolling = true;
	} else {
		std::cout << "'participant' is using 'threading' policy" << std::endl;
	}
	if (otherParticipant->GetExecutionPolicy() == ops::execution_policy::polling) {
		std::cout << "'otherParticipant' is using 'Polling' policy" << std::endl;
		doPartPolling = true;
	} else {
		std::cout << "'otherParticipant' is using 'threading' policy" << std::endl;
	}

	int64_t nextSendTime = (int64_t)getNow() + sendPeriod;

	while (!doExit) {
		std::cout << std::endl << " (? = menu) > ";

        ActivateAll();

		// Repeated sends
		if (doPeriodicSend || doPartPolling) {
			while (!_kbhit()) {
				int64_t const now = (int64_t)getNow();
				if (doPeriodicSend && (now >= nextSendTime)) {
					// write
					WriteToAllSelected(numBurst);
					// Calc next time to send
					nextSendTime = now + sendPeriod;
				}
				if (participant->GetExecutionPolicy() == ops::execution_policy::polling) { participant->Poll(); }
				if (otherParticipant->GetExecutionPolicy() == ops::execution_policy::polling) { otherParticipant->Poll(); }
                if ((!beQuite) && participant->dataAvailable()) {
                    std::cout << "##### data is available\n";
                }
                ops::TimeHelper::sleep(std::chrono::milliseconds(1));
			}
		}

		enum class TFunction {NONE, PUB, SUB};

		TFunction func = TFunction::NONE;

		char buffer[1024];
		char* const ptr = fgets(buffer, sizeof(buffer), stdin);
		if (ptr == nullptr) { continue; }

		std::string line(buffer);

		// trim start
		std::string::size_type idx = line.find_first_not_of(" \t");
		if (idx == std::string::npos) { continue; }
		if (idx > 0) { line.erase(0, idx); }
		if (line.size() == 0) { continue; }

		char ch = line[0];
		line.erase(0, 1);

		if ((ch == 'p') || (ch == 'P')) { func = TFunction::PUB; }
		if ((ch == 's') || (ch == 'S')) { func = TFunction::SUB; }

		if (func != TFunction::NONE) {
			if (line.size() == 0) {
				std::cout << "ERROR: Expected character after '" << ch << "'" << std::endl;
				continue;
			}
			ch = line[0];
			line.erase(0, 1);
		}

		int num = 0;
		if ((ch >= '0') && (ch <= '9')) {
			num = ch - '0';
			if (line.size() > 0) {
				ch = line[0];
				if ((ch >= '0') && (ch <= '9')) {
					num = (10 * num) + (ch - '0');
					line.erase(0, 1);
				}
			}
			ch = '0';
			if (num >= (int)ItemInfoList.size()) {
				std::cout << "ERROR: Index to large. Max = " << ItemInfoList.size() << std::endl;
				continue;
			}
		}

		// trim start
		idx = line.find_first_not_of(" \t");
		if (idx != std::string::npos) {
            if (idx > 0) { line.erase(0, idx); }
		}
		idx = line.find_first_of("\r\n");
		if (idx != std::string::npos) {
			if (idx > 0) { line.erase(idx); }
		}

        ii = ItemInfoList[num];

		switch (ch) {
			case '?':
				menu();
				break;

			case '0':
				ii->selected = !ii->selected;
				break;

			case 'a':
			case 'A':
				doPeriodicSend = !doPeriodicSend;
				break;

			case 'b':
			case 'B':
			{
				num = atoi(line.c_str());
				if (num >= 1) { numBurst = (unsigned int)num; }
				std::cout << "Num burst: " << numBurst << std::endl;
			}
			break;

			case 'c':
			case 'C':
                if (func == TFunction::NONE) {
                    if (line.size() == 0) { break; }
					if (line[0] != '?') {
                        ValidationTest(line, *participant, PD_Kalle);
                        break;
                    }
                }
                for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
					ItemInfo* const info = ItemInfoList[i];
					if (!info->selected) { continue; }
					if (func == TFunction::PUB) {
						info->helper->CreatePublisher(info->part, info->TopicName);
					} else if (func == TFunction::SUB) {
						info->helper->CreateSubscriber(info->part, info->TopicName);
                    } else {
                        info->helper->CheckValid();	// Command "c?"
					}
				}
				break;

			case 'd':
			case 'D':
				if (func == TFunction::NONE) {
					mySink.Enabled = !mySink.Enabled;
					break;
				}
				for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
					ItemInfo* const info = ItemInfoList[i];
					if (!info->selected) { continue; }
					if (func == TFunction::PUB) {
						info->helper->DeletePublisher();
					} else if (func == TFunction::SUB) {
						info->helper->DeleteSubscriber();
					}
				}
				break;

			case 'e':
				gDoRcvDelay = true;
				break;

			case 'i': {
				std::unique_ptr<ops::IOService> ioServ(ops::IOService::create());
				ops::ShowKnownInterfaces(ioServ.get(), line);
				std::cout << "Selected: " << line << " --> " << ops::GetAddrFromNameEx(line, ioServ.get());
				break;
			}

			case 'l':
			case 'L':
			{
				num = atoi(line.c_str());
				if (num >= 0) { NumVessuvioBytes = num; }
				if (FillerStr.size() > (unsigned int)num) { FillerStr.erase(num); }
				while (FillerStr.size() < (unsigned int)num) { FillerStr += " "; }
				std::cout << "Length: " << FillerStr.size() << std::endl;
			}
			break;

			case 'k':
			case 'K':
				gKey = line;
				break;

			case 'm':
			case 'M':
			{
				num = atoi(line.c_str());
				if (num >= -1) { PD_version = num; }
				std::cout << "PB_version: " << PD_version << std::endl;
			}
			break;

			case 's':
			case 'S':
				for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
					ItemInfo* const info = ItemInfoList[i];
					if (!info->selected) { continue; }
					if (func == TFunction::PUB) {
						info->helper->StartPublisher();
					} else if (func == TFunction::SUB) {
						info->helper->StartSubscriber();
					}
				}
				break;

			case 't':
			case 'T':
				for (unsigned int i = 0; i < ItemInfoList.size(); i++) {
					ItemInfo* const info = ItemInfoList[i];
					if (!info->selected) { continue; }
					if (func == TFunction::PUB) {
						info->helper->StopPublisher();
					} else if (func == TFunction::SUB) {
						info->helper->StopSubscriber();
					} else {
						const int64_t timeout = atoll(line.c_str());
						info->helper->SetDeadlineQos(timeout);
					}
				}
				break;

			case 'v':
			case 'V':
				{
					num = atoi(line.c_str());
                    if (num >= 0) { sendPeriod = num; }
					std::cout << "sendPeriod: " << sendPeriod << std::endl;
				}
				break;

			case 'q':
			case 'Q':
				beQuite = !beQuite;
				break;

			case 'w':
			case 'W':
				WriteToAllSelected(numBurst);
				break;

            case 'z':
            case 'Z':
                {
                pizza::special::ExtraAllt ext;
                WriteToAllSelected(numBurst, &ext);
                }
                break;

            case 'x':
			case 'X':
				doExit = true;
				break;

			case 'y':
			case 'Y':
				break;

			default:;
				break;
		}
	}

	// --------------------------------------------------------------------

	for (unsigned int idx = 0; idx < ItemInfoList.size(); idx++) {
        ii = ItemInfoList[idx];
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

	// We have deleted all publishers and subscribers and we have not connected any timers to
	// the IoService(). We have also unreserved() all messages that we reserved().
	delete participant;
	delete otherParticipant;

#ifdef _WIN32
	// --------------------------------------------------------------------
	// We don't need the set resolution any more
	timeEndPeriod(wTimerRes);
#endif

#ifdef USE_MEMORY_POOLS
	ops::memory_pools::memory_pool_manager::Instance().PrintStat(std::cout);
#endif
    return 0;
}
