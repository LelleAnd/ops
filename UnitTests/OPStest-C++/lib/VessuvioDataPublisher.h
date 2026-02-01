//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "VessuvioData.h"

namespace pizza {

class VessuvioDataPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    explicit VessuvioDataPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~VessuvioDataPublisher(void)
    {
    }

    bool write(VessuvioData* data)
    {
        return ops::Publisher::write(data);
    }

    bool write(VessuvioData& data)
    {
        return ops::Publisher::write(&data);
    }

};

}

