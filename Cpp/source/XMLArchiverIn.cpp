/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2022 Lennart Andersson.
 *
 * This file is part of OPS (Open Publish Subscribe).
 *
 * OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
 */
#include "OPSTypeDefs.h"
#include "XMLArchiverIn.h"

namespace ops
{
    size_t countChildElements(tinyxml2::XMLElement* el, const char* name)
    {
        size_t count = 0;
        tinyxml2::XMLElement* el2 = el->FirstChildElement(name);
        while (el2 != nullptr) {
            count++;
            el2 = el2->NextSiblingElement(name);
        }
        return count;
    }

    template <typename T>
    T ConvertValue(InoutName_T, const std::string& s)
    {
        std::stringstream ss(s);
        T inVal{};
        ss >> inVal;
        return inVal;
    }
    template<>
    char ConvertValue(InoutName_T, const std::string& s)
    {
        std::stringstream ss(s);
        int inVal{};
        ss >> inVal;
        return (char)inVal;
    }
    template<>
    bool ConvertValue(InoutName_T name, const std::string& s)
    {
        if (s.compare("true") == 0) { return true; }
        if (s.compare("false") == 0) { return false; }
        if (s.compare("TRUE") == 0) { return true; }
        if (s.compare("FALSE") == 0) { return false; }
        if (s.compare("True") == 0) { return true; }
        if (s.compare("False") == 0) { return false; }
        throw ops::ArchiverException("XMLArchiverIn: Illegal value for bool with name: ", name);
    }
    template<>
    std::string ConvertValue(InoutName_T, const std::string& s)
    {
        return s;
    }

    template <typename T>
    void ExtractValue(InoutName_T name, T& value, tinyxml2::XMLElement* currentNode)
    {
        tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
        if (el != nullptr) {
            tinyxml2::XMLNode* node = el->FirstChild();
            if (node != nullptr) {
                tinyxml2::XMLText* t = node->ToText();
                if (t != nullptr) {
                    value = ConvertValue<T>(name, t->Value());
                }
            }
        }
    }

    template <typename T>
    void ExtractVector(InoutName_T name, std::vector<T>& value, tinyxml2::XMLElement* currentNode)
    {
        tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
        if (el != nullptr) {
            const size_t size = countChildElements(el, "element");
            value.reserve(size);
            value.resize(size, T{});

            tinyxml2::XMLElement* el2 = el->FirstChildElement("element");
            size_t i = 0;
            while (el2 != nullptr) {
                tinyxml2::XMLNode* node2 = el2->FirstChild();
                if (node2 != nullptr) {
                    tinyxml2::XMLText* t = node2->ToText();
                    if (t != nullptr) {
                        value[i] = ConvertValue<T>(name, std::string(t->Value()));
                    }
                }
                i++;
                el2 = el2->NextSiblingElement("element");
            }
        }
    }

    template <typename T>
    void ExtractFixArr(InoutName_T name, T* value, int numElements, tinyxml2::XMLElement* currentNode)
    {
        tinyxml2::XMLElement* el = currentNode->FirstChildElement(NAME(name));
        if (el != nullptr) {
            const size_t size = countChildElements(el, "element");
            if (size != (size_t)numElements) throw ops::ArchiverException("Illegal size of fix bool array received. name: ", name);

            tinyxml2::XMLElement* el2 = el->FirstChildElement("element");
            size_t i = 0;
            while (el2 != nullptr) {
                tinyxml2::XMLNode* node2 = el2->FirstChild();
                if (node2 != nullptr) {
                    tinyxml2::XMLText* t = node2->ToText();
                    if (t != nullptr) {
                        value[i] = ConvertValue<T>(name, std::string(t->Value()));
                    }
                }
                i++;
                el2 = el2->NextSiblingElement("element");
            }
        }
    }

    void XMLArchiverIn::inout(InoutName_T name, bool& value)
    {
        ExtractValue<bool>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, char& value)
    {
        ExtractValue<char>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, int& value)
    {
        ExtractValue<int>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, int16_t& value)
    {
        ExtractValue<int16_t>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, int64_t& value)
    {
        ExtractValue<int64_t>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, float& value)
    {
        ExtractValue<float>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, double& value)
    {
        ExtractValue<double>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::string& value)
    {
        ExtractValue<std::string>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<bool>& value)
    {
        ExtractVector<bool>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<char>& value)
    {
        ExtractVector<char>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<int>& value)
    {
        ExtractVector<int>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<int16_t>& value)
    {
        ExtractVector<int16_t>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<int64_t>& value)
    {
        ExtractVector<int64_t>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<float>& value)
    {
        ExtractVector<float>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<double>& value)
    {
        ExtractVector<double>(name, value, currentNode);
    }

    void XMLArchiverIn::inout(InoutName_T name, std::vector<std::string>& value) 
    {
        ExtractVector<std::string>(name, value, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, bool* value, int numElements, int)
    {
        ExtractFixArr<bool>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, char* value, int numElements, int)
    {
        ExtractFixArr<char>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, int* value, int numElements, int)
    {
        ExtractFixArr<int>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, int16_t* value, int numElements, int)
    {
        ExtractFixArr<int16_t>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, int64_t* value, int numElements, int)
    {
        ExtractFixArr<int64_t>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, float* value, int numElements, int)
    {
        ExtractFixArr<float>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, double* value, int numElements, int)
    {
        ExtractFixArr<double>(name, value, numElements, currentNode);
    }

    void XMLArchiverIn::inoutfixarr(InoutName_T name, std::string* value, int numElements)
    {
        ExtractFixArr<std::string>(name, value, numElements, currentNode);
    }

    int XMLArchiverIn::beginList(InoutName_T name, int)
    {
        PushNode(currentNode);
        currentNode = currentNode->FirstChildElement(NAME(name));
        if (currentNode != nullptr) {
            return (int)countChildElements(currentNode, "element");
        }
        return 0;
    }

    void XMLArchiverIn::endList(InoutName_T)
    {
        PopNode(currentNode);
    }

}

