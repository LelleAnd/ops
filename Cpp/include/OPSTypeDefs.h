/**
 * Copyright (C) 2006-2010 Anton Gravestam.
 * Copyright (C) 2018-2025 Lennart Andersson.
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

 /*
 * File:   OPSTypeDefs.h
 * Author: gravanto
 *
 * Created on February 18, 2010, 9:52 PM
 */

#pragma once

#include <string>
#include <sstream>
#include <cstdint>

// -----------------------------------------------------------------------------
// Macros used for trace of some functionality during development
//#define OPS_ENABLE_TRACE
#ifdef OPS_ENABLE_TRACE
	#include "TraceStream.h"
	#define OPS_TRACE(msg) { ops::trace::os_trace << msg << std::flush; }
    #define OPS_TRACE_INFO(msg) { ops::trace::os_info << msg << std::flush; }
    #define OPS_TRACE_ERROR(msg) { ops::trace::os_error << msg << std::flush; }
    #define OPS_DUMP_MEMORY(msg, addr, len) { \
	  uint8_t* Ptr__ = (uint8_t*)addr; \
	  ops::trace::os_trace << ops::trace::grp(msg) << std::setw(2) << std::hex; \
	  for(int i=0; i<len; ++i) { ops::trace::os_trace << (int)*Ptr__++ << " "; } \
	  ops::trace::os_trace << std::dec << std::endl;\
	}
#else
	#define OPS_TRACE(msg) 
    #define OPS_TRACE_INFO(msg)
    #define OPS_TRACE_ERROR(msg)
    #define OPS_DUMP_MEMORY(msg, addr, len)
#endif
#define OPS_NOTRACE(msg) 

#define OPS_ACK_TRACE(msg)  { OPS_TRACE(ops::trace::grp("ACK") << msg); }
#define OPS_CH_TRACE(msg)   { OPS_NOTRACE(ops::trace::grp("CH") << msg); }
#define OPS_DES_TRACE(msg)  { OPS_NOTRACE(ops::trace::grp("DES") << msg); }
#define OPS_OBJ_TRACE(msg)  { OPS_NOTRACE(ops::trace::grp("OBJ") << msg); }
#define OPS_PIFO_TRACE(msg) { OPS_NOTRACE(ops::trace::grp("PIFO") << msg); }
#define OPS_TCP_TRACE(msg)  { OPS_NOTRACE(ops::trace::grp("TCP") << msg); }
#define OPS_SHM_TRACE(msg)  { OPS_TRACE(ops::trace::grp("SHM") << msg); }

#define OPS_TCP_INFO(msg)   { OPS_TRACE_INFO(ops::trace::grp("TCP") << msg); }

#define OPS_ACK_ERROR(msg)  { OPS_TRACE_ERROR(ops::trace::grp("ACK") << msg); }
#define OPS_TCP_ERROR(msg)  { OPS_TRACE_ERROR(ops::trace::grp("TCP") << msg); }
#define OPS_UDP_ERROR(msg)  { OPS_TRACE_ERROR(ops::trace::grp("UDP") << msg); }

#define OPS_TRACECODE(code) { code }
#define OPS_NOTRACECODE(code)

#define OPS_SHM_TRACE_CODE(code) OPS_NOTRACECODE(code)


// -----------------------------------------------------------------------------
// Some OPS configurations

// We require a c++11 compiler.
#if __cplusplus >= 201103L		// Value according to standard for full C++11 conformity
	#define OPS_C11_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
	// VS2015 still defines _cplusplus to 199711L but supports the features we need.
	// VS2013 an earlier also defines _cplusplus to 199711L but does not support the features.
	#define OPS_C11_DETECTED
#endif
#ifndef OPS_C11_DETECTED
#error C++11 Compiler required
#endif

#if __cplusplus >= 201402L		// Value according to standard for full C++14 conformity
	#define OPS_C14_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 201402L
		#define OPS_C14_DETECTED
	#endif
#endif

#if __cplusplus >= 201703L		// Value according to standard for full C++17 conformity
	#define OPS_C17_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 201703L
		#define OPS_C17_DETECTED
	#endif
#endif

#if __cplusplus >= 202002L		// Value according to standard for full C++20 conformity
#define OPS_C20_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 202002L
		#define OPS_C20_DETECTED
	#endif
#endif

