/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2020 Lennart Andersson.
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

#ifndef ops_ReceiveDataHandlerFactory_h
#define ops_ReceiveDataHandlerFactory_h

#include <map>
#include <vector>
#include <string>

#include "OPSTypeDefs.h"
#include "Lockable.h"
#include "IOService.h"
#include "ReceiveDataHandler.h"

namespace ops
{
    //Forward declaration..
	class Participant;
	class Topic;

    class ReceiveDataHandlerFactory
    {
    private:
        ///By Singelton, one ReceiveDataHandler per Topic (name) on this Participant
        std::map<InternalKey_T, std::shared_ptr<ReceiveDataHandler>> receiveDataHandlerInstances;

        ///Garbage vector for ReceiveDataHandlers, these can safely be deleted.
        std::vector<std::shared_ptr<ReceiveDataHandler>> garbageReceiveDataHandlers;
        ops::Lockable garbageLock;

        inline InternalKey_T makeKey(const Topic& top, IOService* ioServ);

    public:
        explicit ReceiveDataHandlerFactory();
        std::shared_ptr<ReceiveDataHandler> getReceiveDataHandler(Topic& top, Participant& participant);
        void cleanUpReceiveDataHandlers();
        void releaseReceiveDataHandler(Topic& top, Participant& participant);
		bool cleanUpDone();
        bool dataAvailable();
        ~ReceiveDataHandlerFactory();

        // If set, will be called for all unknown transports
        typedef std::shared_ptr<ReceiveDataHandler>(*rcv_factory_t)(Topic top, Participant& part);
        static void SetBackupHandler(rcv_factory_t f);
    };
}
#endif
