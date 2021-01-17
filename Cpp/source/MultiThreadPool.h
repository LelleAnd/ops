/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2021 Lennart Andersson.
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

#ifndef ops_MultiThreadPool
#define ops_MultiThreadPool

#include "ThreadPool.h"
#include "Lockable.h"
#include "Runnable.h"
#include "SingleThreadPool.h"

namespace ops
{
	class MultiThreadPool : public ThreadPool 
	{
	public:
		MultiThreadPool()
		{
		}
		virtual void addRunnable(Runnable* runnable)
		{
			SafeLock lock(mutex);
			t1.addRunnable(runnable);
			t2.addRunnable(runnable);
			t3.addRunnable(runnable);
			t4.addRunnable(runnable);
			t5.addRunnable(runnable);
			t6.addRunnable(runnable);
			t7.addRunnable(runnable);
			t8.addRunnable(runnable);
		}
		virtual void removeRunnable(Runnable* runnable)
		{
			SafeLock lock(mutex);
			t1.removeRunnable(runnable);
			t2.removeRunnable(runnable);
			t3.removeRunnable(runnable);
			t4.removeRunnable(runnable);
			t5.removeRunnable(runnable);
			t6.removeRunnable(runnable);
			t7.removeRunnable(runnable);
			t8.removeRunnable(runnable);
		}
		void start()
		{
			t1.start();
			t2.start();
			t3.start();
			t4.start();
			t5.start();
			t6.start();
			t7.start();
			t8.start();
		}

	private:
		Lockable mutex;
		SingleThreadPool t1;
		SingleThreadPool t2;
		SingleThreadPool t3;
		SingleThreadPool t4;
		SingleThreadPool t5;
		SingleThreadPool t6;
		SingleThreadPool t7;
		SingleThreadPool t8;
	};

}

#endif
