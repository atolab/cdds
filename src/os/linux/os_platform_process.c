#include "os/os.h"
#include <unistd.h>

#include "../snippets/code/os_posix_process.c"

#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
int32_t
os_procGetProcessName(
                      char *procName,
                      unsigned procNameSize)
{
    return snprintf(procName, procNameSize, "bla%lu", (unsigned long)getpid());
}
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
