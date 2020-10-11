# Handling Message References #

By default, as soon as a new sample arrives to a subscribing application, the previous sample is overwritten and its reference is no longer valid.
For example if you save references to messages in your DataListener, these references will be invalid when new data arrives and may therefore cause your application to crash if you try to use these references:

```
///Overide from DataListener, called from OPS receive thread whenever new data arrives.
void onNewData(DataNotifier* notifier)
{
   OPSMessage* mess = sub->getMessage();

   //Save the reference in a thread safe collection to be used later.
   myQ.push_back(mess);
}

///Method called from application thread that prints all messages in myQ
void printMessages()
{
   OPSMessage* mess;
   while ((mess = myQ.get()) != nullptr) {
      //NOTE: This will cause errors, references might no longer be valid
      print(mess);
   }
}
```

The code above will not work as soon as more then one sample has arrived between two calls to printMessages().
Of course, you can copy your data in onNewData before putting it in to your list, and the problem will be solved. This is OK for most applications, but when you have an application dealing with large amounts of data, the copying can be deemed too unefficient.
To deal with this problem OPS offers a reference counting mechanism to mark messages not to be deleted until you say it is OK. This is done by calling reserve() and unreserve() on OPSMessage:

```
///Override from DataListener, called from OPS receive thread whenever new data arrives.
void onNewData(DataNotifier* notifier)
{
   OPSMessage* mess = sub->getMessage();
   mess->reserve();

   //Save the reference in a thread safe collection to be used later.
   myQ.push_back(mess);
}

///Method called from application thread that prints all messages in myQ
void printMessages()
{
  OPSMessage* mess;
  while ((mess = myQ.get()) != nullptr) {
      print(mess);

      //This will state that you no longer have use for this messages, and it will be
      //deleted as soon as no one else has it reserved.
      mess->unreserve();
   }
}
```
