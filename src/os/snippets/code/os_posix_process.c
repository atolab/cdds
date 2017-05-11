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
/** \file os/posix/code/os_process.c
 *  \brief Posix process management
 *
 * Implements process management for POSIX
 */

#include "os/os.h"

#include <sys/types.h>
#ifndef OSPL_NO_VMEM
#include <sys/mman.h>
#endif
#ifndef PIKEOS_POSIX
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#ifndef INTEGRITY
#include <signal.h>
#endif
#include <stdio.h>
#ifndef VXWORKS_RTP
#include <pthread.h>
#endif

static char* processName = NULL;
/** \brief pointer to environment variables */
#ifdef __APPLE__
#include <crt_externs.h>
#else
extern char **environ;
#endif

/* protected functions */
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

/* public functions */

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler by calling \b atexit passing the \b function
 * to be called when the process exits.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */

#ifndef PIKEOS_POSIX
os_result
os_procAtExit(
    void (*function)(void))
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
#endif

/** \brief Returns if threads are terminated by atexit
 *
 * Return values:
 * TRUE - if threads are ungracefully terminated by atexit
 * FALSE - all other situations
 */
#ifndef PIKEOS_POSIX
bool
os_procAreThreadsTerminatedByAtExit(void)
{
    return false;
}
#endif

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
#ifndef PIKEOS_POSIX
void
os_procExit(
    os_exitStatus status)
{
    assert(status != OS_EXIT_SUCCESS || status != OS_EXIT_FAILURE);
    exit((signed int)status);
    return;
}
#endif

