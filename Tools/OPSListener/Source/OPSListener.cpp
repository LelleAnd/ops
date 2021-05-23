//

#include <time.h>
#include <stdio.h>
#include <limits>

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

#include "COpsConfigHelper.h"
#include "SdsSystemTime.h"
#include "MessageDump.h"

#ifndef _WIN32

#define LPWSTR char*

#endif

const char c_program_version[] = "OPSListener Version 2021-05-22";

void showDescription()
{
	std::cout << "  This program can subscribe to any OPS topic and it is possible to choose what ";
	std::cout << "information to present and in which order." << std::endl;
	std::cout << "  This can be used to test if / verify that topics are published." << std::endl;
	std::cout << "  Topic names can use wildcards (eg. \".*zz.*\" means any topic with 'zz' any where in the name)." << std::endl;
	std::cout << std::endl;
}

void ShowUsage()
{
    std::cout << std::endl << "Usage:" << std::endl;
    std::cout << "  OPSListener [-v] [-?] [-c ops_cfg_file [-c ops_cfg_file [...]]]" << std::endl;
    std::cout << "              [-j json_file [-j json_file [...]]] [-jr rowlimit]" << std::endl;
    std::cout << "              [-t] [-pA | -p<option_chars>] [-d [num]]" << std::endl;
    std::cout << "              [-a arg_file [-a arg_file [...]]]" << std::endl;
    std::cout << "              [-GA | -G domain [-G domain [...]]]" << std::endl;
    std::cout << "              [-IA | -I domain [-I domain [...]] [-O]]" << std::endl;
    std::cout << "              [-SA | -S domain [-S domain [...]]]" << std::endl;
    std::cout << "              [-D default_domain] [-C [-E]] [-u] [-n] Topic [Topic ...]" << std::endl;
    std::cout << std::endl;
    std::cout << "    -?                 Shows a short description" << std::endl;
    std::cout << "    -a arg_file        File with command line arguments" << std::endl;
    std::cout << "    -c ops_config_file Specifies an OPS configuration file to use" << std::endl;
    std::cout << "                       If none given, the default 'ops_config.xml' is used" << std::endl;
    std::cout << "    -C                 Do a publication ID check" << std::endl;
    std::cout << "    -d [num]           Dump message content in hex (the part derived from OPSObject)" << std::endl;
    std::cout << "    -D default_domain  Default domain name to use for topics given without domain name" << std::endl;
    std::cout << "                       If none given, the default 'SDSDomain' is used" << std::endl;
    std::cout << "                       A new default can be given between topics" << std::endl;
    std::cout << "    -E                 If -C given, minimize normal output" << std::endl;
    std::cout << "    -G domain          Subscribe to Debug Request/Response from given domain" << std::endl;
    std::cout << "    -GA                Subscribe to Debug Request/Response from all domains in given configuration files" << std::endl;
    std::cout << "    -I domain          Subscribe to Participant Info Data from given domain" << std::endl;
    std::cout << "    -IA                Subscribe to Participant Info Data from all domains in given configuration files" << std::endl;
    std::cout << "    -j json_file       Specifies a JSON-file with OPS Message descriptions (generated from opsc)" << std::endl;
	std::cout << "    -jr rowlimit       Specifies the row limit for presented vectors in the Message descriptions" << std::endl;
	std::cout << "    -n                 Don't subscribe to topics following" << std::endl;
    std::cout << "    -O                 if -I or -IA given, only show arriving and timed out participants" << std::endl;
    std::cout << "    -p<option_chars>   Defines for received messages, which fields to print and in which order" << std::endl;
    std::cout << "                 n       Publisher Name" << std::endl;
    std::cout << "                 i       Publication Id" << std::endl;
    std::cout << "                 T       Topic Name" << std::endl;
    std::cout << "                 y       Type" << std::endl;
    std::cout << "                 s       Sparebytes Size" << std::endl;
    std::cout << "                 k       Key" << std::endl;
    std::cout << "                 S       Source IP::Port" << std::endl;
    std::cout << "                 v       Version mask OPSMessage" << std::endl;
    std::cout << "                 V       Version mask contained OPSObject" << std::endl;
    std::cout << "    -pA                Short for all option chars in the program default order" << std::endl;
    std::cout << "    -S domain          Subscribe to all topics in given domain" << std::endl;
    std::cout << "    -SA                Subscribe to all topics in all domains in given configuration files" << std::endl;
    std::cout << "    -t                 Print receive time for each message" << std::endl;
    std::cout << "    -u                 Force subscription to UDP static route topics (may interfere with real subscriber)" << std::endl;
    std::cout << "    -v                 Verbose output during parsing of command line arguments" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
}

