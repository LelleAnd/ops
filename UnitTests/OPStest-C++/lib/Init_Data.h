#include <string>
#include <typeinfo>
#include <iostream>

#include <ops.h>
#include "OPSArchiverOut.h"
#include "OPSArchiverIn.h"
#include "OPSMessage.h"

#include "Cheese.h"
#include "PizzaData.h"
#include "VessuvioData.h"
#include "CapricosaData.h"
#include "LHCData.h"
#include "ExtraAllt.h"
#include "PizzaProjectTypeFactory.h"

#ifdef _MSC_VER
#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min
#endif

#include <limits>

using namespace std;

namespace init {

const int int_max 			= std::numeric_limits<int>::max();
const int int_min 			= std::numeric_limits<int>::min();
const int short_max			= std::numeric_limits<short>::max();
const int short_min			= std::numeric_limits<short>::min();
const float float_max 		= std::numeric_limits<float>::max();
const float float_min	 	= std::numeric_limits<float>::min();
const double double_max 	= std::numeric_limits<double>::max();
const double double_min 	= std::numeric_limits<double>::min();
const int64_t int64_max 	= std::numeric_limits<int64_t>::max();
const int64_t int64_min 	= std::numeric_limits<int64_t>::min();

void ClearExtraAllt(pizza::special::ExtraAllt &extraAllt) {

	extraAllt.extraCheese 		= false;
	extraAllt.nrOfMushRooms		= ' ';
	extraAllt.meetQuality		= 0;
	extraAllt.timeBakedHours	= 0.0f;
	extraAllt.timeBakedSeconds	= 0.0;
	extraAllt.description		= "";
	extraAllt.cheese_->age		= 0;
	extraAllt.cheese_->name		= "";
	extraAllt.testingShort		= 0;


	(extraAllt.bools).clear();
	(extraAllt.bytes).clear();
	(extraAllt.shorts).clear();
	(extraAllt.ints).clear();
	(extraAllt.floats).clear();
	(extraAllt.doubles).clear();
	(extraAllt.longs).clear();
	(extraAllt.strings).clear();
	(extraAllt.cheeses).clear();
}

void initExtraAlltNormal(pizza::special::ExtraAllt &extraAllt) {

	//reseting if there are data
	ClearExtraAllt(extraAllt);

	extraAllt.extraCheese 		= true;
	extraAllt.nrOfMushRooms		= 'P';
	extraAllt.meetQuality		= 9;
	extraAllt.timeBakedHours	= (float)123.4;
	extraAllt.timeBakedSeconds	= 53.4;
	extraAllt.description		= "Pizza with extra allt";
	extraAllt.cheese_->age		= 5;
	extraAllt.cheese_->name		= "chilli cheese";
	extraAllt.testingShort		= 256;

	(extraAllt.bools).push_back(true);
	(extraAllt.bools).push_back(false);
	(extraAllt.bools).push_back(true);
	(extraAllt.bools).push_back(false);
	(extraAllt.bools).push_back(true);
	(extraAllt.bools).push_back(false);

	(extraAllt.bytes).push_back('p');
	(extraAllt.bytes).push_back('i');
	(extraAllt.bytes).push_back('z');
	(extraAllt.bytes).push_back('Z');
	(extraAllt.bytes).push_back('a');
	(extraAllt.bytes).push_back('!');

	(extraAllt.ints).push_back(0);
	(extraAllt.ints).push_back(123);
	(extraAllt.ints).push_back(-523);
	(extraAllt.ints).push_back(62860);
	(extraAllt.ints).push_back(int_min);
	(extraAllt.ints).push_back(int_max);

	(extraAllt.floats).push_back(0.0f);
	(extraAllt.floats).push_back(123.4f);
	(extraAllt.floats).push_back(-523.2f);
	(extraAllt.floats).push_back(62860.0f);
	(extraAllt.floats).push_back(float_min);
	(extraAllt.floats).push_back(float_max);

	(extraAllt.doubles).push_back(0.0);
	(extraAllt.doubles).push_back(123.4);
	(extraAllt.doubles).push_back(-523.2);
	(extraAllt.doubles).push_back(62860.0);
	(extraAllt.doubles).push_back(double_min);
	(extraAllt.doubles).push_back(double_max);

	(extraAllt.longs).push_back(0);
	(extraAllt.longs).push_back(123);
	(extraAllt.longs).push_back(-523);
	(extraAllt.longs).push_back(3951379600);
	(extraAllt.longs).push_back(int64_min);
	(extraAllt.longs).push_back(int64_max);

	(extraAllt.strings).push_back(string("ExtraAllt"));
	(extraAllt.strings).push_back(string("ar"));
	(extraAllt.strings).push_back(string("den"));
	(extraAllt.strings).push_back(string("basta"));
	(extraAllt.strings).push_back(string("pizzan"));
	(extraAllt.strings).push_back(string("!!!!!!!!!!!!!!!!!!"));

	(extraAllt.shorts).push_back(0);
	(extraAllt.shorts).push_back(123);
	(extraAllt.shorts).push_back(-523);
	(extraAllt.shorts).push_back(30000);
	(extraAllt.shorts).push_back(short_min);
	(extraAllt.shorts).push_back(short_max);

	pizza::special::Cheese testCheese;

	testCheese.age = 1;
	testCheese.name = "cheddar";
	(extraAllt.cheeses).push_back(testCheese);

	testCheese.age = 2;
	testCheese.name = "hardost";
	(extraAllt.cheeses).push_back(testCheese);

	testCheese.age = 3;
	testCheese.name = "mogelost";
	(extraAllt.cheeses).push_back(testCheese);

	testCheese.age = 4;
	testCheese.name = "Farskost";
	(extraAllt.cheeses).push_back(testCheese);

	testCheese.age = 5;
	testCheese.name = "!#%&/()=?";
	(extraAllt.cheeses).push_back(testCheese);

	testCheese.age = 6;
	testCheese.name = "Mesvara";
	(extraAllt.cheeses).push_back(testCheese);
}

void initExtraAlltLarge(pizza::special::ExtraAllt &extraAllt){
	ClearExtraAllt(extraAllt);

	int vecSize = 1000;
	int halfVecsize = vecSize / 2;

	bool boolVal = true;
	char byteVal = 'a';
	int intVal = 5;
	float floatVal = 15.0f;
	double doubleVal = 25.0;
	long longVal = 35;
	short shortVal = 45;
	string stringVal = "hejsan";
	pizza::special::Cheese cheeseVal;
	cheeseVal.age = 3;
	cheeseVal.name = "ecklig ost";

	for(int i = 0; i < vecSize; ++i) {
		(extraAllt.bools).push_back(boolVal);
		(extraAllt.bytes).push_back(byteVal);
		(extraAllt.ints).push_back(intVal);
		(extraAllt.floats).push_back(floatVal);
		(extraAllt.doubles).push_back(doubleVal);
		(extraAllt.longs).push_back(longVal);
		(extraAllt.strings).push_back(stringVal);
		(extraAllt.cheeses).push_back(cheeseVal);
		(extraAllt.shorts).push_back(shortVal);
		if(i == halfVecsize){
			boolVal 		= false;
			byteVal 		= 'b';
			intVal 			= 10;
			floatVal 		= 20.0f;
			doubleVal 		= 30.0;
			longVal 		= 40;
			stringVal 		= "hoppsan";
			cheeseVal.age 	= 6;
			cheeseVal.name 	= "god ost";
			shortVal		= 50;
		}
	}
}

void initExtraAlltNormalTCP(pizza::special::ExtraAllt &extraAllt) {
	initExtraAlltNormal(extraAllt);
	extraAllt.strings[0] = "TCP_NORMAL";
	extraAllt.strings[1] = "TCP_NORMAL";
	extraAllt.strings[2] = "TCP_NORMAL";
	extraAllt.strings[3] = "TCP_NORMAL";
	extraAllt.strings[4] = "TCP_NORMAL";
	extraAllt.strings[5] = "TCP_NORMAL";
}

void initExtraAlltLargeTCP(pizza::special::ExtraAllt &extraAllt) {
	initExtraAlltLarge(extraAllt);
	for(int i = 0; i < 1000; ++i) {
		extraAllt.strings[i] = "TCP_LARGE";
	}
}

void initExtraAlltNormalUDP(pizza::special::ExtraAllt &extraAllt) {
	initExtraAlltNormal(extraAllt);
	extraAllt.strings[0] = "UDP_NORMAL";
	extraAllt.strings[1] = "UDP_NORMAL";
	extraAllt.strings[2] = "UDP_NORMAL";
	extraAllt.strings[3] = "UDP_NORMAL";
	extraAllt.strings[4] = "UDP_NORMAL";
	extraAllt.strings[5] = "UDP_NORMAL";
}

void initExtraAlltLargeUDP(pizza::special::ExtraAllt &extraAllt) {
	initExtraAlltLarge(extraAllt);
	for(int i = 0; i < 1000; ++i) {
		extraAllt.strings[i] = "UDP_LARGE";
	}
}



void initVessuvioData(pizza::VessuvioData &vessuvio){
	vessuvio.cheese 		= "cheddar";
	vessuvio.tomatoSauce 	= "Hot tomato sauce";
	vessuvio.ham 			= "smoked ham";
}


}

#ifdef _MSC_VER
#pragma pop_macro("max")
#pragma pop_macro("min")
#endif
