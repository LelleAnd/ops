// DebugConsole.cpp : Defines the entry point for the console application.
//

#include <iomanip>

#include "DataListener.h"
#include "Participant.h"
#include "ErrorWriter.h"
#include "OPSConfigRepository.h"
#include "TimeHelper.h"
#include "PrintArchiverOut.h"

#include "opsidls/DebugRequestResponseDataPublisher.h"
#include "opsidls/DebugRequestResponseDataSubscriber.h"
#include "opsidls/opsidlsTypeFactory.h"

#ifdef USE_PIZZATEST
#include "pizza\PizzaData.h"
pizza::PizzaData testData;
#endif

opsidls::DebugRequestResponseData request;
bool gverbose = false;

class DebugListener : public ops::DataListener
{
public:
	DebugListener(ops::Participant* const part) : _part(part), _sub(nullptr) {}
	~DebugListener() { removeSubscriber(); }

	DebugListener() = default;
	DebugListener(DebugListener const&) = delete;
	DebugListener(DebugListener&&) = delete;
	DebugListener& operator =(DebugListener&&) = delete;
	DebugListener& operator =(DebugListener const&) = delete;

	std::vector<std::string> _topics;

	void Start()
	{
		setupSubscriber();
	}

	void Stop()
	{
		removeSubscriber();
	}

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373 )
#endif

	virtual void onNewData(ops::DataNotifier* const notifier) override
	{
		ops::Subscriber* const sub = dynamic_cast<ops::Subscriber*>(notifier);
		if (sub != nullptr) {
			opsidls::DebugRequestResponseData* const data = dynamic_cast<opsidls::DebugRequestResponseData*>(sub->getMessage()->getData());
			if (data != nullptr) {
				// Skip All requests
				if (data->Command != 0) { return; }

				// Only responses come here

				ops::PrintArchiverOut prt(std::cout, 1);

				switch (data->Entity) {
				case 0: // Debug
					if ((data->Result1 >= 1) && (data->Result1 <= 3)) {
						switch (data->Result1) {
						case 1: std::cout << "Instance key:" << std::endl; break;
						case 2:	std::cout << "Publisher Topics (key: " << data->getKey() << "):" << std::endl; break;
						case 3:	std::cout << "Subscriber Topics (key: " << data->getKey() << "):" << std::endl; break;
						default:; break;
						}
						_topics = data->Param3;
						for (unsigned int i = 0; i < data->Param3.size(); i++) {
							std::cout << "    " << data->Param3[i] << std::endl;
						}
						std::cout << std::endl;
					}
					if (data->Result1 == 50) {
						std::cout << "Response on Command 50:" << std::endl;
						data->serialize(&prt);
					}
					break;

				case 1: // Participant
					break;

				case 2: // Publisher
					std::cout << "\n(key: " << data->getKey() <<
						") Publisher for Topic: " << data->Name <<
						", Enabled: " << data->Enabled <<
						", PubId: " << data->Result1 <<
						"\n";
					if (gverbose) { data->serialize(&prt); }
					break;

				case 3: // Subscriber
					std::cout << "\n(key: " << data->getKey() <<
						") Subscriber for Topic: " << data->Name <<
						", Enabled: " << data->Enabled <<
						", Num rcv: " << data->Result1 <<
						"\n";
					if (gverbose) { data->serialize(&prt); }
					break;

				default:
					break;
				}

			} else {
				std::cout << "DebugListener::onNewData(), Data could not be cast as expected." << std::endl;
			}
		} else {
			std::cout << "DebugListener::onNewData(), Subscriber could not be cast as expected." << std::endl;
		}
	}

private:
	bool setupSubscriber()
	{
#ifdef OPS_ENABLE_DEBUG_HANDLER
        _sub = new ops::Subscriber(_part->createDebugTopic());
		_sub->addDataListener(this);
		_sub->start();
		return true;
#else
        return false;
#endif
    }

	void removeSubscriber()
	{
		if (_sub != nullptr) { delete _sub; }
		_sub = nullptr;
	}

	ops::Participant* _part = nullptr;
	ops::Subscriber* _sub = nullptr;

//	opsidls::DebugRequestResponseData _request;
};

