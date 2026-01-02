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

#ifndef ops_ShmemReceiverH
#define ops_ShmemReceiverH

#include "OPSTypeDefs.h"

#ifndef REPLACE_TRANSPORT_LAYER
#ifndef OPS_NO_SHMEM_TRANSPORT

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "Participant.h"
#include "Receiver.h"

#include "ByteBuffer.h"
#include "BasicError.h"
#include "BasicWarning.h"
#include "TimeHelper.h"

#include "ShmemBuffer.h"

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

namespace ops
{
    class ShmemReceiver : public Receiver
    {
    private:
        InternalKey_T shmem_name;
        InternalKey_T mutex_name;
        InternalKey_T condv_name;

        boost::interprocess::shared_memory_object shm_obj;
        boost::interprocess::mapped_region region;
        std::unique_ptr<SharedMemoryBuffer> shmem;

        std::unique_ptr <boost::interprocess::named_mutex> mutex;
        std::unique_ptr <boost::interprocess::named_condition> condv;

    public:
        ShmemReceiver(const InternalKey_T& _name) :
            shmem_name(_name),
            mutex_name(_name + InternalKey_T("-mtx")),
            condv_name(_name + InternalKey_T("-cond"))
        {
        }

		// Used to get the sender IP and port for a received message
		// Only safe to call in callback, before a new asynchWait() is called.
		virtual void getSource(Address_T& address, uint16_t& port) override
        {
            address = "0.0.0.0";
            port = 0;
		}

        virtual void getSource(uint32_t& address, uint16_t& port) override
        {
            address = 0;
            port = 0;
        }

        size_t bytesAvailable() override
        {
            // TODO
            return 0;
        }

        ~ShmemReceiver()
        {
			// Make sure thread is terminated
            cancelled = true;
            try {
                cv.notify_one();
                if (condv) { condv->notify_all(); }
                if (runner.joinable()) { runner.join(); }
            }
            catch (...) {
            }
        }

        virtual uint16_t getLocalPort() noexcept override
        {
            return 0u;
        }

		virtual Address_T getLocalAddress() noexcept override
        {
            return "0.0.0.0";
        }

		virtual bool start() override
		{
            // Set variables indicating that we are "active"
            m_working = true;
            cancelled = false;

            // Start thread
            runner = std::thread(&ShmemReceiver::WorkerThread, this);

            return true;
		}

        virtual void stop() override
        {
            // Signal thread to stop execution
            cancelled = true;

            // Signal events in case thread is waiting on any
            cv.notify_one();
            if (condv) { condv->notify_all(); }

            // Can't wait on thread here since it may be in a callback waiting for us to leave (ie. release locks)
            // We detach the thread object from the thread of execution in case we need to reuse it
            runner.detach();
        }

		// Returns true if all thread work has finished
		virtual bool asyncFinished() override
		{
			return !m_working;
		}

        // Set buffer to use for receive
        virtual void asynchWait(char* bytes, int size) override
        {
            {
                std::lock_guard<std::mutex> lk(mtx);
                data = bytes;
                max_length = size;
            }
            cv.notify_one();
        }

        // Get some collected telemetry from the reciever
        virtual Telemetry getTelemetry() override
        {
            if (shmem) {
                return shmem->getTelemetry();
            } else {
                return Telemetry();
            }
        }


    private:
        std::thread runner;
        bool m_working{ false };
        volatile bool cancelled{ false };

        std::mutex mtx;
        std::condition_variable cv;

        void WaitForSharedMemoryToExist()
        {
            while (!cancelled) {
                try {
                    // Try to open (throws if not exist)
                    shm_obj = boost::interprocess::shared_memory_object(
                        boost::interprocess::open_only, shmem_name.c_str(), boost::interprocess::read_only);

                    mutex = std::make_unique<boost::interprocess::named_mutex>(
                        boost::interprocess::open_only, mutex_name.c_str());
                    condv = std::make_unique<boost::interprocess::named_condition>(
                        boost::interprocess::open_only, condv_name.c_str());

                    // Map the whole shared memory in this process
                    region = boost::interprocess::mapped_region(shm_obj, boost::interprocess::read_only);

                    shmem = std::make_unique<SharedMemoryBuffer>(
                        (uint8_t*)region.get_address(), (uint32_t)region.get_size(), SharedMemoryBuffer::type::reader);
                    
                    return;
                }
                catch (...) {
                }
                TimeHelper::sleep(std::chrono::milliseconds(100));
            }
        }

        int max_length = 0;
        char* data = nullptr;

        void WorkerThread()
        {
            while (!cancelled) {
                WaitForSharedMemoryToExist();

                while (!cancelled) {
                    // Wait for an active buffer (via asynchWait(...)), ie. data != nullptr
                    if (data == nullptr) {
                        std::unique_lock<std::mutex> lk(mtx);
                        cv.wait(lk, [this] { return (data != nullptr) || cancelled; });
                    }
                    if (cancelled) break;

                    // Wait for data in shared memory (signal from other process)
                    {
                        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*mutex);
                        condv->wait(lock);
                    }
                    
                    while (!cancelled && (data != nullptr)) {
                        // Read data from shared memory
                        const std::size_t result = shmem.get()->read((uint8_t*)data, (uint16_t)max_length);
                        if (cancelled || (result == 0)) break;

                        // Callback with data buffer (may give us a new buffer via asynchWait(...))
                        char* dataPtr = data;
                        data = nullptr;
                        notifyNewEvent(BytesSizePair(dataPtr, (int)result));
                    }

                    OPS_SHM_TRACE_CODE(
                        SharedMemoryBuffer::ShmemTelemetry tm(shmem->getTelemetry());
                        static uint32_t prev_left = 0;
                        static uint32_t prev_behind = 0;
                        static uint32_t prev_retry = 0;
                        static uint32_t prev_rdfail = 0;
                        if ( (tm.max_lefttoread != prev_left) || (tm.num_behind != prev_behind) || (tm.num_retry != prev_retry) || (tm.num_rdfail != prev_rdfail)) {
                            OPS_SHM_TRACE("Left: " << tm.max_lefttoread << ", Behind: " << tm.num_behind << ", Retry: " << tm.num_retry <<
                                ", Rdfail: " << tm.num_rdfail <<
                                ", Idx: " << tm.curindex << ", Offs: " << tm.curoffset << ", Oldest: " << tm.oldestidx << "\n");
                            prev_left = tm.max_lefttoread;
                            prev_behind = tm.num_behind;
                            prev_retry = tm.num_retry;
                            prev_rdfail = tm.num_rdfail;
                        }
                    )

                }
            }
            m_working = false;
        }

    };
}
#endif
#endif // REPLACE_TRANSPORT_LAYER
#endif
