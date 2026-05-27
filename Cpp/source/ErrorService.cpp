
#include "OPSTypeDefs.h"
#include "ErrorService.h"
#include "BasicError.h"
#include "BasicWarning.h"

namespace ops
{
    void ErrorService::report(Error* const error)
    {
        notifyNewEvent(error);
    }

    void ErrorService::report(const ErrorMessage_T& className, const ErrorMessage_T& methodName, const ErrorMessage_T& errorMessage, const Error::Severity_T severity)
    {
		if (severity == Error::warning) {
			BasicWarning error(className, methodName, errorMessage);
			notifyNewEvent(&error);
		} else {
			BasicError error(className, methodName, errorMessage);
			notifyNewEvent(&error);
		}
	}
}
