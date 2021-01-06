// TestAll_Publish.cpp : Defines the entry point for the console application.
//


#include <ops.h>
#include <TimeHelper.h>
#include "RequestReply.h"
#include "HelloRequestReply/HelloRequestReplyTypeFactory.h"
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

  setup_alt_config("Examples/OPSIdls/HelloRequestReply/ops_config.xml");

  //Create a Participant (i.e. an entry point for using ops.), compare with your ops_config.xml
  ops::Participant* const participant = Participant::getInstance("HelloDomain");
  if(!participant)
  {
	std::cout << "Create participant failed. do you have ops_config.xml on your rundirectory?" << std::endl;
#ifdef _WIN32
    TimeHelper::sleep(std::chrono::seconds(10));
#endif
	exit(1);
  }

  //Add type support for our types, to make this participant understand what we are talking
  participant->addTypeSupport(new HelloRequestReply::HelloRequestReplyTypeFactory());

  //Now, create the Topic we wish to publish on. Might throw ops::NoSuchTopicException if no such Topic exist in ops_config.xml
  Topic const requestTopic = participant->createTopic("RequestHelloTopic");
  Topic const replyTopic = participant->createTopic("HelloTopic");

  ops::RequestReply<hello::RequestHelloData, hello::HelloData> requestReplyHelper(requestTopic, replyTopic, "req_rep_instance1");

  while(true)
  {
	hello::HelloData* reply = nullptr;
	hello::RequestHelloData request;
	request.requestersName = "C++ Requester";

	reply = requestReplyHelper.request(&request, 1000);

	if(reply != nullptr)
	{
		std::cout << "Reply received: " << reply->helloString  <<  std::endl;

		delete reply;
	}
	else
	{
		std::cout << "No reply." << std::endl;
	}
	TimeHelper::sleep(std::chrono::seconds(1)); 
  }

  return 0;
}
