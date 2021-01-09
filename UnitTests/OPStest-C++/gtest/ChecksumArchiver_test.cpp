/**
*
* Copyright (C) 2020 Lennart Andersson.
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

// TODO:
//

#include <sstream>

#include "gtest/gtest.h"

#include "ChecksumArchiver.h"

#include "SerDesObjects.h"

using namespace ops;

// ===============================
// Helper

void InitObjectChecksum(SerDesObject_Core& obj)
{
    obj.bo = true;                  // 0x01
    obj.ch = 'm';                   // 0x6D
    obj.i16 = -456;                 // 0xFE38
    obj.i32 = 3223;                 // 0x0C97
    obj.i64 = 99999;                // 0x01869F
    obj.f32 = 678.98f;              // 0x4429BEB8
    obj.d64 = 123456789.0;          // 0x419D6F3454000000
    obj.str = "Test";               // 0x54657374
    obj.fstr = "fixed";             // 0x6669786564
    for (int i = 0; i < (int)sizeof(obj.buffer); i++) { obj.buffer[i] = (char)i; }
                                    // 0x00010203040506070809
}

TEST(Test_ChecksumArchiver, TestCoreTypes) {

    ChecksumArchiver<example::calculator_xor_8> chk;

    SerDesObject_Core obj;
    InitObjectChecksum(obj);

    chk.calc.sum = 0;
    obj.serialize(&chk);
    EXPECT_EQ(chk.calc.sum, 0xD0);
    EXPECT_EQ(chk.calc.totalbytes, (size_t)47);
    EXPECT_EQ(chk.calc.totalfields, 11);
}

// ===============================
// Helper

void InitObjectChecksum(SerDesObject_Vectors& obj)
{
    obj.bo = { true, false, false, true };  // 0x01000001           -> 0x00
    obj.ch = { 'm', 's' };                  // 0x6D73               -> 0x1E -> 1E
    obj.i16 = { -456 };                     // 0xFE38               -> 0xC6 -> D8
    obj.i32 = { 3223, -987, 123 };          // 0x00000C97           -> 0x9B -> 43
                                            // 0xFFFFFC25           -> 0xD9 -> 9A
                                            // 0x0000007B           -> 0x7B -> E1
    obj.i64 = { 99999, -7777 };             // 0x000000000001869F   -> 0x18 -> F9
                                            // 0xFFFFFFFFFFFFE19F   -> 0x7E -> 87
    obj.f32 = { 678.98f, -4.654f };         // 0x4429BEB8           -> 0x6B -> EC
                                            // 0xC094ED91           -> 0x28 -> C4
    obj.d64 = { 123456789.0, -999.87654 };  // 0x419D6F3454000000   -> 0xD3 -> 17
                                            // 0xC08F3F0327674D16   -> 0x68 -> 7F
    obj.str = { "Test", "std" };            // 0x54657374           -> 0x36 -> 49
                                            // 0x737464             -> 0x63 -> 2A
    obj.fstr = { "Test", "fixed" };         // 0x54657374           -> 0x36 -> 1C
                                            // 0x6669786564         -> 0x76 -> 6A
}

TEST(Test_ChecksumArchiver, TestVectorTypes) {

    ChecksumArchiver<example::calculator_xor_8> chk;

    SerDesObject_Vectors obj;
    InitObjectChecksum(obj);

    chk.calc.sum = 0;
    obj.serialize(&chk);
    EXPECT_EQ(chk.calc.sum, 0x6A);
    EXPECT_EQ(chk.calc.totalbytes, 4+2+2+12+16+8+16+7+9);
    EXPECT_EQ(chk.calc.totalfields, 1+4+6+2+2);  // boolean & string called for each element
}

// ===============================
// Helper

void InitObjectChecksum(SerDesObject_Fixarrays& obj)
{
    obj.bo[0] = true;  obj.bo[1] = false;  obj.bo[2] = false; obj.bo[3] = true;

    obj.ch[0] = 'm';  obj.ch[1] = 's';

    obj.i16[0] = -456;

    obj.i32[0] = 3223;  obj.i32[1] = -987;  obj.i32[2] = 123;

    obj.i64[0] = 99999;  obj.i64[1] = -7777;

    obj.f32[0] = 678.98f;  obj.f32[1] = -4.654f;

    obj.d64[0] = 123456789.0;  obj.d64[1] = -999.87654;

    obj.str[0] = "Test";  obj.str[1] = "std";

    obj.fstr[0] = "Test";
    obj.fstr[1] = "fixed";
}

TEST(Test_ChecksumArchiver, TestFixedArrays) {

    ChecksumArchiver<example::calculator_xor_8> chk;

    SerDesObject_Fixarrays obj;
    InitObjectChecksum(obj);

    chk.calc.sum = 0;
    obj.serialize(&chk);
    EXPECT_EQ(chk.calc.sum, 0x6A);
    EXPECT_EQ(chk.calc.totalbytes, 4 + 2 + 2 + 12 + 16 + 8 + 16 + 7 + 9);
    EXPECT_EQ(chk.calc.totalfields, 1 + 4 + 6 + 2 + 2);  // boolean & string called for each element
}

// ===============================
// Helper

void InitObjectChecksum(SerDesObject_Serializables& obj)
{
    InitObjectChecksum(obj.obj1);
    InitObjectChecksum(*obj.obj2);
    InitObjectChecksum(*obj.obj3);
    obj.vobj1.push_back(SerDesObject_Core());
    InitObjectChecksum(obj.vobj1[0]);
    obj.vobj2.push_back(new SerDesObject_Core);
    InitObjectChecksum(*obj.vobj2[0]);
    InitObjectChecksum(obj.fixarr1[0]);
    InitObjectChecksum(obj.fixarr1[1]);
    InitObjectChecksum(*obj.fixarr2[0]);
    InitObjectChecksum(*obj.fixarr2[1]);
}

TEST(Test_ChecksumArchiver, TestNonCoreTypes) {

    ChecksumArchiver<example::calculator_xor_8> chk;

    SerDesObject_Serializables obj;
    InitObjectChecksum(obj);

    chk.calc.sum = 0;
    obj.serialize(&chk);
    // Each SerDesObject_Core gives checksum = 0xD0
    // Odd number of objects, gives the same checksum
    EXPECT_EQ(chk.calc.sum, 0xD0);
    EXPECT_EQ(chk.calc.totalbytes, 9*47);
    EXPECT_EQ(chk.calc.totalfields, 1+(9*11));
}

