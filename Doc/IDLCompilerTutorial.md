[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

---

# OPS IDL Builder #
Learn more about the OPS IDL language [here](IDLLanguage.md).

To edit IDL files and to compile these files into target languages, OPS comes with a tool called **OPS IDL Builder**. The **OPS IDL Builder** can be seen as an IDE for OPS projects.

Prerequisites for compiling IDL files with the tool **OPS IDL Builder** is to define the  
environment symbol **OPS_OPSC_PATH** with the path to the **opsc.bat** file, e.g.:
```
OPS_OPSC_PATH=D:\OPS\OPS4\Tools\opsc
```

The OPS IDL Builder looks as follows, with some sample code open in a sample project:

![idlcompiler.jpg](idlcompiler.jpg)

With the OPS IDL Compiler you create projects containing possibly multiple IDL files with all the different data types you use in your project. To compile the IDL files press the 'hammer'-button.

Compiled to C++, this is what would be generated from the SimpleData data type above:

```
//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

namespace samples {

constexpr ops::VersionMask_T SimpleData_Level_Mask   = ops::OPSObject_Level_Mask << 1;
static_assert(SimpleData_Level_Mask <= ops::MaxVersionMask, "Inheritance hierarchy too large");

class SimpleData :
	public ops::OPSObject
{
public:
    static ops::TypeId_T getTypeName(){return ops::TypeId_T("samples.SimpleData");}

    char SimpleData_version = 0;

    int i;
    double d;
    std::string s;

    SimpleData()
        : ops::OPSObject()
        , i(0), d(0)
    {
        OPSObject::appendType(std::string("samples.SimpleData"));
    }

    ///Copy-constructor making full deep copy of a(n) SimpleData object.
    SimpleData(const SimpleData& __c)
        : ops::OPSObject()
        , i(0), d(0)
    {
        OPSObject::appendType(ops::TypeId_T("samples.SimpleData"));
        __c.fillClone(this);
    }

    ///Assignment operator making full deep copy of a(n) SimpleData object.
    SimpleData& operator = (const SimpleData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    void serialize(ops::ArchiverInOut* archive)
    {
        ops::OPSObject::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("SimpleData_version", SimpleData_version);
        } else {
            SimpleData_version = 0;
        }
        archive->inout("i", i);
        archive->inout("d", d);
        archive->inout("s", s);
    }

    //Returns a deep copy of this object.
    virtual SimpleData* clone() override
    {
        SimpleData* ret = new SimpleData;
        fillClone(ret);
        return ret;
    }

    void fillClone(SimpleData* obj) const
    {
        ops::OPSObject::fillClone(obj);
        obj->SimpleData_version = SimpleData_version;
        obj->i = i;
        obj->d = d;
        obj->s = s;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~SimpleData(void)
    {
    }
};

}
```

The IDL Compiler is also where you define your topics on which you will publish and subscribe to in your applications. This is done by creating a XML configuration file that looks something like this:

```
<root>
    <ops_config type = "DefaultOPSConfigImpl">
        <domains>
            <!-- Define a domain called FooDomain.-->
            <element type = "Domain">
                <domainID>SampleDomain</domainID>
                <domainAddress>234.5.6.8</domainAddress>
                <topics>
                    <!-- Define a Topic called SimpleTopic. -->
                    <element type = "Topic">
                        <name>SimpleTopic</name>
                        <port>8888</port>
                        <dataType>samples.SimpleData</dataType>
                    </element>
                </topics>
            </element>
        </domains>
    </ops_config>
</root>

```

See what the OPS IDL Builder looks like and how to create a new project here:

<a href='http://www.youtube.com/watch?feature=player_embedded&v=UsdjMwTUV3s' target='_blank'><img src='http://img.youtube.com/vi/UsdjMwTUV3s/0.jpg' width='425' height=344 /></a>
