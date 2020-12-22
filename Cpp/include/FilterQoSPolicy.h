/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020 Lennart Andersson.
*
* This notice apply to all source files, *.cpp, *.h, *.java, and *.cs in this directory 
* and all its subdirectories if nothing else is explicitly stated within the source file itself.
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

#ifndef FilterQoSPolicyH
#define FilterQoSPolicyH

namespace ops
{
    //Forward declaration
    class OPSObject;
    class OPSMessage;

    class FilterQoSPolicy
    {
    public:
        ///Applies a filter in the receiving process in Subscribers.
        ///Returning false from a filter indicates that this data sample (OPSObject)
        ///shall not be propagated to the application layer.
        virtual bool applyFilter(const OPSObject* const o) = 0;

        virtual bool applyFilter(const OPSMessage* const, const OPSObject* const o)
        {
            return applyFilter(o);
        }

        virtual ~FilterQoSPolicy() = default;
    };
}
#endif
