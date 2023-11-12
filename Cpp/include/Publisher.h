/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2018-2022 Lennart Andersson.
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

#include <atomic>
#include <functional>

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "OPSMessage.h"
#include "Topic.h"
#include "SendDataHandler.h"
#include "MemoryMap.h"
#include "Participant.h"
#include "DebugHandler.h"
#include "Listener.h"
#include "ConnectStatus.h"
#include "Lockable.h"
#include "TimeHelper.h"

namespace ops
{
class Publisher : protected Listener<ConnectStatus>, public Notifier<ConnectStatus>
#ifdef OPS_ENABLE_DEBUG_HANDLER
	, DebugNotifyInterface
#endif
{
public:
	Publisher(Topic t);
    virtual ~Publisher();

	void start();
	void stop();

    Topic getTopic() const;

    void setName(ObjectName_T name);
    void setKey(ObjectKey_T key) noexcept;
    ObjectKey_T getKey() const noexcept;
	ObjectName_T getName() const noexcept;

	bool writeOPSObject(OPSObject* obj);

    // Ack specifics
    void AddExpectedAckSender(const ObjectName_T& subname);
    bool CheckAckSender(const ObjectName_T& subname);    // "" --> check all
    void RemoveExpectedAckSender(const ObjectName_T& subname);

    // Defines return status for doResend question
    enum class ResendAlternative_T : uint8_t { yes, no, no_remove };
    using ShouldResendFunc_T = std::function<ResendAlternative_T(const ObjectName_T& subname, int32_t totfail)>;

    // Need to be called periodically to do resends and update SendState.
    // Optionally a callback can be made to decide on eventual resends (called with internal lock held).
    void Activate();
    void Activate(ShouldResendFunc_T shouldResend);

    enum class SendState : uint8_t {
        init,       // Before anything is published
        sending,    // When Ack is enabled, and waiting for Ack's and doing resends
        acked,      // Message sent and acked (if Ack is enabled)
        failed      // When Ack is enabled, and at least one expected Ack sender failed to send Ack after x resends
    };
    SendState getSendState() const noexcept { return _sendState; }

    // For debug it can be handy to access the PublicationID to be able to synchronize with external tools as eg. Wireshark.
    // Note that the value is the ID to be used in the next message (ie. the number of sent messages, excluding ev. resends).
    int64_t getPublicationID() const noexcept { return currentPublicationID; }

protected:
    int64_t currentPublicationID{ 0 };
    std::shared_ptr<SendDataHandler> sendDataHandler{ nullptr };

    virtual bool write(OPSObject* data);

    bool writeSerializedBuffer();

	// Called from SendDataHandler (TCPServer)
	virtual void onNewEvent(Notifier<ConnectStatus>* sender, ConnectStatus arg) override
	{
		UNUSED(sender)
		// Forward status to all connected
		notifyNewEvent(arg);
	}

private:
    Topic topic;
	MemoryMap memMap;
    ByteBuffer buf;

	OPSMessage message;
 
    Participant* participant{ nullptr };

	ObjectName_T m_name;
    ObjectKey_T m_key;

    bool started{ false };

#ifdef OPS_ENABLE_DEBUG_HANDLER
    volatile int64_t _dbgSkip{ 0 };
	Lockable _dbgLock;
	std::vector<OPSObject*> _replace;
	virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp) override;
	bool internalWrite(OPSObject* data);
#endif

    // Ack specifics
    struct AckSubscriber;
    std::unique_ptr<AckSubscriber> _ackSub;
    Lockable _pubLock;
    // Since time_point can't be stored in an atomic, we use a duration from the epoch start
    std::atomic<std::chrono::milliseconds> _ackTimeout;
    const std::chrono::milliseconds _ackTimeoutInc;
    std::atomic<int> _resendsLeft{ -1 };
    bool resendLatest();
    SendState _sendState{ SendState::init };

public:
	//Send behavior parameters
    std::chrono::milliseconds sendSleepTime{ 1 };
    int sleepEverySendPacket{ 100000 };
};

}
