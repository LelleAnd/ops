/**
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

#include "OPSEvent.h"

#ifndef REPLACE_OPS_EVENT
#include <mutex>
#include <condition_variable>
#endif

namespace ops {

#ifndef REPLACE_OPS_EVENT
    class Event::InternalImpl
    {
    public:
        InternalImpl() = default;
        InternalImpl(const InternalImpl&) = default;
        InternalImpl(InternalImpl&&) = default;
        InternalImpl& operator=(const InternalImpl&) = default;
        InternalImpl& operator=(InternalImpl&&) = default;
        virtual ~InternalImpl() = default;

        bool waitFor(const std::chrono::milliseconds& timeout)
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (signaled) {
                signaled = false;
                return true;
            }
            if (cv.wait_for(lock, timeout, [this] { return this->signaled; })) {
                signaled = false;
                return true;
            }
            return false;
        }

        void signal()
        {
            const std::unique_lock<std::mutex> lock(mtx);
            signaled = true;
            cv.notify_one();
        }

    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool signaled = false;
    };

    Event::Event() : _impl(new InternalImpl) {}

    Event::Event(const Event& ) : _impl(new InternalImpl) {}

    Event& Event::operator= (const Event& rhs)
    {
        if (this != &rhs) {
            _impl.reset(new InternalImpl);
        }
        return *this;
    }

    Event::~Event() {}

    bool Event::waitFor(const std::chrono::milliseconds& timeout)
    {
        return _impl->waitFor(timeout);
    }

    void Event::signal()
    {
        _impl->signal();
    }
#endif
}
