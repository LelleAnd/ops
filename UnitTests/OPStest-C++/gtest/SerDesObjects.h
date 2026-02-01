/**
*
* Copyright (C) 2018-2026 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OPSObject.h"
#include "ArchiverInOut.h"

class SerDesObject_Core : public ops::OPSObject
{
public:
    static ops::TypeId_T getTypeName() { return ops::TypeId_T("SerDesObject_Core"); }
#ifdef OPS_C17_DETECTED
    SerDesObject_Core() : ops::OPSObject(getTypeName()) {}
#else
    SerDesObject_Core() { appendType(getTypeName()); }
#endif
    ~SerDesObject_Core() = default;

    bool bo;
    char ch;
    uint8_t u8;
    int16_t i16;
    int i32;
    int64_t i64;
    float f32;
    double d64;
    std::string str;
    ops::strings::fixed_string<30> fstr;
    char buffer[10];

    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        archive->inout("bo", bo);
        archive->inout("ch", ch);
        archive->inout("u8", u8);
        archive->inout("i16", i16);
        archive->inout("i32", i32);
        archive->inout("i64", i64);
        archive->inout("f32", f32);
        archive->inout("d64", d64);
        archive->inout("str", str);
        archive->inout("fstr", fstr);
        archive->inout("buffer", buffer, 10);
    }

    SerDesObject_Core(const SerDesObject_Core& r) = default;
    SerDesObject_Core& operator= (const SerDesObject_Core& l) = default;
    SerDesObject_Core(SerDesObject_Core&&) = default;
    SerDesObject_Core& operator =(SerDesObject_Core&&) = default;
};

class SerDesObject_Vectors : public ops::OPSObject
{
public:
    static ops::TypeId_T getTypeName() { return ops::TypeId_T("SerDesObject_Vectors"); }
#ifdef OPS_C17_DETECTED
    SerDesObject_Vectors() : ops::OPSObject(getTypeName()) {}
#else
    SerDesObject_Vectors() { appendType(getTypeName()); }
#endif
    ~SerDesObject_Vectors() = default;

    std::vector<bool> bo;
    std::vector<char> ch;
    std::vector<uint8_t> u8;
    std::vector<int16_t> i16;
    std::vector<int> i32;
    std::vector<int64_t> i64;
    std::vector<float> f32;
    std::vector<double> d64;
    std::vector<std::string> str;
    std::vector<ops::strings::fixed_string<30>> fstr;

    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        OPSObject::serialize(archive);
        archive->inout("bo", bo);
        archive->inout("ch", ch);
        archive->inout("u8", u8);
        archive->inout("i16", i16);
        archive->inout("i32", i32);
        archive->inout("i64", i64);
        archive->inout("f32", f32);
        archive->inout("d64", d64);
        archive->inout("str", str);
        archive->inout("fstr", fstr);
    }

    SerDesObject_Vectors(const SerDesObject_Vectors& r) = delete;
    SerDesObject_Vectors& operator= (const SerDesObject_Vectors& l) = delete;
    SerDesObject_Vectors(SerDesObject_Vectors&&) = delete;
    SerDesObject_Vectors& operator =(SerDesObject_Vectors&&) = delete;
};

class SerDesObject_Fixarrays : public ops::OPSObject
{
public:
    static ops::TypeId_T getTypeName() { return ops::TypeId_T("SerDesObject_Fixarrays"); }
#ifdef OPS_C17_DETECTED
    SerDesObject_Fixarrays() : ops::OPSObject(getTypeName()) {}
#else
    SerDesObject_Fixarrays() { appendType(getTypeName()); }
#endif
    ~SerDesObject_Fixarrays() = default;

    bool bo[4];
    char ch[2];
    uint8_t u8[3];
    int16_t i16[1];
    int i32[3];
    int64_t i64[2];
    float f32[2];
    double d64[2];
    std::string str[2];
    ops::strings::fixed_string<30> fstr[2];

    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        OPSObject::serialize(archive);
        archive->inoutfixarr("bo", bo, 4, sizeof(bo));
        archive->inoutfixarr("ch", ch, 2, sizeof(ch));
        archive->inoutfixarr("u8", u8, 3, sizeof(u8));
        archive->inoutfixarr("i16", i16, 1, sizeof(i16));
        archive->inoutfixarr("i32", i32, 3, sizeof(i32));
        archive->inoutfixarr("i64", i64, 2, sizeof(i64));
        archive->inoutfixarr("f32", f32, 2, sizeof(f32));
        archive->inoutfixarr("d64", d64, 2, sizeof(d64));
        archive->inoutfixarr("str", str, 2);
        archive->inoutfixarr("fstr", fstr, 2);
    }

    SerDesObject_Fixarrays(const SerDesObject_Fixarrays& r) = delete;
    SerDesObject_Fixarrays& operator= (const SerDesObject_Fixarrays& l) = delete;
    SerDesObject_Fixarrays(SerDesObject_Fixarrays&&) = delete;
    SerDesObject_Fixarrays& operator =(SerDesObject_Fixarrays&&) = delete;
};

class SerDesObject_Serializables : public ops::OPSObject
{
public:
    static ops::TypeId_T getTypeName() { return ops::TypeId_T("SerDesObject_Serializables"); }
#ifdef OPS_C17_DETECTED
    SerDesObject_Serializables() :
        ops::OPSObject(getTypeName()),
        obj2(new SerDesObject_Core()), obj3(new SerDesObject_Core())
    {
        fixarr2[0] = new SerDesObject_Core();
        fixarr2[1] = new SerDesObject_Core();
    }
#else
    SerDesObject_Serializables() :
        obj2(new SerDesObject_Core()), obj3(new SerDesObject_Core())
    {
        appendType(getTypeName());
        fixarr2[0] = new SerDesObject_Core();
        fixarr2[1] = new SerDesObject_Core();
    }
#endif
    virtual ~SerDesObject_Serializables()
    {
        if (obj2 != nullptr) { delete obj2; }
        if (obj3 != nullptr) { delete obj3; }
        for (unsigned int i = 0; i < vobj2.size(); i++) { delete vobj2[i]; }
        if (fixarr2[0] != nullptr) { delete fixarr2[0]; }
        if (fixarr2[1] != nullptr) { delete fixarr2[1]; }
    }

    SerDesObject_Core obj1;
    SerDesObject_Core* obj2;
    SerDesObject_Core* obj3;

    std::vector<SerDesObject_Core> vobj1;
    std::vector<SerDesObject_Core*> vobj2;

    SerDesObject_Core fixarr1[2];
    SerDesObject_Core* fixarr2[2];

    int testException1 = 0;
    int testException2 = 0;

    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        OPSObject::serialize(archive);
        //virtual void inout(InoutName_T name, Serializable& value) = 0;
        archive->inout("obj1", obj1);

        //virtual Serializable* inout(InoutName_T name, Serializable* value) = 0;
        obj2 = (SerDesObject_Core*)archive->inout("obj2", obj2);
        obj3 = (SerDesObject_Core*)archive->inout("obj3", obj3);

        //template <class SerializableType>
        //void inout(InoutName_T name, std::vector<SerializableType>& vec, SerializableType prototype)
        archive->inout("vobj1", vobj1, SerDesObject_Core());

        //template <class SerializableType>
        //void inout(InoutName_T name, std::vector<SerializableType*>& vec)
        archive->inout("vobj2", vobj2);

        //template <class SerializableType>
        //void inoutfixarr(InoutName_T name, SerializableType* value, int numElements)
        archive->inoutfixarr("fixarr1", &fixarr1[0], 2 + testException1);

        //virtual Serializable* inout(InoutName_T name, Serializable* value, int element) = 0;
        //template <class SerializableType>
        //void inoutfixarr(InoutName_T name, SerializableType** value, int numElements)
        archive->inoutfixarr("fixarr2", &fixarr2[0], 2 + testException2);
    }

    SerDesObject_Serializables(const SerDesObject_Serializables& r) = delete;
    SerDesObject_Serializables& operator= (const SerDesObject_Serializables& l) = delete;
    SerDesObject_Serializables(SerDesObject_Serializables&&) = delete;
    SerDesObject_Serializables& operator =(SerDesObject_Serializables&&) = delete;
};
