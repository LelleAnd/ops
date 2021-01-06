// Example1.cpp : Defines the entry point for the console application.
//

#include <ops.h>

// The following include files are created by the OPS IDL Builder
//  - From BaseData.idl
#include "TestAll/BaseData.h"
#include "TestAll/BaseDataPublisher.h"
#include "TestAll/BaseDataSubscriber.h"

//  - From ChildData.idl
#include "TestAll/ChildData.h"
#include "TestAll/ChildDataPublisher.h"
#include "TestAll/ChildDataSubscriber.h"

//  - Factory that is able to create our data objects
#include "TestAll/TestAllTypeFactory.h"

#include "../ConfigFileHelper.h"

using namespace TestAll;

/// =======================================================================

void PublisherExample(const ops::Topic& topic)
{
	std::cout << "Publishing on " << topic.getName() <<
		" [" << topic.getTransport() <<
		"::" << topic.getDomainAddress() <<
		"::" << topic.getPort() <<
		"] " << std::endl;

	// Create a publisher for ChildData
	ChildDataPublisher pub(topic);

	// It's a good practise to set a publisher name, but it's not necessary.
	pub.setName("TestAllPublisher");

	// If there are many publishers on the same topic, a key can be set to identify a
	// specific publisher instance. A subscriber can tell OPS to filter on this key.
	pub.setKey("InstanceOne");

	// Create some data to publish
	ChildData data;
	data.baseText = "Some text";
	data.l = 0;

	while (true) {
		// Change data
		data.l++;

		std::cout << "Writing ChildTopic " << data.l <<  std::endl;
		pub.write(data);

		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
	}
}

/// =======================================================================

void PollingSubscriberExample(const ops::Topic& topic)
{
	std::cout << "Subscribing to " << topic.getName() <<
		" [" << topic.getTransport() <<
		"::" << topic.getDomainAddress() <<
		"::" << topic.getPort() <<
		"] " << std::endl;

	// Create a subscriber for ChildData
	ChildDataSubscriber sub(topic);

	// Setup any filters ...

	// Finally start the subscriber (tell it to start listening for data)
	sub.start();

	while (true) {
#ifndef zzz
        if (sub.waitForNewData(100)) {
			// Need to lock message while using it's data via the reference
            const ops::MessageLock lck(sub);
			ChildData* const data = sub.getTypedDataReference();
			std::cout << "New data found: Received ChildTopic with " << data->l << std::endl;
        } else {
            std::cout << "." << std::flush;
        }
#else
		if (sub.newDataExist()) {
			ChildData data;
			// Message lock is handled internaly in subscriber
			sub.getData(data);
			std::cout << "New data found: Received ChildTopic with " << data.l << std::endl;
		}
		ops::TimeHelper::sleep(std::chrono::milliseconds(10));
#endif
	}
}

/// =======================================================================

// Forward declaration
void CallbackFunc(ops::DataNotifier* sender, void* userData);

void CallbackSubscriberExample(const ops::Topic& topic)
{
	std::cout << "Subscribing to " << topic.getName() <<
		" [" << topic.getTransport() <<
		"::" << topic.getDomainAddress() <<
		"::" << topic.getPort() <<
		"] " << std::endl;

	// Create a subscriber for ChildData
	ChildDataSubscriber sub(topic);

	// Add function callback as listener for data
	sub.addDataListener([](ops::DataNotifier* sender) { CallbackFunc(sender, (void*)0); });

    // Test with another callback
	sub.addDataListener([](ops::DataNotifier* sender) { CallbackFunc(sender, (void*)100); });

	// Setup any filters ...

	// Finally start the subscriber (tell it to start listening for data)
	sub.start();

	while(true) {
		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
	}
}

void CallbackFunc(ops::DataNotifier* const sender, void* const userData)
{
    // NOTE: It's important that we keep this callback fast, it will block
    // the receive for all topics belonging to the participant (currently a single
    // thread for each participant instance, that does all receive handling).

    // Message lock is held while in callback

    ChildDataSubscriber* const sub = dynamic_cast<ChildDataSubscriber*>(sender);
	size_t const user = (size_t)userData;

	// The OPSMessage contains some metadata for the received message
	// eg. publisher name, publication id (message counter), ...
	// These may be of interrest
	ops::OPSMessage* const newMess = sub->getMessage();

	// Get the actual data object published
    ChildData* const data = sub->getTypedDataReference();

	// Use the data
	std::cout << "callback(" << user << "): Received ChildTopic from " << newMess->getPublisherName() << " with " << data->l << std::endl;

	// NOTE that the OPSMessage instance and the data object, as default
	// will be deleted when this callback returns.
	// If you eg. want to buffer messages to keep the callback fast, you can
	// postpone the delete by calling "newMess->reserve()" here in the callback.
	// When you are finished with the message, call "newMess->unreserve()".
	// This will delete the message if the reserve count == 0 (ie. if the number
	// of reserve() and unreserve() calls match.
}

/// =======================================================================

class SubscriptionHandler : ops::DataListener, ops::DeadlineMissedListener
{
public:
	SubscriptionHandler(const ops::Topic& topic) :
        sub(topic)
	{
		std::cout << "Subscribing to " << topic.getName() <<
			" [" << topic.getTransport() <<
			"::" << topic.getDomainAddress() <<
			"::" << topic.getPort() <<
			"] " << std::endl;

		// Add this class instance as listener for data, the method onNewData() will be called
		sub.addDataListener(this);

		// If you want, set a deadline with maximum allowed distance between two messages,
		// the method onDeadlineMissed() will be called if time exceeds maximum
		sub.deadlineMissedEvent.addDeadlineMissedListener(this);
		sub.setDeadlineQoS(5000);

		// If you want, add a keyfilter to just be notified with data objects with the specified key
		sub.addFilterQoSPolicy(new ops::KeyFilterQoSPolicy("InstanceOne"));

#ifdef USE_TIME_BASED_FILTER
		// There are also some other filters that can be set, and we can implement our own
		sub->setTimeBasedFilterQoS(1000);
#endif
	}

