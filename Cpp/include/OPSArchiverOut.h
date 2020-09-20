/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2020 Lennart Andersson.
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

		void inout(InoutName_T, bool& value) override
        {
            char ch = 0;
            value ? ch = 1 : ch = 0;
            buf.WriteChar(ch);
        }

        void inout(InoutName_T, char& value) override
        {
            buf.WriteChar(value);
        }

        void inout(InoutName_T, int& value) override
        {
            buf.WriteInt(value);
        }

        void inout(InoutName_T, int16_t& value) override
        {
            buf.WriteShort(value);
        }

        void inout(InoutName_T, int64_t& value) override
        {
            buf.WriteLong(value);
        }

        void inout(InoutName_T, float& value) override
        {
            buf.WriteFloat(value);
        }

        void inout(InoutName_T, double& value) override
        {
            buf.WriteDouble(value);
        }

        void inout(InoutName_T, std::string& value) override
        {
            buf.WriteString(value);
        }

		virtual void inoutfixstring(InoutName_T, char* value, int& size, int /*max_size*/, int /*idx*/) override
		{
			buf.WriteInt(size);
			buf.WriteChars(value, size);
		}
		
		void inout(InoutName_T, char* buffer, int bufferSize) override
        {
            buf.WriteChars(buffer, bufferSize);
        }

        void inout(InoutName_T, Serializable& value) override
        {
			TypeId_T typeS;
            // Add the special marker that we have an implicit version byte first in each idl class
            if (((OPSObject&)value).idlVersionMask != 0) {
                typeS += "0 ";
            }
            // For non-virtual objects type can be sent as a null string
            if (!optNonVirt) {
				typeS += ((OPSObject&)value).getTypeString();
			}
            buf.WriteString(typeS);
            value.serialize(this);
        }

        Serializable* inout(InoutName_T, Serializable* value, int /*element*/) override
        {
            TypeId_T typeS;
            // Add the special marker that we have an implicit version byte first in each idl class
            if (((OPSObject*)value)->idlVersionMask != 0) {
                typeS += "0 ";
            }
            typeS += ((OPSObject*)value)->getTypeString();
            buf.WriteString(typeS);
            value->serialize(this);
            return value;
        }

        Serializable* inout(InoutName_T, Serializable* value) override
        {
            TypeId_T typeS;
            // Add the special marker that we have an implicit version byte first in each idl class
            if (((OPSObject*)value)->idlVersionMask != 0) {
                typeS += "0 ";
            }
            typeS += ((OPSObject*)value)->getTypeString();
            buf.WriteString(typeS);
            value->serialize(this);
            return value;
        }

        void inout(InoutName_T, std::vector<bool>& value) override
        {
            buf.WriteBooleans(value);
        }

        void inout(InoutName_T, std::vector<char>& value) override
        {
            buf.WriteBytes(value);
        }

        void inout(InoutName_T, std::vector<int>& value) override
        {
            buf.WriteInts(value);
        }

        void inout(InoutName_T, std::vector<int16_t>& value) override
        {
            buf.WriteShorts(value);
        }

        void inout(InoutName_T, std::vector<int64_t>& value) override
        {
            buf.WriteLongs(value);
        }

        void inout(InoutName_T, std::vector<float>& value) override
        {
            buf.WriteFloats(value);
        }

        void inout(InoutName_T, std::vector<double>& value) override
        {
            buf.WriteDoubles(value);
        }

        void inout(InoutName_T, std::vector<std::string>& value) override
        {
            buf.WriteStrings(value);
        }

		///TODO all inoutfixarr methods need to handle byte order on BIG ENDIAN SYSTEMS
		void inoutfixarr(InoutName_T, bool* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, char* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, int* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, int16_t* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, int64_t* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, float* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, double* value, int numElements, int totalSize) override
		{
			buf.WriteInt(numElements);
			buf.WriteChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T, std::string* value, int numElements) override
        {
            buf.WriteInt(numElements);
            for(int i = 0; i < numElements; i++) {
                buf.WriteString(value[i]);
            }
        }

        int beginList(InoutName_T, int size) override
        {
            buf.WriteInt(size);
            return size;
        }

        void endList(InoutName_T) noexcept override
        {
            //Nothing to do in this implementation
        }

    private:
        ByteBuffer& buf;
		bool optNonVirt;
    };
}
#endif
