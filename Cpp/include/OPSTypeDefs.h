/* 
 * File:   OPSTypeDefs.h
 * Author: gravanto
 *
 * Created on February 18, 2010, 9:52 PM
 */

#ifndef _OPSTYPEDEFS_H
#define _OPSTYPEDEFS_H

// -----------------------------------------------------------------------------
// Some OPS configurations
//#define USE_C11           // Enables use of std::mutex, std::thread in some places
                            // instead of boost or WIN32/Linux specific calls

//#define OPSSLIM_NORESERVE // Removes Reservable from OPSMessage

// -----------------------------------------------------------------------------
// OPS uses Little Endian data serialization to improve the performance since
// thats the native packing for x86 and it also works on Arm.
// If OPS is compiled for a Big Endian machine (and it need to communicate with a 
// little endian machine via OPS) you need to uncomment the following define to
// keep the binary compatibility.
//
//#define ON_BIG_ENDIAN_MACHINE

// -----------------------------------------------------------------------------
// Macro used to remove compiler warnings about non used variables/parameters
#define UNUSED(expr) (void)(expr);

#ifndef _WIN32

#define OPS_LINUX

#endif

#ifdef OPS_LINUX

#include "inttypes.h"

typedef  int64_t __int64;
typedef  int16_t __int16;

#endif

#endif	/* _OPSTYPEDEFS_H */

