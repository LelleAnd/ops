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

#include "Publisher.h"
#include "Participant.h"
#include "Domain.h"
#include "OPSArchiverOut.h"
#include "opsidls/OPSConstants.h"
#include "TimeHelper.h"
#include "DataSegmentPool.h"

namespace ops
{
    using namespace opsidls;

    constexpr int UsableSize = OPSConstants::PACKET_MAX_SIZE - OPSConstants::SEGMENT_HEADER_SIZE;

    Publisher::Publisher(Topic t) :
    topic(t),
    memMap(1 + (t.getSampleMaxSize() / UsableSize),
        (t.getSampleMaxSize() >= UsableSize) ? OPSConstants::PACKET_MAX_SIZE : t.getSampleMaxSize() + OPSConstants::SEGMENT_HEADER_SIZE,
        &DataSegmentAllocator::Instance())
    {
        participant = Participant::getInstance(topic.getDomainID(), topic.getParticipantID());
        sendDataHandler = participant->getSendDataHandler(topic);

		message.setKey("");
        message.setPublisherName(name);
        message.setTopicName(topic.getName());
        message.setDataOwner(false);
		
		start();

		// If we let the OS define the port, the transport info isn't available until after start()
		sendDataHandler->updateTransportInfo(topic);
		participant->updateSendPartInfo(topic);

#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.RegisterPub(this, topic.getName());
#endif
	}

    Publisher::~Publisher()
    {
		OPS_DES_TRACE("Pub: Destructor()...\n");
#ifdef OPS_ENABLE_DEBUG_HANDLER
		participant->debugHandler.UnregisterPub(this, topic.getName());
		for (unsigned int i = 0; i < _replace.size(); i++) {
			if (_replace[i] != nullptr) { delete _replace[i]; }
		}
#endif
		stop();
		participant->releaseSendDataHandler(topic);
		OPS_DES_TRACE("Pub: Destructor() finished\n");
	}

	void Publisher::start()
	{
		sendDataHandler->addListener(this);
		sendDataHandler->addPublisher(this, topic);
	}

	void Publisher::stop()
	{
		sendDataHandler->removeListener(this);
		sendDataHandler->removePublisher(this, topic);
	}

    Topic Publisher::getTopic() const
    {
        return this->topic;
    }

    void Publisher::setName(ObjectName_T const name)
    {
        this->name = name;
		message.setPublisherName(name);
	}

    void Publisher::setKey(ObjectKey_T const key) noexcept
    {
        this->key = key;
    }

    ObjectKey_T Publisher::getKey() const noexcept
    {
        return this->key;
    }

	ObjectName_T Publisher::getName() const noexcept
    {
        return this->name;
    }

    bool Publisher::writeOPSObject(OPSObject* const obj)
    {
        return write(obj);
    }

	bool Publisher::write(OPSObject* const data)
	{
        bool sendOK = true;
#ifdef OPS_ENABLE_DEBUG_HANDLER
		const SafeLock lck(&_dbgLock);
		if (_dbgSkip > 0) {
			_dbgSkip--;
		} else {
			if (_replace.size() > 0) {
				sendOK = internalWrite(_replace[_replace.size()-1]);
				delete _replace[_replace.size() - 1];
				_replace.pop_back();
			} else {
				sendOK = internalWrite(data);
			}
		}
        return sendOK;
	}

	bool Publisher::internalWrite(OPSObject* const data)
	{
        bool sendOK = true;
#endif
        if (key != "") {
            data->setKey(key);
        }

        ByteBuffer buf(memMap);

        message.setData(data);
        message.setPublicationID(currentPublicationID);

        buf.writeNewSegment();

        OPSArchiverOut archive(buf, topic.getOptNonVirt());

        archive.inout("message", &message);

        //If data has spare bytes, write them to the end of the buf
        if (message.getData()->spareBytes.size() > 0) {
            buf.WriteChars(&(message.getData()->spareBytes[0]), (int)message.getData()->spareBytes.size());
        }

        buf.finish();

        for (int i = 0; i < buf.getNrOfSegments(); i++) {
            const int segSize = buf.getSegmentSize(i);
            // We need to continue even if a send fails, since it may work to some destinations
            sendOK &= sendDataHandler->sendData(buf.getSegment(i), segSize, topic);
            // Check if we should back off for a while to distribute large messages over time
            if ((i > 0) && (i % sleepEverySendPacket == 0)) {
                TimeHelper::sleep(sendSleepTime);
            }
        }

        currentPublicationID++;
        return sendOK;
    }

#ifdef OPS_ENABLE_DEBUG_HANDLER
	void Publisher::onRequest(opsidls::DebugRequestResponseData& req, opsidls::DebugRequestResponseData& resp)
	{
		const SafeLock lck(&_dbgLock);
		switch (req.Command) {
		case 1: // Request status
			break;
		case 2: // Enable
			_dbgSkip = (req.Param1 == 1) ? 0 : LLONG_MAX;
			break;
		case 3: // PubId
			currentPublicationID += req.Param1;
			break;
		case 4: // Skip next x sends
			_dbgSkip = req.Param1;
			break;
		case 5: // Send directly
            if (req.Objs.size() == 0) { break; }
			if (req.Objs[0] != nullptr) { internalWrite(req.Objs[0]); }
			break;
		case 6: // Send instead of next x ordinary
			// Free ev. objects stored since earlier 
			for (unsigned int i = 0; i < _replace.size(); i++) {
				if (_replace[i] != nullptr) { delete _replace[i]; }
			}
			_replace.clear();
			// Store backwards in _replace vector
			for (auto i = req.Objs.size(); i > 0; i--) {
				_replace.push_back(req.Objs[i-1]);
			}
			// We have now taken over ownership
			req.Objs.clear();
			break;
		default:;
			break;
		}

		// Fill in status
		resp.Enabled = _dbgSkip == 0;
		resp.Result1 = currentPublicationID;
		resp.Result2 = _dbgSkip;
		resp.Result3 = (_replace.size() > 0);
	}
#endif
}
