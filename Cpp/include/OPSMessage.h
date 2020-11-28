/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019 Lennart Andersson.
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

#ifndef OPSMessageH
#define OPSMessageH

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#ifndef OPSSLIM_NORESERVE
  #include "Reservable.h"
#endif
#include "ArchiverInOut.h"

namespace ops
{

    class OPSMessage : public OPSObject
#ifndef OPSSLIM_NORESERVE
	, public Reservable
#endif
    {
    public:
        OPSMessage() : 
            OPSObject()
#ifndef OPSSLIM_NORESERVE
            , Reservable()
#endif
        {
			TypeId_T const typeName("ops.protocol.OPSMessage");
            OPSObject::appendType(typeName);
        }

		// Copy constructor
		// Note: A copied OPSMessage will NOT copy the Reservable data, ie. the copy will not belong to a reference
		// handler and the reserve/unreserv methods will not work until the copy is added to such a handler
		OPSMessage(const OPSMessage& other) : 
			OPSObject(other)
#ifndef OPSSLIM_NORESERVE
			, Reservable()
#endif
		{
			messageType = other.messageType;
			publisherPriority = other.publisherPriority;
			dataOwner = other.dataOwner;
			sourcePort = other.sourcePort;
			sourceIP = other.sourceIP;
			publicationID = other.publicationID;
			publisherName = other.publisherName;
			topicName = other.topicName;
			topLevelKey = other.topLevelKey;
			address = other.address;
			
			// Clone data
			if (other.data != nullptr) {
				data = other.data->clone();
			} else {
				data = nullptr;
			}
		}
		
		// Move constructor
		OPSMessage(OPSMessage&& other) = delete;

		OPSMessage& operator= (const OPSMessage& other) = delete; // Copy assignment operator
		OPSMessage& operator= (OPSMessage&& other) = delete;      // Move assignment operator
		
		virtual ~OPSMessage()
        {
            if (dataOwner) {
				if (data != nullptr) { delete data; }
            }
        }

		virtual void setDataOwner(bool ownership) noexcept
		{
			dataOwner = ownership;
		}

		virtual bool isDataOwner() const noexcept
		{
			return dataOwner;
		}

	private:
		char messageType{ 0 };			// Serialized (not used, always 0)
		char publisherPriority{ 0 };	// Serialized (not used, always 0)
		bool dataOwner{ true };
		int sourcePort{ 0 };
		Address_T sourceIP;
		int64_t publicationID{ 0 };		// Serialized
		ObjectName_T publisherName;		// Serialized
        ObjectName_T topicName;			// Serialized
        ObjectKey_T topLevelKey;		// Serialized (not used, empty string)
		Address_T address;				// Serialized (not used, empty string)
		OPSObject* data{ nullptr };		// Serialized

		// Hide these since we don't support Clone of an OPSMessage()
		virtual OPSMessage* clone() override { return nullptr; }
		void fillClone(OPSMessage* obj) const { UNUSED(obj); }
	
	public:
        int64_t getPublicationID() const noexcept
        {
            return publicationID;
        }

        void setPublicationID(int64_t pubID) noexcept
        {
            publicationID = pubID;
        }

		ObjectName_T getPublisherName() const noexcept
        {
            return publisherName;
        }

        void setPublisherName(ObjectName_T pubName) noexcept
        {
            publisherName = pubName;
        }

        ObjectName_T getTopicName() const noexcept
        {
            return topicName;
        }

        void setTopicName(ObjectName_T topName) noexcept
        {
            topicName = topName;
        }

        void setData(OPSObject* d) noexcept
        {
			if (dataOwner) {
				if ((data != nullptr) && (data != d)) { delete data; }
			}
            data = d;
        }

        OPSObject* getData() const noexcept
        {
            return data;
        }

		void setSource(Address_T addr, int port) noexcept
		{
			sourceIP = addr;
			sourcePort = port;
		}

		void getSource(Address_T& addr, int& port) const noexcept
		{
			addr = sourceIP;
			port = sourcePort;
		}

        virtual void serialize(ArchiverInOut* archive) override
        {
            OPSObject::serialize(archive);
			
			// Can't change/addto these without breaking compatbility
            archive->inout("messageType", messageType);
            archive->inout("publisherPriority", publisherPriority);
            archive->inout("publicationID", publicationID);
            archive->inout("publisherName", publisherName);
            archive->inout("topicName", topicName);
            archive->inout("topLevelKey", topLevelKey);
            archive->inout("address", address);
            data = (OPSObject*) archive->inout("data", data);
        }
    };

}
#endif
