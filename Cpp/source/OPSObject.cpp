/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2025 Lennart Andersson.
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

#include <string.h>
#include "OPSObject.h"

namespace ops
{

#if defined(DEBUG_OPSOBJECT_COUNTER)
	std::atomic<uint32_t> OPSObject::_NumOpsObjects{ 0 };
#endif

    OPSObject::OPSObject()
    {
		OPS_OBJ_TRACE("OPSObject()\n");
#if defined(DEBUG_OPSOBJECT_COUNTER)
        _NumOpsObjects++;
#endif
    }

	// Copy constructor
    OPSObject::OPSObject(const OPSObject& other)
    {
		OPS_OBJ_TRACE("OPSObject(const OPSObject& other)\n");
        idlVersionMask = other.idlVersionMask;
		key = other.key;
        typesString = other.typesString;
        OPSObject_version = other.OPSObject_version;
        spareBytes = other.spareBytes;
#if defined(DEBUG_OPSOBJECT_COUNTER)
        _NumOpsObjects++;
#endif
    }

	// Move constructor
	OPSObject::OPSObject(OPSObject&& other)
	{
		OPS_OBJ_TRACE("OPSObject(OPSObject&& other)\n");
		// Take other's resources
        idlVersionMask = other.idlVersionMask;
        key = std::move(other.key);
		typesString = std::move(other.typesString);
        OPSObject_version = other.OPSObject_version;
        spareBytes = std::move(other.spareBytes);
#if defined(DEBUG_OPSOBJECT_COUNTER)
		_NumOpsObjects++;
#endif
	}

	OPSObject::~OPSObject()
	{
#if defined(DEBUG_OPSOBJECT_COUNTER)
		_NumOpsObjects--;
#endif
		OPS_OBJ_TRACE("~OPSObject()\n");
	}

	OPSObject* OPSObject::clone()
	{
		OPSObject* const obj = new OPSObject();
		fillClone(obj);
		return obj;
	}

	void OPSObject::fillClone(OPSObject* obj) const
	{
        if (this == obj) { return; }
        obj->idlVersionMask = idlVersionMask;
        obj->key = key;
		obj->typesString = typesString;
        obj->OPSObject_version = OPSObject_version;
        // Copy spareBytes vector
		const size_t len = spareBytes.size();
		obj->spareBytes.reserve(len);
		obj->spareBytes.resize(len, 0);
		if (len > 0) { memcpy((void*)&obj->spareBytes[0], &spareBytes[0], len); }
	}

	ObjectKey_T OPSObject::getKey() const noexcept
    {
         return key;
    }

	const TypeId_T& OPSObject::getTypeString() const noexcept
	{
		return typesString;
	}

	void OPSObject::setKey(const ObjectKey_T& k) noexcept
	{
		key = k;
	}

	void OPSObject::serialize(ArchiverInOut* const archive)
	{
        if (idlVersionMask != 0) {
            archive->inout("OPSObject_version", OPSObject_version);
            ValidateVersion("OPSObject", OPSObject_version, OPSObject_idlVersion);
        } else {
            OPSObject_version = 0;
        }
        archive->inout("key", key);
	}

	bool OPSObject::isValid() const noexcept
	{
		return true;
	}

}
