/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/****************************************************************
 * Interface definition for process management                  *
 ****************************************************************/

/** \file os_process.h
 *  \brief Process management - process creation and termination
 */

#ifndef OS_PROCESS_H
#define OS_PROCESS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os/os_defs.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
OSAPI_EXPORT os_procId os_procIdSelf(void);

/** \brief Figure out the identity of the current process
 *
 * Possible Results:
 * - returns the actual length of procIdentity
 *
 * Postcondition:
 * - \b procIdentity is ""
 *     the process identity could not be determined
 * - \b procIdentity is "<decimal number>" | "0x<hexadecimal number>"
 *     only the process numeric identity could be determined
 * - \b procIdentity is "<process name> (<decimal number>)" | "<process name> (0x<hexadecimal number>)"
 *     the process name and numeric identity could be determined
 *
 * \b procIdentity will not be filled beyond the specified \b procIdentitySize
 */
OSAPI_EXPORT int
os_procNamePid(
    _Out_writes_z_(procIdentitySize) char *procIdentity,
    _In_ size_t procIdentitySize);

/** \brief Figure out the name of the current process
 *
 * Possible Results:
 * - returns the actual length of procName
 *
 * Postcondition:
 * - \b procName is ""
 *     the process name could not be determined
 * - \b procName is "<process name>"
 *     the process name could be determined
 *
 * \b procName will not be filled beyond the specified \b procNameSize
 */
OSAPI_EXPORT int
os_procName(
    _Out_writes_z_(procNameSize) char *procName,
    _In_ size_t procNameSize);


/** \brief Register an process exit handler
 *
 * Register an process exit handler. Multiple handlers may be
 * registered. The handlers are called in reverse order of
 * registration.
 *
 * Possible Results:
 * - assertion failure: function = NULL
 */
OSAPI_EXPORT os_result
os_procAtExit(
    void (*function)(void));

#if defined (__cplusplus)
}
#endif

#endif /* OS_PROCESS_H */
