#ifndef pizza_special_ExtraAlltPublisher_h
#define pizza_special_ExtraAlltPublisher_h

#include "Publisher.h"
#include "Topic.h"
#include "OPSObject.h"
#include "ExtraAllt.h"

namespace pizza { namespace special {

class ExtraAlltPublisher : public ops::Publisher
{
    using ops::Publisher::write;    // Hide base
public:
    ExtraAlltPublisher(ops::Topic t)
        : ops::Publisher(t)
    {
    }

    ~ExtraAlltPublisher(void)
    {
    }

    void write(ExtraAllt* data)
    {
        ops::Publisher::write(data);
    }

    void write(ExtraAllt& data)
    {
        ops::Publisher::write(&data);
    }

};

}}


#endif
