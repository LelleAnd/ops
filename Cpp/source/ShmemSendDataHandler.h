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
#ifndef ops_ShmemSendDataHandler_h
#define	ops_ShmemSendDataHandler_h

#include "OPSTypeDefs.h"
#include "SendDataHandler.h"

namespace ops
{

#ifndef OPS_NO_SHMEM_TRANSPORT
    class ShmemSendDataHandler : public SendDataHandler
    {
    private:
        class DummySender;
        class Impl;

        std::unique_ptr<Impl> shmem;

    public:
        explicit ShmemSendDataHandler(const InternalKey_T& name, const Topic& top);
        ~ShmemSendDataHandler();    // Need to be implemented in cpp-file where class Impl is known (due to unique_ptr)

        bool sendData(char*, int, const Topic&) override;

        // Get some collected telemetry from the sender
        virtual Telemetry getTelemetry() override;

    };
#endif

}
#endif
