/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2025 Lennart Andersson.
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
#include "opsidls/OPSConstants.h"
#include "ByteBuffer.h"

namespace ops
{

    ByteBuffer::ByteBuffer(MemoryMap& mMap, bool ):
        memMap(mMap),
        nextSegmentAt(memMap.getSegmentSize())
    {
		// Check that each segment in map is larger than our needed segment header
		if (memMap.getSegmentSize() <= opsidls::OPSConstants::SEGMENT_HEADER_SIZE) { throw illformed_memmap(); }
    }

	///Resets the whole buffer to creation state
	void ByteBuffer::Reset()
	{
        totalSize = 0;
        index = 0;
        currentSegment = 0;
        nextSegmentAt = memMap.getSegmentSize();
	}

    int ByteBuffer::getNrOfSegments() const noexcept
    {
        return currentSegment + 1;
    }

    int ByteBuffer::getSegmentSize(int const i) const
    {
        if (i < currentSegment)
        {
            return memMap.getSegmentSize();
        }
        else
        {
            return index;
        }
    }

    char* ByteBuffer::getSegment(int const i) const
    {
        return memMap.getSegment(i);
    }

    void ByteBuffer::finish()
    {
        int const oldIndex = index;
        int const oldNextSegmentAt = nextSegmentAt;
        nextSegmentAt = 0;
        int const nrSeg = getNrOfSegments();
        for (int i = 0; i < nrSeg; i++)
        {
            index = 6;
            currentSegment = i;
            nextSegmentAt += memMap.getSegmentSize();
            WriteInt(nrSeg);
        }
        index = oldIndex;
        nextSegmentAt = oldNextSegmentAt;
    }

    ByteBuffer::~ByteBuffer()
    {
    }

    void ByteBuffer::WriteChars(const char* const chars, const int length)
    {
        const int bytesLeftInSegment = memMap.getSegmentSize() - index;
        if (bytesLeftInSegment >= length)
        {
            memcpy((void*) (memMap.getSegment(currentSegment) + index), chars, length);
            index += length;
            totalSize += length;
        }
        else
        {
            memcpy((void*) (memMap.getSegment(currentSegment) + index), chars, bytesLeftInSegment);
            index += bytesLeftInSegment;
            totalSize += bytesLeftInSegment;
            nextSegmentAt += memMap.getSegmentSize();
            currentSegment++;
            writeNewSegment();
            WriteChars(chars + bytesLeftInSegment, length - bytesLeftInSegment);
        }
    }

    void ByteBuffer::ReadChars(char* const chars, const int length)
    {
        int bytesLeftInSegment = memMap.getSegmentSize() - index;
        if (bytesLeftInSegment >= length)
        {
            memcpy((void*) chars, memMap.getSegment(currentSegment) + index, length);
            index += length;
            totalSize += length;
        }
        else
        {
            memcpy((void*) chars, memMap.getSegment(currentSegment) + index, bytesLeftInSegment);
            index += bytesLeftInSegment;
            currentSegment++;
            totalSize += bytesLeftInSegment;
            readNewSegment();
            ReadChars(chars + bytesLeftInSegment, length - bytesLeftInSegment);
        }
    }

    void ByteBuffer::writeNewSegment()
    {
        index = 0;
        writeProtocol();
        const int tInt = 0;
        WriteInt(tInt);
        WriteInt(currentSegment);
    }

    void ByteBuffer::readNewSegment()
    {
        index = 0;
        nextSegmentAt += memMap.getSegmentSize();
        checkProtocol();
        ReadInt();
        ReadInt();
    }

    int ByteBuffer::GetSize() const noexcept
    {
        return totalSize;
    }

    ///Utility method for swaping byte order of basic types (int float etc.)
    void ByteSwap(unsigned char* const b, const int n)
    {
        int i = 0;
        int j = n - 1;
        while (i < j)
        {
            std::swap(b[i], b[j]);
            i++;
            j--;
        }
    }

    template<typename T>
    T ByteSwap(const T in)
    {
        T res;
        for (int i = 0, j = sizeof(in) - 1; i < sizeof(in); ++i, --j) {
            ((uint8_t*)&res)[j] = ((uint8_t*)&in)[i];
        }
        return res;
    }

    // -----------------------------------------------------------------

    void ByteBuffer::WriteFloat(const float f)
    {
#ifdef ON_BIG_ENDIAN_MACHINE
        uint32_t tmp = ByteSwap<uint32_t>(*reinterpret_cast<const uint32_t*>(&f));
        WriteChars(((const char*)&tmp), sizeof(tmp));
#else
        WriteChars((const char*)&f, 4);
#endif
    }

    void ByteBuffer::WriteInt(const int i)
    {
#ifdef ON_BIG_ENDIAN_MACHINE
        uint32_t tmp = ByteSwap<uint32_t>(*reinterpret_cast<const uint32_t*>(&i));
        WriteChars(((const char*)&tmp), sizeof(tmp));
#else
        WriteChars((const char*)&i, 4);
#endif
    }

