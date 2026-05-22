[Examples](SimpleCpp.md) [BuildInstructions](BuildInstructions.md)

---

# OPS IDL Builder #
Learn more about the OPS IDL language [here](IDLLanguage.md).

To edit IDL files and to compile these files into target languages, OPS comes with a tool called **OPS IDL Builder**. The **OPS IDL Builder** can be seen as an IDE for OPS projects.

Prerequisites for compiling IDL files with the tool **OPS IDL Builder**, is to define the environment symbol **OPS_OPSC_PATH** with the path to the OPS commandline compiler [opsc](IDLCommandlineCompiler.md), on windows **opsc.bat** and on Linux **opsc.sh**, e.g.:
```
set OPS_OPSC_PATH=D:\OPS\OPS4\Tools\opsc
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

class SimpleData :
	public ops::OPSObject
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("samples.SimpleData");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

    // Defined to be able to ensure that all generated classes have the reworked copy constructor
    using samples_SimpleData_new_copycons = bool;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("samples.SimpleData"); }

    static const uint8_t SimpleData_idlVersion = 0;

    uint8_t SimpleData_version = SimpleData_idlVersion;

    int i{ 0 };
    double d{ 0 };
    std::string s;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    SimpleData() : SimpleData(std::string_view(_inheritDesc)) {}

protected:
    SimpleData(std::string_view tName)
        : ops::OPSObject(tName)
    {
#else
    SimpleData()
        : ops::OPSObject()
    {
        OPSObject::appendType(ops::TypeId_T("samples.SimpleData"));
#endif
    }

#ifdef OPS_C17_DETECTED
public:
#endif
    ///Copy-constructor making full deep copy of a(n) SimpleData object.
    SimpleData(const SimpleData& _c)
       : ops::OPSObject(_c)
    {
        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) SimpleData object.
    SimpleData& operator = (const SimpleData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    SimpleData(SimpleData&& other) noexcept : ops::OPSObject(std::move(other))
    {
        SimpleData_version = std::move(other.SimpleData_version);
        i = std::move(other.i);
        d = std::move(other.d);
        s = std::move(other.s);
    }

    // Move assignment operator taking other's resources
    SimpleData& operator= (SimpleData&& other) noexcept
    {
        if (this != &other) {
            ops::OPSObject::operator=(std::move(other));
            SimpleData_version = other.SimpleData_version;
            i = other.i;
            d = other.d;
            s = std::move(other.s);
        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("SimpleData_version", SimpleData_version);
            ValidateVersion("SimpleData", SimpleData_version, SimpleData_idlVersion);
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
        if (this == obj) { return; }
        ops::OPSObject::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(SimpleData* obj) const
    {
        obj->SimpleData_version = SimpleData_version;
        obj->i = i;
        obj->d = d;
        obj->s = s;
    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && ops::OPSObject::isValid();
        _valid = _valid && (SimpleData_version == SimpleData_idlVersion);
		return _valid;
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
