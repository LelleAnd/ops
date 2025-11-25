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

#pragma once

#include <cinttypes>
#include <cstring>
#include <exception>
#include <memory>
#include <limits>

#include "Telemetry.h"

namespace ops
{

    // A single writer multiple reader buffer for use in shared memory on a node.
    // Thread safety need to be considered outside of this buffer
    class SharedMemoryBuffer
    {
    public:
        struct ShmemTelemetry
        {
            // for reader
            uint32_t num_behind{ 0 };
            uint32_t num_retry{ 0 };
            uint32_t num_rdfail{ 0 };
            uint32_t lefttoread{ 0 };
            uint32_t max_lefttoread{ 0 };

            // for writer
            uint32_t freespace{ 0 };
            uint32_t min_freespace{ 0 };
            uint32_t num_indexes{ 0 };

            // for both
            uint32_t curindex{ 0 };
            uint32_t curoffset{ 0 };
            uint32_t oldestidx{ 0 };

            // spare
            uint32_t arr[5]{ 0 };

            Telemetry get() const noexcept
            {
                Telemetry t{ {
                    num_behind,
                    num_retry,
                    num_rdfail,
                    lefttoread,
                    max_lefttoread,
                    freespace,
                    min_freespace,
                    num_indexes,
                    curindex,
                    curoffset,
                    oldestidx
                } };
                return t;
            }

            ShmemTelemetry() = default;

            explicit ShmemTelemetry(const Telemetry& t) :
                num_behind(t.arr[0]),
                num_retry(t.arr[1]),
                num_rdfail(t.arr[2]),
                lefttoread(t.arr[3]),
                max_lefttoread(t.arr[4]),
                freespace(t.arr[5]),
                min_freespace(t.arr[6]),
                num_indexes(t.arr[7]),
                curindex(t.arr[8]),
                curoffset(t.arr[9]),
                oldestidx(t.arr[10])
            { }
        };
        static_assert(sizeof(ShmemTelemetry) == sizeof(Telemetry));

    private:
        // Format in shared memory:
        //    0: oldest index and num indexes 64-bits
        //                          xxxxxxxx xxxxxxxx
        //         oldest index     xxxxxxxx
        //         numindexes                xxxxxxxx
        //    8: layout 64-bits
        //         headersizebits   xx
        //         indexbits          xx
        //         spare                xxxx
        //         databufsize               xxxxxxxx
        //   16-63: spare
        static constexpr uint8_t sizeheaderbits{ 6u };
        static constexpr uint32_t sizeheader{ 1u << sizeheaderbits };

        //   64: circular offset array[0..indexmax-1]
        //         indexmax * 64-bit
        static constexpr uint32_t indexbits{ 12u };
        static constexpr uint32_t indexmax{ 1u << indexbits };         // 2^n, to be able to use binary operations
        static constexpr uint32_t indexmask{ indexmax - 1u };
        static constexpr uint32_t sizeoffsetarr{ sizeof(uint64_t) * indexmax };
        static constexpr uint32_t sizenondata{ sizeheader + sizeoffsetarr };

        //   nn: circular data buffer[...]
        //         bytes ...

        uint8_t* const base;
        uint8_t* const offsetarr;
        uint8_t* const databuffer;
        const uint32_t databufsize;

        // Indexes are 32 bits but only the 'n' lowest bits are used as index. The upper bits acts
        // as a 'generation' counter. Index format:
        //          32 bit   xxxxxxxx
        //   Count  nn bit   xxxxx                  Implicit Wrap-a-round counter
        //   Index  nn bit       xxxx               Index into offset array [0..2^n - 1]

        // Writer: Next index in offsetarray to write to
        // Reader: Next index in offsetarray to read from
        uint32_t curindex{ 0u };

        // Index for the oldest data in buffer
        uint32_t oldestindex{ 0u };

        // Number of indexes with valid data
        uint32_t numindexes{ 0u };

        // Writer: Next offset in databuffer to write to
        uint32_t curoffset{ 0u };

        // Writer: Number of bytes left in databuffer
        uint32_t freespace{ databufsize };

        // Reader: Number of indexes available to read
        uint32_t lefttoread{ 0u };

        // Some statistics that can be fetched
        ShmemTelemetry tm{};

        // -------------------------------------------------------------------------

        void updateHeader()
        {
            *((uint32_t*) base      ) = oldestindex;
            *((uint32_t*)(base + 4u)) = numindexes;
        }

        void getHeaderData(uint32_t& oldestidx, uint32_t& numidx) const
        {
            oldestidx = *((uint32_t*)base);
            numidx    = *((uint32_t*)(base + 4u));
        }

        void exportLayout()
        {
            base[8] = sizeheaderbits;
            base[9] = indexbits;
            *(uint32_t*)(base + 12u) = databufsize;
        }

        void getLayout(uint8_t& sizebits, uint8_t& idxbits, uint32_t& size) const
        {
            sizebits = base[8];
            idxbits  = base[9];
            size     = *(uint32_t*)(base + 12u);
        }