#if !defined VXWORKS_RTP && !defined __QNX__
/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is built from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child process then replaces the running program with the
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
    os_procAttr * procAttr,
    os_procId *procId)
{
    os_result rv = os_resultSuccess;
#if !defined INTEGRITY && !defined PIKEOS_POSIX
    pid_t pid;
    char *argv[64];
    int argc = 1;
    int go_on = 1;
    int i = 0;
    int sq_open = 0;
    int sq_close = 0;
    int dq_open = 0;
    int dq_close = 0;
    char *argin;
#if 0 /* FIXME */
    struct sched_param sched_param, sched_current;
#endif
    int sched_policy;
    char environment[512];

    assert(executable_file != NULL);
    assert(name != NULL);
    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);
    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        sched_policy = SCHED_FIFO;
    } else if (procAttr->schedClass == OS_SCHED_TIMESHARE) {
        sched_policy = SCHED_OTHER;
    } else if (procAttr->schedClass == OS_SCHED_DEFAULT) {
        sched_policy = SCHED_OTHER;
    } else {
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
                    "scheduling class outside valid range for executable: %s",
                    name);
        return os_resultInvalid;
    }
    if (procAttr->schedPriority < sched_get_priority_min (sched_policy) ||
        procAttr->schedPriority > sched_get_priority_max (sched_policy)) {

        procAttr->schedPriority = (sched_get_priority_max (sched_policy) +
                                   sched_get_priority_min(sched_policy)) / 2;
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
            "scheduling priority outside valid range for the policy reverted to valid value (%s)",
            name);
    }
    if (access(executable_file, X_OK) != 0) {
        rv = os_resultInvalid;
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
                            "Insufficient rights to execute executable %s",
                            name);
    } else {
        /* first translate the input string into an argv structured list */
        argin = os_malloc(strlen(arguments) + 1);
        strcpy(argin, arguments);
        argv[0] = os_malloc(strlen(name) + 1);
        argv[0] = strcpy(argv[0], name);
        while (go_on && (unsigned int)argc <= (sizeof(argv)/(sizeof(char *)))) {
            while (argin[i] == ' ' || argin[i] == '\t' ) {
                i++;
            }
            if (argin[i] == '\0' ) {
                break;
            } else if (argin[i] == '\'') {
                if (sq_open == sq_close) {
                    sq_open++;
                    argv[argc] = &argin[i];
                } else {
                    sq_close++;
                }
                i++;
            } else if (argin[i] == '\"') {
                if (dq_open == dq_close) {
                    dq_open++;
                } else {
                    dq_close++;
                }
                i++;
            } else {
                argv[argc] = &argin[i];
                argc++;
                while ((argin[i] != ' ' && argin[i] != '\t') ||
                       (sq_open != sq_close) ||
                       (dq_open != dq_close)) {
                    if (argin[i] == '\0') {
                        go_on = 0;
                        break;
                    } else if (argin[i] == '\'') {
                        sq_close++;
                        if ((sq_open == sq_close) && (dq_open == dq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else if (argin[i] == '\"') {
                        dq_close++;
                        if ((dq_open == dq_close) && (sq_open == sq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else {
                        i++;
                    }
                }
                argin[i] = '\0';
                i++;
            }
        }
        argv [argc] = NULL;
        if ((pid = fork()) == -1) {
            OS_REPORT(OS_WARNING, "os_procCreate", 1,
                        "fork failed with error %d (%s, %s)", os_getErrno(), executable_file, name);
            rv = os_resultFail;
        } else if (pid == 0) {
            /* child process */
#if 0 /* FIXME */
            if (procAttr->schedClass == OS_SCHED_REALTIME) {
                sched_param.sched_priority = procAttr->schedPriority;

                if (sched_setscheduler(pid, SCHED_FIFO, &sched_param) == -1) {
                  char msg[64];
                  os_strerror_r(os_getErrno(), msg, sizeof(msg));
                    OS_REPORT(OS_WARNING, "os_procCreate", 1,
                            "sched_setscheduler failed with error %d (%s) for process '%s'",
                            os_getErrno(), msg, name);
                }
            } else {
                    sched_getparam (0, &sched_current);
                    if (sched_current.sched_priority != procAttr->schedPriority) {
                        sched_param.sched_priority = procAttr->schedPriority;
                        if (sched_setscheduler(pid, SCHED_OTHER, &sched_param) == -1) {
                            OS_REPORT(OS_WARNING, "os_procCreate", 1,
                                        "sched_setscheduler failed with error %d (%s). Requested priority was %d, current is %d",
                                        os_getErrno(), name, procAttr->schedPriority,
                                        sched_current.sched_priority);
                    }
                }
            }
            if (getuid() == 0) {
                /* first set the gid */
                if (procAttr->userCred.gid) {
                    setgid(procAttr->userCred.gid);
                }
                /* then set the uid */
                if (procAttr->userCred.uid) {
                    setuid(procAttr->userCred.uid);
                }
            }
#endif
            /* Set the process name via environment variable SPLICE_PROCNAME */
            snprintf(environment, sizeof(environment), "SPLICE_PROCNAME=%s", name);
            putenv(environment);
            /* exec executable file */
#if __APPLE__
            char **environ = *_NSGetEnviron ();
#endif
            execve(executable_file, argv, environ); /* Will only return on error */
            OS_REPORT(OS_WARNING, "os_procCreate", 1, "execve failed with error %d (%s)", os_getErrno(), executable_file);
            /* if executing this, something has gone wrong */
            rv = os_resultFail; /* Just to fool QAC */
            os_procExit(OS_EXIT_FAILURE);
        } else {
            /* parent process */
            *procId = pid;
            rv = os_resultSuccess;
        }
        os_free(argv[0]);
        os_free(argin);
    }
#endif

    return rv;
}
#endif

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
#if !defined INTEGRITY && !defined PIKEOS_POSIX
os_result
os_procCheckStatus(
    os_procId procId,
    int32_t *status)
{
    os_procId result;
    os_result rv;
    int les;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }

    result = waitpid(procId, &les, WNOHANG);
    if (result == procId) {
        if (WIFEXITED(les)) {
            *status = WEXITSTATUS(les);
        } else {
            *status = OS_EXIT_FAILURE;
        }
        rv = os_resultSuccess;
    } else if (result == 0) {
        rv = os_resultBusy;
    } else if ((result == -1) && (os_getErrno() == ECHILD)) {
        rv = os_resultUnavailable;
    } else {
        rv = os_resultFail;
    }
    return rv;
}
#endif

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
    return (os_procId)getpid();
}

/* _OS_PROCESS_DEFAULT_CMDLINE_LEN_ is defined as
 * strlen("/proc/<max_pid>/cmdline" + 1, max_pid = 32768 on Linux, so a reason-
 * able default is 20 */
#define _OS_PROCESS_DEFAULT_CMDLINE_LEN_ (20)
#define _OS_PROCESS_PROCFS_PATH_FMT_     "/proc/%d/cmdline"
#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)

/** \brief Figure out the identity of the current process
 *
 * os_procFigureIdentity determines the numeric, and if possible named
 * identity of a process. It will first check if the environment variable
 * SPLICE_PROCNAME is set (which is always the case if the process is started
 * via os_procCreate). If so, that value will be returned. Otherwise it will be
 * attempted to determine the commandline which started the process through the
 * procfs. If that fails, the PID will be returned.
 *
 * \param procIdentity  Pointer to a char-buffer to which the result can be
 *                      written. If a name could be resolved, the result will
 *                      have the format "name <PID>". Otherwise it will just
 *                      be "<PID>".
 * \param procIdentitySize Size of the buffer pointed to by procIdentitySize
 * \return same as snprintf returns
 */
int32_t
os_procFigureIdentity(
    char *procIdentity,
    uint32_t procIdentitySize)
{
    int size = 0;
    char process_name[_OS_PROCESS_DEFAULT_NAME_LEN_];

    size = os_procGetProcessName(process_name,_OS_PROCESS_DEFAULT_NAME_LEN_);
    if (size > 0) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d>",
                        process_name, os_procIdSelf());
    }
    else {
         /* No processname could be determined, so default to PID */
         size = snprintf(procIdentity, procIdentitySize, "<%d>",
                 os_procIdSelf());
     }
    return size;
}

#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_PROCFS_PATH_FMT_
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
#ifndef INTEGRITY
#if !defined (VXWORKS_RTP) && !defined (PIKEOS_POSIX)

/* os_procServiceDestroy will need an alternative for VXWORKS when ospl
 * extended to include VXWORKS - function not called by 'old' (vxworks) ospl
 */

/** \brief Terminate process \b pid by progressive use of stronger kill
 * signals followed by \b checkcount cycles of status tests to confirm
 * action of kill signal. Checkcount cycles are 100ms long.
 * Blocking state is passed via \b isblocking
 *
 * \b returns appropriate \b os_resultSuccess or Fail following final
 * status check
 */

os_result
os_procServiceDestroy(
    int32_t pid,
    bool isblocking,
    int32_t checkcount)
{
    os_result osr = os_resultFail;
    os_time sleepTime;
    int waitPid;
    int exitStatus;
    int32_t sleepCount = 0;
    os_procId pidgrp;

    /* Send the TERM signal to the spliced, which should cause the spliced  and
     * its own services to terminate cleanly.  To check spliced has terminated
     * call the waitForProcess function (up to the serviceTerminatePeriod limit).
     * If it has not terminated, we take more evasive action by sending the
     * KILL  signal and then perform the same test.  The results of that are then
     * reported.
     */

    pidgrp = getpgid(pid); /* in case needed by killgrp and avoids reading from file */

    (void) kill (pid, SIGTERM);

    /* wait for process */

    /* use 10th of a second sleep periods */
    sleepTime.tv_sec  = 0;
    sleepTime.tv_nsec = 100000000;
    sleepCount = checkcount;

    while (sleepCount > 0)
    {
        if (isblocking)
        {
            /* Unless it has been waited for with wait or waitpid, a child process
             * that terminates becomes a "zombie".   That is the case in blocking
             * mode : we cannot just do the kill (pid, 0) test, since the 0 signal
             * will always be delivered to the zombie.
             * Instead, we have to do the waitpid test, and if it returns successfully,
             * record its exit status.
             */
            waitPid = waitpid(pid, &exitStatus, WNOHANG);
            if(waitPid > 0)
            {
                /* the process has now terminated, obtain its exit status */

                OS_REPORT(OS_INFO, "setExitStatus", 0,
                    "Process spliced <%d> %s %d",
                    pid,
                    WIFEXITED(exitStatus) ? "exited normally, exit status:" : (
                        WIFSIGNALED(exitStatus) ? "terminated due to an unhandled signal:" : "stopped with an unknown status"),
                        WIFEXITED(exitStatus) ? WEXITSTATUS(exitStatus) : (
                        WIFSIGNALED(exitStatus) ? WTERMSIG(exitStatus) : -1));

                osr = os_resultSuccess;
                break;
            }
            else if (waitPid == -1 && os_getErrno() == ECHILD)
            {
                /* already gone, no exit status information available */
                osr = os_resultSuccess;
                break;
            }
        }
        else if (kill(pid, 0) == -1)
        {
            /* unable to send signal to the process so it must have terminated */
            osr = os_resultSuccess;
            break;
        }

        if (sleepCount % 10 == 0)
        {
           /* Only print 1 . per second */
            printf (".");
            fflush(stdout);
        }
        os_nanoSleep(sleepTime);    /* shutdown attempt interval */
        sleepCount--;

    }

    /* since (unlike windows) associated processes are not killed explictly
     * need to attempt killgrp before escalating kill signal */

    if(killpg(pidgrp, 0) == 0)     /* is this ok for blocking? */
    {
        int killResult = killpg (pidgrp, SIGKILL);
        OS_REPORT(OS_WARNING, "removeProcesses", 0,
                "The OpenSplice Domain service <%d> did not terminate in time, "
                "sent KILL signal to process group (ret %d)",
                pidgrp, killResult);
        if (killResult == 0)
        {
            osr = os_resultSuccess;
        }
        else
        {
            osr = os_resultFail;
        }
    }

    if (osr != os_resultSuccess)
    {
        (void) kill (pid, SIGKILL);   /* escalate kill signal */

        /* wait for process */

        sleepCount = checkcount;
        osr = os_resultFail;

        while (sleepCount > 0)
        {
            if (isblocking)
            {
                /* Unless it has been waited for with wait or waitpid, a child process
                 * that terminates becomes a "zombie".   That is the case in blocking
                 * mode : we cannot just do the kill (pid, 0) test, since the 0 signal
                 * will always be delivered to the zombie.
                 * Instead, we have to do the waitpid test, and if it returns successfully,
                 * record its exit status.
                 */
                waitPid = waitpid(pid, &exitStatus, WNOHANG);
                if(waitPid > 0)
                {
                    /* the process has now terminated, obtain its exit status */

                    OS_REPORT(OS_INFO, "setExitStatus", 0,
                        "Process spliced <%d> %s %d",
                        pid,
                        WIFEXITED(exitStatus) ? "exited normally, exit status:" : (
                            WIFSIGNALED(exitStatus) ? "terminated due to an unhandled signal:" : "stopped with an unknown status"),
                            WIFEXITED(exitStatus) ? WEXITSTATUS(exitStatus) : (
                            WIFSIGNALED(exitStatus) ? WTERMSIG(exitStatus) : -1));

                    osr = os_resultSuccess;
                    break;
                }
                else if (waitPid == -1 && os_getErrno() == ECHILD)
                {
                    /* already gone, no exit status information available */
                    osr = os_resultSuccess;
                    break;
                }
            }
            else if (kill(pid, 0) == -1)
            {
                /* unable to send signal to the process so it must have terminated */
                osr = os_resultSuccess;
                break;
            }

            if (sleepCount % 10 == 0)
            {
               /* Only print 1 . per second */
                printf (".");
                fflush(stdout);
            }

            os_nanoSleep(sleepTime);  /* shutdown attempt interval */
            sleepCount--;

        }

        if(killpg(pidgrp, 0) == 0)     /* is this ok for blocking? */
        {
            int killResult = killpg (pidgrp, SIGKILL);
            OS_REPORT(OS_WARNING, "removeProcesses", 0,
                    "The OpenSplice Domain service <%d> did not terminate in time, "
                    "sent KILL signal to process group (ret %d)",
                    pidgrp, killResult);
            if (killResult == 0)
            {
                osr = os_resultSuccess;
            }
            else
            {
                osr = os_resultFail;
            }
        }
    }

    printf ("\n");
    fflush(stdout);

    return osr;
}

#endif
#endif

/** \brief Send a signal to the identified process
 *
 * \b os_procDestroy sends the dignal \b signal
 * to process \b procId by calling \b kill with
 * the same parameters.
 *
 * - If \b kill returns a value != -1, \b os_resultSuccess is
 *   returned.
 * - If \b kill returns with -1 and \b errno is set to \b EINVAL,
 *   \b os_resultInvalid is returned to indicate an invalid
 *   parameter.
 * - If \b kill returns with -1 and \b errno is set to \b ESRCH,
 *   \b os_resultUnavailable is returned to indicate that
 *   the identified process is not found.
 * - On any other return from \b kill, \b os_resultFail is returned.
 */
#ifndef INTEGRITY
os_result
os_procDestroy(
    os_procId procId,
    int32_t signal)
{
    os_result rv;
    int err;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }

    if (kill(procId, signal) == -1) {
        err = os_getErrno ();
        if (err == EINVAL) {
            rv = os_resultInvalid;
        } else if (err == ESRCH) {
            rv = os_resultUnavailable;
        } else {
            rv = os_resultFail;
        }
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}


#if !defined VXWORKS_RTP && !defined OS_RTEMS_DEFS_H  && !defined PIKEOS_POSIX
/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT if
 *   the class effective could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
#if 0
    os_schedClass class;
    int policy;

    policy = sched_getscheduler(getpid());
    switch (policy)
    {
       case SCHED_FIFO:
       case SCHED_RR:
          class = OS_SCHED_REALTIME;
          break;
       case SCHED_OTHER:
          class = OS_SCHED_TIMESHARE;
          break;
       case -1:
          OS_REPORT(OS_WARNING, "os_procAttrGetClass", 1,
                    "sched_getscheduler failed with error %d", os_getErrno());
          class = OS_SCHED_DEFAULT;
          break;
       default:
          OS_REPORT(OS_WARNING, "os_procAttrGetClass", 1,
                      "sched_getscheduler unexpected return value %d", policy);
          class = OS_SCHED_DEFAULT;
          break;
    }
    return class;
#else
  return OS_SCHED_DEFAULT;
#endif
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform and scheduling class dependent valid priority
 */
int32_t
os_procAttrGetPriority(void)
{
#if 0
    struct sched_param param;

    param.sched_priority = 0;
    if (sched_getparam(getpid(), &param) == -1)
    {
       OS_REPORT (OS_WARNING, "os_procAttrGetPriority", 1,
                    "sched_getparam failed with error %d", os_getErrno());
    }
    return param.sched_priority;
#else
  return 0;
#endif
}
#endif
#endif

#if !defined ( OSPL_NO_VMEM ) && !defined(_POSIX_MEMLOCK)
#error "Error: the posix implementation on this platform does not support page locking!"
#endif

os_result
os_procMLockAll(
    unsigned flags)
{
#ifndef OSPL_NO_VMEM
    int f;
    int r;
    os_result result;

    f = 0;
#if 0 /* FIXME */
    if (flags & OS_MEMLOCK_CURRENT) {
        f |= MCL_CURRENT;
    }
    if (flags & OS_MEMLOCK_FUTURE) {
        f |= MCL_FUTURE;
    }
#endif

    r = mlockall(f);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (os_getErrno() == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLockAll",
                0, "Current process has insufficient privilege");
        } else {
            if (os_getErrno() == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLockAll",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }

    return result;
#else
    return os_resultSuccess;
#endif
}

#if !defined( OSPL_NO_VMEM ) && ! defined( _POSIX_MEMLOCK_RANGE )
#error "Error: the posix implementation on this platform does not support page range locking!"
#endif

os_result
os_procMLock(
    const void *addr,
    uintptr_t length)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = (int) mlock(addr, (size_t)length);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (os_getErrno() == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLock",
                0, "Current process has insufficient privilege");
        } else {
            if (os_getErrno() == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLock",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}

os_result
os_procMUnlock(
    const void *addr,
    uintptr_t length)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = (int)munlock(addr, (size_t)length);
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        if (os_getErrno() == EPERM) {
            OS_REPORT(OS_ERROR, "os_procMLock",
                0, "Current process has insufficient privilege");
        } else {
            if (os_getErrno() == ENOMEM) {
                OS_REPORT(OS_ERROR, "os_procMLock",
                    0, "Current process has non-zero RLIMIT_MEMLOCK");
            }
        }
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}

/** \brief  re-enable paging for calling process.
 *
 *  Enables paging for all pages mapped into the address space ofi
 *  the calling process.
 */
os_result
os_procMUnlockAll(void)
{
#ifndef OSPL_NO_VMEM
    int r;
    os_result result;

    r = munlockall();
    if (r == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
#else
    return os_resultSuccess;
#endif
}
