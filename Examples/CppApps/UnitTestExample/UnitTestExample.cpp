//
// Example program that shows how to use the "inprocess" transport (see ops_config.xml) to enable
// unit testing of units/classes that use OPS Publishers and/or Subscribers.
//

#include <iostream>

#include "gtest/gtest.h"

#include <ops.h>

// The following include files are created by the OPS IDL Builder
//  - Factory that is able to create our data objects
#include "TestAll/TestAllTypeFactory.h"

int main(int argc, char** argv)
{
    // Create OPS participant to access a domain in the default configuration file
    // "ops_config.xml" in current working directory. There are other overloads to
    // create a participant for a specific configuration file.
    ops::Participant* const participant = ops::Participant::getInstance("TestAllDomain");

    // Add our generated factory so OPS can create our data objects
    participant->addTypeSupport(new TestAll::TestAllTypeFactory());

    // Add an errorwriter instance to the participant to catch ev. internal OPS errors
    // We can easily write our own if we want to log data in another way.
    ops::ErrorWriter* const errorWriter = new ops::ErrorWriter(std::cout);
    participant->addListener(errorWriter);

    // required init of gtest library
    ::testing::InitGoogleTest(&argc, argv);
    int const result = RUN_ALL_TESTS();

    std::cout << "RUN_ALL_TESTS returned " << result << std::endl;

    return result;
}

