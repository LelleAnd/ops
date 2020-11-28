/**
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

// NOTE: 
// *  Currently only tests for Little Endian.
//    Big Endian tests requires compilation with define ON_BIG_ENDIAN_MACHINE and also 
//    uses the 'preserveWrittenData' parameter.

// TODO:
// *  ReadBytes / WriteBytes over several segments not tested

#include "gtest/gtest.h"

#include "ByteBuffer.h"

using namespace ops;


TEST(Test_ByteBuffer, TestFundamentals) {

	{
		MemoryMap illformed(1, 14);
		EXPECT_THROW(ByteBuffer buf(illformed), ByteBuffer::illformed_memmap);
	}

	MemoryMap map(1, 60);
	ByteBuffer buf(map);

	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);
	EXPECT_EQ(buf.getNrOfSegments(), 1);
	EXPECT_EQ(buf.getSegmentSize(0), 0);
	EXPECT_NE(buf.getSegment(0), nullptr);

	// Write the correct protocol header
	buf.writeProtocol();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 6);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Check the correct protocol header
	EXPECT_TRUE(buf.checkProtocol());

	// ----------------------------------------
	// Garble the protocolID
	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);
	buf.WriteChar('a');
	EXPECT_EQ(buf.GetIndex(), 1);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);

	// Check the garbled protocol header
	EXPECT_FALSE(buf.checkProtocol());

	// ----------------------------------------
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Write the correct protocol header
	buf.writeProtocol();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 6);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Garble the low version
	buf.ReadInt();	// skip 4 bytes
	buf.WriteChar('z');
	EXPECT_EQ(buf.GetIndex(), 5);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);

	// Check the garbled protocol header
	EXPECT_FALSE(buf.checkProtocol());

	// ----------------------------------------
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Write the correct protocol header
	buf.writeProtocol();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 6);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetSize(), 6);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Garble the high version
	buf.ReadInt();	// skip 4 bytes
	buf.ReadChar(); // skip low version
	buf.WriteChar('z');
	EXPECT_EQ(buf.GetIndex(), 6);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);

	// Check the garbled protocol header
	EXPECT_FALSE(buf.checkProtocol());
}

// ==============================================
// Test data

const float TEST_FLOAT = 9.8765f;
const double TEST_DOUBLE = 123.4567;
const int TEST_INT = 456789;
const int16_t TEST_SHORT = 234;
const int64_t TEST_LONG = 1234567890;
const char TEST_CHAR = 64;
const std::string TEST_STRING = "qwerty";
const std::string TEST_FIXSTRING = "fixed string test";


TEST(Test_ByteBuffer, TestCoretypes) {

	MemoryMap map(1, 65000);
	ByteBuffer buf(map);
	int size = 0;

	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	// Fill buffer with values from our core types
	float f1 = TEST_FLOAT;
	buf.WriteFloat(f1);
	size += 4;
	EXPECT_EQ(f1, TEST_FLOAT);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	double d1 = TEST_DOUBLE;
	buf.WriteDouble(d1);
	size += 8;
	EXPECT_EQ(d1, TEST_DOUBLE);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	int16_t s1 = TEST_SHORT;
	buf.WriteShort(s1);
	size += 2;
	EXPECT_EQ(s1, TEST_SHORT);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	int i1 = TEST_INT;
	buf.WriteInt(i1);
	size += 4;
	EXPECT_EQ(i1, TEST_INT);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	int64_t l1 = TEST_LONG;
	buf.WriteLong(l1);
	size += 8;
	EXPECT_EQ(l1, TEST_LONG);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	char c1 = TEST_CHAR;
	buf.WriteChar(c1);
	size += 1;
	EXPECT_EQ(c1, TEST_CHAR);
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::string str1 = TEST_STRING;
	buf.WriteString(str1);
	size += 4 + (int)TEST_STRING.size();
	EXPECT_STREQ(str1.c_str(), TEST_STRING.c_str());
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	str1 = TEST_STRING;
	buf.WriteString(str1);
	size += 4 + (int)TEST_STRING.size();
	EXPECT_STREQ(str1.c_str(), TEST_STRING.c_str());
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	str1 = "";
	buf.WriteString(str1);
	size += 4;
	EXPECT_STREQ(str1.c_str(), "");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	strings::fixed_string<100> fstr1 = TEST_FIXSTRING;
	buf.WriteString(fstr1);
	size += 4 + (int)TEST_FIXSTRING.size();
	EXPECT_STREQ(fstr1.c_str(), TEST_FIXSTRING.c_str());
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	fstr1 = "";
	buf.WriteString(fstr1);
	size += 4;
	EXPECT_STREQ(fstr1.c_str(), "");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	// Reset index so we can read and verify all data
	buf.ResetIndex();
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), 0);
	
	size = 0;

	// Read the values from the buffer
	float f2 = buf.ReadFloat();
	size += 4;
	EXPECT_EQ(f2, TEST_FLOAT);
	EXPECT_EQ(buf.GetIndex(), size);

	double d2 = buf.ReadDouble();
	size += 8;
	EXPECT_EQ(d2, TEST_DOUBLE);
	EXPECT_EQ(buf.GetIndex(), size);

	int16_t s2 = buf.ReadShort();
	size += 2;
	EXPECT_EQ(s2, TEST_SHORT);
	EXPECT_EQ(buf.GetIndex(), size);

	int i2 = buf.ReadInt();
	size += 4;
	EXPECT_EQ(i2, TEST_INT);
	EXPECT_EQ(buf.GetIndex(), size);

	int64_t l2 = buf.ReadLong();
	size += 8;
	EXPECT_EQ(l2, TEST_LONG);
	EXPECT_EQ(buf.GetIndex(), size);

	char c2 = buf.ReadChar();
	size += 1;
	EXPECT_EQ(c2, TEST_CHAR);
	EXPECT_EQ(buf.GetIndex(), size);

	std::string str2 = buf.ReadString();
	size += 4 + (int)TEST_STRING.size();
	EXPECT_STREQ(str2.c_str(), TEST_STRING.c_str());
	EXPECT_EQ(buf.GetIndex(), size);

	str2 = "";
	buf.ReadString(str2);
	size += 4 + (int)TEST_STRING.size();
	EXPECT_STREQ(str2.c_str(), TEST_STRING.c_str());
	EXPECT_EQ(buf.GetIndex(), size);

	str2 = buf.ReadString();
	size += 4;
	EXPECT_STREQ(str2.c_str(), "");
	EXPECT_EQ(buf.GetIndex(), size);

	strings::fixed_string<100> fstr2;
	buf.ReadString(fstr2);
	size += 4 + (int)TEST_FIXSTRING.size();
	EXPECT_STREQ(fstr2.c_str(), TEST_FIXSTRING.c_str());
	EXPECT_EQ(buf.GetIndex(), size);

	fstr2 = buf.ReadString();
	size += 4;
	EXPECT_STREQ(fstr2.c_str(), "");
	EXPECT_EQ(buf.GetIndex(), size);
}

TEST(Test_ByteBuffer, TestTooLargeString) {

	MemoryMap map(1, 16);
	ByteBuffer buf(map);

	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Should fit exactly in buffer 
	std::string ttt("123456123456");
	buf.WriteString(ttt);
	EXPECT_EQ(buf.GetSize(), 16);
	EXPECT_EQ(buf.GetIndex(), 16);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);

	std::string str2 = buf.ReadString();
	EXPECT_STREQ(str2.c_str(), "123456123456");
	EXPECT_EQ(buf.GetIndex(), 16);

	// Clear buffer
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Write a too large string
	ttt += "7";
	EXPECT_THROW(buf.WriteString(ttt), MemoryMapException);

	// Clear buffer
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Try to read a too large string
	// We do this by writing garbage to buffer and try to read a string
	buf.WriteInt(13);		// 13 is < shortStringBufferLength(255)
	buf.ResetIndex();

	EXPECT_EQ(buf.GetIndex(), 0);

	EXPECT_THROW(buf.ReadString(str2), MemoryMapException);

	// Clear buffer
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Try to read a too large string
	// We do this by writing garbage to buffer and try to read a string
	buf.WriteInt(256);		// 256 > shortStringBufferLength(255)
	buf.ResetIndex();

	EXPECT_EQ(buf.GetIndex(), 0);

	EXPECT_THROW(buf.ReadString(str2), ByteBuffer::data_corrupted);
}

TEST(Test_ByteBuffer, TestTooLargeFixedString) {

	MemoryMap map(1, 16);
	ByteBuffer buf(map);

	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Should fit exactly in buffer 
	strings::fixed_string<40> ttt("123456123456");
	buf.WriteString(ttt);
	EXPECT_EQ(buf.GetSize(), 16);
	EXPECT_EQ(buf.GetIndex(), 16);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);

	strings::fixed_string<20> str2;
	buf.ReadString(str2);
	EXPECT_STREQ(str2.c_str(), "123456123456");
	EXPECT_EQ(buf.GetIndex(), 16);

	// Clear buffer
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Write a too large string
	ttt += "7";
	EXPECT_THROW(buf.WriteString(ttt), MemoryMapException);

	// Clear buffer
	buf.Reset();
	EXPECT_EQ(buf.GetSize(), 0);
	EXPECT_EQ(buf.GetIndex(), 0);

	// Try to read a too large string (larger than fixed_string)
	buf.WriteInt(21);
	EXPECT_THROW(buf.ReadString(str2), ByteBuffer::fixed_string_to_small);
}

// ==================================================
// Test data

std::vector<float> FLOAT_VECTOR;
std::vector<double> DOUBLE_VECTOR;
std::vector<char> CHAR_VECTOR;
std::vector<int16_t> SHORT_VECTOR;
std::vector<int> INT_VECTOR;
std::vector<int64_t> LONG_VECTOR;
std::vector<std::string> STRING_VECTOR;
std::vector<strings::fixed_string<50>> FIXSTRING_VECTOR;
std::vector<bool> BOOL_VECTOR;

void InitializeTestVectors()
{
	FLOAT_VECTOR.clear();
	FLOAT_VECTOR.push_back(-1.345f);
	FLOAT_VECTOR.push_back(0.00675f);
	FLOAT_VECTOR.push_back(12345.876f);

	DOUBLE_VECTOR.clear();
	DOUBLE_VECTOR.push_back(-1234.6543);
	DOUBLE_VECTOR.push_back(2345.1);
	DOUBLE_VECTOR.push_back(93485.0);

	CHAR_VECTOR.clear();
	CHAR_VECTOR.push_back(1);
	CHAR_VECTOR.push_back(17);
	CHAR_VECTOR.push_back(111);
	CHAR_VECTOR.push_back(-103);

	SHORT_VECTOR.clear();
	SHORT_VECTOR.push_back(4);
	SHORT_VECTOR.push_back(7);
	SHORT_VECTOR.push_back(18);

	INT_VECTOR.clear();
	INT_VECTOR.push_back(17);
	INT_VECTOR.push_back(23);
	INT_VECTOR.push_back(35);
	INT_VECTOR.push_back(10000);

	LONG_VECTOR.clear();
	LONG_VECTOR.push_back(1234);
	LONG_VECTOR.push_back(5678);
	LONG_VECTOR.push_back(9100);

	STRING_VECTOR.clear();
	STRING_VECTOR.push_back("hej");
	STRING_VECTOR.push_back("hopp");
	STRING_VECTOR.push_back("i");
	STRING_VECTOR.push_back("lingonskogen");

	BOOL_VECTOR.clear();
	BOOL_VECTOR.push_back(true);
	BOOL_VECTOR.push_back(false);
	BOOL_VECTOR.push_back(false);
	BOOL_VECTOR.push_back(true);
}

// Vector compare templates
template< typename T >
void VectorCompare(std::vector<T>& a, std::vector<T>& b, std::string msg)
{
	EXPECT_EQ(a.size(), b.size()) << msg;
	for (unsigned int i = 0; i < a.size(); i++) {
		EXPECT_EQ(a[i], b[i]) << msg;
	}
}

template<>
void VectorCompare(std::vector<std::string>& a, std::vector<std::string>& b, std::string msg)
{
	EXPECT_EQ(a.size(), b.size()) << msg;
	for (unsigned int i = 0; i < a.size(); i++) {
		EXPECT_STREQ(a[i].c_str(), b[i].c_str()) << msg;
	}
}

TEST(Test_ByteBuffer, TestVectors) {
	
	MemoryMap map(1, 65000);
	ByteBuffer buf(map);
	int size = 0;

	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	InitializeTestVectors();

	// Write vectors to buffer
	std::vector<float> fv(FLOAT_VECTOR);
	buf.WriteFloats(fv);
	size += 4 + ((int)fv.size() * 4);
	VectorCompare<float>(fv, FLOAT_VECTOR, "Write Float vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<double> dv(DOUBLE_VECTOR);
	buf.WriteDoubles(dv);
	size += 4 + ((int)dv.size() * 8);
	VectorCompare<double>(dv, DOUBLE_VECTOR, "Write Double vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<char> cv(CHAR_VECTOR);
	buf.WriteBytes(cv);
	size += 4 + (int)cv.size();
	VectorCompare<char>(cv, CHAR_VECTOR, "Write Char vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int16_t> sv(SHORT_VECTOR);
	buf.WriteShorts(sv);
	size += 4 + ((int)sv.size() * 2);
	VectorCompare<int16_t>(sv, SHORT_VECTOR, "Write Short vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int> iv(INT_VECTOR);
	buf.WriteInts(iv);
	size += 4 + ((int)iv.size() * 4);
	VectorCompare<int>(iv, INT_VECTOR, "Write Int vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int64_t> lv(LONG_VECTOR);
	buf.WriteLongs(lv);
	size += 4 + ((int)lv.size() * 8);
	VectorCompare<int64_t>(lv, LONG_VECTOR, "Write Long vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<std::string> strv(STRING_VECTOR);
	buf.WriteStrings(strv);
	size += 4;
	for (unsigned int i = 0; i < strv.size(); i++) { size += 4 + (int)strv[i].size(); }
	VectorCompare(strv, STRING_VECTOR, "Write String vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<bool> bv(BOOL_VECTOR);
	buf.WriteBooleans(bv);
	size += 4 + (int)bv.size();
	VectorCompare<bool>(bv, BOOL_VECTOR, "Write Bool vector");
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	// Reset index so we can read and verify all data
	buf.ResetIndex();
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), 0);

	size = 0;

	// Read the vectors from the buffer
	std::vector<float> fvv;
	buf.ReadFloats(fvv);
	size += 4 + ((int)fv.size() * 4);
	VectorCompare<float>(fv, fvv, "Read Float vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<double> dvv;
	buf.ReadDoubles(dvv);
	size += 4 + ((int)dv.size() * 8);
	VectorCompare<double>(dv, dvv, "Read Double vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<char> cvv;
	buf.ReadBytes(cvv);
	size += 4 + (int)cv.size();
	VectorCompare<char>(cv, cvv, "Read Char vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int16_t> svv;
	buf.ReadShorts(svv);
	size += 4 + ((int)sv.size() * 2);
	VectorCompare<int16_t>(sv, svv, "Read Short vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int> ivv;
	buf.ReadInts(ivv);
	size += 4 + ((int)iv.size() * 4);
	VectorCompare<int>(iv, ivv, "Read Int vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<int64_t> lvv;
	buf.ReadLongs(lvv);
	size += 4 + ((int)lv.size() * 8);
	VectorCompare<int64_t>(lv, lvv, "Read Long vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<std::string> strvv;
	buf.ReadStrings(strvv);
	size += 4;
	for (unsigned int i = 0; i < strv.size(); i++) { size += 4 + (int)strv[i].size(); }
	VectorCompare(strv, strvv, "Read String vector");
	EXPECT_EQ(buf.GetIndex(), size);

	std::vector<bool> bvv;
	buf.ReadBooleans(bvv);
	size += 4 + (int)bv.size();
	VectorCompare<bool>(bv, bvv, "Read Bool vector");
	EXPECT_EQ(buf.GetIndex(), size);
}

TEST(Test_ByteBuffer, TestSegmentFundamentals) {

	MemoryMap map(4, 16);
	ByteBuffer buf(map);
	int size = 0;

	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);
	EXPECT_EQ(buf.getNrOfSegments(), 1);

	// Start buffer with a segment header in segment 0
	buf.writeNewSegment();
	size += 14;
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);

	buf.WriteShort(2);
	size += 2;
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);
	EXPECT_EQ(buf.getNrOfSegments(), 1);

	// next will change to segment 1, write header & then value
	buf.WriteShort(4);
	size += 14 + 2;
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), 16);
	EXPECT_EQ(buf.getNrOfSegments(), 2);

	// next will change to segment 2, write header & then first 2 bytes from value then
	// change to segment 3, write a new header and the last 2 bytes from value
	buf.WriteInt(6);
	size += 14 + 2 + 14 + 2;
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), 16);
	EXPECT_EQ(buf.GetSize(), map.getTotalSize());
	EXPECT_EQ(buf.getNrOfSegments(), 4);

	buf.finish();

	EXPECT_EQ(buf.getNrOfSegments(), 4);
	EXPECT_EQ(buf.getSegmentSize(0), 16);
	EXPECT_EQ(buf.getSegmentSize(1), 16);
	EXPECT_EQ(buf.getSegmentSize(2), 16);
	EXPECT_EQ(buf.getSegmentSize(3), 16);
	EXPECT_NE(buf.getSegment(0), nullptr);
	EXPECT_NE(buf.getSegment(1), nullptr);
	EXPECT_NE(buf.getSegment(2), nullptr);
	EXPECT_NE(buf.getSegment(3), nullptr);

	// Copy content to another buffer and verify content
	char buffer[64];
	map.copyToBytes(buffer, 0, 63);

	MemoryMap tmp(buffer, sizeof(buffer));
	ByteBuffer buf2(tmp);

	// Segment 0
	EXPECT_TRUE(buf2.checkProtocol());
	EXPECT_EQ(buf2.ReadInt(), 4);
	EXPECT_EQ(buf2.ReadInt(), 0);

	EXPECT_EQ(buf2.ReadShort(), 2);

	// Segment 1
	EXPECT_TRUE(buf2.checkProtocol());
	EXPECT_EQ(buf2.ReadInt(), 4);
	EXPECT_EQ(buf2.ReadInt(), 1);

	EXPECT_EQ(buf2.ReadShort(), 4);

	// Segment 2
	EXPECT_TRUE(buf2.checkProtocol());
	EXPECT_EQ(buf2.ReadInt(), 4);
	EXPECT_EQ(buf2.ReadInt(), 2);

	EXPECT_EQ(buf2.ReadShort(), 6);		// 2 LS bytes of integer value (6)

	// Segment 3
	EXPECT_TRUE(buf2.checkProtocol());
	EXPECT_EQ(buf2.ReadInt(), 4);
	EXPECT_EQ(buf2.ReadInt(), 3);

	EXPECT_EQ(buf2.ReadShort(), 0);		// 2 MS bytes of integer value (6)

	// Verify content in original buffer
	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);
	EXPECT_EQ(buf.getNrOfSegments(), 1);

	// readNewSegment() is private so read the first segment header in a compatible way
	EXPECT_TRUE(buf.checkProtocol());
	EXPECT_EQ(buf.ReadInt(), 4);
	EXPECT_EQ(buf.ReadInt(), 0);

	// then read the actual values written (readNewSegment() will be called when needed)
	EXPECT_EQ(buf.ReadShort(), 2);
	EXPECT_EQ(buf.getNrOfSegments(), 1);
	EXPECT_EQ(buf.ReadShort(), 4);
	EXPECT_EQ(buf.getNrOfSegments(), 2);
	EXPECT_EQ(buf.ReadInt(), 6);

	EXPECT_EQ(buf.GetIndex(), 16);
	EXPECT_EQ(buf.getNrOfSegments(), 4);
}

TEST(Test_ByteBuffer, TestSegmentsMinimal) {

	// Setup a buffer that only have space for a segment header and 1 byte in each segment.
	// This will lead to all data types (larger than 1 byte) to be split in several segments.
	MemoryMap map(1000, 15);
	ByteBuffer buf(map);
	int size = 0;

	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.GetIndex(), size);
	EXPECT_EQ(buf.getNrOfSegments(), 1);

	// Start buffer with a segment header in segment 0
	buf.writeNewSegment();

	// Fill buffer with values from our core types
	float const f1 = TEST_FLOAT;
	buf.WriteFloat(f1);
	size += 15 * 4;
	EXPECT_EQ(buf.GetSize(), size);

	double const d1 = TEST_DOUBLE;
	buf.WriteDouble(d1);
	size += 15 * 8;
	EXPECT_EQ(buf.GetSize(), size);

	int16_t const s1 = TEST_SHORT;
	buf.WriteShort(s1);
	size += 15 * 2;
	EXPECT_EQ(buf.GetSize(), size);

	int const i1 = TEST_INT;
	buf.WriteInt(i1);
	size += 15 * 4;
	EXPECT_EQ(buf.GetSize(), size);

	int64_t const l1 = TEST_LONG;
	buf.WriteLong(l1);
	size += 15 * 8;
	EXPECT_EQ(buf.GetSize(), size);

	char const c1 = TEST_CHAR;
	buf.WriteChar(c1);
	size += 15 * 1;
	EXPECT_EQ(buf.GetSize(), size);

	const std::string str1 = TEST_STRING;
	buf.WriteString(str1);
	size += 15 * (4 + (int)TEST_STRING.size());
	EXPECT_EQ(buf.GetSize(), size);

	const strings::fixed_string<100> fstr1 = TEST_FIXSTRING;
	buf.WriteString(fstr1);
	size += 15 * (4 + (int)TEST_FIXSTRING.size());
	EXPECT_EQ(buf.GetSize(), size);
	EXPECT_EQ(buf.getNrOfSegments(), 58);

	buf.ResetIndex();
	EXPECT_EQ(buf.GetIndex(), 0);
	EXPECT_EQ(buf.getNrOfSegments(), 1);

	// readNewSegment() is private so read the first segment header in a compatible way
	EXPECT_TRUE(buf.checkProtocol());
	buf.ReadInt();		// Skip number of segments
	EXPECT_EQ(buf.ReadInt(), 0);

	// Read the values from the buffer
	float f2 = buf.ReadFloat();
	EXPECT_EQ(f2, TEST_FLOAT);

	double d2 = buf.ReadDouble();
	EXPECT_EQ(d2, TEST_DOUBLE);

	int16_t s2 = buf.ReadShort();
	EXPECT_EQ(s2, TEST_SHORT);

	int i2 = buf.ReadInt();
	EXPECT_EQ(i2, TEST_INT);

	int64_t l2 = buf.ReadLong();
	EXPECT_EQ(l2, TEST_LONG);

	char c2 = buf.ReadChar();
	EXPECT_EQ(c2, TEST_CHAR);

	std::string str2 = buf.ReadString();
	EXPECT_STREQ(str2.c_str(), TEST_STRING.c_str());

	strings::fixed_string<100> fstr2;
	buf.ReadString(fstr2);
	EXPECT_STREQ(fstr2.c_str(), TEST_FIXSTRING.c_str());

	EXPECT_EQ(buf.getNrOfSegments(), 58);
}
