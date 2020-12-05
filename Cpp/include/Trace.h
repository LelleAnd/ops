#pragma once

/**
 * Copyright (C) 2020 Lennart Andersson.
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

#include <ostream>

namespace ops {

    namespace trace {

        enum class level_t {Trace, Info, Warning, Error};

        std::ostream& operator<< (std::ostream& os, level_t level);

        struct Sink
        {
            static Sink* Instance();
            virtual void Log(const level_t level, const char* grp, const char* msg) noexcept = 0;
        };

        // Call vith a nullptr, restores the default Trace Sink
        void InstallSink(Sink* s = nullptr);

        // Used to limit trace in the default Trace Sink, to given level and greater
        void SetTraceLevel(level_t level);
    }
}