        // -------------------------------------------------------------------------

        // Offset format:
        //                   xxxx xxxxxxxx xxxx 
        //   Count  16 bits  xxxx                   Wrap-around counter (gen. counter from curindex)
        //   Offset 32 bits       xxxxxxxx          Offset into data buffer
        //   Size   16 bits                xxxx     Size of data ( > 0)

        void setOffsetEntry(uint32_t idx, uint64_t value)
        {
            *((uint64_t*)(offsetarr + (sizeof(uint64_t) * (idx & indexmask)))) = value;
        }

        uint64_t getOffsetEntryRaw(uint32_t idx) const
        {
            return *((uint64_t*)(offsetarr + (sizeof(uint64_t) * (idx & indexmask))));
        }

        void updateOffsetEntry(uint32_t idx, uint32_t offs, uint16_t size)
        {
            setOffsetEntry(idx, ((uint64_t)((idx >> indexbits) & 0xFFFFu)) << 48u | ((uint64_t)offs) << 16u | (uint64_t)size);
        }

        void getOffsetEntry(uint32_t idx, uint64_t& value, uint32_t& cnt, uint32_t& offs, uint16_t& size) const
        {
            value = getOffsetEntryRaw(idx);
            cnt = (value >> 48u) & 0xFFFFu;
            offs = (value >> 16u) & 0xFFFFFFFFu;
            size = value & 0xFFFFu;
        }

        uint16_t getOffsetEntrySize(uint32_t idx) const
        {
            uint64_t value = getOffsetEntryRaw(idx);
            return (uint16_t)(value & 0xFFFFu);
        }

        // -------------------------------------------------------------------------

        uint32_t freeupspaceBetween(uint32_t lowoffset, uint32_t highoffset)
        {
            uint32_t freed = 0u;

            //   Start at oldestindex
            uint32_t previdx = oldestindex;
            uint32_t prevnum = numindexes;

            //   First count how many to remove, update numindexes and oldestindex
            for (; numindexes > 0; --numindexes) {
                uint64_t raw;
                uint32_t cnt, offs;
                uint16_t size;
                getOffsetEntry(oldestindex, raw, cnt, offs, size);
                
                if ((offs < lowoffset) || (offs > highoffset)) break;

                freed += size;
                ++oldestindex;
            }
            updateHeader();

            //   Then clear the actual entries
            prevnum = prevnum - numindexes;
            for (; prevnum > 0; --prevnum, ++previdx) {
                setOffsetEntry(previdx, 0u);
            }
            return freed;
        }

        // Clear from 'offset' so that freespace >= 'numbytes' 
        void freeupspace(uint32_t offset, uint32_t numbytes)
        {
            if ((offset + numbytes) > databufsize) {
                // wrap-a-round
                // We need atleast the space from offset to the top, so
                // clear all entries with an offset >= offset
                freespace += freeupspaceBetween(offset, databufsize);

                // Update values for ev. more freeup from start
                numbytes -= databufsize - offset;
                offset = 0;
            }

            // We need atleast the space from offset to offset+numbytes, so 
            // clear entries with offset < offset+numbytes
            freespace += freeupspaceBetween(offset, offset + numbytes);

            if (freespace > databufsize) { freespace = databufsize; }
        }

        // -------------------------------------------------------------------------

    public:
        static constexpr uint32_t min_size{ sizenondata };
        static constexpr uint32_t max_blocks{ indexmax - 1u };

        struct illegal_params : public std::exception {
            const char* what() const noexcept override { return "SharedMemoryBuffer(): Illegal parameters"; }
        };

        struct layout_mismatch : public std::exception {
            const char* what() const noexcept override { return "SharedMemoryBuffer(): Layout mismatch"; }
        };

        enum class type { reader, writer };

        // Expects a pointer to some shared memory and its size.
        // Provides write and read methods for transferring data from ONE writer to one or more readers.
        SharedMemoryBuffer(uint8_t* _base, uint32_t size, type t) :
            base(_base), offsetarr(base + sizeheader), databuffer(offsetarr + sizeoffsetarr),
            databufsize((size < min_size) ? 0 : size - min_size)
        {
            if ((base == nullptr) || (size < min_size)) { 
                throw illegal_params();
            }
            if (t == type::writer) {
                // Clear housekeeping vars
                std::memset((void*)base, 0, sizenondata);
                exportLayout();
                tm.freespace = freespace;
                tm.min_freespace = freespace;
            }
            else {
                uint8_t l_sizebits, l_idxbits;
                uint32_t l_size;
                getLayout(l_sizebits, l_idxbits, l_size);
                if ((l_sizebits != sizeheaderbits) || (l_idxbits != indexbits)) {
                    throw layout_mismatch();
                }
                if (l_size != databufsize) {
                    throw illegal_params();
                }
                // Initialize so we are near the top (skipping most old messages)
                getHeaderData(oldestindex, numindexes);
                if (numindexes > 0) {
                    curindex = oldestindex + numindexes - 1;
                }
            }
        }

