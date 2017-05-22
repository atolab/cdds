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

#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Definition of the process identification
 *
 * os_procId is a platform specific definition for a process identification.
 */
//typedef int32_t os_procId;

/**
* A platform specific definition of an invalid os_procId value.
*/
#define OS_INVALID_PID (-1)

/** \brief Exit status definition
 */
typedef enum os_exitStatus {
    /** Return SUCCESSful completion to the parent process */
    OS_EXIT_SUCCESS = 0,
    /** Return FAILURE completion to the parent process */
    OS_EXIT_FAILURE = 1
} os_exitStatus;

/** \brief Definition of the process attributes
 */
typedef struct os_procAttr {
    /** Specifies the scheduling class */
    os_schedClass       schedClass;
    /** Specifies the process priority */
    int32_t             schedPriority;
#if 0
    /** Specifies the process page locking policy */
    os_lockPolicy       lockPolicy;
    /** Process user credentials */
    os_userCred         userCred;
#endif
    /** Do active redirection of stdout/stderr*/
    uint32_t           activeRedirect;
} os_procAttr;

/** \brief Register an process exit handler
 *
 * Register an process exit handler. Multiple handlers may be
 * registered. The handlers are called in reverse order of
 * registration.
 *
 * Possible Results:
 * - assertion failure: function = NULL
 */
OS_API os_result
os_procAtExit(
    void (*function)(void));

/** \brief Returns if threads are terminated by atexit
 *
 * Return values:
 * TRUE - if threads are ungracefully terminated by atexit
 * FALSE - all other situations
 */
OS_API bool
os_procAreThreadsTerminatedByAtExit(
    void);

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * Possible Results:
 * - assertion failure: status != OS_EXIT_SUCCESS &&
 *                      status != OS_EXIT_FAILURE
 * - does not return
 */
OS_API void
os_procExit(
    os_exitStatus status);

/** \brief Create a process that is an instantiation of a program
 *
 * Create a process that is an instantiation of the program
 * specified by the executable file taking the process
 * attributes into account. The process is named by name
 * which will become argv[0] and arguments are passed in
 * argv[1..n]. The process ID is returned in procId.
 *
 * Postcondition:
 * - after successful creation of the process it is not
 *   guaranteed that the schduling and locking process
 *   attributes are effective
 *
 * Possible Results:
 * - assertion failure: executable_file = NULL ||
 *                      name = NULL || arguments = NULL ||
 *                      procAttr = NULL || procId = NULL
 * - returns os_resultSuccess if
 *     the process is created and procId contains the
 *     ID of the process
 * - returns os_resultInvalid if
 *     the executable_file is not executable
 *     or inconsistent process attrributes are passed
 * - returns os_resultFail if
 *     the process is not created because of a failure
 */
OS_API os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr *procAttr,
    os_procId *procId);

/** \brief Send a signal to the identified process
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     the identified signal is send to the identified
 *     process
 * - returns os_resultUnavailable if
 *     the identified process was not found
 * - returns os_resultInvalid if
 *     an invalid argument is passed
 * - returns os_resultFail if
 *     a general failure occurred
 */
OS_API os_result
os_procDestroy(
    os_procId procId,
    int32_t signal);

OS_API os_result
os_procServiceDestroy(
    os_procId pid,
    bool isblocking,
    int32_t checkcount);

/** \brief Check the child exit status of the identified process
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     the child process exit status is stored in status
 * - returns os_resultUnavailable if
 *     the identified process was not found
 * - returns os_resultBusy if
 *     the identified process is still running
 * - returns os_resultInvalid if
 *     an invalid argument is passed
 * - returns os_resultFail if
 *     a general failure occurred
 */
OS_API os_result
os_procCheckStatus(
    os_procId procId,
    int32_t *status);

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
OS_API os_procId
os_procIdSelf(void);

OS_API bool os_procIdIsValid(os_procId);

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
OS_API int32_t
os_procFigureIdentity(
    char *procIdentity,
    uint32_t procIdentitySize);

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
OS_API int32_t
os_procGetProcessName(
    char *procName,
    uint32_t procNameSize);

/** \brief Set the default process attributes
 *
 * Postcondition:
 * - process scheduling class is OS_SCHED_DEFAULT
 * - processing priority is set platform dependent
 * - locking policy is set platform dependent
 * - user credentials is set platform dependent
 *
 * Possible Results:
 * - assertion failure: procAttr = NULL
 */
_Post_satisfies_(procAttr->schedClass == OS_SCHED_DEFAULT)
_Post_satisfies_(procAttr->schedPriority == THREAD_PRIORITY_NORMAL)
_Post_satisfies_(procAttr->activeRedirect == 0)
OS_API void
os_procAttrInit(
        _Out_ os_procAttr *procAttr)
    __nonnull_all__;

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT in
 *   case class could not be determined
 */
OS_API os_schedClass
os_procAttrGetClass(void);

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform dependent valid priority
 */
OS_API int32_t
os_procAttrGetPriority(void);

/** \brief disable paging for all pages mapped into the address
 *         space of the calling process and/or future pages.
 *
 * The flags parameter can be constructed from the bitwise OR
 * of the following constants:
 * - OS_MEMLOCK_CURRENT: lock all pages currently mapped into
 *                       the address space of the process.
 * - OS_MEMLOCK_FUTURE: lock all pages which will become mapped
 *                      into the address space of the process in
 *                      the future.
 */
OS_API os_result
os_procMLockAll(
    unsigned flags);

/** \brief disable paging for a range of pages
 */
OS_API os_result
os_procMLock(
    const void *addr,
    uintptr_t length);

/** \brief enable paging for a range of pages
 */
OS_API os_result
os_procMUnlock(
    const void *addr,
    uintptr_t length);

/** \brief  reenable paging for calling process.
 *
 *  Enables paging for all pages mapped into the address space ofi
 *  the calling process.
 */
OS_API os_result
os_procMUnlockAll(void);

/* Docced in impl file */
OS_API void
os_procInitializeOpenSpliceService(bool isSpliceD);

/* Docced in impl file */
OS_API bool
os_procIsOpenSpliceService(void);

/* Docced in impl file */
OS_API bool
os_procIsOpenSpliceDomainDaemon(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_PROCESS_H */
