/**
 *
 * Copyright (C) 2025 Lennart Andersson.
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

#include "OPSTypeDefs.h"

namespace ops {

    std::string GetUsedBuildOptions()
    {
        std::string res = "";

#if defined(OPS_C23_DETECTED)
        res += "C++23, ";
#elif defined(OPS_C20_DETECTED)
        res += "C++20, ";
#elif defined(OPS_C17_DETECTED)
        res += "C++17, ";
#elif defined(OPS_C14_DETECTED)
        res += "C++14, ";
#endif
#ifdef OPS_ENABLE_TRACE
        res += "OPS_ENABLE_TRACE, ";
#endif
#ifdef OPS_NO_SHMEM_TRANSPORT
        res += "OPS_NO_SHMEM_TRANSPORT, ";
#endif
#ifdef OPS_REMOVE_ASSERT
        res += "OPS_REMOVE_ASSERT, ";
#endif
#ifdef OPSSLIM_NORESERVE
        res += "OPSSLIM_NORESERVE, ";
#endif
#ifdef REPLACE_TRANSPORT_LAYER
        res += "REPLACE_TRANSPORT_LAYER, ";
#endif
#ifdef REPLACE_OPS_CONFIG
        res += "REPLACE_OPS_CONFIG, ";
#endif
#ifdef REPLACE_NETWORK_ALLOC
        res += "REPLACE_NETWORK_ALLOC, ";
#endif
#ifdef REPLACE_OPS_LOCKABLE
        res += "REPLACE_OPS_LOCKABLE, ";
#endif
#ifdef REPLACE_OPS_EVENT
        res += "REPLACE_OPS_EVENT, ";
#endif
#ifdef OPS_ENABLE_DEBUG_HANDLER
        res += "OPS_ENABLE_DEBUG_HANDLER, ";
#endif
#ifdef FIXED_NO_STD_STRING
        res += "FIXED_NO_STD_STRING, ";
#endif
#ifdef USE_FIXED_LENGTH_STRINGS
        res += "\nUSE_FIXED_LENGTH_STRINGS";
        res += ", OBJECT_NAME:" + std::string(NumberToString(FIXED_OBJECT_NAME_SIZE).c_str());
        res += ", MESSAGE_KEY:" + std::string(NumberToString(FIXED_MESSAGE_KEY_SIZE).c_str());
        res += ", TYPE_ID:" + std::string(NumberToString(FIXED_TYPE_ID_SIZE).c_str());
        res += ", CHANNEL_ID:" + std::string(NumberToString(FIXED_CHANNEL_ID_SIZE).c_str());
        res += ", FILENAME:" + std::string(NumberToString(FIXED_FILENAME_SIZE).c_str());
        res += "\n";
#endif
#ifdef ON_BIG_ENDIAN_MACHINE
        res += "ON_BIG_ENDIAN_MACHINE, ";
#endif
#ifdef DEBUG_OPSOBJECT_COUNTER
        res += "DEBUG_OPSOBJECT_COUNTER, ";
#endif
        res += "OPS_COMPILESIGNATURE:";
        res += OPS_COMPILESIGNATURE;
        return res;
    }

}