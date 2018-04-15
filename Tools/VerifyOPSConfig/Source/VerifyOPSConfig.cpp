// VerifyOPSConfig.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include <vector>

#include "OPSConfig.h"
#include "PrintArchiverOut.h"

#include "Configuration.h"

const std::string sVersion = "Version 2018-04-13";


bool gWarningGiven = false;

#define LOG_WARN(message) { std::cout << message; gWarningGiven = true; } 

#define LOG_DEBUG(message) { if (bDebug) std::cout << message; } 

class CVerifyOPSConfig 
{
private:
	// Arguments
	bool bDebug;
	
	// 
	int iNumDomains;
	std::vector<std::string> vDomains;
	std::vector<std::string> vDomainMeta;
	std::vector<std::string> vTopics;
	std::vector<std::string> vChannels;
	std::vector<std::string> vTransportTopics;
	bool bUdpRequireMetadata;

public:
	CVerifyOPSConfig(std::string filename, bool debug):
	    bDebug(debug),
		iNumDomains(0),
		bUdpRequireMetadata(false)
	{
		try {
			std::string s;
			Configuration config(filename);

			LOG_DEBUG( std::endl << "Checking file: " << filename << std::endl << std::endl );

			// Some error control. 
			s = config.getParseResult();
			if (s != "") {
				LOG_WARN( ">>> Error parsing file, Result: " << s << std::endl );
				return;
			}

			//<root>
			//    <ops_config type = "DefaultOPSConfigImpl">
			//        <domains>

			config.root();

			// Check for unknown entries
			{
				std::vector<std::string> known = { "root" };
				CheckForUnknown(config, known, "in file top-level");
			}

			verifyOnlyOneEntry(config, "root", "File");

			if (!config.enter("root")) {
				LOG_WARN( ">>> Missing <root>" << std::endl );
				return;
			}

			VerifyNoAttributes(config, "in <root ...> node");

			// Check for unknown entries
			{
				std::vector<std::string> known = { "ops_config" };
				CheckForUnknown(config, known, "in <root> section");
			}

			verifyOnlyOneEntry(config, "ops_config", "<root>");

			if (!config.enter("ops_config")) {
				LOG_WARN( ">>> Missing <ops_config ...>" << std::endl );
				return;
			}
			if (config.getAttribute("type") != "DefaultOPSConfigImpl") {
				LOG_WARN( ">>> Unknown <ops_config type=" << config.getAttribute("type") << "> " << std::endl );
			}

			// Check for unknown attributes
			{
				std::vector<std::string> known = { "type" };
				CheckForUnknownAttributes(config, known, "in <ops_config ...> node");
			}
			// Check for unknown entries
			{
				std::vector<std::string> known = { "domains" };
				CheckForUnknown(config, known, "in <ops_config> section");
			}

			verifyOnlyOneEntry(config, "domains", "<ops_config>");

			if (!config.enter("domains")) {
				LOG_WARN( ">>> Missing <domains>" << std::endl );
				return;
			}

			VerifyNoAttributes(config, "in <domains ...> node");

			// Check for unknown entries
			{
				std::vector<std::string> known = { "element" };
				CheckForUnknown(config, known, "in <domains> section");
			}

			for (int iDomains = 0; iDomains < config.numEntries("element"); iDomains++) {
				if (config.enter("element", iDomains)) {
					iNumDomains++;
					VerifyDomain(config);
					config.exit();
				}
			}
		}
		catch (...)
		{
		}
		LOG_DEBUG("Total # domains: " << iNumDomains << std::endl);

		try {
			// We have checked the xml-file.
			// Now do further checks using the ops objects created from the xml-file
			ops::OPSConfig* cfg = ops::OPSConfig::getConfig(filename.c_str());
			if (cfg) {
				// Trick to get all topics fully initialized
				std::vector<ops::Domain*> domains = cfg->getRefToDomains();
				for (unsigned int i = 0; i < domains.size(); i++) {
					domains[i]->getTopics();
				}

				// 


				if (bDebug) {
					// Dump all domains and topics to standard out 
					ops::PrintArchiverOut prt(std::cout);
					prt.printObject("", cfg);
					std::cout << std::endl;
				}
			}
		}
		catch (std::exception& e) {
			LOG_WARN(">>> Exception: " << e.what() << std::endl);
		}
	}

