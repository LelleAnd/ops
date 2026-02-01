//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>


namespace pizza {


class PizzaData :
	public ops::OPSObject
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.PizzaData");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

    // Defined to be able to ensure that all generated classes have the reworked copy constructor
    using pizza_PizzaData_new_copycons = bool;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.PizzaData"); }

    static const uint8_t PizzaData_idlVersion = 0;

    uint8_t PizzaData_version = PizzaData_idlVersion;

    std::string cheese;
    std::string tomatoSauce;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    PizzaData() : PizzaData(std::string_view(_inheritDesc)) {}

    PizzaData(std::string_view tName)
        : ops::OPSObject(tName)
    {
#else
    PizzaData()
        : ops::OPSObject()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.PizzaData"));
#endif

    }

    ///Copy-constructor making full deep copy of a(n) PizzaData object.
    PizzaData(const PizzaData& _c)
       : ops::OPSObject(_c)
    {

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) PizzaData object.
    PizzaData& operator = (const PizzaData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    PizzaData(PizzaData&& other) noexcept : ops::OPSObject(std::move(other))
    {
        PizzaData_version = std::move(other.PizzaData_version);
        cheese = std::move(other.cheese);
        tomatoSauce = std::move(other.tomatoSauce);

    }

    // Move assignment operator taking other's resources
    PizzaData& operator= (PizzaData&& other) noexcept
    {
        if (this != &other) {
            ops::OPSObject::operator=(std::move(other));
            PizzaData_version = other.PizzaData_version;
            cheese = std::move(other.cheese);
            tomatoSauce = std::move(other.tomatoSauce);

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        ops::OPSObject::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("PizzaData_version", PizzaData_version);
            ValidateVersion("PizzaData", PizzaData_version, PizzaData_idlVersion);
        } else {
            PizzaData_version = 0;
        }
        archive->inout("cheese", cheese);
        archive->inout("tomatoSauce", tomatoSauce);

    }

    //Returns a deep copy of this object.
    virtual PizzaData* clone() override
    {
        PizzaData* ret = new PizzaData;
        fillClone(ret);
        return ret;

    }

    void fillClone(PizzaData* obj) const
    {
        if (this == obj) { return; }
        ops::OPSObject::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(PizzaData* obj) const
    {
        obj->PizzaData_version = PizzaData_version;
        obj->cheese = cheese;
        obj->tomatoSauce = tomatoSauce;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && ops::OPSObject::isValid();

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~PizzaData(void)
    {

    }


};

}

