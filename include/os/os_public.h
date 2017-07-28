#ifndef OS_PUBLIC_H
#define OS_PUBLIC_H

#include <stddef.h>

#if __linux__ == 1
#include "os/posix/os_platform_public.h"
#elif defined(__VXWORKS__)
#include "os/posix/os_platform_public.h"
#elif __sun == 1
#include "os/solaris/os_platform_public.h"
#elif defined(__INTEGRITY)
#include "os/integrity/os_platform_public.h"
#elif  __PikeOS__ == 1
#include "os/pikeos3/os_platform_public.h"
#elif defined(__QNX__)
#include "os/qnx/os_platform_public.h"
#elif defined(_MSC_VER)
#ifdef _WIN32_WCE
#include "os/wince/os_platform_public.h"
#else
#include "os/windows/os_platform_public.h"
#endif
#elif defined __APPLE__
#include "os/posix/os_platform_public.h"
#elif defined __CYGWIN__
#include "os/cygwin/os_platform_public.h"
#else
#error "Platform missing from os_public.h list"
#endif

#include "os/os_decl_attributes.h"

#endif
