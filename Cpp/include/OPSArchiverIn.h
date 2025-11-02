/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2025 Lennart Andersson.
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

#ifndef ops_OPSArchiverInH
#define ops_OPSArchiverInH

#include <vector>

#include "ByteBuffer.h"
#include "SerializableInheritingTypeFactory.h"
#include "Serializable.h"
#include "ArchiverInOut.h"
#include "OPSObject.h"

namespace ops
{

    class OPSArchiverIn : public ArchiverInOut
    {
    public:
        OPSArchiverIn(ByteBuffer& _buf, SerializableInheritingTypeFactory* _factory) noexcept :
			buf(_buf), factory(_factory)
        {
        }

        ~OPSArchiverIn()
        {
        }

		// Returns true if it's an output archiver
		virtual bool isOut() noexcept override { return false; }

		void inout(InoutName_T, bool& value) override
        {
            value = buf.ReadChar() > 0;
        }

        void inout(InoutName_T, char& value) override
        {
            value = buf.ReadChar();
        }

        void inout(InoutName_T, uint8_t& value) override
        {
            value = buf.ReadByte();
        }

        void inout(InoutName_T, int& value) override
        {
            value = buf.ReadInt();
        }

        void inout(InoutName_T, int16_t& value) override
        {
            value = buf.ReadShort();
        }

        void inout(InoutName_T, int64_t& value) override
        {
            value = buf.ReadLong();
        }

        void inout(InoutName_T, float& value) override
        {
            value = buf.ReadFloat();
        }

        void inout(InoutName_T, double& value) override
        {
            value = buf.ReadDouble();
        }

        void inout(InoutName_T, std::string& value) override
        {
            value = buf.ReadString();
        }

		virtual void inoutfixstring(InoutName_T name, char* value, int& size, int max_size, int /*idx*/) override
		{
			size = buf.ReadInt();
			if (size > max_size) throw ops::ArchiverException("Illegal size of fix string received. name: ", name);
			buf.ReadChars(value, size);
			value[size] = '\0';
		}

		void inout(InoutName_T, char* buffer, int bufferSize) override
		{
			buf.ReadChars(buffer, bufferSize);
		}

		void inout(InoutName_T, Serializable& value) override
        {
            TypeId_T types;
            buf.ReadString(types);  // For non-virtual objects we can always skip the string, may be sent as a null string
            if ((types.size() > 0) && (types[0] == '0')) {
                //Set version mask indicator
                ((OPSObject&)value).idlVersionMask = MaxVersionMask;
            }
            value.serialize(this);
        }

        Serializable* inout(InoutName_T, Serializable* value, int /*element*/) override
        {
            if (value != nullptr) { delete value; }
			TypeId_T types;
			buf.ReadString(types);
            VersionMask_T verMask = 0;
            if ((types.size() > 0) && (types[0] == '0')) {
                verMask = MaxVersionMask;
                types[0] = ' ';
            }
            Serializable* newSer = factory->create(types);
            if (newSer != nullptr) {
                //Set version mask indicator
                ((OPSObject*)newSer)->idlVersionMask = verMask;
                newSer->serialize(this);
            }
            return newSer;
        }

        Serializable* inout(InoutName_T, Serializable* value) override
        {
            if (value != nullptr) { delete value; }
			TypeId_T types;
			buf.ReadString(types);
            VersionMask_T verMask = 0;
            if ((types.size() > 0) && (types[0] == '0')) {
                verMask = MaxVersionMask;
                types[0] = ' ';
            }
            Serializable* newSer = factory->create(types);
            if (newSer != nullptr) {
                //Do this to preserve type information even if slicing has occured.
                ((OPSObject*) newSer)->typesString = types;

                //Set version mask indicator
                ((OPSObject*)newSer)->idlVersionMask = verMask;
                newSer->serialize(this);
            }
            return newSer;
        }

        void inout(InoutName_T, std::vector<bool>& value) override
        {
            buf.ReadBooleans(value);
        }

        void inout(InoutName_T, std::vector<char>& value) override
        {
            buf.ReadBytes(value);
        }

        void inout(InoutName_T, std::vector<uint8_t>& value) override
        {
            buf.ReadBytes(value);
        }

        void inout(InoutName_T, std::vector<int>& value) override
        {
            buf.ReadInts(value);
        }

        void inout(InoutName_T, std::vector<int16_t>& value) override
        {
            buf.ReadShorts(value);
        }

        void inout(InoutName_T, std::vector<int64_t>& value) override
        {
            buf.ReadLongs(value);
        }

        void inout(InoutName_T, std::vector<float>& value) override
        {
            buf.ReadFloats(value);
        }

        void inout(InoutName_T, std::vector<double>& value) override
        {
            buf.ReadDoubles(value);
        }

        void inout(InoutName_T, std::vector<std::string>& value) override
        {
            buf.ReadStrings(value);
        }

		///TODO all inoutfixarr methods need to handle byte order on BIG ENDIAN SYSTEMS
		void inoutfixarr(InoutName_T name, bool* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, char* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

        void inoutfixarr(InoutName_T name, uint8_t* value, int numElements, int totalSize) override
        {
            const int num = buf.ReadInt();
            if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
            buf.ReadChars((char*)value, totalSize);
        }

        void inoutfixarr(InoutName_T name, int* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, int16_t* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, int64_t* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, float* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, double* value, int numElements, int totalSize) override
		{
			const int num = buf.ReadInt();
			if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
			buf.ReadChars((char *)value, totalSize);
		}

		void inoutfixarr(InoutName_T name, std::string* value, int numElements) override
        {
            const int num = buf.ReadInt();
            if (num != numElements) throw ops::ArchiverException("Illegal size of fix array received. name: ", name);
            for(int i = 0; i < numElements; i++) {
                value[i] = buf.ReadString();
            }
        }

        int beginList(InoutName_T, int /*size*/) override
        {
            return buf.ReadInt();
        }

        void endList(InoutName_T) override
        {
            //Nothing to do in this implementation
        }

    private:
        ByteBuffer& buf;
        SerializableInheritingTypeFactory* factory;
    };

}
#endif
