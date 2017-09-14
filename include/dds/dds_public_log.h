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
#include "dds/dds_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

DDS_EXPORT void dds_log_info (_In_z_ _Printf_format_string_ const char * fmt, ...);
DDS_EXPORT void dds_log_warn (_In_z_ _Printf_format_string_ const char * fmt, ...);
DDS_EXPORT void dds_log_error (_In_z_ _Printf_format_string_ const char * fmt, ...);
DDS_EXPORT void dds_log_fatal (_In_z_ _Printf_format_string_ const char * fmt, ...);

#if defined (__cplusplus)
}
#endif
#endif
