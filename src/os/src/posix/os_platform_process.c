#include "os/os.h"
#include <unistd.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "../snippets/code/os_posix_process.c"

#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
int
os_procName(
    char *procName,
    size_t procNameSize)
{
#ifdef __APPLE__
    char* exec, *processName = NULL;
    int ret;
    uint32_t usize = _OS_PROCESS_DEFAULT_NAME_LEN_;
    processName = os_malloc(usize);
    *processName = 0;
    if (_NSGetExecutablePath(processName, &usize) != 0) {
        /* processName is longer than allocated */
        processName = os_realloc(processName, usize + 1);
        if (_NSGetExecutablePath(processName, &usize) == 0) {
            /* path set successful */
        }
    }
    exec = strrchr(processName,'/');
    if (exec) {
        /* move everything following the last slash forward */
        memmove (processName, exec+1, strlen (exec+1) + 1);
    }
    ret = snprintf(procName, procNameSize, "%s", processName);
    os_free (processName);
    return ret;
#else
    return snprintf(procName, procNameSize, "bla%lu", (unsigned long)getpid());
#endif
}
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
