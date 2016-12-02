/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2015 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/** \file os/win32/code/os_process.c
 *  \brief WIN32 process management
 *
 * Implements process management for WIN32
 */
#include "os/os.h"

#include <process.h>

/* List of exithandlers */
struct _ospl_handlerList_t {
    void (*handler)(void);
    struct _ospl_handlerList_t *next;
};

/* #642 fix : define mapping between scheduling abstraction and windows
 * Windows provides 6 scheduling classes for the process
 * IDLE_PRIORITY_CLASS
 * BELOW_NORMAL_PRIORITY_CLASS
 * NORMAL_PRIORITY_CLASS
 * ABOVE_NORMAL_PRIORITY_CLASS
 * HIGH_PRIORITY_CLASS
 * REALTIME_PRIORITY_CLASS */

/* These defaults should be modifiable through configuration */
static const os_schedClass TIMESHARE_DEFAULT_SCHED_CLASS = NORMAL_PRIORITY_CLASS;
static const os_schedClass REALTIME_DEFAULT_SCHED_CLASS  = REALTIME_PRIORITY_CLASS;

static os_atomic_voidp_t _ospl_handlerList     = OS_ATOMIC_VOIDP_INIT(0);
static char* processName                       = NULL;

/* Protected functions */
void
os_processModuleInit(void)
{
    return;
}

void
os_processModuleExit(void)
{
    os_free(processName);
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
void
os_procExit(
    os_exitStatus status)
{
    exit((int)status);
}

#define OS_STRLEN_SPLICE_PROCNAME (16) /* strlen("SPLICE_PROCNAME="); */

/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is build from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child precess then replaces the running program with the
 * program provided by the \b executable_file by calling \b execve.
 *
 * The parent process processes the scheduling class and
 * scheduling priority attributes from \b procAttr and
 * sets the scheduling properties of the child process
 * accordingly by calling \b sched_setscheduler.
 */
os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr *procAttr,
    os_procId *procId)
{
    PROCESS_INFORMATION process_info;
    STARTUPINFO si;
    char *inargs;
    LPTCH environment;
    LPTCH environmentCopy;

    os_schedClass effective_process_class;
    int32_t effective_priority;
    //    assert(executable_file != NULL);
    //    assert(name != NULL);
    //    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);

    inargs = (char*)os_malloc(strlen (name) + strlen (arguments) + 4);

    os_strcpy(inargs, "\"");
    os_strcat(inargs, name);
    os_strcat(inargs, "\" ");
    os_strcat(inargs, arguments);

    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if (procAttr->activeRedirect) {
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    /* Duplicate the environment of the current process for the child-process
     * and set the SPLICE_PROCNAME environment variable to name.
     * CreateEnvironmentBlock cannot be used conveniently here, since the
     * environment has to be modified and SPLICE_PROCNAME can be set in the
     * current process too. */
    environment = GetEnvironmentStrings();
    if(environment){
        size_t len = 0;
        size_t newLen = 0;
        size_t spliceVarLen = 0;
        LPTSTR currentVar = (LPTSTR)environment;
        LPTSTR spliceVar = currentVar; /* If all vars are larger, then insert has to happen at beginning */
        int equals;

        /* The environment block needs to be ordered */
        while(*currentVar){
            if(spliceVarLen == 0){ /* Only do strncmp if EXACT location not found */
                equals = strncmp("SPLICE_PROCNAME=", currentVar, OS_STRLEN_SPLICE_PROCNAME);
                if(equals == 0){
                    spliceVar = currentVar; /* Mark address where replacement has to happen */
                    spliceVarLen = lstrlen(spliceVar) + 1;
                } else if (equals > 0){
                    /* Mark address where insertion has to happen */
                    spliceVar = currentVar + lstrlen(currentVar) + 1;
                }
            }
            len += lstrlen(currentVar) + 1;
            currentVar = environment + len;
        }

        newLen = len - spliceVarLen
            + OS_STRLEN_SPLICE_PROCNAME + strlen(name) + 1;

        environmentCopy = (LPTCH)os_malloc(newLen + 1 /* End of array */);
        if(environmentCopy){
            size_t until = spliceVar - environment;
            size_t newSpliceVarLen;
            /* First copy up until location of spliceVar */
            memcpy(environmentCopy, environment, until);
            /* Now write the new SPLICE_PROCNAME */
            newSpliceVarLen = sprintf(environmentCopy + until, "SPLICE_PROCNAME=%s", name) + 1;
            /* Now copy tail */
            memcpy(environmentCopy + until + newSpliceVarLen, spliceVar + spliceVarLen, len - (until + spliceVarLen));
            environmentCopy[newLen] = (TCHAR)0;
        } else {
            OS_REPORT(OS_ERROR,
                        "os_procCreate", 1,
                        "Out of (heap) memory.");
        }
        FreeEnvironmentStrings(environment);
    } else {
        OS_REPORT(OS_ERROR,
                "os_procCreate", 1,
                "GetEnvironmentStrings failed, environment will be inherited from parent-process without modifications.", os_getErrno());
		environmentCopy = NULL;
    }

    if (CreateProcess(executable_file,
                      inargs,
                      NULL,                     // ProcessAttributes
                      NULL,                     // ThreadAttributes
                      procAttr->activeRedirect, // InheritHandles
                      CREATE_NO_WINDOW,         // dwCreationFlags
                      (LPVOID)environmentCopy,  // Environment
                      NULL,                     // CurrentDirectory
                      &si,
                      &process_info) == 0) {
        const int errorCode = os_getErrno ();
        OS_REPORT(OS_ERROR, "os_procCreate", 1, "Process creation for exe file %s failed with %d", executable_file, errorCode);
        os_free(inargs);
        return (ERROR_FILE_NOT_FOUND == errorCode ||
                ERROR_PATH_NOT_FOUND == errorCode ||
                ERROR_ACCESS_DENIED  == errorCode)
               ? os_resultInvalid : os_resultFail;
    }

    if(environmentCopy){
        os_free(environmentCopy);
    }
    *procId = process_info.dwProcessId;

    os_free(inargs);

/* Check to see if the client has requested "realtime" behaviour
   via the OS_SCHED_REALTIME abstraction
 */
/* #642 fix */
    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        effective_process_class = REALTIME_DEFAULT_SCHED_CLASS;
    } else {
        effective_process_class = TIMESHARE_DEFAULT_SCHED_CLASS;
    }
    if (SetPriorityClass(process_info.hProcess, effective_process_class) == 0) {
        OS_REPORT(OS_WARNING, "os_procCreate", 0, "SetPriorityClass failed with %d", os_getErrno());
    }

    effective_priority = procAttr->schedPriority;
    if (effective_process_class == REALTIME_PRIORITY_CLASS) {
        if (procAttr->schedPriority < -7) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (procAttr->schedPriority > 6) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    } else {
        if (procAttr->schedPriority < THREAD_PRIORITY_LOWEST) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (procAttr->schedPriority > THREAD_PRIORITY_HIGHEST) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    }
    if (effective_priority != THREAD_PRIORITY_NORMAL) {
        if (SetThreadPriority(process_info.hThread, procAttr->schedPriority) == 0) {
            OS_REPORT(OS_WARNING, "os_procCreate", 0, "SetThreadPriority failed with %d", os_getErrno());
        }
    }

    return os_resultSuccess;
}

