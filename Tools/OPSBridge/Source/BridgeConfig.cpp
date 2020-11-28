/**
*
* Copyright (C) 2010-2012 Saab Dynamics AB
*   author Lennart Andersson <nnnn@saabgroup.com>
*
* Copyright (C) 2018-2019 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <inttypes.h>

#include <OPSUtilities.h>

#include "BridgeLog.h"

#include "BridgeConfig.h"

namespace opsbridge {

const int64_t c_1KB = 1024;
const int64_t c_1MB = 1024 * c_1KB;
const int64_t c_1GB = 1024 * c_1MB;

BridgeConfig* BridgeConfig::_instance = nullptr;

BridgeConfig* BridgeConfig::Instance()
{
	return _instance;
}

int64_t BridgeConfig::parseSize(std::string s, int64_t const defaultValue)
{
	std::basic_string <char>::size_type index1;
	int64_t factor = 1;
	int64_t value = defaultValue;
	try {
		if ((index1 = s.find("GB")) != std::string::npos) {
			s.erase(index1, std::string::npos);
			factor = c_1GB;
			value = 0;
		} else if ((index1 = s.find("MB")) != std::string::npos) {
			s.erase(s.begin() + index1, s.end() - 1);
			factor = c_1MB;
			value = 0;
		} else if ((index1 = s.find("KB")) != std::string::npos) {
			s.erase(s.begin() + index1, s.end() - 1);
			factor = c_1KB;
			value = 0;
		}
		std::stringstream ss(s);
		ss >> value;
		return factor * value;
	}
	catch (...) {
		return defaultValue;
	}
}

BridgeConfig::THandlingType BridgeConfig::ParseHandlingType(std::string const str, THandlingType const defaultValue)
{
	if (str == "Discard") {
		return discard;
	} else if (str == "KeepLatest") {
		return keepLatest;
	} else if (str == "KeepAll") {
		return keepAll;
	}
	return defaultValue;
}

std::string BridgeConfig::HandlingTypeToStr(THandlingType const value)
{
	switch (value) {
		case discard:    return "Discard"; break;
        case keepLatest: return "KeepLatest"; break;
        case keepAll:    return "KeepAll"; break;
		default: break;
	}
	return "Unknown";
}

void BridgeConfig::checkTopicDomain(ops::ObjectName_T topicName)
{
	ops::ObjectName_T const dom = ops::utilities::domainName(topicName);
	if (dom != "") {
		for (uint32_t i = 0; i < vDomains.size(); i++) {
			if (dom == vDomains[i]) { return; }
		}
		vDomains.push_back(dom);
	}
}

void BridgeConfig::ParseBridgeConfiguration(std::string const tracestr, Configuration& config, TBridgeConfig& bc)
{
    UNUSED(tracestr);

	std::string s;

	// Set default values
	bc.endpoint.eType = tcpServer;
	bc.endpoint.localPort = 0;
	bc.endpoint.remoteHost = "";
	bc.endpoint.remotePort = 0;
	bc.iBufferSize = 1 * c_1GB;
	bc.iMinPubTime_ms = 1;

	// Check attributes for bridge
	s = config.getAttribute("name");
	if (s == "") { s = "NotSet"; }
	bc.sBridgeName = s;
	if (logConfig) BL_TRACE("# Bridge: name = %s\n", bc.sBridgeName.c_str());

#ifdef NOT_USED_YET
///TODO
	s = config.getAttribute("bufferSize");
	bc.iBufferSize = parseSize(s, bc.iBufferSize);
	if (logConfig) BL_TRACE("# Bridge: maxBufferSize = %" PRId64 "\n", bc.iBufferSize);

	s = config.getAttribute("minPubTime");
	bc.iMinPubTime_ms = config.parseInt64(s, bc.iMinPubTime_ms);
	if (logConfig) BL_TRACE("# Bridge: minPubTime = %" PRId64 "\n", bc.iMinPubTime_ms);
#endif

	// Read Endpoint configuration
	if (config.enter("endpoint")) {
		s = config.getAttribute("type");
		if (s == "tcpserver") { bc.endpoint.eType = tcpServer; }
		if (s == "tcpclient") { bc.endpoint.eType = tcpClient; }
		if (s == "local") { bc.endpoint.eType = local; }
		if (logConfig) BL_TRACE("#   Endpoint: type = %s\n", s.c_str());

		if (bc.endpoint.eType == local) {
			s = config.getAttribute("otherBridge");
			bc.endpoint.bridgeName = s;
			if (logConfig) BL_TRACE("#   Endpoint: otherBridge = %s\n", bc.endpoint.bridgeName.c_str());
		}

		if (bc.endpoint.eType == tcpClient) {
			s = config.getAttribute("remoteHost");
			bc.endpoint.remoteHost = s;
			if (logConfig) BL_TRACE("#   Endpoint: remoteHost = %s\n", s.c_str());

			s = config.getAttribute("remotePort");
			bc.endpoint.remotePort = config.parseInt(s, bc.endpoint.remotePort);
			if (logConfig) BL_TRACE("#   Endpoint: remotePort = %d\n", bc.endpoint.remotePort);
		}

		if (bc.endpoint.eType == tcpServer) {
			s = config.getAttribute("localPort");
			bc.endpoint.localPort = config.parseInt(s, bc.endpoint.localPort);
			if (logConfig) BL_TRACE("#   Endpoint: localPort = %d\n", bc.endpoint.localPort);
		}

		config.exit();
	}

	// Read topics
	if (config.enter("topics")) {
		// Set default values
		ops::ObjectKey_T defKey = "";
		ops::ObjectName_T defDestDomain = sDefaultDomain;
		int64_t defMinTime_ms = 0;
		int defPriority = 0;
		THandlingType defConnected = keepAll;
		THandlingType defDisconnected = discard;

		// Get eventual redefined default values
		//	<default priority="3" disconnected="KeepLatest"/>
		if (config.enter("default")) {
			s = config.getAttribute("destDomain");
			if (s != "") {
				defDestDomain = s;
			}
			if (logConfig) BL_TRACE("#   Default destDomain  = %s\n", defDestDomain.c_str());
	
			s = config.getAttribute("key");
			if (s != "") { defKey = s; }
			if (logConfig) BL_TRACE("#   Default key         = %s\n", defKey.c_str());
	
			s = config.getAttribute("mintime");
			defMinTime_ms = config.parseInt64(s, defMinTime_ms);
			if (logConfig) BL_TRACE("#   Default minTime     = %" PRId64 "\n", defMinTime_ms);

			s = config.getAttribute("priority");
			defPriority = config.parseInt(s, defPriority);
			if (defPriority < 0) { defPriority = 0; }
			if (defPriority > HIGHEST_PRIO) { defPriority = HIGHEST_PRIO; }
			if (logConfig) BL_TRACE("#   Default priority    = %d\n", defPriority);

			s = config.getAttribute("connected");
			defConnected = ParseHandlingType(s, defConnected);
			if (logConfig) BL_TRACE("#   Default connected   = %s\n", HandlingTypeToStr(defConnected).c_str());

			s = config.getAttribute("disconnected");
			defDisconnected = ParseHandlingType(s, defDisconnected);
			if (logConfig) BL_TRACE("#   Default disconnected= %s\n", HandlingTypeToStr(defDisconnected).c_str());

			config.exit();
		}

		// Get all defined topics
		//  <topic name="CTopic" key="A" mintime="500" priority="7" disconnected="Discard"/>
		for (int i = 0; i < config.numEntries("topic"); i++) {
			if (config.enter("topic", i)) {
				TTopicConfig tc;
				tc.sKey = defKey;
				tc.iMinTime_ms = defMinTime_ms;
				tc.iPriority = defPriority;
				tc.eConnectedHandling = defConnected;
				tc.eDisconnectedHandling = defDisconnected;

				// Topic name to subscribe for
				s = config.getAttribute("name");
				checkTopicDomain(s);
				tc.sTopicName = ops::utilities::fullTopicName(ops::utilities::domainName(s), ops::utilities::topicName(s));
				if (logConfig) BL_TRACE("#   Topic: Name    = %s\n", tc.sTopicName.c_str());

				// Destination topic name to publish on
				s = config.getAttribute("destName");
				ops::ObjectName_T dom, top;
				ops::utilities::splitTopicName(s, dom, top);
				if (dom == "") { dom = defDestDomain; }
				if (top == "") { top = ops::utilities::topicName(tc.sTopicName); }
				tc.sDestTopicName = ops::utilities::fullTopicName(dom, top);
				if (logConfig) BL_TRACE("#          DestName= %s\n", tc.sDestTopicName.c_str());
	
				s = config.getAttribute("key");
				if (s != "") { tc.sKey = s; }
				if (logConfig) BL_TRACE("#          Key     = %s\n", tc.sKey.c_str());
	
				s = config.getAttribute("mintime");
				tc.iMinTime_ms = config.parseInt64(s, tc.iMinTime_ms);
				if (logConfig) BL_TRACE("#          MinTime = %" PRId64 "\n", tc.iMinTime_ms);

				s = config.getAttribute("priority");
				tc.iPriority = config.parseInt(s, tc.iPriority);
				if (tc.iPriority < 0) { tc.iPriority = 0; }
				if (tc.iPriority > HIGHEST_PRIO) { tc.iPriority = HIGHEST_PRIO; }
				if (logConfig) BL_TRACE("#          Priority= %d\n", tc.iPriority);

				s = config.getAttribute("connected");
				tc.eConnectedHandling = ParseHandlingType(s, tc.eConnectedHandling);
				if (logConfig) BL_TRACE("#         connected= %s\n", HandlingTypeToStr(tc.eConnectedHandling).c_str());

				s = config.getAttribute("disconnected");
				tc.eDisconnectedHandling = ParseHandlingType(s, tc.eDisconnectedHandling);
				if (logConfig) BL_TRACE("#      disconnected= %s\n", HandlingTypeToStr(tc.eDisconnectedHandling).c_str());

				bc.vTopics.push_back(tc);

				config.exit();
			}	
		}
		config.exit();
	}

	// Read Raw configuration
	if (config.enter("raw")) {
		// Get all receives
		for (int i = 0; i < config.numEntries("receive"); i++) {
			if (config.enter("receive", i)) {
				TRawReceiveConfig rrc;
				rrc.sIp = config.getAttribute("ip");
				s = config.getAttribute("port");
				rrc.port = config.parseInt(s, rrc.port);
				rrc.sIfc = config.getAttribute("if", rrc.sIfc.c_str());
				if (logConfig) BL_TRACE("#   Raw receive from %s::%d (If:%s)\n", rrc.sIp.c_str(), rrc.port, rrc.sIfc.c_str());
				if ((rrc.sIp.length() > 0) && (rrc.port != 0)) {
					bc.vRawReceives.push_back(rrc);
				}
				config.exit();
			}
		}

		// Get all sends
		for (int i = 0; i < config.numEntries("send"); i++) {
			if (config.enter("send", i)) {
				TRawSendConfig rsc;
				rsc.rec.sIp = config.getAttribute("ip");
				s = config.getAttribute("port");
				rsc.rec.port = config.parseInt(s, rsc.rec.port);
				if ((rsc.rec.sIp.length() > 0) && (rsc.rec.port != 0)) {
					rsc.sNewIp = config.getAttribute("newip");
					s = config.getAttribute("newport");
					rsc.newPort = config.parseInt(s, rsc.newPort);
					rsc.sIfc = config.getAttribute("if", rsc.sIfc.c_str());
					s = config.getAttribute("ttl");
					rsc.ttl = config.parseInt(s, rsc.ttl);
					bc.vRawSends.push_back(rsc);
					if (logConfig) BL_TRACE("#   Raw send translate %s::%d --> %s::%d, If:%s, ttl:%d\n", 
						rsc.rec.sIp.c_str(), rsc.rec.port, 
						rsc.sNewIp.c_str(), rsc.newPort, rsc.sIfc.c_str(), rsc.ttl
					);
				}
				config.exit();
			}
		}

		config.exit();
	}
}

BridgeConfig::BridgeConfig(std::string fileName)
{
	// Set default values
	// Vectors have default size = 0
	logConfig = false;
	logDebug = false;
	maxWorkingSetSize = 1 * c_1GB;
	sDefaultDomain = "SDSDomain";
	sBridgeStatusTopic = "BridgeStatusTopic";
	sBridgeCommandTopic = "BridgeCommandTopic";

	vDomains.push_back(sDefaultDomain);

	// Save instance 
	_instance = this;

	BL_TRACE("# Using configuration file: %s\n", fileName.c_str());

	// Read configuration 
	std::ifstream inStream(fileName.c_str());
	if(inStream.is_open()) {
		try {
			std::string s;
			Configuration config(inStream, "root");

			// Some error control. Exit and use default values if parsing failed
			s = config.getParseResult();
			if (s != "") {
				BL_TRACE("#### %s\n", s.c_str());
				return;
			}

			config.root();
			if (config.enter("logging")) {
				logConfig = config.getAttribute("config") == "true";
				logDebug  = config.getAttribute("debug") == "true";
			}
#ifdef NOT_USED_YET
///TODO
			// <memory maxWorkingSetSize="300MB"/> 
			config.root();
			if (config.enter("memory")) {
				s = config.getAttribute("maxWorkingSetSize");
				maxWorkingSetSize = parseSize(s, maxWorkingSetSize);
				if (logConfig) BL_TRACE("# Memory: maxWorkingSetSize = %" PRId64 "\n", maxWorkingSetSize);
			}

			// <defaultDomain>SDSDomain</defaultDomain>
			config.root();
			s = config.getString("defaultDomain");
			if (s != "") sDefaultDomain = s;
			vDomains.clear();
			vDomains.push_back(sDefaultDomain);
			if (logConfig) BL_TRACE("# Default Domain: %s\n", sDefaultDomain.c_str());
#endif

			// Bridges
			config.root();
			if (config.enter("bridges")) {
#ifdef NOT_USED_YET
///TODO
				// <bridges commandTopic="BridgeCommandTopic" statusTopic="BridgeStatusTopic">
				sBridgeCommandTopic = config.getAttribute("commandTopic", sBridgeCommandTopic);
				if (logConfig) BL_TRACE("# commandTopic: %s\n", sBridgeCommandTopic.c_str());

				sBridgeStatusTopic = config.getAttribute("statusTopic", sBridgeStatusTopic);
				if (logConfig) BL_TRACE("# statusTopic: %s\n", sBridgeStatusTopic.c_str());
#endif
                if (logConfig) BL_TRACE("# Number of Bridges = %d\n", config.numEntries("bridge"));
				vBridges.resize(config.numEntries("bridge"));
				for (int iBridge = 0; iBridge < config.numEntries("bridge"); iBridge++) {
					if (config.enter("bridge", iBridge)) {
						ParseBridgeConfiguration("Bridge ", config, vBridges[iBridge]);
						config.exit();
					}
				}
				config.exit();
			}
		}
		catch (...) {
			BL_ERROR("#### ERROR reading configuration file. Using default values ####\n");
		}
		if (logConfig) BL_TRACE("-----------------------\n");
	}
}

BridgeConfig::~BridgeConfig()
{
};

}

