#include "os/os.h"
#include <stdlib.h>
#include <assert.h>

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler by calling \b atexit passing the \b function
 * to be called when the process exits.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */
os_result
os_procAtExit(
    _In_ void (*function)(void))
{
    int result;
    os_result osResult;

    assert (function != NULL);

    result = atexit (function);
    if(!result)
    {
        osResult = os_resultSuccess;
    } else
    {
        osResult = os_resultFail;
    }
    return osResult;
}
