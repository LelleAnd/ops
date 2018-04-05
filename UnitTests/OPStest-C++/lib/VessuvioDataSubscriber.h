#ifndef pizza_VessuvioDataSubscriber_h
#define pizza_VessuvioDataSubscriber_h

#include "Subscriber.h"
#include "Topic.h"
#include "OPSObject.h"
#include "VessuvioData.h"


namespace pizza {


class VessuvioDataSubscriber : public ops::Subscriber
{
public:
    VessuvioDataSubscriber(ops::Topic t)
        : ops::Subscriber(t)
    {
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(VessuvioData* d)
    {
        if(!data) return false;
        aquireMessageLock();
        getTypedDataReference()->fillClone(d);
        releaseMessageLock();
        return true;
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(VessuvioData& d)
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
    VessuvioData* getTypedDataReference()
    {
        return dynamic_cast<VessuvioData*>(getDataReference());
    }

    ~VessuvioDataSubscriber(void)
    {
    }

};

}


#endif
