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

#include "Listener.h"
#include "Notifier.h"
#include "DataListener.h"
#include "DataNotifier.h"
#include "DeadlineMissedListener.h"

using namespace ops;

// ===============================
// Helper classes

class MyListener : public Listener<int>
{
public:
	int value;
	MyListener() : value(0) {}
	virtual void onNewEvent(Notifier<int>* sender, int arg) override { UNUSED(sender);  value = arg; }

	~MyListener() = default;
	MyListener(const MyListener& other) = delete;
	MyListener& operator= (const MyListener& other) = delete;
	MyListener(MyListener&& other) = delete;
	MyListener& operator =(MyListener&& other) = delete;
};

class MyNotifier : public Notifier<int>
{
public:
	void notify(int const value) { notifyNewEvent(value); }

	MyNotifier() = default;
	virtual ~MyNotifier() = default;
	MyNotifier(const MyNotifier& other) = delete;
	MyNotifier& operator= (const MyNotifier& other) = delete;
	MyNotifier(MyNotifier&& other) = delete;
	MyNotifier& operator =(MyNotifier&& other) = delete;
};

// ===============================

TEST(Test_NotifierListener, TestNotifierListener) {

	MyListener listener1;
	MyListener listener2;
	MyNotifier notifier;

	EXPECT_EQ(notifier.getNrOfListeners(), 0);
	EXPECT_EQ(listener1.value, 0);
	EXPECT_EQ(listener2.value, 0);

	notifier.notify(1);

	EXPECT_EQ(listener1.value, 0);
	EXPECT_EQ(listener2.value, 0);

	notifier.addListener(&listener1);

	EXPECT_EQ(notifier.getNrOfListeners(), 1);
	
	notifier.notify(10);

	EXPECT_EQ(listener1.value, 10);
	EXPECT_EQ(listener2.value, 0);

	notifier.addListener(&listener2);

	EXPECT_EQ(notifier.getNrOfListeners(), 2);

	notifier.notify(20);

	EXPECT_EQ(listener1.value, 20);
	EXPECT_EQ(listener2.value, 20);

	notifier.removeListener(&listener1);

	EXPECT_EQ(notifier.getNrOfListeners(), 1);

	notifier.notify(30);

	EXPECT_EQ(listener1.value, 20);
	EXPECT_EQ(listener2.value, 30);

	notifier.removeListener(&listener2);

	EXPECT_EQ(notifier.getNrOfListeners(), 0);

	notifier.notify(40);

	EXPECT_EQ(listener1.value, 20);
	EXPECT_EQ(listener2.value, 30);
}

// ===============================
// Helper classes

class MyDataListener : public DataListener
{
public:
	int counter;
	MyDataListener() : counter(0) {}
	virtual void onNewData(DataNotifier* sender) override { UNUSED(sender); counter++; }

	~MyDataListener() = default;
	MyDataListener(const MyDataListener& other) = delete;
	MyDataListener& operator= (const MyDataListener& other) = delete;
	MyDataListener(MyDataListener&& other) = delete;
	MyDataListener& operator =(MyDataListener&& other) = delete;
};

class MyDataNotifier : public DataNotifier
{
public:
	void notify() { notifyNewData(); }

	MyDataNotifier() = default;
	virtual ~MyDataNotifier() = default;
	MyDataNotifier(const MyDataNotifier& other) = delete;
	MyDataNotifier& operator= (const MyDataNotifier& other) = delete;
	MyDataNotifier(MyDataNotifier&& other) = delete;
	MyDataNotifier& operator =(MyDataNotifier&& other) = delete;
};

void MyCallback(DataNotifier* , void* const userData)
{
	MyDataListener* const listener = (MyDataListener*)userData;
	listener->counter++;
}

TEST(Test_NotifierListener, TestDataNotifierListener) {

	MyDataListener listener1;
	MyDataListener listener2;
	MyDataNotifier notifier;

	EXPECT_EQ(listener1.counter, 0);
	EXPECT_EQ(listener2.counter, 0);

	notifier.addDataListener(&listener1);
	notifier.notify();

	EXPECT_EQ(listener1.counter, 1);
	EXPECT_EQ(listener2.counter, 0);

	notifier.addDataListener(&listener2);
	notifier.notify();

	EXPECT_EQ(listener1.counter, 2);
	EXPECT_EQ(listener2.counter, 1);

#if defined(_MSC_VER)
#pragma warning(disable : 4996) 
#endif
	notifier.addDataListener(MyCallback, (void*)&listener2);
	notifier.notify();

	EXPECT_EQ(listener1.counter, 3);
	EXPECT_EQ(listener2.counter, 3);

	notifier.addDataListener(&listener1);
	notifier.notify();

	EXPECT_EQ(listener1.counter, 5);
	EXPECT_EQ(listener2.counter, 5);

	notifier.notify();

	EXPECT_EQ(listener1.counter, 7);
	EXPECT_EQ(listener2.counter, 7);

	// Test listener using a closure
	int counter = 0;
	notifier.addDataListener([&](ops::DataNotifier* ) { counter++; });

	notifier.notify();

	EXPECT_EQ(listener1.counter, 9);
	EXPECT_EQ(listener2.counter, 9);
	EXPECT_EQ(counter, 1);
}

// ===============================
// Helper classes

class MyDeadlineListener : public DeadlineMissedListener
{
public:
	int counter;
	MyDeadlineListener() : counter(0) {}
	virtual void onDeadlineMissed(DeadlineMissedEvent* e) override { UNUSED(e); counter++; }

	~MyDeadlineListener() = default;
	MyDeadlineListener(const MyDeadlineListener& other) = delete;
	MyDeadlineListener& operator= (const MyDeadlineListener& other) = delete;
	MyDeadlineListener(MyDeadlineListener&& other) = delete;
	MyDeadlineListener& operator =(MyDeadlineListener&& other) = delete;
};

TEST(Test_NotifierListener, TestDeadlineMissedListener) {

	MyDeadlineListener listener1;
	MyDeadlineListener listener2;
	DeadlineMissedEvent notifier;

	EXPECT_EQ(listener1.counter, 0);
	EXPECT_EQ(listener2.counter, 0);

	notifier.addDeadlineMissedListener(&listener1);
	notifier.notifyDeadlineMissed();

	EXPECT_EQ(listener1.counter, 1);
	EXPECT_EQ(listener2.counter, 0);

	notifier.addDeadlineMissedListener(&listener2);
	notifier.notifyDeadlineMissed();

	EXPECT_EQ(listener1.counter, 2);
	EXPECT_EQ(listener2.counter, 1);

	// Test listener using a closure
	int counter = 0;
	notifier.addDeadlineMissedListener([&](ops::DeadlineMissedEvent* ) { counter++; });

	notifier.notifyDeadlineMissed();

	EXPECT_EQ(listener1.counter, 3);
	EXPECT_EQ(listener2.counter, 2);
	EXPECT_EQ(counter, 1);
}
