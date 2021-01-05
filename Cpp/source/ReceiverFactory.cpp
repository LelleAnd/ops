/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2020 Lennart Andersson.
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

#include "OPSTypeDefs.h"
#include "ReceiverFactory.h"
#include "Domain.h"
#include "BasicError.h"
#include "NetworkSupport.h"

namespace ops
{
    std::unique_ptr<Receiver> ReceiverFactory::getReceiver(const Topic& top, Participant& participant)
    {
        std::unique_ptr<Receiver> receiver;

        IOService* const ioService = participant.getIOService();

        //This should never happen, log an internal error and return nullptr;
        if (ioService == nullptr)
        {
			BasicError err("ReceiverFactory", "getReceiver", "Unexpected error, ioServide == nullptr");
            participant.reportError(&err);
            return receiver;
        }

		Address_T localIf = doSubnetTranslation(top.getLocalInterface(), ioService);

        if (top.getTransport() == Topic::TRANSPORT_MC)
        {
            receiver = Receiver::createMCReceiver(top.getDomainAddress(), top.getPort(), ioService, localIf, top.getInSocketBufferSize());
        }
        //TRANSPORT_TCP creation moved into TCPReceiveDataHandler
        else if (top.getTransport() == Topic::TRANSPORT_UDP)
        {
            int port = 0;
            // If UDP topic is configured with my node address, we use the configured port, otherwise
            // we use port 0 which will force the OS to create a unique port that we listen to.
            if (isMyNodeAddress(top.getDomainAddress(), ioService)) {
                localIf = top.getDomainAddress();
                port = top.getPort();
            }
            receiver = Receiver::createUDPReceiver(port, ioService, localIf, top.getInSocketBufferSize());
        }
        return receiver;
    }
}
