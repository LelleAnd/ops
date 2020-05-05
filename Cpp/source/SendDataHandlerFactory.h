/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020 Lennart Andersson.
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

#ifndef ops_SendDataHandlerFactory_h
#define ops_SendDataHandlerFactory_h

#include <map>

#include "OPSTypeDefs.h"
#include "Topic.h"
#include "Lockable.h"
#include "SendDataHandler.h"

namespace ops
{
    class SendDataHandler;
    class Participant;

    class SendDataHandlerFactory
    {
    private:
        std::map<InternalKey_T, std::shared_ptr<SendDataHandler>> sendDataHandlers;
        Lockable mutex;

        void PostSetup(Topic& top, Participant& participant, std::shared_ptr<SendDataHandler> sdh);

    public:
        explicit SendDataHandlerFactory();
		// Make sure all SendDataHandlers are released before freeing the instance 
 	    ~SendDataHandlerFactory();

        std::shared_ptr<SendDataHandler> getSendDataHandler(Topic& top, Participant& participant);
        void releaseSendDataHandler(Topic& top, Participant& participant);
    };

}

#endif
