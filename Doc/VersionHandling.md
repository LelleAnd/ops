# Version handling of OPS IDLs #

What strategy should one use when existing IDL classes need to have additional information?

Is it possible to keep backward compatibility?

When you have defined some IDL's and started to use them from different programs and then need to add additional information or change/remove existing fields, there are some different aspects to think about.

Can we recompile everything and deploy the complete system at once again? I.e. do we have any thing to be backward compatible with.

Are there e.g. any stored binary data using the OPS serialized format, e.g. in some persistence service you may have written.

The following are some different alternatives you can use when extending a system.

## Add new IDL classes and Topics ##

One alternative is to just add more Topics using new IDL classes and keep the old ones unchanged.
This leads to that new Publishers and Subscribers need to be set up and used.


## Extending existing IDL classes on existing Topics ##

Another alternative is to use the OPS inheritance, see [IDL language](IDLLanguage.md), and extend existing IDL classes for the existing Topics. This does only affect the Publishers and Subscribers that need the new data, so the recompile can be limited. See also [Data class slicing and spareBytes](DataSlicing.md) that describes a use case for this.

This is the prefered way providing full backward compatibility but it is not always practical/possible, since the extending only can be done on the most specialized IDL classes (i.e. the leafs in inheritance hierarchy). Extending a base class, i.e. inserting a new class in the hierachy, will automatically break backward compatibility.


## Version tagging of separate fields in existing IDL classes ##

A third alternative is to use version tagging of fields, see description of directives in [IDL language](IDLLanguage.md#compile-directives), which makes it possible to add, change or remove fields from beeing [de]serialized for different versions of the IDL.

If a field in an IDL is version tagged, each class in the complete inheritance hierarchy is extended with an implicit version field that is [de]serialized. To indicate to the receiver that this implicit version field exists, the sent *data type* is tagged with a leading "0 ", see also the [binary protocol](Protocol.md) description. This tagging of the *data type* gives us only some backward compatibility. New binaries aware of version tagging can send and receive both version tagged and non version tagged data, but old binaries will fail to receive data if it is version tagged.

When version tagged fields are used, there are some implicitly generated fields in the class available to the application:
* **idlVersionMask**, set to 0 to NOT use version tagging and != 0 to use version tagging. When set to 0, only the fields valid for **classname_version** = 0 is used.
* **classname_idlVersion**, generated constant(s) with the highest version number known for the idl
* **classname_version**, if version tagging is used, the current version of the class to send or that was received. Defines which fields that are valid and need to be set before sending and checked after receiving.
