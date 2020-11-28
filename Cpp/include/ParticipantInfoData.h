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
#ifndef ops_ParticipantInfoData_h
#define	ops_ParticipantInfoData_h

#include "OPSTypeDefs.h"
#include "OPSObject.h"
#include "TopicInfoData.h"

namespace ops
{
	/// NOTE. Must be kept in sync with other OPS language implementations
	class ParticipantInfoData : public OPSObject
	{
	public:
		ParticipantInfoData()
		{
			appendType(TypeId_T("ops.ParticipantInfoData"));
		}
		
		void serialize(ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);

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

		virtual ~ParticipantInfoData(){}

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
