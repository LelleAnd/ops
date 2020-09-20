/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2020 Lennart Andersson.
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
#ifndef ops_TopicInfoData_h
#define	ops_TopicInfoData_h

#include "OPSObject.h"
#include "Topic.h"

namespace ops
{
    constexpr VersionMask_T TopicInfoData_Level_Mask = OPSObject_Level_Mask << 1;

    /// NOTE. Must be kept in sync with other OPS language implementations
	class TopicInfoData : public OPSObject
	{
	public:
        char TopicInfoData_version = 0;

        TopicInfoData()
		{
			appendType(TypeId_T("TopicInfoData"));
		}

		explicit TopicInfoData(const Topic& topic)
		{
			appendType(TypeId_T("TopicInfoData"));
			name = topic.getName();
			type = topic.getTypeID();
			transport = topic.getTransport();
			address = topic.getDomainAddress();
			port = topic.getPort();
		}
		
		void serialize(ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
            if (idlVersionMask != 0) {
                archiver->inout("TopicInfoData_version", TopicInfoData_version);
            } else {
                TopicInfoData_version = 0;
            }
            archiver->inout("name", name);
			archiver->inout("type", type);
			archiver->inout("transport", transport);
			archiver->inout("address", address);
			archiver->inout("port", port);
			archiver->inout("keys", keys);
		}

		virtual ~TopicInfoData() {}

	public:
		ObjectName_T name;
		TypeId_T type;
		Transport_T transport;
		Address_T address;
        int port{ 0 };
		std::vector<ObjectKey_T> keys;		// Currently not used
	};

}
#endif
