# Serializing #

To serialize and deserialize OPSObjects to and from bytes to send on the network, OPS uses a serialize design inspired by boosts Serialize (http://boost.org). The idea is to have a method _serialize_ that excepts a visitor as argument. In the serialize method, the object calls a method on the visitor with each of its fields as inout/output parameters to that method. How and to what the fields are serialized is in that way completely transparent to the object itself. This is best illustrated by an example:

```
virtual void serialize(ops::ArchiverInOut* archive) override
{
  UserData::serialize(archive);
  if (idlVersionMask != 0) {
    archive->inout("ExtendedUserData_version", ExtendedUserData_version);
    ValidateVersion("ExtendedUserData", ExtendedUserData_version, ExtendedUserData_idlVersion);
  } else {
    ExtendedUserData_version = 0;
  }
  archive->inout(std::string("nickname"), nickname);
  archive->inout(std::string("email"), email);
  archive->inout(std::string("telephone"), telephone);
  archive->inout(std::string("profileImageUrl"), profileImageUrl);
}
```

The above method is taken from the generated C++ class ExtendedUserData from the ChatExample which extends UserData.

The visitor, ArchiverInOut, is a pure virtual interface and one of its implementations serialize the data to the bytes sent by OPS. Of course you are free to create your own implementations of this interface if you want to serialize your data to something else, e.g. a log message or to an ini-file. Note that the arguments to inout are references and therefor can both serialize and deserialize this object depending on the implementation of ArchiverInOut (hence the name).
