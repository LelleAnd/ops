# Binary Protocol #
When data is sent over OPS, it is always packaged in something called OPSMessage, see [Understanding OPSObject and OPSMessage](OpsMessage.md). The OPSMessage is then serialized and split up into one or more *segments* where each *segment* is limited to a maximum of 60000 bytes, see also [Sending Large Messages > 60000 bytes](LargeMessages.md).

To see how individual data types are serialized, see [Defining Data Types in IDL](IDLLanguage.md).

All data is serialized using Little Endian byte order.

## Segments ##
The serialized data is split into one or more *segments* as described above.

To be able to correctly reassemble *segments* into a complete message, each *segment* starts with a *segment header* (14 bytes):

```
Field                 Type          Bytes
-----                 ----          -----
protocol id                         4        = "opsp"
version               short         2        currently = 5
total # segments      int           4
segment number        int           4
```
The header is directly followed by the serialized data from the message, max 60000-14 bytes. The rest of the serialized data, if any, continues in a new *segment*.

## OPSMessage ##
An OPSMessage is serialized into the following:
```
Field                 Type          Bytes
-----                 ----          -----
data type             string        4 + n    n = length of 'data type', see note below.
                                             Here 'data type' = "ops.protocol.OPSMessage "
=OPSObject=
  <version>           byte          1        optional version, see 'data type' note below
  key                 string        4        (always = "")
=OPSMessage=
  <version>           byte          1        optional version, see 'data type' note below
  message type        byte          1
  pub prio            byte          1
  pub identification  int64         8
  pub name            string        4 + n    n = length of Publisher name, if any set
  topic name          string        4 + n    n = length of Topic name
  top level key       string        4        (always = "")
  address             string        4        (always = "")
  data                OPSObject*    
    data type         string        4 + n    n = length of 'data type', see note below
    =OPSObject=
       <version>      byte          1        optional version, see 'data type' note below
       key            string        4 + n    n = length of 'key', see note below
    =DerivedObject=
       <version>      byte          1        optional version, see 'data type' note below
       fields         ...           ...      depends on sent object
```
** Note: 'data type' **

The data type field is a concatenation of all data types (IDL class names) in the inheritance hierarchy for the message sent (datatypes are separated by space characters). If element _optNonVirt_ is set to true in the _Domain_ section of the [OPS configuration file](OpsConfig.md), the string for the data type is sent as an empty string for non-virtual fields, see also the [IDL language](IDLLanguage.md) description.

If an IDL class uses version directives for fields (*//@*), see the [IDL language](IDLLanguage.md) description, the data type (possibly empty string) is prefixed with a tag ("0 ") to indicate that there is implicit version fields for each IDL class in the inheritance hierarchy. These implicit version fields are in this case sent first for each IDL class. The tag is removed by the OPS core during receiving. See also [version handling](VersionHandling.md) of OPS IDLs.

** Note: 'key'**

The key field sent is the Publisher key if it has been set. If it hasn't been set, the key will be the sent message key, which defaults to "" if not set by the user.

## Transport specifics ##

### TCP protocol version 1 ###
The original TCP protocol only sends data in one direction, from the Publisher (TCPServer) to the Subscribers (TCPClients).

When *TCP* is used as transport, each segment is preceded by the following *TCP* unique header (22 bytes):

```
Field                 Type          Bytes
-----                 ----          -----
protocol id                         18       = "opsp_tcp_size_info"
length of data        int           4        size of segment
```

### TCP protocol version 2 ###
Version 2 adds heartbeats in both directions to enable faster detection of a broken link. To be backward compatible heartbeats are only enabled if both sides support protocol version 2 or greater.

The version detection is initiated by the TCPClient sending a *Probe message*. A v1 TCPServer don't read anything, so the message will be ignored. A v2 or greater TCPServer reacts by sending a *Heartbeat message*. When the TCPClient gets the *Heartbeat message*, both sides know the version on the other side.

Both sides shall send a *Heartbeat message* if no other data is sent since 1 second (default value, can be configured in the OPS [configuration file](OpsConfig.md)).
The link shall be determined as broken and closed if no bytes are received during 3 seconds (default value, can be configured in the OPS [configuration file](OpsConfig.md))

**Probe Message** (23 bytes)
```
Field                 Type          Bytes
-----                 ----          -----
protocol id                         8        = "opsprobe"
protocol version      int           4        = 2
protocol spare                      6        = "______"
length of data        int           4        = 1
data                  byte          1        = 0
```

**Heartbeat Message** (22 bytes)
```
Field                 Type          Bytes
-----                 ----          -----
protocol id                         8        = "opsprobe"
protocol version      int           4        = 2
protocol spare                      6        = "______"
length of data        int           4        = 0
```
