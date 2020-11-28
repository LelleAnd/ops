# Reliability in transports #

To have reliability in OPS communication you can use [TCP](TcpTransport.md) as the transport.

Both [UDP](UdpTransport.md) and [Multicast](MulticastTransport.md) transports can drop messages in the network stack and/or the communication equipment between nodes, without messages beeing resent or any notification given.

Besides using TCP, OPS has some other features to help in cases where TCP can't be used for some reason.

## Detection of lost messages ##

In OPS each *Publisher* has its own publication counter, *publicationID*, that is put in the [OPSMessage](OpsMessage.md) meta data and incremented by one for each published message. The *Subscriber* can check the *publicationID* to detect any data losses and act upon it. In C++ there also exist a *PublicationIdChecker* class that can be instantiated and given to the *Subscriber* to do such a check.

With this technic you can only detect that a message has been lost when another message is received. It can't detect that a message is lost if no more messages from that *Publisher* is sent, e.g. messages for a single events.

## Using Acknowledge messages ##

Another feature, currently only available in C++, is to enable Ack's on a *Topic*, see [configuration](OpsConfig.md). When using this feature you configure the *Topic* to use Ack's, configure the maximum number of resends and the minimum time between resends.

With this enabled, the *Subscriber* sends an acknowledge for each received message. If either the message or the acknowledge is lost, the *Publisher* will resend the message with the same *publicationID* so that a *Subscriber* can detect if it already has received the message.

The *Publisher* has no queue of messages to send, it only keep the latest message in a buffer. If the *Publisher* sends a new message before the previous one has been acknowledged by all *Subscribers*, the new message will replace the old one and the *Subscriber* need to check the *publicationID* to detect if a message was lost.

The acknowledge *Topic* is implicitly defined and use the same transport mechanism as the *Topic* it acknowledges.

Currently Ack's are only available for [Multicast](MulticastTransport.md) transports.

## Request-Reply mechanism ##

Another way is to use the Request-Reply mechanism described [here](RequestReply.md).
