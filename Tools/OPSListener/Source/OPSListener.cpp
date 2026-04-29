//

#include <time.h>
#include <stdio.h>
#include <limits>
#include <deque>

#ifdef _WIN32
#include <direct.h>
#include <conio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif

#include "ops.h"
#include "OPSUtilities.h"
#include "Lockable.h"
#include "ParticipantInfoData.h"
#include "PubIdChecker.h"
#include "opsidls/DebugRequestResponseData.h"
#include "opsidls/SendAckPatternData.h"
#include "NetworkSupport.h"
#include "Publisher.h"
#include "Subscriber.h"

#include "CArguments.h"
#include "COpsConfigHelper.h"
#include "SdsSystemTime.h"
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include "MessageDump.h"
#include "FilterMessage.h"

const char c_program_version[] = "OPSListener Version 2026-04-23";


// =======================================================================================

// Specialized factory used for receiving ANY topic as an OPSObject
class AllOpsTypeFactory : public ops::SerializableFactory
{
public:
	virtual ops::Serializable* create(CreateType_T type) override
	{
		if (type != "") {
			return new ops::OPSObject();
		}
		return nullptr;
	}

	AllOpsTypeFactory() = default;
	virtual ~AllOpsTypeFactory() = default;
	AllOpsTypeFactory(const AllOpsTypeFactory& other) = delete;
	AllOpsTypeFactory& operator= (const AllOpsTypeFactory& other) = delete;
	AllOpsTypeFactory(AllOpsTypeFactory&& other) = delete;
	AllOpsTypeFactory& operator =(AllOpsTypeFactory&& other) = delete;
};

// =======================================================================================

class MyLogger : public ILogInterface
{
	virtual void Log(const char* const szFormatString, ...) override
	{
		char buff[1000];
		memset(buff, 0, sizeof(buff));
		va_list argList;
		va_start(argList, szFormatString);
#ifdef _WIN32
		vsnprintf_s( buff, sizeof(buff), _TRUNCATE, szFormatString, argList );
#else
		vsnprintf( buff, sizeof(buff), szFormatString, argList );
#endif
		printf("%s", buff);
	}
public:
	MyLogger() = default;
	~MyLogger() = default;
	MyLogger(const MyLogger& other) = delete;
	MyLogger& operator= (const MyLogger& other) = delete;
	MyLogger(MyLogger&& other) = delete;
	MyLogger& operator =(MyLogger&& other) = delete;
};

// =======================================================================================

typedef std::string (*TFormatFunc)(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData);

std::string publisherName(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(opsData);
	if (mess == nullptr) { return ""; }
	return "Pub: " + std::string(mess->getPublisherName().c_str());
}
std::string publicationId(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(opsData);
	if (mess == nullptr) { return ""; }
	std::stringstream str;
	str << mess->getPublicationID();
	const std::string IdStr(str.str());
	return "PubId: " + IdStr;
}
std::string topicName(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(opsData);
	if (mess == nullptr) { return ""; }
	return "Topic: " + std::string(mess->getTopicName().c_str());
}
std::string source(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(opsData);
	if (mess == nullptr) { return ""; }
	ops::Address_T srcIP;
	uint16_t srcPort;
	mess->getSource(srcIP, srcPort);
	std::ostringstream myPort;
	myPort << srcPort << std::ends;
	return "Source: " + std::string(srcIP.c_str()) + "::" + myPort.str();
}
std::string messVersion(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
    UNUSED(opsData);
    if (mess == nullptr) { return ""; }
    std::string myVer = "0";
    if (mess->getVersionMask() != 0) { myVer = "1"; }
    return "messVer: " + myVer;
}

// ---------------------------------------------------------------------------------------

std::string spareBytes(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(mess);
	if (opsData == nullptr) { return ""; }
	std::stringstream str;
	str << opsData->spareBytes.size();
	std::string const SizeStr(str.str());
	return "Spare: " + SizeStr;
}
std::string key(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(mess);
	if (opsData == nullptr) { return ""; }
	return "Key: " + std::string(opsData->getKey().c_str());
}
std::string typeString(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
	UNUSED(mess);
	if (opsData == nullptr) { return ""; }
	return "Type: " + std::string(opsData->getTypeString().c_str());
}
std::string objectVersion(const ops::OPSMessage* const mess, const ops::OPSObject* const opsData)
{
    UNUSED(mess);
    if (opsData == nullptr) { return ""; }
    std::string myVer = "0";
    if (opsData->getVersionMask() != 0) { myVer = "1"; }
    return "objVer: " + myVer;
}

// =======================================================================================
// =======================================================================================

// ---------------------------------------------------------------------------------------
//Create a class to act as a listener for OPS data and deadlines
class Main : ops::DataListener, ops::Listener<ops::PublicationIdNotification_T>, ops::Listener<ops::ConnectStatus>
{
private:
	CArguments& args;
	MyLogger logger;
	COpsConfigHelper opsHelper;