    void ByteBuffer::WriteShort(const int16_t i)
    {
#ifdef ON_BIG_ENDIAN_MACHINE
        uint16_t tmp = ByteSwap<uint16_t>(*reinterpret_cast<const uint16_t*>(&i));
        WriteChars(((const char*)&tmp), sizeof(tmp));
#else
        WriteChars((const char*)&i, 2);
#endif
    }

    void ByteBuffer::WriteLong(const int64_t l)
    {
#ifdef ON_BIG_ENDIAN_MACHINE
        uint64_t tmp = ByteSwap<uint64_t>(*reinterpret_cast<const uint64_t*>(&l));
        WriteChars(((const char*)&tmp), sizeof(tmp));
#else
        WriteChars((const char*)&l, 8);
#endif
    }

    void ByteBuffer::WriteDouble(const double d)
    {
#ifdef ON_BIG_ENDIAN_MACHINE
        uint64_t tmp = ByteSwap<uint64_t>(*reinterpret_cast<const uint64_t*>(&d));
        WriteChars(((const char*)&tmp), sizeof(tmp));
#else
        WriteChars((const char*)&d, 8);
#endif
    }

    void ByteBuffer::WriteChar(const char c)
    {
        WriteChars((const char*)&c, 1);
    }

    void ByteBuffer::WriteByte(const uint8_t c)
    {
        WriteChars((const char*)&c, 1);
    }

    void ByteBuffer::WriteString(const std::string& s)
    {
        const int siz = (int) s.size();
        WriteInt(siz);
        WriteChars(s.c_str(), siz);
    }

	// -----------------------------------------------------------------

    float ByteBuffer::ReadFloat()
    {
        float ret = 0;
        ReadChars((char*)&ret, 4);
#ifdef ON_BIG_ENDIAN_MACHINE
        ByteSwap((unsigned char*)&ret, 4);
#endif
        return ret;
    }

    double ByteBuffer::ReadDouble()
    {
        double ret = 0;
        ReadChars((char*)&ret, 8);
#ifdef ON_BIG_ENDIAN_MACHINE
        ByteSwap((unsigned char*)&ret, 8);
#endif
        return ret;
    }

    int ByteBuffer::ReadInt()
    {
        int ret = 0;
        ReadChars((char*)&ret, 4);
#ifdef ON_BIG_ENDIAN_MACHINE
        ByteSwap((unsigned char*)&ret, 4);
#endif
        return ret;
    }

    int16_t ByteBuffer::ReadShort()
    {
        int16_t ret = 0;
        ReadChars((char*)&ret, 2);
#ifdef ON_BIG_ENDIAN_MACHINE
        ByteSwap((unsigned char*)&ret, 2);
#endif
        return ret;
    }

    int64_t ByteBuffer::ReadLong()
    {
        int64_t ret = 0;
        ReadChars((char*)&ret, 8);
#ifdef ON_BIG_ENDIAN_MACHINE
        ByteSwap((unsigned char*)&ret, 8);
#endif
        return ret;
    }

    char ByteBuffer::ReadChar()
    {
        char ret = 0;
        ReadChars((char*)&ret, 1);
        return ret;
    }

    uint8_t ByteBuffer::ReadByte()
    {
        uint8_t ret = 0;
        ReadChars((char*)&ret, 1);
        return ret;
    }

    std::string ByteBuffer::ReadString()
    {
        int length = ReadInt();
		if (length <= 0) { return ""; }

		if (length <= shortStringBufferLength) {
			ReadChars(shortStringBuffer, length);
			shortStringBuffer[length] = '\0';
			return shortStringBuffer;
		}

		// This is just a sanity check so that we don't try to allocate a 
		// lot of memory below. ReadChars does the actual check on the exact limits.
		if (length > memMap.getTotalSize()) { throw data_corrupted(); }

        char* text = new char[length];
        ReadChars(text, length);
        std::string ret(text, length);
        delete[] text;

		return ret;
    }

	void ByteBuffer::ReadString(std::string& value)
	{
		value = ReadString();
	}

	// -----------------------------------------------------------------

    void ByteBuffer::ReadBooleans(std::vector<bool>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, false);
        for (int i = 0; i < size; i++)
        {
            out[i] = ReadChar() > 0;
        }
    }

    void ByteBuffer::WriteBooleans(const std::vector<bool>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        for (int i = 0; i < size; i++)
        {
            if (out[i]) {
                WriteChar(1);
            } else {
                WriteChar(0);
            }
        }
    }

    void ByteBuffer::ReadBytes(std::vector<char>& out)
    {
        int const length = ReadInt();
        out.reserve(length);
        out.resize(length, 0);
        if (length > 0)
        {
            ReadChars((char*)&out[0], length);
        }
    }

    void ByteBuffer::ReadBytes(std::vector<uint8_t>& out)
    {
        int const length = ReadInt();
        out.reserve(length);
        out.resize(length, 0);
        if (length > 0)
        {
            ReadChars((char*)&out[0], length);
        }
    }

    void ByteBuffer::WriteBytes(const std::vector<char>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
            WriteChars((const char*)&out[0], size);
        }
    }

    void ByteBuffer::WriteBytes(const std::vector<uint8_t>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
            WriteChars((const char*)&out[0], size);
        }
    }

    void ByteBuffer::ReadDoubles(std::vector<double>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, 0);
        if (size > 0)
        {
            ReadChars((char*) & out[0], size * 8);
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                ByteSwap((unsigned char*) &out[i], 8);
            }
