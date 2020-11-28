/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019 Lennart Andersson.
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

#ifndef ops_OPSArchiverOutH
#define ops_OPSArchiverOutH

#include <vector>

#include "OPSTypeDefs.h"
#include "ByteBuffer.h"
#include "Serializable.h"
#include "ArchiverInOut.h"
#include "OPSObject.h"

namespace ops
{

    class OPSArchiverOut : public ArchiverInOut
    {
    public:

        OPSArchiverOut(ByteBuffer& _buf, bool _optNonVirt) noexcept :
			buf(_buf), optNonVirt(_optNonVirt)
        {
        }

        ~OPSArchiverOut()
        {
        }

		// Returns true if it's an output archiver
		virtual bool isOut() noexcept override { return true; }

		void inout(InoutName_T name, bool& value) override
        {
            UNUSED(name)
            char ch = 0;
            value ? ch = 1 : ch = 0;
            buf.WriteChar(ch);
        }

        void inout(InoutName_T name, char& value) override
        {
            UNUSED(name)
            buf.WriteChar(value);
        }

        void inout(InoutName_T name, int& value) override
        {
            UNUSED(name)
            buf.WriteInt(value);
        }

        void inout(InoutName_T name, int16_t& value) override
        {
            UNUSED(name)
            buf.WriteShort(value);
        }

        void inout(InoutName_T name, int64_t& value) override
        {
            UNUSED(name)
            buf.WriteLong(value);
        }

        void inout(InoutName_T name, float& value) override
        {
            UNUSED(name)
            buf.WriteFloat(value);
        }

        void inout(InoutName_T name, double& value) override
        {
            UNUSED(name)
            buf.WriteDouble(value);
        }

        void inout(InoutName_T name, std::string& value) override
        {
            UNUSED(name)
            buf.WriteString(value);
        }

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int idx) override
		{
			UNUSED(name)
			UNUSED(max_size)
			UNUSED(idx)
			buf.WriteInt(size);
			buf.WriteChars(value, size);
		}
		
		void inout(InoutName_T name, char* buffer, int bufferSize) override
        {
            UNUSED(name)
            buf.WriteChars(buffer, bufferSize);
        }

        void inout(InoutName_T name, Serializable& value) override
        {
			UNUSED(name);
			// For non-virtual objects type can be sent as a null string
			TypeId_T typeS;
			if (!optNonVirt) {
				typeS = ((OPSObject&)value).getTypeString();
			}
            buf.WriteString(typeS);
            value.serialize(this);
        }

        Serializable* inout(InoutName_T name, Serializable* value, int element) override
        {
            UNUSED(name)
            UNUSED(element)
            TypeId_T typeS = ((OPSObject*) value)->getTypeString();
            buf.WriteString(typeS);
            value->serialize(this);
            return value;
        }

        Serializable* inout(InoutName_T name, Serializable* value) override
        {
            UNUSED(name)
            const TypeId_T typeS = ((OPSObject*) value)->getTypeString();
            buf.WriteString(typeS);
            value->serialize(this);
            return value;
        }

        void inout(InoutName_T name, std::vector<bool>& value) override
        {
            UNUSED(name)
            buf.WriteBooleans(value);
        }

        void inout(InoutName_T name, std::vector<char>& value) override
        {
            UNUSED(name)
            buf.WriteBytes(value);
        }

        void inout(InoutName_T name, std::vector<int>& value) override
        {
            UNUSED(name)
            buf.WriteInts(value);
        }

        void inout(InoutName_T name, std::vector<int16_t>& value) override
        {
            UNUSED(name)
            buf.WriteShorts(value);
        }

        void inout(InoutName_T name, std::vector<int64_t>& value) override
        {
            UNUSED(name)
            buf.WriteLongs(value);
        }

        void inout(InoutName_T name, std::vector<float>& value) override
        {
            UNUSED(name)
            buf.WriteFloats(value);
        }

        void inout(InoutName_T name, std::vector<double>& value) override
        {
            UNUSED(name)
            buf.WriteDoubles(value);
        }

        void inout(InoutName_T name, std::vector<std::string>& value) override
        {
            UNUSED(name)
            buf.WriteStrings(value);
        }

		///TODO all inoutfixarr methods need to handle byte order on BIG ENDIAN SYSTEMS
		void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) override
		{
			UNUSED(name)
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
        {
            UNUSED(name)
            buf.WriteInt(numElements);
            for(int i = 0; i < numElements; i++) {
                buf.WriteString(value[i]);
            }
        }

        int beginList(InoutName_T name, int size) override
        {
            UNUSED(name)
            buf.WriteInt(size);
            return size;
        }

        void endList(InoutName_T name) noexcept override
        {
            //Nothing to do in this implementation
            UNUSED(name)
        }

    private:
        ByteBuffer& buf;
		bool optNonVirt;
    };
}
#endif