	void verifyOnlyOneEntry(Configuration& config, std::string name, std::string parent)
	{
		if (config.numEntries(name) > 1) {
			LOG_WARN(">>> " + parent + " should only contain ONE <" + name + ">" << std::endl);
		}
	}

	void CheckForUnknown(Configuration& config, std::vector<std::string>& known, std::string msg)
	{
		// Check for unknown entries
		int num = config.numEntries();
		//std::cout << "Num entries=" << num << std::endl;
		for (int i = 0; i < num; i++) {
			std::string entryName = config.getNodeName(i);
			//std::cout << "Name " << i << " = " << entryName << std::endl;
			// if not a known name, log a warning
			if (!CheckExist(entryName, known)) {
				LOG_WARN( ">>> Unknown entry <" << entryName << "> found " << msg << std::endl );
			}
		}
	}

	void CheckForUnknownAttributes(Configuration& config, std::vector<std::string>& known, std::string msg)
	{
		// Check for unknown attributes
		int num = config.numAttributes();
		//std::cout << "Num attributes=" << num << std::endl;
		for (int i = 0; i < num; i++) {
			std::string attrName = config.getAttributeName(i);
			//std::cout << "Name " << i << " = " << attrName << std::endl;
			// if not a known name, log a warning
			if (!CheckExist(attrName, known)) {
				LOG_WARN(">>> Unknown attribute '" << attrName << "' used " << msg << std::endl);
			}
		}
	}

	void VerifyNoAttributes(Configuration& config, std::string msg)
	{
		std::vector<std::string> known = { "" };
		CheckForUnknownAttributes(config, known, msg);
	}

	bool CheckExist(std::string name, std::vector<std::string>& vect)
	{
		for (unsigned int i = 0; i < vect.size(); i++) {
			if (vect[i] == name) return true;
		}
		return false;
	}

	bool CheckDuplicate(std::string name, std::vector<std::string>& vect)
	{
		for (unsigned int i = 0; i < vect.size(); i++) {
			if (vect[i] == name) return true;
		}
		vect.push_back(name);
		return false;
	}

	bool EraseIfExist(std::string name, std::vector<std::string>& vect)
	{
		for (unsigned int i = 0; i < vect.size(); i++) {
			if (vect[i] == name) {
				vect.erase(vect.begin() + i);
				return true;
			}
		}
		return false;
	}