    SubscriptionHandler() = delete;
	SubscriptionHandler(SubscriptionHandler const&) = delete;
	SubscriptionHandler(SubscriptionHandler&&) = delete;
	SubscriptionHandler& operator =(SubscriptionHandler&&) = delete;
	SubscriptionHandler& operator =(SubscriptionHandler const&) = delete;

	~SubscriptionHandler()
	{
	}

    void start()
    {
        // Start the subscriber (tell it to start listening for data)
        sub.start();
    }

#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	// Override from ops::DataListener, called whenever new data arrives.
	virtual void onNewData(ops::DataNotifier* const subscriber) override
    {
		// NOTE: It's important that we keep this callback fast, it will block
		// the receive for all topics belonging to the participant (currently a single
		// thread for each participant instance, that does all receive handling).

		// Message lock is held while in callback

		// If the class subscribes for more than one topic, all will come here
		// So check which it is
		if (subscriber == &sub) {
			// The OPSMessage contains some metadata for the received message
			// eg. publisher name, publication id (message counter), ...
			// These may be of interrest
			ops::OPSMessage* const newMess = sub.getMessage();

			// Get the actual data object published
            ChildData* const data = sub.getTypedDataReference();

			// Use the data
			std::cout << "Received ChildTopic from " << newMess->getPublisherName() << " with " << data->l << std::endl;

			// NOTE that the OPSMessage instance and the data object, as default
			// will be deleted when this callback returns.
			// If you eg. want to buffer messages to keep the callback fast, you can
			// postpone the delete by calling "newMess->reserve()" here in the callback.
			// When you are finished with the message, call "newMess->unreserve()".
			// This will delete the message if the reserve count == 0 (ie. if the number
			// of reserve() and unreserve() calls match.
		}
	}

	// Override from ops::DeadlineMissedListener, called if no new data has arrived within deadlineQoS.
	virtual void onDeadlineMissed(ops::DeadlineMissedEvent* ) override
	{
		std::cout << "Deadline Missed!" << std::endl;
	}
private:
    TestAll::ChildDataSubscriber sub;
};

void ObjectSubscriberExample(const ops::Topic& topic)
{
	SubscriptionHandler handler(topic);
    handler.start();

	while(true) {
		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
	}
}

/// =======================================================================

void SubscriberLamdaExample(const ops::Topic& topic)
{
    std::cout << "Subscribing to " << topic.getName() <<
        " [" << topic.getTransport() <<
        "::" << topic.getDomainAddress() <<
        "::" << topic.getPort() <<
        "] " << std::endl;

    // Create a subscriber for ChildData
    ChildDataSubscriber sub(topic);

    // Setup any filters ...

    // Setup listeners
    sub.addDataListener(
        [&](ops::DataNotifier* /*subscriber*/) {
            // The same rules as for the callback example and for the listener onNewData() example applies
            ops::OPSMessage* const newMess = sub.getMessage();

            // Get the actual data object published
            ChildData* const data = sub.getTypedDataReference();

            // Use the data
            std::cout << "Received ChildTopic from " << newMess->getPublisherName() << " with " << data->l << std::endl;
        }
    );

    //sub.deadlineMissedEvent.addDeadlineMissedListener(
    //    [&](ops::DeadlineMissedEvent* /*sender*/) {
    //        std::cout << "Deadline Missed for topic " << sub.getTopic().getName() << std::endl;
    //    }
    //);

    // Finally start the subscriber (tell it to start listening for data)
    sub.start();

    while (true) {
        ops::TimeHelper::sleep(std::chrono::milliseconds(10));
    }
}


void usage()
{
	std::cout << "" << std::endl;
	std::cout << "Usage: Example1 pub|sub_poll|sub_object|sub_callback|sub_lamda" << std::endl;
	std::cout << "" << std::endl;
}

int main(const int argc, const char* argv[])
{
    // Add all Domain's from given file(s)
    setup_alt_config("Examples/OPSIdls/TestAll/ops_config.xml");

    // Create OPS participant to access a domain in the default configuration file
    // "ops_config.xml" in current working directory. There are other overloads to
    // create a participant for a specific configuration file.
    ops::Participant* const participant = ops::Participant::getInstance("TestAllDomain");

    // Add our generated factory so OPS can create our data objects
    participant->addTypeSupport(new TestAll::TestAllTypeFactory());

    // Add an errorwriter instance to the participant to catch ev. internal OPS errors
    // We can easily write our own if we want to log data in another way.
    ops::ErrorWriter* const errorWriter = new ops::ErrorWriter(std::cout);
    participant->addListener(errorWriter);

    // Create the topic to use, might throw ops::NoSuchTopicException
    // The topic must exist in the used ops configuration file
    const ops::Topic topic = participant->createTopic("ChildTopic");

    if (argc > 1) {
		std::string const arg(argv[1]);
		if (arg == "pub") {
			PublisherExample(topic);
		} else if (arg == "sub_callback"){
			CallbackSubscriberExample(topic);
		} else if (arg == "sub_object"){
			ObjectSubscriberExample(topic);
		} else if (arg == "sub_poll"){
			PollingSubscriberExample(topic);
        } else if (arg == "sub_lambda"){
            SubscriberLamdaExample(topic);
		} else {
			usage();
		}
	} else {
		usage();
	}
}
