/** @file
 *
 * @brief DDS C Logging API
 *
 * @todo add copyright header?
 * @todo do we really need to expose this as an API?
 *
 * This header file defines the public API for logging in the
 * VortexDDS C language binding.
 */
#ifndef DDS_LOG_H
#define DDS_LOG_H

#include "os/os_public.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef _WIN32_DLL_
  #if defined VDDS_BUILD
    #define OS_API OS_API_EXPORT
  #else
    #define OS_API OS_API_IMPORT
  #endif
#else
#define OS_API extern
#endif

OS_API void dds_log_info (const char * fmt, ...);
OS_API void dds_log_warn (const char * fmt, ...);
OS_API void dds_log_error (const char * fmt, ...);
OS_API void dds_log_fatal (const char * fmt, ...);

#undef OS_API
#if defined (__cplusplus)
}
#endif
#endif
