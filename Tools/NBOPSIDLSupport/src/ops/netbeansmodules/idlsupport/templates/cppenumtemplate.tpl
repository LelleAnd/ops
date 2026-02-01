//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"

__packageDeclaration

__classComment
class __className :
	public ops::OPSObject
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("__packageName.__className");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

public:
   static ops::TypeId_T getTypeName(){return ops::TypeId_T("__packageName.__className");}

__declarations
    int value{ 0 };

    ///Default constructor.
#ifdef OPS_C17_DETECTED
protected:
    __className(std::string_view tName)
        : ops::OPSObject(tName)
    {
    }

public:
    __className() : __className(std::string_view(_inheritDesc)) {}
#else
    __className()
        : ops::OPSObject()
    {
        OPSObject::appendType(getTypeName());
    }
#endif

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInOut.
    void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        archive->inout("value", value);
    }

__memoryPoolDecl
};

__packageCloser