#endif
        }
    }

    void ByteBuffer::WriteDoubles(const std::vector<double>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                uint64_t tmp = ByteSwap<uint64_t>(*reinterpret_cast<const uint64_t*>(&out[i]));
                WriteChars(((const char*)&tmp), sizeof(tmp));
            }
#else
            WriteChars((const char*)&out[0], size * 8);
#endif
        }
    }

    void ByteBuffer::ReadInts(std::vector<int>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, 0);
        if (size > 0)
        {
            ReadChars((char*) & out[0], size * 4);
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                ByteSwap((unsigned char*) &out[i], 4);
            }
#endif
        }
    }

    void ByteBuffer::WriteInts(const std::vector<int>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                uint32_t tmp = ByteSwap<uint32_t>(*reinterpret_cast<const uint32_t*>(&out[i]));
                WriteChars(((const char*)&tmp), sizeof(tmp));
            }
#else
            WriteChars((const char*)&out[0], size * 4);
#endif
        }
    }

    void ByteBuffer::ReadShorts(std::vector<int16_t>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, 0);
        if (size > 0)
        {
            ReadChars((char*) & out[0], size * 2);
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                ByteSwap((unsigned char*) &out[i], 2);
            }
#endif
        }
    }

    void ByteBuffer::WriteShorts(const std::vector<int16_t>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                uint16_t tmp = ByteSwap<uint16_t>(*reinterpret_cast<const uint16_t*>(&out[i]));
                WriteChars(((const char*)&tmp), sizeof(tmp));
            }
#else
            WriteChars((const char*)&out[0], size * 2);
#endif
        }
    }

    void ByteBuffer::ReadFloats(std::vector<float>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, 0.0);
        if (size > 0)
        {
            ReadChars((char*) & out[0], size * 4);
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                ByteSwap((unsigned char*) &out[i], 4);
            }
#endif
        }
    }

    void ByteBuffer::WriteFloats(const std::vector<float>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                uint32_t tmp = ByteSwap<uint32_t>(*reinterpret_cast<const uint32_t*>(&out[i]));
                WriteChars(((const char*)&tmp), sizeof(tmp));
            }
#else
            WriteChars((const char*)&out[0], size * 4);
#endif
        }
    }

    void ByteBuffer::ReadLongs(std::vector<int64_t>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, 0);
        if (size > 0)
        {
            ReadChars((char*) & out[0], size * 8);
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                ByteSwap((unsigned char*) &out[i], 8);
            }
#endif
        }
    }

    void ByteBuffer::WriteLongs(const std::vector<int64_t>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        if (size > 0)
        {
#ifdef ON_BIG_ENDIAN_MACHINE
            for(unsigned int i = 0; i < out.size() ; i++)
            {
                uint64_t tmp = ByteSwap<uint64_t>(*reinterpret_cast<const uint64_t*>(&out[i]));
                WriteChars(((const char*)&tmp), sizeof(tmp));
            }
#else
            WriteChars((const char*)&out[0], size * 8);
#endif
        }
    }

    void ByteBuffer::ReadStrings(std::vector<std::string>& out)
    {
        int const size = ReadInt();
        out.reserve(size);
        out.resize(size, "");
        for (int i = 0; i < size; i++)
        {
            out[i] = ReadString();
        }
    }

    void ByteBuffer::WriteStrings(const std::vector<std::string>& out)
    {
        int const size = (int)out.size();
        WriteInt(size);
        for (int i = 0; i < size; i++)
        {
            WriteString(out[i]);
        }
    }

	const char versionHigh = 0;
	const char versionLow = 5;
	const char protocolID[] = "opsp";

	bool ByteBuffer::checkProtocol()
    {
		char inProtocolID[4];
        ReadChars(&inProtocolID[0], 4);

		if (memcmp(inProtocolID, protocolID, 4) != 0)
        {
            return false;
        }

        char const inVersionLow = ReadChar();
        char const inVersionHigh = ReadChar();

        if ((inVersionHigh != versionHigh) || (inVersionLow != versionLow))
        {
            return false;
        }

        return true;
    }

    void ByteBuffer::writeProtocol()
    {
        WriteChars(protocolID, 4);
        WriteChar(versionLow);
        WriteChar(versionHigh);
    }

    int ByteBuffer::GetIndex() const noexcept
    {
        return index;
    }

    void ByteBuffer::ResetIndex() noexcept
    {
        index = 0;
		currentSegment = 0;
    }

}
