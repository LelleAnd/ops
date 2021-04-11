/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2021 Lennart Andersson.
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

#ifndef ops_topic_h
#define ops_topic_h

#include "OPSTypeDefs.h"
#include "OPSObject.h"

namespace ops
{
    class Participant;

	class Topic : public OPSObject
    {
		friend class Channel;
		friend class Domain;
		friend class Participant;
    
	public:
        char Topic_version = Topic_idlVersion;

        static const char Topic_idlVersion = 0;

        Topic();
        Topic(const ObjectName_T namee, int portt, const TypeId_T typeIDd, const Address_T domainAddresss);
        Topic(const ObjectName_T namee, const TypeId_T typeIDd, int sampleMaxSize, bool useAck, const Topic& base);

        Topic(const Topic& other) = default;
        Topic(Topic&& other) = default;
        Topic& operator= (const Topic& other) = default;
        Topic& operator= (Topic&& other) = default;
        virtual ~Topic() = default;

        // Create ACK topic based on given topic
        static Topic CreateAckTopic(const Topic& base);

        // Returns a newely allocated deep copy/clone of this object.
        virtual Topic* clone() override;

        // Fills the parameter obj with all values from this object.
        void fillClone(Topic* obj) const;

		void setDomainID(ObjectName_T domID) noexcept;
		ObjectName_T getDomainID() const noexcept;

        void setParticipantID(ObjectName_T partID) noexcept;
		ObjectName_T getParticipantID() const noexcept;

		void setTransport(Transport_T transp) noexcept;
		Transport_T getTransport() const noexcept;

		ObjectName_T getName() const noexcept;
		TypeId_T getTypeID() const noexcept;

		void setDomainAddress(Address_T domainAddr) noexcept;
		Address_T getDomainAddress() const noexcept;

		void setLocalInterface(Address_T localIf) noexcept;
		Address_T getLocalInterface() const noexcept;

		void setSampleMaxSize(int size) noexcept;
		int getSampleMaxSize() const noexcept;

		void setPort(int port) noexcept;
		int getPort() const noexcept;

		void setTimeToLive(int ttl) noexcept;
		int getTimeToLive() const noexcept;

		void serialize(ArchiverInOut* archiver) override;

		int64_t getOutSocketBufferSize() const noexcept;
		void setOutSocketBufferSize(int64_t size) noexcept;

		int64_t getInSocketBufferSize() const noexcept;
		void setInSocketBufferSize(int64_t size) noexcept;

		bool getOptNonVirt() const noexcept;
		int getHeartbeatPeriod() const noexcept;
		int getHeartbeatTimeout() const noexcept;
		ChannelId_T getChannelId() const noexcept;

        bool getUseAck() const noexcept;
        void setUseAck(bool value) noexcept;
        int getNumResends() const noexcept;
        int getResendTimeMs() const noexcept;
        int getRegisterTimeMs() const noexcept;

//		Participant* getParticipant() const noexcept
//		{
//			return participant;
//		}

		static Transport_T TRANSPORT_MC;
		static Transport_T TRANSPORT_TCP;
		static Transport_T TRANSPORT_UDP;

	private:
        ObjectName_T name;					// Serialized

        int port{ 0 };						// Serialized
        int timeToLive{ -1 };
		TypeId_T typeID;					// Serialized
		Address_T domainAddress;			// Serialized
		Address_T localInterface;
		ObjectName_T participantID;
		ObjectName_T domainID;
		int sampleMaxSize;					// Serialized
		Transport_T transport;				// Serialized
        int64_t outSocketBufferSize{ -1 };	// Serialized
        int64_t inSocketBufferSize{ -1 };	// Serialized

        bool useAck{ false };               // Serialized from XML-files only

///        Participant* participant{ nullptr };

        // Replaced by values from Domain when getting Topic
        bool optNonVirt{ false };
        int heartbeatPeriod{ 0 };
        int heartbeatTimeout{ 0 };

        // Replaced by values from Channel if specified there,
        // otherwise from Domain when getting Topic
        int resendNum{ -1 };
        int resendTimeMs{ -1 };
        int registerTimeMs{ -1 };

        // Set when created from a channel
		ChannelId_T channelID;
	};
}
#endif
