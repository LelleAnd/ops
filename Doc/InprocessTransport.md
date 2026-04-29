# In-process Transport #

The In-process transport of OPS is similar to Multicast Transport, but messages are only distributed within the same program. One use case is in test code (unit/behaviour tests) to drive input to subscribers resp. catch output from publishers. E.g. see the _UnitTestExample_ in the C++ Examples folder.

Note that Subscriber callbacks in this case, are executed directly by the thread publishing the message.

To use the In-process transport for a topic, this is how to setup your [Topic Config](OpsConfig.md) file:

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
                        <!-- Required for inprocess Transport-->
                        <name>FooTopic</name>
                        <dataType>foopackage.FooData</dataType>
                        <transport>inprocess</transport>
                    </element>
                    <!-- TODO: Add more topics here... -->
                </topics>
            </element>
        </domains>
    </ops_config>
</root>

```
As you can see, the field transport must be set to "inprocess".
