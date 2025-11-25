/**
*
* Copyright (C) 2025 Lennart Andersson.
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

#include "gtest/gtest.h"

#include "ShmemBuffer.h"

// ===============================

using namespace ops;

namespace internal {

    static uint8_t buffer[SharedMemoryBuffer::min_size + (4*65536u)];

    // 
    static uint8_t swap(uint32_t offs, uint8_t val)
    {
        uint8_t orgval = buffer[offs];
        buffer[offs] = val;
        return orgval;
    }

    // header
    static uint32_t oldestIndex() { return *(uint32_t*)&buffer[0]; }
    static uint32_t numIndexes() { return *(uint32_t*)&buffer[4]; }

    // layout
    static uint8_t headerBits() { return buffer[8]; }
    static uint8_t indexBits() { return buffer[9]; }
    static uint32_t dataBufferSize() { return *(uint32_t*)&buffer[12]; }

    // index area
    static uint64_t indexlistraw(uint32_t idx) { return ((uint64_t*)&buffer[64])[idx & 0x0fff]; }
    static uint32_t indexlistofs(uint32_t idx) { return (indexlistraw(idx) >> 16) & 0xffffffffL; }
    static uint16_t indexlistsiz(uint32_t idx) { return indexlistraw(idx) & 0xffffL; }

}

TEST(Test_SharedMemoryBuffer, Constructor)
{
    // Illegal buffer pointer
    EXPECT_THROW(SharedMemoryBuffer(nullptr, 60000, SharedMemoryBuffer::type::writer),
        SharedMemoryBuffer::illegal_params);
    EXPECT_THROW(SharedMemoryBuffer(nullptr, 60000, SharedMemoryBuffer::type::reader),
        SharedMemoryBuffer::illegal_params);

    // Illegal size
    EXPECT_THROW(SharedMemoryBuffer(internal::buffer, SharedMemoryBuffer::min_size - 1, SharedMemoryBuffer::type::writer),
        SharedMemoryBuffer::illegal_params);
    EXPECT_THROW(SharedMemoryBuffer(internal::buffer, SharedMemoryBuffer::min_size - 1, SharedMemoryBuffer::type::reader),
        SharedMemoryBuffer::illegal_params);

    // Clearing of housekeeping area for writers
    {
        memset(internal::buffer, 44, sizeof(internal::buffer));
        SharedMemoryBuffer shmem(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::writer);

        for (uint32_t i = 0; i < sizeof(internal::buffer); ++i) {
            if ((i >= 8) && (i < 16)) continue;  // Skip layout definition area
            if (internal::buffer[i] != 0u) {
                EXPECT_EQ(i, SharedMemoryBuffer::min_size);
                break;
            }
        }

        EXPECT_EQ(internal::oldestIndex(), 0u);
        EXPECT_EQ(internal::numIndexes(), 0u);

        // Check exported layout
        EXPECT_EQ(internal::headerBits(), 6u);
        EXPECT_EQ(internal::indexBits(), 12u);
        EXPECT_EQ(internal::dataBufferSize(), sizeof(internal::buffer) - SharedMemoryBuffer::min_size);

        // Test for reader that internal layout matches writers (relies on buffer initialised above)
        EXPECT_NO_THROW(SharedMemoryBuffer(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::reader));
    }

    // Test for reader where internal layout don't match writers (relies on buffer initialised above)
    //   headerbits
    uint8_t orgval = internal::swap(8, 33);
    EXPECT_THROW(SharedMemoryBuffer(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::reader),
        SharedMemoryBuffer::layout_mismatch);
    internal::swap(8, orgval);

    //   indexbits
    orgval = internal::swap(9, 37);
    EXPECT_THROW(SharedMemoryBuffer(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::reader),
        SharedMemoryBuffer::layout_mismatch);
    internal::swap(9, orgval);

    //   databuffersize
    EXPECT_THROW(SharedMemoryBuffer(internal::buffer, sizeof(internal::buffer) - 1, SharedMemoryBuffer::type::reader),
        SharedMemoryBuffer::illegal_params);

}

static void VerifyIndex(uint32_t idx, uint32_t offs, uint16_t size)
{
    EXPECT_EQ(internal::indexlistofs(idx), offs - SharedMemoryBuffer::min_size);
    EXPECT_EQ(internal::indexlistsiz(idx), size);
    EXPECT_EQ(internal::indexlistraw(idx + 1), 0u);
}

// Test with small blocks so that the internal index list wrap-around before the buffer space,
// ie. numIndexes will be maxed out but not bufferspace
TEST(Test_SharedMemoryBuffer, Write_Small)
{
    uint8_t source[16]{};

    SharedMemoryBuffer shmem(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::writer);
    
    uint32_t expectedFreeSpace = sizeof(internal::buffer) - shmem.min_size;

    EXPECT_EQ(shmem.debugGetFreespace(), expectedFreeSpace);

    // ----------------------------------------
    // Write one small block
    memset(source, 1, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    
    uint32_t tstidx = 0u;
    uint32_t numidx = 1u;
    uint32_t offs = shmem.min_size;

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), numidx);

    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    int res = memcmp(source, &internal::buffer[offs], sizeof(source));
    EXPECT_EQ(res, 0);
    offs += sizeof(source);

    expectedFreeSpace -= sizeof(source);
    EXPECT_EQ(shmem.debugGetFreespace(), expectedFreeSpace);

    // Write several small blocks
    for (uint32_t i = 2; i <= shmem.max_blocks; ++i) {
        memset(source, i & 0xFF, sizeof(source));
        EXPECT_TRUE(shmem.write(source, sizeof(source)));
        tstidx++;
        numidx++;

        EXPECT_EQ(internal::oldestIndex(), 0u);
        EXPECT_EQ(internal::numIndexes(), numidx);

        VerifyIndex(tstidx, offs, sizeof(source));

        // Check content in buffer
        res = memcmp(source, &internal::buffer[offs], sizeof(source));
        EXPECT_EQ(res, 0);
        offs += sizeof(source);
    }

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), shmem.max_blocks);

    expectedFreeSpace -= (shmem.max_blocks - 1) * sizeof(source);
    EXPECT_EQ(shmem.debugGetFreespace(), expectedFreeSpace);

    // ----------------------------------------
    // Write one small block, should wrap-around internal index list 
    memset(source, 42, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    tstidx++;

    uint32_t oldest = 1u;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), shmem.max_blocks);

    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], sizeof(source));
    EXPECT_EQ(res, 0);
    offs += sizeof(source);

    EXPECT_EQ(shmem.debugGetFreespace(), expectedFreeSpace);

    // Write several small blocks (still with free bufferspace so 'offs' stay within our buffer)
    for (uint32_t i = 2; i <= 2 * shmem.max_blocks; ++i) {
        memset(source, i & 0xFF, sizeof(source));
        EXPECT_TRUE(shmem.write(source, sizeof(source)));
        tstidx++;
        oldest++;

        EXPECT_EQ(internal::oldestIndex(), oldest);
        EXPECT_EQ(internal::numIndexes(), shmem.max_blocks);

        VerifyIndex(tstidx, offs, sizeof(source));

        // Check content in buffer
        res = memcmp(source, &internal::buffer[offs], sizeof(source));
        EXPECT_EQ(res, 0);
        offs += sizeof(source);
    }

    EXPECT_EQ(shmem.debugGetFreespace(), expectedFreeSpace);
}

// Test with large blocks so that the buffer space will wrap-around before the internal index list,
// ie. the bufferspace will be maxed out and the buffer will be removing the oldest indexes
TEST(Test_SharedMemoryBuffer, Write_Large)
{
    static uint8_t source[15000]{};

    // -----------------------------------
    // Try to write a too large block to an empty buffer
    {
        SharedMemoryBuffer shmem(internal::buffer, SharedMemoryBuffer::min_size + 10000u, SharedMemoryBuffer::type::writer);
        EXPECT_EQ(internal::oldestIndex(), 0u);
        EXPECT_EQ(internal::numIndexes(), 0u);

        EXPECT_FALSE(shmem.write(source, sizeof(source)));

        EXPECT_EQ(internal::oldestIndex(), 0u);
        EXPECT_EQ(internal::numIndexes(), 0u);
    }

    // -----------------------------------
    // Write a few large blocks
    SharedMemoryBuffer shmem(internal::buffer, SharedMemoryBuffer::min_size + 40000u, SharedMemoryBuffer::type::writer);

    memset(source, 1, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));

    uint32_t tstidx = 0u;
    uint32_t offs = shmem.min_size;

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 1u);

    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    int res = memcmp(source, &internal::buffer[offs], sizeof(source));
    EXPECT_EQ(res, 0);
    offs += sizeof(source);

    // Content: 111xxxxx

    memset(source, 2, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    tstidx++;

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 2u);

    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], sizeof(source));
    EXPECT_EQ(res, 0);
    offs += sizeof(source);

    // Content: 111222xx

    // Next write will wrap-around, so it need to clear some area and index(es)
    memset(source, 3, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    tstidx++;

    uint32_t oldest = 1u;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), 2u);

    EXPECT_EQ(internal::indexlistraw(oldest - 1), 0u);
    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], 10000u);
    EXPECT_EQ(res, 0);
    offs = shmem.min_size;
    res = memcmp(&source[10000u], &internal::buffer[offs], 5000u);
    EXPECT_EQ(res, 0);
    offs += 5000u;

    // Content: 3xx22233

    // This will not wrap-around but need space to be cleared
    memset(source, 4, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    tstidx++;
    oldest++;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), 2u);

    EXPECT_EQ(internal::indexlistraw(oldest - 1), 0u);
    VerifyIndex(tstidx, offs, sizeof(source));

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], sizeof(source));
    EXPECT_EQ(res, 0);
    offs += sizeof(source);

    // Content: 3444xx33

    // Write 2 small blocks which will fit in buffer
    // - first
    memset(source, 5, sizeof(source));
    EXPECT_TRUE(shmem.write(source, 50u));
    tstidx++;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), 3u);

    VerifyIndex(tstidx, offs, 50u);

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], 50u);
    EXPECT_EQ(res, 0);
    offs += 50u;

    // - second
    memset(source, 6, sizeof(source));
    EXPECT_TRUE(shmem.write(source, 50u));
    tstidx++;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), 4u);

    VerifyIndex(tstidx, offs, 50u);

    // Check content in buffer
    res = memcmp(source, &internal::buffer[offs], 50u);
    EXPECT_EQ(res, 0);
    offs += 50u;

    // Try to write a too large buffer, will clear buffer
    EXPECT_FALSE(shmem.write(source, 0xFFFFu));
    oldest += 4;

    EXPECT_EQ(internal::oldestIndex(), oldest);
    EXPECT_EQ(internal::numIndexes(), 0u);

    EXPECT_EQ(internal::indexlistraw(oldest - 4), 0u);
    EXPECT_EQ(internal::indexlistraw(oldest - 3), 0u);
    EXPECT_EQ(internal::indexlistraw(oldest - 2), 0u);
    EXPECT_EQ(internal::indexlistraw(oldest - 1), 0u);
    EXPECT_EQ(internal::indexlistraw(oldest    ), 0u);
    EXPECT_EQ(internal::indexlistraw(oldest + 1), 0u);
}

TEST(Test_SharedMemoryBuffer, Write_Read_Small)
{
    uint8_t source[16]{};
    uint8_t dest[256]{};

    SharedMemoryBuffer shmem(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::writer);
    SharedMemoryBuffer shmr1(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::reader);
    SharedMemoryBuffer shmr2(internal::buffer, sizeof(internal::buffer), SharedMemoryBuffer::type::reader);

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 0u);

    // ----------------------------------------
    // Read from an empty buffer
    EXPECT_EQ(shmr1.read(dest, sizeof(dest)), 0u);

    // ----------------------------------------
    // Write some blocks
    for (uint32_t i = 0u; i < 10u; ++i) {
        memset(source, i, sizeof(source));
        EXPECT_TRUE(shmem.write(source, sizeof(source)));
    }

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 10u);

    // Read blocks and compare data using shmr1
    for (uint32_t i = 0u; i < 10u; ++i) {
        memset(source, i, sizeof(source));
        std::size_t num = shmr1.read(dest, sizeof(dest));
        EXPECT_EQ(num, sizeof(source));

        // Check content in buffer
        int res = memcmp(source, dest, num);
        EXPECT_EQ(res, 0);
    }
    EXPECT_EQ(shmr1.read(dest, sizeof(dest)), 0u);

    // Write some more blocks
    for (uint32_t i = 10u; i < 15u; ++i) {
        memset(source, i, sizeof(source));
        EXPECT_TRUE(shmem.write(source, sizeof(source)));
    }
    // Read blocks and compare data using shmr2
    for (uint32_t i = 0u; i < 15u; ++i) {
        memset(source, i, sizeof(source));
        std::size_t num = shmr2.read(dest, sizeof(dest));
        EXPECT_EQ(num, sizeof(source));

        // Check content in buffer
        int res = memcmp(source, dest, num);
        EXPECT_EQ(res, 0);
    }
    EXPECT_EQ(shmr2.read(dest, sizeof(dest)), 0u);

    // Read remaing blocks and compare data using shmr1
    for (uint32_t i = 10u; i < 15u; ++i) {
        memset(source, i, sizeof(source));
        std::size_t num = shmr1.read(dest, sizeof(dest));
        EXPECT_EQ(num, sizeof(source));

        // Check content in buffer
        int res = memcmp(source, dest, num);
        EXPECT_EQ(res, 0);
    }
    EXPECT_EQ(shmr1.read(dest, sizeof(dest)), 0u);

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 15u);
}

TEST(Test_SharedMemoryBuffer, Write_Read_Large)
{
    uint8_t source[1500]{};
    uint8_t dest[2000]{};

    SharedMemoryBuffer shmem(internal::buffer, SharedMemoryBuffer::min_size + 4000u, SharedMemoryBuffer::type::writer);
    SharedMemoryBuffer shmr1(internal::buffer, SharedMemoryBuffer::min_size + 4000u, SharedMemoryBuffer::type::reader);

    EXPECT_EQ(internal::oldestIndex(), 0u);
    EXPECT_EQ(internal::numIndexes(), 0u);

    memset(source, 1, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    memset(source, 2, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));
    memset(source, 3, sizeof(source));
    EXPECT_TRUE(shmem.write(source, sizeof(source)));

    EXPECT_EQ(internal::oldestIndex(), 1u);
    EXPECT_EQ(internal::numIndexes(), 2u);

    // Content: 3xx22233
    // Read blocks and compare data using shmr1
    memset(source, 2, sizeof(source));
    std::size_t num = shmr1.read(dest, sizeof(dest));
    EXPECT_EQ(num, sizeof(source));

    // Check content in buffer
    int res = memcmp(source, dest, num);
    EXPECT_EQ(res, 0);

    // Read blocks and compare data using shmr1
    memset(source, 3, sizeof(source));
    num = shmr1.read(dest, sizeof(dest));
    EXPECT_EQ(num, sizeof(source));

    // Check content in buffer
    res = memcmp(source, dest, num);
    EXPECT_EQ(res, 0);

    EXPECT_EQ(shmr1.read(dest, sizeof(dest)), 0u);

    EXPECT_EQ(internal::oldestIndex(), 1u);
    EXPECT_EQ(internal::numIndexes(), 2u);
}
