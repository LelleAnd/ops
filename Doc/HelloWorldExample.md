[Building Examples](BuildingExamples.md), [Build Instructions](BuildInstructions.md)

---
## Hello World Example ##


This page will show you how to write an application that sends a Hello World message with OPS. It will guide you through the steps of writing a Data class in IDL, defining a Topic configuration and implement publish and subscribe code in both C++ and Java.

Make sure you have understand the [Basic Concepts](BasicConcepts.md) of OPS before continuing.

First we need a Data type to carry our Hello World message. We define this Data class using OPS IDL (See also [IDL Language](IDLLanguage.md) and [OPS IDL Builder](IDLCompilerTutorial.md)):

```
package hello;

class HelloData
{
    /* This string holds hello messages */
    string helloString;
}
```

OK, so now we have a Data class to carry a string where we can put our Hello World message.

Now we also need a Topic to publish our Data class on. We define the Topic in a XML file that we call ops\_config.xml like this (see also [OPS Config](OpsConfig.md)):

```
<?xml version="1.0" encoding="UTF-8"?>
<!--
 Description:
 A template ops_config.xml file, this file shall be put on run directory of all applications that wants to use these topics.
-->
<root>
    <ops_config type = "DefaultOPSConfigImpl">
        <domains>
            <!-- The one and only domain in our Hello World example -->
            <element type = "Domain">
                <domainID>HelloDomain</domainID>
                <domainAddress>234.5.6.8</domainAddress>
                <topics>
                    <!-- The one and only topic in our Hello World example -->
                    <element type = "Topic">
                        <name>HelloTopic</name>
                        <port>12000</port>
                        <dataType>hello.HelloData</dataType>
                    </element>
                </topics>
            </element>
        </domains>
    </ops_config>
</root>
```

As you can see we specify one Topic called HelloTopic and attach this Topic to a Domain that we call HelloDomain. We choose a port (12000) and a domainAddress (234.5.6.8) to communicate on.

We have now defined a programming language independent interface for sending Hello World messages, and we can go on to implement a Publisher:

In Java
```
package helloworldjavaimpl;

import hello.HelloData;
import hello.HelloDataPublisher;
import java.util.logging.Level;
import java.util.logging.Logger;
import ops.CommException;
import ops.Participant;

public class MainPublish
{

    public static void main(String[] args)
    {
        try {
            Participant participant = Participant.getInstance("HelloDomain");
            participant.addTypeSupport(new HelloWorld.HelloWorldTypeFactory());
            HelloDataPublisher publisher = new HelloDataPublisher(participant.createTopic("HelloTopic"));

            HelloData data = new HelloData();
            data.helloString = "Hello World From Java!!!";
            while(true) {
                publisher.write(data);
                Thread.sleep(1000);
            }

        } catch (InterruptedException ex) {
            Logger.getLogger(MainPublish.class.getName()).log(Level.SEVERE, null, ex);
        } catch (CommException ex) {
            Logger.getLogger(MainPublish.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
```
And in C++
```
#include <ops.h>
#include "hello/HelloDataPublisher.h"
#include "HelloWorld/HelloWorldTypeFactory.h"
#include <iostream>
#include <windows.h>

int main(int argc, const char* args[])
{
	using namespace ops;

	ops::Participant* participant = Participant::getInstance("HelloDomain");
	if (participant == nullptr) {
		std::cout << "Create participant failed. do you have ops_config.xml on your rundirectory?" << std::endl;
		Sleep(10000); exit(1);
	}

	participant->addTypeSupport(new HelloWorld::HelloWorldTypeFactory());

	Topic topic = participant->createTopic("HelloTopic");

	hello::HelloDataPublisher pub(topic);
	pub.setName("HelloTopicPublisher"); //Optional identity of the publisher

	hello::HelloData data;

	data.helloString = "Hello World From C++!!";

	int mainSleep = 1000;
	while (true) {
		pub.write(&data);
		std::cout << "Writing data"  <<  std::endl;
		Sleep(mainSleep);
	}

	return 0;
}
```

Note that this is ALL the code you need to create a simple Publisher. To deeper understand whats going on on each line, have a look at [Publishing On Topics](PublishingOnTopic.md).

Now let's subscribe to the HelloTopic:

In Java
```
package helloworldjavaimpl;

import HelloWorld.HelloWorldTypeFactory;
import hello.HelloData;
import hello.HelloDataSubscriber;
import java.util.Observable;
import java.util.Observer;
import java.util.logging.Level;
import java.util.logging.Logger;
import ops.Participant;

public class MainSubscribe
{
    public static void main(String[] args)
    {
        Participant participant = Participant.getInstance("HelloDomain");
        participant.addTypeSupport(new HelloWorldTypeFactory());

        final HelloDataSubscriber subscriber = new HelloDataSubscriber(participant.createTopic("HelloTopic"));

        subscriber.addObserver(new Observer()
        {
            public void update(Observable sub, Object o)
            {
                HelloData data = subscriber.getData();
                System.out.println("" + data.helloString);
            }
        });
        subscriber.start();

        //Just keep program alive, action takes place in update above...
        while (true) {
            try {
                Thread.sleep(100000);
            } catch (InterruptedException ex) {
                Logger.getLogger(MainSubscribe.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
}
```
And in C++:
```
#include <ops.h>
#include "hello/HelloDataSubscriber.h"
#include "HelloWorld/HelloWorldTypeFactory.h"
#include <iostream>
#include <vector>

//Create a class to act as a listener for OPS data
class Main : ops::DataListener
{
public:
	//Use a member subscriber so we can use it from onNewData, see below.
	hello::HelloDataSubscriber* sub;

public:
	Main()
	{
		ops::Participant* participant = ops::Participant::getInstance("HelloDomain");
		if (participant == nullptr) {
			std::cout << "Create participant failed. Do you have ops_config.xml in your run directory?" << std::endl;
			Sleep(10000); exit(1);
		}

		participant->addTypeSupport(new HelloWorld::HelloWorldTypeFactory());

		ops::Topic topic = participant->createTopic("HelloTopic");

		sub = new hello::HelloDataSubscriber(topic);

		sub->addDataListener(this);

		sub->start();
	}

	///Override from ops::DataListener, called whenever new data arrives.
	void onNewData(ops::DataNotifier* subscriber)
	{
		hello::HelloData data;
		if (sub == subscriber) {
			sub->getData(&data);
			std::cout << data.helloString << std::endl;
		}
	}
	~Main()
	{
		delete sub;
	}
};

//Application entry point
int main(int argc, char* args)
{
	//Create an object that will listen to OPS events
	Main m;

	//Just keep program alive, action will take place in Main::onNewData()
	while (true) {
		Sleep(10000);
	}

	return 0;
}
```

Once again if you are not following have a look at [Subscribing To Topics](SubscribingToTopic.md)

Thats all code needed to publish and subscribe to Hello World messages to and from C++ and Java. To build and run this example, check out [Installation](Installation.md) and [building examples](BuildingExamples.md).
