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

#ifndef ops_ReplyH
#define ops_ReplyH

#include "OPSObject.h"

namespace ops
{
	class Reply : public OPSObject
	{
#ifdef OPS_C17_DETECTED
	protected:
		// Compile-time generated type and inheritance description strings
		constexpr static auto _typeName = ops::strings::make_fixed_string_trunc("Reply");
		constexpr static auto _inheritDesc = ops::strings::make_fixed_string_trunc(_typeName, ops::OPSObject::_inheritDesc, ' ');
#endif

	public:
		static constexpr uint8_t Reply_idlVersion = 0;
		uint8_t Reply_version = Reply_idlVersion;

        std::string requestId;
        bool requestAccepted{ false };
        std::string message;

#ifdef OPS_C17_DETECTED
	protected:
		Reply(std::string_view tName) :
			ops::OPSObject(tName)
		{
		}

	public:
		Reply() : Reply(std::string_view(_inheritDesc)) {}
#else
		Reply()
		{
		}
#endif

		void serialize(ops::ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
			if (idlVersionMask != 0) {
				archiver->inout("Reply_version", Reply_version);
                ValidateVersion("Reply", Reply_version, Reply_idlVersion);
            } else {
				Reply_version = 0;
			}
			archiver->inout("requestId", requestId);
			archiver->inout("requestAccepted", requestAccepted);
			archiver->inout("message", message);
		}

		Reply* clone() override
		{
			Reply* obj = new Reply;
			fillClone(obj);
			return obj;
		}

		void fillClone(Reply* obj) const
		{
			if (this == obj) { return; }
			OPSObject::fillClone(obj);
			obj->Reply_version = Reply_version;
			obj->requestId = requestId;
			obj->requestAccepted = requestAccepted;
			obj->message = message;
		}

	};
}
#endif