	std::vector<ops::Participant*> vParts;
	std::vector<ops::Subscriber*> vSubs;
	std::map<ops::Subscriber*, ops::PublicationIdChecker*> pubIdMap;

	typedef struct _entry {
        int64_t time{ 0 };
        ops::Subscriber* sub{ nullptr };
        ops::OPSMessage* mess{ nullptr };
	} TEntry;
	std::deque<TEntry> List;
	ops::Lockable ListLock;

	std::map<char, TFormatFunc> formatMap;

	std::vector<ops::InternalString_T> ownPartInfoNames;

	struct PartMapEntry
	{
		int64_t time{ 0 };
		ops::ParticipantInfoData pidata;
	};
	std::map<ops::InternalString_T, PartMapEntry> partMap;

	// local storage of current worked on entry time
	int64_t _messageTime = 0;

	// Since our factory creates OPSObjects, the spareBytes starts after the OPSObject fields, 
	// so we need to skip the OPSObject fields on top level.
	message_dump::SkipFunc_T skipFunc = [=](const std::string& name) -> bool { return name == "ops.OPSObject"; };
	message_dump::MessageDump MsgDump;

	// Helper for delayed CRLF
	bool needCRLF = false;

	void CheckDelayedCRLF()
	{
		if (needCRLF) {
			std::cout << "\n";
		}
		needCRLF = false;
	}

public:
	int messDataCounter = 0;

	int numQueued() const {return (int)List.size();}

	void LogTopic(ops::ObjectName_T const domainName, const ops::Topic& top)
	{
		// For our purpose we can use any participant
		ops::Participant* const part = opsHelper.getDomainParticipant(opsHelper.vDomains[0]);

		std::cout <<
			"Subscribing to Topic: " << ops::utilities::fullTopicName(domainName, top.getName()) <<
			" [ " << top.getTransport() <<
			"::" << top.getDomainAddress() <<
			"::" << top.getPort() <<
			" (" << top.getLocalInterface() <<
			" --> " << ops::doSubnetTranslation(top.getLocalInterface(), part->getIOService()) <<
			") ] " <<
			std::endl;
	}

	void updateVector(std::vector<ops::ObjectName_T>& v, const ops::ObjectName_T& topName) const
	{
		for (unsigned int j = 0; j < v.size(); j++) {
			if (v[j] == topName) {
				return;
			}
		}
		v.push_back(topName);
	}

