#ifndef __underscoredPackName___classNameSubscriber_h
#define __underscoredPackName___classNameSubscriber_h

#include "Subscriber.h"
#include "Topic.h"
#include "OPSObject.h"
#include "__className.h"


__packageDeclaration


class __classNameSubscriber : public ops::Subscriber
{
public:
    __classNameSubscriber(ops::Topic t)
        : ops::Subscriber(t)
    {
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(__className* d)
    {
        if(!data) return false;
        aquireMessageLock();
        ops::Subscriber::getData()->fillClone(d);
        releaseMessageLock();
        return true;
    }

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    bool getData(__className& d)
    {
        if(!data) return false;
        aquireMessageLock();
        ops::Subscriber::getData()->fillClone(&d);
        releaseMessageLock();
        return true;
    }

    // Returns a reference to the latest received data object.
    // Clears the "new data" flag (see newDataExist()).
    // NOTE: MessageLock should be hold while working with the data object, to
    // prevent a new incoming message to delete the current one while in use.
    __className* getTypedDataReference()
    {
        return dynamic_cast<__className*>(getDataReference());
    }

    ~__classNameSubscriber(void)
    {
    }

};


__packageCloser

#endif
