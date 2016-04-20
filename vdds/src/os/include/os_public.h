#ifndef OS_PUBLIC_H
#define OS_PUBLIC_H

#include <stddef.h>

#if __linux__ == 1
#include "../linux/include/os_platform_public.h"
#elif __vxworks == 1
#if __RTP__ == 1
#include "../vxworks_rtp/include/os_platform_public.h"
#else
#include "../vxworks_km/include/os_platform_public.h"
#endif
#elif __sun == 1
#include "../solaris/include/os_platform_public.h"
#elif defined(__INTEGRITY)
#include "../integrity/include/os_platform_public.h"
#elif  __PikeOS__ == 1
#include "../pikeos3/include/os_platform_public.h"
#elif defined(__QNX__)
#include "../qnx/include/os_platform_public.h"
#elif defined(_MSC_VER)
#ifdef _WIN32_WCE
#include "../wince/include/os_platform_public.h"
#else
#include "../win32/include/os_platform_public.h"
#endif
#elif defined __APPLE__
#include "../darwin/include/os_platform_public.h"
#elif defined __CYGWIN__
#include "../cygwin/include/os_platform_public.h"
#else
#error "Platform missing from os_public.h list"
#endif

#endif
