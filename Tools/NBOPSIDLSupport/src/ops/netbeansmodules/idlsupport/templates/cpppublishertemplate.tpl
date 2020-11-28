//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "__className.h"

__packageDeclaration

class __classNamePublisher : public ops::Publisher
{

public:
    explicit __classNamePublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~__classNamePublisher(void)
    {
    }

    bool write(__className* data)
    {
        return ops::Publisher::write(data);
    }

    bool write(__className& data)
    {
        return ops::Publisher::write(&data);
    }

};

__packageCloser
