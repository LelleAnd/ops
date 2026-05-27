#ifndef ops_ErrorService_h
#define ops_ErrorService_h

#include "OPSTypeDefs.h"

#include "Error.h"
#include "Notifier.h"

namespace ops 
{
	class ErrorService : public Notifier<Error*>
	{
	public:
		void report(Error* error);
		void report(const ErrorMessage_T& className, const ErrorMessage_T& methodName, const ErrorMessage_T& errorMessage, const Error::Severity_T severity = Error::error);
	};
}
#endif
