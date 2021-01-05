// VerifySerDes.cpp
//
#ifdef _WIN32
  #include <Windows.h>
  #include <conio.h>
#else
  #include <stdlib.h>
#endif
#include <math.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <thread>

#include "TestAll/ChildData.h"
#include "TestAll/Definitions.h"
TestAll::Definitions::Command cmd = TestAll::Definitions::Command::PAUSE;
char arr[TestAll::Definitions::const_b];

//Include a publisher for the data type we want to publish, generated from our IDL project TestAll.
#include "TestAll/ChildDataPublisher.h"
#include "TestAll/ChildDataSubscriber.h"
//Include type support for the data types we have defined in our IDL project, generated from our IDL project TestAll.
#include "TestAll/TestAllTypeFactory.h"
#include <ops.h>
#include <MemoryMap.h>
#include <OPSConfigRepository.h>
#include "PrintArchiverOut.h"
#include "ChecksumArchiver.h"
#include "OPSArchiverOut.h"

#ifdef _WIN32
	#include <crtdbg.h>
#endif

#include "../../../ConfigFileHelper.h"

#ifdef NOT_USED_NOW
TestAll::BaseData::MemoryPool TestAll::BaseData::_pool(10);
TestAll::ChildData::MemoryPool TestAll::ChildData::_pool(10);
TestAll::Fruit::MemoryPool TestAll::Fruit::_pool(50);
TestAll::TestData::MemoryPool TestAll::TestData::_pool(100);
#endif

