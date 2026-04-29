# Shared Memory Transport #

The Shared Memory Transport of OPS is a transport mechanism within a node (localhost) using named shared memory between processes. The implementation uses the Boost interprocess library and works only on OS:es where this Boost library exist. To compile OPS without Shared Memory support, define OPS_NO_SHMEM_TRANSPORT, see [C++ configurations](../Cpp/Readme.md).

The Shared Memory transport can be configured in two different ways:

* Without metadata enabled, a fixed name is used for the shared memory and it is therefore a _one-to-many_ transport mechanism.

* With metadata enabled, OPS uses the metadata sent by participants to get the publishers process ID and use this as part of the naming of the shared memory. This is a _many-to-many_ transport mechanism.

To use the Shared memory transport for a topic, this is how to setup your [Topic Config](OpsConfig.md) file:

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
                <metaDataMcPort>9494</metaDataMcPort>
                <channels>
                    <element type = "Channel">
                        <!-- Shared Memory transport, ie. only within node (localhost) -->
                        <name>Channel-SHMEM</name>
                        <linktype>shmem</linktype>
                        <outSocketBufferSize>130000000</outSocketBufferSize>
                    </element>
                </channels>
                <transports>
                    <element type = "Transport">
                        <channelID>Channel-SHMEM</channelID>
                        <topics>
                            <element>VessuvioTopic2</element>
                            <element>VessuvioTopic3</element>
                        </topics>
                    </element>
                </transports>
                <topics>
                    <element type = "Topic">
                        <name>VessuvioTopic2</name>
                        <dataType>pizza.VessuvioData</dataType>
                    </element>
                    <element type = "Topic">
                        <name>VessuvioTopic3</name>
                        <dataType>pizza.VessuvioData</dataType>
                    </element>
                    <!-- TODO: Add more topics here... -->
                </topics>
            </element>
        </domains>
    </ops_config>
</root>

```
As you can see, the example defines _channels_ and _transports_ to separate transport information from the specification of _topics_. The field _linktype_ must be set to "shmem". 

The field _outSocketBufferSize_ specifies in this case how large the Shared Memory is.

## Caveats when using shared memory transport ##
* It's very important to do proper cleanup/delete of _Publishers_, _Subscribers_ and _Participants_ before the program exits. If not done, other programs using the same named shared mamory may hang when they are trying to publish data.

* Resources/Files used by the Shared Memory (ie. the Boost library) may be left on disk taking up space if programs isn't cleaned up correctly. On Windows normally in _C:\ProgramData\boost_interprocess\..._. On Linux in _/dev/shm/..._.
