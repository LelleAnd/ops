
//Auto generated code example for subscribing to RequestHelloTopic on domain HelloDomain generated from project HelloRequestReply
#include <ops.h>
#include "hello/RequestHelloDataSubscriber.h"
#include "hello/HelloDataPublisher.h"
#include "HelloRequestReply/HelloRequestReplyTypeFactory.h"
#include <iostream>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

#include "../ConfigFileHelper.h"

//Create a class to act as a listener for OPS data and deadlines
class Main : ops::DataListener, ops::DeadlineMissedListener
{
public:
	//Use a member subscriber so we can use it from onNewData, see below.
	hello::RequestHelloDataSubscriber* sub;
	hello::HelloDataPublisher* pub;

public:

	Main()
	{
		//Create a Participant (i.e. an entry point for using ops.), compare with your ops_config.xml
		ops::Participant* const participant = ops::Participant::getInstance("HelloDomain");
		if(!participant)
		{
			std::cout << "Create participant failed. do you have ops_config.xml in your rundirectory?" << std::endl;
#ifdef _WIN32
			Sleep(10000); exit(1);
#else
      exit(1);
#endif
		}

		//Add type support for our types, to make this participant understand what we are talking
		participant->addTypeSupport(new HelloRequestReply::HelloRequestReplyTypeFactory());

		//Now, create the Topic we wish to subscribe on. Might throw ops::NoSuchTopicException if no such Topic exist in ops_config.xml
		ops::Topic topic = participant->createTopic("RequestHelloTopic");

		//Create a subscriber on that topic.
		sub = new hello::RequestHelloDataSubscriber(topic);

		//Tell the subscriber that we expect data at least once every 1500 ms
		sub->setDeadlineQoS(1500);

		//Add this class as a listener for new data events
		sub->addDataListener(this);

		//Add this class as a listener for deadline missed events
		sub->deadlineMissedEvent.addDeadlineMissedListener(this);

		// Setup publisher for sending reply on
		ops::Topic replyTopic = participant->createTopic("HelloTopic");
		pub = new hello::HelloDataPublisher(replyTopic);

		//Start the subscription
		sub->start();
	}
	///Override from ops::DataListener, called whenever new data arrives.
	virtual void onNewData(ops::DataNotifier* const subscriber) override
	{
		hello::RequestHelloData data;
		if(sub == subscriber)
		{
			sub->getData(data);
			std::cout << "Data received. RequestId: " << data.requestId << std::endl;

			hello::HelloData reply;
			reply.requestId = data.requestId;
			reply.helloString = "Hello there, here is my reply";
			reply.setKey(data.getKey());
			pub->write(reply);
		}
	}
	///Override from ops::DeadlineMissedListener, called if no new data has arrived within deadlineQoS.
	virtual void onDeadlineMissed(ops::DeadlineMissedEvent* const evt) override
	{
		UNUSED(evt);
		std::cout << "Deadline Missed!" << std::endl;
	}
	~Main()
	{
		delete sub;
	}
	Main(Main const&) = delete;
	Main(Main&&) = delete;
	Main& operator =(Main&&) = delete;
	Main& operator =(Main const&) = delete;
};

//Application entry point
int main(const int argc, const char* args[])
{
	UNUSED(argc);
	UNUSED(args);
    
    setup_alt_config("Examples/OPSIdls/HelloRequestReply/ops_config.xml");

    //Create an object that will listen to OPS events
	Main const m;

	//Just keep program alive, action will take place in Main::onNewData()
	while(true)
	{
#ifdef _WIN32
		Sleep(10000);
#else
		usleep(10000000);
#endif
	}

	return 0;
}
