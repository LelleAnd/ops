/**
*
* Copyright (C) 2018-2026 Lennart Andersson.
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

#include "Serializable.h"
#include "SerializableFactory.h"
#include "SerializableCompositeFactory.h"
#include "SerializableInheritingTypeFactory.h"

using namespace ops;

// ===============================
// Helper classes

class Object_A : public Serializable
{
public:
	Object_A() = default;
	virtual ~Object_A() = default;
	Object_A(const Object_A& other) = delete;
	Object_A& operator= (const Object_A& other) = delete;
	Object_A(Object_A&& other) = delete;
	Object_A& operator =(Object_A&& other) = delete;
private:
    virtual void serialize(ArchiverInOut* archiver) override { UNUSED(archiver); }
};

class Object_B : public Serializable
{
public:
	Object_B() = default;
    virtual ~Object_B() = default;
	Object_B(const Object_B& other) = delete;
	Object_B& operator= (const Object_B& other) = delete;
	Object_B(Object_B&& other) = delete;
	Object_B& operator =(Object_B&& other) = delete;
private:
    virtual void serialize(ArchiverInOut* archiver) override { UNUSED(archiver); }
};

class Object_C : public Serializable
{
public:
	Object_C() = default;
    virtual ~Object_C() = default;
	Object_C(const Object_C& other) = delete;
	Object_C& operator= (const Object_C& other) = delete;
	Object_C(Object_C&& other) = delete;
	Object_C& operator =(Object_C&& other) = delete;
private:
    virtual void serialize(ArchiverInOut* archiver) override { UNUSED(archiver); }
};

class ObjectFactory_1 : public SerializableFactory
{
public:
	virtual Serializable* create(CreateType_T type) override
	{
		if (type == "Object_A") { return new Object_A(); }
		if (type == "Object_B") { return new Object_B(); }
		return nullptr;
	}

	ObjectFactory_1() = default; 
    virtual ~ObjectFactory_1() = default;
	ObjectFactory_1(const ObjectFactory_1& r) = delete;	
	ObjectFactory_1& operator= (const ObjectFactory_1& l) = delete;	
	ObjectFactory_1(ObjectFactory_1&&) = delete; 
	ObjectFactory_1& operator =(ObjectFactory_1&&) = delete;
};

class ObjectFactory_2 : public SerializableFactory
{
public:
	virtual Serializable* create(CreateType_T type) override
	{
		if (type == "Object_C") { return new Object_C(); }
		return nullptr;
	}

	ObjectFactory_2() = default;
    virtual ~ObjectFactory_2() = default;
	ObjectFactory_2(const ObjectFactory_2& r) = delete;	
	ObjectFactory_2& operator= (const ObjectFactory_2& l) = delete;	
	ObjectFactory_2(ObjectFactory_2&&) = delete; 
	ObjectFactory_2& operator =(ObjectFactory_2&&) = delete;
};

// ===============================

TEST(Test_ObjectFactories, TestComposite) {

	ObjectFactory_1 fact1;
	ObjectFactory_2 fact2;
	SerializableCompositeFactory fact;

	// Empty factory
	Serializable* ptr = fact.create("Object_A");
	EXPECT_EQ(ptr, nullptr);

	// Add a factory
	fact.add(&fact1);

	ptr = fact.create("Object_A");
	EXPECT_NE(dynamic_cast<Object_A*>(ptr), nullptr);
	delete ptr;

	ptr = fact.create("Object_B");
	EXPECT_NE(dynamic_cast<Object_B*>(ptr), nullptr);
	delete ptr;

	ptr = fact.create("Object_C");
	EXPECT_EQ(ptr, nullptr);

	// Add another factory
	fact.add(&fact2);

	ptr = fact.create("Object_A");
	EXPECT_NE(dynamic_cast<Object_A*>(ptr), nullptr);
	delete ptr;

	ptr = fact.create("Object_C");
	EXPECT_NE(dynamic_cast<Object_C*>(ptr), nullptr);
	delete ptr;

	ptr = fact.create("Kalle");
	EXPECT_EQ(ptr, nullptr);
	
	// Remove non-existing factory
	EXPECT_FALSE(fact.remove(nullptr));

	// Remove the first factory
	EXPECT_TRUE(fact.remove(&fact1));

	ptr = fact.create("Object_A");
	EXPECT_EQ(ptr, nullptr);

	ptr = fact.create("Object_C");
	EXPECT_NE(dynamic_cast<Object_C*>(ptr), nullptr);
	delete ptr;

	ptr = fact.create("Kalle");
	EXPECT_EQ(ptr, nullptr);

	// Note must remove all factories since they are put on the stack above
	// otherwise fact tries to delete them at exit!!
	EXPECT_TRUE(fact.remove(&fact2));
}

TEST(Test_ObjectFactories, TestInheriting) {
	ObjectFactory_1 fact1;
	SerializableInheritingTypeFactory fact;

	// Add factories
	fact.add(&fact1);

	Serializable* ptr = fact.create("Olle Kalle Pelle dummy");
	EXPECT_EQ(ptr, nullptr);

	// First in list
	ptr = fact.create("Object_A Olle Kalle Pelle Object_B Object_C");
	EXPECT_NE(dynamic_cast<Object_A*>(ptr), nullptr);
	delete ptr;

	// Not first or last
	ptr = fact.create("Olle Kalle Object_A Pelle Object_B Object_C");
	EXPECT_NE(dynamic_cast<Object_A*>(ptr), nullptr);
	delete ptr;

	// Last in list
	ptr = fact.create("Olle Kalle Pelle Object_C Object_A");
	EXPECT_NE(dynamic_cast<Object_A*>(ptr), nullptr);
	delete ptr;

	// Note must remove all factories since they are put on the stack above
	// otherwise fact tries to delete them at exit!!
	EXPECT_TRUE(fact.remove(&fact1));
}