void ListDomains()
{
	std::shared_ptr<ops::OPSConfig> cfg = ops::OPSConfigRepository::Instance()->getConfig();
	if (cfg == nullptr) { return; }

	std::vector<ops::Domain*>& domains = cfg->getRefToDomains();

	std::cout << "\n        Domains with Debug enabled\n\n";
	std::cout << "   " << std::setw(20) << "Domain ID" << "   " << std::setw(20) << "Local Interface" << 
		"   " << std::setw(20) << "Domain Address" << "   " << std::setw(8) << "Dbg Port" << 
		"\n";
	std::cout << "   " << std::setw(20) << "---------" << "   " << std::setw(20) << "---------------" << 
		"   " << std::setw(20) << "--------------" << "   " << std::setw(8) << "--------" <<
		"\n";
	for (const auto& d : domains) {
		if (d->getDebugMcPort() != 0) {
			std::cout << 
				"   " << std::setw(20) << d->getDomainID() << 
				"   " << std::setw(20) << d->getLocalInterface() << 
				"   " << std::setw(20) << d->getDomainAddress() <<
				"   " << std::setw(8)  << d->getDebugMcPort() <<
				"\n";
		}
	}
}

void ForAllTopics(DebugListener& listener, opsidls::DebugRequestResponseDataPublisher& pub)
{
	for (const auto n : listener._topics) {
		request.Name = n;
		pub.write(request);
		ops::TimeHelper::sleep(std::chrono::milliseconds(20));
	}
}

void CommandLoop(const ops::Participant* const part)
{
	request.setKey("Pizza");
	request.Entity = 2;	// Publisher
	request.Name = "PizzaTopic";
	request.Command = 1;
	request.Param1 = 1;

#ifdef OPS_ENABLE_DEBUG_HANDLER
    ops::Topic const top = part->createDebugTopic();
	opsidls::DebugRequestResponseDataPublisher pub(top);
	pub.start();
	pub.write(request);
#endif

	ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
}

void Usage()
{
	std::cout << std::endl;
	std::cout << "Version 2020-11-28" << std::endl;
	std::cout << std::endl;
	std::cout << "  Usage:" << std::endl;
	std::cout << "    DebugConsole [-?][-v] [-cfg file] -list" << std::endl;
	std::cout << std::endl;
	std::cout << "      -? | -h         Help" << std::endl;
	std::cout << "      -v              Verbose" << std::endl;
	std::cout << "      -cfg file       ops_config.xml file to use (default ops_config.xml in CWD)" << std::endl;
	std::cout << "      -list           lists all OPS Domains in given config file with debugging enabled" << std::endl;
	std::cout << std::endl;
	std::cout << "    DebugConsole -cfg file -d domain <command> [-k key]" << std::endl;
	std::cout << std::endl;
	std::cout << "      -d domain       OPS Domain to use" << std::endl;
	std::cout << "      -lk             List all instance keys" << std::endl;
	std::cout << "      -lp             List all publishers (*)" << std::endl;
	std::cout << "      -ls             List all subscribers (*)" << std::endl;
	std::cout << "                        * can be limited by giving a key (-k)" << std::endl;
	std::cout << std::endl;
	std::cout << "    DebugConsole -cfg file -d domain -k key <command>" << std::endl;
	std::cout << std::endl;
	std::cout << "      -k key          Key to set in sent debug message" << std::endl;
	std::cout << "      -lpa            List info from all publisher topics" << std::endl;
	std::cout << "      -lpi name       List info from publisher topic 'name'" << std::endl;
	std::cout << "      -lsa            List info from all subscriber topics" << std::endl;
	std::cout << "      -lsi name       List info from subscriber topic 'name'" << std::endl;
	std::cout << std::endl;
	std::cout << "    DebugConsole -cfg file -d domain -k key -e n -n name -c cmd -p1 num" << std::endl;
	std::cout << std::endl;
	std::cout << "      -e type         Entity type (2 = Publisher, 3 = Subscriber)" << std::endl;
	std::cout << "      -n name         Entity name (for pub/sub the topic name)" << std::endl;
	std::cout << "      -c cmd          Command to send to entity" << std::endl;
	std::cout << "      -p1 num         Value for parameter 1" << std::endl;
	std::cout << std::endl;
}


