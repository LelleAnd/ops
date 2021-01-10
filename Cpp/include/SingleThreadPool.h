/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2021 Lennart Andersson.
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

#ifndef ops_SingleThreadPool
#define ops_SingleThreadPool

#include <vector>
#include <atomic>
#include <chrono>

#include "Thread.h"
#include "ThreadPool.h"
#include "Lockable.h"
#include "Runnable.h"
#include "TimeHelper.h"

namespace ops
{

    class SingleThreadPool : private Thread, public ThreadPool
    {
    public:
        // While we are running, ie. executing run(), we don't allow add/remove to/from the runnables container.
        // And we don't want to hold a mutex while executing runnables (due to the fact that atleast one of the
        // OS:es we run on only allow a thread to hold one (1) lock at a time).

        // That is the reason that we have a separate running flag, do a loop in add and remove, and
        // only hold the lock a short period

        // Add a runnable to the list of things to run. If threadpool has been started, this will hang until
        // the run() method exits, to be backward compatible.
        virtual void addRunnable(Runnable* runnable) override
        {
            for (;;) {
                {
                    SafeLock lock(mutex);
                    if (!running.load()) {
                        runnables.push_back(runnable);
                        return;
                    }
                }
                TimeHelper::sleep(std::chrono::milliseconds(1));
            }
        }

        // Remove a runnable from the list of things to run. If threadpool has been started, this will hang until
        // the run() method exits, to be backward compatible.
        virtual void removeRunnable(Runnable* runnable) override
        {
            for (;;) {
                {
                    SafeLock lock(mutex);
                    if (!running.load()) {
                        std::vector<Runnable*>::iterator it = runnables.begin();
                        for (unsigned int i = 0; i < runnables.size(); i++) {
                            if (runnables[i] == runnable) {
                                it += i;
                                runnables.erase(it);
                                break;
                            }
                        }
                        return;
                    }
                }
                TimeHelper::sleep(std::chrono::milliseconds(1));
            }
        }

        // Starts the thread that calls our run() method
        virtual void start() override
        {
            Thread::start();
        }

        // Called by Thread. While we are running we don't allow changes to the runnables container.
        void run() override
        {
            // Take mutex to make sure add/remove isn't in process of updating the runnables container
            {
                SafeLock lock(mutex);
                running.store(true);
            }
            // We don't want to hold the mutex while executing runnables, so the runnables
            // are instead protected by the running flag, set above and cleared below
            for (auto &r : runnables) {
                try {
                    r->run();
                }
                catch (...) {
                }
            }
            running.store(false);
        }

        virtual ~SingleThreadPool()
        {
            // We need to stop the thread explicitly so that the run() method exits
            // before our mutex destructor is called.
            Thread::stop();
            Thread::join();
        }
        
        bool isRunning() const noexcept { return running.load(); }

    private:
        std::vector<Runnable*> runnables;
        Lockable mutex;
        std::atomic<bool> running{ false };
    };

}
#endif
