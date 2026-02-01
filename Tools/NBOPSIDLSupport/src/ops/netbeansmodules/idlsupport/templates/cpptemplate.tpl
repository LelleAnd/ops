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
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("__packageName.__className");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, __baseClassName::_inheritDesc, ' ');
#endif

__generationCheck
public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("__packageName.__className"); }

__declarations
    ///Default constructor.
#ifdef OPS_C17_DETECTED
    __className() : __className(std::string_view(_inheritDesc)) {}

protected:
    __className(std::string_view tName)
        : __baseClassName(tName)
    {
#else
    __className()
        : __baseClassName()
    {
        OPSObject::appendType(ops::TypeId_T("__packageName.__className"));
#endif
__constructorBody
    }

#ifdef OPS_C17_DETECTED
public:
#endif
    ///Copy-constructor making full deep copy of a(n) __className object.
    __className(const __className& _c)
       : __baseClassName(_c)
    {
__copyConstructorBody
        _c.fillCloneShallow(this);
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
    __className(__className&& other) noexcept : __baseClassName(std::move(other))
    {
__moveconstructor
    }

    // Move assignment operator taking other's resources
    __className& operator= (__className&& other) noexcept
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
        __baseClassName::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(__className* obj) const
    {
__fillClone
    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && __baseClassName::isValid();
__validation
		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~__className(void)
    {
__destructorBody
    }

__memoryPoolDecl
};

__packageCloser
