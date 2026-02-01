//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

#include "PizzaData.h"


namespace pizza {


class VessuvioData :
	public PizzaData
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.VessuvioData");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, PizzaData::_inheritDesc, ' ');
#endif

    // Ensure that base class has the reworked copy constructor. If not, re-generate base class
    using pizza_VessuvioData_new_copycons = pizza_PizzaData_new_copycons;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.VessuvioData"); }

    static const uint8_t VessuvioData_idlVersion = 0;

    uint8_t VessuvioData_version = VessuvioData_idlVersion;

    std::string ham;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    VessuvioData() : VessuvioData(std::string_view(_inheritDesc)) {}

    VessuvioData(std::string_view tName)
        : PizzaData(tName)
    {
#else
    VessuvioData()
        : PizzaData()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.VessuvioData"));
#endif

    }

    ///Copy-constructor making full deep copy of a(n) VessuvioData object.
    VessuvioData(const VessuvioData& _c)
       : PizzaData(_c)
    {

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) VessuvioData object.
    VessuvioData& operator = (const VessuvioData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    VessuvioData(VessuvioData&& other) noexcept : PizzaData(std::move(other))
    {
        VessuvioData_version = std::move(other.VessuvioData_version);
        ham = std::move(other.ham);

    }

    // Move assignment operator taking other's resources
    VessuvioData& operator= (VessuvioData&& other) noexcept
    {
        if (this != &other) {
            PizzaData::operator=(std::move(other));
            VessuvioData_version = other.VessuvioData_version;
            ham = std::move(other.ham);

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        PizzaData::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("VessuvioData_version", VessuvioData_version);
            ValidateVersion("VessuvioData", VessuvioData_version, VessuvioData_idlVersion);
        } else {
            VessuvioData_version = 0;
        }
        archive->inout("ham", ham);

    }

    //Returns a deep copy of this object.
    virtual VessuvioData* clone() override
    {
        VessuvioData* ret = new VessuvioData;
        fillClone(ret);
        return ret;

    }

    void fillClone(VessuvioData* obj) const
    {
        if (this == obj) { return; }
        PizzaData::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(VessuvioData* obj) const
    {
        obj->VessuvioData_version = VessuvioData_version;
        obj->ham = ham;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && PizzaData::isValid();
        _valid = _valid && (VessuvioData_version == VessuvioData_idlVersion);

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~VessuvioData(void)
    {

    }


};

}

