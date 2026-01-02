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

#include "ShmemSendDataHandler.h"
#include "ShmemBuffer.h"
#include "BasicError.h"
#include "Participant.h"

#ifndef REPLACE_TRANSPORT_LAYER

#ifndef OPS_NO_SHMEM_TRANSPORT

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

namespace ops
{
    class ShmemSendDataHandler::DummySender : public Sender
    {
        virtual bool sendTo(const char*, const int, const Address_T&, const uint16_t) override
        {
            return false;
        }
        virtual bool open() override
        {
            return true;
        }
        virtual void close() override {}
        virtual uint16_t getLocalPort() override
        {
            return 0;
        }
        virtual Address_T getLocalAddress() override
        {
            return "0.0.0.0";
        }
        virtual uint32_t getLocalAddressHost() override
        {
            return 0;
        }
    };

    using namespace boost::interprocess;

    class ShmemSendDataHandler::Impl
    {
    private:
        InternalKey_T shmem_name;
        InternalKey_T mutex_name;
        InternalKey_T condv_name;

        // Create mutex and condv before shared memory object so that reader knows that they exist when the shared memory exist
        named_mutex mutex;
        named_condition condv;
        shared_memory_object shm_obj;
        mapped_region region;
        std::unique_ptr<SharedMemoryBuffer> shmem;
 
    public:
        Impl(const InternalKey_T& _name, uint64_t size) :
            shmem_name(_name), 
            mutex_name(_name + InternalKey_T("-mtx")),
            condv_name(_name + InternalKey_T("-cond")),
            mutex(create_only, mutex_name.c_str()),
            condv(create_only, condv_name.c_str()),
            shm_obj(create_only, shmem_name.c_str(), read_write)
        {
            shm_obj.truncate(SharedMemoryBuffer::min_size + size);

            // Map the whole shared memory in this process
            region = mapped_region(shm_obj, read_write);

            shmem = std::make_unique<SharedMemoryBuffer>(
                (uint8_t*)region.get_address(), (uint32_t)region.get_size(), SharedMemoryBuffer::type::writer);
        }

        bool write(char* buf, int bufSize)
        {
            const bool result = shmem->write((uint8_t*)buf, (uint16_t)bufSize);

            if (result) {
                // Signal that shared memory updated
                condv.notify_all();
            }

            OPS_SHM_TRACE_CODE(
                SharedMemoryBuffer::ShmemTelemetry tm(shmem->getTelemetry());
                static uint32_t prev_frsp = 0;
                static uint32_t prev_nidx = 0;
                if ((tm.freespace != prev_frsp) || (tm.num_indexes != prev_nidx)) {
                    OPS_SHM_TRACE("Free: " << tm.freespace << ", NumIdx: " << tm.num_indexes
                        << ", CurIdx: " << tm.curindex
                        << ", CurOffs: " << tm.curoffset
                        << "\n");
                    prev_frsp = tm.freespace;
                    prev_nidx = tm.num_indexes;
                }
            )

            return result;
        }
        
        // Get some collected telemetry from the sender
        Telemetry getTelemetry()
        {
            return shmem->getTelemetry();
        }

        ~Impl()
        {
            named_condition::remove(condv_name.c_str());
            named_mutex::remove(mutex_name.c_str());
            shared_memory_object::remove(shmem_name.c_str());
        }
    };

    ShmemSendDataHandler::ShmemSendDataHandler(const InternalKey_T& name, const Topic& top)
    {
        try {
            sender = std::make_unique<DummySender>();
            shmem = std::make_unique<Impl>(name, top.getOutSocketBufferSize());
        } catch (...) {
            ErrorMessage_T msg("Failed to create Shared Memory Handler for: ");
            msg += name;
            ops::BasicError err("ShmemSendDataHandler", "Constructor", msg);
            Participant::reportStaticError(&err);
            throw;
        }
    }

    // Need to be implemented in cpp-file where class Impl is known (due to unique_ptr)
    ShmemSendDataHandler::~ShmemSendDataHandler() = default;

    bool ShmemSendDataHandler::sendData(char* buf, int bufSize, const Topic&)
    {
        SafeLock lock(mutex);

        return shmem.get()->write(buf, bufSize);
    }

    // Get some collected telemetry from the sender
    Telemetry ShmemSendDataHandler::getTelemetry()
    {
        return shmem.get()->getTelemetry();
    }

}
#endif
#endif // REPLACE_TRANSPORT_LAYER
