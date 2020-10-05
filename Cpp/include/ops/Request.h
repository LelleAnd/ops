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

#ifndef ops_RequestH
#define ops_RequestH

#include "OPSObject.h"

namespace ops
{
	class Request : public OPSObject
	{
	public:
        char Request_version = Request_idlVersion;

        static const char Request_idlVersion = 0;
        std::string requestId;

		void serialize(ops::ArchiverInOut* archiver) override
		{
			OPSObject::serialize(archiver);
			if (idlVersionMask != 0) {
				archiver->inout("Request_version", Request_version);
                ValidateVersion("Request", Request_version, Request_idlVersion);
            } else {
				Request_version = 0;
			}
			archiver->inout("requestId", requestId);
		}

		Request* clone() override
		{
			Request* obj = new Request;
			fillClone(obj);
			return obj;
		}

		void fillClone(Request* obj) const
		{
			OPSObject::fillClone(obj);
			obj->requestId = requestId;
		}

	};
}
#endif
