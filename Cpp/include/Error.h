#ifndef ops_Error_h
#define ops_Error_h

#include "OPSTypeDefs.h"

namespace ops
{
	///Interface for errors in OPS
	class Error
	{
	public:
		typedef enum { warning, error } Severity_T;
		virtual int getErrorCode() const = 0;
		virtual ErrorMessage_T getMessage() const = 0;
		virtual Severity_T getSeverity() const noexcept { return error; }

		virtual ~Error() = default;
	};
}
#endif
