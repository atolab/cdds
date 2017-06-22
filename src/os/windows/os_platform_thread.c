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
/** \file os/win32/code/os_thread.c
 *  \brief WIN32 thread management
 *
 * Implements thread management for WIN32
 */

#include "os/os.h"

#include <assert.h>

typedef struct {
    char *threadName;
    void *arguments;
    os_threadRoutine startRoutine;
} os_threadContext;

static DWORD tlsIndex;
static os_threadHook os_threadCBs;

static int
os_threadStartCallback(
    os_threadId id,
    void *arg)
{
    return 0;
}

static int
os_threadStopCallback(
    os_threadId id,
    void *arg)
{
    return 0;
}

static void
os_threadHookInit(void)
{
    os_threadCBs.startCb = os_threadStartCallback;
    os_threadCBs.startArg = NULL;
    os_threadCBs.stopCb = os_threadStopCallback;
    os_threadCBs.stopArg = NULL;
}

static void
os_threadHookExit(void)
{
    return;
}

static os_result
os_threadMemInit(void)
{
    void **tlsMemArray;
    BOOL result;

    tlsMemArray = os_malloc (sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
    memset(tlsMemArray, 0, sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
    result = TlsSetValue(tlsIndex, tlsMemArray);
    if (!result) {
        //OS_INIT_FAIL("os_threadMemInit: failed to set TLS");
        goto err_setTls;
    }
    return os_resultSuccess;

err_setTls:
    os_free(tlsMemArray);
    return os_resultFail;
}

static void
os_threadMemExit(void)
{
    void **tlsMemArray;
    int i;

    tlsMemArray = (void **)TlsGetValue(tlsIndex);
    if (tlsMemArray != NULL) {
/*The compiler doesn't realize that tlsMemArray has always size OS_THREAD_MEM_ARRAY_SIZE. */
#pragma warning(push)
#pragma warning(disable: 6001)
        for (i = 0; i < OS_THREAD_MEM_ARRAY_SIZE; i++) {
            if (tlsMemArray[i] != NULL) {
                os_free(tlsMemArray[i]);
            }
        }
#pragma warning(pop)
        os_free(tlsMemArray);
        TlsSetValue(tlsIndex, NULL);
    }
}

/** \brief Initialize the thread module
 *
 * \b os_threadModuleInit initializes the thread module for the
 *    calling process
 */
os_result
os_threadModuleInit(void)
{
    if ((tlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
        //OS_INIT_FAIL("os_threadModuleInit: could not allocate thread-local memory (System Error Code: %i)", os_getErrno());
        goto err_tlsAllocFail;
    }
    os_threadHookInit();
    return os_resultSuccess;

err_tlsAllocFail:
    return os_resultFail;
}

/** \brief Deinitialize the thread module
 *
 * \b os_threadModuleExit deinitializes the thread module for the
 *    calling process
 */
void
os_threadModuleExit(void)
{
   LPVOID data = TlsGetValue(tlsIndex);

   printf("*** %s - start, tlsIndex=%d, data=%p, errno=%i\n", OS_FUNCTION, tlsIndex, data, os_getErrno());
   if (data != NULL) {
	  printf("*** %s - 1\n", OS_FUNCTION);
      LocalFree((HLOCAL) data);
   }
   printf("*** %s - 2\n", OS_FUNCTION);
   TlsFree(tlsIndex);
   printf("*** %s - 3\n", OS_FUNCTION);
   os_threadHookExit();
   printf("*** %s - end\n", OS_FUNCTION);

}

os_result
os_threadModuleSetHook(
    os_threadHook *hook,
    os_threadHook *oldHook)
{
    os_result result;
    os_threadHook oh;

    result = os_resultFail;
    oh = os_threadCBs;

    if (hook) {
        if (hook->startCb) {
            os_threadCBs.startCb = hook->startCb;
            os_threadCBs.startArg = hook->startArg;
        } else {
            os_threadCBs.startCb = os_threadStartCallback;
            os_threadCBs.startArg = NULL;
        }
        if (hook->stopCb) {
            os_threadCBs.stopCb = hook->stopCb;
            os_threadCBs.stopArg = hook->stopArg;
        } else {
            os_threadCBs.stopCb = os_threadStopCallback;
            os_threadCBs.stopArg = NULL;
        }

        if (oldHook) {
            *oldHook = oh;
        }
    }

    return result;
}

const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   /** Must be 0x1000. */
   DWORD dwType;
   /** Pointer to name (in user addr space). */
   LPCSTR szName;
   /** Thread ID (-1=caller thread). */
   DWORD dwThreadID;
   /**  Reserved for future use, must be zero. */
   DWORD dwFlags;
} THREADNAME_INFO;
#pragma pack(pop)

/**
* Usage: os_threadSetThreadName (-1, "MainThread");
* @pre ::
* @see http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
* @param dwThreadID The thread ID that is to be named, -1 for 'self'
* @param threadName The name to apply.
*/
void os_threadSetThreadName( DWORD dwThreadID, char* threadName)
{
   char* tssThreadName;
#ifndef WINCE /* When we merge the code, this first bit won't work there */
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

/* Empty try/except that catches everything is done on purpose to set the
 * thread name. This code equals the official example on msdn, including
 * the warning suppressions. */
#pragma warning(push)
#pragma warning(disable: 6320 6322)
   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
#pragma warning(pop)
#endif /* No reason why the restshouldn't though */

    tssThreadName = (char *)os_threadMemGet(OS_THREAD_NAME);
    if (tssThreadName == NULL)
    {
        tssThreadName = (char *)os_threadMemMalloc(OS_THREAD_NAME, (strlen(threadName) + 1));
        strcpy(tssThreadName, threadName);
    }
}

/** \brief Wrap thread start routine
 *
 * \b os_startRoutineWrapper wraps a threads starting routine.
 * before calling the user routine. It tries to set a thread name
 * that will be visible if the process is running under the MS
 * debugger.
 */
static uint32_t
os_startRoutineWrapper(
    _In_ _Post_invalid_ void *threadContext)
{
    os_threadContext *context = threadContext;
    uint32_t resultValue = 0;
    os_threadId id;

    /* allocate an array to store thread private memory references */
    os_threadMemInit();

    /* Set a thread name that will take effect if the process is running under a debugger */
    os_threadSetThreadName(-1, context->threadName);

    id.threadId = GetCurrentThreadId();
    id.handle = GetCurrentThread();
    /* Call the start callback */
    if (os_threadCBs.startCb(id, os_threadCBs.startArg) == 0) {
        /* Call the user routine */
        resultValue = context->startRoutine(context->arguments);
    }

    os_threadCBs.stopCb(id, os_threadCBs.stopArg);

    os_report_stack_free();
    os_reportClearApiInfo();

    /* Free the thread context resources, arguments is responsibility */
    /* for the caller of os_threadCreate                                */
    os_free (context->threadName);
    os_free (context);

    /* deallocate the array to store thread private memory references */
    os_threadMemExit ();

    /* return the result of the user routine */
    return resultValue;
}

/** \brief Create a new thread
 *
 * \b os_threadCreate creates a thread by calling \b CreateThread.
 */
os_result
os_threadCreate(
    _Out_ os_threadId *threadId,
    _In_z_ const char *name,
    _In_ const os_threadAttr *threadAttr,
    _In_ os_threadRoutine start_routine,
    _In_opt_ void *arg)
{
    HANDLE threadHandle;
    DWORD threadIdent;
    os_threadContext *threadContext;

    int32_t effective_priority;

    assert(threadId != NULL);
    assert(name != NULL);
    assert(threadAttr != NULL);
    assert(start_routine != NULL);

    /* Take over the thread context: name, start routine and argument */
    threadContext = os_malloc(sizeof (*threadContext));
    threadContext->threadName = os_strdup(name);
    threadContext->startRoutine = start_routine;
    threadContext->arguments = arg;
    threadHandle = CreateThread(NULL,
        (SIZE_T)threadAttr->stackSize,
        (LPTHREAD_START_ROUTINE)os_startRoutineWrapper,
        (LPVOID)threadContext,
        (DWORD)0, &threadIdent);
    if (threadHandle == 0) {
        OS_REPORT(OS_WARNING, "os_threadCreate", os_getErrno(), "Failed with System Error Code: %i\n", os_getErrno ());
        return os_resultFail;
    }

    fflush(stdout);

    threadId->handle   = threadHandle;
    threadId->threadId = threadIdent;

    /*  #642 fix (JCM)
     *  Windows thread priorities are in the range below :
    -15 : THREAD_PRIORITY_IDLE
    -2  : THREAD_PRIORITY_LOWEST
    -1  : THREAD_PRIORITY_BELOW_NORMAL
     0  : THREAD_PRIORITY_NORMAL
     1  : THREAD_PRIORITY_ABOVE_NORMAL
     2  : THREAD_PRIORITY_HIGHEST
    15  : THREAD_PRIORITY_TIME_CRITICAL
    For realtime threads additional values are allowed : */

    /* PROCESS_QUERY_INFORMATION rights required
     * to call GetPriorityClass
     * Ensure that priorities are effectively in the allowed range depending
     * on GetPriorityClass result */
    effective_priority = threadAttr->schedPriority;
    if (GetPriorityClass(GetCurrentProcess()) == REALTIME_PRIORITY_CLASS) {
        if (threadAttr->schedPriority < -7) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (threadAttr->schedPriority > 6) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    } else {
        if (threadAttr->schedPriority < THREAD_PRIORITY_LOWEST) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (threadAttr->schedPriority > THREAD_PRIORITY_HIGHEST) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    }
    if (SetThreadPriority (threadHandle, effective_priority) == 0) {
        OS_REPORT(OS_INFO, "os_threadCreate", os_getErrno(), "SetThreadPriority failed with %i", os_getErrno());
    }

   /* ES: dds2086: Close handle should not be performed here. Instead the handle
    * should not be closed until the os_threadWaitExit(...) call is called.
    * CloseHandle (threadHandle);
    */
   return os_resultSuccess;
}

/** \brief Return the integer representation of the given thread ID
 *
 * Possible Results:
 * - returns the integer representation of the given thread ID
 */
uintmax_t
os_threadIdToInteger(os_threadId id)
{
   return id.threadId;
}

/** \brief Return the thread ID of the calling thread
 *
 * \b os_threadIdSelf determines the own thread ID by
 * calling \b GetCurrentThreadId ().
 */
os_threadId
os_threadIdSelf(
    void)
{
   os_threadId id;
   id.threadId = GetCurrentThreadId();
   id.handle = GetCurrentThread();   /* pseudo HANDLE, no need to close it */

   return id;
}

/** \brief Wait for the termination of the identified thread
 *
 * \b os_threadWaitExit wait for the termination of the
 * thread \b threadId by calling \b pthread_join. The return
 * value of the thread is passed via \b thread_result.
 */
os_result
os_threadWaitExit(
    _In_ os_threadId threadId,
    _Out_opt_ uint32_t *thread_result)
{
    DWORD tr;
    DWORD err;
    DWORD waitres;
    BOOL status;

    if(threadId.handle == NULL){
        //OS_DEBUG("os_threadWaitExit", "Parameter threadId is null");
        return os_resultFail;
    }

    waitres = WaitForSingleObject(threadId.handle, INFINITE);
    if (waitres != WAIT_OBJECT_0) {
        err = os_getErrno();
        //OS_DEBUG_1("os_threadWaitExit", "WaitForSingleObject Failed %d", err);
        return os_resultFail;
    }

    status = GetExitCodeThread(threadId.handle, &tr);
    if (!status) {
       err = os_getErrno();
       //OS_DEBUG_1("os_threadWaitExit", "GetExitCodeThread Failed %d", err);
       return os_resultFail;
    }

    assert(tr != STILL_ACTIVE);
    if (thread_result) {
        *thread_result = tr;
    }
    CloseHandle(threadId.handle);

    return os_resultSuccess;
}

/** \brief Figure out the identity of the current thread
 *
 * Possible Results:
 * - returns the actual length of threadIdentity
 */
int
os_threadFigureIdentity(
    char *threadIdentity,
    uint32_t threadIdentitySize)
{
   int size;
   char* threadName;

   threadName = (char *)os_threadMemGet(OS_THREAD_NAME);
   if (threadName != NULL) {
       size = snprintf (threadIdentity, threadIdentitySize, "%s 0x%"PRIx32, threadName, GetCurrentThreadId());
   } else {
       size = snprintf (threadIdentity, threadIdentitySize, "0x%"PRIx32, GetCurrentThreadId());
   }

   return size;
}

int
os_threadGetThreadName(
    char *buffer,
    uint32_t length)
{
    char *name;

    assert (buffer != NULL);

    if ((name = os_threadMemGet(OS_THREAD_NAME)) == NULL) {
        name = "";
    }

    return snprintf (buffer, length, "%s", name);
}

/** \brief Allocate thread private memory
 *
 * Allocate heap memory of the specified \b size and
 * relate it to the thread by storing the memory
 * reference in an thread specific reference array
 * indexed by \b index. If the indexed thread reference
 * array location already contains a reference, no
 * memory will be allocated and NULL is returned.
 *
 * Possible Results:
 * - returns NULL if
 *     index < 0 || index >= OS_THREAD_MEM_ARRAY_SIZE
 * - returns NULL if
 *     no sufficient memory is available on heap
 * - returns NULL if
 *     os_threadMemGet (index) returns != NULL
 * - returns reference to allocated heap memory
 *     of the requested size if
 *     memory is successfully allocated
 */
void *
os_threadMemMalloc(
    int32_t index,
    size_t size)
{
   void **tlsMemArray;
   void *threadMemLoc = NULL;

    printf("*** %s - start, index=%d, size=%d, OS_THREAD_MEM_ARRAY_SIZE=%d\n", OS_FUNCTION, index, (int)size, OS_THREAD_MEM_ARRAY_SIZE);
    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {

        printf("*** %s - 1. tlsIndex=%d\n", OS_FUNCTION, tlsIndex);

        tlsMemArray = (void **)TlsGetValue(tlsIndex);

        /* From Windows tlsGetValue documentation:
         * The data stored in a TLS slot can have a value of 0 because it still has its
         * initial value or because the thread called the TlsSetValue function with 0.
         * Therefore, if the return value is 0, you must check whether GetLastError
         * returns ERROR_SUCCESS before determining that the function has failed.
         * If GetLastError returns ERROR_SUCCESS, then the function has succeeded and
         * the data stored in the TLS slot is 0. Otherwise, the function has failed.
         */

        if ((tlsMemArray == NULL) && (os_getErrno() != ERROR_SUCCESS)) {
            printf("*** %s - 2. tlsMemArray=NULL, errno != ERROR_SUCCESS, errno=%i\n", OS_FUNCTION, os_getErrno());

        	/* TlsGetValue has failed, it may be that tlsIndex is present */
            if ((tlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
                 goto err_tlsAllocFail;
            }
            tlsMemArray = (void **)TlsGetValue(tlsIndex);
        }
#if 0
        if ((tlsMemArray == NULL) && (os_getErrno() == ERROR_SUCCESS)) {
            printf("*** %s - 3. tlsMemArray=NULL, errno=%i\n", OS_FUNCTION, os_getErrno());

            if (os_threadMemInit() == os_resultSuccess) {
                printf("*** %s - 4. os_threadMemInit hurdle taken\n", OS_FUNCTION);
                tlsMemArray = (void **)TlsGetValue(tlsIndex);
                printf("*** %s - 5. tlsMemArray=%p\n", OS_FUNCTION, tlsMemArray);
            }
        }
#endif
        if (tlsMemArray != NULL) {
            printf("*** %s - 6. tlsMemArray=%p\n", OS_FUNCTION, *tlsMemArray);
            if (tlsMemArray[index] == NULL) {
                threadMemLoc = os_malloc(size);
                tlsMemArray[index] = threadMemLoc;
                printf("*** %s - 7. threadMemLoc=%p\n", OS_FUNCTION, threadMemLoc);

            }
        }
    }
    printf("*** %s - end, threadMemAlloc = %p\n", OS_FUNCTION, threadMemLoc);
    return threadMemLoc;

err_tlsAllocFail:
    return NULL;
}

/** \brief Free thread private memory
 *
 * Free the memory referenced by the thread reference
 * array indexed location. If this reference is NULL,
 * or index is invalid, no action is taken.
 * The reference is set to NULL after freeing the
 * heap memory.
 *
 * Postcondition:
 * - os_threadMemGet (index) = NULL and allocated
 *   heap memory is freed
 */
void
os_threadMemFree(
    int32_t index)
{
    void **tlsMemArray;
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        tlsMemArray = (void **)TlsGetValue(tlsIndex);
        if ((tlsMemArray != NULL) && (os_getErrno() == ERROR_SUCCESS)) {
            threadMemLoc = tlsMemArray[index];
            if (threadMemLoc != NULL) {
                tlsMemArray[index] = NULL;
                os_free(threadMemLoc);
            }
        }
    }
    return;
}

/** \brief Get thread private memory
 *
 * Possible Results:
 * - returns NULL if
 *     No heap memory is related to the thread for
 *     the specified index
 * - returns a reference to the allocated memory
 */
void *
os_threadMemGet(
    int32_t index)
{
    void **tlsMemArray;
    void *data;

    data = NULL;
    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        tlsMemArray = TlsGetValue(tlsIndex);
        if (tlsMemArray != NULL) {
            data = tlsMemArray[index];
        }
    }

    return data;
}
