
//Include ops
#include <ops.h>
#include <TimeHelper.h>
//Include a publisher for the data type we want to publish, generated from our IDL project HelloWorld.
#include "hello/HelloDataPublisher.h"
//Include type support for the data types we have defined in our IDL project, generated from our IDL project HelloWorld.
#include "HelloWorld/HelloWorldTypeFactory.h"
//Include iostream to get std::cout
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

#include "../ConfigFileHelper.h"

int main(const int argc, const char* args[])
{
	UNUSED(argc);
	UNUSED(args);
	using namespace ops;

    setup_alt_config("Examples/OPSIdls/HelloWorld/ops_config.xml");

    //Create a Participant (i.e. an entry point for using ops.), compare with your ops_config.xml
	ops::Participant* const participant = Participant::getInstance("HelloDomain");
	if(!participant)
	{
		std::cout << "Create participant failed. do you have ops_config.xml on your rundirectory?" << std::endl;
		ops::TimeHelper::sleep(std::chrono::seconds(10));
		exit(1);
	}

	//Add type support for our types, to make this participant understand what we are talking
	participant->addTypeSupport(new HelloWorld::HelloWorldTypeFactory());

	//Now, create the Topic we wish to publish on. Might throw ops::NoSuchTopicException if no such Topic exist in ops_config.xml
	Topic const topic = participant->createTopic("HelloTopic");

	//Create a publisher on that topic
	hello::HelloDataPublisher pub(topic);
	pub.setName("HelloTopicPublisher"); //Optional identity of the publisher

	//Create some data to publish, this is our root object.
	hello::HelloData data;

	data.helloString = "Hello World From C++!!";

	//Publish the data peridically 
	while(true)
	{
		pub.write(&data);
		std::cout << "Writing data"  <<  std::endl;
		ops::TimeHelper::sleep(std::chrono::milliseconds(1000));
	}

	return 0;
}

