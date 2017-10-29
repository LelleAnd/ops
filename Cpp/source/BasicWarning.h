#pragma once

#include "Error.h"

namespace ops
{
	///Basic implementaion of an error for OPS.
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
		virtual Severity_T getSeverity() { return Error::warning; }
		virtual int getErrorCode()
		{
			return ERROR_CODE;
		}
		virtual ErrorMessage_T getMessage()
		{
			return _message;
		}
		virtual ~BasicWarning(){}
	private:
		ErrorMessage_T _message;
	};
}
