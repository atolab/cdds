/* TODO: add copyright header?
   TODO: do we really need to expose this as an API? */

/** @file
 *
 * @brief DDS C Logging API
 *
 * This header file defines the public API for logging in the
 * VortexDDS C language binding.
 */
#ifndef DDS_LOG_H
#define DDS_LOG_H

#include "os/os_public.h"
#include "dds/dds_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

DDS_EXPORT void dds_log_info (const char * fmt, ...);
DDS_EXPORT void dds_log_warn (const char * fmt, ...);
DDS_EXPORT void dds_log_error (const char * fmt, ...);
DDS_EXPORT void dds_log_fatal (const char * fmt, ...);

#if defined (__cplusplus)
}
#endif
#endif
