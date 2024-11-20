#ifndef pizza_PizzaDataPublisher_h
#define pizza_PizzaDataPublisher_h

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "PizzaData.h"

namespace pizza {

class PizzaDataPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    PizzaDataPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~PizzaDataPublisher(void)
    {
    }

    void write(PizzaData* data)
    {
        ops::Publisher::write(data);
    }

    void write(PizzaData& data)
    {
        ops::Publisher::write(&data);
    }

};

}


#endif
