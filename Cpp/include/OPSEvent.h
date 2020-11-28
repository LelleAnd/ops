/**
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

#pragma once

#include <memory>
#include <chrono>

#include "OPSTypeDefs.h"

namespace ops {

    class Event
    {
    public:
        Event();
        Event(const Event& rhs);
        Event& operator= (const Event& rhs);
        Event(Event&&) = default;
        Event& operator=(Event&&) = default;
        ~Event();
        
        bool waitFor(const std::chrono::milliseconds& timeout);
        // Signal a waiting thread (only one is released)
        void signal();

    private:
        class InternalImpl;
        std::unique_ptr<InternalImpl> _impl;
    };

}
