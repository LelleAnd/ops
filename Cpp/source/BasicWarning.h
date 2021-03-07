#pragma once

#include "Error.h"

namespace ops
{
	///Basic implementation of a warning for OPS.
	class BasicWarning : public Error
	{
	public:
		static const int ERROR_CODE = 1;
		BasicWarning(ErrorMessage_T className, ErrorMessage_T method, ErrorMessage_T mess)
		{
			_message = className;
			_message += "::";
			_message += method;
			_message += "(): ";
			_message += mess;
		}
		virtual Severity_T getSeverity() const noexcept override { return Error::warning; }
		virtual int getErrorCode() const noexcept override
		{
			return ERROR_CODE;
		}
		virtual ErrorMessage_T getMessage() const noexcept override
		{
			return _message;
		}
		virtual ~BasicWarning(){}
	private:
		ErrorMessage_T _message;
	};
}
