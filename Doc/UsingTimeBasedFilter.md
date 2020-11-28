# Using Time Based Filters #

Another useful feature when subscribing to a topic is to use something called Time Based Filter QoS. This allows you to tell the OPS core that your subscriber wants data with at least a certain time separation between two consecutive samples. This is useful in situations when you want to subscribe to topics that are published with a high rate but you are only interested in the same data at a lower rate.

This saves both CPU power and simplifies your data handling.

Setting up a Time Based Filter QoS is easy and is done in the same way in both C++ and Java. The following shows how to tell the subscriber to only get samples at a maximum rate of 1Hz:

```
sub.setTimeBasedFilterQoS(1000) //Minimum separation between to samples in milliseconds
```

Setting the Time Based Filter QoS applies to both polling and listener approaches (See SubscribingToTopic).

The Time Based Filter QoS can be updated at any time.
