// VerifyOPSConfig.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include <vector>
#include <memory>

#include "OPSConfig.h"
#include "PrintArchiverOut.h"
#include "NetworkSupport.h"
#include "IOService.h"

#include "Configuration.h"

const std::string sVersion = "Version 2020-07-14";


bool gErrorGiven = false;
bool gWarningGiven = false;

#define LOG_ERROR(message) { std::cout << message; gErrorGiven = true; } 
#define LOG_WARN(message) { std::cout << message; gWarningGiven = true; } 
#define LOG_INFO(message) { std::cout << "Info: " << message; }
#define LOG_DEBUG(message) { if (bDebug) {std::cout << message;} } 

class CVerifyOPSConfig 
{
public:
	CVerifyOPSConfig() = delete;
	~CVerifyOPSConfig() = default;

	CVerifyOPSConfig(CVerifyOPSConfig const&) = delete;
	CVerifyOPSConfig(CVerifyOPSConfig&&) = delete;
	CVerifyOPSConfig& operator =(CVerifyOPSConfig&&) = delete;
	CVerifyOPSConfig& operator =(CVerifyOPSConfig const&) = delete;

	CVerifyOPSConfig(std::string filename, bool const debug):
	    bDebug(debug),
		iNumDomains(0),
		bUdpRequireMetadata(false),
		bTcpRequireMetadata(false)
	{
		try {
			std::string s;
			Configuration config(filename);

			LOG_DEBUG( std::endl << "Checking file: " << filename << std::endl << std::endl );

			// Some error control. 
			s = config.getParseResult();
			if (s != "") {
				LOG_ERROR( ">>> Error parsing file, Result: " << s << std::endl );
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
				LOG_ERROR( ">>> Missing <root>" << std::endl );
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
				LOG_ERROR( ">>> Missing <ops_config ...>" << std::endl );
				return;
			}
			if (config.getAttribute("type") != "DefaultOPSConfigImpl") {
				LOG_ERROR( ">>> Unknown <ops_config type=" << config.getAttribute("type") << "> " << std::endl );
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
				LOG_ERROR( ">>> Missing <domains>" << std::endl );
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
		catch (std::exception& e)
		{
			LOG_ERROR( ">>> ERROR: Exception: " << e.what() << std::endl );
		}
		LOG_DEBUG("Total # domains: " << iNumDomains << std::endl);

		try {
			// We have checked the xml-file.
			// Now do further checks using the ops objects created from the xml-file
			std::shared_ptr<ops::OPSConfig> const cfg = ops::OPSConfig::getConfig(filename.c_str());
			if (cfg != nullptr) {
				// Trick to get all topics fully initialized
				std::vector<ops::Domain*> domains = cfg->getRefToDomains();
				for (unsigned int i = 0; i < domains.size(); i++) {
					domains[i]->getTopics();
				}

				// 


				if (bDebug) {
					// Dump all domains and topics to standard out 
					ops::PrintArchiverOut prt(std::cout);
					prt.printObject("", cfg.get());
					std::cout << std::endl;
				}
			}
		}
		catch (std::exception& e) {
			LOG_WARN(">>> Exception: " << e.what() << std::endl);
		}
	}

	void verifyOnlyOneEntry(Configuration& config, std::string const name, std::string const parent) const 
	{
		if (config.numEntries(name) > 1) {
			LOG_WARN(">>> " + parent + " should only contain ONE <" + name + ">" << std::endl);
		}
	}

	void CheckForUnknown(Configuration& config, std::vector<std::string>& known, std::string const msg) const
	{
		// Check for unknown entries
		int const num = config.numEntries();
		//std::cout << "Num entries=" << num << std::endl;
		for (int i = 0; i < num; i++) {
			std::string const entryName = config.getNodeName(i);
			//std::cout << "Name " << i << " = " << entryName << std::endl;
			// if not a known name, log a warning
			if (!CheckExist(entryName, known)) {
				LOG_WARN( ">>> Unknown entry <" << entryName << "> found " << msg << std::endl );
			}
		}
	}

	void CheckForUnknownAttributes(Configuration& config, std::vector<std::string>& known, std::string const msg) const
	{
		// Check for unknown attributes
		int const num = config.numAttributes();
		//std::cout << "Num attributes=" << num << std::endl;
		for (int i = 0; i < num; i++) {
			std::string const attrName = config.getAttributeName(i);
			//std::cout << "Name " << i << " = " << attrName << std::endl;
			// if not a known name, log a warning
			if (!CheckExist(attrName, known)) {
				LOG_WARN(">>> Unknown attribute '" << attrName << "' used " << msg << std::endl);
			}
		}
	}

	void VerifyNoAttributes(Configuration& config, std::string const msg) const
	{
		std::vector<std::string> known = { "" };
		CheckForUnknownAttributes(config, known, msg);
	}

	void verifyMCAddress(std::string const address, std::string const msg) const
	{
		// check that domainaddress is a MC address
		if (!ops::isValidMCAddress(address)) {
			LOG_ERROR(">>> Invalid MC address '" << address << "' detected " << msg << std::endl);
		}
	}

	void verifyValidAddress(std::string const address, std::string const linkType, std::string const msg) const
	{
		if (address == "") { return; }
		if ((linkType == "multicast") || (linkType == "")) {
			verifyMCAddress(address, msg);
		}
		else if ((linkType == "udp") || (linkType == "tcp")) {
			// subscriber address or tcp server address
			if (!ops::isValidNodeAddress(address)) {
				LOG_ERROR(">>> Invalid Node address '" << address << "' detected " << msg << std::endl);
			}
		}
	}


	void verifyLocalInterface(std::string localIf, std::string const msg) const
	{
		std::unique_ptr<ops::IOService> ioServ(ops::IOService::create());
		ops::Address_T const subnetIf = ops::doSubnetTranslation(localIf, ioServ.get());
		LOG_INFO(msg + ": localInterface " << localIf << " --> " << subnetIf << std::endl);
		if (!ops::isValidNodeAddress(subnetIf)) {
			LOG_ERROR(">>> Invalid Node address '" << subnetIf << "' extracted from localInterface '" << localIf << "'" << std::endl);
		}
	}

	bool CheckExist(std::string const name, std::vector<std::string>& vect) const
	{
		for (unsigned int i = 0; i < vect.size(); i++) {
			if (vect[i] == name) { return true; }
		}
		return false;
	}

	bool CheckDuplicate(std::string const name, std::vector<std::string>& vect) const
	{
		for (unsigned int i = 0; i < vect.size(); i++) {
			if (vect[i] == name) { return true; }
		}
		vect.push_back(name);
		return false;
	}

	bool EraseIfExist(std::string const name, std::vector<std::string>& vect) const
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
		std::string domainName;
		int iNumTopics = 0;
		vChannels.clear();
		vTransportTopics.clear();
		bUdpRequireMetadata = false;
		bTcpRequireMetadata = false;

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

		std::string const domainType = config.getAttribute("type");
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
				"domainID", "domainAddress", "metaDataMcPort", "debugMcPort", 
				"localInterface", "timeToLive",
				"channels", "transports",
				"outSocketBufferSize", "inSocketBufferSize",
				"topics", "optNonVirt", "heartbeatPeriod", "heartbeatTimeout"
			};
			CheckForUnknown(config, known, " in <domains> for domainID '" + domainName + "'");
		}