	//
	Main(CArguments& args_) :
		args(args_),
		opsHelper(&logger, &logger, args.defaultDomain),
		MsgDump(skipFunc)
	{
		using namespace ops;

		// Initialize the format map
		formatMap['T'] = topicName;
		formatMap['k'] = key;
		formatMap['n'] = publisherName;
		formatMap['i'] = publicationId;
		formatMap['s'] = spareBytes;
		formatMap['y'] = typeString;
		formatMap['S'] = source;
        formatMap['v'] = messVersion;
        formatMap['V'] = objectVersion;

		ErrorWriter* const errorWriter = new ErrorWriter(std::cout);

		MsgDump.SetRowLimit(args.rowlimit);
        for (unsigned int i = 0; i < args.jsonFiles.size(); i++) { MsgDump.AddDefinitions(args.jsonFiles[i]); }

		// First, Add all domains in given configuration files to the helper
		for (unsigned int i = 0; i < args.cfgFiles.size(); i++) { opsHelper.DomainMapAdd(args.cfgFiles[i]); }

		std::cout << std::endl;

		// Check for regex in topicNames 
		std::vector<ops::ObjectName_T> vec;
		for (unsigned int i = 0; i < args.topicNames.size(); i++) {
			if (opsHelper.checkExpansion(vec, args.topicNames[i])) {
				args.topicNames[i] = "";
			}
		}
		for (const auto& x : vec) {
			updateVector(args.topicNames, x);
		}
		vec.clear();

		// Check for regex in skipTopicNames 
		for (unsigned int i = 0; i < args.skipTopicNames.size(); i++) {
			if (opsHelper.checkExpansion(vec, args.skipTopicNames[i])) {
				args.skipTopicNames[i] = "";
			}
		}
		for (const auto& x : vec) {
			updateVector(args.skipTopicNames, x);
		}
		vec.clear();

		// Now, Check given topics and find all unique domains for these
		for (unsigned int i = 0; i < args.topicNames.size(); i++) {
            if (args.topicNames[i] == "") { continue; }
			if (!opsHelper.existsTopic(args.topicNames[i])) {
				std::cout << "##### Topic '" << args.topicNames[i] << "' not found in configuration file(s)" << std::endl;
				args.topicNames[i] = "";
				continue;
			}
			opsHelper.checkTopicDomain(args.topicNames[i]);
		}

		// Handle case where we are requested to subscribe to all topics from all existing domains
		if (args.allTopics) {
			// Replace args.subscribeDomains with all known domains in configuration files
			opsHelper.getAvailableDomains(args.subscribeDomains);
		}

		// Check given subscribe domains and make sure they exist and ev. add them to the unique list
		for (unsigned int i = 0; i < args.subscribeDomains.size(); i++) {
			ops::ObjectName_T domainName = args.subscribeDomains[i];
			if (opsHelper.existsDomain(domainName)) {
				domainName += "::";		// use domain syntax for routine below
				opsHelper.checkTopicDomain(domainName);
			} else {
				std::cout << "##### Domain '" << domainName << "' not found. Have you forgot configuration file(s) ?" << std::endl;
				args.subscribeDomains[i] = "";
			}
		}

		// Handle case where we are requested to listen to debug Request/Response data from all existing domains
		if (args.allDebugDomains) {
			// Replace args.debugDomains with all known domains in configuration files
			opsHelper.getAvailableDomains(args.debugDomains);
		}

		// Check given debug domains and make sure they exist and ev. add them to the unique list
		for (unsigned int i = 0; i < args.debugDomains.size(); i++) {
			ops::ObjectName_T domainName = args.debugDomains[i];
			if (opsHelper.existsDomain(domainName)) {
				domainName += "::";		// use domain syntax for routine below
				opsHelper.checkTopicDomain(domainName);
			} else {
				std::cout << "##### Domain '" << domainName << "' not found. Have you forgot configuration file(s) ?" << std::endl;
				args.debugDomains[i] = "";
			}
		}

		// Handle case where we are requested to listen to partition info from all existing domains
		if (args.allInfoDomains) {
			// Replace args.infoDomains with all known domains in configuration files
			opsHelper.getAvailableDomains(args.infoDomains);
		}

		// Check given info domains and make sure they exist and ev. add them to the unique list
		for (unsigned int i = 0; i < args.infoDomains.size(); i++) {
			ops::ObjectName_T domainName = args.infoDomains[i];
			if (opsHelper.existsDomain(domainName)) {
				domainName += "::";		// use domain syntax for routine below
				opsHelper.checkTopicDomain(domainName);
			} else {
				std::cout << "##### Domain '" << domainName << "' not found. Have you forgot configuration file(s) ?" << std::endl;
				args.infoDomains[i] = "";
			}
		}

		// Create a participant for each unique domain
		for (unsigned int i = 0; i < opsHelper.vDomains.size(); i++) {
			try {
				ops::Participant* const part = opsHelper.getDomainParticipant(opsHelper.vDomains[i]);
				if (part == nullptr) {
					std::cout << "##### Domain '" << opsHelper.vDomains[i] << "' not found. Have you forgot configuration file(s) ?" << std::endl;
					continue;
				}
				vParts.push_back(part);
				part->addTypeSupport(new AllOpsTypeFactory());
				part->addListener(errorWriter);
				ownPartInfoNames.push_back(part->getPartInfoName());
			}
			catch(...)
			{
				std::cout << "##### Domain '" << opsHelper.vDomains[i] << "' not found. Have you forgot configuration file(s) ?" << std::endl;
			}
		}

		// Now we can get the domain object for the 'subscribe domains' and add their topics to the topic list
		for (unsigned int i = 0; i < args.subscribeDomains.size(); i++) {
			ops::ObjectName_T domainName = args.subscribeDomains[i];
			if (domainName == "") { continue; }

			ops::Participant* const part = opsHelper.getDomainParticipant(domainName);
			if (part == nullptr) { continue; }
			ops::Domain* const dom = part->getDomain();
			if (dom == nullptr) { continue; }

			std::vector<ops::Topic*> topics = dom->getTopics();

			for (unsigned int t = 0; t < topics.size(); t++) {
				ops::ObjectName_T const topName = ops::utilities::fullTopicName(domainName, topics[t]->getName());
				updateVector(args.topicNames, topName);
			}
		}

		std::cout << std::endl;

		// Create subscribers for all existing topics
		for (unsigned int i = 0; i < args.topicNames.size(); i++) {
			ops::ObjectName_T const topName = args.topicNames[i];
			if (topName == "") { continue; }

			// if skip topic, continue
			bool found = false;
			for (unsigned int j = 0; j < args.skipTopicNames.size(); j++) {
				if (args.skipTopicNames[j] == topName) {
					found = true;
					break;
				}
			}
			if (found) { continue; }

			ops::Participant* const part = opsHelper.getDomainParticipant(ops::utilities::domainName(topName));
			if (part == nullptr) { continue; }

			Topic topic = part->createTopic(ops::utilities::topicName(topName));

			// If channels are specified, ensure that topic belongs to a specified channel
			if (args.channels.size() > 0) {
				ops::ChannelId_T chan = topic.getChannelId();
				for (const auto& x : args.channels) {
					if (chan == x) {
						found = true;
						break;
					}
				}
				if (!found) { continue; }
			}

			// Need to skip Topics using UDP with static route, to not interfere with the real subscriber
			if (topic.getTransport() == ops::Topic::TRANSPORT_UDP) {
				if (isMyNodeAddress(topic.getDomainAddress(), part->getIOService())) {
					if (args.dontSkipUdpStaticRoute) {
						std::cout << "##### Subscribing to '" << topName << "' may interfere with real subscriber (UDP static route)" << std::endl;
					} else {
						std::cout << "##### Skipping topic '" << topName << "' to not interfere with real subscriber (UDP static route)" << std::endl;
						continue;
					}
				}
			}

			LogTopic(ops::utilities::domainName(topName), topic);

            // We don't want to be part of the ACK handling, we just listen to topics
            bool useAck = topic.getUseAck();
            topic.setUseAck(false);

			ops::Subscriber* const sub = new ops::Subscriber(topic);
			sub->addDataListener(this);
			sub->addListener(this);
			sub->start();

			vSubs.push_back(sub);
			if (args.doPubIdCheck) {
				// We don't use the subscriber to do the checking due to performance, instead we do it "off-line"
				pubIdMap[sub] = new ops::PublicationIdChecker();
				pubIdMap[sub]->addListener(this);
			}

            if (useAck) {
                // Add subscriber for the ACK's to this Topic
                Topic ackTopic(Topic::CreateAckTopic(topic));
                LogTopic(ops::utilities::domainName(topName), ackTopic);
                ops::Subscriber* const subAck = new ops::Subscriber(ackTopic);
                subAck->addDataListener(this);
                subAck->start();
                vSubs.push_back(subAck);
            }
		}

#ifdef OPS_ENABLE_DEBUG_HANDLER
		// Create subscribers to all DebugRequestResponseData
		for (unsigned int i = 0; i < args.debugDomains.size(); i++) {
			try {
				ops::ObjectName_T const domainName = args.debugDomains[i];
				if (domainName == "") { continue; }

				ops::Participant* const part = opsHelper.getDomainParticipant(domainName);
				if (part == nullptr) { continue; }

				Topic top = part->createDebugTopic();
				if (top.getPort() == 0) { continue; }

				LogTopic(domainName, top);

				ops::Subscriber* const sub = new ops::Subscriber(top);
				sub->addDataListener(this);
				sub->start();

				vSubs.push_back(sub);
				if (args.doPubIdCheck) {
					// We don't use the subscriber to do the checking due to performance, instead we do it "off-line"
					pubIdMap[sub] = new ops::PublicationIdChecker();
					pubIdMap[sub]->addListener(this);
				}
			} catch (...)
			{
			}
		}
#endif

		// Create subscribers to all ParticipantInfoData
		for (unsigned int i = 0; i < args.infoDomains.size(); i++) {
			try {
				ops::ObjectName_T const domainName = args.infoDomains[i];
				if (domainName == "") { continue; }

				ops::Participant* const part = opsHelper.getDomainParticipant(domainName);
				if (part == nullptr) { continue; }

				Topic top = part->createParticipantInfoTopic();
				if (top.getPort() == 0) { continue; }

				LogTopic(domainName, top);

				ops::Subscriber* const sub = new ops::Subscriber(top);
				sub->addDataListener(this);
				sub->start();

				vSubs.push_back(sub);
				if (args.doPubIdCheck) {
					// We don't use the subscriber to do the checking due to performance, instead we do it "off-line"
					pubIdMap[sub] = new ops::PublicationIdChecker();
					pubIdMap[sub]->addListener(this);
				}
			}
			catch(...)
			{
			}
		}

		std::cout << std::endl;

		if (vSubs.size() == 0) {
			std::cout << "##### No subscriptions !!!!. Check topics !!!" << std::endl;
			exit(-1);
		}
	}
	//
	virtual ~Main()
	{
		// Delete PubIdCheckers
		for (auto it = pubIdMap.begin(); it != pubIdMap.end(); ++it) {
			delete it->second;
		}
		pubIdMap.clear();
		// Delete subscribers
		for (unsigned int i = 0; i < vSubs.size(); i++) {
			vSubs[i]->stop();
			delete vSubs[i];
		}
		vSubs.clear();
		// Delete Participants
		for (unsigned int i = 0; i < vParts.size(); i++) {
			delete vParts[i];
		}
		vParts.clear();
	}
	Main(const Main& other) = delete;
	Main& operator= (const Main& other) = delete;
	Main(Main&& other) = delete;
	Main& operator =(Main&& other) = delete;
	//
	///Override from ops::DataListener, called whenever new data arrives.
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	virtual void onNewData(ops::DataNotifier* const subscriber) override
	{
		ops::Subscriber* const sub = dynamic_cast<ops::Subscriber*>(subscriber);
		if (sub != nullptr) {
			ops::OPSMessage* const mess = sub->getMessage();
			if (mess == nullptr) { return; }

			// Reserve message and queue, so we don't delay the subscriber thread
			mess->reserve();

			TEntry ent;
			ent.time = sds::sdsSystemTime();
			ent.mess = mess;
			ent.sub = dynamic_cast<ops::Subscriber*>(subscriber);		// Save subscriber so we can lookup the pubIdChecker

			ListLock.lock();

			List.push_back(ent);
			messDataCounter++;

			ListLock.unlock();
		}
	}
	//
	virtual void onNewEvent(ops::Notifier<ops::PublicationIdNotification_T>* const sender, ops::PublicationIdNotification_T arg) override
	{
		UNUSED(sender);
		ops::Address_T address;
		uint16_t port;
		arg.mess->getSource(address, port);

		std::string const newPub = (arg.newPublisher) ? "NEW Publisher" : "SEQ ERROR";

		std::string str = "";
		if (args.logTime) {
			str += "[" + sds::sdsSystemTimeToLocalTime(_messageTime) + "] ";
		}
		CheckDelayedCRLF();
		std::cout << str <<
			"PubIdChecker(): " << newPub <<
			" on Topic: " << arg.mess->getTopicName() <<
			", Addr: " << address <<
			", Port: " << port <<
			", Expected: " << arg.expectedPubID <<
			", Got: " << arg.mess->getPublicationID() <<
			std::endl;
	}
	virtual void onNewEvent(ops::Notifier<ops::ConnectStatus>* const sender, ops::ConnectStatus const arg) override
	{
		ops::Subscriber* const sb = dynamic_cast<ops::Subscriber*>(sender);
		CheckDelayedCRLF();
		if (sb != nullptr) {
			std::cout << "[Publisher on " << sb->getTopic().getName() << "] ";
		}
		std::cout << "IP: " << arg.addr << "::" << arg.port;
		if (arg.connected) {
			std::cout << " Connected.";
		} else {
			std::cout << " Disconnected.";
		}
		std::cout << " Total: " << arg.totalNo << "\n";
	}
	//
	void ShowDebugEntity(const opsidls::DebugRequestResponseData* const data)
	{
		CheckDelayedCRLF();
		switch (data->Command) {
		case 0:
			std::cout << "  Response, ";
			switch (data->Result1) {
			case 1:	std::cout << "Filter key in Param3: "; break;
			case 2:	std::cout << "Publishers in Param3: "; break;
			case 3: std::cout << "Suscribers in Param3: "; break;
			case 50:std::cout << "Generic command"; break;
			default:; break;
			}
			std::cout << std::endl;
			break;
		case 1:
			std::cout << "  Request status\n";
			break;
		case 2:
			switch (data->Param1) {
			case 1: std::cout << "  List Instance Key\n"; break;
			case 2: std::cout << "  List Publishers\n"; break;
			case 3: std::cout << "  List Subscribers\n"; break;
			default:; break;
			}
			break;
		case 50:
			std::cout << "  Generic command\n";
			break;
		default:;
			break;
		}
	}
	void ShowPubEntity(const opsidls::DebugRequestResponseData* const data)
	{
		CheckDelayedCRLF();
		switch (data->Command) {
		case 0:
			std::cout << "  '" << data->Name << "' Publisher Response, ";
			std::cout << "Enabled: " << (data->Enabled ? "True" : "False");
			std::cout << ", Pub Id: " << data->Result1;
			std::cout << ", #sends to skip: " << data->Result2;
			std::cout << ", msg to send: " << (data->Result3 ? "True" : "False");
			std::cout << std::endl << std::endl;
			break;
		case 1:
			std::cout << "  Request '" << data->Name << "' Publisher status\n";
			break;
		case 2:
			switch (data->Param1) {
			case 0: std::cout << "  Disable '" << data->Name << "' Publisher\n"; break;
			case 1: std::cout << "  Enable '" << data->Name << "' Publisher\n"; break;
			default:; break;
			}
			break;
		case 3:
			std::cout << "  Increment '" << data->Name << "' Pub Id with " << data->Param1 << std::endl;
			break;
		case 4:
			std::cout << "  Skip " << data->Param1 << " sends\n";
			break;
		case 5:
			std::cout << "  Send message in Objs[0] directly\n";
			break;
		case 6:
			std::cout << "  Send message(s) in Objs instead of ordinary sends\n";
			break;
		default:;
			break;
		}
	}
	void ShowSubEntity(const opsidls::DebugRequestResponseData* const data)
	{
		CheckDelayedCRLF();
		switch (data->Command) {
		case 0:
			std::cout << "  '" << data->Name << "' Subscriber Response, ";
			std::cout << "Enabled: " << (data->Enabled ? "True" : "False");
			std::cout << ", #msg rcvd: " << data->Result1;
			std::cout << ", #rcvs to skip: " << data->Result2;
			std::cout << ", key filter active: " << (data->Result3 ? "True" : "False");
			std::cout << std::endl << std::endl;
			break;
		case 1:
			std::cout << "  Request '" << data->Name << "' Subscriber status\n";
			break;
		case 2:
			switch (data->Param1) {
			case 0: std::cout << "  Disable '" << data->Name << "' Subscriber\n"; break;
			case 1: std::cout << "  Enable '" << data->Name << "' Subscriber\n"; break;
			default:; break;
			}
			break;
		case 4:
			std::cout << "  Skip " << data->Param1 << " receives\n";
			break;
		default:;
			break;
		}
	}
    void ShowAckMessage(const TEntry& ent, const ops::OPSMessage* const mess, const opsidls::SendAckPatternData* const data)
    {
		CheckDelayedCRLF();
		if (args.logTime) {
            std::cout << "[" << sds::sdsSystemTimeToLocalTime(ent.time) << "] ";
        }
        ops::Address_T addr;
        uint16_t port;
        mess->getSource(addr, port);

        switch (data->messageType) {
        case opsidls::SendAckPatternData::MType::ACK :
            /* Valid for ACK type, source IP/Port is the sender of the data we acknowledge */
            //int   sourceIP;
            //short sourcePort;
            //long  publicationID;
            std::cout << "ACK from '" << mess->getPublisherName() << "', " << addr << "::" << port << 
                " on topic '" << mess->getTopicName() << ", PubId: " << data->publicationID << "'\n";
            break;
        case opsidls::SendAckPatternData::MType::REGISTER :
            std::cout << "REGISTER from '" << mess->getPublisherName() << "', " << addr << "::" << port << 
                " on topic '" << mess->getTopicName() << "'\n";
            break;
        case opsidls::SendAckPatternData::MType::UNREGISTER :
            std::cout << "UNREGISTER from '" << mess->getPublisherName() << "', " << addr << "::" << port << 
                " on topic '" << mess->getTopicName() << "'\n";
            break;
        }
    }
    void ShowDebugMessage(const TEntry& ent, const ops::OPSMessage* const mess, const opsidls::DebugRequestResponseData* const data)
	{
		// Show Debug Request/Response data
		std::string str = "";
		CheckDelayedCRLF();
		if (args.logTime) {
			str += "[" + sds::sdsSystemTimeToLocalTime(ent.time) + "] ";
		}
		std::cout << str <<
			"Key: " << data->getKey() <<
			", Entity: " << data->Entity <<
			", Name: " << data->Name <<
			", Command: " << data->Command <<
			", Param1: " << data->Param1 <<
			", Enabled: " << data->Enabled <<
			", Result1: " << data->Result1 <<
			", Result2: " << data->Result2 <<
			", Result3: " << data->Result3 <<
			", " << source(mess, data) <<
			std::endl;

		switch (data->Entity) {
		case 0: ShowDebugEntity(data); break;
		case 2: ShowPubEntity(data); break;
		case 3: ShowSubEntity(data); break;
		default:; break;
		}

		for (unsigned int i = 0; i < data->Param3.size(); ++i) {
			std::cout << "    Param3(" << i << "): " << data->Param3[i] << '\n';
		}

		if (data->Param3.size() > 0) { std::cout << std::endl; }

		//Objs
	}
	void ShowTopicInfoData(const ops::TopicInfoData& tid)
	{
		if (!args.detailedTopicInfo) {
			// Show all topic names on one line
			std::cout << tid.name << " ";
		}
		else {
			// Show each topic on a separate line 
			std::cout <<
				"    " << std::setw(30) << tid.name <<
				", " << tid.transport <<
				"::" << tid.address <<
				"::" << tid.port <<
				", " << tid.type <<
				std::endl;
		}
	}
	void ShowParticipantInfo(const ops::OPSMessage* const mess, const ops::ParticipantInfoData* const piData)
	{
		// Skip if it's from any of our own participants (we won't show them)
		for (unsigned int i = 0; i < ownPartInfoNames.size(); i++) {
			if (ownPartInfoNames[i] == piData->name) { return; }
		}

		// Not from us. Look up entry from earlier, if any
		auto it = partMap.find(piData->name);
		if (it == partMap.end()) {
			CheckDelayedCRLF();
			std::cout << "[" << partMap.size() + 1 << "] >>>>> Participant '" << piData->name << "' arrived" << std::endl;
			auto x = partMap.insert({ piData->name, PartMapEntry() });
			it = x.first;
		}
		// Get reference to entry
		PartMapEntry& pme = it->second;
		pme.time = sds::sdsSystemTime();

		// Present info
		if (!args.onlyArrivingLeaving) {
			// Check for diff
			if ((piData->getKey().size() == 0) || (piData->getKey() != pme.pidata.getKey())) {
				CheckDelayedCRLF();
				std::cout <<
					"[" << partMap.size() << "] " <<
					"name: " << piData->name <<
					", domain: " << piData->domain <<
					", partId: " << piData->id <<
					", ip: " << piData->ip <<
					", mcudp: " << piData->mc_udp_port <<
					", mctcp: " << piData->mc_tcp_port <<
					std::endl;
					ops::Address_T srcIP;
					uint16_t srcPort;
				mess->getSource(srcIP, srcPort);
				std::cout <<
					"  lang: " << piData->languageImplementation <<
					", opsver: " << piData->opsVersion <<
					", From: " << srcIP << ":" << srcPort <<
					", pubId: " << mess->getPublicationID() <<
					", key: " << piData->getKey() <<
					std::endl;
				//std::vector<TopicInfoData> subscribeTopics;
				std::cout << "  subscr Topics: ";
				if (args.detailedTopicInfo) { std::cout << std::endl; }
				for (const auto& tid : piData->subscribeTopics) {
					ShowTopicInfoData(tid);
				}
				if (!args.detailedTopicInfo) { std::cout << std::endl; }
				//std::vector<TopicInfoData> publishTopics;
				std::cout << "  pub Topics: ";
				if (args.detailedTopicInfo) { std::cout << std::endl; }
				for (const auto& tid : piData->publishTopics) {
					ShowTopicInfoData(tid);
				}
				if (!args.detailedTopicInfo) { std::cout << std::endl; }
				////std::vector<std::string> knownTypes;
				//std::cout << "  knownTypes: ";
				//for (const auto& x : piData->knownTypes) {
				//	std::cout << x << " ";
				//}
				//std::cout << std::endl;
			}
			else {
				std::cout << "." << std::flush;
				needCRLF = true;
			}
		}

		// Save received data as previuos
		pme.pidata = *piData;
	}
	//
    static char toAscii(uint8_t const val)
    {
        if (val < 0x20) { return '.'; }
        if (val > 0x7F) { return '.'; }
        return (char)val;
    }
    void dumpHex(const char* const ptr, size_t numbytes)
    {
		CheckDelayedCRLF();
		int offset = 0;
        if (numbytes > args.maxDumpBytes) { numbytes = args.maxDumpBytes; }
        while (numbytes > 0) {
            const uint8_t* Ptr__ = (const uint8_t*)&ptr[offset];
            int len = (int)numbytes;
            if (len > 16) { len = 16; }

            std::cout << std::hex << "    " << std::setw(8) << offset << ": ";
            for (int i = 0; i < len; ++i) { std::cout << std::setw(2) << (int)*Ptr__++ << " "; }
            for (int i = len; i < 16; ++i) { std::cout << "   "; }
            std::cout << "    ";
            Ptr__ = (const uint8_t*)&ptr[offset];
            for (int i = 0; i < len; ++i) { std::cout << toAscii(*Ptr__++); }
            std::cout << std::dec << "\n";

            numbytes -= len;
            offset += len;
        }
    }
    //
	void WorkOnList(int const numMess)
	{
		/// Don't loop to much, to not loose mmi responsiveness
		for (int loopCnt=0; loopCnt < numMess; loopCnt++) {
			TEntry ent;
			ops::OPSMessage* mess = nullptr;
			ops::OPSObject* opsData = nullptr;
			ops::ParticipantInfoData* piData = nullptr;
			opsidls::DebugRequestResponseData* debugData = nullptr;
            opsidls::SendAckPatternData* ackData = nullptr;

			ListLock.lock();
			if (List.size() > 0) {
				ent = List.front();
				mess = ent.mess;
				List.pop_front();
			}
			ListLock.unlock();

			if (mess == nullptr) { return; }

			opsData = mess->getData();
			piData = dynamic_cast<ops::ParticipantInfoData*>(opsData);
			debugData = dynamic_cast<opsidls::DebugRequestResponseData*>(opsData);
			ackData = dynamic_cast<opsidls::SendAckPatternData*>(opsData);

			if (args.filter != nullptr) {
				if (!args.filter->accept(mess, opsData)) { continue; }
			}

			// Calculate statistics
			if (args.statistics) {
				static uint32_t countPerSlot[100]{ 0 };
				int64_t timeMs = sds::sdsSystemTimeUnitsToMs(ent.time);
				int ms = timeMs % 1000;
				static int64_t prevTimestamp = -1;
				int64_t timestamp = timeMs - ms;		// Whole seconds
				ms = ms / 10;							// 10ms slots
				if (prevTimestamp != timestamp) {
					if (prevTimestamp >= 0) {
						int64_t time = sds::msToSdsSystemTimeUnits(prevTimestamp);
						CheckDelayedCRLF();
						if (args.logTime) {
							std::cout << "[" + sds::sdsSystemTimeToLocalTime(time) + "] ";
						}
						std::cout << "{stat} ";
						uint32_t sum = 0;
						for (int slot = 0; slot < 100; ++slot) {
							if ((slot % 20) == 0) { std::cout << " "; }
							sum += countPerSlot[slot];
							std::cout << std::setw(2) << countPerSlot[slot] << " ";
						}
						std::cout << " sum= " << sum << "\n";
						memset(&countPerSlot[0], 0, sizeof(countPerSlot));
					}
					prevTimestamp = timestamp;
				}
				countPerSlot[ms]++;
			}

			// Show messages
			if (piData != nullptr) {
				// Show Participant Info
				ShowParticipantInfo(mess, piData);
			
			} else if (debugData != nullptr) {
				ShowDebugMessage(ent, mess, debugData);

            } else if (ackData != nullptr) {
                ShowAckMessage(ent, mess, ackData);

            } else {
				// Ordinary Topic
				std::string str = "";
				str.reserve(256);
				CheckDelayedCRLF();
				if (args.logTime) {
					str += "[" + sds::sdsSystemTimeToLocalTime(ent.time) + "] ";
				}
				for (unsigned int i = 0; i < args.printFormat.size(); i++) {
					str += formatMap[args.printFormat[i]](mess, opsData) + ", ";
				}
				if ((str != "") && (!args.doMinimizeOutput)) {
					std::cout << str << std::endl;
				}
				if (opsData != nullptr) {
					// Dump OPSData->spareBytes content in hex
					if ((args.maxDumpBytes > 0) && (opsData->spareBytes.size() > 0)) {
						dumpHex(&opsData->spareBytes[0], opsData->spareBytes.size());
					}
					// Dump OPSData->spareBytes content in clear text
					if (MsgDump.Any() && (opsData->spareBytes.size() > 0)) {
						std::string tname = opsData->getTypeString().c_str();
						// Skip ev leading spaces
						std::string::size_type const idx = tname.find_first_not_of(' ');
						if (idx != std::string::npos) {
							if (idx > 0) { tname.erase(0, idx); }
							const auto pos = tname.find(' ');
							if (pos != tname.npos) {
								tname = tname.substr(0, pos);
							}
							MsgDump.Dump(tname, opsData->getVersionMask(), &opsData->spareBytes[0]);
						}
					}
				}
                if (args.doPubIdCheck) {
					// This may call our "onNewEvent(ops::Notifier<ops::PublicationIdNotification_T>* ...") method
					// We may need the time in that method so save it in a member variable
					_messageTime = ent.time;
					pubIdMap[ent.sub]->Check(mess);
				}
			}

			mess->unreserve();
		}
	}

