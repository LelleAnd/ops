/**
 *
 * Copyright (C) 2024 Lennart Andersson.
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
#ifndef ops_InProcDistributor_h
#define	ops_InProcDistributor_h

#include <map>
#include <functional>

#include "OPSTypeDefs.h"

#include "OPSMessage.h"
#include "Topic.h"

namespace ops
{

    class InProcDistributor
    {
    private:
        // Map that holds pointers to listeners using closures
        std::map<ops::ObjectName_T, std::function<void(OPSMessage* message)>> listeners;

    public:
        InProcDistributor() { }

        void regInProcReceiver(const Topic& topic, std::function<void(OPSMessage* message)> callback)
        {
            listeners[topic.getName()] = callback;
        }

        void unregInProcReceiver(const Topic& topic)
        {
            listeners.erase(topic.getName());
        }

        bool sendMessage(const Topic& topic, const OPSMessage& msg)
        {
            // distribution of message
            if (listeners.find(topic.getName()) != listeners.end()) {
                // call listener with a copy of the message
                listeners[topic.getName()](new OPSMessage(msg));
            }
            return true;
        }

        virtual ~InProcDistributor() = default;

    };

}
#endif
