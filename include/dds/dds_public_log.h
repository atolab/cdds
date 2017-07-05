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

/* TODO: Set dllexport/dllimport for other supporting compilers too; e.g. clang, gcc using CMake generate export header. */
#if defined (_WIN32)
  #if defined(vddsc_EXPORTS)
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT
#endif

DDS_EXPORT void dds_log_info (const char * fmt, ...);
DDS_EXPORT void dds_log_warn (const char * fmt, ...);
DDS_EXPORT void dds_log_error (const char * fmt, ...);
DDS_EXPORT void dds_log_fatal (const char * fmt, ...);

#undef DDS_EXPORT
#if defined (__cplusplus)
}
#endif
#endif
