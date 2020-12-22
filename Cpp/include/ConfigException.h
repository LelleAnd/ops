
#ifndef ops_ConfigException_h
#define ops_ConfigException_h

#include <exception> 

#include "OPSTypeDefs.h"

namespace ops
{
    class ConfigException : public std::exception
    {
    public:
        ConfigException(ExceptionMessage_T mess) noexcept : message(mess)
        {
        }

		const char* what() const noexcept override { return message.c_str(); }

    private:
		ExceptionMessage_T message;
    };
}
#endif
