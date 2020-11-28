
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

    void ErrorService::report(ErrorMessage_T const className, ErrorMessage_T const methodName, ErrorMessage_T const errorMessage, Error::Severity_T const severity)
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