#undef OS_STRLEN_SPLICE_PROCNAME

/** \brief Check the child exit status of the identified process
 *
 * \b os_procCheckStatus calls \b waitpid with flag \b WNOHANG
 * to check the status of the child process.
 * - On return of \b waitpid with result \b procId, the process
 *   has terminated and its result status is in \b status.
 * - On return of \b waitpid with result 0, the child process is
 *   not yet terminated and \b os_resultBusy is returned.
 * - On return of \b waitpid with result -1 and \b errno is \b ECHILD
 *   the identified child process is not found and \b
 *   os_resultUnavailable is returned.
 * - On any other return from \b waitpid, \b os_resultFail is returned.
 */
os_result
os_procCheckStatus(
    os_procId procId,
    int32_t *status)
{
    DWORD tr;
    int err;
    HANDLE procHandle;
    register int callstatus;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }
    assert(status != NULL);
    procHandle = os_procIdToHandle(procId);

    callstatus = GetExitCodeProcess(procHandle, &tr);
    if (callstatus == 0) {
        err = os_getErrno();
        CloseHandle (procHandle);
        if (err == ERROR_INVALID_HANDLE) {
            return os_resultUnavailable;
        }

        OS_REPORT(OS_INFO, "os_procCheckStatus", 0, "os_procCheckStatus GetExitCodeProcess Failed %d", err);
        return os_resultFail;
    }
    CloseHandle (procHandle);

    if (tr == STILL_ACTIVE) {
        return os_resultBusy;
    }

    *status = (int32_t)tr;
    return os_resultSuccess;
}