int main(const int argc, const char* argv[])
{
	// Default values
	bool printUsage = false;
	bool listDomains = false;
	bool allpubtopics = false;
	bool allsubtopics = false;
	std::string cfgFile("ops_config.xml");
	std::string domain = "";
	std::string key = "*";

#ifndef OPS_ENABLE_DEBUG_HANDLER
    std::cout << "\n DebugConsole not working since OPS compiled without debug support (#define OPS_ENABLE_DEBUG_HANDLER)\n\n";
    return 1;
#endif

    // Helper variables
	std::string* strp = nullptr;
	int* intp = nullptr;
	int64_t* int64p = nullptr;

	if (argc <= 1) { printUsage = true; }

	for (int i = 1; i < argc; i++) {
		std::string const arg = argv[i];
		if (arg == "?") {
			printUsage = true;

		} else if ((arg == "-?") || (arg == "-h") || (arg == "--help")) {
			printUsage = true;

		} else if (arg == "-c") {
			intp = &request.Command;

		} else if (arg == "-cfg") {
			strp = &cfgFile;

		} else if (arg == "-d") {
			strp = &domain;

		} else if (arg == "-e") {
			intp = &request.Entity;

		} else if (arg == "-k") {
			strp = &key;

		} else if (arg == "-list") {
			listDomains = true;

		} else if (arg == "-lk") {
			request.Entity = 0;
			request.Command = 2;
			request.Param1 = 1;

		} else if (arg == "-lp") {
			request.Entity = 0;
			request.Command = 2;
			request.Param1 = 2;

		} else if (arg == "-lpa") {
			allpubtopics = true;
			request.Entity = 0;
			request.Command = 2;
			request.Param1 = 2;

		} else if (arg == "-lpi") {
			request.Entity = 2;
			request.Command = 1;
			strp = &request.Name;

		} else if (arg == "-ls") {
			request.Entity = 0;
			request.Command = 2;
			request.Param1 = 3;

		} else if (arg == "-lsa") {
			allsubtopics = true;
			request.Entity = 0;
			request.Command = 2;
			request.Param1 = 3;

		} else if (arg == "-lsi") {
			request.Entity = 3;
			request.Command = 1;
			strp = &request.Name;

		} else if (arg == "-n") {
			strp = &request.Name;

		} else if (arg == "-p1") {
			int64p = &request.Param1;

#ifdef USE_PIZZATEST
		} else if (arg == "-pizza") {
			testData.cheese = "Hello from DebugConsole";
			request.Objs.push_back(&testData);
#endif
		} else if (arg == "-v") {
			gverbose = true;

		}

		if ((strp != nullptr) && ((i+1) < argc)) {
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

	request.setKey(key);

	if (printUsage) {
		Usage();
		ops::PrintArchiverOut prt(std::cout, 1);
		std::cout << "Dump of Request:" << std::endl;
		request.serialize(&prt);

	} else {
		// Setup the OPS static error service (common for all participants, reports errors during participant creation)
		ops::ErrorWriter* const errorWriterStatic = new ops::ErrorWriter(std::cout);
		ops::Participant::getStaticErrorService()->addListener(errorWriterStatic);

		// Add all Domain's from given file(s)
		ops::OPSConfigRepository::Instance()->Add(cfgFile);

		if (listDomains) {
			ListDomains();
			return 0;
		}

		// Create participant
		ops::Participant* const participant = ops::Participant::getInstance(domain);
		if (participant == nullptr) {
			std::cout << "Failed to create Participant. Missing ops_config.xml ??" << std::endl;
			exit(-1);
		}
		participant->addTypeSupport(new opsidls::opsidlsTypeFactory());

		DebugListener listener(participant);
		listener.Start();

#ifdef OPS_ENABLE_DEBUG_HANDLER
        ops::Topic const top = participant->createDebugTopic();
		opsidls::DebugRequestResponseDataPublisher pub(top);
		pub.start();
		pub.write(request);

		request.Objs.clear();

		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));

		if (allpubtopics || allsubtopics) {
			if (allpubtopics) {
				request.Entity = 2;
				request.Command = 1;
				ForAllTopics(listener, pub);
			}
			if (allsubtopics) {
				request.Entity = 3;
				request.Command = 1;
				ForAllTopics(listener, pub);
			}
			ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
		}
#endif
	}
    return 0;
}

