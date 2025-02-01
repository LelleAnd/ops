#ifndef ops_BasicError_h
#define ops_BasicError_h

#include "Error.h"

namespace ops
{
	///Basic implementation of an error for OPS.
	class BasicError : public Error
	{
	public:
		static constexpr int ERROR_CODE = 1;
		BasicError(ErrorMessage_T className, ErrorMessage_T method, ErrorMessage_T mess) :
			_message(className)
		{
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
		virtual ~BasicError() = default;
	private:
		ErrorMessage_T _message;
	};
}
#endif
