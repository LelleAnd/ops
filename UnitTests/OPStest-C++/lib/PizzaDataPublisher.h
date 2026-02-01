//Auto generated OPS-code. DO NOT MODIFY!
#pragma once

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "PizzaData.h"

namespace pizza {

class PizzaDataPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    explicit PizzaDataPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~PizzaDataPublisher(void)
    {
    }

    bool write(PizzaData* data)
    {
        return ops::Publisher::write(data);
    }

    bool write(PizzaData& data)
    {
        return ops::Publisher::write(&data);
    }

};

}

