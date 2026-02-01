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
#ifndef ops_Domain_h
#define	ops_Domain_h

#include <vector>

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "Topic.h"
#include "Channel.h"
#include "Transport.h"

namespace ops
{
    class Domain : public OPSObject
	{
#ifdef OPS_C17_DETECTED
	protected:
		// Compile-time generated type and inheritance description strings
		constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("Domain");
		constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
	private:
#endif
		static constexpr uint8_t Domain_idlVersion = 0;
		uint8_t Domain_version = Domain_idlVersion;

        Address_T domainAddress;
        int timeToLive{ 1 };
        Address_T localInterface{ "0.0.0.0" };
        int inSocketBufferSize{ -1 };		// Use OS default, Topics may override
        int outSocketBufferSize{ -1 };		// Use OS default, Topics may override
		std::vector<Topic* > topics;
		ObjectName_T domainID;
        int metaDataMcPort{ 9494 };         // Default port 
        int debugMcPort{ 0 };
        bool optNonVirt{ false };
        int heartbeatPeriod{ 1000 };
        int heartbeatTimeout{ 3000 };
        int resendNum{ 0 };
        int resendTimeMs{ 10 };
        int registerTimeMs{ 1000 };

        std::vector<Channel* > channels;
		std::vector<Transport* > transports;

		void checkTopicValues(Topic* top) const;
		void checkTransports() const;
		Channel* findChannel(ChannelId_T id) const;
		Topic* findTopic(ObjectName_T id) const;

#ifdef OPS_C17_DETECTED
	protected:
		Domain(std::string_view tName);

	public:
		Domain() : Domain(std::string_view(_inheritDesc)) {}
#else
	public:
		Domain();
#endif

		Address_T getDomainAddress() const noexcept;
		virtual std::vector<Topic* > getTopics() const;
		virtual Topic getTopic(ObjectName_T name) const;
		ObjectName_T getDomainID() const noexcept;
		int getMetaDataMcPort() const noexcept;
		int getDebugMcPort() const noexcept;
		bool getOptNonVirt() const noexcept;
		int getHeartbeatPeriod() const noexcept;
		int getHeartbeatTimeout() const noexcept;
		int getTimeToLive() const noexcept;
		Address_T getLocalInterface() const noexcept;
		int getInSocketBufferSize() const noexcept;
		int getOutSocketBufferSize() const noexcept;
		
		bool existsTopic(ObjectName_T name) const;

		// Returns a newely allocated deep copy/clone of this object.
		virtual Domain* clone() override;

		// Fills the parameter obj with all values from this object.
		void fillClone(Domain* obj) const;

		void serialize(ArchiverInOut* archiver) override;

		Domain(const Domain& other);
		Domain& operator= (const Domain& other);
		Domain(Domain&& other) = default;
		Domain& operator= (Domain&& other) = default;
		virtual ~Domain();
	};
}
#endif
