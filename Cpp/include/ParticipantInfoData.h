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
#ifndef ops_ParticipantInfoData_h
#define	ops_ParticipantInfoData_h

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "TopicInfoData.h"

namespace ops
{
	///Defines the meta-data that can be sent between Participants
	class ParticipantInfoData : public OPSObject
	{
	public:
		// NOTE. Must be kept in sync with other OPS language implementations
		char ParticipantInfoData_version = ParticipantInfoData_idlVersion;

        static const char ParticipantInfoData_idlVersion = 0;

        ParticipantInfoData()
		{
			appendType(TypeId_T("ops.ParticipantInfoData"));
		}
		
		// Returns a newely allocated deep copy/clone of this object.
		virtual ParticipantInfoData* clone() override
		{
			ParticipantInfoData* ret = new ParticipantInfoData;
			fillClone(ret);
			return ret;
		}

		// Fills the parameter obj with all values from this object.
		void fillClone(ParticipantInfoData* obj) const
		{
			if (this == obj) { return; }
			ops::OPSObject::fillClone(obj);
			obj->ParticipantInfoData_version = ParticipantInfoData_version;
			obj->name = name;
			obj->id = id;
			obj->domain = domain;
			obj->ip = ip;
			obj->languageImplementation = languageImplementation;
			obj->opsVersion = opsVersion;
			obj->mc_udp_port = mc_udp_port;
			obj->mc_tcp_port = mc_tcp_port;
			obj->subscribeTopics = subscribeTopics;
			obj->publishTopics = publishTopics;
			obj->knownTypes = knownTypes;
		}

		void serialize(ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
            if (idlVersionMask != 0) {
                archiver->inout("ParticipantInfoData_version", ParticipantInfoData_version);
                ValidateVersion("ParticipantInfoData", ParticipantInfoData_version, ParticipantInfoData_idlVersion);
            } else {
                ParticipantInfoData_version = 0;
            }
			archiver->inout("name", name);
			archiver->inout("domain", domain);
			archiver->inout("id", id);
			archiver->inout("ip", ip);
			archiver->inout("languageImplementation", languageImplementation);
			archiver->inout("opsVersion", opsVersion);
			archiver->inout("mc_udp_port", mc_udp_port);
			archiver->inout("mc_tcp_port", mc_tcp_port);
			archiver->inout<TopicInfoData>("subscribeTopics", subscribeTopics, TopicInfoData());
			archiver->inout<TopicInfoData>("publishTopics", publishTopics, TopicInfoData());
			archiver->inout("knownTypes", knownTypes);
		}

		ParticipantInfoData(const ParticipantInfoData& other) = default;
		ParticipantInfoData(ParticipantInfoData&& other) = default;
		ParticipantInfoData& operator= (const ParticipantInfoData& other) = default;
		ParticipantInfoData& operator= (ParticipantInfoData&& other) = default;
		virtual ~ParticipantInfoData() = default;

	public:
		InternalString_T name;
		ObjectName_T id;
		ObjectName_T domain;
		Address_T ip;
		InternalString_T languageImplementation;
		InternalString_T opsVersion;
        int mc_udp_port{ 0 };
        int mc_tcp_port{ 0 };

		std::vector<TopicInfoData> subscribeTopics;
		std::vector<TopicInfoData> publishTopics;
		std::vector<TypeId_T> knownTypes;		// Currently not used
	};

}
#endif
