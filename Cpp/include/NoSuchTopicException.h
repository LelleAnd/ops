
#ifndef ops_NoSuchTopicException_h
#define ops_NoSuchTopicException_h

#include <exception> 

#include "OPSTypeDefs.h"

namespace ops
{
	class NoSuchTopicException : public std::exception
	{
	public:
		NoSuchTopicException(ExceptionMessage_T mess) noexcept : message(mess)
		{
		}
		const char* what() const noexcept override { return message.c_str(); }

	private:
		ExceptionMessage_T message;
	};
}
#endif
