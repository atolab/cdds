#ifndef DDS_LOG_H
#define DDS_LOG_H

/** @file log.h
 *  @brief Vortex Lite logging support
 */

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
