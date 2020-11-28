# Receiver side filter #

It is simple to create your own filters for receiver side filtering of incoming data and have it work in the same manner as KeyFilterQoSPolicy (See [Using Keys](UsingKeys.md)).

Say you want to create a filter which accepts all publications from publishers with a name that start with a certain prefix. This is how you can do it in Java using your own FilterQoSPolicy implementation (C++ works more or less in the same way, except that you cannot declare the filter inline in the same way):

```
String pubNamePrefix = "fooPrefix";

sub.addFilterQoSPolicy(new FilterQoSPolicy()
{
   public boolean applyFilter(OPSObject o)
   {
      OPSMessage m = (OPSMessage)o;
      if (m.getPublisherName().startsWith(fooPrefix)) {
         return true;
      } else {
         return false;
      }
   }
});
```

Of course you are not restricted to use only publisherName or key, lets say if FooData has a string field fooText, you could have used that one instead if you cast the data from m.getData() to FooData, or use what ever you like criteria for having the filter accept or reject the data. Note that applyFilter method has an OPSMessage as its parameter, you can read more about OPSMessage and OPSObject [here](OpsMessage.md).
