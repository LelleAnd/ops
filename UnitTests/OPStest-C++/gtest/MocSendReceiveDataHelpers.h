/**
*
* Copyright (C) 2020-2025 Lennart Andersson.
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

#pragma once

#include "ReceiveDataHandlerFactory.h"
#include "SendDataHandlerFactory.h"

struct MocReceiveDataHandler : public ops::ReceiveDataHandler
{
    ops::Topic topic;
    int ss_cnt{ 0 };
    int rcv_cnt{ 0 };
    char* dstBuffer{ nullptr };
    int dstSize{ 0 };

    struct MocReceiver : public ops::Receiver
    {
        MocReceiveDataHandler& owner;
        uint32_t src_address{ 0 };
        uint16_t src_port{ 0 };
        MocReceiver(MocReceiveDataHandler& o) : owner(o) {}
        // Set the receive buffer to use
        virtual void asynchWait(char* bytes, int size) override
        {
            //std::cout << "asyncWait()\n";
            owner.dstBuffer = bytes; owner.dstSize = size;
        };
        virtual bool start() override { ++owner.ss_cnt; return true; }
        virtual void stop() override { --owner.ss_cnt; owner.dstBuffer = nullptr; owner.dstSize = 0; }
        virtual bool asyncFinished() override { return true; }
        virtual size_t bytesAvailable() override { return 0; }

        virtual uint16_t getLocalPort() override { return 0; }
        virtual ops::Address_T getLocalAddress() override { return ""; }

        // Used to get the sender IP and port for a received message
        // Only safe to call in callback, before a new asynchWait() is called.
        virtual void getSource(ops::Address_T& , uint16_t& ) override { }
        virtual void getSource(uint32_t& address, uint16_t& port) override { address = src_address; port = src_port; }
        void IndicateNewData(int size, uint32_t address, uint16_t port)
        {
            src_address = address;
            src_port = port;
            notifyNewEvent(ops::BytesSizePair(owner.dstBuffer, size));
            owner.rcv_cnt++;
        }
    };

    struct MocReceiveDataChannel : public ops::ReceiveDataChannel
    {
        MocReceiveDataHandler& owner;
        MocReceiveDataChannel(MocReceiveDataHandler& o, ops::Topic top, ops::Participant& part) :
            ops::ReceiveDataChannel(top, part, std::unique_ptr<ops::Receiver>(new MocReceiver(o))), owner(o)
        {
            //std::cout << "MocReceiveDataChannel()\n";
        }
        ~MocReceiveDataChannel()
        {
            //std::cout << "~MocReceiveDataChannel()\n";
        }
        MocReceiver* getReceiver()
        {
            return dynamic_cast<MocReceiver*>(receiver.get());
        }
    };

    MocReceiveDataHandler(ops::Topic top, ops::Participant& part) :
        ops::ReceiveDataHandler(part, std::make_unique<MocReceiveDataChannel>(*this, top, part)), topic(top)
    {
        //std::cout << "MocReceiveDataHandler()\n";
    }

    ~MocReceiveDataHandler()
    {
        //std::cout << "~MocReceiveDataHandler()\n";
    }

    void connectStatus(ops::ConnectStatus& cst)
    {
        onStatusChange(*rdcs[0], cst);
    }

    bool setData(const char* buf, const int size, uint32_t address, uint16_t port)
    {
        //std::cout << "setData()\n";
        if ((dstBuffer != nullptr) && (dstSize >= size)) {
            memcpy(dstBuffer, buf, size);
            ((MocReceiveDataChannel*)rdcs[0].get())->getReceiver()->IndicateNewData(size, address, port);
        }
        return true;
    }
};

struct RAII_RDHF
{
    static std::map<ops::ObjectName_T, std::shared_ptr<MocReceiveDataHandler>> RDHS;

    // Will be called for unknown topic transports, which we use to create a MOC ReceiveDataHandler
    static std::shared_ptr<ops::ReceiveDataHandler> yyyy(ops::Topic top, ops::Participant& part)
    {
        //std::cout << "RDHF(), top = " << top.getName() << "\n";
        RDHS[top.getName()] = std::make_shared<MocReceiveDataHandler>(top, part);
        return RDHS[top.getName()];
    }

    RAII_RDHF() { ops::ReceiveDataHandlerFactory::SetBackupHandler(yyyy); }
    ~RAII_RDHF() {
        ops::ReceiveDataHandlerFactory::SetBackupHandler(nullptr);
        for (auto& x : RDHS) { x.second.reset(); }
        RDHS.clear();
    }
};

struct MocSendDataHandler : public ops::SendDataHandler
{
    ops::Topic topic;
    int oc_cnt{ 0 };
    int send_cnt{ 0 };
    std::shared_ptr<MocReceiveDataHandler> rdh{ nullptr };
    bool forward = true;

    uint32_t local_address = 0;
    uint16_t local_port = 0;

    struct MocSender : public ops::Sender
    {
        MocSendDataHandler& owner;
    public:
        MocSender(MocSendDataHandler& o) : owner(o) {}
        virtual bool sendTo(const char* , const int , const ops::Address_T& , const uint16_t ) override { return true; }
        virtual bool open() override { ++owner.oc_cnt; return true; };
        virtual void close() override { --owner.oc_cnt; };

        virtual uint16_t getLocalPort() override { return owner.local_port; };
        virtual ops::Address_T getLocalAddress() override { return ""; };
        virtual uint32_t getLocalAddressHost() override { return owner.local_address; };
    };

    MocSendDataHandler(ops::Topic top, ops::Participant& , uint16_t port) : topic(top), local_port(port)
    {
        //std::cout << "MocSendDataHandler()\n";
        sender = std::unique_ptr<ops::Sender>(new MocSender(*this));
    }

    bool sendData(char* buf, int bufSize, const ops::Topic& ) override
    {
        //std::cout << "MocSendDataHandler::sendData()\n";
        ++send_cnt;
        if ((rdh != nullptr) && (forward)) {
            rdh->setData(buf, bufSize, local_address, local_port);
        }
        return true;
    }

    virtual ~MocSendDataHandler()
    {
        //std::cout << "~MocSendDataHandler()\n";
    }
};

struct RAII_SDHF
{
    static std::map<ops::ObjectName_T, std::shared_ptr<MocSendDataHandler>> SDHS;

    // Will be called for unknown topic transports, which we use to create a MOC SendDataHandler
    static std::shared_ptr<ops::SendDataHandler> zzzz(ops::Topic top, ops::Participant& part)
    {
        //std::cout << "SDHF(), top = " << top.getName() << "\n";
        static uint16_t port = 0;
        SDHS[top.getName()] = std::make_shared<MocSendDataHandler>(top, part, port++);
        return SDHS[top.getName()];
    }

    RAII_SDHF() { ops::SendDataHandlerFactory::SetBackupHandler(zzzz); }
    ~RAII_SDHF() {
        ops::SendDataHandlerFactory::SetBackupHandler(nullptr);
        for (auto& x : SDHS) { x.second.reset(); }
        SDHS.clear();
    }
};
