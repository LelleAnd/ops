/**
*
* Copyright (C) 2018-2020 Lennart Andersson.
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

#include <atomic>
#include <exception>
#include <thread>

#include "SingleThreadPool.h"

using namespace ops;

// ===============================
// Helper classes

using mytype = std::atomic<uint32_t>;

class MyRunnable : public Runnable
{
public:
    MyRunnable(mytype& state, uint32_t const inc) : _state(state), _inc(inc) {}
    MyRunnable() = delete;
    MyRunnable(const MyRunnable&) = delete;
    MyRunnable(MyRunnable&&) = delete;
    MyRunnable& operator=(const MyRunnable&) = delete;
    MyRunnable& operator=(MyRunnable&&) = delete;
    virtual ~MyRunnable() = default;

    virtual void run() override 
    {
        while (_state.load() != 0) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
        if (_inc == 999) { throw std::exception(); }
        _state += _inc;
    }
private:
    mytype& _state;
    uint32_t _inc;
};

// ===============================

TEST(Test_ThreadPool, Test) {

    mytype s1{ 0 }, s2{ 0 }, s3{ 0 };
    MyRunnable mr1(s1, 1);
    MyRunnable mr2(s2, 2);
    MyRunnable mr3(s3, 3);

    {
        SingleThreadPool pool;
        pool.addRunnable(&mr1);
        pool.addRunnable(&mr2);
        pool.addRunnable(&mr3);
        pool.start();
    }

    EXPECT_EQ(s1, 1u);
    EXPECT_EQ(s2, 2u);
    EXPECT_EQ(s3, 3u);

    // -----------
    s1 = 0;
    s2 = 0;
    s3 = 0;
    {
        SingleThreadPool pool;
        pool.addRunnable(&mr1);
        pool.addRunnable(&mr2);
        pool.addRunnable(&mr3);
        pool.removeRunnable(&mr1);  // remove first
        pool.removeRunnable(&mr3);  // remove last
        pool.start();
    }

    EXPECT_EQ(s1, 0u);
    EXPECT_EQ(s2, 2u);
    EXPECT_EQ(s3, 0u);

    // -----------
    s1 = 0;
    s2 = 0;
    s3 = 0;
    {
        SingleThreadPool pool;
        pool.addRunnable(&mr1);
        pool.addRunnable(&mr2);
        pool.addRunnable(&mr3);
        pool.removeRunnable(&mr2);  // remove in between
        pool.removeRunnable(&mr2);  // remove non existent
        pool.start();
    }

    EXPECT_EQ(s1, 1u);
    EXPECT_EQ(s2, 0u);
    EXPECT_EQ(s3, 3u);

    // -----------
    s1 = 0;
    s2 = 0;
    s3 = 0;
    {
        SingleThreadPool pool;
        MyRunnable mr(s2, 999);
        pool.addRunnable(&mr1);
        pool.addRunnable(&mr);      // throws
        pool.addRunnable(&mr3);
        pool.start();
    }

    EXPECT_EQ(s1, 1u);
    EXPECT_EQ(s2, 0u);
    EXPECT_EQ(s3, 3u);
}

static void threadpool_worker(SingleThreadPool& pool, Runnable* const r, bool const add, mytype& state)
{
    state = 1;
    if (add) {
        pool.addRunnable(r);
    } else {
        pool.removeRunnable(r);
    }
    state = 2;
}

TEST(Test_ThreadPool, TestRunning) {

    mytype s1{ 555 };   // So that runnable loops waiting for 0
    mytype s2{ 0 }, s3{ 0 };
    MyRunnable mr1(s1, 17);

    {
        SingleThreadPool pool;
        pool.addRunnable(&mr1);
        pool.start();

        // Make sure the SingleThreadPool thread is running
        // We can safely do this test since MyRunnable won't exit until we change s1
        while (!pool.isRunning()) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }

        // Start a task that tries to add
        std::thread t2(threadpool_worker, std::ref(pool), &mr1, true, std::ref(s2));

        // Start a task that tries to remove
        std::thread t3(threadpool_worker, std::ref(pool), &mr1, false, std::ref(s3));

        // Check that tasks are waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        EXPECT_EQ(s2, 1u);
        EXPECT_EQ(s3, 1u);

        // Let threadpool run and be finished
        s1 = 0;

        t2.join();
        t3.join();
    }

    EXPECT_EQ(s1, 17u);
    EXPECT_EQ(s2, 2u);
    EXPECT_EQ(s3, 2u);
}