std::ostream& operator<<(std::ostream& os, TestAll::Definitions::Command const c)
{
	switch (c)
	{
	case TestAll::Definitions::Command::CONTINUE: os << "CONTINUE"; break;
	case TestAll::Definitions::Command::PAUSE: os << "PAUSE";       break;
	case TestAll::Definitions::Command::START: os << "START";       break;
	case TestAll::Definitions::Command::STOP: os << "STOP";         break;
	default: os.setstate(std::ios_base::failbit); break;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, TestAll::ChildData::Order const c)
{
	switch (c)
	{
	case TestAll::ChildData::Order::ABC: os << "ABC";    break;
	case TestAll::ChildData::Order::DEF: os << "DEF";    break;
	case TestAll::ChildData::Order::GHI: os << "GHI";    break;
	case TestAll::ChildData::Order::JKL: os << "JKL";    break;
	default: os.setstate(std::ios_base::failbit); break;
	}
	return os;
}

bool gTestFailed = false;

template <typename T>
T AssertEQ(T val, T exp, std::string mess = "")
{
	if (val != exp) {
		gTestFailed = true;
		std::cout << "Failed: " << mess << ", value= " << val << ", expected= " << exp << std::endl;
	}
	return val;
}

template <typename T>
T AssertNEQ(T val, T exp, std::string mess = "")
{
	if (val == exp) {
		gTestFailed = true;
		std::cout << "Failed: " << mess << ", value= " << val << std::endl;
	}
	return val;
}

void checkEmpty(const TestAll::TestData& data)
{
	AssertEQ<std::string>(data.text, "");
	AssertEQ<double>(data.value, 0.0);
}

void checkEmpty(const TestAll::ChildData& data)
{
	std::cout << "Checking empty object..." << std::endl;

	// BaseData
	AssertEQ<std::string>(data.baseText, "");
	AssertEQ<size_t>(data.stringOpenArr.size(), 0);
	for (int i = 0; i < 5; i++) { AssertEQ<std::string>(data.stringFixArr[i], ""); }
	AssertEQ<ops::strings::fixed_string<23>>(data.fixLengthString, "");
	AssertEQ<size_t>(data.fixLengthStringOpenArr.size(), 0);
	for (int i = 0; i < 10; i++) { AssertEQ<ops::strings::fixed_string<16>>(data.fixLengthStringFixArr[i], ""); }

	// ChildData
	//  enums
	AssertEQ<TestAll::ChildData::Order>(data.enu1, TestAll::ChildData::Order::ABC);
	AssertEQ<size_t>(data.enuVec.size(), 0);
	for (int i = 0; i < 6; i++) { AssertEQ<TestAll::ChildData::Order>(data.enuFixArr[i], TestAll::ChildData::Order::ABC); }
	AssertEQ< TestAll::Definitions::Command>(data.cmd, TestAll::Definitions::Command::START);
	for (int i = 0; i < 2; i++) { AssertEQ<TestAll::Definitions::Command>(data.cmds[i], TestAll::Definitions::Command::START); }

	//  core types
	AssertEQ<bool>(data.bo, false, "data.bo");
    AssertEQ<char>(data.b, 0, "data.b");
    AssertEQ<short>(data.sh, 0);
    AssertEQ<int>(data.i, 0);
    AssertEQ<int64_t>(data.l, 0);
    AssertEQ<float>(data.f, 0.0);
    AssertEQ<double>(data.d, 0.0);
	AssertEQ<std::string>(data.s, "");

	checkEmpty(data.test2);

	AssertNEQ<void*>(data.testPointer, nullptr);
	checkEmpty(*data.testPointer);

	AssertEQ<int>(data.fruit.value, TestAll::Fruit::APPLE);

	// vectors
	AssertEQ<size_t>(data.bos.size(), 0);
	for (int i = 0; i < 11; i++) { AssertEQ<bool>(data.fbos[i], false); }

	AssertEQ<size_t>(data.bs.size(), 0);
	for (int i = 0; i < 256; i++) { AssertEQ<char>(data.fbs[i], 0); }

	AssertEQ<size_t>(data.shs.size(), 0);
	for (int i = 0; i < 4; i++) { AssertEQ<short>(data.fshs[i], 0); }

	AssertEQ<size_t>(data.is_.size(), 0);
	for (int i = 0; i < 3; i++) { AssertEQ<int>(data.fis_[i], 0); }

	AssertEQ<size_t>(data.ls.size(), 0);
	for (int i = 0; i < 6; i++) { AssertEQ<int64_t>(data.fls[i], 0); }

	AssertEQ<size_t>(data.fs.size(), 0);
	for (int i = 0; i < 77; i++) { AssertEQ<float>(data.ffs[i], 0.0); }

	AssertEQ<size_t>(data.ds.size(), 0);
	for (int i = 0; i < 5; i++) { AssertEQ<double>(data.fds[i], 0.0); }

	AssertEQ<size_t>(data.ss.size(), 0);
	for (int i = 0; i < 10; i++) { AssertEQ<std::string>(data.fss[i], ""); }

	AssertEQ<size_t>(data.test2s.size(), 0);
	for (int i = 0; i < 5; i++) { checkEmpty(*data.ftest2s[i]); }

	AssertEQ<size_t>(data.secondVirtArray.size(), 0);
	for (int i = 0; i < 7; i++) { checkEmpty(*data.fsecondVirtArray[i]); }

	AssertEQ<size_t>(data.test2s2.size(), 0);
	for (int i = 0; i < 4; i++) { checkEmpty(data.ftest2s2[i]); }

	AssertEQ<size_t>(data.fruitarr.size(), 0);
	for (int i = 0; i < 15; i++) { AssertEQ<int>(data.ffruitarr[i].value, TestAll::Fruit::APPLE); }
}

void checkObjects(const TestAll::TestData& data, const TestAll::TestData& exp)
{
	// Check that objects are different
	AssertNEQ(&data, &exp, "Error same object");

	AssertEQ<std::string>(data.text, exp.text);
	AssertEQ<double>(data.value, exp.value);
}

void checkObjects(const TestAll::ChildData& data, const TestAll::ChildData& exp)
{
	std::cout << "Comparing objects object..." << std::endl;

	// Check that objects are different
	AssertNEQ(&data, &exp, "Error same object");

	// Test fields in BaseData
	AssertEQ<std::string>(data.baseText, exp.baseText, "baseText");

	AssertEQ<size_t>(data.stringOpenArr.size(), exp.stringOpenArr.size(), "stringOpenArr");
    AssertEQ<size_t>(data.stringOpenArr.size(), 2, "stringOpenArr");
    for (size_t i = 0; i < 2; i++) { AssertEQ<std::string>(data.stringOpenArr[i], exp.stringOpenArr[i], "stringOpenArr"); }

	for (int i = 0; i < 5; i++) { AssertEQ<std::string>(data.stringFixArr[i], exp.stringFixArr[i], "stringFixArr"); }

	AssertEQ<ops::strings::fixed_string<23>>(data.fixLengthString, exp.fixLengthString, "fixLengthString");

	AssertEQ<size_t>(data.fixLengthStringOpenArr.size(), exp.fixLengthStringOpenArr.size(), "fixLengthStringOpenArr");
    AssertEQ<size_t>(data.fixLengthStringOpenArr.size(), 2, "fixLengthStringOpenArr");
    for (size_t i = 0; i < 2; i++) { AssertEQ<ops::strings::fixed_string<16>>(data.fixLengthStringOpenArr[i], exp.fixLengthStringOpenArr[i], "fixLengthStringOpenArr"); }

	for (int i = 0; i < 10; i++) { AssertEQ<ops::strings::fixed_string<16>>(data.fixLengthStringFixArr[i], exp.fixLengthStringFixArr[i], "fixLengthStringFixArr"); }

	// Test fields in ChildData
	//  enums
	AssertEQ<TestAll::ChildData::Order>(data.enu1, exp.enu1);
	AssertEQ<size_t>(data.enuVec.size(), exp.enuVec.size());
    AssertEQ<size_t>(data.enuVec.size(), 5);
    for (unsigned int i = 0; i < 5; i++) { AssertEQ<TestAll::ChildData::Order>(data.enuVec[i], exp.enuVec[i]); }
	for (int i = 0; i < 6; i++) { AssertEQ<TestAll::ChildData::Order>(data.enuFixArr[i], exp.enuFixArr[i]); }
	AssertEQ< TestAll::Definitions::Command>(data.cmd, exp.cmd);
	for (int i = 0; i < 2; i++) { AssertEQ<TestAll::Definitions::Command>(data.cmds[i], exp.cmds[i]); }

	//  core types
	AssertEQ<bool>(data.bo, exp.bo, "data.bo");
    AssertEQ<char>(data.b, exp.b, "data.b");
    AssertEQ<short>(data.sh, exp.sh);
    AssertEQ<int>(data.i, exp.i);
    AssertEQ<int64_t>(data.l, exp.l);
    AssertEQ<float>(data.f, exp.f);
    AssertEQ<double>(data.d, exp.d);
	AssertEQ<std::string>(data.s, exp.s);

	checkObjects(data.test2, exp.test2);

	AssertNEQ<void*>(data.testPointer, nullptr, "data.testPointer");
	AssertNEQ<void*>(exp.testPointer, nullptr, "exp.testPointer");
	AssertNEQ<void*>(data.testPointer, exp.testPointer, "data.testPointer");
	AssertNEQ<void*>(dynamic_cast<TestAll::TestData*>(data.testPointer), nullptr, "data.testPointer");
	AssertNEQ<void*>(dynamic_cast<TestAll::TestData*>(exp.testPointer), nullptr, "data.testPointer");

	checkObjects(*data.testPointer, *exp.testPointer);

	AssertEQ<int>(data.fruit.value, exp.fruit.value);

	// vectors
	AssertEQ<size_t>(data.bos.size(), exp.bos.size());
    AssertEQ<size_t>(data.bos.size(), 3);
    for (unsigned int i = 0; i < 3; i++) { AssertEQ<bool>(data.bos[i], exp.bos[i]); }
	for (int i = 0; i < 11; i++) { AssertEQ<bool>(data.fbos[i], exp.fbos[i]); }

	AssertEQ<size_t>(data.bs.size(), exp.bs.size());
    AssertEQ<size_t>(data.bs.size(), 3);
    for (unsigned int i = 0; i < 3; i++) { AssertEQ<char>(data.bs[i], exp.bs[i]); }
	for (int i = 0; i < 256; i++) { AssertEQ<char>(data.fbs[i], exp.fbs[i]); }

	AssertEQ<size_t>(data.shs.size(), exp.shs.size());
    AssertEQ<size_t>(data.shs.size(), 2);
    for (unsigned int i = 0; i < 2; i++) { AssertEQ<short>(data.shs[i], exp.shs[i]); }
	for (int i = 0; i < 4; i++) { AssertEQ<short>(data.fshs[i], exp.fshs[i]); }

	AssertEQ<size_t>(data.is_.size(), exp.is_.size());
    AssertEQ<size_t>(data.is_.size(), 4);
    for (unsigned int i = 0; i < 4; i++) { AssertEQ<int>(data.is_[i], exp.is_[i]); }
	for (int i = 0; i < 3; i++) { AssertEQ<int>(data.fis_[i], exp.fis_[i]); }

	AssertEQ<size_t>(data.ls.size(), exp.ls.size());
    AssertEQ<size_t>(data.ls.size(), 4);
    for (unsigned int i = 0; i < 4; i++) { AssertEQ<int64_t>(data.ls[i], exp.ls[i]); }
    for (int i = 0; i < 6; i++) { AssertEQ<int64_t>(data.fls[i], exp.fls[i]); }

	AssertEQ<size_t>(data.fs.size(), exp.fs.size());
    AssertEQ<size_t>(data.fs.size(), 4);
    for (unsigned int i = 0; i < 4; i++) { AssertEQ<float>(data.fs[i], exp.fs[i]); }
    for (int i = 0; i < 77; i++) { AssertEQ<float>(data.ffs[i], exp.ffs[i]); }

	AssertEQ<size_t>(data.ds.size(), exp.ds.size());
    AssertEQ<size_t>(data.ds.size(), 2);
    for (unsigned int i = 0; i < 2; i++) { AssertEQ<double>(data.ds[i], exp.ds[i]); }
	for (int i = 0; i < 5; i++) { AssertEQ<double>(data.fds[i], exp.fds[i]); }

	AssertEQ<size_t>(data.ss.size(), exp.ss.size());
    AssertEQ<size_t>(data.ss.size(), 3);
    for (unsigned int i = 0; i < 3; i++) { AssertEQ<std::string>(data.ss[i], exp.ss[i]); }
	for (int i = 0; i < 10; i++) { AssertEQ<std::string>(data.fss[i], exp.fss[i]); }

	AssertEQ<size_t>(data.test2s.size(), exp.test2s.size());
    AssertEQ<size_t>(data.test2s.size(), 4);
    for (unsigned int i = 0; i < 4; i++) { checkObjects(*data.test2s[i], *exp.test2s[i]); }
	for (int i = 0; i < 5; i++) { checkObjects(*data.ftest2s[i], *exp.ftest2s[i]); }

	AssertEQ<size_t>(data.secondVirtArray.size(), exp.secondVirtArray.size());
    AssertEQ<size_t>(data.secondVirtArray.size(), 2);
    for (unsigned int i = 0; i < 2; i++) { checkObjects(*data.secondVirtArray[i], *exp.secondVirtArray[i]); }
	for (int i = 0; i < 7; i++) { checkObjects(*data.fsecondVirtArray[i], *exp.fsecondVirtArray[i]); }

	AssertEQ<size_t>(data.test2s2.size(), exp.test2s2.size());
    AssertEQ<size_t>(data.test2s2.size(), 11);
    for (unsigned int i = 0; i < 11; i++) { checkObjects(data.test2s2[i], exp.test2s2[i]); }
	for (int i = 0; i < 4; i++) { checkObjects(data.ftest2s2[i], exp.ftest2s2[i]); }

	AssertEQ<size_t>(data.fruitarr.size(), exp.fruitarr.size());
    AssertEQ<size_t>(data.fruitarr.size(), 2);
    for (unsigned int i = 0; i < 2; i++) { AssertEQ<int>(data.fruitarr[i].value, exp.fruitarr[i].value); }
	for (int i = 0; i < 15; i++) { AssertEQ<int>(data.ffruitarr[i].value, exp.ffruitarr[i].value); }
}

// Fill ChildData with fixed values used for tests between languages
void fillChildData(TestAll::ChildData& data)
{
	// Data for fields in BaseData
	data.baseText = "dynamic string";
	data.stringOpenArr.push_back("dyn str 1");
	data.stringOpenArr.push_back("dyn str 2");
	data.stringFixArr[0] = "dsf 0";
	data.stringFixArr[1] = "dsf 1";
	data.stringFixArr[2] = "dsf 2";
	data.stringFixArr[3] = "dsf 3";
	data.stringFixArr[4] = "dsf 4";
	data.fixLengthString = "fixed length string";
	data.fixLengthStringOpenArr.push_back("fix len str 1");
	data.fixLengthStringOpenArr.push_back("fix len str 2");
	data.fixLengthStringFixArr[0] = "fsf 0";
	data.fixLengthStringFixArr[1] = "fsf 1";
	data.fixLengthStringFixArr[2] = "fsf 2";
	data.fixLengthStringFixArr[3] = "fsf 3";
	data.fixLengthStringFixArr[4] = "fsf 4";
	data.fixLengthStringFixArr[5] = "fsf 5";
	data.fixLengthStringFixArr[6] = "fsf 6";
	data.fixLengthStringFixArr[7] = "fsf 7";
	data.fixLengthStringFixArr[8] = "fsf 8";
	data.fixLengthStringFixArr[9] = "fsf 9";

	// Data for fields in ChildData
	//  enums
	data.enu1 = TestAll::ChildData::Order::GHI;

	data.enuVec.push_back(TestAll::ChildData::Order::GHI);
	data.enuVec.push_back(TestAll::ChildData::Order::JKL);
	data.enuVec.push_back(TestAll::ChildData::Order::JKL);
	data.enuVec.push_back(TestAll::ChildData::Order::ABC);
	data.enuVec.push_back(TestAll::ChildData::Order::DEF);

	data.enuFixArr[0] = TestAll::ChildData::Order::DEF;
	data.enuFixArr[4] = TestAll::ChildData::Order::JKL;
	data.enuFixArr[5] = TestAll::ChildData::Order::DEF;

	data.cmd = TestAll::Definitions::Command::CONTINUE;
	data.cmds[0] = TestAll::Definitions::Command::PAUSE;
	data.cmds[1] = TestAll::Definitions::Command::STOP;

	//  core types
	data.bo = true;
    data.b = 7;
    data.sh = -99;
    data.i = 19;
    data.l = 3456789;
    data.f = 123.4567f;
    data.d = 987.12345678901;
    data.s = "Test of [de]serializing";

	data.test2.text = "TestData";
	data.test2.value = 555.5;

	data.testPointer->text = "TestPtr";
	data.testPointer->value = 777.7;

	data.fruit.value = TestAll::Fruit::PEAR;

	data.bos.push_back(false);
	data.bos.push_back(true);
	data.bos.push_back(false);

	// vectors
	memset(&data.fbos, 0, sizeof(data.fbos));
	data.fbos[5] = true;
	data.fbos[10] = true;

	data.bs.push_back(10);
	data.bs.push_back(20);
	data.bs.push_back(30);

	for (int i = 0; i < 256; i++) { data.fbs[i] = (char)i; }

	data.shs.push_back(1111);
	data.shs.push_back(2222);

	data.fshs[0] =  21;
	data.fshs[1] = 121;
	data.fshs[2] = 221;
	data.fshs[3] = 321;

	data.is_.push_back(100000);
	data.is_.push_back(101010);
	data.is_.push_back(110101);
	data.is_.push_back(111111);

	data.fis_[0] = -1;
	data.fis_[1] = -2;
	data.fis_[2] = -3;

	data.ls.push_back(9);
	data.ls.push_back(8);
	data.ls.push_back(7);
	data.ls.push_back(6);

	data.fls[0] = 9999;
	data.fls[1] = 9998;
	data.fls[2] = 9997;
	data.fls[3] = 9996;
	data.fls[4] = 9995;
	data.fls[5] = 9994;

	data.fs.push_back(3.1f);
	data.fs.push_back(31.14f);
	data.fs.push_back(4.56f);
	data.fs.push_back(987.0f);

	for (int i = 0; i < 77; i++) { data.ffs[i] = 0.0; }
	data.ffs[21] = 3.1415f;

	data.ds.push_back(1.987654321);
	data.ds.push_back(2.3456789);

	data.fds[0] = 1.1;
	data.fds[1] = 2.1;
	data.fds[2] = 3.1;
	data.fds[3] = 4.1;
	data.fds[4] = 5.1;

	data.ss.push_back("Index 0");
	data.ss.push_back("Index 1");
	data.ss.push_back("Index 2");

	data.fss[4] = "4 string";
	data.fss[7] = "7 string";
	data.fss[9] = "9 string";

	data.test2s.push_back(new TestAll::TestData());
	data.test2s.push_back(new TestAll::TestData());
	data.test2s.push_back(new TestAll::TestData());
	data.test2s.push_back(new TestAll::TestData());
	data.ftest2s[2]->text = "Index 2";
	data.ftest2s[2]->value = 7.7;

	data.secondVirtArray.push_back(new TestAll::TestData());
	data.secondVirtArray.push_back(new TestAll::TestData());
	data.fsecondVirtArray[5]->text = "Index 5";
	data.fsecondVirtArray[5]->value = -9.99;

	data.test2s2.reserve(11);
	data.test2s2.resize(11);
	data.ftest2s2[3].text = "";
	data.ftest2s2[1].value = 710.6;

	data.fruitarr.reserve(2);
	data.fruitarr.resize(2);
	data.fruitarr[0].value = TestAll::Fruit::PEAR;
	data.fruitarr[1].value = TestAll::Fruit::BANANA;

	data.ffruitarr[0].value = TestAll::Fruit::PEAR;
	data.ffruitarr[14].value = TestAll::Fruit::PEAR;
}

volatile bool gTerminate = false;

void SignalHandler(const int signal)
{
	if (signal == SIGINT) { gTerminate = true; }
}

TestAll::ChildData f(TestAll::ChildData o)
{
	return o;
}

int main(const int argc, const char* args[])
{
#if defined(DEBUG_OPSOBJECT_COUNTER)
	std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif
	try {
		signal(SIGINT, SignalHandler);

		std::cout << "OPS Library Compile Signature: " << ops::Participant::LibraryCompileSignature() << std::endl;
		std::cout << "OPS Header Compile Signature : " << ops::Participant::HeaderCompileSignature() << std::endl;

		if (!ops::Participant::CheckCompileSignature()) {
			std::cout << "ERROR: Headers and compiled library are not consistent !!" << std::endl;
			gTestFailed = true;
		}

#ifdef _WIN32
		_CrtMemState s1, s2, sd;
		_CrtMemCheckpoint(&s1);

		ops::OPSObject od0;

		_CrtMemCheckpoint(&s2);
		_CrtMemDifference(&sd, &s1, &s2);
#endif

		// ===========================================

		UNUSED(argc);
		UNUSED(args);
		TestAll::ChildData cd1;
		TestAll::ChildData cd2, cd2a;
		TestAll::ChildData cd3;

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		// ==============================================================
		std::cout << "Test initial state..." << std::endl;
		checkEmpty(cd1);
		checkEmpty(cd2);
		checkEmpty(cd3);

		std::cout << "Finished " << std::endl;

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		fillChildData(cd1);

		std::cout << "Test copy constructor..." << std::endl;
		const TestAll::ChildData cd2c(cd1);
		checkObjects(cd1, cd2c);
		std::cout << "Finished " << std::endl;

		std::cout << "Test copy assignment..." << std::endl;
		cd2a = cd1;
		checkObjects(cd1, cd2a);
		std::cout << "Finished " << std::endl;

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		{
			std::cout << "Test move constructor..." << std::endl;
			TestAll::ChildData obj4 = f(cd1);	// Makes a copy of cd1 which is given to f(), which is moved from
			checkObjects(cd1, obj4);
			std::cout << "Finished " << std::endl;

			std::cout << "Test move assignment..." << std::endl;
			TestAll::ChildData obj5;
			obj5 = std::move(obj4);
			checkObjects(cd1, obj5);
			std::cout << "Finished " << std::endl;
		}

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		std::cout << "Test cloning..." << std::endl;
		cd1.fillClone(&cd2);
		checkObjects(cd1, cd2);

		// Test that all is cleared in cd2 by cloning an empty object
		cd3.fillClone(&cd2);
        checkEmpty(cd2);
        
		// Fill cd2 object again, it is needed below
		cd1.fillClone(&cd2);
		checkObjects(cd1, cd2);

		// Delete first object, recreate it and compare again
		// This to check that cd2 is really a deep clone (no common object)
		///TODO


		std::cout << "Finished " << std::endl;

		std::cout << "Test Print Archiver" << std::endl;
		{
			ops::PrintArchiverOut prt(std::cout);
			prt.printObject("data", &cd1);
		}
		std::cout << "Print Archiver Test Finished " << std::endl;

        std::cout << "Test Checksum Archiver" << std::endl;
        {
            ops::ChecksumArchiver<ops::example::calculator_xor_8> chk;
            chk.calc.sum = 0;
            cd1.serialize(&chk);
            AssertEQ((int)chk.calc.sum, 140, "Checksum error");

            std::cout << "Checksum Archiver # fields = " << chk.calc.totalfields << std::endl;
            std::cout << "Checksum Archiver # bytes = " << chk.calc.totalbytes << std::endl;
            std::cout << "Checksum Archiver 8-bit XOR = " << (int)chk.calc.sum << std::endl;
        }

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		// ==============================================================
		std::cout << "Serialize filled object" << std::endl;
		{
			ops::MemoryMap map(1, 65536);
			ops::ByteBuffer buf(map);
			ops::OPSArchiverOut ao(buf, false);

			std::cout << "  GetSize()= " << buf.GetSize() << std::endl;
			ao.inout("data", cd1);
			std::cout << "  optNonVirt = false, GetSize()= " << buf.GetSize() << std::endl;
			AssertEQ(buf.GetSize(), 3150, "Serialized size error");
		}
		{
			ops::MemoryMap map(1, 65536);
			ops::ByteBuffer buf(map);
			ops::OPSArchiverOut ao(buf, true);

			std::cout << "  GetSize()= " << buf.GetSize() << std::endl;
			ao.inout("data", cd1);
			std::cout << "  optNonVirt = true,  GetSize()= " << buf.GetSize() << std::endl;
			AssertEQ(buf.GetSize(), 2591, "Serialized size error");
		}
		std::cout << "Serialize finished" << std::endl;

#ifdef kjshfdjkdshfkhsdkfj
		std::ofstream myfile("dump-opt.bin", std::ios::out | std::ios::app | std::ios::binary);
		if (myfile.is_open()) {
			myfile.write(buf.getSegment(0), buf.getSegmentSize(0));
			myfile.close();
		} else {
			std::cout << "Unable to open file\n";
		}
#endif

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		// ==============================================================
		std::cout << "Test publish/subscribe" << std::endl;

        setup_alt_config("Examples/OPSIdls/TestAll/ops_config.xml");
        
        ops::Participant::getStaticErrorService()->addListener(new ops::ErrorWriter(std::cout));

		ops::Participant* const participant = ops::Participant::getInstance("TestAllDomain");
		if (!participant) {
			std::cout << "Create participant failed. do you have ops_config.xml on your rundirectory?" << std::endl;
			throw std::exception();
		}
		participant->addTypeSupport(new TestAll::TestAllTypeFactory());

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		std::cout << "Dump of configuration" << std::endl;
		{
			ops::PrintArchiverOut prt(std::cout);
			prt.printObject("ops_config", participant->getConfig());
		}
		std::cout << "Dump of configuration Finished " << std::endl;

		// Add an errorwriter instance to the participant to catch ev. internal OPS errors
		// We can easily write our own if we want to log data in another way.
		ops::ErrorWriter* const errorWriter = new ops::ErrorWriter(std::cout);
		participant->addListener(errorWriter);

		{
			// Setup & start subscriber w polling
			ops::Topic const topic = participant->createTopic("ChildTopic");
			TestAll::ChildDataSubscriber sub(topic);
			sub.start();

			// Setup & start publisher
			TestAll::ChildDataPublisher pub(topic);
			pub.setName("C++");
			pub.start();

#if defined(DEBUG_OPSOBJECT_COUNTER)
			std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif
			// Publish data
			pub.write(cd1);

			// Check that sent data isn't affected by publish
			checkObjects(cd1, cd2);


			// Check received values against sent values
			AssertEQ<bool>(sub.waitForNewData(100), true, "No data received");
			bool const flag = AssertEQ<bool>(sub.getData(cd3), true, "No received data");

			sub.aquireMessageLock();
			if (sub.getMessage() != nullptr) {
				AssertEQ<size_t>(sub.getMessage()->spareBytes.size(), 0, "spareBytes");
			} else {
				std::cout << "Failed: sub.getMessage() == NULL" << std::endl;
				gTestFailed = true;
			}
			sub.releaseMessageLock();

			if (flag) { checkObjects(cd1, cd3); }

			std::cout << "Finished " << std::endl;

#if defined(DEBUG_OPSOBJECT_COUNTER)
			std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

			std::cout << "Waiting for more data during 60 seconds... (Press Ctrl-C to terminate)" << std::endl;
			int64_t PubTime = ops::TimeHelper::currentTimeMillis() + 5000;
			const int64_t Limit = ops::TimeHelper::currentTimeMillis() + 60000;
			while ((!gTerminate) && (Limit >= ops::TimeHelper::currentTimeMillis())) {
				if (sub.waitForNewData(100)) {
					sub.aquireMessageLock();
					std::cout << "Received new data from " << sub.getMessage()->getPublisherName() << ". Checking..." << std::endl;
					sub.releaseMessageLock();
					sub.getData(cd3);
					sub.aquireMessageLock();
					AssertEQ<size_t>(sub.getMessage()->spareBytes.size(), 0, "spareBytes");
					sub.releaseMessageLock();
					checkObjects(cd3, cd1);
					std::cout << "Data check done" << std::endl;
				}
				if (PubTime < ops::TimeHelper::currentTimeMillis()) {
					PubTime = ops::TimeHelper::currentTimeMillis() + 5000;
					pub.write(cd1);
				}
			}
		}

#if defined(DEBUG_OPSOBJECT_COUNTER)
		std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

		delete participant;
		ops::OPSConfigRepository::Instance()->TotalClear();

	}
	catch (std::exception& e) {
		std::cout << "Caugth std:exception: " << e.what() << std::endl;
		gTestFailed = true;
	}
#if defined(DEBUG_OPSOBJECT_COUNTER)
	std::cout << "ops::OPSObject::NumOpsObjects(): " << ops::OPSObject::NumOpsObjects() << std::endl;
#endif

	if (gTestFailed) {
		std::cout << std::endl << "T e s t   F a i l e d" << std::endl;
	} else {
		std::cout << std::endl << "T e s t   O k" << std::endl;
	}

	std::cout << std::endl << "Sleeping for 5 seconds..." << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	return 0;
}
