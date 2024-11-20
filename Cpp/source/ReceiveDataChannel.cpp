/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2018-2024 Lennart Andersson.
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
#include "ReceiveDataChannel.h"
#include "OPSArchiverIn.h"
#include "BasicError.h" 
#include "Domain.h"
#include "Participant.h"
#include "ReceiverFactory.h"
#include "CommException.h"
#include "DataSegmentPool.h"

namespace ops
{
    using namespace opsidls;

	ReceiveDataChannel::ReceiveDataChannel(const Topic& top, Participant& part, std::unique_ptr<Receiver> recv) :
        sampleMaxSize(calcSampleMaxSize(top)),
        memMap(1 + ((sampleMaxSize - 1) / OPSConstants::USABLE_SEGMENT_SIZE), OPSConstants::PACKET_MAX_SIZE, &DataSegmentAllocator::Instance()),
        participant(part),
        receiver(std::move(recv))
    {
		if (receiver.get() == nullptr) { receiver = ReceiverFactory::getReceiver(top, participant); }

        if (receiver.get() == nullptr) {
            throw exceptions::CommException("Could not create receiver");
        }

        receiver->addListener(this);
    }

	void ReceiveDataChannel::start()
    {
		receiver->start();
		expectedSegment = 0;
		currentMessageSize = 0;
        receiver->asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
	}

    void ReceiveDataChannel::stop()
    {
		receiver->stop();
	}

	static void ReportError(Participant& participant, ErrorMessage_T message, uint32_t const addr, uint16_t const port)
	{
		message += " [";
        message += NumberToString((addr >> 24u) & 0xFFu);
        message += ".";
        message += NumberToString((addr >> 16u) & 0xFFu);
        message += ".";
        message += NumberToString((addr >>  8u) & 0xFFu);
        message += ".";
        message += NumberToString((addr       ) & 0xFFu);
		message += "::";
		message += NumberToString(port);
		message += ']';
		BasicError err("ReceiveDataChannel", "onNewEvent", message);
		participant.reportError(&err);
	}

    void ReceiveDataChannel::onNewEvent(Notifier<ConnectStatus>*, ConnectStatus arg)
    {
        if (_client != nullptr) {
            _client->onStatusChange(*this, arg);
        }
    }

    ///Override from Listener
    ///Called whenever the receiver has new data.
    void ReceiveDataChannel::onNewEvent(Notifier<BytesSizePair>* const sender, BytesSizePair const byteSizePair)
    {
		UNUSED(sender);
        if (byteSizePair.size <= 0) {
            BasicError err("ReceiveDataChannel", "onNewEvent", "Empty message or error.");
            participant.reportError(&err);

            receiver->asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
            return;
        }

		///TODO Check that all segments come from the same source (IP and port)
		uint32_t addr;
		uint16_t port;
		receiver->getSource(addr, port);

        //Create a temporay map and buf to peek data before putting it in to memMap
        MemoryMap tMap(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
        ByteBuffer tBuf(tMap);

        //Check protocol
        if (tBuf.checkProtocol())
        {
            const int nrOfFragments = tBuf.ReadInt();
            const int currentFragment = tBuf.ReadInt();

            if (currentFragment != (nrOfFragments - 1) && byteSizePair.size != OPSConstants::PACKET_MAX_SIZE)
            {
				ReportError(participant, "Debug: Received broken package.", addr, port);
            }

            currentMessageSize += byteSizePair.size;

            if (currentFragment != expectedSegment)
            {//For testing only...
                if (firstReceived)
                {
					ReportError(participant, "Segment Error, sample will be lost.", addr, port);
                    firstReceived = false;
                }
                expectedSegment = 0;
                currentMessageSize = 0;
                receiver->asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
                return;
            }

            if (currentFragment == (nrOfFragments - 1))
            {
                firstReceived = true;
                expectedSegment = 0;
                ByteBuffer buf(memMap);

                buf.checkProtocol();
                const int i1 = buf.ReadInt();
                const int i2 = buf.ReadInt();
                UNUSED(i1)
                UNUSED(i2)
                const int segmentPaddingSize = buf.GetSize();

                //Read of the actual OPSMessage
                OPSArchiverIn archiver(buf, participant.getObjectFactory());

                OPSMessage* message = nullptr;

				try {
					message = dynamic_cast<OPSMessage*> (archiver.inout("message", message));
				} catch (const std::exception& e) {
					ErrorMessage_T msg("Invalid data on network. Exception: ");
					msg += e.what();
					ReportError(participant, msg, addr, port);
				}
				if (message != nullptr)
                {
					//Check that we succeded in creating the actual data message
					if (message->getData() != nullptr) 
					{
						//Put spare bytes in data of message
						if (!calculateAndSetSpareBytes(buf, message->getData(), segmentPaddingSize)) {
							//Message has consumed more bytes than received
							ReportError(participant, "Invalid data on network. Wrong message length.", addr, port);
						}

						// Add IP and port for source as meta data into OPSMessage
						message->setSource(addr, port);

						//Send it to ReceiveDataHandler
						if (_client != nullptr) {
							_client->onMessage(*this, message);
						}

						currentMessageSize = 0;
					}
					else
					{
						ReportError(participant, "Failed to deserialize message. Check added Factories.", addr, port);
						delete message;
					}
                }
                else
                {
                    //Inform participant that invalid data is on the network.
					ReportError(participant, "Unexpected type received. Type creation failed.", addr, port);
                }
            }
            else
            {
                expectedSegment++;

				if (expectedSegment >= memMap.getNrOfSegments()) {
					ReportError(participant, "Buffer too small for received message.", addr, port);
					expectedSegment = 0;
					currentMessageSize = 0;
				}
            }
            receiver->asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
        }
        else
        {
            //Inform participant that invalid data is on the network.
			ReportError(participant, "Protocol ERROR.", addr, port);
            receiver->asynchWait(memMap.getSegment(expectedSegment), memMap.getSegmentSize());
        }
    }

	// Called when there are no more listeners and we are about to be put on the garbage-list for later removal
    void ReceiveDataChannel::clear()
    {
        receiver->removeListener(this);
    }

    bool ReceiveDataChannel::calculateAndSetSpareBytes(ByteBuffer &buf, OPSObject* obj, int const segmentPaddingSize)
    {
        //We must calculate how many unserialized segment headers we have and substract that total header size from the size of spareBytes.
        const int nrOfSerializedBytes = buf.GetSize();
        const int totalNrOfSegments = currentMessageSize / memMap.getSegmentSize();
        const int nrOfSerializedSegements = nrOfSerializedBytes / memMap.getSegmentSize();
        const int nrOfUnserializedSegments = totalNrOfSegments - nrOfSerializedSegements;

        const int nrOfSpareBytes = currentMessageSize - buf.GetSize() - (nrOfUnserializedSegments * segmentPaddingSize);

        if (nrOfSpareBytes > 0) {
            obj->spareBytes.reserve(nrOfSpareBytes);
            obj->spareBytes.resize(nrOfSpareBytes, 0);

            //This will read the rest of the bytes as raw bytes and put them into spareBytes field of data.
            buf.ReadChars(&(obj->spareBytes[0]), nrOfSpareBytes);
		}

		//Return false if message has consumed more bytes than received (but still within our buffer size)
		return (nrOfSpareBytes >= 0);
    }

}
