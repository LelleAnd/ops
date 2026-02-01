//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "Subscriber.h"
#include "Topic.h"
#include "OPSObject.h"
#include "PizzaData.h"

namespace pizza {

class PizzaDataSubscriber : public ops::Subscriber
{
public:
    explicit PizzaDataSubscriber(ops::Topic t)
        : ops::Subscriber(t)
    {
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(PizzaData* d)
    {
        if(!data) return false;
        aquireMessageLock();
        getTypedDataReference()->fillClone(d);
        releaseMessageLock();
        return true;
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(PizzaData& d)
    {
        if(!data) return false;
        aquireMessageLock();
        getTypedDataReference()->fillClone(&d);
        releaseMessageLock();
        return true;
    }

    // Returns a reference to the latest received data object.
    // Clears the "new data" flag (see newDataExist()).
    // NOTE: MessageLock should be held while working with the data object, to
    // prevent a new incoming message to delete the current one while in use.
    PizzaData* getTypedDataReference()
    {
        return dynamic_cast<PizzaData*>(getDataReference());
    }

    ~PizzaDataSubscriber(void)
    {
    }

};

}

