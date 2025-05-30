//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

__imports

__packageDeclaration

__classComment
class __className :
	public __baseClassName
{
public:
  	static ops::TypeId_T getTypeName(){return ops::TypeId_T("__packageName.__className");}

  	char __className_version = __className_idlVersion;

__declarations
    ///Default constructor.
    __className()
        : __baseClassName()
__constructorHead
    {
        OPSObject::appendType(ops::TypeId_T("__packageName.__className"));
__constructorBody
    }

    ///Copy-constructor making full deep copy of a(n) __className object.
    __className(const __className& __c)
       : __baseClassName()
__constructorHead
    {
        OPSObject::appendType(ops::TypeId_T("__packageName.__className"));
__constructorBody
        __c.fillClone(this);
    }

    ///Assignment operator making full deep copy of a(n) __className object.
    __className& operator = (const __className& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    __className(__className&& other) : __baseClassName(std::move(other))
    {
__moveconstructor
    }

    // Move assignment operator taking other's resources
    __className& operator= (__className&& other)
    {
        if (this != &other) {
            __baseClassName::operator=(std::move(other));
__moveassignment
        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
__serialize
    }

    //Returns a deep copy of this object.
    virtual __className* clone() override
    {
__clone
    }

    void fillClone(__className* obj) const
    {
        if (this == obj) { return; }
__fillClone
    }

	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool valid = true;
		valid = valid && __baseClassName::isValid();
		valid = valid && (__className_version >= 0) && (__className_version <= __className_idlVersion);
__validation
		return valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~__className(void)
    {
__destructorBody
    }

__memoryPoolDecl
};

__packageCloser
