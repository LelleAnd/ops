/**
*
* Copyright (C) 2025 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ops_Validation_h
#define ops_Validation_h

#include <functional>

#include <OPSTypeDefs.h>

namespace ops
{
    class OPSObject;
    class OPSMessage;
    class Publisher;
    class Subscriber;

    // Validation strategies
    //  Participant defines the default used by publishers and subscribers.
    //  Value can be overridden on resp. publisher and subscriber. 
    // 
    //  None:       isValid() on Data objects are NOT called. [default]
    //  Exception:  isValid() is called and if it returns 'false', exception 'ValidationFailed' is thrown from 
    //              publishers and for subscribers the error is logged and message skipped.
    //  Callback:   isValid() is called and if it returns 'false', the provided callback is called.
    //              The callback decides what to do by returning an Action.
    //  Default:    Used when reverting back to default strategy for component.
    enum class ValidationStrategy { None, Exception, Callback, Default };
    
    // Callback result
    //  Skip:       Message is NOT sent from publisher / NOT received by subscribers.
    //  Throw:      Exception 'ValidationFailed' is thrown from publishers and for subscribers the error is 
    //              logged and message skipped.
    //  Pass:       Message is sent anyway / received by subscribers.
    enum class ValidationAction { Skip, Throw, Pass };

    // Callback signatures
    using ValidationPubCallback = std::function<ValidationAction(const Publisher* const, const OPSObject* const)>;
    using ValidationSubCallback = std::function<ValidationAction(const Subscriber* const, const OPSMessage* const, const OPSObject* const)>;

    // Exception
	class ValidationFailed : public std::exception
    {
      public:
        ValidationFailed(ExceptionMessage_T mess) noexcept : message(mess) {}
        const char* what() const noexcept override
        {
            return message.c_str();
        }

      private:
        ExceptionMessage_T message;
    };

}

#endif
