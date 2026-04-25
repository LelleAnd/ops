#ifndef ops_BasicError_h
#define ops_BasicError_h

#include "Error.h"

namespace ops
{
	///Basic implementation of an error for OPS.
	class BasicError : public Error
	{
	public:
		// Error codes
		static constexpr int ERROR_CODE    = 1;  // Generic error code
		static constexpr int ILLEGAL_DATA  = 10; //
		static constexpr int PARAM_ERROR   = 11; //
		static constexpr int CONFIG_ERROR  = 12; //
		static constexpr int ALREADY_INUSE = 13; //
		static constexpr int NO_MATCH      = 14; //

		BasicError(const ErrorMessage_T& className, const ErrorMessage_T& method, const ErrorMessage_T& mess, int errCode = ERROR_CODE) :
			_message(className), _errorCode(errCode)
		{
			_message += "::";
			_message += method;
			_message += "(): ";
			_message += mess;
		}
		virtual int getErrorCode() const noexcept override
		{
			return _errorCode;
		}
		virtual ErrorMessage_T getMessage() const noexcept override
		{
			return _message;
		}
		virtual ~BasicError() = default;

	private:
		ErrorMessage_T _message;
		int _errorCode{ 0 };
	};
}
#endif
