//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

#include "CapricosaData.h"
#include "PizzaData.h"


namespace pizza { namespace special {


class LHCData :
	public pizza::CapricosaData
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.special.LHCData");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, pizza::CapricosaData::_inheritDesc, ' ');
#endif

    // Ensure that base class has the reworked copy constructor. If not, re-generate base class
    using pizza_special_LHCData_new_copycons = pizza_CapricosaData_new_copycons;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.special.LHCData"); }

    static const uint8_t LHCData_idlVersion = 0;

    uint8_t LHCData_version = LHCData_idlVersion;

    std::string bearnaise;
    std::string beef;
    std::vector<pizza::PizzaData> p;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    LHCData() : LHCData(std::string_view(_inheritDesc)) {}

    LHCData(std::string_view tName)
        : pizza::CapricosaData(tName)
    {
#else
    LHCData()
        : pizza::CapricosaData()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.special.LHCData"));
#endif

    }

    ///Copy-constructor making full deep copy of a(n) LHCData object.
    LHCData(const LHCData& _c)
       : pizza::CapricosaData(_c)
    {

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) LHCData object.
    LHCData& operator = (const LHCData& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    LHCData(LHCData&& other) noexcept : pizza::CapricosaData(std::move(other))
    {
        LHCData_version = std::move(other.LHCData_version);
        bearnaise = std::move(other.bearnaise);
        beef = std::move(other.beef);
        p = std::move(other.p);

    }

    // Move assignment operator taking other's resources
    LHCData& operator= (LHCData&& other) noexcept
    {
        if (this != &other) {
            pizza::CapricosaData::operator=(std::move(other));
            LHCData_version = other.LHCData_version;
            bearnaise = std::move(other.bearnaise);
            beef = std::move(other.beef);
            std::swap(p, other.p);

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        pizza::CapricosaData::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("LHCData_version", LHCData_version);
            ValidateVersion("LHCData", LHCData_version, LHCData_idlVersion);
        } else {
            LHCData_version = 0;
        }
        archive->inout("bearnaise", bearnaise);
        archive->inout("beef", beef);
        archive->inout<pizza::PizzaData>("p", p, pizza::PizzaData());

    }

    //Returns a deep copy of this object.
    virtual LHCData* clone() override
    {
        LHCData* ret = new LHCData;
        fillClone(ret);
        return ret;

    }

    void fillClone(LHCData* obj) const
    {
        if (this == obj) { return; }
        pizza::CapricosaData::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(LHCData* obj) const
    {
        obj->LHCData_version = LHCData_version;
        obj->bearnaise = bearnaise;
        obj->beef = beef;
        obj->p = p;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && pizza::CapricosaData::isValid();
        _valid = _valid && (LHCData_version == LHCData_idlVersion);
        for (size_t _i = 0; _i < this->p.size(); _i++) {
            _valid = _valid && this->p[_i].isValid();
        }

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~LHCData(void)
    {

    }


};

}}

