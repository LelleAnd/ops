/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
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

#ifndef ops_PublisherH
#define	ops_PublisherH

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "OPSMessage.h"
#include "Topic.h"
#include "SendDataHandler.h"
#include "MemoryMap.h"
#include "Participant.h"
#include "OPSExport.h"
#include "DebugHandler.h"

namespace ops
{
class OPS_EXPORT Publisher
#ifdef OPS_ENABLE_DEBUG_HANDLER
	: DebugNotifyInterface
#endif
{
public:
	Publisher(Topic t);
    virtual ~Publisher();

	void start();
	void stop();

    Topic getTopic();

    void setName(ObjectName_T name);
    void setKey(ObjectKey_T key);
    ObjectKey_T getKey();
	ObjectName_T getName();

	void writeOPSObject(OPSObject* obj);

protected:
	void write(OPSObject* data);

private:
    Topic topic;

	MemoryMap memMap;

	SendDataHandler* sendDataHandler;

	OPSMessage message;
 
	Participant* participant;

    int64_t currentPublicationID;
	ObjectName_T name;
    ObjectKey_T key;
	
#ifdef OPS_ENABLE_DEBUG_HANDLER
	volatile int64_t _dbgSkip;
	Lockable _dbgLock;
	std::vector<OPSObject*> _replace;
	virtual void onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp);
	void internalWrite(OPSObject* data);
#endif

public:
	//Send behavior parameters
	int64_t sendSleepTime;
	int sleepEverySendPacket;
};

}
#endif
