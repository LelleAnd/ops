#ifndef ops_ErrorService_h
#define ops_ErrorService_h

#include "Error.h"
#include "Notifier.h"

namespace ops 
{
	class ErrorService : public Notifier<Error*>
	{
	public:
		void report(Error* error);
		void report(ErrorMessage_T className, ErrorMessage_T methodName, ErrorMessage_T errorMessage, Error::Severity_T severity = Error::error);
	};
}
#endif