		// domainAddress
		verifyOnlyOneEntry(config, "domainAddress", "<domains> <element> section for domainID '" + domainName + "'");
		std::string const domainAddress = config.getString("domainAddress");
		if (domainAddress == "") {
			LOG_WARN( ">>> Missing <domainAddress> for domain: " << domainName << std::endl );
		}

		// check that domainaddress is a MC address
		verifyMCAddress(domainAddress, "for domain: " + domainName);

		// metaDataMcPort, optional but necessary if UDP transports are used.
		// If not set to 0, then DomainAddress::metaDataMcPort should be unique
		verifyOnlyOneEntry(config, "metaDataMcPort", "<domains> <element> section");
		std::string const metaDataMcPort = config.getString("metaDataMcPort");
		LOG_DEBUG("metaDataMcPort: '" << metaDataMcPort << "', parsed as: " << config.parseInt(metaDataMcPort, 9494) << std::endl );
		if (config.parseInt(metaDataMcPort, 9494) != 0) {
			if (CheckDuplicate(domainAddress + "::" + metaDataMcPort, vDomainMeta)) {
				LOG_WARN(">>> Duplicate use of DomainAddress::metaDataMcPort for domain: " << domainName << std::endl);
			}
		}

		// debugMcPort, optional
		// If not set to 0, then DomainAddress::debugMcPort should be unique
		verifyOnlyOneEntry(config, "debugMcPort", "<domains> <element> section");
		std::string const debugMcPort = config.getString("debugMcPort");
		LOG_DEBUG("debugMcPort: '" << debugMcPort << "', parsed as: " << config.parseInt(debugMcPort, 0) << std::endl);
		if (config.parseInt(debugMcPort, 0) != 0) {
			if (CheckDuplicate(domainAddress + "::" + debugMcPort, vDomainMeta)) {
				LOG_WARN(">>> Duplicate use of DomainAddress::debugMcPort for domain: " << domainName << std::endl);
			}
		}