class CArguments
{
public:
	static std::string getValidFormatChars() { return "TknisySvV"; }
	static ops::ObjectName_T getDefaultDomain() { return "SDSDomain"; }

	std::vector<ops::FileName_T> cfgFiles;
    std::vector<std::string> jsonFiles;
    std::vector<ops::ObjectName_T> topicNames;
	std::vector<ops::ObjectName_T> subscribeDomains;
	std::vector<ops::ObjectName_T> infoDomains;
	std::vector<ops::ObjectName_T> debugDomains;
	std::vector<ops::ObjectName_T> skipTopicNames;
	std::string printFormat = "";
	ops::ObjectName_T defaultDomain;
	bool verboseOutput = false;
	bool logTime = false;
	bool allDebugDomains = false;
	bool allInfoDomains = false;
	bool allTopics = false;
	bool onlyArrivingLeaving = false;
	bool doPubIdCheck = false;
	bool doMinimizeOutput = false;
	bool skipTopics = false;
	bool dontSkipUdpStaticRoute = false;
    int maxDumpBytes = 0;
	int rowlimit = INT_MAX;

	CArguments() : defaultDomain(getDefaultDomain())
	{
		// Create a map with all valid format chars, used for validating -p<...> argument
		validFormatChars = getValidFormatChars();
		for (unsigned int i = 0; i < validFormatChars.size(); i++) { validChars[validFormatChars[i]] = true; }
	}

	~CArguments() = default;
	CArguments(const CArguments& other) = delete;
	CArguments& operator= (const CArguments& other) = delete;
	CArguments(CArguments&& other) = delete;
	CArguments& operator =(CArguments&& other) = delete;

	bool HandleArguments(const LPWSTR* const szArglist, int const nStart, int const nArgs)
	{
		if (verboseOutput) {
			for (int i = nStart; i < nArgs; i++) {
#ifdef _WIN32
				printf("%s %d: %ws\n", indent.c_str(), i, szArglist[i]);
#else
				printf("%s %d: %s\n", indent.c_str(), i, szArglist[i]);
#endif
			}
		}

		// Decode arguments
		int argIdx = nStart;
		while (argIdx < nArgs) {
			std::string argument = toAnsi(szArglist[argIdx++]);

			if ((argument == "?") || (argument == "-?")) {
				showDescription();
				return false;
			}

			// Command line argument file
			if (argument == "-a") {
#ifdef _WIN32
				if (argIdx >= nArgs) {
					std::cout << "Argument '-a' is missing value" << std::endl;
					return false;
				}
				if (!ParseFile(toAnsi(szArglist[argIdx++]))) return false;
#else
				std::cout << "Argument '-a' currently NOT supported on non-WIN32" << std::endl;
#endif
				continue;
			}

			// Configuration files
			if (argument == "-c") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-c' is missing value" << std::endl;
					return false;
				}
				cfgFiles.push_back(toAnsi(szArglist[argIdx++]).c_str());
				continue;
			}

			// PuIdChecker
			if (argument == "-C") {
				doPubIdCheck = true;
				continue;
			}

            // DumpMessage
            if (argument == "-d") {
                maxDumpBytes = 0x7FFFFFFF;
                if (argIdx >= nArgs) {
                    // No more args so max should be used
                    continue;
                }
                // Check next arg if it belongs to -d
                std::string val = toAnsi(szArglist[argIdx]);
                if (val.size() > 0) {
                    if (isdigit(val[0]) != 0) {
                        maxDumpBytes = atoi(val.c_str());
                        argIdx++;
                    }
                }
                continue;
            }

