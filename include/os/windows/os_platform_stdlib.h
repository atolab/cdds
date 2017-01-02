#ifndef OS_PLATFORM_STDLIB_H
#define OS_PLATFORM_STDLIB_H

#include <sys/stat.h>
#include <io.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_OS_FILESEPCHAR '\\'
#define OS_OS_PATHSEPCHAR ';'
#define OS_OS_EXESUFFIX   ".exe"
#define OS_OS_BATSUFFIX   ".bat"
#define OS_OS_LIB_LOAD_PATH_VAR "PATH"

#define OS_ROK (_S_IREAD)
#define OS_WOK (_S_IWRITE)
#define OS_XOK (_S_IEXEC)
#define OS_FOK (0)

#define OS_ISDIR(mode) (mode & _S_IFDIR)
#define OS_ISREG(mode) (mode & _S_IFREG)
#define OS_ISLNK(mode) (0) /* not supported on this platform */

    /* on this platform these permission masks are don't cares! */
#define S_IRWXU 00700
#define S_IRWXG 00070
#define S_IRWXO 00007

    /* The value _POSIX_PATH_MAX is defined in limits.h, however you have
     * to define _POSIX_ during compilation.This again will remove the
     * _read, _open and _close prototypes!
     */
#define OS_PATH_MAX 255

    typedef HANDLE os_os_dirHandle;

#define MAXHOSTNAMELEN MAX_HOSTNAME_LEN

#if defined (__cplusplus)
}
#endif

#endif /* OS_PLATFORM_STDLIB_H */
