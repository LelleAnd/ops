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
#ifndef ops_TopicInfoData_h
#define	ops_TopicInfoData_h

#include "OPSObject.h"
#include "Topic.h"

namespace ops
{
	///Defines part of the meta-data that can be sent between Participants
	class TopicInfoData : public OPSObject
	{
#ifdef OPS_C17_DETECTED
	protected:
		// Compile-time generated type and inheritance description strings
		constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("TopicInfoData");
		constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

	public:
		// NOTE. Must be kept in sync with other OPS language implementations
		static constexpr uint8_t TopicInfoData_idlVersion = 0;
		uint8_t TopicInfoData_version = TopicInfoData_idlVersion;

#ifdef OPS_C17_DETECTED
	protected:
		TopicInfoData(std::string_view tName) :
			ops::OPSObject(tName)
		{
		}

	public:
		TopicInfoData() : TopicInfoData(std::string_view(_inheritDesc)) {}
#else
		TopicInfoData()
		{
			appendType(TypeId_T("TopicInfoData"));
		}
#endif

		explicit TopicInfoData(const Topic& topic)
#ifdef OPS_C17_DETECTED
			: ops::OPSObject(std::string_view(_inheritDesc))
		{
#else
		{
			appendType(TypeId_T("TopicInfoData"));
#endif
			name = topic.getName();
			type = topic.getTypeID();
			transport = topic.getTransport();
			address = topic.getDomainAddress();
			port = topic.getPort();
		}

		// Returns a newely allocated deep copy/clone of this object.
		virtual TopicInfoData* clone() override
		{
			TopicInfoData* ret = new TopicInfoData;
			fillClone(ret);
			return ret;
		}

		// Fills the parameter obj with all values from this object.
		void fillClone(TopicInfoData* obj) const
		{
			if (this == obj) { return; }
			ops::OPSObject::fillClone(obj);
			obj->TopicInfoData_version = TopicInfoData_version;
			obj->name = name;
			obj->type = type;
			obj->transport = transport;
			obj->address = address;
			obj->port = port;
			obj->keys = keys;
		}
		
		void serialize(ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
            if (idlVersionMask != 0) {
                archiver->inout("TopicInfoData_version", TopicInfoData_version);
                ValidateVersion("TopicInfoData", TopicInfoData_version, TopicInfoData_idlVersion);
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

		TopicInfoData(const TopicInfoData& other) = default;
		TopicInfoData(TopicInfoData&& other) = default;
		TopicInfoData& operator= (const TopicInfoData& other) = default;
		TopicInfoData& operator= (TopicInfoData&& other) = default;
		virtual ~TopicInfoData() = default;

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
