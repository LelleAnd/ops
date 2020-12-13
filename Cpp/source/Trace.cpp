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

#include "Trace.h"
#include "TraceStream.h"

namespace ops {

    namespace trace {

        std::ostream& operator<< (std::ostream& os, level_t level)
        {
            switch (level) {
            case level_t::Error:   os << "Error"; break;
            case level_t::Info:    os << "Info";  break;
            case level_t::Trace:   os << "Trace"; break;
            case level_t::Warning: os << "Warn";  break;
            default:               os << "UNDEF"; break;
            }
            return os;
        }

        // Used to limit trace in the default Trace Sink, to given level and greater
        static level_t currentLevel = level_t::Trace;

        void SetTraceLevel(level_t level) noexcept
        {
            currentLevel = level;
        }

        struct DefaultSink : Sink
        {
            void Log(const level_t level, const char* grp, const char* msg) override
            {
                if (level >= currentLevel) {
                    std::cout << "[" << level << "] " << grp << ": " << msg;
                }
            }
        };

        static DefaultSink AA;

        static Sink* currentSink = &AA;

        Sink* Sink::Instance() noexcept
        {
            return currentSink;
        }

        tracestream_t os_trace(level_t::Trace);
        tracestream_t os_info(level_t::Info);
        tracestream_t os_warning(level_t::Warning);
        tracestream_t os_error(level_t::Error);

        void InstallSink(Sink* s) noexcept
        {
            if (s == nullptr) {
                currentSink = &AA;
            } else {
                currentSink = s;
            }
        }

    }
}