			// Default domain
			if (argument == "-D") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-D' is missing value" << std::endl;
					return false;
				}
				defaultDomain = toAnsi(szArglist[argIdx++]).c_str();
				continue;
			}

			// minimize output
			if (argument == "-E") {
				doMinimizeOutput = true;
				continue;
			}

			// Subscribe to Debug Request/Response Data
			if (argument == "-G") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-G' is missing value" << std::endl;
					return false;
				}
				debugDomains.push_back(toAnsi(szArglist[argIdx++]).c_str());
				continue;
			}

			if (argument == "-GA") {
				allDebugDomains = true;
				continue;
			}

			// Subscribe to Participant Info Data
			if (argument == "-I") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-I' is missing value" << std::endl;
					return false;
				}
				infoDomains.push_back(toAnsi(szArglist[argIdx++]).c_str());
				continue;
			}

			if (argument == "-IA") {
				allInfoDomains = true;
				continue;
			}

            // JSON files
            if (argument == "-j") {
                if (argIdx >= nArgs) {
                    std::cout << "Argument '-j' is missing value" << std::endl;
                    return false;
                }
                jsonFiles.push_back(toAnsi(szArglist[argIdx++]).c_str());
                continue;
            }

			if (argument == "-jr") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-jr' is missing value" << std::endl;
					return false;
				}
				std::string val = toAnsi(szArglist[argIdx++]);
				if (val.size() > 0) {
					if (isdigit(val[0]) != 0) {
						int tmp = atoi(val.c_str());
						if (tmp > 0) {
							rowlimit = tmp;
						}
					}
				}
				continue;
			}

			if (argument == "-O") {
				onlyArrivingLeaving = true;
				continue;
			}

			// Subscribe to domain
			if (argument == "-S") {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-S' is missing value" << std::endl;
					return false;
				}
				subscribeDomains.push_back(toAnsi(szArglist[argIdx++]).c_str());
				continue;
			}

			if (argument == "-SA") {
				allTopics = true;
				continue;
			}

			if (argument == "-n") {
				skipTopics = true;
				continue;
			}

			/// -p with all options
			if (argument == "-pA") {
				printFormat = getValidFormatChars();
				continue;
			}

			/// -p<...>
			if (argument.find("-p") == 0) {
				argument.erase(0, 2);
				for (unsigned int i = 0; i < argument.size(); i++) {
					if (validChars.find(argument[i]) == validChars.end()) {
						std::cout << "Argument '-p' has an invalid option char: " << argument[i] << std::endl;
						return false;
					}
				}
				printFormat = argument;
				continue;
			}

			if (argument == "-t") {
				logTime = true;
				continue;
			}

			// Force subscription to UDP static route topics
			if (argument == "-u") {
				dontSkipUdpStaticRoute = true;
				continue;
			}

			// Verbose output or not
			if (argument == "-v") {
				verboseOutput = true;
				continue;
			}

			// The rest is topic names
			if (argument != "") {
				ops::ObjectName_T topname = argument.c_str();
				/// If no domain given, set the default domain
				if (ops::utilities::topicName(topname) == topname) {
					topname = ops::utilities::fullTopicName(defaultDomain, topname);
				}
				bool found = false;
				if (skipTopics) {
					for (unsigned int i = 0; i < skipTopicNames.size(); i++) {
						if (skipTopicNames[i] == topname) {
							found = true;
							break;
						}
					}
					if (!found) { skipTopicNames.push_back(topname); }
				} else {
					for (unsigned int i = 0; i < topicNames.size(); i++) {
						if (topicNames[i] == topname) {
							found = true;
							break;
						}
					}
					if (!found) { topicNames.push_back(topname); }
				}
			}
		}

		return true;
	}

	///TODO for Linux
