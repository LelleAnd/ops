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

#ifndef ops_LockableH
#define ops_LockableH

#include <memory>

#include "OPSTypeDefs.h"

namespace ops
{
	class Lockable
	{
	private:
        class InternalLock;
        std::unique_ptr<InternalLock> _lock;

	public:
		Lockable();
		Lockable(const Lockable& l);
		Lockable& operator= (const Lockable& l);
        Lockable(Lockable&&) = delete;
        Lockable& operator=(Lockable&&) = delete;

		bool lock();
        bool trylock();
        void unlock();
		virtual ~Lockable();
	};

	class SafeLock
	{
	private:
		Lockable& _lockable;
	public:
#ifdef OPS_C14_DETECTED
		[[deprecated("Deprecated. Use constructor taking reference instead")]]
#endif
		explicit SafeLock(Lockable* lockable) : _lockable(*lockable)
		{
			_lockable.lock();
		}
		explicit SafeLock(Lockable& lockable) : _lockable(lockable)
		{
			_lockable.lock();
		}
		~SafeLock()
		{
			_lockable.unlock();
		}
		SafeLock() = delete;
		SafeLock(const SafeLock&) = delete;
		SafeLock& operator= (const SafeLock& l) = delete;
		SafeLock(SafeLock&&) = delete;
		SafeLock& operator=(SafeLock&&) = delete;
	};

    class SafeTryLock
    {
    private:
        Lockable& _lockable;
		bool locked{ false };
    public:
#ifdef OPS_C14_DETECTED
		[[deprecated("Deprecated. Use constructor taking reference instead")]]
#endif
		explicit SafeTryLock(Lockable* lockable) : _lockable(*lockable)
        {
            locked = _lockable.trylock();
        }
		explicit SafeTryLock(Lockable& lockable) : _lockable(lockable)
		{
			locked = _lockable.trylock();
		}
		bool isLocked() noexcept { return locked; }
        ~SafeTryLock()
        {
			if (locked) { _lockable.unlock(); }
        }
		SafeTryLock() = delete;
		SafeTryLock(const SafeTryLock&) = delete;
		SafeTryLock& operator= (const SafeTryLock& l) = delete;
		SafeTryLock(SafeTryLock&&) = delete;
		SafeTryLock& operator=(SafeTryLock&&) = delete;
	};

}
#endif
