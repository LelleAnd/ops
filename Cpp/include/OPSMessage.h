/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2026 Lennart Andersson.
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
#ifdef OPS_C17_DETECTED
    protected:
        // Compile-time generated type and inheritance description strings
        constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("ops.protocol.OPSMessage");
        constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

    public:
        static constexpr uint8_t OPSMessage_idlVersion = 0;
        uint8_t OPSMessage_version = OPSMessage_idlVersion;

#ifdef OPS_C17_DETECTED
    protected:
        OPSMessage(std::string_view tName) :
            ops::OPSObject(tName)
#ifndef OPSSLIM_NORESERVE
            , Reservable()
#endif
        {
        }

    public:
        OPSMessage() : OPSMessage(std::string_view(_inheritDesc)) {}
#else
        OPSMessage() :
            OPSObject()
    #ifndef OPSSLIM_NORESERVE
            , Reservable()
    #endif
        {
            TypeId_T const typeName("ops.protocol.OPSMessage");
            OPSObject::appendType(typeName);
        }
#endif

		// Copy constructor
		// Note: A copied OPSMessage will NOT copy the Reservable data, ie. the copy will not belong to a reference
		// handler and the reserve/unreserv methods will not work until the copy is added to such a handler
		OPSMessage(const OPSMessage& other) : 
			OPSObject(other)
#ifndef OPSSLIM_NORESERVE
			, Reservable()
#endif
		{
            OPSMessage_version = other.OPSMessage_version;
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
                dataOwner = true;
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
		uint16_t sourcePort{ 0 };
        uint32_t sourceIP{ 0 };
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

		void setSource(uint32_t addr, uint16_t port) noexcept
		{
			sourceIP = addr;
			sourcePort = port;
		}

		void getSource(Address_T& addr, uint16_t& port) const
		{
            addr = NumberToString((sourceIP >> 24) & 0xFF);
            addr += ".";
            addr += NumberToString((sourceIP >> 16) & 0xFF);
            addr += ".";
            addr += NumberToString((sourceIP >> 8) & 0xFF);
            addr += ".";
            addr += NumberToString((sourceIP) & 0xFF);
			port = sourcePort;
		}

        void getSource(uint32_t& addr, uint16_t& port) const noexcept
        {
            addr = sourceIP;
            port = sourcePort;
        }

        virtual void serialize(ArchiverInOut* archive) override
        {
            OPSObject::serialize(archive);
            if (idlVersionMask != 0) {
                archive->inout("OPSMessage_version", OPSMessage_version);
                ValidateVersion("OPSMessage", OPSMessage_version, OPSMessage_idlVersion);
            } else {
                OPSMessage_version = 0;
            }
            archive->inout("messageType", messageType);
            archive->inout("publisherPriority", publisherPriority);
            archive->inout("publicationID", publicationID);
            archive->inout("publisherName", publisherName);
            archive->inout("topicName", topicName);
            archive->inout("topLevelKey", topLevelKey);
            archive->inout("address", address);
            data = dynamic_cast<OPSObject*>(archive->inout("data", data));
        }
    };

}
#endif