#ifdef _WIN32
	bool ParseFile(std::string fileName)
	{
		char buffer[16384];
		wchar_t wbuffer[32768];
		std::string oldIndent = indent;

		FILE* stream = nullptr;
#ifdef _WIN32
		if (fopen_s(&stream, fileName.c_str(), "r") != 0) {
			std::cout << "Failed to open argument file: " << fileName << std::endl;
			return false;
		}
#else
		stream = fopen(fileName.c_str(), "r");
#endif
		if (stream == nullptr) {
			std::cout << "Failed to open argument file: " << fileName << std::endl;
			return false;
		}

		indent += "  ";
		if (verboseOutput) std::cout << indent << "Parsing argument file: " << fileName << std::endl;

		bool returnValue = true;
		while (!feof(stream) && returnValue) {
			if (fgets(buffer, sizeof(buffer), stream) == nullptr) break;

			int len = (int)strlen(buffer);
			if (len == 0) continue;
			if (buffer[0] == '#') continue;

			if (buffer[len - 1] == '\n') buffer[len - 1] = ' ';

			size_t numConverted = 0;
			if (mbstowcs_s(&numConverted, wbuffer, 32768, buffer, _TRUNCATE) != 0) {
				std::cout << "mbstowcs_s() failed" << std::endl;
				returnValue = false;
				break;
			}

			LPWSTR *szArglist;
			int nArgs;

			szArglist = CommandLineToArgvW(wbuffer, &nArgs);
			if (nullptr == szArglist) {
				std::cout << "CommandLineToArgvW() failed" << std::endl;
				returnValue = false;
			} else {
				returnValue = HandleArguments(szArglist, 0, nArgs);
			}

			// Free memory allocated for CommandLineToArgvW arguments.
			if (szArglist) LocalFree(szArglist);
		}
		fclose(stream);
		if (verboseOutput) std::cout << indent << "Finished parsing argument file: " << fileName << std::endl;
		indent = oldIndent;
		return returnValue;
	}
#endif

#ifdef _WIN32
	bool HandleCommandLine(const int , const char** )
	{
		bool returnValue = false;
		int nArgs;
		LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
		if (nullptr == szArglist) {
			std::cout << "CommandLineToArgvW failed" << std::endl;
			return false;
		} else {
			returnValue = HandleArguments(szArglist, 1, nArgs);
		}
		// Free memory allocated for CommandLineToArgvW arguments.
		LocalFree(szArglist);
		return returnValue;
#else
	bool HandleCommandLine(const int argc, const char* argv[])
	{
		return HandleArguments(argv, 1, argc);
#endif
	}

	bool ValidateArguments()
	{
		// Validate arguments
		if ((topicNames.size() == 0) && (subscribeDomains.size() == 0) && (infoDomains.size() == 0) && (debugDomains.size() == 0) &&
			!allInfoDomains && !allTopics && !allDebugDomains) {
			std::cout << "No topics/domains given!!" << std::endl;
			return false;
		}

		if (printFormat == "") {
			// Set default format
			if ((topicNames.size() > 1) || (subscribeDomains.size() > 0) || allTopics) {
				printFormat = "Tisky";
			} else {
				printFormat = "isky";
			}
		}

		if (!doPubIdCheck) { doMinimizeOutput = false; }

		if (verboseOutput) {
			//std::vector<std::string> cfgFiles;
			std::cout << "" << std::endl;

			//std::vector<std::string> topicNames;
			std::cout << "" << std::endl;

			//std::string printFormat;
			std::cout << "  Print format:        " << printFormat << std::endl;
			std::cout << std::endl << std::endl;
		}

		return true;
	}

private:
	std::string validFormatChars;
	std::map<char, bool> validChars;
	std::string indent = "";

	static std::string toAnsi(const LPWSTR wStr)
	{
#ifdef _WIN32
		// Convert current wide string to ANSI (std::string)
		char tmp[16384];
		size_t numConverted;
		wcstombs_s(&numConverted, tmp, sizeof(tmp), wStr, _TRUNCATE);
		return tmp;
#else
		return std::string(wStr);
#endif
	}
};

// =======================================================================================
// =======================================================================================

// Specialized factory used for receiving ANY topic as an OPSObject
class AllOpsTypeFactory : public ops::SerializableFactory
{
public:
	virtual ops::Serializable* create(const ops::TypeId_T& type) override
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

// ---------------------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------------------
//Create a class to act as a listener for OPS data and deadlines
class Main : ops::DataListener, ops::Listener<ops::PublicationIdNotification_T>, ops::Listener<ops::ConnectStatus>
{
private:
	bool logTime;
	MyLogger logger;
	COpsConfigHelper opsHelper;
	std::string printFormat;
	bool onlyArrivingLeaving;
	bool doPubIdCheck;
	bool doMinimizeOutput;
    size_t maxDumpBytes;

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

	std::map<ops::InternalString_T, int64_t> partMap;

	// local storage of current worked on entry time
	int64_t _messageTime = 0;

