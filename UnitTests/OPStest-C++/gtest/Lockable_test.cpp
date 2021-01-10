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
#include <thread>
#include <chrono>

#include "Lockable.h"

using namespace ops;

// ===============================
// Helpers

using mytype = std::atomic<uint32_t>;

static void wait_for(mytype& order, const uint32_t value)
{
    while (order < value) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
}

static void lockable_worker(Lockable& lock, mytype& order, mytype& state)
{
    state = 1;

    wait_for(order, 1);
    state = 2;
    {
        const SafeLock lck1(lock);
        state = 3;
        {
            const SafeLock lck2(lock);
            state = 4;

            wait_for(order, 2);
        }
    }
    state = 5;

    wait_for(order, 3);
    state = 6;
}

// ===============================

TEST(Test_Lockable, Test) {

    // Default constructor
    Lockable l;

    EXPECT_TRUE(l.lock());
    l.unlock();

    EXPECT_TRUE(l.trylock());
    l.unlock();

    EXPECT_TRUE(l.lock());
    EXPECT_TRUE(l.lock());
    EXPECT_TRUE(l.trylock());
    EXPECT_TRUE(l.trylock());
    l.unlock();
    l.unlock();
    l.unlock();
    l.unlock();

    // Copy assignment, Self assignment
    l = l;

    {
        // Copy constructor
        Lockable l2(l);
        l2.lock();
        l2.unlock();

        // Copy assignment
        l2 = l;
    }

    mytype o1{ 0 }, o2{ 0 };
    mytype s1{ 0 }, s2{ 0 };
    std::thread t1(lockable_worker, std::ref(l), std::ref(o1), std::ref(s1));
    std::thread t2(lockable_worker, std::ref(l), std::ref(o2), std::ref(s2));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 1u);
    EXPECT_EQ(s2, 1u);

    o1 = 1;
    wait_for(s1, 4);

    o2 = 1;
    wait_for(s1, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 4u);
    EXPECT_EQ(s2, 2u);

    o1 = 99;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 6u);
    EXPECT_EQ(s2, 4u);

    o2 = 99;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 6u);
    EXPECT_EQ(s2, 6u);

    t1.join();
    t2.join();
}

// ===============================

static void lockable_try_worker(Lockable& lock, mytype& order, mytype& state)
{
    state = 1;

    wait_for(order, 1);
    state = 2;
    while (true) {
        SafeTryLock lck1(lock);
        if (lck1.isLocked()) {
            state = 3;
            while (true) {
                SafeTryLock lck2(lock);
                if (lck2.isLocked()) {
                    state = 4;

                    wait_for(order, 4);
                    break;

                } else {
                    state = 40;
                }

                wait_for(order, 3);
            }
            break;

        } else {
            state = 30;
        }

        wait_for(order, 2);
    }
    state = 5;

    wait_for(order, 5);
    state = 6;
}

TEST(Test_Lockable, Test_Try) {

    // Default constructor
    Lockable l;

    mytype o1{ 0 }, o2{ 0 };
    mytype s1{ 0 }, s2{ 0 };
    std::thread t1(lockable_try_worker, std::ref(l), std::ref(o1), std::ref(s1));
    std::thread t2(lockable_try_worker, std::ref(l), std::ref(o2), std::ref(s2));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 1u);
    EXPECT_EQ(s2, 1u);

    o1 = 1;
    wait_for(s1, 4);

    o2 = 1;
    wait_for(s2, 30);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 4u);
    EXPECT_EQ(s2, 30u);

    o1 = 99;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 6u);
    EXPECT_EQ(s2, 30u);

    o2 = 2;
    wait_for(s1, 4);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 6u);
    EXPECT_EQ(s2, 4u);

    o2 = 99;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(s1, 6u);
    EXPECT_EQ(s2, 6u);

    t1.join();
    t2.join();
}
