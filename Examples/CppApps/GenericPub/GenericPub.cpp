
#include <iostream>
#include <chrono>
#include <ops.h>
#include <OPSConfigRepository.h>
#include <PrintArchiverOut.h>
#include <OPSObject.h>
#include <Publisher.h>

// ================================================================================

//#include "TestAll/TestAllTypeFactory.h"
#include "PizzaProject/PizzaProjectTypeFactory.h"

ops::SerializableFactory* createFactory()
{
	//return new TestAll::TestAllTypeFactory();
	return new PizzaProject::PizzaProjectTypeFactory();
}

std::string FactoryDescription("PizzaProject");

// ================================================================================

std::string gDomainName = "";
std::string gTopicName = "";
std::string gDataType = "";
std::string gPubName = "";
std::string gKey = "";
int gSpareSize = -1;
bool gVerbose = false;

void Usage()
{
	std::cout << std::endl;
	std::cout << "Version 2024-12-18" << std::endl;
	std::cout << std::endl;
	std::cout << "  Usage:" << std::endl;
	std::cout << "    GenericPub [-?] [<options>] -list -c file [-d domain] [-t topic] " << std::endl;
	std::cout << "    GenericPub [-?] [<options>] -c file -d domain -t topic" << std::endl;
	std::cout << std::endl;
	std::cout << "      -? | -h | --help  Help" << std::endl;
	std::cout << "      -list             List information" << std::endl;
	std::cout << "      -c file           ops_config.xml file to use (default ops_config.xml in CWD)" << std::endl;
	std::cout << "      -d domain         Domain name to use [" << gDomainName << "]" << std::endl;
	std::cout << "      -t name           Topic name to use [" << gTopicName << "]" << std::endl;
	std::cout << std::endl;
	std::cout << "    options:" << std::endl;
	std::cout << "      -k key            Publisher key to use [" << gKey << "]" << std::endl;
	std::cout << "      -p name           Publisher name to use [" << gPubName << "]" << std::endl;
	std::cout << "      -S <num>          If <num> > 0, publish an OPSObject with <num> sparebytes (advanced) [" << gSpareSize << "]" << std::endl;
	std::cout << "      -T <datatype>     Use <datatype> instead of Topic datatype (advanced)" << std::endl;
	std::cout << "      -v                Verbose" << std::endl;
	std::cout << std::endl;
	std::cout << "  Compiled with Factory: " << FactoryDescription << std::endl;
	std::cout << std::endl;
}

struct DummyObject : public ops::OPSObject
{
	static ops::TypeId_T fakeTypeName;
	static ops::TypeId_T getTypeName() { return fakeTypeName; }
	DummyObject()
	{
		OPSObject::appendType(fakeTypeName);
	}
};
ops::TypeId_T DummyObject::fakeTypeName;

