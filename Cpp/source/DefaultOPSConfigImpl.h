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

#ifndef ops_DefaultOPSConfigImpl_h
#define ops_DefaultOPSConfigImpl_h

#include <vector>

#include "OPSTypeDefs.h"
#include "OPSConfig.h"
#include "ArchiverInOut.h"

namespace ops
{
    class DefaultOPSConfigImpl : public OPSConfig
    {
    public:
        static constexpr uint8_t DefaultOPSConfigImpl_idlVersion = 0;
        uint8_t DefaultOPSConfigImpl_version = DefaultOPSConfigImpl_idlVersion;

        DefaultOPSConfigImpl()
        {
            appendType(TypeId_T("DefaultOPSConfigImpl"));
        }

        DefaultOPSConfigImpl(const DefaultOPSConfigImpl& other) = default;
        DefaultOPSConfigImpl& operator= (const DefaultOPSConfigImpl& other) = default;
        DefaultOPSConfigImpl(DefaultOPSConfigImpl&& other) = default;
        DefaultOPSConfigImpl& operator= (DefaultOPSConfigImpl&& other) = default;
        ~DefaultOPSConfigImpl() = default;

        // Returns a newely allocated deep copy/clone of this object.
        virtual DefaultOPSConfigImpl* clone() override
        {
            DefaultOPSConfigImpl* ret = new DefaultOPSConfigImpl;
            fillClone(ret);
            return ret;
        }

        // Fills the parameter obj with all values from this object.
        void fillClone(DefaultOPSConfigImpl* obj) const
        {
            if (this == obj) { return; }
            ops::OPSConfig::fillClone(obj);
            obj->DefaultOPSConfigImpl_version = DefaultOPSConfigImpl_version;
        }

        void serialize(ArchiverInOut* archiver) override
        {
            OPSConfig::serialize(archiver);
            if (idlVersionMask != 0) {
                archiver->inout("DefaultOPSConfigImpl_version", DefaultOPSConfigImpl_version);
                ValidateVersion("DefaultOPSConfigImpl", DefaultOPSConfigImpl_version, DefaultOPSConfigImpl_idlVersion);
            } else {
                DefaultOPSConfigImpl_version = 0;
            }
        }
    };
}
#endif
