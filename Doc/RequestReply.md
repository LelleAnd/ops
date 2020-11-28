# Request Reply #

OPS is primarily a publish subscribe based middleware which is a specialization of the more general Message Oriented Middleware (MOM) class of middlewares. This class of middlewares (including OPS) focus on passing asynchronous messages from producers to consumers in an event manner, i.e. a consumer registers an interest in getting data once, and then receive data continuously without requesting each data sample.

Most systems however have needs for some kind of request/reply communication in some parts. Middlewares that deals with this kind of communication include Remote Method Invocation (RMI) middlewares and Object Oriented Middlewares (or Object Request Brokers), e.g. CORBA, Java RMI and the Internet Communications Engine (ICE).

Of course, a system can mix technologies and have CORBA and OPS running alongside each other. And if you needs a lot of both request/reply and pub/sub communication, this is probably a good idea.

OPS however offers a request/reply mechanism built on top of common OPS topics. The idea is simple, and you may already have found yourself implementing the same feature:
You create two topics, one request topic (this can be seen as the parameters for the request) and one reply topic (the return value of the request), and finally OPS implement some helper classes to help you with matching the reply to the request and timeout if there is no reply within a certain timeout.

This is what you need to do to create a simple request/reply case where a requester can request a Hello message:

Create two Data classes in IDL, one for the request and one for the reply, notice that to mark an IDL class as a request or a reply, you extend the OPS core classes *ops.Request* and *ops.Reply* directly from your IDL.

```
package hello;

class RequestHelloData extends ops.Request
{
    /*The name of the requester, will be used to request a reply like "Hello requesterName"*/
    string requestersName;
}
```
and
```
package hello;

class HelloData extends ops.Reply
{
    /*Reply of a Hello request*/
    string helloString;
}
```
Also, define two topics to send our data on:
```
<?xml version="1.0" encoding="UTF-8"?>
<!--
 Description:
 A template ops_config.xml file, this file shall be put on run directory of all applications that wants to use these topics.
-->
<root>
    <ops_config type = "DefaultOPSConfigImpl">
        <domains>
            <element type = "Domain">
                <domainID>HelloDomain</domainID>
                <domainAddress>234.5.6.8</domainAddress>
                <topics>
                    <element type = "Topic">
                        <name>RequestHelloTopic</name>
                        <port>6686</port>
                        <dataType>hello.RequestHelloData</dataType>
                    </element>
                   <element type = "Topic">
                        <name>HelloTopic</name>
                        <port>6687</port>
                        <dataType>hello.HelloData</dataType>
                    </element>
                </topics>
            </element>
        </domains>
    </ops_config>
</root>
```
ops.Request has one public field which will be used in the background to match replies with the request
```
string requestId;
```
And ops.Reply has three public fields, which are used for matching and to signal if the request was accepted, and if not a message where the requester can get information about what went wrong.
```
string requestId;
boolean requestAccepted;
string message;
```

You generate your code just like usual and you can use the generated classes for publishing and subscribing just like you are used to if you like.

But, to get some help with the request/reply part, you can use a helper class offered by OPS called *RequestReply*. Usage of this class is best described by an example. Assumed that you generated code for the IDL above, this is how to use the *RequestReply* class to send a request for a Hello message over and over again from C++:

```
using namespace ops;

//Create a Participant (i.e. an entry point for using ops.), compare with your ops_config.xml
ops::Participant* participant = Participant::getInstance("HelloDomain");
if (participant == nullptr) {
	std::cout << "Create participant failed. Do you have ops_config.xml on your run directory?" << std::endl;
	Sleep(10000); exit(1);
}

//Add type support for our types, to make this participant understand what we are talking
participant->addTypeSupport(new HelloRequestReply::HelloRequestReplyTypeFactory());

//Create topics for the request and reply
Topic requestTopic = participant->createTopic("RequestHelloTopic");
Topic replyTopic = participant->createTopic("HelloTopic");

//Create a RequestReply object with requestTopic and replyTopic + a key that globally defines this RequestReply object.
ops::RequestReply<hello::RequestHelloData, hello::HelloData> requestReplyHelper(requestTopic, replyTopic, "req_rep_instance1");

while (true) {
  hello::HelloData* reply = nullptr;
  hello::RequestHelloData request;
  request.requestersName = "C++ Requester";

  //Send the request and wait for reply 1000 milliseconds.
  //Notice that the reply is a copy of the underlying data
  //and you need to delete it when you no longer want to use it.
  reply = requestReplyHelper.request(&request, 1000);

  if (reply != nullptr) {
    if (reply->requestAccepted) {
      std::cout << "Reply received and request was accepted: " << reply->helloString  <<  std::endl;
    } else {
      std::cout << "Request was not accepted." <<  std::endl;
    }
    delete reply;
  } else {
    std::cout << "No reply." << std::endl;
  }
  Sleep(1000);
}
```
On the reply side, all we need to do is to implement a subscriber which subscribe to request and send replies as appropriate. Example in Java here:
```
Participant participant = Participant.getInstance("HelloDomain");
participant.addTypeSupport(new HelloRequestReplyTypeFactory());

final RequestHelloDataSubscriber subscriber = new hello.RequestHelloDataSubscriber(participant.createTopic("RequestHelloTopic"));
final HelloDataPublisher helloDataPublisher = new HelloDataPublisher(participant.createTopic("HelloTopic"));
subscriber.addObserver(new Observer()
{
  public void update(Observable sub, Object o)
  {
    RequestHelloData request = subscriber.getData();
    System.out.println("Request received from " + request.requestersName + ".");
    //Create a reply
    HelloData helloData = new HelloData();
    //Set the requestId of the reply to the requstId of the reply
    helloData.requestId = request.requestId;
    helloData.requestAccepted = true;
    helloData.helloString = "Hello " + request.requestersName + "!!";
    //Set the key of this publication to the key of the request
    helloDataPublisher.setKey(request.getKey());
    helloDataPublisher.write(helloData);
  }
});
```
That's all that's needed to create request/reply over OPS, and what is nice about it is that because it uses normal OPS topics under the hood, you can debug and use them just as normal topics whenever you want.
