//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

#include "VessuvioData.h"


namespace pizza {


class CapricosaData :
	public VessuvioData
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.CapricosaData");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, VessuvioData::_inheritDesc, ' ');
#endif

    // Ensure that base class has the reworked copy constructor. If not, re-generate base class
    using pizza_CapricosaData_new_copycons = pizza_VessuvioData_new_copycons;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.CapricosaData"); }

    static const uint8_t CapricosaData_idlVersion = 0;

    uint8_t CapricosaData_version = CapricosaData_idlVersion;

    std::string mushrooms;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    CapricosaData() : CapricosaData(std::string_view(_inheritDesc)) {}

    CapricosaData(std::string_view tName)
        : VessuvioData(tName)
    {
#else
    CapricosaData()
        : VessuvioData()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.CapricosaData"));
#endif

    }

    ///Copy-constructor making full deep copy of a(n) CapricosaData object.
    CapricosaData(const CapricosaData& _c)
       : VessuvioData(_c)
    {

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) CapricosaData object.
    CapricosaData& operator = (const CapricosaData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    CapricosaData(CapricosaData&& other) noexcept : VessuvioData(std::move(other))
    {
        CapricosaData_version = std::move(other.CapricosaData_version);
        mushrooms = std::move(other.mushrooms);

    }

    // Move assignment operator taking other's resources
    CapricosaData& operator= (CapricosaData&& other) noexcept
    {
        if (this != &other) {
            VessuvioData::operator=(std::move(other));
            CapricosaData_version = other.CapricosaData_version;
            mushrooms = std::move(other.mushrooms);

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        VessuvioData::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("CapricosaData_version", CapricosaData_version);
            ValidateVersion("CapricosaData", CapricosaData_version, CapricosaData_idlVersion);
        } else {
            CapricosaData_version = 0;
        }
        archive->inout("mushrooms", mushrooms);

    }

    //Returns a deep copy of this object.
    virtual CapricosaData* clone() override
    {
        CapricosaData* ret = new CapricosaData;
        fillClone(ret);
        return ret;

    }

    void fillClone(CapricosaData* obj) const
    {
        if (this == obj) { return; }
        VessuvioData::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(CapricosaData* obj) const
    {
        obj->CapricosaData_version = CapricosaData_version;
        obj->mushrooms = mushrooms;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && VessuvioData::isValid();
        _valid = _valid && (CapricosaData_version == CapricosaData_idlVersion);

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~CapricosaData(void)
    {

    }


};

}

