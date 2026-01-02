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

#include "Lockable.h"

#include <mutex>

namespace ops
{
#ifndef REPLACE_OPS_LOCKABLE
    class Lockable::InternalLock : public std::recursive_mutex 
    {
    public:
        InternalLock() = default;
        explicit InternalLock(const InternalLock&) = delete;
        explicit InternalLock(InternalLock&&) = delete;
        InternalLock& operator=(const InternalLock&) = delete;
        InternalLock& operator=(InternalLock&&) = delete;
        virtual ~InternalLock() = default;
    };

    Lockable::Lockable()
    {
        _lock = std::make_unique<InternalLock>();
    }

    Lockable::Lockable(const Lockable&)
    {
        _lock = std::make_unique<InternalLock>();
    }

    Lockable& Lockable::operator=(const Lockable& rhs)
    {
        if (this != &rhs) {
            std::unique_ptr<InternalLock> tmp = std::make_unique<InternalLock>();
            std::swap(_lock, tmp);
        }
        return *this;
    }

    bool Lockable::lock()
    {
        _lock->lock();
        return true;
    }

    bool Lockable::trylock()
    {
        return _lock->try_lock();
    }

    void Lockable::unlock()
    {
        _lock->unlock();
    }

    Lockable::~Lockable()
    {
    }
#endif
}
