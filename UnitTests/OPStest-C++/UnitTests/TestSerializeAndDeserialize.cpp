#include "SubscribeDataAndTest.h"
#include "../lib/Init_Data.h"


using namespace std;

pizza::VessuvioData vessuvio;
pizza::special::ExtraAllt extraAllt;


class Test_OPS_Serialization_And_Deserialization : public testing::Test
{
public:
	Test_OPS_Serialization_And_Deserialization() {
	}

  virtual ~Test_OPS_Serialization_And_Deserialization() {}

	Test_OPS_Serialization_And_Deserialization(const Test_OPS_Serialization_And_Deserialization& r) = delete;
	Test_OPS_Serialization_And_Deserialization& operator= (const Test_OPS_Serialization_And_Deserialization& l) = delete;
	Test_OPS_Serialization_And_Deserialization(Test_OPS_Serialization_And_Deserialization&&) = delete;
	Test_OPS_Serialization_And_Deserialization& operator =(Test_OPS_Serialization_And_Deserialization&&) = delete;

	ops::OPSObject* vessuvioObject = nullptr;
    ops::OPSMessage* vessuvioMessage = nullptr;
    pizza::VessuvioData* recreatedVessuvio = nullptr;

	ops::OPSObject* extraAlltObject = nullptr;
	ops::OPSMessage* extraAlltMessage = nullptr;
	pizza::special::ExtraAllt* recreatedExtraAllt = nullptr;

};

TEST_F(Test_OPS_Serialization_And_Deserialization, test_Vesuvio_OPSObject) {
	ops::MemoryMap map(1, 10000);
	ops::ByteBuffer buf(map);

	ops::SerializableInheritingTypeFactory fact;
	fact.add(new PizzaProject::PizzaProjectTypeFactory());

	ops::OPSArchiverOut out(buf, false);
	ops::OPSArchiverIn  in(buf, &fact);

	out.inout("data", &vessuvio);

	EXPECT_EQ(buf.GetSize(), 88);

	buf.Reset();

	EXPECT_EQ(buf.GetSize(), 0);

	recreatedVessuvio = dynamic_cast<pizza::VessuvioData*>(in.inout("data", recreatedVessuvio));

	EXPECT_TRUE(recreatedVessuvio) << "failed creating Vessuvio object";

	test::testVessuvio(*recreatedVessuvio);

	EXPECT_EQ((int) buf.GetSize(), 88);
}

TEST_F(Test_OPS_Serialization_And_Deserialization, test_Vesuvio_OPSMessage) {
	ops::MemoryMap map(1, 10000);
	ops::ByteBuffer buf(map);

	// Setup factory to use when de-serializing a buffer
	ops::SerializableInheritingTypeFactory fact;
	fact.add(new PizzaProject::PizzaProjectTypeFactory());


	ops::OPSArchiverOut out(buf, false);
	ops::OPSArchiverIn  in(buf, &fact);


	ops::OPSMessage mess;
	mess.setDataOwner(false);
	mess.setData(&vessuvio);

	EXPECT_EQ(buf.GetSize(), 0);

	out.inout("data", &mess);

	EXPECT_EQ((int) buf.GetSize(), 146);

	// Create object from buffer (using type info in buffer)
	buf.Reset();

	EXPECT_EQ((int) buf.GetSize(), 0);

	vessuvioObject 		= dynamic_cast<ops::OPSObject*>(in.inout("data", vessuvioObject));
	vessuvioMessage 	= dynamic_cast<ops::OPSMessage*>(vessuvioObject);
	recreatedVessuvio 	= dynamic_cast<pizza::VessuvioData*>(vessuvioMessage->getData());

	EXPECT_TRUE(vessuvioObject);
	EXPECT_TRUE(vessuvioMessage);
	EXPECT_TRUE(recreatedVessuvio);

	test::testVessuvio(*recreatedVessuvio);

	EXPECT_EQ((int) buf.GetSize(), 146);

}

TEST_F(Test_OPS_Serialization_And_Deserialization, test_ExtraAllt_OPSObject) {
	ops::MemoryMap map(1, 10000);
	ops::ByteBuffer buf(map);

	// Setup factory to use when de-serializing a buffer
	ops::SerializableInheritingTypeFactory fact;
	fact.add(new PizzaProject::PizzaProjectTypeFactory());

	// Setup archivers
	ops::OPSArchiverOut out(buf, false);
	ops::OPSArchiverIn  in(buf, &fact);


	EXPECT_EQ(buf.GetSize(), 0);

	out.inout("data", &extraAllt);

	EXPECT_EQ(buf.GetSize(), 807);

	// Create object from buffer (using type info in buffer)
	buf.Reset();

	EXPECT_EQ((int) buf.GetSize(), 0);

	recreatedExtraAllt = dynamic_cast<pizza::special::ExtraAllt*>(in.inout("data", recreatedExtraAllt));

	EXPECT_TRUE(recreatedExtraAllt) << "Failed to create ExtraAllt object";

	test::testExtraAlltNormal(*recreatedExtraAllt);

	EXPECT_EQ((int) buf.GetSize(), 807);
}

TEST_F(Test_OPS_Serialization_And_Deserialization, test_ExtraAllt_OPSMessage) {
	ops::MemoryMap map(1, 10000);
	ops::ByteBuffer buf(map);

	// Setup factory to use when de-serializing a buffer
	ops::SerializableInheritingTypeFactory fact;
	fact.add(new PizzaProject::PizzaProjectTypeFactory());


	ops::OPSArchiverOut out(buf, false);
	ops::OPSArchiverIn  in(buf, &fact);

	pizza::special::ExtraAllt extraAllt2;
	init::initExtraAlltNormal(extraAllt2);


	ops::OPSMessage mess;
	mess.setDataOwner(false);
	mess.setData(&extraAllt2);

	EXPECT_EQ(buf.GetSize(), 0);

	out.inout("data", &mess);

	EXPECT_EQ(buf.GetSize(), 865);

	// Create object from buffer (using type info in buffer)
	buf.Reset();

	EXPECT_EQ(buf.GetSize(), 0);

	extraAlltObject 	= dynamic_cast<ops::OPSObject*>(in.inout("data", extraAlltObject));
	extraAlltMessage 	= dynamic_cast<ops::OPSMessage*>(extraAlltObject);
	recreatedExtraAllt 	= dynamic_cast<pizza::special::ExtraAllt*>(extraAlltMessage->getData());

	EXPECT_TRUE(extraAlltObject) 	<< "Failed to create object" ;
	EXPECT_TRUE(extraAlltMessage) 	<< "Failed to create object as OPSMessage";
	EXPECT_TRUE(recreatedExtraAllt)	<< "Failed to create ExtraAllt object";

	//testing values of extra allt
	test::testExtraAlltNormal(*recreatedExtraAllt);

	EXPECT_EQ(buf.GetSize(), 865);

}

int main(int argc, char** argv) {
	ops::OPSMessage myMess;
	myMess.setDataOwner(true);
	init::initVessuvioData(vessuvio);
	init::initExtraAlltNormal(extraAllt);
    // required init of gtest library
    ::testing::InitGoogleTest(&argc, argv);
    int const result = RUN_ALL_TESTS();

    std::cout << "RUN_ALL_TESTS returned " << result << std::endl;
    return result;
}
