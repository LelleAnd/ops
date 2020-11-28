# Using Keys #

In addition to just subscribing or publishing on a topic, publishers and subscribers can choose to only subscribe on that topic if the data has a certain _key_.

Key is a field shared by all OPSObjects and can be accessed with getKey() and setKey() methods.

Key can be seen as a way for applications to dynamically add instances on a topic and set up subscriptions which are key specific.

This is how you create a subscriber that will only receive data on a certain key:

```
//Get a participant reference, this is your entry point to OPS
Participant participant = Participant.getInstance("FooDomain");
if (participant == null) {
    //Report error
    return;
}
//Add support for serializing/deserializing our data types
participant.addTypeSupport(new FooProject.FooProjectTypeFactory());

Topic topic = participant.createTopic("FooTopic");

FooDataSubscriber sub = new FooDataSubscriber(topic);

KeyFilterQoSPolicy myKeyFilter = new KeyFilterQoSPolicy("my_key")
sub.addFilterQoS(myKeyFilter);

sub.start();
```

No matter whether you use polling or a listener, you will only be able to get data from the subscriber with the key "my\_key". The key can be changed at any time though.

```
myKeyFilter.setKey("my_other_key");
```

The example above is in Java, but the same apply to C++.

On the publisher side, all that is required is to set the key on the data to be published, this time exemplified in C++:

```
//Get a participant reference, this is your entry point to OPS
Participant* participant = Participant::getInstance("FooDomain");
if (participant == nullptr) {
    //Report error
    return;
}
//Add support for serializing/deserializing our data types
participant->addTypeSupport(new FooProject::FooProjectTypeFactory());

Topic topic = participant->createTopic("FooTopic");

FooDataPublisher pub(topic);

//Create some data
FooData data;

data.setKey("my_key");

//Publish the data
pub.write(&data);
```
