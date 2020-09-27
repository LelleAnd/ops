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

#ifndef ops_OPSObject_h
#define ops_OPSObject_h

#include "OPSTypeDefs.h"
#if defined(DEBUG_OPSOBJECT_COUNTER)
#include <atomic>
#endif

#include "OPSExport.h"
#include "Serializable.h"
#include "ArchiverInOut.h"

namespace ops
{
    constexpr VersionMask_T OPSObject_Level_Mask = 1;

    ///Base class for object that can be serialized with OPSArchivers
    class OPS_EXPORT OPSObject : public Serializable
    {
    private:
#if defined(DEBUG_OPSOBJECT_COUNTER)
        static std::atomic<uint32_t> _NumOpsObjects;
#endif

        friend class ByteBuffer;
        friend class OPSArchiverIn;
        friend class OPSArchiverOut;

	protected:
        // Used to indicate which classes in the hierachy, if any, that enabled the use of version bytes for this object instance
        VersionMask_T idlVersionMask = 0;

        //Should only be set by the Publisher at publication time and by ByteBuffer at deserialization time.
        ObjectKey_T key;
        TypeId_T typesString;

        char OPSObject_version = 0;

        void appendType(const TypeId_T& type)
		{
			TypeId_T old = typesString;
			typesString = type;
			typesString += ' ';
			typesString += old;
		}

    public:
		ObjectKey_T getKey() const noexcept;
		const TypeId_T& getTypeString() const noexcept;
		void setKey(const ObjectKey_T& k) noexcept;
		virtual void serialize(ArchiverInOut* archive) override;

		///Bytes that hold unserialized data for this object.
		///This happens if a type can not be fully understood by a participants type support.
		std::vector<char> spareBytes;

		///Returns a newely allocated deep copy/clone of this object.
		virtual OPSObject* clone();

		///Fills the parameter obj with all values from this object.
		void fillClone(OPSObject* obj) const;

        VersionMask_T getVersionMask() const
        {
            return idlVersionMask;
        }

        void setVersionMask(VersionMask_T verMask)
        {
            idlVersionMask = verMask;
        }

    public:
        OPSObject();
        virtual ~OPSObject();

        OPSObject(const OPSObject& other);                       // Copy constructor
        OPSObject(OPSObject&& other);                            // Move constructor

        OPSObject& operator= (const OPSObject& other) = default; // Copy assignment operator
        OPSObject& operator= (OPSObject&& other) = default;      // Move assignment operator

#if defined(DEBUG_OPSOBJECT_COUNTER)
        static uint32_t NumOpsObjects() noexcept { return _NumOpsObjects; }
#endif
    };
}
#endif
