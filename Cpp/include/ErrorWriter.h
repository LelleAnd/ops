#ifndef ops_ErrorWriter_h
#define ops_ErrorWriter_h

#include <string>
#include <ostream>
#include "OPSTypeDefs.h"
#include "Listener.h"
#include <Error.h>
#include "TimeHelper.h"

namespace ops
{
	///Utility class for writing Error to an ostream
	class ErrorWriter : public Listener<Error*>
	{
	public:
		ErrorWriter(std::ostream& os) : oStream(os) {}
		virtual ~ErrorWriter() {}

		void onNewEvent(Notifier<Error*>* notifier, Error* error) override
		{
			UNUSED(notifier);
			oStream << "@" << TimeHelper::getTimeToString();
			if (error->getSeverity() == Error::warning) {
				oStream << " - Warning, code: ";
			} else {
				oStream << " - Error, code: ";
			}
			oStream << error->getErrorCode() << ". Message: " << error->getMessage() << "." << std::endl;
		}

	private:
		std::ostream& oStream;
	};
}
#endif
