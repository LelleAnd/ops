//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "OPSObject.h"
#include "ArchiverInOut.h"
#include <string.h>		// for memset() on Linux
#include <vector>

#include "Cheese.h"
#include "LHCData.h"


namespace pizza { namespace special {

///This is a really nice Pizza!

class ExtraAllt :
	public LHCData
{
protected:
#ifdef OPS_C17_DETECTED
    // Compile-time generated type and inheritance description strings
    constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("pizza.special.ExtraAllt");
    constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, LHCData::_inheritDesc, ' ');
#endif

    // Ensure that base class has the reworked copy constructor. If not, re-generate base class
    using pizza_special_ExtraAllt_new_copycons = pizza_special_LHCData_new_copycons;

public:
  	static ops::TypeId_T getTypeName(){ return ops::TypeId_T("pizza.special.ExtraAllt"); }

    static const uint8_t ExtraAllt_idlVersion = 0;

    uint8_t ExtraAllt_version = ExtraAllt_idlVersion;

    ///Does the order "include" extra cheese???
    bool extraCheese{ false };
    uint8_t nrOfMushRooms{ 0 };
    int meetQuality{ 0 };
    int64_t timestamp{ 0 };
    float timeBakedHours{ 0 };
    double timeBakedSeconds{ 0 };
    std::string description;
    short testingShort{ 0 };
    pizza::special::Cheese* cheese_;
    std::vector<bool> bools;
    std::vector<uint8_t> bytes;
    std::vector<short> shorts;
    std::vector<int> ints;
    std::vector<int64_t> longs;
    std::vector<float> floats;
    std::vector<double> doubles;
    std::vector<std::string> strings;
    std::vector<pizza::special::Cheese> cheeses;

    ///Default constructor.
#ifdef OPS_C17_DETECTED
    ExtraAllt() : ExtraAllt(std::string_view(_inheritDesc)) {}