#if __cplusplus >= 202302L		// Value according to standard for full C++23 conformity
	#define OPS_C23_DETECTED
#elif defined(_MSC_VER) && (_MSC_VER >= 1916)
	#if _MSVC_LANG >= 202302L
		#define OPS_C23_DETECTED
	#endif
#endif


//#define OPS_NO_SHMEM_TRANSPORT	// Removes the Shared Memory transport that currently uses boost

//#define OPS_REMOVE_ASSERT			// Define to skip assert() calls in OPS code.
									// (can also be done by defining NDEBUG)

//#define OPSSLIM_NORESERVE			// Removes Reservable from OPSMessage

//#define REPLACE_TRANSPORT_LAYER	// Removes IOService.cpp, Sender.cpp, Receiver.cpp, DeadlineTimer.cpp,
									// NetworkSupport.cpp, TimeHelper.cpp and ShmemNNN
									// from library so you can use your own implementations.

//#define REPLACE_OPS_CONFIG		// Removes the OPSConfig file reader from library so you can implement 
									// your own for targets without a filesystem.

//#define REPLACE_NETWORK_ALLOC		// Removes the ops::DataSegmentPool::Allocate/Deallocate from library so you
									// can use our own implementation.

//#define REPLACE_OPS_LOCKABLE      // Removes Lockable() implementation from library so you can use your own

//#define REPLACE_OPS_EVENT			// Removes Event() implementation from library so you can use your own

#define OPS_ENABLE_DEBUG_HANDLER	// Adds some debug functionality

// -----------------------------------------------------------------------------
// Configure the fixed length string implementation to NOT have std::string support members.
#define FIXED_NO_STD_STRING
#include "fixed_string.h"
#include "fixed_string_support.h"

// -----------------------------------------------------------------------------
// Defines for String handling in OPS

//#define USE_FIXED_LENGTH_STRINGS

#ifdef USE_FIXED_LENGTH_STRINGS

#include "OPSStringLengths.h"

namespace ops {

	// Rest of lengths are internal or defined in relation to the above lengths
	// Need to be able to handle Domain::ParticipantId
	constexpr int FIXED_PART_KEY_SIZE = (FIXED_OBJECT_NAME_SIZE + 2 + FIXED_OBJECT_NAME_SIZE);
	// xxx.xxx.xxx.xxx/xxx.xxx.xxx.xxx or a node-name
	constexpr int FIXED_ADDRESS_SIZE         = 64;
	constexpr int FIXED_TRANSPORT_SIZE       = 10;
	constexpr int FIXED_INTERNAL_STRING_SIZE = 64;
	constexpr int FIXED_EXCEPTION_MSG_SIZE   = 256;
	constexpr int FIXED_ERROR_MSG_SIZE       = 256;
	// channelid::transport::xxx.xxx.xxx.xxx::port
	constexpr int FIXED_INTERNAL_KEY_SIZE = (FIXED_CHANNEL_ID_SIZE + 2 + FIXED_TRANSPORT_SIZE + 2 + FIXED_ADDRESS_SIZE + 2 + 5);

	typedef strings::fixed_string<FIXED_OBJECT_NAME_SIZE>     ObjectName_T;
	typedef strings::fixed_string<FIXED_FILENAME_SIZE>        FileName_T;
	typedef strings::fixed_string<FIXED_MESSAGE_KEY_SIZE>     ObjectKey_T;
	typedef strings::fixed_string<FIXED_TYPE_ID_SIZE>         TypeId_T;
	typedef strings::fixed_string<FIXED_CHANNEL_ID_SIZE>      ChannelId_T;
	// OPS internal
	typedef strings::fixed_string<FIXED_PART_KEY_SIZE>        ParticipantKey_T;
	typedef strings::fixed_string<FIXED_ADDRESS_SIZE>         Address_T;
	typedef strings::fixed_string<FIXED_TRANSPORT_SIZE>       Transport_T;
	typedef strings::fixed_string<FIXED_INTERNAL_STRING_SIZE> InternalString_T;
	typedef strings::fixed_string<FIXED_EXCEPTION_MSG_SIZE, strings::overrun_policy_t::truncate_string> ExceptionMessage_T;
	typedef strings::fixed_string<FIXED_INTERNAL_KEY_SIZE>    InternalKey_T;
	typedef strings::fixed_string<FIXED_ERROR_MSG_SIZE, strings::overrun_policy_t::truncate_string>     ErrorMessage_T;
	typedef const char*                                       InoutName_T;

