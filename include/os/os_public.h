#ifndef OS_PUBLIC_H
#define OS_PUBLIC_H

#include <stddef.h>

#if __linux__ == 1
#include "posix/os_platform_public.h"
#elif __vxworks == 1
#if __RTP__ == 1
#include "vxworks_rtp/os_platform_public.h"
#else
#include "vxworks_km/os_platform_public.h"
#endif
#elif __sun == 1
#include "solaris/os_platform_public.h"
#elif defined(__INTEGRITY)
#include "integrity/os_platform_public.h"
#elif  __PikeOS__ == 1
#include "pikeos3/os_platform_public.h"
#elif defined(__QNX__)
#include "qnx/os_platform_public.h"
#elif defined(_MSC_VER)
#ifdef _WIN32_WCE
#include "wince/os_platform_public.h"
#else
#include "windows/os_platform_public.h"
#endif
#elif defined __APPLE__
#include "posix/os_platform_public.h"
#elif defined __CYGWIN__
#include "cygwin/os_platform_public.h"
#else
#error "Platform missing from os_public.h list"
#endif

#ifndef OS_API_EXPORT
#define OS_API_EXPORT
#define OS_API_IMPORT
#endif

#include "os/os_decl_attributes.h"

#endif
