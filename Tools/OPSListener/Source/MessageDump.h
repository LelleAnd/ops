#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <functional>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace message_dump {

    std::string Trim(std::string str);

    struct indent {
        int level;
        indent(int l) : level(l) {}
        friend std::ostream& operator<<(std::ostream& os, const indent& ind); 
    };

    std::ostream& operator<<(std::ostream& os, const indent& ind);

    typedef std::function<bool(const std::string& name)> SkipFunc_T;

    class MessageDump
    {
    public:
        // The passed lamda, if any, tells which object(s) that should be skipped on top-level,
        // since the fields for these object(s) already are taken care of before calling Dump()
        // (i.e. the buffer in the Dump()-call starts @ the field after the skipped object(s)).
        // As default nothing is skipped.
        MessageDump(SkipFunc_T func = nullptr) : skipFunc(func)
        {
            if (skipFunc == nullptr) {
                skipFunc = [=](const std::string&) -> bool { return false; };
            }
        }

        void AddDefinitions(std::string filename)
        {
            // Create empty property tree object
            pt::ptree tree;
            pt::read_json(filename, tree);
            //printTree(tree, 1);

            // Find all enums first
            for (pt::ptree::iterator pos = tree.begin(); pos != tree.end(); ++pos) {
                CreateEnums(pos->second);
            }
            for (pt::ptree::iterator pos = tree.begin(); pos != tree.end(); ++pos) {
                CreateDumper(pos->second);
            }

            ResolveExtends();

            std::cout << "\n\n";
        }

        bool Any() { return objs.size() > 0; }

        void SetRowLimit(int limit)
        {
            rowlimit = limit;
        }

        // Dump given object type, 'ptr' should point to field in object to start from (inherited
        // objects can be skipped, see skipFunc).
        void Dump(std::string tname, uint32_t verMask, char* ptr)
        {
            auto search = objs.find(tname);
            if (search != objs.end()) {
                DumpObject* dmp = search->second;
                dmp->tagVersion = (verMask != 0);
                dmp->operator()(ptr, 0);
            }
        }

        // Dump object, 'ptr' should point to string with object type followed by object data
        void DumpFromType(char* ptr)
        {
            DumpFieldObject dmp(*this, false, "", objs);
            dmp.operator()(ptr, 0);
        }

    private:
        SkipFunc_T skipFunc = nullptr;

        int rowlimit = INT_MAX;

        struct CheckVersion
        {
            struct Range
            {
                uint8_t low = 0;
                uint8_t high = 255;
                bool within_range(char version) { return ((uint8_t)version >= low) && ((uint8_t)version <= high); }
                Range() {}
            };
            std::vector<Range> ranges;
            bool operator()(char version)
            {
                for (auto& r : ranges) { if (r.within_range(version)) return true; }
                return false;
            }
        };

        struct DumpObject;

        struct DumpValue
        {
            MessageDump& msgDump;
            DumpObject* parent = nullptr;
            CheckVersion* chkVer = nullptr;

            bool optionalOnTagVersion = false;
            bool skipField()
            {
                if (optionalOnTagVersion) {
                    if (parent != nullptr) {
                        return !parent->tagVersion;
                    }
                }
                if (chkVer != nullptr) {
                    if (parent != nullptr) {
                        return !chkVer->operator()(parent->idlVersion);
                    }
                }
                return false;
            }

            std::string name;
            void SetName(std::string n) { name = n; }

            int16_t GetShort(char*& ptr) { int16_t res = *(int16_t*)ptr; ptr += 2; return res; }
            int32_t GetInt(char*& ptr) { int32_t res = *(int32_t*)ptr; ptr += 4; return res; }
            std::string GetString(char*& ptr, int len) {
                char buffer[2048];
                size_t actual = len;
                if (actual >= sizeof(buffer)) actual = sizeof(buffer) - 1;
                if (actual > 0) memcpy(&buffer[0], ptr, actual);
                buffer[actual] = '\0';
                ptr += len;
                return buffer;
            }

            virtual void operator()(char*& ptr, int level) = 0;

            explicit DumpValue(MessageDump& mdump) : msgDump(mdump) {}
            DumpValue() = delete;
            virtual ~DumpValue() = default;
        };

        template <typename T, typename P = T>
        struct DumpV : DumpValue
        {
            DumpV() = delete;
            explicit DumpV(MessageDump& mdump) : DumpValue(mdump) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                std::cout << indent(level) << name << " <" << sizeof(T) << ">: "; 
                if (std::is_same<T, bool>::value) {
                    std::cout << (*ptr ? "true" : "false") << "\n";
                } else {
                    std::cout << (P) * (T*)ptr << "\n";
                }
                ptr += sizeof(T);
            }
        };
        using DumpBoolean = DumpV<bool>;
        using DumpByte    = DumpV<int8_t, int>;
        using DumpShort   = DumpV<int16_t>;
        using DumpInt     = DumpV<int32_t>;
        using DumpLong    = DumpV<int64_t>;
        using DumpFloat   = DumpV<float>;
        using DumpDouble  = DumpV<double>;

        static int width(int value)
        {
            int num = 1;
            while (value > 9) { ++num; value /= 10; }
            return num;
        }

        template <typename T, int numw = 8, typename P = T>
        struct DumpVector : DumpValue
        {
            DumpVector() = delete;
            explicit DumpVector(MessageDump& mdump) : DumpValue(mdump) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                int w = width(len);
                int row = 0;
                for (int i = 0; i < len; ) {
                    int num = len - i;
                    ++row;
                    if ((row < (msgDump.rowlimit-1)) || (num <= numw)) {
                        if (num > numw) { 
                            num = numw; 
                        } else {
                            if (row >= msgDump.rowlimit) { std::cout << indent(level) << name << "[" << std::setw(w) << " " << " ... " << std::setw(w) << " " << "]\n"; }
                        }
                        std::cout << indent(level) << name << "[" << std::setw(w) << i << " ... " << std::setw(w) << (i + num - 1) << "] <" << sizeof(T) << ">: ";
                        for (int j = 0; (i < len) && (j < numw); ++i, ++j) {
                            if (std::is_same<T, bool>::value) {
                                std::cout << (*ptr ? "true" : "false") << "  ";
                            } else {
                                std::cout << (P) * (T*)ptr << "  ";
                            }
                            ptr += sizeof(T);
                        }
                        std::cout << "\n";
                    } else {
                        ptr += numw * sizeof(T);
                        i += numw;
                    }
                }
            }
        };
        using DumpVectorBoolean = DumpVector<bool, 8>;
        using DumpVectorByte = DumpVector<int8_t, 16, int>;
        using DumpVectorShort = DumpVector<int16_t, 16>;
        using DumpVectorInt = DumpVector<int32_t>;
        using DumpVectorLong = DumpVector<int64_t>;
        using DumpVectorFloat = DumpVector<float>;
        using DumpVectorDouble = DumpVector<double>;

        struct DumpString : DumpValue
        {
            DumpString() = delete;
            explicit DumpString(MessageDump& mdump) : DumpValue(mdump) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int len = GetInt(ptr);
                std::cout << indent(level) << name << " <string[" << len << "]>: " << GetString(ptr, len) << "\n";
            }
        };
        struct DumpVectorString : DumpValue
        {
            DumpVectorString() = delete;
            explicit DumpVectorString(MessageDump& mdump) : DumpValue(mdump) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int len1 = GetInt(ptr);
                if (len1 == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                int w = width(len1);
                for (int i = 0; i < len1; ++i) {
                    int len = GetInt(ptr);
                    if (i < (msgDump.rowlimit-2)) {
                        std::cout << indent(level) << name << "[" << std::setw(w) << i << "] <string[" << len << "]>: " << GetString(ptr, len) << "\n";
                    } else {
                        if (i == (len1 - 1)) {
                            std::cout << indent(level) << name << "[ ... ]\n";
                            std::cout << indent(level) << name << "[" << std::setw(w) << i << "] <string[" << len << "]>: " << GetString(ptr, len) << "\n";
                        } else {
                            GetString(ptr, len);
                        }
                    }
                }
            }
        };

        struct DumpObject : DumpValue
        {
            bool tagVersion = false;
            char idlVersion = 0;
            std::vector<DumpValue*> fields;
            DumpObject* base = nullptr;
            std::string extends;
            void Add(DumpValue* dmp) { fields.push_back(dmp); }
            void SetExtends(std::string n) { extends = n; }
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                if (base != nullptr) {
                    base->tagVersion = tagVersion;
                    base->operator()(ptr, level);
                }
                // Skip ops.OPSOBject if on top level, since it is already handled when calling Dump()
                if ((level > 0) || (!msgDump.skipFunc(name))) {
                    std::cout << indent(level + 1) << "<<<" << name << ">>>\n";
                    if (tagVersion) {
                        // Version is always placed first if it exist
                        idlVersion = *ptr;
                    } else {
                        idlVersion = 0;
                    }
                    for (auto& dmp : fields) {
                        dmp->operator()(ptr, level + 1);
                    }
                }
            }
            DumpObject() = delete;
            explicit DumpObject(MessageDump& mdump) : DumpValue(mdump) {}
        };
        struct DumpEnum : DumpValue
        {
            std::vector<std::string> enumvalues;
            void AddEnum(std::string value) { enumvalues.push_back(value); }
            DumpEnum() = delete;
            explicit DumpEnum(MessageDump& mdump) : DumpValue(mdump) {}
        };
        struct DumpEnumShort : DumpEnum
        {
            DumpEnumShort() = delete;
            explicit DumpEnumShort(MessageDump& mdump) : DumpEnum(mdump) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int16_t val = GetShort(ptr);
                if ((size_t)val >= enumvalues.size()) {
                    std::cout << indent(level) << name << " <2>: UNKNOWN enum value\n";
                } else {
                    std::cout << indent(level) << name << " <2>: " << enumvalues[val] << "\n";
                }
            }
        };
        struct DumpVectorEnumShort : DumpValue
        {
            DumpEnumShort* dmp;
            DumpVectorEnumShort() = delete;
            DumpVectorEnumShort(MessageDump& mdump, DumpEnumShort* d) : DumpValue(mdump), dmp(d) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    int16_t val = GetShort(ptr);
                    if ((size_t)val >= dmp->enumvalues.size()) {
                        std::cout << indent(level) << name << "[" << i << "] <2>: UNKNOWN enum value\n";
                    }
                    else {
                        std::cout << indent(level) << name << "[" << i << "] <2>: " << dmp->enumvalues[val] << "\n";
                    }
                }
            }
        };

        struct DumpFieldObject : DumpValue
        {
            std::map<std::string, DumpObject*>& objs;
            const bool isvirtual;
            const std::string fieldtype;
            DumpFieldObject() = delete;
            DumpFieldObject(MessageDump& mdump, bool isvirt, std::string fname, std::map<std::string, DumpObject*>& o) :
                DumpValue(mdump),
                objs(o), isvirtual(isvirt), fieldtype(fname) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                // Get typestring
                int len = GetInt(ptr);
                std::string ftype = Trim(GetString(ptr, len));
                bool tag = false;
                if ((ftype.size() > 0) && (ftype[0] == '0')) {
                    tag = true;
                    ftype[0] = ' ';
                    ftype = Trim(ftype);
                }

                auto pos = ftype.find_first_of(' ');
                if (pos != std::string::npos) {
                    ftype = ftype.substr(0, pos);
                }

                // When we use 'optNonVirt = true', ftype will be empty, use fieldtype instead
                if (ftype == "") {
                    ftype = fieldtype;
                }

                // Dump object
                auto search = objs.find(ftype);
                if (search != objs.end()) {
                    DumpObject* dmp = search->second;
                    std::cout << indent(level) << name << ":\n";
                    dmp->tagVersion = tag;
                    dmp->operator()(ptr, level + 1);
                } else {
                    std::cout << "FAILED to find type: '" << ftype << "'\n";
                }
            }
        };
        struct DumpFieldObjectVector : DumpValue
        {
            DumpFieldObject obj;
            DumpFieldObjectVector() = delete;
            DumpFieldObjectVector(MessageDump& mdump, bool isvirt, std::string fname, std::map<std::string, DumpObject*>& o) :
                DumpValue(mdump), obj(mdump, isvirt, fname, o) {}
            void operator()(char*& ptr, int level) override
            {
                if (skipField()) return;
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    std::cout << indent(level) << name << "[" << i << "]";
                    obj.operator()(ptr, level + 1);
                }
            }
        };

        std::map<std::string, DumpEnumShort*> enums;
        std::map<std::string, DumpObject*> objs;

        void CreateEnums(pt::ptree& pt)
        {
            std::string objtype, basetype;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "type") {
                    objtype = pos->second.data();
                } else if (pos->first == "basetype") {
                    basetype = pos->second.data();
                } else if (pos->first == "enum_definition") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  basetype: " << basetype << "\n";
                    if (basetype != "short") {
                        std::cout << "  UNKNOWN enum of size \"" << basetype << "\"\n";
                    }
                    DumpEnumShort* dmp = new DumpEnumShort(*this);
                    enums[objtype] = dmp;
                    dmp->SetName(objtype);
                    AddEnums(pos->second, dmp);
                }
            }
        }

        void CreateDumper(pt::ptree& pt)
        {
            std::string objtype, basetype;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "type") {
                    objtype = pos->second.data();
                } else if (pos->first == "extends") {
                    basetype = pos->second.data();
                } else if (pos->first == "basetype") {
                    // Skip
                } else if (pos->first == "an_idea") {
                    // Skip
                } else if (pos->first == "ops_internals") {
                    // Skip
                } else if (pos->first == "constants") {
                    // Skip
                } else if (pos->first == "desc") {
                    // Just a comment, skip
                } else if (pos->first == "fields") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  extends: " << basetype << "\n";
                    DumpObject* dmp = new DumpObject(*this);
                    objs[objtype] = dmp;
                    dmp->SetName(objtype);
                    dmp->SetExtends(basetype);
                    AddFields(pos->second, dmp);
                } else if (pos->first == "enum") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  extends: " << basetype << "\n";
                    DumpObject* dmp = new DumpObject(*this);
                    objs[objtype] = dmp;
                    dmp->SetName(objtype);
                    dmp->SetExtends(basetype);
                    DumpValue* val = new DumpInt(*this);
                    val->SetName("enum");
                    dmp->Add(val);
                } else if (pos->first == "enum_definition") {
                    // Skip
                } else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
        }

        void AddEnums(pt::ptree& pt, DumpEnumShort* dmp)
        {
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                AddEnum(pos->second, dmp);
            }
        }

        void AddEnum(pt::ptree& pt, DumpEnumShort* dmp)
        {
            std::string enumname, enumvalue;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "enum") {
                    enumname = pos->second.data();
                }
                else if (pos->first == "value") {
                    enumvalue = pos->second.data();
                }
                else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
            dmp->AddEnum(enumname);
        }

        void AddFields(pt::ptree& pt, DumpObject* dmp)
        {
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                AddField(pos->second, dmp);
            }
        }

        CheckVersion::Range CreateRangeChecker(pt::ptree& pt)
        {
            CheckVersion::Range rng;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "low") {
                    std::string value = pos->second.data();
                    int val = atoi(value.c_str());
                    if (val < 0) { val = 0; }
                    if (val > 255) { val = 255; }
                    rng.low = (char)val;
                } else if (pos->first == "high") {
                    std::string value = pos->second.data();
                    int val = atoi(value.c_str());
                    if (val < 0) { val = 0; }
                    if (val > 255) { val = 255; }
                    rng.high = (char)val;
                }
            }
            return rng;
        }

        CheckVersion* CreateVersionChecker(pt::ptree& pt)
        {
            CheckVersion* chkVer = new CheckVersion();
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                chkVer->ranges.push_back(CreateRangeChecker(pos->second));
            }
            return chkVer;
        }

        void AddField(pt::ptree& pt, DumpObject* dmp)
        {
            std::string fieldname, fieldtype, elementtype;
            bool optionalOnTagVersion = false;
            CheckVersion* chkVer = nullptr;

            //{ "name": "Kalle", "type" : "int", "version" : [{ "low": "1", "high" : "5" }, { "low": "10", "high" : "255" }], "desc" : "hej hopp" }

            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "name") {
                    fieldname = pos->second.data();
                } else if (pos->first == "type") {
                    fieldtype = pos->second.data();
                } else if (pos->first == "desc") {
                    // Just a comment, skip
                } else if (pos->first == "elementtype") {
                    elementtype = pos->second.data();
                } else if (pos->first == "optional") {
                    if (pos->second.data() == "type_is_version_tagged") { /// typestring dependent?
                        optionalOnTagVersion = true;
                    }
                } else if (pos->first == "version") { /// Version dependent?
                    chkVer = CreateVersionChecker(pos->second);
                } else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
            
            DumpValue* val = nullptr;
            if      (fieldtype == "boolean")   { val = new DumpBoolean(*this); }
            else if (fieldtype == "byte")      { val = new DumpByte(*this); }
            else if (fieldtype == "short")     { val = new DumpShort(*this); }
            else if (fieldtype == "int"   )    { val = new DumpInt(*this); }
            else if (fieldtype == "long"  )    { val = new DumpLong(*this); }
            else if (fieldtype == "float")     { val = new DumpFloat(*this); }
            else if (fieldtype == "double")    { val = new DumpDouble(*this); }
            else if (fieldtype == "string")    { val = new DumpString(*this); }
            else if (fieldtype == "vector<T>") { 
                if      (elementtype == "boolean") { val = new DumpVectorBoolean(*this); }
                else if (elementtype == "byte")    { val = new DumpVectorByte(*this); }
                else if (elementtype == "short")   { val = new DumpVectorShort(*this); }
                else if (elementtype == "int")     { val = new DumpVectorInt(*this); }
                else if (elementtype == "long")    { val = new DumpVectorLong(*this); }
                else if (elementtype == "float")   { val = new DumpVectorFloat(*this); }
                else if (elementtype == "double")  { val = new DumpVectorDouble(*this); }
                else if (elementtype == "string")  { val = new DumpVectorString(*this); }
                else {
                    bool isvirt = false;
                    auto pos = elementtype.find("virtual");
                    if (pos != elementtype.npos) {
                        isvirt = true;
                        elementtype = elementtype.substr(pos + 8);
                    }

                    //std::cout << "  FieldObject, name: '" << fieldname << "', type: '" << elementtype << "'\n";
                    auto search = enums.find(elementtype);
                    if (search != enums.end()) {
                        DumpEnumShort* dmp2 = search->second;
                        val = new DumpVectorEnumShort(*this, dmp2);
                    } else {
                        val = new DumpFieldObjectVector(*this, isvirt, elementtype, objs);
                    }
                }
            } else { 
                bool isvirt = false;
                auto pos = fieldtype.find("virtual");
                if (pos != fieldtype.npos) {
                    isvirt = true;
                    fieldtype = fieldtype.substr(pos + 8);
                }
                //std::cout << "  FieldObject, name: '" << fieldname << "', type: '" << fieldtype << "'\n";
                auto search = enums.find(fieldtype);
                if (search != enums.end()) {
                    DumpEnumShort* dmp2 = search->second;
                    val = new DumpEnumShort(*dmp2);
                } else {
                    val = new DumpFieldObject(*this, isvirt, fieldtype, objs);
                }
            }

            if (val) {
                val->SetName(fieldname);
                val->optionalOnTagVersion = optionalOnTagVersion;
                val->chkVer = chkVer;
                val->parent = dmp;
                dmp->Add(val);
            }
        }

        void ResolveExtends(DumpObject* dmp)
        {
            if (dmp->extends != "") {
                // Look up base
                auto search = objs.find(dmp->extends);
                if (search != objs.end()) {
                    DumpObject* base = search->second;
                    dmp->base = base;
                    dmp->extends = "";
                }
            }
        }

        void ResolveExtends()
        {
            for (auto& o : objs) {
                ResolveExtends(o.second);
            }
        }

        void printTree(pt::ptree& pt, int level) {
            if (pt.empty()) {
                std::cerr << "\"" << pt.data() << "\"";

            } else {
                if (level) std::cerr << std::endl;

                std::cerr << indent(level) << "{" << std::endl;

                for (pt::ptree::iterator pos = pt.begin(); pos != pt.end();) {
                    std::cerr << indent(level + 1) << "\"" << pos->first << "\": ";

                    printTree(pos->second, level + 1);
                    ++pos;
                    if (pos != pt.end()) {
                        std::cerr << ",";
                    }
                    std::cerr << std::endl;
                }

                std::cerr << indent(level) << "}";
            }
        }

    };

}