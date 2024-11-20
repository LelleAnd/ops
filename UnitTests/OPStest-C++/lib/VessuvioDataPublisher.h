#ifndef pizza_VessuvioDataPublisher_h
#define pizza_VessuvioDataPublisher_h

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "VessuvioData.h"

namespace pizza {

class VessuvioDataPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    VessuvioDataPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~VessuvioDataPublisher(void)
    {
    }

    void write(VessuvioData* data)
    {
        ops::Publisher::write(data);
    }

    void write(VessuvioData& data)
    {
        ops::Publisher::write(&data);
    }

};

}


#endif