		// localInterface, optional
		verifyOnlyOneEntry(config, "localInterface", "<domains> <element> section for domainID '" + domainName + "'");
		std::string const localIf = config.getString("localInterface");
		if (localIf.size() > 0) {
			verifyLocalInterface(localIf, "Domain '" + domainName + "'");
		}

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
		if (bTcpRequireMetadata && (config.parseInt(metaDataMcPort, 9494) == 0)) {
			LOG_WARN(">>> TCP used as transport and '<metaDataMcPort>' set to 0. TCP requires metaDataMcPort != 0." << std::endl);
		}

		// Check that all transport topics have been found
		if (vTransportTopics.size() > 0) {
			for (unsigned int i = 0; i < vTransportTopics.size(); i++) {
				LOG_WARN( ">>> Missing topic definition in domain: " << domainName << " for topic: " << vTransportTopics[i] << std::endl );
			}
		}

		LOG_DEBUG( " # Topics: " << iNumTopics << std::endl );
	}

	void VerifyChannel(Configuration& config, std::string const domainName)
	{
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
				"outSocketBufferSize", "inSocketBufferSize", "sampleMaxSize"
			};
			CheckForUnknown(config, known, "in <channels> <element> section for domain: " + domainName + " channel: " + channelName);
		}

		verifyOnlyOneEntry(config, "linktype", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "address", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "port", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);

		std::string const linkType = config.getString("linktype");
		std::string const address = config.getString("address");
		std::string const port = config.getString("port");

		// linktype
		if (linkType == "") {
			LOG_WARN( ">>> Missing <linktype> for channel '" << channelName << "' in domain '" << domainName << "', multicast assumed." << std::endl );
		}

		// address & port
		if (port == "") {
			if ((linkType != "udp") && (linkType != "tcp")) {
				LOG_WARN(">>> Missing <port> for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
			}
		} else {
			if (config.parseInt(port, 65536u) > 65535u) {
				LOG_WARN(">>> port > 65535 for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
			}
		}

		if (linkType == "tcp") {
			if (address == "") {
				bTcpRequireMetadata = true;
				if (port != "") {
					LOG_WARN(">>> Superfluous <port> given for channel '" << channelName << "' in domain '" << domainName << "'" << std::endl);
				}
			} else {
				if (port == "") {
					LOG_WARN(">>> Missing <port> for channel '" << channelName << "' in domain '" << domainName << "'. <port> is required if <address> specified" << std::endl);
				}
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
		verifyValidAddress(address, linkType, "for channel '" + channelName + "' in domain '" + domainName + "'");

		verifyOnlyOneEntry(config, "localInterface", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		std::string const localIf = config.getString("localInterface");
		if (localIf.size() > 0) {
			verifyLocalInterface(localIf, "Domain '" + domainName + "', Channel '" + channelName + "'");
		}

		verifyOnlyOneEntry(config, "timeToLive", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "outSocketBufferSize", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
		verifyOnlyOneEntry(config, "inSocketBufferSize", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);

        verifyOnlyOneEntry(config, "sampleMaxSize", "<channels> <element> section in domain: " + domainName + " with name: " + channelName);
        uint64_t sampleMaxSize = config.parseInt64(config.getString("sampleMaxSize"), 0);
        if (sampleMaxSize > 60000) {
            // For datatypes > 60000, we require different ports, so the corresponding transport section should only contain 1 topic
            ///TODO
        }

	}

	void VerifyTransport(Configuration& config, std::string const domainName)
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
				std::string const topicName = config.getString("element", iTopics);
				LOG_DEBUG( "    topicName: " << topicName << std::endl );
				if (CheckDuplicate(topicName, vTransportTopics)) {
					LOG_WARN( ">>> Duplicate topic detected in transport for domain '" << domainName << "'. Topic name '" << topicName << "' already specified." << std::endl );
				}
			}
			config.exit();
		}
	}

	void VerifyTopic(Configuration& config, std::string const domainName)
	{
		std::string topicName, dataType;
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
		bool const topicDefinedInTransports = EraseIfExist(topicName, vTransportTopics);

		verifyOnlyOneEntry(config, "transport", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "address", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "port", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);

		// transport optional
		std::string const linkType = config.getString("transport");
		std::string const address = config.getString("address");
		std::string const port = config.getString("port");

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
				//LOG_WARN(">>> Missing <linkType> for topic '" << topicName << "' in domain '" << domainName << "', multicast assumed." << std::endl)
			}

			// address & port
			if (port == "") {
				if ((linkType != "udp") && (linkType != "tcp")) {
					LOG_WARN(">>> Missing <port> for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
				}
			} else {
				if (config.parseInt(port, 65536u) > 65535u) {
					LOG_WARN(">>> port > 65535 for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
				}
			}

			if (linkType == "tcp") {
				if (address == "") {
					bTcpRequireMetadata = true;
					if (port != "") {
						LOG_WARN(">>> Superfluous <port> for topic '" << topicName << "' in domain '" << domainName << "'" << std::endl);
					}
				} else {
					if (port == "") {
						LOG_WARN(">>> Missing <port> for topic '" << topicName << "' in domain '" << domainName << "'. <port> is required if <address> specified" << std::endl);
					}
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
		verifyValidAddress(address, linkType, "for topic '" + topicName + "' in domain '" + domainName + "'");

		// inSocketBufferSize optional
		// outSocketBufferSize optional
		verifyOnlyOneEntry(config, "outSocketBufferSize", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
		verifyOnlyOneEntry(config, "inSocketBufferSize", "<topics> <element> section in domain: " + domainName + " topic: " + topicName);
	}

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
	bool bTcpRequireMetadata;

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

int main(const int argc, const char* argv[])
{
	std::string infile = "";
	bool printUsage = false;
	bool printDescription = false;
	bool debug = false;

	for (int i = 1; i < argc; i++) {
		std::string const arg = argv[i];
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

		CVerifyOPSConfig const verify(infile, debug);

		if ((!gErrorGiven) && (!gWarningGiven)) { std::cout << "Check OK  " << std::endl; }

		std::cout << std::endl;
	}

	if (gErrorGiven) { return 10; }
	if (gWarningGiven) { return 1; }
	return 0;
}

