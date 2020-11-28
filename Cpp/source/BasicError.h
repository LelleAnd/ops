#ifndef ops_BasicError_h
#define ops_BasicError_h

#include "Error.h"

namespace ops
{
	///Basic implementaion of an error for OPS.
	class BasicError : public Error
	{
	public:
		static const int ERROR_CODE = 1;
		BasicError(ErrorMessage_T className, ErrorMessage_T method, ErrorMessage_T mess)
		{
			_message = className;
			_message += "::";
			_message += method;
			_message += "(): ";
			_message += mess;
		}
		virtual int getErrorCode() const noexcept override
		{
			return ERROR_CODE;
		}
		virtual ErrorMessage_T getMessage() const noexcept override
		{
			return _message;
		}
		virtual ~BasicError() {}
	private:
		ErrorMessage_T _message;
	};
}
#endif
