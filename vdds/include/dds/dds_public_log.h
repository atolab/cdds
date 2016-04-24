#ifndef DDS_LOG_H
#define DDS_LOG_H

/** @file log.h
 *  @brief Vortex Lite logging support
 */
 
#if defined (__cplusplus)
extern "C" {
#endif

#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_DLL
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
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