        // used for debug and unittests 
        uint32_t debugGetFreespace() const noexcept { return freespace; }
        
        Telemetry getTelemetry() noexcept
        {
            tm.curindex = curindex;
            tm.curoffset = curoffset;
            tm.oldestidx = oldestindex;
            return tm.get();
        }

        bool write(uint8_t* buf, uint16_t bufSize)
        {
            if (bufSize == 0) { return false; }

            // Check that bufSize bytes are available in data buffer
            if (bufSize > freespace) {
                // We need to free up space (clear some of the next index entries)
                freeupspace(curoffset, bufSize);
                tm.freespace = freespace;

                // Check if we managed to free up enough space
                if (bufSize > freespace) { return false; }
            }
            
            // Copy data to databuffer[offs]
            const uint32_t entryoffs = curoffset;
            const uint16_t entrysize = bufSize;
            uint32_t num = bufSize;
            uint32_t offs = 0;
            if ((curoffset + num) > databufsize) {
                // wrap-a-round
                num = databufsize - curoffset;
                memcpy((void*)(databuffer + curoffset), (void*)buf, num);
                curoffset = 0;
                freespace -= num;
                offs = num;
                num = bufSize - num;
            }
            memcpy((void*)(databuffer + curoffset), (void*)(buf + offs), num);
            curoffset += num;
            freespace -= num;

            // Clear next entry, incl. update of oldest index if needed
            uint32_t nextindex = curindex + 1;
            if ((numindexes > 0) && (nextindex & indexmask) == (oldestindex & indexmask)) {
                // Compensate freespace with oldest entry skipped
                freespace += getOffsetEntrySize(oldestindex);
                ++oldestindex;
                --numindexes;
            }
            setOffsetEntry(nextindex, 0u);

            // update offsets[idx] with correct offset and size
            updateOffsetEntry(curindex, entryoffs, entrysize);

            ++numindexes;
            updateHeader();

            curindex = nextindex;

            // Statistics
            tm.num_indexes = numindexes;
            tm.freespace = freespace;
            if (freespace < tm.min_freespace) { tm.min_freespace = freespace; }

            return true;
        }

        std::size_t read(uint8_t* buf, uint16_t maxsize)
        {
            const int maxtries = 100;

            for (int i = 0; i < maxtries; ++i) {
                // Calculate index to read from
                //   curindex is next to read, check against oldestindex
                getHeaderData(oldestindex, numindexes);
                if (curindex < oldestindex) {
                    // Either we are behind or we have wrapped-around but oldestindex has not yet done so
                    if ((oldestindex - curindex) < ~indexmask) {
                        // Behind
                        curindex = oldestindex;
                        tm.num_behind++;    // Statistics
                        lefttoread = numindexes;
                    } else {
                        lefttoread = numindexes - (std::numeric_limits<uint32_t>::max() - oldestindex + curindex);
                    }
                } else {
                    // Either we have read some data or oldestindex have wrapped-a-round
                    if ((curindex - oldestindex) > indexmax) {
                        // Behind
                        curindex = oldestindex;
                        tm.num_behind++;    // Statistics
                    }
                    uint32_t already_read = curindex - oldestindex;
                    if (already_read <= numindexes) {
                        lefttoread = numindexes - already_read;
                    } else {
                        lefttoread = 0;
                    }
                }
                // Statistics
                tm.lefttoread = lefttoread;
                if (lefttoread > tm.max_lefttoread) { tm.max_lefttoread = lefttoread; }

                // Nothing to read yet in buffer
                if (lefttoread == 0) { return 0; }

                // Get entry data (if lefttoread == 0, reading offsetEntry may give partially updated data if writer is 
                // updating it simultaneously, hence the test above)
                uint64_t raw;
                uint32_t cnt, num = 0;
                uint16_t size;
                getOffsetEntry(curindex, raw, cnt, curoffset, size);
                if (raw == 0) { return 0; }

                // Some sanity checks
                if (((curindex >> indexbits) & 0xFFFFu) != cnt) {
                    tm.num_rdfail++;    // Statistics
                    continue;
                }
                if (size == 0) {
                    tm.num_rdfail++;    // Statistics
                    continue;
                }

                // Copy memory, with ev. wrap-a-round
                if (size > maxsize) { size = maxsize; }
                size_t result = size;
                if ((curoffset + size) > databufsize) {
                    // wrap-a-round
                    num = databufsize - curoffset;
                    memcpy((void*)buf, (void*)(databuffer + curoffset), num);
                    size -= (uint16_t)num;
                    curoffset = 0;
                }
                memcpy((void*)(buf + num), (void*)(databuffer + curoffset), size);

                // Get entry data again, if changed the read data is not valid and we need to redo
                uint64_t raw2 = getOffsetEntryRaw(curindex++);
                if (raw == raw2) { return result; }

                // Writer has changed data while we were reading, so we are now behind. Try reading again ...

                tm.num_retry++;     // Statistics
            }
            return 0;
        }
    };

}