/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b 0
 */
_Post_satisfies_(procAttr->schedClass == OS_SCHED_DEFAULT)
_Post_satisfies_(procAttr->schedPriority == THREAD_PRIORITY_NORMAL)
_Post_satisfies_(procAttr->activeRedirect == 0)
void
os_procAttrInit(
	_Out_ os_procAttr *procAttr)
{
    assert(procAttr != NULL);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = THREAD_PRIORITY_NORMAL;
    procAttr->activeRedirect = 0;
}

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
   /* returns a pseudo HANDLE to process, no need to close it */
   return GetProcessId (GetCurrentProcess());
}
HANDLE
os_procIdToHandle(os_procId procId)
{
    return OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, procId);
}
os_procId
os_handleToProcId(HANDLE procHandle)
{
    return GetProcessId(procHandle);
}

/** \brief Figure out the identity of the current process
 *
 * Possible Results:
 * - returns the actual length of procIdentity
 *
 * Postcondition:
 * - \b procIdentity is ""
 *     the process identity could not be determined
 * - \b procIdentity is "<decimal number>"
 *     only the process numeric identity could be determined
 * - \b procIdentity is "name <pid>"
 *     the process name and numeric identity could be determined
 *
 * \b procIdentity will not be filled beyond the specified \b procIdentitySize
 */
#define _OS_PROC_PROCES_NAME_LEN (512)
int32_t
os_procFigureIdentity(
    char *procIdentity,
    unsigned procIdentitySize)
{
    int size = 0;
    char process_name[_OS_PROC_PROCES_NAME_LEN];

    size = os_procGetProcessName(process_name,_OS_PROC_PROCES_NAME_LEN);

    if (size > 0) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d>",
                process_name, os_procIdSelf());
    }
    else {
        /* No processname could be determined, so default to PID */
        size = snprintf(procIdentity, procIdentitySize, "<%d>",
                os_procIdSelf());
    }

    return (int32_t)size;
}

int32_t
os_procGetProcessName(
    char *procName,
    unsigned procNameSize)
{
    int size = 0;
    char *process_name = NULL;
    char *process_env_name;
    char *exec = NULL;

    if (processName == NULL) {
        /* free is done in os_processModuleExit() */
        processName = (char*) os_malloc(_OS_PROC_PROCES_NAME_LEN);
        *processName = '\0';
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            size = snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s",process_env_name);
        } else {
            char *tmp;
            DWORD nSize;
            DWORD allocated = 0;
            do {
                   /* While procNameSize could be used (since the caller cannot
                    * store more data anyway, it is not used. This way the amount that
                    * needs to be allocated to get the full-name can be determined. */
                   allocated++;
                   tmp = (char*) os_realloc(process_name, allocated * _OS_PROC_PROCES_NAME_LEN);
                   if(tmp){
                       process_name = tmp;

                       /* First parameter NULL retrieves module-name of executable */
                       nSize = GetModuleFileNameA (NULL, process_name, allocated * _OS_PROC_PROCES_NAME_LEN);
                   } else {
                       /* Memory-claim denied, revert to default */
                       size = 0;
                       if(process_name){
                           os_free(process_name);
                           process_name = NULL; /* Will break loop */
                       }
                   }

               /* process_name will only be guaranteed to be NULL-terminated if nSize <
                * (allocated * _OS_PROC_PROCES_NAME_LEN), so continue until that's true */
               } while (process_name && nSize >= (allocated * _OS_PROC_PROCES_NAME_LEN));

            if(process_name){
                exec = strrchr(process_name,'\\');
                if (exec) {
                    /* skip all before the last '\' */
                    exec++;
                    snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s", exec);
                } else {
                    snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s", process_name);
                }
                os_free(process_name);
            }
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return (int32_t)size;
}
#undef _OS_PROC_PROCES_NAME_LEN

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT in
 *   case class could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    os_schedClass cl = OS_SCHED_TIMESHARE;
    DWORD prioClass;

    prioClass = GetPriorityClass(GetCurrentProcess());
    if (prioClass == REALTIME_PRIORITY_CLASS) {
        cl = OS_SCHED_REALTIME;
    }

    return cl;
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform dependent valid priority
 */
int32_t
os_procAttrGetPriority(void)
{
    return GetThreadPriority(GetCurrentThread());
}
