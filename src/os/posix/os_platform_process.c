#include "os/os.h"
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "../snippets/code/os_posix_process.c"

#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
int32_t
os_procGetProcessName(
                      char *procName,
                      unsigned procNameSize)
{
#ifdef __APPLE__
    char* process_env_name;
    char* exec;

    if (processName == NULL) {
        processName = os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = 0;
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            (void) snprintf(processName, _OS_PROCESS_DEFAULT_NAME_LEN_, "%s", process_env_name);
        } else {
            uint32_t usize = _OS_PROCESS_DEFAULT_NAME_LEN_;
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
        }
    }
    return snprintf(procName, procNameSize, "%s", processName);
#else
    return snprintf(procName, procNameSize, "bla%lu", (unsigned long)getpid());
#endif
}
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