	// Simple constant to be able to check that include file and compiled library has the same configurable constants
	constexpr uint64_t fixed_string_length_check_value =
		(static_cast<uint64_t>(FIXED_OBJECT_NAME_SIZE) << 0) +
		(static_cast<uint64_t>(FIXED_MESSAGE_KEY_SIZE) << 8) +
		(static_cast<uint64_t>(FIXED_TYPE_ID_SIZE)     << 16) +
		(static_cast<uint64_t>(FIXED_CHANNEL_ID_SIZE)  << 24) +
		(static_cast<uint64_t>(FIXED_FILENAME_SIZE)    << 32);

#else
namespace ops {

	typedef std::string ObjectName_T;
	typedef std::string FileName_T;
	typedef std::string ObjectKey_T;
	typedef std::string TypeId_T;
	typedef std::string ChannelId_T;
	// OPS internal
	typedef std::string ParticipantKey_T;
	typedef std::string Address_T;
	typedef std::string Transport_T;
	typedef std::string InternalString_T;
	typedef std::string ExceptionMessage_T;
	typedef std::string InternalKey_T;
	typedef std::string ErrorMessage_T;
	typedef const std::string& InoutName_T;

	constexpr uint64_t fixed_string_length_check_value = 0;
#endif

    // Other type helpers
    typedef uint32_t VersionMask_T;
    constexpr VersionMask_T MaxVersionMask = 0x7FFFFFFF;

// -----------------------------------------------------------------------------
// OPS uses Little Endian data serialization to improve the performance since
// thats the native packing for x86 and it also works on Arm.
// If OPS is compiled for a Big Endian machine (and it need to communicate with a 
// little endian machine via OPS) you need to uncomment the following define to
// keep the binary compatibility.
//
//#define ON_BIG_ENDIAN_MACHINE

// Define this to add counting of create/delete of OPSObject()
// Also adds a debug function for reading the current number of living OPSObjects
//#define DEBUG_OPSOBJECT_COUNTER


// -----------------------------------------------------------------------------
// Macro used to remove compiler warnings about non used variables/parameters
#define UNUSED(expr) (void)(expr);

// Helper for converting a number to a string
template <typename T>
InternalString_T NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number << std::ends;
	return ss.str().c_str();
}

} // namespace

// Defines for integer types
#ifdef _WIN32

#else 

// Linux
#include "inttypes.h"

typedef int64_t __int64;	// Deprecated, just for backward compatibility (used by some users) 
typedef int16_t __int16;	// - " -

#endif

// ---------------------------------------------
// Construct a compilation signature to be able to detect mismatched headers and compiled libraries.
// The compile signature is compiled into libraries and are checked at runtime against the define.

#ifdef FIXED_NO_STD_STRING
	#define COMPILESIGNATURE_FIXED ""
#else
	#define COMPILESIGNATURE_FIXED "STD"
#endif

#define COMPILESIGNATURE_CXX "C11"

#ifdef OPSSLIM_NORESERVE
	#define COMPILESIGNATURE_SLIM "SLIM"
#else
	#define COMPILESIGNATURE_SLIM ""
#endif

#ifdef OPS_REMOVE_ASSERT
	#define COMPILESIGNATURE_ASSERT ""
#else
	#define COMPILESIGNATURE_ASSERT "ASSERT"
#endif

#ifdef OPS_ENABLE_DEBUG_HANDLER
	#define COMPILESIGNATURE_DBGHND "DBGHND"
#else
	#define COMPILESIGNATURE_DBGHND ""
#endif

#ifdef USE_FIXED_LENGTH_STRINGS
	#define COMPILESIGNATURE_STRINGS "FIX"
#else
	#define COMPILESIGNATURE_STRINGS "STD"
#endif

#ifdef DEBUG_OPSOBJECT_COUNTER
	#define COMPILESIGNATURE_CTR "CTR"
#else
	#define COMPILESIGNATURE_CTR ""
#endif

#define OPS_COMPILESIGNATURE (COMPILESIGNATURE_FIXED COMPILESIGNATURE_CXX COMPILESIGNATURE_SLIM COMPILESIGNATURE_ASSERT\
                              COMPILESIGNATURE_DBGHND COMPILESIGNATURE_STRINGS COMPILESIGNATURE_CTR)