	void VerifyDomain(Configuration& config)
	{
		std::string domainName, s;
		int iNumTopics = 0;
		vChannels.clear();
		vTransportTopics.clear();
		bUdpRequireMetadata = false;

		//<element type = "MulticastDomain"> or <element type = "Domain">
        //    <domainID>SDSDomain</domainID>
        //    <domainAddress>239.7.1.6</domainAddress>
 	    //    <localInterface>127.0.0.1</localInterface>
        //    <topics>
        //        <element type = "Topic">
        //            <name>RecordingControlTopic</name>
        //            <port>7001</port>
        //            <dataType>sds.RecordingControlData</dataType>
        //        </element>

		// Check domainID first so we can use name in later logging
		domainName = config.getString("domainID");
		if (domainName == "") {
			LOG_WARN(">>> Missing <domainID> for a domain" << std::endl);
			domainName = "Missing <domainID>";
		} else {
			LOG_DEBUG("Domain: " << domainName << std::endl);
			// Check unique domain name
			if (CheckDuplicate(domainName, vDomains)) {
				LOG_WARN(">>> Duplicate domain detected. '" << domainName << "' already used." << std::endl);
			}
		}
		verifyOnlyOneEntry(config, "domainID", "<domains> <element> section for domainID '" + domainName + "'");

		std::string domainType = config.getAttribute("type");
		if ((domainType != "MulticastDomain") && (domainType != "Domain")) {
			LOG_WARN( ">>> Unknown <element type=" << domainType << "> " << std::endl );
		}

		// Check for unknown attributes
		{
			std::vector<std::string> known = { "type" };
			CheckForUnknownAttributes(config, known, "in <domains> <element> section for domainID '" + domainName + "'");
		}

		// Check for unknown entries
		{
			std::vector<std::string> known = { 
				"domainID", "domainAddress", "metaDataMcPort", 
				"localInterface", "timeToLive",
				"channels", "transports",
				"outSocketBufferSize", "inSocketBufferSize",
				"topics" 
			};
			CheckForUnknown(config, known, " in <domains> for domainID '" + domainName + "'");
		}

		// domainAddress
		verifyOnlyOneEntry(config, "domainAddress", "<domains> <element> section for domainID '" + domainName + "'");
		std::string domainAddress = config.getString("domainAddress");
		if (domainAddress == "") {
			LOG_WARN( ">>> Missing <domainAddress> for domain: " << domainName << std::endl );
		}

		// metaDataMcPort, optional but necessary if UDP transports are used.
		// If not set to 0, then DomainAddress::metaDataMcPort should be unique
		verifyOnlyOneEntry(config, "metaDataMcPort", "<domains> <element> section");
		std::string metaDataMcPort = config.getString("metaDataMcPort");
		LOG_DEBUG("metaDataMcPort: '" << metaDataMcPort << "', parsed as: " << config.parseInt(metaDataMcPort, 9494) << std::endl );
		if (config.parseInt(metaDataMcPort, 9494) != 0) {
			if (CheckDuplicate(domainAddress + "::" + metaDataMcPort, vDomainMeta)) {
				LOG_WARN(">>> Duplicate use of DomainAddress::metaDataMcPort for domain: " << domainName << std::endl);
			}
		}

		// localInterface, optional
		verifyOnlyOneEntry(config, "localInterface", "<domains> <element> section for domainID '" + domainName + "'");

		verifyOnlyOneEntry(config, "timeToLive", "<domains> <element> section for domainID '" + domainName + "'");

		verifyOnlyOneEntry(config, "outSocketBufferSize", "<domains> <element> section for domainID '" + domainName + "'");

		verifyOnlyOneEntry(config, "inSocketBufferSize", "<domains> <element> section for domainID '" + domainName + "'");

		// channels, optional
		verifyOnlyOneEntry(config, "channels", "<domains> <element> section for domainID '" + domainName + "'");
		if (config.enter("channels")) {
			VerifyNoAttributes(config, "in <channels ...> node for domain: " + domainName);

			// Check for unknown entries
			std::vector<std::string> known = { "element" };
			CheckForUnknown(config, known, "in <channels> node for domain: " + domainName);

			for (int iChannels = 0; iChannels < config.numEntries("element"); iChannels++) {
				if (config.enter("element", iChannels)) {
					VerifyChannel(config, domainName);
					config.exit();
				}
			}
			config.exit();
		}

		// transports, optional
		verifyOnlyOneEntry(config, "transports", "<domains> <element> section for domainID '" + domainName + "'");
		if (config.enter("transports")) {
			VerifyNoAttributes(config, "in <transports ...> node for domain: " + domainName);

			// Check for unknown entries
			std::vector<std::string> known = { "element" };
			CheckForUnknown(config, known, "in <transports> node for domain: " + domainName);

			for (int iTransports = 0; iTransports < config.numEntries("element"); iTransports++) {
				if (config.enter("element", iTransports)) {
					VerifyTransport(config, domainName);
					config.exit();
				}
			}
			config.exit();
		}

		// topics
		verifyOnlyOneEntry(config, "topics", "<domains> <element> section for domainID '" + domainName + "'");
		if (!config.enter("topics")) {
			LOG_WARN( ">>> Missing <topics> for domain: " << domainName << std::endl );

		} else {
			VerifyNoAttributes(config, "in <topics ...> node for domain: " + domainName);

			// Check for unknown entries
			std::vector<std::string> known = { "element" };
			CheckForUnknown(config, known, "in <topics> node for domain: " + domainName);

			vTopics.clear();
			for (int iTopics = 0; iTopics < config.numEntries("element"); iTopics++) {
				if (config.enter("element", iTopics)) {
					iNumTopics++;
					VerifyTopic(config, domainName);
					config.exit();
				}
			}
			config.exit();
		}

		// Check if udp is used which require a metaDataMcPort
		if (bUdpRequireMetadata && (config.parseInt(metaDataMcPort, 9494) == 0)) {
			LOG_WARN( ">>> UDP used as transport and '<metaDataMcPort>' set to 0. UDP requires metaDataMcPort != 0." << std::endl );
		}

		// Check that all transport topics have been found
		if (vTransportTopics.size() > 0) {
			for (unsigned int i = 0; i < vTransportTopics.size(); i++) {
				LOG_WARN( ">>> Missing topic definition in domain: " << domainName << " for topic: " << vTransportTopics[i] << std::endl );
			}
		}

		LOG_DEBUG( " # Topics: " << iNumTopics << std::endl );
	}

