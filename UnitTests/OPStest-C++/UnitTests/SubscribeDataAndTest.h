#pragma once

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <typeinfo>
#include <iostream>
#include <limits>

#include "gtest/gtest.h"

#include "ExtraAllt.h"
#include "ExtraAlltSubscriber.h"
#include "ExtraAlltPublisher.h"
#include <ops.h>
#include "OPSArchiverOut.h"
#include "OPSArchiverIn.h"
#include "OPSMessage.h"
#include "PizzaData.h"
#include "PizzaProjectTypeFactory.h"
#include "PizzaDataSubscriber.h"
#include "PizzaDataPublisher.h"
#include "Types.h"
#include "VessuvioData.h"
#include "VessuvioDataSubscriber.h"
#include "VessuvioDataPublisher.h"

#undef USE_MESSAGE_HEADER

namespace test {
    void testExtraAlltNormal(pizza::special::ExtraAllt &extraAllt);
    void testExtraAlltNormal(pizza::special::ExtraAllt &extraAllt, SendType sendType);
    void testExtraAlltLarge(pizza::special::ExtraAllt &extraAllt, SendType sendType);
    void testVessuvio(pizza::VessuvioData &vessuvioData);
}

template <class DataType>
class CHelperListener
{
public:
	virtual void onData(ops::Subscriber* sub, int64_t pubId, DataType* data) = 0;
	virtual ~CHelperListener() = default;
	CHelperListener() = default;
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
	virtual ~IHelper() = default;
	IHelper() = default;
	IHelper(const IHelper&) = delete;
	IHelper(IHelper&&) = delete;
	IHelper& operator=(const IHelper&) = delete;
	IHelper& operator=(IHelper&&) = delete;
};

template <class DataType, class DataTypeSubscriber>
class CHelper : public IHelper, ops::DataListener, ops::DeadlineMissedListener
{
private:
	CHelperListener<DataType>* client{ nullptr };
	ops::Subscriber* sub{ nullptr };
	int64_t expectedPubId{ -1 };

public:
	DataType data;

	CHelper(CHelperListener<DataType>* client_):
		client(client_)
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
			client->onData(sub, newMess->getPublicationID(), (DataType*)newMess->getData());
		}
	}

	///Override from ops::DeadlineMissedListener, called if no new data has arrived within deadlineQoS.
	virtual void onDeadlineMissed(ops::DeadlineMissedEvent* evt) override
	{
		UNUSED(evt)
		std::cout << "Deadline Missed for topic " << sub->getTopic().getName() << std::endl;
	}
};

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
