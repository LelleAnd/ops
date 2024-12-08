#pragma once

//
// Example class for demo of unit testing of code using OPS Publisher and/or Subscriber
//

#include <ops.h>

// The following include files are created by the OPS IDL Builder
//  - From ChildData.idl
#include "TestAll/ChildData.h"
#include "TestAll/ChildDataPublisher.h"
#include "TestAll/ChildDataSubscriber.h"

class ClassUnderTest
{
    TestAll::ChildData data;
    TestAll::ChildDataSubscriber sub;
    TestAll::ChildDataPublisher pub;

    bool gotData{ false };
    int64_t latestReceived{ -1 };

public:
    ClassUnderTest(ops::Topic subTopic, ops::Topic pubTopic) :
        sub(subTopic), pub(pubTopic)
    {
        // Set some data to publish
        data.baseText = "Some Server text";
        data.l = 0;

        // Set some publisher properties
        pub.setName("ServerPublisher");
        pub.setKey("InstanceOne");

        // Setup listener
        sub.addDataListener(
            [&](ops::DataNotifier* /*subscriber*/) {
                // In lambdas the same rules applies, as for callbacks and onNewData() listeners

                // Get the actual data object published
                TestAll::ChildData* const dt = sub.getTypedDataReference();

                // Use the data
                latestReceived = dt->l;
                gotData = true;
            }
        );

        // Finally start the subscriber (tell it to start listening for data)
        sub.start();
    }

    std::pair<bool, int64_t> getValue()
    {
        return { gotData, latestReceived };
    }

    void update(int64_t value)
    {
        data.l = value;
        pub.write(data);
    }
};

