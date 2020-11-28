/**
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

#include "Reservable.h"
#include "ReserveInfo.h"
#include "ReferenceHandler.h"
#include "Participant.h"

using namespace ops;

// ===============================
// Helper classes

static int MyObject_Ctr = 0;

class MyObject : public Reservable
{
public:
	MyObject()
	{
		MyObject_Ctr++;
	}
	MyObject(const MyObject& r) : Reservable(r)
	{
		MyObject_Ctr++;
	}
	MyObject& operator= (const MyObject& l)
	{
		Reservable::operator=(l);
		if (&l != this) {
			// No data to copy
		}
		return *this;
	}
	MyObject(MyObject&&) = delete;
	MyObject& operator =(MyObject&&) = delete;
	virtual ~MyObject()
	{
		MyObject_Ctr--;
	}
};

// ===============================

TEST(Test_Reservable, TestRefCounter) {

	EXPECT_EQ(MyObject_Ctr, 0);

	MyObject obj;

	EXPECT_TRUE(ops::Participant::CheckCompileSignature());

	// We can't test ref counter without a ref handler if assert() is enabled
#ifdef OPS_REMOVE_ASSERT
	EXPECT_EQ(MyObject_Ctr, 1);
	EXPECT_EQ(obj.getNrOfReservations(), 0);
	obj.reserve();
	EXPECT_EQ(obj.getNrOfReservations(), 1);
	obj.reserve();
	obj.reserve();
	EXPECT_EQ(obj.getNrOfReservations(), 3);
	obj.unreserve();
	EXPECT_EQ(obj.getNrOfReservations(), 2);
	obj.unreserve();
	EXPECT_EQ(obj.getNrOfReservations(), 1);
	// Note object will not be deleted when we reach 0, since we haven't provided a refHandler
	obj.unreserve();
	EXPECT_EQ(obj.getNrOfReservations(), 0);
#endif
}

TEST(Test_Reservable, TestRefHandler) {

	EXPECT_TRUE(ops::Participant::CheckCompileSignature());

	EXPECT_EQ(MyObject_Ctr, 0);

	ReferenceHandler ref;
	EXPECT_EQ(ref.size(), 0);

	MyObject* obj = new MyObject();
	EXPECT_EQ(MyObject_Ctr, 1);
	EXPECT_EQ(obj->getReferenceHandler(), nullptr);

	ref.addReservable(obj);
	EXPECT_EQ(obj->getReferenceHandler(), &ref);
	EXPECT_EQ(ref.size(), 1);

	obj->reserve();
	obj->reserve();
	EXPECT_EQ(obj->getNrOfReservations(), 2);

	obj->unreserve();
	EXPECT_EQ(obj->getNrOfReservations(), 1);
	EXPECT_EQ(ref.size(), 1);

	// If we now do unreserve(), obj should be deleted
	obj->unreserve();
	obj = nullptr;

	EXPECT_EQ(ref.size(), 0);
	EXPECT_EQ(MyObject_Ctr, 0);

	// Now add several objects
	MyObject* obj1 = new MyObject();
	MyObject* obj2 = new MyObject();
	MyObject* obj3 = new MyObject();
	EXPECT_EQ(MyObject_Ctr, 3);

	ref.addReservable(obj1);
	ref.addReservable(obj2);
	ref.addReservable(obj3);
	EXPECT_EQ(ref.size(), 3);

	obj1->reserve();
	obj2->reserve();
	obj3->reserve();

	obj2->unreserve(); obj2 = nullptr;
	EXPECT_EQ(ref.size(), 2);
	EXPECT_EQ(MyObject_Ctr, 2);
	obj1->unreserve(); obj1 = nullptr;
	EXPECT_EQ(ref.size(), 1);
	EXPECT_EQ(MyObject_Ctr, 1);
	obj3->unreserve(); obj3 = nullptr;
	EXPECT_EQ(ref.size(), 0);
	EXPECT_EQ(MyObject_Ctr, 0);
}

TEST(Test_Reservable, TestCopy) {

	EXPECT_TRUE(ops::Participant::CheckCompileSignature());

	{
		EXPECT_EQ(MyObject_Ctr, 0);

		MyObject obj;
		// We can't test ref counter without a ref handler if assert() is enabled
#ifdef OPS_REMOVE_ASSERT
		obj.reserve();
		obj.reserve();
#endif

		EXPECT_EQ(MyObject_Ctr, 1);
#ifdef OPS_REMOVE_ASSERT
		EXPECT_EQ(obj.getNrOfReservations(), 2);
#endif

		MyObject obj2(obj);
		EXPECT_EQ(MyObject_Ctr, 2);
#ifdef OPS_REMOVE_ASSERT
		EXPECT_EQ(obj.getNrOfReservations(), 2);
		EXPECT_EQ(obj2.getNrOfReservations(), 0);
#endif
		EXPECT_EQ(obj2.getReferenceHandler(), nullptr);

		MyObject obj3;
		obj3 = obj;
		EXPECT_EQ(MyObject_Ctr, 3);
#ifdef OPS_REMOVE_ASSERT
		EXPECT_EQ(obj.getNrOfReservations(), 2);
		EXPECT_EQ(obj3.getNrOfReservations(), 0);
#endif
		EXPECT_EQ(obj3.getReferenceHandler(), nullptr);

#ifdef OPS_REMOVE_ASSERT
		obj.unreserve();
		obj.unreserve();
		// Note object will not be deleted when we reach 0, since we haven't provided a refHandler
		EXPECT_EQ(obj.getNrOfReservations(), 0);
#endif
	}
	EXPECT_EQ(MyObject_Ctr, 0);
	{
		ReferenceHandler ref;
		EXPECT_EQ(ref.size(), 0);

		MyObject* obj = new MyObject();
		ref.addReservable(obj);
		obj->reserve();
		EXPECT_EQ(MyObject_Ctr, 1);
		EXPECT_EQ(obj->getNrOfReservations(), 1);
		EXPECT_EQ(obj->getReferenceHandler(), &ref);

		// Copy constructor
		MyObject* const obj2 = new MyObject(*obj);
		EXPECT_EQ(obj2->getNrOfReservations(), 0);
		EXPECT_EQ(obj2->getReferenceHandler(), nullptr);
		ref.addReservable(obj2);
		obj2->reserve();
		EXPECT_EQ(MyObject_Ctr, 2);
		EXPECT_EQ(obj2->getNrOfReservations(), 1);
		EXPECT_EQ(obj2->getReferenceHandler(), &ref);

		// Assignment constructor
		MyObject mo;
		mo = *obj2;
		EXPECT_EQ(MyObject_Ctr, 3);
		EXPECT_EQ(mo.getNrOfReservations(), 0);
		EXPECT_EQ(mo.getReferenceHandler(), nullptr);
		EXPECT_EQ(obj2->getNrOfReservations(), 1);
		EXPECT_EQ(obj2->getReferenceHandler(), &ref);

		// Two pointers to same object
		MyObject* const obj3 = obj2;
		obj3->reserve();
		EXPECT_EQ(obj3->getNrOfReservations(), 2);
		EXPECT_EQ(obj2->getNrOfReservations(), 2);

		obj->unreserve();
		EXPECT_EQ(MyObject_Ctr, 2);

		obj2->unreserve();
		EXPECT_EQ(MyObject_Ctr, 2);

		obj3->unreserve();
		EXPECT_EQ(MyObject_Ctr, 1);
	}
	EXPECT_EQ(MyObject_Ctr, 0);
}
