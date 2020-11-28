# Subscribing To Topics #

Given an IDL project FooProject with a data type FooData used by a topic FooTopic on a domain FooDomain this is how you set up a subscription and receive data:

There are two basic ways of subscribing to data in OPS, by registering a listener that will be notified when new data arrives, or by polling the subscriber for data.

## Java ##

In Java, no matter what way you use to receive the data the setup of a subscription is always the same and look as follows:

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

sub.start();
```

If we want to poll the subscriber for data the most simple approach we can have is:

```
FooData data = null;
data = sub.getData();
if (data != null) {
    //Do something with the data...
}
```

Data will be null until we receive any data.

If we want to use a listener approach we can use Java's Observer classes in the following way (in this example the Observer is implemented inline):

```
sub.addObserver(new Observer()
{
    public void update(Observable o, Object arg)
    {
        FooData data = (FooData)arg;
        //Do something with the data...
    }
});

```

It is also possible to have an application thread wait for incoming data and poll it as soon as data is available, the following lines will wait for data for 1000 milliseconds and return a data (!null) only if a new sample arrives within the timeout:

```
FooData data = null;
data = sub.waitForNextData(1000);
if (data != null) {
    //Do something with the data...
}

```


## C++ ##

Just as in Java, no matter what way (polling or listening) you use to receive the data in C++ the setup of a subscription is always the same and look as follows:

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

FooDataSubscriber sub(topic);

sub.start();
```

If we want to poll the subscriber for data the most simple approach we can have is:

```
FooData data;
if (sub.newDataExist()) {
  if (sub.getData(&data)) {
    //Do something with the data...
  }
}
```

newDataExist() will return false until we receive new data.

If we want to use a listener approach we can use OPS DataListener and DataNotifier classes by creating a listener class in the following way:

```
class MyListener : public ops::DataListener
{
   FooDataSubscriber* fooSub;
   MyListener(FooDataSubscriber* s)
   {
       fooSub = s;
       fooSub->addDataListener(this);
   }
   ///Override from ops::DataListener
   void onNewData(ops::DataNotifier*)
   {
      FooData data;
      if (fooSub->getData(&data)) {
          //Do something with the data...
      }    
   }
}
```

and in addition to your init code above register this listener with the subscriber

```
MyListener listener(&sub);

```

In a similar way as for the Java Observer example above, we can use C++ lamdas to setup a listener instead of creating a class inheriting from the DataListener

```
sub.addDataListener(
    [&](ops::DataNotifier* /*subscriber*/) {
        // Get the actual data object published
        FooData* data = sub.getTypedDataReference();
        //Do something with the data...
    }
```

It is also possible to have an application thread wait for incoming data and poll it as soon as data is available, the following lines will wait for data for 1000 milliseconds and return true only if a new sample arrives within the timeout:

```
FooData data;
if (sub.waitForNewData(1000)) {
      sub.getData(&data);
      //Do something with the data...
}
```
