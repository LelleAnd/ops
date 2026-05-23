#include "SubscribeDataAndTest.h"

namespace test {

#ifdef _MSC_VER
#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min
#endif

const float float_max 		= std::numeric_limits<float>::max();
const float float_min	 	= std::numeric_limits<float>::min();
const double double_max 	= std::numeric_limits<double>::max();
const double double_min 	= std::numeric_limits<double>::min();

double delta 				= 0.0001;

void testExtraAlltNormal(pizza::special::ExtraAllt &extraAllt) {

	EXPECT_EQ(extraAllt.extraCheese, true);
	EXPECT_EQ(extraAllt.nrOfMushRooms, 'P');
	EXPECT_EQ(extraAllt.meetQuality, 9);
	EXPECT_NEAR(extraAllt.timeBakedHours, 123.4, delta);
	EXPECT_EQ(extraAllt.timeBakedSeconds, 53.4);
	EXPECT_EQ(extraAllt.description, "Pizza with extra allt");
	EXPECT_EQ(extraAllt.cheese_->age, 5);
	EXPECT_EQ(extraAllt.cheese_->name, "chilli cheese");
	EXPECT_EQ(extraAllt.testingShort, 256);


	//test bools
	EXPECT_EQ((extraAllt.bools).size(),(size_t)6) << "size of the bools vector are not correct";
	EXPECT_EQ((extraAllt.bools).at(0),true);
	EXPECT_EQ((extraAllt.bools).at(1),false);
	EXPECT_EQ((extraAllt.bools).at(2),true);
	EXPECT_EQ((extraAllt.bools).at(3),false);
	EXPECT_EQ((extraAllt.bools).at(4),true);
	EXPECT_EQ((extraAllt.bools).at(5),false);

	//test bytes
	EXPECT_EQ((extraAllt.bytes).size(),(size_t)6) << "size of the bytes vector are not correct";
	EXPECT_EQ((extraAllt.bytes).at(0), 'p');
	EXPECT_EQ((extraAllt.bytes).at(1), 'i');
	EXPECT_EQ((extraAllt.bytes).at(2), 'z');
	EXPECT_EQ((extraAllt.bytes).at(3), 'Z');
	EXPECT_EQ((extraAllt.bytes).at(4), 'a');
	EXPECT_EQ((extraAllt.bytes).at(5), '!');

	//test ints
	EXPECT_EQ((extraAllt.ints).size(), (size_t)6) << "size of the ints vector are not correct";
	EXPECT_EQ((extraAllt.ints).at(0), 0);
	EXPECT_EQ((extraAllt.ints).at(1), 123);
	EXPECT_EQ((extraAllt.ints).at(2), -523);
	EXPECT_EQ((extraAllt.ints).at(3), 62860);
	EXPECT_EQ((extraAllt.ints).at(4), std::numeric_limits<int32_t>::min());//  -2147483648);
	EXPECT_EQ((extraAllt.ints).at(5), std::numeric_limits<int32_t>::max());//  2147483647);

	//test shorts
	EXPECT_EQ((extraAllt.shorts).size(), (size_t)6) << "size of the shorts vector are not correct";
	EXPECT_EQ((extraAllt.shorts).at(0), 0);
	EXPECT_EQ((extraAllt.shorts).at(1), 123);
	EXPECT_EQ((extraAllt.shorts).at(2), -523);
	EXPECT_EQ((extraAllt.shorts).at(3), 30000);
	EXPECT_EQ((extraAllt.shorts).at(4), -32768);
	EXPECT_EQ((extraAllt.shorts).at(5), 32767);

	//test floats
	EXPECT_EQ((extraAllt.floats).size(), (size_t)6) << "size of the floats vector are not correct";
	EXPECT_NEAR(extraAllt.floats.at(0), 0.0f, delta);
	EXPECT_NEAR(extraAllt.floats.at(1), 123.4f, delta);
	EXPECT_NEAR(extraAllt.floats.at(2), -523.2f, delta);
	EXPECT_NEAR(extraAllt.floats.at(3), 62860.0f, delta);
	EXPECT_NEAR(extraAllt.floats.at(4), float_min, delta);
	EXPECT_NEAR(extraAllt.floats.at(5), float_max, delta);

	//test doubles
	EXPECT_EQ((extraAllt.doubles).size(), (size_t)6) << "size of the doubles vector are not correct";
	EXPECT_NEAR((extraAllt.doubles).at(0), 0.0, delta);
	EXPECT_NEAR((extraAllt.doubles).at(1), 123.4, delta);
	EXPECT_NEAR((extraAllt.doubles).at(2), -523.2, delta);
	EXPECT_NEAR((extraAllt.doubles).at(3), 62860.0, delta);
	EXPECT_NEAR((extraAllt.doubles).at(4), double_min, delta);
	EXPECT_NEAR((extraAllt.doubles).at(5), double_max, delta);

	//test longs
	EXPECT_EQ((extraAllt.longs).size(), (size_t)6) << "size of the longs vector are not correct";
	EXPECT_EQ((extraAllt.longs).at(0), 0);
	EXPECT_EQ((extraAllt.longs).at(1), 123);
	EXPECT_EQ((extraAllt.longs).at(2), -523);
	EXPECT_EQ((extraAllt.longs).at(3), 3951379600);
	EXPECT_EQ((extraAllt.longs).at(4), std::numeric_limits<int64_t>::min());//  -9223372036854775808);
	EXPECT_EQ((extraAllt.longs).at(5), std::numeric_limits<int64_t>::max());//  9223372036854775807);

	//test cheeses
	EXPECT_EQ((extraAllt.cheeses).size(), (size_t)6) << "size of the cheeses vector are not correct";
	EXPECT_EQ(((extraAllt.cheeses).at(0)).age, 1);
	EXPECT_EQ(((extraAllt.cheeses).at(0)).name, "cheddar");
	EXPECT_EQ(((extraAllt.cheeses).at(1)).age, 2);
	EXPECT_EQ(((extraAllt.cheeses).at(1)).name, "hardost");
	EXPECT_EQ(((extraAllt.cheeses).at(2)).age, 3);
	EXPECT_EQ(((extraAllt.cheeses).at(2)).name, "mogelost");
	EXPECT_EQ(((extraAllt.cheeses).at(3)).age, 4);
	EXPECT_EQ(((extraAllt.cheeses).at(3)).name, "Farskost");
	EXPECT_EQ(((extraAllt.cheeses).at(4)).age, 5);
	EXPECT_EQ(((extraAllt.cheeses).at(4)).name, "!#%&/()=?");
	EXPECT_EQ(((extraAllt.cheeses).at(5)).age, 6);
	EXPECT_EQ(((extraAllt.cheeses).at(5)).name, "Mesvara");
}

void testExtraAlltNormal(pizza::special::ExtraAllt &extraAllt, SendType sendType) {

	testExtraAlltNormal(extraAllt);

	switch(sendType){
	case NORMAL:
		EXPECT_EQ((extraAllt.strings).at(0), std::string("ExtraAllt"));
		EXPECT_EQ((extraAllt.strings).at(1), std::string("ar"));
		EXPECT_EQ((extraAllt.strings).at(2), std::string("den"));
		EXPECT_EQ((extraAllt.strings).at(3), std::string("basta"));
		EXPECT_EQ((extraAllt.strings).at(4), std::string("pizzan"));
		EXPECT_EQ((extraAllt.strings).at(5), std::string("!!!!!!!!!!!!!!!!!!"));
		break;
	case TCP:
		EXPECT_EQ((extraAllt.strings).at(0), std::string("TCP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(1), std::string("TCP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(2), std::string("TCP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(3), std::string("TCP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(4), std::string("TCP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(5), std::string("TCP_NORMAL"));
		break;
	case UDP:
		EXPECT_EQ((extraAllt.strings).at(0), std::string("UDP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(1), std::string("UDP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(2), std::string("UDP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(3), std::string("UDP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(4), std::string("UDP_NORMAL"));
		EXPECT_EQ((extraAllt.strings).at(5), std::string("UDP_NORMAL"));
		break;
	}

}

void testExtraAlltLarge(pizza::special::ExtraAllt &extraAllt, SendType sendType) {

	int exptectVecSize 	= 1000;
	int halfVecSize 	= exptectVecSize/2;

	EXPECT_EQ(extraAllt.extraCheese,false);
	EXPECT_EQ(extraAllt.nrOfMushRooms, ' ');
	EXPECT_EQ(extraAllt.meetQuality, 0);
	EXPECT_EQ(extraAllt.timeBakedHours, 0.0f);
	EXPECT_EQ(extraAllt.timeBakedSeconds, 0.0);
	EXPECT_EQ(extraAllt.description, "");
	EXPECT_EQ(extraAllt.cheese_->age, 0);
	EXPECT_EQ(extraAllt.cheese_->name, "");
	EXPECT_EQ(extraAllt.testingShort, 0);


	EXPECT_EQ(extraAllt.bools.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.bytes.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.ints.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.floats.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.doubles.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.longs.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.strings.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.cheeses.size(), (size_t)exptectVecSize);
	EXPECT_EQ(extraAllt.shorts.size(), (size_t)exptectVecSize);


	for(int i = 0; i < exptectVecSize; ++i) {

		switch (sendType) {
		case TCP:
			EXPECT_EQ(extraAllt.strings.at(i), "TCP_LARGE");
			break;
		case UDP:
			EXPECT_EQ(extraAllt.strings.at(i), "UDP_LARGE");
			break;
		case NORMAL:
			break;
		}

		if (i < halfVecSize+1) {
			EXPECT_EQ((extraAllt.bools).at(i),true);
			EXPECT_EQ(extraAllt.bytes.at(i), 'a');
			EXPECT_EQ(extraAllt.ints.at(i), 5);
			EXPECT_NEAR(extraAllt.floats.at(i), 15.0f, delta);
			EXPECT_NEAR(extraAllt.doubles.at(i), 25.0, delta);
			EXPECT_EQ(extraAllt.longs.at(i), 35);
			EXPECT_EQ(extraAllt.shorts.at(i), 45);

			if(sendType == NORMAL) { EXPECT_EQ(extraAllt.strings.at(i), "hejsan"); }

			EXPECT_EQ(extraAllt.cheeses.at(i).age, 3);
			EXPECT_EQ(extraAllt.cheeses.at(i).name, "ecklig ost");

		} else {
			EXPECT_EQ(extraAllt.bools.at(i), false);
			EXPECT_EQ(extraAllt.bytes.at(i), 'b');
			EXPECT_EQ(extraAllt.ints.at(i), 10);
			EXPECT_NEAR(extraAllt.floats.at(i), 20.0f, delta);
			EXPECT_NEAR(extraAllt.doubles.at(i), 30.0, delta);
			EXPECT_EQ(extraAllt.longs.at(i), 40);
			EXPECT_EQ(extraAllt.shorts.at(i), 50);


			if(sendType == NORMAL) { EXPECT_EQ(extraAllt.strings.at(i), "hoppsan"); }

			EXPECT_EQ((extraAllt.cheeses.at(i)).age, 6);
			EXPECT_EQ((extraAllt.cheeses.at(i)).name, "god ost");
		}
	}
}

void testVessuvio(pizza::VessuvioData &vessuvioData) {
	EXPECT_EQ(vessuvioData.cheese , 	"cheddar");
	EXPECT_EQ(vessuvioData.tomatoSauce, "Hot tomato sauce");
	EXPECT_EQ(vessuvioData.ham, 		"smoked ham");
}

#ifdef _MSC_VER
#pragma pop_macro("max")
#pragma pop_macro("min")
#endif

}
