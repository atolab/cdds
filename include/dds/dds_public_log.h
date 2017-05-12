/** @file
 *
 * @brief DDS C99 Logging API
 *
 * @todo add copyright header?
 * @todo do we really need to expose this as an API?
 *
 * This header file defines the public API for logging in the
 * VortexDDS C99 language binding.
 */
#ifndef DDS_LOG_H
#define DDS_LOG_H

#if defined (__cplusplus)
extern "C" {
#endif

#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
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
