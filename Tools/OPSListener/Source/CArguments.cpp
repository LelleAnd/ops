
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif

#include "OPSUtilities.h"

#include "CArguments.h"

void CArguments::showDescription()
{
	std::cout << "  This program can subscribe to any OPS topic and it is possible to choose what information to present and in which order." << std::endl;
	std::cout << "  This can be used to test if / verify that topics are published." << std::endl;
	std::cout << "  Topic names can use regex (eg. \".*zz.*\" means any topic with 'zz' any where in the name)." << std::endl;
	std::cout << std::endl;
}

void CArguments::ShowUsage()
{
	std::cout << std::endl << "Usage:" << std::endl;
	std::cout << "  OPSListener [-v] [-?] [-c ops_cfg_file [-c ops_cfg_file [...]]]" << std::endl;
	std::cout << "              [-a arg_file [-a arg_file [...]]]" << std::endl;
	std::cout << "              [-j json_file [-j json_file [...]]] [-jr rowlimit]" << std::endl;
	std::cout << "              [-t] [-pA | -p<option_chars>] [-d [num]]" << std::endl;
	std::cout << "              [-s] " << std::endl;
	std::cout << "              [-GA | -G domain [-G domain [...]]]" << std::endl;
	std::cout << "              [-IA | -I domain [-I domain [...]] [-o] [-O]]" << std::endl;
	std::cout << "              [-SA | -S domain [-S domain [...]]]" << std::endl;
	std::cout << "              [-CH channel [-CH channel [...]]]" << std::endl;
	std::cout << "              [-f<knSy> param [-f<knSy> param [...]]]" << std::endl;
	std::cout << "              [-D default_domain] [-C [-E]] [-u] [-n] Topic [Topic ...]" << std::endl;
	std::cout << std::endl;
	std::cout << "    -?                 Shows a short description" << std::endl;
	std::cout << "    -a arg_file        File with command line arguments" << std::endl;
	std::cout << "    -c ops_config_file Specifies an OPS configuration file to use" << std::endl;
	std::cout << "                       If none given, the default 'ops_config.xml' is used" << std::endl;
	std::cout << "    -C                 Do a publication ID check" << std::endl;
	std::cout << "    -CH channel        Limit to topics on given channel(s)" << std::endl;
	std::cout << "    -d [num]           Dump message content in hex (the part derived from OPSObject)" << std::endl;
	std::cout << "    -D default_domain  Default domain name to use for topics given without domain name" << std::endl;
	std::cout << "                       If none given, the default 'SDSDomain' is used" << std::endl;
	std::cout << "                       A new default can be given between topics" << std::endl;
	std::cout << "    -E                 If -C or -s given, minimize normal output" << std::endl;
	std::cout << "    -f<knSy> param     Filter messages shown on given value (regex)" << std::endl;
	std::cout << "                 k       param = Key" << std::endl;
	std::cout << "                 n       param = Publisher Name" << std::endl;
	std::cout << "                 S       param = Source IP::Port" << std::endl;
	std::cout << "                 y       param = Type" << std::endl;
	std::cout << "    -G domain          Subscribe to Debug Request/Response from given domain" << std::endl;
	std::cout << "    -GA                Subscribe to Debug Request/Response from all domains in given configuration files" << std::endl;
	std::cout << "    -I domain          Subscribe to Participant Info Data from given domain" << std::endl;
	std::cout << "    -IA                Subscribe to Participant Info Data from all domains in given configuration files" << std::endl;
	std::cout << "    -j json_file       Specifies a JSON-file with OPS Message descriptions (generated from opsc)" << std::endl;
	std::cout << "    -jr rowlimit       Specifies the row limit for presented vectors in the Message descriptions" << std::endl;
	std::cout << "    -n                 Don't subscribe to topics following" << std::endl;
	std::cout << "    -o                 if -I or -IA given, show more detailed Topic info" << std::endl;
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
	std::cout << "    -s                 Calculate and show statistics" << std::endl;
	std::cout << "    -S domain          Subscribe to all topics in given domain" << std::endl;
	std::cout << "    -SA                Subscribe to all topics in all domains in given configuration files" << std::endl;
	std::cout << "    -t                 Print receive time for each message" << std::endl;
	std::cout << "    -u                 Force subscription to UDP static route topics (may interfere with real subscriber)" << std::endl;
	std::cout << "    -v                 Verbose output during parsing of command line arguments" << std::endl;
	std::cout << std::endl;
}

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

