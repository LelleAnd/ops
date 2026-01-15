/**
 *
 * Copyright (C) 2024-2026 Lennart Andersson.
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
#ifndef ops_InProcSendDataHandler_h
#define	ops_InProcSendDataHandler_h

#include "OPSTypeDefs.h"
#include "SendDataHandler.h"
#include "InProcDistributor.h"

namespace ops
{

    class InProcSendDataHandler : public SendDataHandler
    {
    private:
        std::shared_ptr<InProcDistributor> distributor;

        class DummySender : public Sender
        {
            virtual bool sendTo(const char*, const int, const Address_T&, const uint16_t) override { return false; }
            virtual bool send(const char*, const int) override { return false; }
            virtual bool open() override { return true; }
            virtual void close() override { }
            virtual uint16_t getLocalPort() override { return 0; }
            virtual Address_T getLocalAddress() override { return "0.0.0.0"; }
            virtual uint32_t getLocalAddressHost() override { return 0; }
        };

    public:
        explicit InProcSendDataHandler(std::shared_ptr<InProcDistributor> dist) : distributor(dist)
        {
            sender = std::make_unique<DummySender>();
        }

        // Should not be called
        bool sendData(char*, int, const Topic&) override { return false; }

        // Used for inprocess transport
        virtual bool sendMessage(const Topic& topic, const OPSMessage& message) override
        {
            return distributor->sendMessage(topic, message);
        }

        virtual ~InProcSendDataHandler() = default;

    };

}
#endif
