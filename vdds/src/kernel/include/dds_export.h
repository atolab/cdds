#ifndef _DDS_EXPORT_H_
#define _DDS_EXPORT_H_

#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_DLL
    #undef OS_API_EXPORT
    #define DDS_EXPORT extern __declspec (dllexport)
    #define OS_API_EXPORT __declspec (dllexport)
  #else
    #undef OS_API_IMPORT
    #define DDS_EXPORT extern _declspec (dllimport)
    #define OS_API_IMPORT _declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
#endif

#ifdef _WIN32
#define DDS_INLINE
#else
#define DDS_INLINE inline
#endif

#endif
