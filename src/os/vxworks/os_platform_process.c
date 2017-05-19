#ifdef _WRS_KERNEL

#include "os/os.h"

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
    return getpid(); /* Mapped to taskIdSelf() in kernel mode */
}

#else
#include "../posix/os_platform_process.c"
#endif /* _WRS_KERNEL */
