#pragma once

/**
 * Copyright (C) 2020-2021 Lennart Andersson.
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

#include <streambuf>
#include <iostream>
#include <iomanip>

#include "Trace.h"
#include "OPSExport.h"

namespace ops {

    namespace trace {

        //Usage:
        // A local stream with the default trace level:
        //   ops::trace::tracestream<256> os();
        //   os << "Hej hopp\n";
        //   os << "i lingonskogen" << std::endl;     // std::endl will end a log line
        //   os << "jippi";
        //   os << std::flush;                        // so will std::flush
        //
        // Some standard log streams defined below are:
        //   os_trace, os_info, os_warning, os_error
        //
        // A stream manipulator to set the log group exist:
        //   grp("loggroup")
        //
        //   os_trace << grp("Group1") << "Hej hopp\n";
        //   os_trace << "i lingonskogen" << std::endl;     // std::endl will end a log line
        //

        // Non-templated base class for storing the used log group
        class tracebuf_grp
        {
        protected:
            std::string _grp;
        public:
            void set_grp(const std::string& g) { _grp = g; }
        };

        // Templated stream buffer with a log level and static buffer, that will output the
        // stream content using ops::trace::Sink::Instance()->Log(...)
        template <size_t siz>
        class tracebuf : public std::streambuf, public tracebuf_grp
        {
        private:
            level_t _level;
            char_type _buffer[siz+1]; // Always a spare at the end
        public:
            explicit tracebuf(level_t level) : _level(level)
            {
                _buffer[siz] = 0;       // null terminate the buffer at the end
                setp(_buffer, _buffer + siz);
                setg(_buffer, _buffer, _buffer + siz);
            }
            void Reset()
            {
                setp(_buffer, _buffer + siz);
                setg(_buffer, _buffer, _buffer + siz);
            }
            int underflow() override { return EOF; }
            int sync() override
            {
                // make sure string is null terminated (we have a spare byte at the end)
                *pptr() = 0;
                // send buffer to logger
                Sink::Instance()->Log(_level, _grp.c_str(), _buffer);
                Reset();
                return 0;
            }
        };

        // Helper class to get the stream buffer fully initiaized
        template <size_t siz>
        class tracebuf_fixed
        {
        protected:
            tracebuf<siz> _buf;
        public:
            explicit tracebuf_fixed(level_t level = level_t::Trace) : _buf(level) {}
        };

        // Stream class including a fixed sized stream buffer.
        // First inherit from the helper above to ensure that the stream buffer is
        // initialized before it is used by the output stream
        template <size_t siz>
        class tracestream : protected tracebuf_fixed<siz>, public std::ostream
        {
        public:
            explicit tracestream(level_t level = level_t::Trace) :
                tracebuf_fixed<siz>(level), std::ostream(&this->_buf)
            {}
        };

        // Some standard log streams
        using tracestream_t = tracestream<256>;
        // To access these when OPS is compiled as a DLL, you need to define OPS_LIBRARY_IMPORT
        extern OPS_EXPORT tracestream_t os_trace;
        extern OPS_EXPORT tracestream_t os_info;
        extern OPS_EXPORT tracestream_t os_warning;
        extern OPS_EXPORT tracestream_t os_error;

        // Stream manipulator that is used to set the log group if the stream buffer is our trace buffer
        class grp
        {
            std::string g;
        public:
            explicit grp(const char *g_) : g(g_) {}
            friend std::ostream& operator<<(std::ostream& os, const grp& g_)
            {
                if (dynamic_cast<tracebuf_grp*>(os.rdbuf()) != nullptr) {
                    dynamic_cast<tracebuf_grp*>(os.rdbuf())->set_grp(g_.g);
                }
                return os;
            }
        };
    }
}
