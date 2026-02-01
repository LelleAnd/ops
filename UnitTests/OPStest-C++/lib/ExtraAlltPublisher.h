//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "ExtraAllt.h"

namespace pizza { namespace special {

class ExtraAlltPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    explicit ExtraAlltPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~ExtraAlltPublisher(void)
    {
    }

    bool write(ExtraAllt* data)
    {
        return ops::Publisher::write(data);
    }

    bool write(ExtraAllt& data)
    {
        return ops::Publisher::write(&data);
    }

};

}}