bool CArguments::HandleArguments(const LPWSTR* const szArglist, int const nStart, int const nArgs)
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
			if (argIdx >= nArgs) {
				std::cout << "Argument '-a' is missing value" << std::endl;
				return false;
			}
			if (!ParseFile(toAnsi(szArglist[argIdx++]))) { return false; }
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

		// Limit to channels
		if (argument == "-CH") {
			if (argIdx >= nArgs) {
				std::cout << "Argument '-CH' is missing value" << std::endl;
				return false;
			}
			channels.push_back(toAnsi(szArglist[argIdx++]).c_str());
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

		// Filters
		if (argument.find("-f") == 0) {
			try {
				if (argIdx >= nArgs) {
					std::cout << "Argument '-f[knSy]' is missing value" << std::endl;
					return false;
				}
				std::string param(toAnsi(szArglist[argIdx++]).c_str());
				if (filter == nullptr) {
					filter = new filter::FilterAny();
				}
				if (argument == "-fk") {
					filter->Add(new filter::FilterKey(param));
				}
				if (argument == "-fn") {
					filter->Add(new filter::FilterPubName(param));
				}
				if (argument == "-fS") {
					filter->Add(new filter::FilterSource(param));
				}
				if (argument == "-fy") {
					filter->Add(new filter::FilterType(param));
				}
			}
			catch (std::exception& e) {
				std::cout << "Argument '-f[knSy]' exception: " << e.what() << std::endl;
				return false;
			}
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

		if (argument == "-o") {
			detailedTopicInfo = true;
			continue;
		}

		if (argument == "-O") {
			onlyArrivingLeaving = true;
			continue;
		}

		if (argument == "-s") {
			statistics = true;
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
			// Call recursive to handle rest of arguments with verboseOutput
			HandleArguments(szArglist, argIdx, nArgs);
			break;
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
			}
			else {
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

bool CArguments::ParseFile(std::string fileName)
{
	char buffer[16384];
#ifdef _WIN32
	wchar_t wbuffer[32768];
#endif
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
	if (verboseOutput) { std::cout << indent << "Parsing argument file: " << fileName << std::endl; }

	bool returnValue = true;
	while (!feof(stream) && returnValue) {
		if (fgets(buffer, sizeof(buffer), stream) == nullptr) { break; }

		int len = (int)strlen(buffer);
		if (len == 0) { continue; }
		if (buffer[0] == '#') { continue; }

		for (int i = 0; i < len; ++i) {
			if ((buffer[i] == '\n') || (buffer[i] == '\r')) {
				buffer[i] = ' ';
			}
		}

#ifdef _WIN32
		LPWSTR* szArglist = nullptr;
		int nArgs = 0;
		size_t numConverted = 0;
		if (mbstowcs_s(&numConverted, wbuffer, 32768, buffer, _TRUNCATE) != 0) {
			std::cout << "mbstowcs_s() failed" << std::endl;
			returnValue = false;
			break;
		}

		szArglist = CommandLineToArgvW(wbuffer, &nArgs);
		if (nullptr == szArglist) {
			std::cout << "CommandLineToArgvW() failed" << std::endl;
			returnValue = false;
		}
		else {
			returnValue = HandleArguments(szArglist, 0, nArgs);
		}

		// Free memory allocated for CommandLineToArgvW arguments.
		if (szArglist) { LocalFree(szArglist); }
#else
		// Create argv[] and argc in-place in buffer
		// \x escapes	--> x
		// "x" string	--> x
		std::vector<char*> args;
		bool escaped = false;
		bool inString = false;
		bool pushArg = true;
		for (int i = 0, j = 0; i < len; ++i, ++j) {
			if (pushArg) {
				args.push_back(&buffer[i]);
				pushArg = false;
			}
			if (j < i) {
				// compact
				buffer[j] = buffer[i];
			}
			switch (buffer[i]) {
			case '\\':
				escaped = !escaped;
				--j;
				break;
			case '"':
				if (!escaped) {
					inString = !inString;
					--j;
				}
				escaped = false;
				break;
			case ' ':
				if (!inString && !escaped) {
					buffer[j] = '\0';
					pushArg = true;
					j = i;
				}
				escaped = false;
				break;
			default:;
				escaped = false;
				break;
			}
		}
		returnValue = HandleArguments((const char**)&args[0], 0, args.size());
#endif
	}
	fclose(stream);

	if (verboseOutput) { std::cout << indent << "Finished parsing argument file: " << fileName << std::endl; }
	indent = oldIndent;
	return returnValue;
}

#ifdef _WIN32
bool CArguments::HandleCommandLine(const int, const char**)
{
	bool returnValue = false;
	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nullptr == szArglist) {
		std::cout << "CommandLineToArgvW failed" << std::endl;
		return false;
	}
	else {
		returnValue = HandleArguments(szArglist, 1, nArgs);
	}
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);
	return returnValue;
#else
bool CArguments::HandleCommandLine(const int argc, const char* argv[])
{
	return HandleArguments(argv, 1, argc);
#endif
}

bool CArguments::ValidateArguments()
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
		}
		else {
			printFormat = "isky";
		}
	}

	if (!doPubIdCheck && !statistics) { doMinimizeOutput = false; }

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

