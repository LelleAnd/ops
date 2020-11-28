/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
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

//
// File:   KeyFilterQoSPolicy.h
// Author: Anton Gravestam
//
// Created on den 28 november 2007, 19:59
//

#ifndef _KEYFILTERQOSPOLICY_H
#define	_KEYFILTERQOSPOLICY_H

#include <vector>

#include "OPSTypeDefs.h"
#include "OPSExport.h"
#include "FilterQoSPolicy.h"
#include "Lockable.h"

namespace ops
{
    //Forward declaration
    class OPSObject;

    class OPS_EXPORT KeyFilterQoSPolicy
		: public FilterQoSPolicy, public Lockable
	{
    public:
        // Creates an empty filter that lets all objects thru
        KeyFilterQoSPolicy() noexcept;

        // Creates a filter with one key, that must match for objects to come thru
        KeyFilterQoSPolicy(ObjectKey_T keyString);

        // Creates a filter with N keys, if any matches the object come thru
        // If the provided vector is empty, all objects come thru
        KeyFilterQoSPolicy(std::vector<ObjectKey_T> keyStrings);

        // Replaces current key[s] with the new keys provided
        // If the provided vector is empty, all objects come thru
        void setKeys(std::vector<ObjectKey_T> keyStrings);

        // Replaces current key[s] with the new single key provided
        void setKey(ObjectKey_T key);

        // Returns the current keys
        std::vector<ObjectKey_T> getKeys();

        virtual ~KeyFilterQoSPolicy();

        //Overides applyFilter(OPSObject* o) in FilterQoSPolicy
        bool applyFilter(const OPSObject* const o) override;

    private:
        std::vector<ObjectKey_T> keyStrings;
    };

}

#endif	/* _KEYFILTERQOSPOLICY_H */
