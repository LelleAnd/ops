#ifndef ops_ErrorService_h
#define ops_ErrorService_h

#include "OPSExport.h"
#include "Error.h"
#include "Notifier.h"

namespace ops 
{
	class OPS_EXPORT ErrorService : public Notifier<Error*>
	{
	public:
		void report(Error* error);
		void report(std::string className, std::string methodName, std::string errorMessage, Error::Severity_T severity = Error::error);
		
	};

}

#endif