	// Since our factory creates OPSObjects, the spareBytes starts after the OPSObject fields, 
	// so we need to skip the OPSObject fields on top level.
	message_dump::SkipFunc_T skipFunc = [=](const std::string& name) -> bool { return name == "ops.OPSObject"; };
	message_dump::MessageDump MsgDump;

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
	Main(CArguments& args) :
		logTime(args.logTime),
		opsHelper(&logger, &logger, args.defaultDomain),
		printFormat(args.printFormat),
		onlyArrivingLeaving(args.onlyArrivingLeaving),
		doPubIdCheck(args.doPubIdCheck),
		doMinimizeOutput(args.doMinimizeOutput),
        maxDumpBytes(args.maxDumpBytes),
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
			if (doPubIdCheck) {
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
				if (doPubIdCheck) {
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
				if (doPubIdCheck) {
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
		if (logTime) {
			str += "[" + sds::sdsSystemTimeToLocalTime(_messageTime) + "] ";
		}
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
	static void ShowDebugEntity(const opsidls::DebugRequestResponseData* const data)
	{
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
	static void ShowPubEntity(const opsidls::DebugRequestResponseData* const data)
	{
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
	static void ShowSubEntity(const opsidls::DebugRequestResponseData* const data)
	{
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
    void ShowAckMessage(const TEntry& ent, const ops::OPSMessage* const mess, const opsidls::SendAckPatternData* const data) const
    {
        if (logTime) {
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
    void ShowDebugMessage(const TEntry& ent, const ops::OPSMessage* const mess, const opsidls::DebugRequestResponseData* const data) const
	{
		// Show Debug Request/Response data
		std::string str = "";
		if (logTime) {
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
	void ShowParticipantInfo(const ops::OPSMessage* const mess, const ops::ParticipantInfoData* const piData)
	{
		// First check if it's from any of our own participants
		bool fromMe = false;
		for (unsigned int i = 0; i < ownPartInfoNames.size(); i++) {
			if (ownPartInfoNames[i] == piData->name) { fromMe = true; }
		}
		if (!fromMe) {
			if (partMap.find(piData->name) == partMap.end()) {
				std::cout << "[" << partMap.size() + 1 << "] >>>>> Participant '" << piData->name << "' arrived" << std::endl;
			}
			partMap[piData->name] = sds::sdsSystemTime();
		}
		if (!fromMe && !onlyArrivingLeaving) {
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
				std::endl;
			//std::vector<TopicInfoData> subscribeTopics;
			std::cout << "  subscr Topics: ";
			for (unsigned int i = 0; i < piData->subscribeTopics.size(); i++) {
				std::cout << piData->subscribeTopics[i].name << " ";
			}
			std::cout << std::endl;
			//std::vector<TopicInfoData> publishTopics;
			std::cout << "  pub Topics: ";
			for (unsigned int i = 0; i < piData->publishTopics.size(); i++) {
				std::cout << piData->publishTopics[i].name << " ";
			}
			std::cout << std::endl;
			//std::vector<std::string> knownTypes;
			std::cout << "  knownTypes: ";
			for (unsigned int i = 0; i < piData->knownTypes.size(); i++) {
				std::cout << piData->knownTypes[i] << " ";
			}
			std::cout << std::endl;
		}
	}
	//
    static char toAscii(uint8_t const val)
    {
        if (val < 0x20) { return '.'; }
        if (val > 0x7F) { return '.'; }
        return (char)val;
    }
    void dumpHex(const char* const ptr, size_t numbytes) const
    {
        int offset = 0;
        if (numbytes > maxDumpBytes) { numbytes = maxDumpBytes; }
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
				if (logTime) {
					str += "[" + sds::sdsSystemTimeToLocalTime(ent.time) + "] ";
				}
				for (unsigned int i = 0; i < printFormat.size(); i++) {
					str += formatMap[printFormat[i]](mess, opsData) + ", ";
				}
				if ((str != "") && (!doMinimizeOutput)) {
					std::cout << str << std::endl;
				}
				if (opsData != nullptr) {
					// Dump OPSData->spareBytes content in hex
					if (opsData->spareBytes.size() > 0) {
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
                if (doPubIdCheck) {
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
		for (std::map<ops::InternalString_T, int64_t>::iterator it = partMap.begin(); it != partMap.end(); ++it) {
			if (it->second < limit) {
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
		int const numMess = 100;

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
					}
				}
			}
			ops::TimeHelper::sleep(std::chrono::milliseconds(10));

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
	ShowUsage();
	return 0;
}