    ExtraAllt(std::string_view tName)
        : LHCData(tName)
    {
#else
    ExtraAllt()
        : LHCData()
    {
        OPSObject::appendType(ops::TypeId_T("pizza.special.ExtraAllt"));
#endif
        cheese_ = new pizza::special::Cheese;

    }

    ///Copy-constructor making full deep copy of a(n) ExtraAllt object.
    ExtraAllt(const ExtraAllt& _c)
       : LHCData(_c)
    {
        cheese_ = nullptr;

        _c.fillCloneShallow(this);
    }

    ///Assignment operator making full deep copy of a(n) ExtraAllt object.
    ExtraAllt& operator = (const ExtraAllt& other)
    {
        if (this != &other) {
            other.fillClone(this);
        }
        return *this;
    }

    ///Move-constructor taking other's resources
    ExtraAllt(ExtraAllt&& other) noexcept : LHCData(std::move(other))
    {
        ExtraAllt_version = std::move(other.ExtraAllt_version);
        extraCheese = std::move(other.extraCheese);
        nrOfMushRooms = std::move(other.nrOfMushRooms);
        meetQuality = std::move(other.meetQuality);
        timestamp = std::move(other.timestamp);
        timeBakedHours = std::move(other.timeBakedHours);
        timeBakedSeconds = std::move(other.timeBakedSeconds);
        description = std::move(other.description);
        testingShort = std::move(other.testingShort);
        cheese_ = std::move(other.cheese_);
        other.cheese_ = nullptr;
        bools = std::move(other.bools);
        bytes = std::move(other.bytes);
        shorts = std::move(other.shorts);
        ints = std::move(other.ints);
        longs = std::move(other.longs);
        floats = std::move(other.floats);
        doubles = std::move(other.doubles);
        strings = std::move(other.strings);
        cheeses = std::move(other.cheeses);

    }

    // Move assignment operator taking other's resources
    ExtraAllt& operator= (ExtraAllt&& other) noexcept
    {
        if (this != &other) {
            LHCData::operator=(std::move(other));
            ExtraAllt_version = other.ExtraAllt_version;
            extraCheese = other.extraCheese;
            nrOfMushRooms = other.nrOfMushRooms;
            meetQuality = other.meetQuality;
            timestamp = other.timestamp;
            timeBakedHours = other.timeBakedHours;
            timeBakedSeconds = other.timeBakedSeconds;
            description = std::move(other.description);
            testingShort = other.testingShort;
            std::swap(cheese_, other.cheese_);
            std::swap(bools, other.bools);
            std::swap(bytes, other.bytes);
            std::swap(shorts, other.shorts);
            std::swap(ints, other.ints);
            std::swap(longs, other.longs);
            std::swap(floats, other.floats);
            std::swap(doubles, other.doubles);
            std::swap(strings, other.strings);
            std::swap(cheeses, other.cheeses);

        }
        return *this;
    }

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    virtual void serialize(ops::ArchiverInOut* archive) override
    {
        LHCData::serialize(archive);
        if (idlVersionMask != 0) {
            archive->inout("ExtraAllt_version", ExtraAllt_version);
            ValidateVersion("ExtraAllt", ExtraAllt_version, ExtraAllt_idlVersion);
        } else {
            ExtraAllt_version = 0;
        }
        archive->inout("extraCheese", extraCheese);
        archive->inout("nrOfMushRooms", nrOfMushRooms);
        archive->inout("meetQuality", meetQuality);
        archive->inout("timestamp", timestamp);
        archive->inout("timeBakedHours", timeBakedHours);
        archive->inout("timeBakedSeconds", timeBakedSeconds);
        archive->inout("description", description);
        archive->inout("testingShort", testingShort);
        cheese_ = (pizza::special::Cheese*) archive->inout("cheese_", cheese_);
        archive->inout("bools", bools);
        archive->inout("bytes", bytes);
        archive->inout("shorts", shorts);
        archive->inout("ints", ints);
        archive->inout("longs", longs);
        archive->inout("floats", floats);
        archive->inout("doubles", doubles);
        archive->inout("strings", strings);
        archive->inout<pizza::special::Cheese>("cheeses", cheeses, pizza::special::Cheese());

    }

    //Returns a deep copy of this object.
    virtual ExtraAllt* clone() override
    {
        ExtraAllt* ret = new ExtraAllt;
        fillClone(ret);
        return ret;

    }

    void fillClone(ExtraAllt* obj) const
    {
        if (this == obj) { return; }
        LHCData::fillClone(obj);
        fillCloneShallow(obj);
    }

private:
    void fillCloneShallow(ExtraAllt* obj) const
    {
        obj->ExtraAllt_version = ExtraAllt_version;
        obj->extraCheese = extraCheese;
        obj->nrOfMushRooms = nrOfMushRooms;
        obj->meetQuality = meetQuality;
        obj->timestamp = timestamp;
        obj->timeBakedHours = timeBakedHours;
        obj->timeBakedSeconds = timeBakedSeconds;
        obj->description = description;
        obj->testingShort = testingShort;
        if(obj->cheese_) delete obj->cheese_;
        obj->cheese_ = (pizza::special::Cheese*)cheese_->clone();
        obj->bools = bools;
        obj->bytes = bytes;
        obj->shorts = shorts;
        obj->ints = ints;
        obj->longs = longs;
        obj->floats = floats;
        obj->doubles = doubles;
        obj->strings = strings;
        obj->cheeses = cheeses;

    }

public:
	///Validation routine for fields
	virtual bool isValid() const noexcept override
    {
		bool _valid = true;
		_valid = _valid && LHCData::isValid();
        _valid = _valid && (ExtraAllt_version == ExtraAllt_idlVersion);
        _valid = _valid && this->cheese_->isValid();
        for (size_t _i = 0; _i < this->cheeses.size(); _i++) {
            _valid = _valid && this->cheeses[_i].isValid();
        }

		return _valid;
    }

    ///Destructor: Note that all aggregated data and vectors are completely deleted.
    virtual ~ExtraAllt(void)
    {
        if (cheese_ != nullptr) { delete cheese_; }

    }


};

}}

