
Outdated information

---

This page is intended to be a place where developers can summarize what they want to change in OPS 4.x

When applicable, file Issues for the bullets on this page and tag them 4.x.

### API Changes ###
  * participant.addTypeSupport() will be removed from normal usage and will be handled automatically by generated subscribers as needed.
  * add methods getInboundRate and getOutboundRate to subscribers and publishers.
  * clone methods in java
  * object printers based on configuration lib

### Transports ###
  * automatically chosen "in process" transport for java when applicable

### Protocol ###
  * Type hash codes based on serialization metainfo to ensure serialization will not fail.

```
   class Hatt
   {
       int myInt;
       string myString;
       double myDouble;
       MyType myType;
   }
```

could yield e.g. (at code generation time rather than runtime)

```
       int hash = ("i" + "s" + "d" + MyType.hashString()).hashCode()

```

### IDL ###
  * Enums
