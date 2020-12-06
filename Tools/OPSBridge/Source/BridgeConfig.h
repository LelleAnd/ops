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

#pragma once

#include <vector>
#include <ops.h>

#include "Configuration.h"

namespace opsbridge {

	constexpr int HIGHEST_PRIO = 9;

	class BridgeConfig
	{
	public:
		BridgeConfig(std::string fileName);
		~BridgeConfig();

		static BridgeConfig* Instance();

		// Logging flags
		bool logConfig;
		bool logDebug;

		// Memory surveillance
		int64_t maxWorkingSetSize;

		// Domains used
		ops::ObjectName_T sDefaultDomain;			// Default domain if not specified for a topic
		std::vector<ops::ObjectName_T> vDomains;	// Domains specified with topics

		// Bridge Configuration
		enum TEndpointType {local, tcpClient, tcpServer};
		enum THandlingType {discard, keepLatest, keepAll};

		typedef struct {
			TEndpointType eType;
			std::string bridgeName;			// local
			std::string remoteHost;			// tcpClient
			int remotePort;					// tcpClient
			int localPort;					// tcpServer
		} TEndpointConfig;

		typedef struct {
			ops::ObjectName_T sTopicName;
			ops::ObjectName_T sDestTopicName;
			// Filters
			ops::ObjectKey_T sKey;						// == "", no key
			int64_t iMinTime_ms;						// == 0, no min time
			// Buffer handling
			int iPriority;								// 0..HIGHEST_PRIO, default prio == 0
			THandlingType eConnectedHandling;
			THandlingType eDisconnectedHandling;
		} TTopicConfig;

		typedef struct rawReceiveConfig_t {
			ops::Address_T sIp;
			int port{ 0 };
			ops::Address_T sIfc{ "127.0.0.1" };
		} TRawReceiveConfig;

		typedef struct rawSendConfig_t {
			TRawReceiveConfig rec;
			ops::Address_T sNewIp;
			int newPort{ 0 };
			ops::Address_T sIfc{ "127.0.0.1" };
			int ttl{ 1 };
		} TRawSendConfig;

		typedef struct {
			std::string sBridgeName;
			int64_t iBufferSize;
			int64_t iMinPubTime_ms;
			TEndpointConfig endpoint;
			std::vector<TTopicConfig> vTopics;	// List with topics to be "bridged"
			std::vector<TRawReceiveConfig> vRawReceives;
			std::vector<TRawSendConfig> vRawSends;
		} TBridgeConfig;

		std::vector<TBridgeConfig> vBridges;
		std::string sBridgeStatusTopic;
		std::string sBridgeCommandTopic;

	private:
		static BridgeConfig* _instance;

		int64_t parseSize(std::string s, int64_t defaultValue);

		THandlingType ParseHandlingType(std::string str, THandlingType defaultValue);
		std::string HandlingTypeToStr(THandlingType value);

		void checkTopicDomain(ops::ObjectName_T topicName);

		void ParseBridgeConfiguration(std::string tracestr, Configuration& config, TBridgeConfig& bc);
	};

}
