
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <process.h>
#endif

#include <limits>
#include <vector>
#include <chrono>
#include <ops.h>

// The following include files are created by the OPS IDL Builder
#include "Echotime/EchotimeData.h"
#include "Echotime/EchotimeDataPublisher.h"
#include "Echotime/EchotimeDataSubscriber.h"
#include "Echotime/EchotimeTypeFactory.h"

// Helper for finding and loading of the configuration file for the OPS examples
#include "../ConfigFileHelper.h"

using namespace Echotime;

// Variables affected by command line arguments
static uint32_t gNumMessages = 500;
static uint32_t gPeriod = 10;
static uint32_t gSize = 64;
static uint32_t gRepeat = 1;
static bool gVerbose = false;
static bool gMaster = false;
static bool gEcho = false;

// Helper
uint32_t parseUInt32(std::string s, uint32_t defaultValue)
{
    uint32_t value = 0;
    try {
        std::stringstream ss(s);
        ss >> value;
        return value;
    }
    catch (...) {
        return defaultValue;
    }
}

/// =======================================================================

void Publisher(const ops::Topic& reqTopic, const ops::Topic& replyTopic)
{
    if (gVerbose) {
        std::cout << "Publishing on " << reqTopic.getName() <<
            " [" << reqTopic.getTransport() <<
            "::" << reqTopic.getDomainAddress() <<
            "::" << reqTopic.getPort() <<
            "] " << std::endl;

        std::cout << "Subscribing to " << replyTopic.getName() <<
            " [" << replyTopic.getTransport() <<
            "::" << replyTopic.getDomainAddress() <<
            "::" << replyTopic.getPort() <<
            "] " << std::endl;
    }

    // Create publisher and subcriber for EchotimeData
	EchotimeDataPublisher pub(reqTopic);
    EchotimeDataSubscriber sub(replyTopic);

    // Vector for temporary holding of recieved messages
    std::vector<ops::OPSMessage*> vec;

    // The subscriber timestamps the message and saves it in a vector for later work
    sub.addDataListener(
        [&](ops::DataNotifier* /*subscriber*/) {
            //int64_t now = (int64_t)std::chrono::system_clock().now().time_since_epoch().count();
            int64_t now = (int64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
            EchotimeData* const data = sub.getTypedDataReference();
            data->RecvTive = now;
            ops::OPSMessage* const newMess = sub.getMessage();
            newMess->reserve();
            vec.push_back(newMess);
        }
    );
    sub.start();

    // Create some data to publish
    EchotimeData data;
    for (uint32_t i = 0; i < gSize; i++) { data.Data.push_back(' '); }

    // When using metadata to connect, subscribers/publishers need some time to connect 
    std::cout << "Wait 2 second ...";
    ops::TimeHelper::sleep(std::chrono::milliseconds(2000));

    // We may repeat the work several times with the same Publisher and Subscriber
    for (uint32_t r = 0; r < gRepeat; r++) {
        vec.clear();

        // Send some timestamped data to be echoed by other side.
        // Our subscriber timestamps each massage at reception.
        std::cout << "\rStarting with period " << gPeriod << " [ms] ... ";
        for (uint32_t i = 0; i < gNumMessages; i++) {
            //data.SendTime = (int64_t)std::chrono::system_clock().now().time_since_epoch().count();
            data.SendTime = (int64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
            pub.write(data);
            ops::TimeHelper::sleep(std::chrono::milliseconds(gPeriod));
        }

        // Wait some time for messages in-fly to arrive
        std::cout << "Sent " << gNumMessages << " messages. Wait 1 second ... ";
        ops::TimeHelper::sleep(std::chrono::milliseconds(1000));

        // Output result from the echo test
        size_t num = vec.size();
        std::cout << "Got " << num << " messages\n";

        // Calculate transfer time
        #undef min
        #undef max
        int64_t minvalue = std::numeric_limits<int64_t>::max();
        int64_t maxvalue = std::numeric_limits<int64_t>::min();
        uint64_t sum = 0;
        for (ops::OPSMessage* mess : vec) {
            EchotimeData* data = dynamic_cast<EchotimeData*>(mess->getData());
            // Divide by 2, assuming equal transfer time in each direction
            int64_t diff = (data->RecvTive - data->SendTime) >> 1;
            sum += diff;
            if (minvalue > diff) { minvalue = diff; }
            if (maxvalue < diff) { maxvalue = diff; }
            if (gVerbose) { std::cout << "Received replyTopic " << " with " << data->SendTime << ", diff/2: " << diff << " [us]\n"; }
            mess->unreserve();
        }

        if (num > 0) {
            std::cout << "Min: " << minvalue << " Max: " << maxvalue << " Average: " << sum / num << " [us]\n";
        }
    }
}

/// =======================================================================

void EchoHandler(const ops::Topic& reqTopic, const ops::Topic& replyTopic)
{
    std::cout << "Subscribing to " << reqTopic.getName() <<
        " [" << reqTopic.getTransport() <<
        "::" << reqTopic.getDomainAddress() <<
        "::" << reqTopic.getPort() <<
        "] " << std::endl;

	std::cout << "Publishing on " << replyTopic.getName() <<
		" [" << replyTopic.getTransport() <<
		"::" << replyTopic.getDomainAddress() <<
		"::" << replyTopic.getPort() <<
		"] " << std::endl;

	EchotimeDataSubscriber sub(reqTopic);
	EchotimeDataPublisher pub(replyTopic);

    // Subscriber just echos data as is
    sub.addDataListener(
        [&](ops::DataNotifier* /*subscriber*/) {
            EchotimeData* const data = sub.getTypedDataReference();
            pub.write(data);
        }
    );

    sub.start();

    // Infinite loop
    while (true) {
        ops::TimeHelper::sleep(std::chrono::milliseconds(100));
    }
}

/// =======================================================================

void usage()
{
	std::cout << "\n";
	std::cout << "Usage: Echotime [options] master|echo\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -?              Show this help\n";
    std::cout << "  -n <num>        Number of message to send [" << gNumMessages << "]\n";
    std::cout << "  -p <period>     Period in [ms] between sent messages [" << gPeriod << "]\n";
    std::cout << "  -r <count>      Repeat 'count' times [" << gRepeat << "]\n";
    std::cout << "  -s <size>       Num extra bytes in message [" << gSize << "]\n";
    std::cout << "  -v              Verbose output [" << gVerbose << "]\n";
    std::cout << "\n";
}

int main(const int argc, const char* argv[])
{
    for (int i = 1; i < argc; i++) {
        std::string const arg = argv[i];
        uint32_t* valp = nullptr;

        if (arg == "-?") {
            usage();
            exit(0);

        } else if (arg == "-n") {
            valp = &gNumMessages;

        } else if (arg == "-p") {
            valp = &gPeriod;

        } else if (arg == "-s") {
            valp = &gSize;

        } else if (arg == "-r") {
            valp = &gRepeat;

        } else if (arg == "-v") {
            gVerbose = true;

        } else if (arg == "master") {
            gMaster = true;
            gEcho = false;

        } else if (arg == "echo") {
            gMaster = false;
            gEcho = true;

        } else {
            usage();
            exit(0);
        }

        if (valp != nullptr) {
            if (i < argc) {
                i++;
                *valp = parseUInt32(argv[i], *valp);
                valp = nullptr;
            }
        }
    }

    if (gMaster == false && gEcho == false) {
        usage();
        exit(0);
    }

#ifdef _WIN32
    // --------------------------------------------------------------------
    // Try to set timer resolution to 1 ms
    UINT const TARGET_RESOLUTION = 1;         // 1-millisecond target resolution
    UINT wTimerRes = TARGET_RESOLUTION;
    timeBeginPeriod(wTimerRes);
#endif

    // Add all Domain's from given file(s)
    setup_alt_config("Examples/OPSIdls/Echotime/ops_config.xml");

    // Create OPS participant to access a domain in the default configuration file
    // "ops_config.xml" in current working directory. There are other overloads to
    // create a participant for a specific configuration file.
    ops::Participant* const participant = ops::Participant::getInstance("EchoDomain");

    // Add our generated factory so OPS can create our data objects
    participant->addTypeSupport(new Echotime::EchotimeTypeFactory());

    // Add an errorwriter instance to the participant to catch ev. internal OPS errors
    // We can easily write our own if we want to log data in another way.
    ops::ErrorWriter* const errorWriter = new ops::ErrorWriter(std::cout);
    participant->addListener(errorWriter);

    // Create the topic to use, might throw ops::NoSuchTopicException
    // The topic must exist in the used ops configuration file
    const ops::Topic topicReq = participant->createTopic("EchotimeRequestTopic");
	const ops::Topic topicRep = participant->createTopic("EchotimeReplyTopic");

    if (gMaster) {
        Publisher(topicReq, topicRep);
    }
    if (gEcho) {
        EchoHandler(topicReq, topicRep);
    }

#ifdef _WIN32
    // --------------------------------------------------------------------
    // We don't need the set resolution any more
    timeEndPeriod(wTimerRes);
#endif
}
