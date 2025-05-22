[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

# Basic C++ Example #

Given the following type defined in [OPS IDL](IDLLanguage.md):

```
package foo;

class FooData
{
    string fooText;
}
```

And a topic FooTopic for that data type.

This is how you create a subscriber

```
Participant* participant = Participant::getInstance("FooDomain");
participant->addTypeSupport(new Foo::FooTypeFactory());

Topic topic = participant->createTopic("FooTopic");

FooDataSubscriber sub(topic);

...
```

This is how you create a publisher

```
ops::Participant* participant = Participant::getInstance("FooDomain");
participant->addTypeSupport(new Foo::FooTypeFactory());

Topic topic = participant->createTopic("FooTopic");

FooDataPublisher pub(topic);

...
```
---
See also [Example1.cpp](../Examples/CppApps/Example1/Example1.cpp) in the Examples folder.