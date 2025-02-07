![C/C++ CI development](https://github.com/LelleAnd/ops/actions/workflows/C/C++%20CI/badge.svg?branch=development)
![C/C++ CI xmltest](https://github.com/LelleAnd/ops/actions/workflows/C/C++%20CI/badge.svg?branch=xmltest)
# Welcome to OPS - Open Publish-Subscribe #


---

See [Build Instructions](Doc/BuildInstructions.md) or [User Guide](Doc/UserGuide.md) to get started! and [Release Notes](ReleaseNotes.md) for a development overview.


---


# Introduction #

OPS primarily targets distributed real-time and embedded (DRE) systems and distributed control systems (DCS).

OPS aims to offer asynchronous, efficient and type secure publish-subscribe communication between processes running on multiple platforms written in multiple programming languages.

The communication model of OPS is a typed topic publish-subscribe model. That means that processes that take part in the OPS communication can act as publishers or subscribers to different topics. Data published on a certain topic will be delivered to all subscribers to that topic in a way that hides the publishers from the subscribers and vice versa. By ensuring this anonymity the middleware allows for low coupling between different system participants which enables software reuse, modularity and redundancy among other desirable properties.

As mentioned above, OPS offers typed topics, which in addition to connecting publishers and subscribers also dictates the format of the data that is sent between them. The messages sent in OPS are Data classes that are defined by the system developers in a data definition language called OPS IDL. These data classes may be complex constructs, with inheritance, dynamic arrays, strings, classes of classes and arrays of classes. These Data classes are used to auto generate source code, for different target programming languages, that allows for communicating this data in a simple and secure way. The complexity of serializing and deserializing these classes and to be able to send them across the network is completely hidden from the participants, and the classes can be used in an object oriented and type safe way in the application layer of the participants.

Presently OPS is available in Java (Windows, Linux, Unix, Mac), C++ (Windows, Linux), C# (Windows), Delphi (Object Pascal, Windows), Ada (Windows, Linux) and Python (2.7 and 3.x) programming languages. OPS can also be used under Matlab through the Java Matlab bridge available with Matlab.


## Basic Principles ##

  * Participants (i.e. applications residing in computer nodes) publish and subscribe on Typed topics
  * Decentralized architecture - there is no central service, each Participant carry enough of the middleware to "participate", i.e. communicate with each other.
  * Communication between publishers and subscribers is asynchronous and anonymous.
  * Networking and protocol complexity is hidden from application developers.


![pub_sub_overview.jpg](Doc/pub_sub_overview.jpg)

Head on to the [User Guide](Doc/UserGuide.md) to learn more...
