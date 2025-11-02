/**
 *
 * Copyright (C) 2020-2025 Lennart Andersson.
 *
 * This notice apply to all source files, *.cpp, *.h, *.java, and *.cs in this directory
 * and all its subdirectories if nothing else is explicitly stated within the source file itself.
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
#pragma once

#include "ArchiverInOut.h"
#include "OPSObject.h"

namespace ops {

    namespace example {

        struct calculator_xor_8
        {
            uint8_t sum = 0;
            size_t totalbytes = 0;
            uint32_t totalfields = 0;

            // A simple byte wise xor
            void calc(InoutName_T , const void* ptr, size_t size)
            {
                totalbytes += size;
                totalfields++;
                //std::cout << "chksum: " << name << ", size = " << size << "\n";
                for (size_t i = 0; i < size; i++) {
                    sum = sum ^ ((const uint8_t*)ptr)[i];
                }
            }
        };

        struct calculator_xor_64
        {
            uint64_t sum = 0;

            // A 64-bit xor with padding per call
            void calc(InoutName_T , const void* ptr, size_t size)
            {
                //std::cout << "chksum2: " << name << "\n";
                const uint64_t* src = (const uint64_t*)ptr;
                while (size >= 8) {
                    sum = sum ^ *src++;
                    size -= 8;
                }
                if (size > 0) {
                    uint64_t part = 0;
                    memcpy(&part, src, size);
                    sum = sum ^ part;
                }
            }
        };

    }

    template <typename T>
    class ChecksumArchiver : public ArchiverInOut
    {
    public:
        T calc;

        virtual ~ChecksumArchiver() {}

        bool isOut() override { return true; }

        void inout(InoutName_T name, bool& value) override
        {
            calc.calc(name, &value, 1);
        }
        void inout(InoutName_T name, char& value) override
        {
            calc.calc(name, &value, 1);
        }
        void inout(InoutName_T name, uint8_t& value) override
        {
            calc.calc(name, &value, 1);
        }
        void inout(InoutName_T name, int& value) override
        {
            calc.calc(name, &value, 4);
        }
        void inout(InoutName_T name, int16_t& value) override
        {
            calc.calc(name, &value, 2);
        }
        void inout(InoutName_T name, int64_t& value) override
        {
            calc.calc(name, &value, 8);
        }
        void inout(InoutName_T name, float& value) override
        {
            calc.calc(name, &value, 4);
        }
        void inout(InoutName_T name, double& value) override
        {
            calc.calc(name, &value, 8);
        }
        void inout(InoutName_T name, std::string& value) override
        {
            calc.calc(name, value.c_str(), value.size());
        }
        void inoutfixstring(InoutName_T name, char* value, int& size, int , int ) override
        {
            calc.calc(name, value, size);
        }
        void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
            calc.calc(name, buffer, bufferSize);
        }

        Serializable* inout(InoutName_T , Serializable* value) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*>(value);
            if (opsO != nullptr) {
                value->serialize(this);
            }
            return value;
        }
        void inout(InoutName_T , Serializable& value) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*>(&value);
            if (opsO != nullptr) {
                value.serialize(this);
            }
        }
        Serializable* inout(InoutName_T , Serializable* value, int ) override
        {
            OPSObject* opsO = dynamic_cast<OPSObject*>(value);
            if (opsO != nullptr) {
                value->serialize(this);
            }
            return value;
        }

        void inout(InoutName_T name, std::vector<bool>& value) override
        {
            for (bool b : value) {
                inout(name, b);
            }
        }
        void inout(InoutName_T name, std::vector<char>& value) override
        {
            calc.calc(name, value.data(), value.size());
        }
        void inout(InoutName_T name, std::vector<uint8_t>& value) override
        {
            calc.calc(name, value.data(), value.size());
        }
        void inout(InoutName_T name, std::vector<int>& value) override
        {
            calc.calc(name, value.data(), 4 * value.size());
        }
        void inout(InoutName_T name, std::vector<int16_t>& value) override
        {
            calc.calc(name, value.data(), 2 * value.size());
        }
        void inout(InoutName_T name, std::vector<int64_t>& value) override
        {
            calc.calc(name, value.data(), 8 * value.size());
        }
        void inout(InoutName_T name, std::vector<float>& value) override
        {
            calc.calc(name, value.data(), 4 * value.size());
        }
        void inout(InoutName_T name, std::vector<double>& value) override
        {
            calc.calc(name, value.data(), 8 * value.size());
        }
        void inout(InoutName_T name, std::vector<std::string>& value) override
        {
            for (auto& s : value) {
                inout(name, s);
            }
        }

        void inoutfixarr(InoutName_T name, bool* value, int numElements, int ) override
        {
            for (int i = 0; i < numElements; i++) {
                inout(name, value[i]);
            }
        }
        void inoutfixarr(InoutName_T name, char* value, int numElements, int ) override
        {
            calc.calc(name, value, numElements);
        }
        void inoutfixarr(InoutName_T name, uint8_t* value, int numElements, int) override
        {
            calc.calc(name, value, numElements);
        }
        void inoutfixarr(InoutName_T name, int* value, int numElements, int ) override
        {
            calc.calc(name, value, 4 * numElements);
        }
        void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int ) override
        {
            calc.calc(name, value, 2 * numElements);
        }
        void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int ) override
        {
            calc.calc(name, value, 8 * numElements);
        }
        void inoutfixarr(InoutName_T name, float* value, int numElements, int ) override
        {
            calc.calc(name, value, 4 * numElements);
        }
        void inoutfixarr(InoutName_T name, double* value, int numElements, int ) override
        {
            calc.calc(name, value, 8 * numElements);
        }
        void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
        {
            for (int i = 0; i < numElements; i++) {
                inout(name, value[i]);
            }
        }

    protected:
        int beginList(InoutName_T, int size) override
        {
            return size;
        }
        void endList(InoutName_T) override
        {
        }
    };
}
