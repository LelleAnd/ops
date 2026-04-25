#pragma once

#include <limits.h>
#include <string>
#include <vector>

#include "ops.h"
#include "FilterMessage.h"

#ifndef _WIN32
#define LPWSTR char*
#endif

class CArguments
{
public:
	static std::string getValidFormatChars() { return "TknisySvV"; }
	static ops::ObjectName_T getDefaultDomain() { return "SDSDomain"; }

	filter::FilterAny* filter{ nullptr };
	std::vector<ops::FileName_T> cfgFiles;
	std::vector<std::string> jsonFiles;
	std::vector<ops::ObjectName_T> topicNames;
	std::vector<ops::ObjectName_T> subscribeDomains;
	std::vector<ops::ObjectName_T> infoDomains;
	std::vector<ops::ObjectName_T> debugDomains;
	std::vector<ops::ObjectName_T> skipTopicNames;
	std::vector<ops::ChannelId_T> channels;
	std::string printFormat = "";
	ops::ObjectName_T defaultDomain;
	bool allDebugDomains = false;
	bool allInfoDomains = false;
	bool allTopics = false;
	bool detailedTopicInfo = false;
	bool doMinimizeOutput = false;
	bool dontSkipUdpStaticRoute = false;
	bool doPubIdCheck = false;
	bool logTime = false;
	size_t maxDumpBytes = 0;
	bool onlyArrivingLeaving = false;
	int rowlimit = INT_MAX;
	bool skipTopics = false;
	bool statistics = false;
	bool verboseOutput = false;

	static void showDescription();
	static void ShowUsage();

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

	bool HandleArguments(const LPWSTR* const szArglist, int const nStart, int const nArgs);
	bool ParseFile(std::string fileName);

#ifdef _WIN32
	bool HandleCommandLine(const int, const char**);
#else
	bool HandleCommandLine(const int argc, const char* argv[]);
#endif

	bool ValidateArguments();

private:
	std::string validFormatChars;
	std::map<char, bool> validChars;
	std::string indent = "";

};
