//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>



namespace pizza { namespace special {


class Cheese :
	public ops::OPSObject
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.special.Cheese");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

    // Defined to be able to ensure that all generated classes have the reworked copy constructor
    using pizza_special_Cheese_new_copycons = bool;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.special.Cheese"); }

    static const uint8_t Cheese_idlVersion = 0;

    uint8_t Cheese_version = Cheese_idlVersion;

    std::string name;
    double age{ 0 };

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    Cheese() : Cheese(std::string_view(_inheritDesc)) {}

    Cheese(std::string_view tName)
        : ops::OPSObject(tName)
    {
#else
    Cheese()
        : ops::OPSObject()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.special.Cheese"));
#endif

    }

    ///Copy-constructor making full deep copy of a(n) Cheese object.
    Cheese(const Cheese& _c)
       : ops::OPSObject(_c)
    {

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) Cheese object.
    Cheese& operator = (const Cheese& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    Cheese(Cheese&& other) noexcept : ops::OPSObject(std::move(other))
    {
        Cheese_version = std::move(other.Cheese_version);
        name = std::move(other.name);
        age = std::move(other.age);

    }

    // Move assignment operator taking other's resources
    Cheese& operator= (Cheese&& other) noexcept
    {
        if (this != &other) {
            ops::OPSObject::operator=(std::move(other));
            Cheese_version = other.Cheese_version;
            name = std::move(other.name);
            age = other.age;

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("Cheese_version", Cheese_version);
            ValidateVersion("Cheese", Cheese_version, Cheese_idlVersion);
        } else {
            Cheese_version = 0;
        }
        archive->inout("name", name);
        archive->inout("age", age);

    }

    //Returns a deep copy of this object.
    virtual Cheese* clone() override
    {
        Cheese* ret = new Cheese;
        fillClone(ret);
        return ret;

    }

    void fillClone(Cheese* obj) const
    {
        if (this == obj) { return; }
        ops::OPSObject::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(Cheese* obj) const
    {
        obj->Cheese_version = Cheese_version;
        obj->name = name;
        obj->age = age;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && ops::OPSObject::isValid();
        _valid = _valid && (Cheese_version == Cheese_idlVersion);

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~Cheese(void)
    {

    }


};

}}

