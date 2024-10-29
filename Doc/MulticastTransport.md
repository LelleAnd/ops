# Multicast Transport #


---

_Note, this is the default transport mechanism for OPS4._

---

By default, messages communicated with OPS are sent over an IP multicast transport (Multicast Transport). Communication model is simple, each topic belongs to a combination of a multicast address and a port. Both multicast address and port can be used by any number of topics, but separating them into different address/port combinations can greatly improve performance and network usage.

When you create a subscriber and start it, OPS will add a multicast membership to the address/port attached to the topic of the subscriber and start listening for messages on that port. Whenever data is received, the communication layer of OPS will check if the messages is intended to the subscriber and if it is, deserialize the data into an object of the data class of the topic and deliver it to your subscriber.

In the same way, a publisher will setup a multicast communication with the parameters of your topic and when ever you issue a write on the publisher serialize the data into bytes and send them with UDP messages to the multicast address/port of the topic.

Let's have a look at the topic config file and how it can be used to control the OPS traffic for the Multicast Transport:

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
                <domainID>FooDomain</domainID>
                <domainAddress>234.5.6.8</domainAddress>
                <localInterface>127.0.0.1</localInterface>
                <topics>

                    <element type = "Topic">
                        <name>FooTopic1</name>
                        <port>7000</port>
                        <dataType>foopackage.FooData</dataType>
                    </element>

                    <element type = "Topic">
                        <name>FooTopic2</name>
                        <port>7001</port>
                        <dataType>foopackage.FooData</dataType>
                        <address>234.5.6.9</address>
                    </element>

                    <element type = "Topic">
                        <name>FooTopic3</name>
                        <port>7001</port>
                        <dataType>foopackage.FooData</dataType>
                        <sampleMaxSize>60000</sampleMaxSize>
                        <inSocketBufferSize>1000000</inSocketBufferSize>
                        <outSocketBufferSize>1000000</outSocketBufferSize>
                        <transport>multicast</transport>
                        <address>234.5.6.9</address>
                    </element>
                    <!-- TODO: Add more topics here... -->
                </topics>
            </element>

        </domains>
    </ops_config>
</root>

```
In the file above we have configured one domain called FooDomain. The fields **domainAddress** and **localInterface** are of interest for the Multicast Transport. The **domainAddress** is the address that will be used to communicate by default for all topics on this domain.
The domain address must be the same for all applications in the system to make them communicate.

The next field **localInterface** is optional (the first interface found on the operating system will be used if it is not specified) and tells the Multicast Transport which local network interface (specified by IP address or subnet) to use when communicating over multicast. This setting can be different for different applications in an OPS system. Especially if localhost "127.0.0.1" is used as in the example above, communication will stay in the local machine.

Further more, the domain is made up of three different topics: FooTopic1, FooTopic2 and FooTopic3. They all have the same data type, but differs in configuration of the communication.

**FooTopic1** has the minimal set of configuration needed for a multicast topic: **name**, **port** and **dataType**. FooTopic1 have no address explicitly set and will thus be communicated over the multicast address specified by the **domainAddress**. The port number will be 7000.

**FooTopic2** has an **address** field specified to another multicast address then the domain address and this will override the domain address for FooTopic2 which will be communicated on 234.5.6.9/7001 in this case.

**FooTopic3** has the same address/port combination as FooTopic2 which is perfectly valid but will yield a receiver side filtering in applications if there exist a subscriber for one but not the other of the two topics. It is often a good idea to only use the same address/port combination for topics that go close together, i.e. if an application subscribes to one of the topics, it is likley that it also subscribes to the other. Moreover, to save ports (which is a limited resource of the operating system) it can be a good idea to have the same address/port combination for topics with low rates and sample sizes.

In addition to **name**, **port**, **address** and **dataType**, the configuration of FooTopic3 also specifies other parameters that affect the Multicast Transport. **inSocketBufferSize** and **outSocketBufferSize** tells OPS to try to set the buffer sizes of the underlying sockets to the specified value. The maximum values for these settings depend on the platform and OPS will only report a warning but continue if the buffer size can not be adjusted to the specified value.
The **sampleMaxSize** parameter tells OPS what it can expect as the maximum size of a data sample of the given topic. This will influence how much memory OPS use for data buffers. OPS will not accept publications of messages larger then the value in bytes specified by sampleMaxSize. See also [Sending Large Messages](LargeMessages.md).

See also, [Topic Config](OpsConfig.md).
