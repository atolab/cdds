#ifndef OS_PLATFORM_PUBLIC_H
#define OS_PLATFORM_PUBLIC_H

#if __linux__ == 1
  #include "os/posix/os_platform_public.h"
#elif defined(__VXWORKS__)
  #include "os/posix/os_platform_public.h"
#elif defined(_MSC_VER)
  #include "os/windows/os_platform_public.h"
#elif defined __APPLE__
  #include "os/posix/os_platform_public.h"
#else
  #error "Platform missing from os_public.h list"
#endif

#endif
