#pragma once

#include <vector>
#include <regex>

#include <OPSMessage.h>

namespace filter {

    class FilterAbs
    {
    public:
        virtual bool accept(const ops::OPSMessage* const m, const ops::OPSObject* const o) = 0;
        virtual ~FilterAbs() = default;
    };

    class FilterAll : public FilterAbs
    {
    public:
        void Add(FilterAbs* filter) { filters.push_back(filter); }
        virtual bool accept(const ops::OPSMessage* const m, const ops::OPSObject* const o) override
        {
            for (const auto& x : filters) {
                if (!x->accept(m, o)) { return false; }
            }
            return true;
        }
        ~FilterAll()
        {
            for (const auto& x : filters) {
                delete x;
            }
        }
    private:
        std::vector<FilterAbs*> filters;
    };

    class FilterAny : public FilterAbs
    {
    public:
        void Add(FilterAbs* filter) { filters.push_back(filter); }
        virtual bool accept(const ops::OPSMessage* const m, const ops::OPSObject* const o) override
        {
            for (const auto& x : filters) {
                if (x->accept(m, o)) { return true; }
            }
            return false;
        }
    private:
        std::vector<FilterAbs*> filters;
    };

    class FilterKey : public FilterAbs
    {
    public:
        FilterKey(const ops::ObjectKey_T& key) : rex(key.c_str()) {}
        virtual bool accept(const ops::OPSMessage* const, const ops::OPSObject* const o) override
        {
            if (o == nullptr) { return false; }
            return std::regex_match(o->getKey().c_str(), rex);
        }
    private:
        std::regex rex;
    };

    class FilterPubName : public FilterAbs
    {
    public:
        FilterPubName(const ops::ObjectName_T& name) : rex(name.c_str()) {}
        virtual bool accept(const ops::OPSMessage* const m, const ops::OPSObject* const) override
        {
            if (m == nullptr) { return false; }
            return std::regex_match(m->getPublisherName().c_str(), rex);
        }
    private:
        std::regex rex;
    };

    class FilterSource : public FilterAbs
    {
    public:
        FilterSource(const ops::Address_T& addr) : rex(addr.c_str()) {}
        virtual bool accept(const ops::OPSMessage* const m, const ops::OPSObject* const) override
        {
            if (m == nullptr) { return false; }
            ops::Address_T adr;
            uint16_t port;
            m->getSource(adr, port);
            adr += "::";
            adr += ops::NumberToString(port);
            return std::regex_match(adr.c_str(), rex);
        }
    private:
        std::regex rex;
    };

    class FilterType : public FilterAbs
    {
    public:
        FilterType(const ops::TypeId_T& typ) : rex(typ.c_str()) {}
        virtual bool accept(const ops::OPSMessage* const, const ops::OPSObject* const o) override
        {
            if (o == nullptr) { return false; }
            return std::regex_match(o->getTypeString().c_str(), rex);
        }
    private:
        std::regex rex;
    };

}
