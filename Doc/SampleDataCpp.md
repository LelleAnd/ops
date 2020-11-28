
```
//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

#include "UserData.h"

namespace samples {

class SampleData :
	public ops::OPSObject
{
public:
  	static ops::TypeId_T getTypeName(){return ops::TypeId_T("samples.SampleData");}

  	char SampleData_version = SampleData_idlVersion;

    enum class Order {
        UNDEFINED, START, STOP
    };

    static const char SampleData_idlVersion = 0;

    static const int max = 42;
    bool boo;
    char b;
    short sh;
    int i;
    int64_t l;
    float f;
    double d;
    std::string s;
    ops::strings::fixed_string<25> s25;
    UserData uData;
    Order command;
    std::vector<bool> boos;
    std::vector<char> bytes;
    std::vector<short> shorts;
    std::vector<int> ints;
    std::vector<int64_t> longs;
    std::vector<float> floats;
    std::vector<double> doubles;
    std::vector<std::string> strings;
    std::vector<ops::strings::fixed_string<43>> s43vect;
    std::vector<UserData> uDatas;
    int intarr[42];

    ///Default constructor.
    SampleData()
        : ops::OPSObject()
        , boo(false), b(0), sh(0), i(0), l(0), f(0), d(0), command(Order::UNDEFINED)
    {
        OPSObject::appendType(ops::TypeId_T("samples.SampleData"));
        memset(&intarr[0], 0, sizeof(intarr));
    }

    ///Copy-constructor making full deep copy of a(n) SampleData object.
    SampleData(const SampleData& __c)
       : ops::OPSObject()
        , boo(false), b(0), sh(0), i(0), l(0), f(0), d(0), command(Order::UNDEFINED)
    {
        OPSObject::appendType(ops::TypeId_T("samples.SampleData"));
        memset(&intarr[0], 0, sizeof(intarr));

        __c.fillClone(this);
    }

    ///Assignment operator making full deep copy of a(n) SampleData object.
    SampleData& operator = (const SampleData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("SampleData_version", SampleData_version);
            ValidateVersion("SampleData", SampleData_version, SampleData_idlVersion);
        } else {
            SampleData_version = 0;
        }
        archive->inout("boo", boo);
        archive->inout("b", b);
        archive->inout("sh", sh);
        archive->inout("i", i);
        archive->inout("l", l);
        archive->inout("f", f);
        archive->inout("d", d);
        archive->inout("s", s);
        archive->inout("s25", s25);
        archive->inout("uData", uData);
        archive->inoutenum("command", command);
        archive->inout("boos", boos);
        archive->inout("bytes", bytes);
        archive->inout("shorts", shorts);
        archive->inout("ints", ints);
        archive->inout("longs", longs);
        archive->inout("floats", floats);
        archive->inout("doubles", doubles);
        archive->inout("strings", strings);
        archive->inout("s43vect", s43vect);
        archive->inout<UserData>("uDatas", uDatas, UserData());
        archive->inoutfixarr("intarr", &intarr[0], 42, sizeof(intarr));
    }

    //Returns a deep copy of this object.
    virtual SampleData* clone() override
    {
        SampleData* ret = new SampleData;
        fillClone(ret);
        return ret;
    }

    void fillClone(SampleData* obj) const
    {
        ops::OPSObject::fillClone(obj);
        obj->SampleData_version = SampleData_version;
        obj->boo = boo;
        obj->b = b;
        obj->sh = sh;
        obj->i = i;
        obj->l = l;
        obj->f = f;
        obj->d = d;
        obj->s = s;
        obj->s25 = s25;
        obj->uData = uData;
        obj->command = command;
        obj->boos = boos;
        obj->bytes = bytes;
        obj->shorts = shorts;
        obj->ints = ints;
        obj->longs = longs;
        obj->floats = floats;
        obj->doubles = doubles;
        obj->strings = strings;
        obj->s43vect = s43vect;
        obj->uDatas = uDatas;
        memcpy(&obj->intarr[0], &intarr[0], sizeof(intarr));
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~SampleData(void)
    {
    }

};
}
```