	void VerifyChannel(Configuration& config, std::string domainName)
	{
		std::string s;

		//must have a unique channel name. Check first to get a name to use in later logging
		std::string channelName = config.getString("name");
		if (channelName == "") {
			LOG_WARN( ">>> Missing <name> for a channel in domain: " << domainName << std::endl );
			channelName = "Missing <name>";
		} else {
			LOG_DEBUG( "Channel: " << channelName << std::endl );
			// Check unique channel name
			if (CheckDuplicate(channelName, vChannels)) {
				LOG_WARN( ">>> Duplicate channel name detected in domain '" << domainName << "'. Channel name '" << channelName << "' already used." << std::endl );
			}
		}
		verifyOnlyOneEntry(config, "name", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);

		// Check for unknown attributes
		{
			std::vector<std::string> known = { "type" };
			CheckForUnknownAttributes(config, known, "in <channels> <element ...> node for domain: " + domainName + " channel: " + channelName);
		}

		// Check for unknown entries
		{
			std::vector<std::string> known = { 
				"name", "linktype", "address", "localInterface", "port", "timeToLive",
				"outSocketBufferSize", "inSocketBufferSize"
			};
			CheckForUnknown(config, known, "in <channels> <element> section for domain: " + domainName + " channel: " + channelName);
		}

		verifyOnlyOneEntry(config, "linktype", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "address", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "port", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);

		std::string linkType = config.getString("linktype");
		std::string address = config.getString("address");
		std::string port = config.getString("port");

		// linktype
		if (linkType == "") {
			LOG_WARN( ">>> Missing <linktype> for channel '" << channelName << "' in domain '" << domainName << "', multicast assumed." << std::endl );
		}

		// address
		if ((address == "") && (linkType != "udp")) {
			LOG_WARN( ">>> Missing <address> for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl );
		}

		// port
		if (port == "") {
			if (linkType != "udp") {
				LOG_WARN(">>> Missing <port> for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
			}
		} else {
			if (config.parseInt(port, -1) > 65535) {
				LOG_WARN(">>> port > 65535 for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
			}
		}

		if (linkType == "udp") {
			if (address == "") {
				bUdpRequireMetadata = true;
				if (port != "") {
					LOG_WARN(">>> Superfluous <port> given for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
				}
			} else {
				if (port == "") {
					LOG_WARN(">>> Missing <port> for channel '" << channelName << "' in domain '" << domainName << "'. <port> is required if <address> specified" << std::endl);
				}
			}
		}

		///TODO can have local interface, buffer sizes
		verifyOnlyOneEntry(config, "localInterface", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "timeToLive", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "outSocketBufferSize", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "inSocketBufferSize", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
	}

	void VerifyTransport(Configuration& config, std::string domainName)
	{
		///must have an existing channel name. Check first to have a name for later logging
		std::string channelName = config.getString("channelID");
		if (channelName == "") {
			LOG_WARN( ">>> Missing <channelID> for a transport in domain: " << domainName << std::endl );
			channelName = "Missing <channelID>";
		} else {
			LOG_DEBUG( "  channelID: " << channelName << std::endl );
			if (!CheckExist(channelName, vChannels)) {
				LOG_WARN( ">>> Unknown channel name '" << channelName << "' used in transport for domain '" << domainName << "'." << std::endl );
			}
		}
		verifyOnlyOneEntry(config, "channelID", "<transports> <element> section in domain: " + domainName + " with channelID: " + channelName);

		// Check for unknown attributes
		{
			std::vector<std::string> known = { "type" };
			CheckForUnknownAttributes(config, known, "in <transports> <element ...> node for domain: " + domainName + " channel: " + channelName);
		}

		// Check for unknown entries
		{
			std::vector<std::string> known = { "channelID", "topics" };
			CheckForUnknown(config, known, "in <transports> <element ...> node for domain: " + domainName + " channel: " + channelName);
		}

		///can have zero or more topic names (save all topicnames and check that they are defined in topics section)
		///check that topic only exist in one transport
		verifyOnlyOneEntry(config, "topics", "<transports> <element> section in domain: " + domainName + " with channelID: " + channelName);
		if (!config.enter("topics")) {
			LOG_WARN( ">>> Missing <topics> for transport '" << channelName << "' in domain '" << domainName << "'" << std::endl );

		} else {
			// Check for unknown entries
			std::vector<std::string> known = { "element" };
			CheckForUnknown(config, known, "in <transports> <element> <topics> node for domain: " + domainName + "channel: " + channelName);

			for (int iTopics = 0; iTopics < config.numEntries("element"); iTopics++) {
				std::string topicName = config.getString("element", iTopics);
				LOG_DEBUG( "    topicName: " << topicName << std::endl );
				if (CheckDuplicate(topicName, vTransportTopics)) {
					LOG_WARN( ">>> Duplicate topic detected in transport for domain '" << domainName << "'. Topic name '" << topicName << "' already specified." << std::endl );
				}
			}
			config.exit();
		}
	}

	void VerifyTopic(Configuration& config, std::string domainName)
	{
		std::string topicName, dataType, s;
		uint64_t sampleMaxSize;

		//<element type = "Topic">
		//  <name>PingTopic</name>
	    //  <port>8003</port>
		//  <dataType>sds.TemplateMessageData_struct_sPing</dataType>
		//  <sampleMaxSize>10000000</sampleMaxSize>
		//  <inSocketBufferSize>32000000</inSocketBufferSize>
		//  <outSocketBufferSize>32000000</outSocketBufferSize>
		//</element>

		// Check name first to use in later logging
		topicName = config.getString("name");
		if (topicName == "") {
			LOG_WARN( ">>> Missing <name> for a topic in domain: " << domainName << std::endl );
			topicName = "Missing <name>";
		} else {
			LOG_DEBUG( "Checking topic: " << topicName << std::endl );
			// Check that name is unique in domain
			if (CheckDuplicate(topicName, vTopics)) {
				LOG_WARN( ">>> Duplicate Topic detected. Name '" << topicName << "' already used in domain: " << domainName << std::endl );
			}
		}
		verifyOnlyOneEntry(config, "name", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);

		// Check for unknown attributes
		{
			std::vector<std::string> known = { "type" };
			CheckForUnknownAttributes(config, known, "in <topics> <element ...> node for domain: " + domainName + " topic: " + topicName);
		}

		if (config.getAttribute("type") != "Topic") {
			LOG_WARN(">>> Unknown <element type=" << config.getAttribute("type") << std::endl);
		}

		// Check for unknown entries
		{
			std::vector<std::string> known = {
				"name", "dataType",
				"sampleMaxSize",
				"address", "transport", "port",
				"outSocketBufferSize", "inSocketBufferSize"
			};
			CheckForUnknown(config, known, "in <topics> for domain: " + domainName + " topic: " + topicName);
		}

		// dataType
		verifyOnlyOneEntry(config, "dataType", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		dataType = config.getString("dataType");
		if (dataType == "") {
			LOG_WARN(">>> Missing <dataType> for topic: " << topicName << ", in domain: " << domainName << std::endl);
		}

		// sampleMaxSize optional, required if sample size os > 60000
		verifyOnlyOneEntry(config, "sampleMaxSize", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		sampleMaxSize = config.parseInt64(config.getString("sampleMaxSize"), 60000);
		if (sampleMaxSize > 60000) {
			// For datatypes > 60000, we require different ports
			///TODO
		}

		//
		bool topicDefinedInTransports = EraseIfExist(topicName, vTransportTopics);

		verifyOnlyOneEntry(config, "transport", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "address", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "port", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);

		// transport optional
		std::string linkType = config.getString("transport");
		std::string address = config.getString("address");
		std::string port = config.getString("port");

		if (topicDefinedInTransports) {
			if (linkType != "") {
				LOG_WARN(">>> Superfluous <transport> for topic '" << topicName << "', in domain: '" << domainName << "', since topic listed in the domains transport section." << std::endl);
			}
			if (address != "") {
				LOG_WARN(">>> Superfluous <address> for topic '" << topicName << "', in domain: '" << domainName << "', since topic listed in the domains transport section." << std::endl);
			}
			if (port != "") {
				LOG_WARN(">>> Superfluous <port> for topic '" << topicName << "', in domain: '" << domainName << "', since topic listed in the domains transport section." << std::endl);
			}

		} else {
			// linktype
			if (linkType == "") {
				//LOG_WARN(">>> Missing <transport> for topic '" << topicName << "' in domain '" << domainName << "', multicast assumed." << std::endl);
			}

			// address
			if ((address == "") && (linkType == "tcp")) {
				LOG_WARN(">>> Missing <address> for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
			}

			// port
			if (port == "") {
				if (linkType != "udp") {
					LOG_WARN(">>> Missing <port> for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
				}
			} else {
				if (config.parseInt(port, -1) > 65535) {
					LOG_WARN(">>> port > 65535 for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
				}
			}

			if (linkType == "udp") {
				if (address == "") {
					bUdpRequireMetadata = true;
					if (port != "") {
						LOG_WARN(">>> Superfluous <port> for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
					}
				} else {
					if (port == "") {
						LOG_WARN(">>> Missing <port> for topic '" << topicName << "' in domain '" << domainName << "'. <port> is required if <address> specified" << std::endl);
					}
				}
			}
		}

		// inSocketBufferSize optional
		// outSocketBufferSize optional
		verifyOnlyOneEntry(config, "outSocketBufferSize", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "inSocketBufferSize", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
	}

};

void PrintDescription()
{
	std::cout << std::endl;
	std::cout << "  " << sVersion << std::endl;
	std::cout << "  " << std::endl;
	std::cout << "  Program verifies that: " << std::endl;
	std::cout << "    - XML syntax is OK for OPS usage" << std::endl;
	std::cout << "    - OPS required entries exist" << std::endl;
	std::cout << "    - Domain names are unique" << std::endl;
	std::cout << "    - Channel names are unique within the given domain" << std::endl;
	std::cout << "    - Topic names are unique within the given domain" << std::endl;
	std::cout << "  " << std::endl;
	std::cout << "  Program also checks for some common mistakes and miss-configurations." << std::endl;
	std::cout << std::endl;
}

void Usage()
{
	std::cout << std::endl;
	std::cout << "  Usage:" << std::endl;
	std::cout << "    VerifyOPSConfig [-?] [-debug] filename " << std::endl;
	std::cout << std::endl;
	std::cout << "    -?          Help" << std::endl;
	std::cout << "    -debug      Print some debug info during work" << std::endl;
}

int main(int argc, char* argv[])
{
	std::string infile = "";
	bool printUsage = false;
	bool printDescription = false;
	bool debug = false;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "?") {
			printUsage = true;
			printDescription = true;

		} else if (arg == "-?") {
			printUsage = true;
			printDescription = true;

		} else if (arg == "-debug") {
			debug = true;

		} else {
			infile = arg;
			break;
		}
	}

	if (printUsage || (infile == "")) {
		Usage();
		if (printDescription) {
			PrintDescription();
		}

	} else {
		std::cout << std::endl;

		CVerifyOPSConfig verify(infile, debug);

		if (!gWarningGiven) std::cout << "Check OK  " << std::endl;

		std::cout << std::endl;
	}

	return 0;
}