int main(const int argc, const char* argv[])
{
	bool printUsage = false;
	bool dolist = false;
	std::string cfgfile;

	// Helper variables
	std::string* strp = nullptr;
	int* intp = nullptr;
	int64_t* int64p = nullptr;

	if (argc <= 1) { printUsage = true; }

	for (int i = 1; i < argc; i++) {
		std::string const arg = argv[i];
		if (arg == "?") {
			printUsage = true;

		}
		else if ((arg == "-?") || (arg == "-h") || (arg == "--help")) {
			printUsage = true;

		}
		else if (arg == "-c") {
			strp = &cfgfile;

		}
		else if (arg == "-d") {
			strp = &gDomainName;

		}
		else if (arg == "-k") {
			strp = &gKey;

		}
		else if (arg == "-list") {
			dolist = true;

		}
		else if (arg == "-p") {
			strp = &gPubName;

		}
		else if (arg == "-t") {
			strp = &gTopicName;

		}
		else if (arg == "-S") {
			intp = &gSpareSize;

		}
		else if (arg == "-T") {
			strp = &gDataType;

		}
		else if (arg == "-v") {
			gVerbose = true;

		}

		if ((strp != nullptr) && ((i + 1) < argc)) {
			i++;
			*strp = argv[i];
			strp = nullptr;
		}
		if ((intp != nullptr) && ((i + 1) < argc)) {
			i++;
			*intp = atoi(argv[i]);
			intp = nullptr;
		}
		if ((int64p != nullptr) && ((i + 1) < argc)) {
			i++;
			*int64p = atoll(argv[i]);
			int64p = nullptr;
		}
	}

	if (printUsage) {
		Usage();
		return 0;
	}

	// Add an errorwriter instance to OPS to catch ev. internal OPS errors
	ops::ErrorWriter* const errorWriter = new ops::ErrorWriter(std::cout);
	ops::Participant::getStaticErrorService()->addListener(errorWriter);

	ops::OPSConfigRepository::Instance()->Add(cfgfile);

	// --------------  L i s t   F u n c t i o n a l i t y  -----------------------

	if (dolist) {
		std::shared_ptr<ops::OPSConfig> cfg = ops::OPSConfigRepository::Instance()->getConfig();
		std::vector<ops::Domain*> doms = cfg.get()->getRefToDomains();
		if (gDomainName == "") {
			std::cout << "Available Domains:" << std::endl;
			for (auto& dom : doms) {
				std::cout << "  " << dom->getDomainID() << std::endl;
			}
		}
		else if (gTopicName == "") {
			ops::Domain* dom = cfg->getDomain(gDomainName);
			if (dom == nullptr) {
				std::cerr << "Error: Unknown Domain name: '" << gDomainName << "'" << std::endl;
			}
			else {
				std::vector<ops::Topic*> tops = dom->getTopics();
				std::cout << "Available Topics in given Domain:" << std::endl;
				for (auto& top : tops) {
					std::cout << "  " << top->getName() << std::endl;
				}
			}
		}
		else {
			ops::Domain* dom = cfg->getDomain(gDomainName);
			if (dom == nullptr) {
				std::cerr << "Error: Unknown Domain name: '" << gDomainName << "'" << std::endl;
			} else if (!dom->existsTopic(gTopicName)) {
				std::cerr << "Error: Unknown Topic name: '" << gTopicName << "'" << std::endl;
			}
			else {
				ops::Topic top = dom->getTopic(gTopicName);
				ops::PrintArchiverOut prt(std::cout);
				prt.printObject("topic", &top);
			}
		}
		return 0;
	}

	// --------------  P u b l i s h   F u n c t i o n a l i t y  -----------------------

	// Create OPS participant
	ops::Participant* const participant = ops::Participant::getInstance(gDomainName);
	if (participant == nullptr) {
		std::cerr << "Error: Unknown Domain name: '" << gDomainName << "'" << std::endl;
		return -1;
	}

	try {
		// Create the topic to use, might throw ops::NoSuchTopicException
		// The topic must exist in the used ops configuration file
		const ops::Topic topic = participant->createTopic(gTopicName);

		// Get the typeid of the topic as described in ops_config file
		ops::TypeId_T tid = topic.getTypeID();
		if (gVerbose) {
			std::cout << "Topic TypeID: " << tid << std::endl;
		}
		if (gDataType != "") {
			std::cout << "Replacing Topic TypeID: '" << tid << "' with '" << gDataType << "'" << std::endl;
			tid = gDataType;
		}

		ops::SerializableFactory* fact = createFactory();

		ops::OPSObject* data = nullptr;

		if (gSpareSize < 0) {
			// Use the factory to create an object instance of the typeid
			data = dynamic_cast<ops::OPSObject*>(fact->create(tid));
		}
		else {
			DummyObject::fakeTypeName = tid;
			data = new DummyObject();
			if (gSpareSize > 0) {
				for (int i = 0; i < gSpareSize; i++) {
					data->spareBytes.push_back(0);
				}
			}
		}

		if (data != nullptr) {
			std::cout << "Publishing on " << topic.getName() <<
				" [" << topic.getTransport() <<
				"::" << topic.getDomainAddress() <<
				"::" << topic.getPort() <<
				"] " << std::endl;

			ops::Publisher pub(topic);
			pub.setName(gPubName);
			pub.setKey(gKey);

			while (true) {
				std::cout << "Writing '" << topic.getName() << "' using type '" << data->getTypeString() << "'" << std::endl;
				pub.writeOPSObject(data);

				ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
			}
		}
		else {
			std::cerr << "Error: Failed to create object for '" << gTopicName << "'" << std::endl;
			return -3;
		}
	}
	catch (...) {
		std::cerr << "Error: Failed to create topic '" << gTopicName << "'" << std::endl;
		return -2;
	}
}

