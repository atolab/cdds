#ifndef OS_PLATFORM_H
#define OS_PLATFORM_H

#include <vxWorks.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define PRIdSIZE "zd"
#define PRIuSIZE "zu"
#define PRIxSIZE "zx"

#ifdef _WRS_KERNEL
/* inttypes.h does not exist in VxWorks DKM */
#include <st_inttypes.h>
#include <cafe/inttypes.h>
#ifdef _WRS_CONFIG_LP64 /* Used in cafe/inttypes.h too */
#define _PFX_64 "l"
#else
#define _PFX_64 "ll"
#endif

/* FIXME: Not a complete replacement for inttypes.h (yet) */
#define PRIu32 "u"
#define PRIu64 _PFX_64 "u"
#define PRId32 "d"
#define PRId64 _PFX_64 "d"
#define PRIi32 "i"
#define PRIi64 _PFX_64 "i"

#define INFINITY infinity()
#define NAN       ((float)(INFINITY * 0.0F))

#if !defined(__PPC) && !defined(__x64_64__)
/* FIXME: Is this still required for VxWorks 7? */
#define OS_USE_ALLIGNED_MALLOC
#endif

#else
#include <inttypes.h>
#endif /* _WRS_KERNEL */

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_VXWORKS 1
#define OS_SOCKET_USE_FCNTL 0
#define OS_SOCKET_USE_IOCTL 1
#define OS_HAS_TSD_USING_THREAD_KEYWORD 1
#define OS_FILESEPCHAR '/'
#define OS_HAS_NO_SET_NAME_PRCTL 1 /* FIXME: Move to CMake */
#define OS_HAS_UCONTEXT_T 1

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ /* FIXME: Move to CMake */
#define OS_ENDIANNESS OS_BIG_ENDIAN
#else
#define OS_ENDIANNESS OS_LITTLE_ENDIAN
#endif

#if defined(__PPC) || defined(__x86_64__) /* FIXME: Move to CMake */
#define OS_64BIT
#endif

typedef double os_timeReal;
typedef int os_timeSec;
typedef uid_t os_uid;
typedef gid_t os_gid;
typedef mode_t os_mode_t;
#ifdef _WRS_KERNEL
typedef RTP_ID os_procId; /* typedef struct wind_rtp *RTP_ID */
#else
typedef pid_t os_procId;

/* If unistd.h is included after stdint.h, intptr_t will be defined twice.
 * It seems like this is an issue with the VxWorks provided header-files. The
 * define done by stdint.h is not checked in unistd.h. Below is a workaround
 * for this issue. */
#if !defined _INTPTR_T && defined _INTPTR
# define _INTPTR_T _INTPTR
#endif
#endif

#include "os/posix/os_platform_socket.h"
#ifdef _WRS_KERNEL
/* Pulling in netinet/in.h automatically pulls in net/mbuf.h in VxWorks DKM */
#undef m_next
#undef m_flags
#endif
#include "os/posix/os_platform_sync.h"
#include "os/posix/os_platform_thread.h"
#include "os/posix/os_platform_stdlib.h"

#if defined (__cplusplus)
}
#endif

#endif /* OS_PLATFORM_H */
