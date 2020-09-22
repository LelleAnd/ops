
#include <ops.h>
#include "hello/HelloDataSubscriber.h"
#include "HelloWorld/HelloWorldTypeFactory.h"
#include <iostream>
#include <vector>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

#include "../ConfigFileHelper.h"

//Create a class to act as a listener for OPS data
class Main : ops::DataListener
{
public:
	//Use a member subscriber so we can use it from onNewData, see below.
	std::unique_ptr<hello::HelloDataSubscriber> sub;

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
		participant->addTypeSupport(new HelloWorld::HelloWorldTypeFactory());

		//Now, create the Topic we wish to subscribe on. Might throw ops::NoSuchTopicException if no such Topic exist in ops_config.xml
		ops::Topic topic = participant->createTopic("HelloTopic");

		//Create a subscriber on that topic.
		sub.reset(new hello::HelloDataSubscriber(topic));

		//Add this class as a listener for new data events
		sub->addDataListener(this);

		//Start the subscription
		sub->start();

	}
	///Override from ops::DataListener, called whenever new data arrives.
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
	virtual void onNewData(ops::DataNotifier* const subscriber) override
	{
		hello::HelloData data;
		if(sub.get() == subscriber)
		{
			sub->getData(&data);
			std::cout << data.helloString << std::endl;
		}
	}
	~Main()
	{
	}
	Main(Main const&) = delete;
	Main(Main&&) = delete;
	Main& operator =(Main&&) = delete;
	Main& operator =(Main const&) = delete;

};

//Application entry point
int main(const int argc, const char* args[])
{
	//Create an object that will listen to OPS events
	UNUSED(argc);
	UNUSED(args);
    
    setup_alt_config("Examples/OPSIdls/HelloWorld/ops_config.xml");
    
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