	void periodicalThings()
	{
		// Check for disappering participants
		int64_t const limit = sds::sdsSystemTime() - sds::msToSdsSystemTimeUnits(5000);
		for (auto it = partMap.begin(); it != partMap.end(); ++it) {
			if (it->second.time < limit) {
				CheckDelayedCRLF();
				std::cout << "[" << partMap.size() - 1 << "] <<<<< Participant '" << it->first << "' has left" << std::endl;
				partMap.erase(it);
				break;
			}
		}
	}
};

#ifndef _WIN32
#include <time.h>
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
#endif

int main(const int argc, const char* argv[])
{
	std::cout << std::endl << c_program_version << std::endl << std::endl;

	sds::sdsSystemTimeInit();

	CArguments args;

	if (!args.HandleCommandLine(argc, argv)) { goto doShowUsage; }
	if (!args.ValidateArguments()) { goto doShowUsage; }

	{
		//Create an object that will listen to OPS events
		Main* m = new Main(args);

		bool doPause = false;
		int const numMess = 500;

		while (true) {
			if (_kbhit() != 0) {
				char buffer[1024];
				if (fgets(buffer, sizeof(buffer), stdin) != nullptr) {
					std::string line(buffer);

					// trim start
					std::string::size_type const idx = line.find_first_not_of(" \t");
					if (idx == std::string::npos) { continue; }
					if (idx > 0) { line.erase(0, idx); }
					if (line.size() == 0) { continue; }

					char const ch = line[0];
					line.erase(0, 1);

					if (ch == 0x1b) { break; }
					if ((ch == 'q') || (ch == 'Q')) { break; }
					if ((ch == 'x') || (ch == 'X')) { break; }

					if ((ch == 'p') || (ch == 'P')) {
						doPause = !doPause;
					}
					if ((ch == 's') || (ch == 'S')) {
						doPause = true;
						m->WorkOnList(1);
					}

					if (ch == '?') {
						std::cout << "Commands:\n";
						std::cout << "  q|Q|x|X     Exit program\n";
						std::cout << "  ?           Output this help\n";
						std::cout << "  p           Pause output\n";
						std::cout << "  s           Single step output\n";
					}
				}
			}
			ops::TimeHelper::sleep(std::chrono::milliseconds(1));

			if (doPause) {
				printf("Queued %d\r", m->numQueued());
			} else {
				m->WorkOnList(numMess);
			}
			m->periodicalThings();
		}

		delete m;
	}
	return 0;

doShowUsage:
	CArguments::ShowUsage();
	return 0;
}
