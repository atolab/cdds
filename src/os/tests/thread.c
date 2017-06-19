#include "dds.h"
#include "CUnit/Runner.h"
#include "os/os.h"
#include "assert.h"

#define ENABLE_TRACING 0

char          arg_result[30];
int           threadCalled;
int           startCallbackCount;
int           stopCallbackCount;
void          *returnval;

static void
sleepMsec(int32_t msec)
{
    os_time delay;

    assert(msec > 0);
    assert(msec < 1000);

    delay.tv_sec = 0;
    delay.tv_nsec = msec*1000*1000;

    os_nanoSleep(delay);
}

uint32_t new_thread (_In_ void *args)
{
  (void)snprintf (arg_result, sizeof (arg_result), "%s", (char *)args);
  sleepMsec (500);
  return 0;
}

static uintmax_t thread_id_from_thread;

uint32_t threadId_thread (_In_opt_ void *args)
{
  if (args != NULL)
  {
    sleepMsec (500);
  }
  thread_id_from_thread = os_threadIdToInteger (os_threadIdSelf ());
  return (uint32_t)thread_id_from_thread; /* Truncates potentially; just used for checking passing a result-value. */
}

uint32_t get_threadExit_thread (void *args)
{
  os_threadId * threadId = args;
  uint32_t id;
  os_result ret = os_threadWaitExit (*threadId, &id);

  return id;
}

uint32_t threadIdentity_thread (_In_ void *args)
{
  char *identity = args;
  os_threadFigureIdentity (identity, 512);
  return 0;
}

static uint32_t threadMain(_In_opt_ void *args)
{
  OS_UNUSED_ARG(args);
  threadCalled = 1;
  sleepMsec(500);
  return 0;
}

static int threadStartCallback(os_threadId id, void *arg)
{
  int *count = &startCallbackCount;
  OS_UNUSED_ARG(id);
  if (arg != NULL)
  {
      count = (int *)arg;
  }
  (*count)++;
  return 0;
}

uint32_t threadMemory_thread (_In_opt_ void *args)
{
    OS_UNUSED_ARG(args);

  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with success result for child thread */
    printf("Starting tc_os_threadMemMalloc_003\n");
  #endif
    returnval = os_threadMemMalloc (3, 100);
    CU_ASSERT (returnval != NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with fail result for child thread for index already in use */
    printf("Starting tc_os_threadMemMalloc_004\n");
  #endif
    returnval = os_threadMemMalloc (3, 100);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemGet for child thread and non allocated index */
    printf("Starting tc_os_threadMemGet_003\n");
  #endif
    returnval = os_threadMemGet (OS_THREAD_WARNING);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemGet for child thread and allocated index */
    printf("Starting tc_os_threadMemGet_004\n");
  #endif
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval != NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemFree for child thread and non allocated index */
    printf("Starting tc_os_threadMemFree_003\n");
  #endif
    os_threadMemFree (OS_THREAD_WARNING);
    returnval = os_threadMemGet (OS_THREAD_WARNING);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemFree for child thread and allocated index */
    printf("Starting tc_os_threadMemFree_004\n");
  #endif
    os_threadMemFree (3);
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval == NULL);

    return 0;
}

static int threadStopCallback(os_threadId id, void *arg)
{
    int *count = &stopCallbackCount;
    OS_UNUSED_ARG(id);
    if (arg != NULL) {
        count = (int *)arg;
    }
    (*count)++;
    return 0;
}

static int threadStartCallbackFAIL(os_threadId id, void *arg)
{
    int *count = &startCallbackCount;
    OS_UNUSED_ARG(id);
    if (arg != NULL) {
        count = (int *)arg;
    }
    (*count)++;
    return 1;
}

CUnit_Suite_Initialize(thread)
{
    int result = 0;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_thread_init\n");
  #endif

    return result;
}

CUnit_Suite_Cleanup(thread)
{
    int result = DDS_RETCODE_OK;

  #if ENABLE_TRACING
    printf("Run suite_abstraction_thread_clean\n");
  #endif
    os_osExit();
    return result;
}

CUnit_Test(thread, create)
{
    int result;
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
  #ifndef WIN32
    int           result_int;
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate with Success result\n\t\t
       (check thread creation and check argument passing) */
    printf ("Starting tc_os_threadCreate_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate1", &thread_os_threadAttr, &new_thread, "os_threadCreate");
    CU_ASSERT (result == os_resultSuccess);
    if (result == os_resultSuccess) {
  #ifdef _WRS_KERNEL
        taskDelay(1 * sysClkRateGet());
  #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            result = strcmp (arg_result, "os_threadCreate");
            CU_ASSERT (result == DDS_RETCODE_OK);
          #if ENABLE_TRACING
            if (result == DDS_RETCODE_OK)
                printf("Thread created and argument correctly passed.\n");
            else
                printf("Thread created but argument incorrectly passed.\n");
          #endif
        } else {
          #if ENABLE_TRACING
            printf("os_threadCreate success, failed os_threadWaitExit.\n");
          #endif
        }
    }

  #if ENABLE_TRACING
    /* Check os_threadCreate with Failed result */
    printf ("Starting tc_os_threadCreate_002\n");
    printf ("N.A - Failure cannot be forced\n");
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_DEFAULT */
    printf ("Starting tc_os_threadCreate_003\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    thread_os_threadAttr.schedClass = OS_SCHED_DEFAULT;
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate3", &thread_os_threadAttr, &new_thread, "os_threadCreate");
    CU_ASSERT (result == os_resultSuccess);
  #if !(defined _WRS_KERNEL || defined WIN32)
    if (result == os_resultSuccess) {
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
        CU_ASSERT (result_int == DDS_RETCODE_OK);

        if (result_int != DDS_RETCODE_OK) {
          #if ENABLE_TRACING
            printf ("pthread_getschedparam failed");
          #endif
        } else
            CU_ASSERT (policy == SCHED_OTHER);

        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }
  #endif

  /* SCHED_TIMESHARE not supported by vxworks kernel */
  #ifndef _WRS_KERNEL
  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_TIMESHARE */
    printf ("Starting tc_os_threadCreate_004\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    thread_os_threadAttr.schedClass = OS_SCHED_TIMESHARE;
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate4", &thread_os_threadAttr, &new_thread, "os_threadCreate");
    CU_ASSERT (result == os_resultSuccess);
    if (result == os_resultSuccess) {
      #ifndef WIN32
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
        CU_ASSERT (result_int == DDS_RETCODE_OK);

        if (result_int != DDS_RETCODE_OK) {
          #if ENABLE_TRACING
            printf ("pthread_getschedparam failed");
          #endif
        } else
            CU_ASSERT (policy == SCHED_OTHER);
      #endif /* WIN32 */
        result = os_threadWaitExit (thread_os_threadId, NULL);
    } else {
    #if ENABLE_TRACING
      #ifdef _WRS_KERNEL
        printf ("Not supported timeshare.\n");
      #endif
        printf ("os_threadCreate failed.\n");
    #endif
    }
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_REALTIME */
    printf ("Starting tc_os_threadCreate_005\n");
  #endif
    #if ! defined WIN32 && ! defined _WRS_KERNEL
    #ifndef VXWORKS_RTP
    if (getuid() != 0 && geteuid() != 0)
    {
      #if ENABLE_TRACING
        printf ("N.A - Need root privileges to do the test\n");
      #endif
    }
    else
    #endif /* VXWORKS_RTP */
    {
        os_threadAttrInit (&thread_os_threadAttr);
        thread_os_threadAttr.schedClass = OS_SCHED_REALTIME;
        thread_os_threadAttr.schedPriority = sched_get_priority_min (SCHED_FIFO);
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate5", &thread_os_threadAttr, &new_thread, "os_threadCreate");
        CU_ASSERT (result == os_resultSuccess);
        if (result == os_resultSuccess) {
            int policy;
            struct sched_param sched_param;

            result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
            CU_ASSERT (result_int == DDS_RETCODE_OK);

            if (result_int == DDS_RETCODE_OK) {
                CU_ASSERT (policy == SCHED_FIFO);
            } else {
              #if ENABLE_TRACING
                printf ("pthread_getschedparam failed\n");
              #endif
            }
            result = os_threadWaitExit (thread_os_threadId, NULL);
        } else {
          #if ENABLE_TRACING
            printf ("os_threadCreate failed\n");
          #endif
        }
    }
  #else /* WIN32 */
    #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows or vxworks kernel\n");
    #endif
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_TIMESHARE and min priority */
    printf ("Starting tc_os_threadCreate_006\n");
  #endif
  #ifndef WIN32
    os_threadAttrInit (&thread_os_threadAttr);
    thread_os_threadAttr.schedClass = OS_SCHED_TIMESHARE;
    #ifdef _WRS_KERNEL
      thread_os_threadAttr.schedPriority = 250;
    #else
      thread_os_threadAttr.schedPriority = sched_get_priority_min (SCHED_OTHER);
    #endif
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate6", &thread_os_threadAttr, &new_thread, "os_threadCreate");
  #ifdef _WRS_KERNEL
    #if ENABLE_TRACING
    if (result == os_resultSuccess)
        printf ("os_threadCreate failed - Expected failure from VXWORKS\n");
    else
        printf ("OS_SCHED_TIMESHARE not supported\n");
    #endif
  #else
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
        CU_ASSERT (result_int == DDS_RETCODE_OK);

        if (result_int == DDS_RETCODE_OK) {
            CU_ASSERT (sched_param.sched_priority == sched_get_priority_min (SCHED_OTHER));
        } else {
          #if ENABLE_TRACING
            printf ("pthread_getschedparam failed\n");
          #endif
        }
        result = os_threadWaitExit (thread_os_threadId, NULL);
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }
  #endif /* VXWORKS */
  #else
    #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows.\n");
    #endif
  #endif /* WIN32 */

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_TIMESHARE and max priority */
    printf ("Starting tc_os_threadCreate_007\n");
  #endif
  #ifndef WIN32
    os_threadAttrInit (&thread_os_threadAttr);
    thread_os_threadAttr.schedClass = OS_SCHED_TIMESHARE;
    #ifdef _WRS_KERNEL
        thread_os_threadAttr.schedPriority = 60;
    #else
        thread_os_threadAttr.schedPriority = sched_get_priority_max (SCHED_OTHER);
    #endif
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate7", &thread_os_threadAttr, &new_thread, "os_threadCreate");
  #ifdef _WRS_KERNEL
    #if ENABLE_TRACING
    if (result == os_resultSuccess)
        printf ("os_threadCreate failed - Expected failure from VXWORKS\n");
    else
        printf ("OS_SCHED_TIMESHARE not supported\n");
    #endif
  #else
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
        CU_ASSERT (result_int == DDS_RETCODE_OK);

        if (result_int == DDS_RETCODE_OK) {
            CU_ASSERT (sched_param.sched_priority == sched_get_priority_max (SCHED_OTHER));
        } else {
          #if ENABLE_TRACING
            printf ("pthread_getschedparam failed\n");
          #endif
        }
        result = os_threadWaitExit (thread_os_threadId, NULL);
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }
  #endif /* VXWORKS */
  #else
    #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows.\n");
    #endif
  #endif /* WIN32 */

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_REALTIME and min priority */
    printf ("Starting tc_os_threadCreate_008\n");
  #endif
  #ifndef WIN32
    #ifndef VXWORKS_RTP
    if (getuid() != 0 && geteuid() != 0)
    {
      #if ENABLE_TRACING
        printf ("N.A - Need root privileges to do the test\n");
      #endif
    }
    else
    #endif /* VXWORKS_RTP */
    {
        os_threadAttrInit (&thread_os_threadAttr);
        thread_os_threadAttr.schedClass = OS_SCHED_REALTIME;
      #ifdef _WRS_KERNEL
        thread_os_threadAttr.schedPriority = 250;
      #else
        thread_os_threadAttr.schedPriority = sched_get_priority_min (SCHED_FIFO);
      #endif
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate8", &thread_os_threadAttr, &new_thread, "os_threadCreate");
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
          #ifdef _WRS_KERNEL
            TASK_ID id;
            int pri;
            STATUS status;
            sleepSeconds (2);
            pri = 0;
            id = taskNameToId("ThreadCreate8");
            status = taskPriorityGet(id,&pri);
            CU_ASSERT (status == OK);
            CU_ASSERT (pri == 250);
          #else
            int policy;
            struct sched_param sched_param;

            result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
            CU_ASSERT (result_int == DDS_RETCODE_OK);

            if (result_int == 0) {
                CU_ASSERT (sched_param.sched_priority == sched_get_priority_min (SCHED_FIFO));
            } else {
              #if ENABLE_TRACING
                printf ("pthread_getschedparam failed.\n");
              #endif
             }
           #endif /* VXWORKS */
            result = os_threadWaitExit (thread_os_threadId, NULL);
        } else {
          #if ENABLE_TRACING
            printf ("os_threadCreate failed.\n");
          #endif
        }
    }
  #else /* WIN32 */
    #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows\n");
    #endif
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate with scheduling class SCHED_REALTIME and max priority */
    printf ("Starting tc_os_threadCreate_009\n");
  #endif
  #ifndef WIN32
    #ifndef VXWORKS_RTP
    if (getuid() != 0 && geteuid() != 0)
    {
      #if ENABLE_TRACING
        printf ("N.A - Need root privileges to do the test\n");
      #endif
    }
    else
    #endif /* VXWORKS_RTP */
    {
        os_threadAttrInit (&thread_os_threadAttr);
        thread_os_threadAttr.schedClass = OS_SCHED_REALTIME;
      #ifdef _WRS_KERNEL
        thread_os_threadAttr.schedPriority = 250;
      #else
        thread_os_threadAttr.schedPriority = sched_get_priority_max (SCHED_FIFO);
      #endif
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate9", &thread_os_threadAttr, &new_thread, "os_threadCreate");
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
          #ifdef _WRS_KERNEL
            int status;
            sleepSeconds (2);
            status = 0;
            taskPriorityGet(taskNameToId("ThreadCreate9"),&status);
            CU_ASSERT (status == 250);
          #else
            int policy;
            struct sched_param sched_param;

            result_int = pthread_getschedparam (thread_os_threadId.v, &policy, &sched_param);
            CU_ASSERT (result_int == DDS_RETCODE_OK);

            if (result_int == 0) {
                CU_ASSERT (sched_param.sched_priority == sched_get_priority_max (SCHED_FIFO));
            } else {
              #if ENABLE_TRACING
                printf ("pthread_getschedparam failed.\n");
              #endif
            }
          #endif
            result = os_threadWaitExit (thread_os_threadId, NULL);
        } else {
          #if ENABLE_TRACING
            printf ("os_threadCreate failed.\n");
          #endif
        }
    }
  #else /* WIN32 */
    #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows\n");
    #endif
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate by checking scheduling scope PTHREAD_SCOPE_SYSTEM */
    printf ("Starting tc_os_threadCreate_010\n");
    printf ("N.A - No way to queuery scope from running thread");
  #endif

  #if ENABLE_TRACING
    /* Check os_threadCreate and stacksize sttribute */
    printf ("Starting tc_os_threadCreate_011\n");
    printf ("N.A - No way to queuery scope from running thread");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_threadCreate\n");
  #endif
}

CUnit_Test(thread, idself)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    int result;
    uint32_t result_from_thread;

  #if ENABLE_TRACING
    /* Check if own thread ID is correctly provided */
    printf ("Starting tc_os_threadIdSelf_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "OwnThreadId", &thread_os_threadAttr, &threadId_thread, NULL);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, &result_from_thread);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            uintmax_t tmp_thread_os_threadId = os_threadIdToInteger(thread_os_threadId);
            CU_ASSERT (thread_id_from_thread == tmp_thread_os_threadId);
            CU_ASSERT (result_from_thread == (uint32_t)tmp_thread_os_threadId);
        } else {
          #if ENABLE_TRACING
            printf ("os_threadWaitExit failed.\n");
          #endif
        }
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }

  #if ENABLE_TRACING
    printf ("Ending tc_threadIdSelf\n");
  #endif
}

CUnit_Test(thread, join)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    int result;
    uint32_t result_from_thread;

  #if ENABLE_TRACING
    /* Wait for thread to terminate and get the return value with Success result,
       while thread is still running */
    printf ("Starting tc_os_threadWaitExit_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, &threadId_thread, (void *)1);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, &result_from_thread);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            CU_ASSERT (thread_id_from_thread == os_threadIdToInteger(thread_os_threadId));
            CU_ASSERT (result_from_thread == (uint32_t)thread_id_from_thread);
        } else {
          #if ENABLE_TRACING
            printf ("os_threadWaitExit failed.\n");
          #endif
        }
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }

  #if ENABLE_TRACING
    /* Wait for thread to terminate and get the return value with Success result,
       while thread is already terminated */
    printf ("Starting tc_os_threadWaitExit_002\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, &threadId_thread, NULL);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, &result_from_thread);
        CU_ASSERT(result == os_resultSuccess);

        if (result == os_resultSuccess) {
            CU_ASSERT (thread_id_from_thread == os_threadIdToInteger(thread_os_threadId));
            CU_ASSERT (result_from_thread == (uint32_t)thread_id_from_thread);
        } else {
         #if ENABLE_TRACING
            printf ("os_threadWaitExit failed.\n");
          #endif
        }
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }

  #if ENABLE_TRACING
    /* Get thread return value with Fail result because result is already read */
    printf ("Starting tc_os_threadWaitExit_003\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, &threadId_thread, NULL);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }

  #if ENABLE_TRACING
    /* Wait for thread to terminate and get the return value by multiple threads,
       one thread gets Success other Fail */
    printf ("Starting tc_os_threadWaitExit_004\n");
  #endif
  #ifndef WIN32
    os_threadAttrInit (&thread_os_threadAttr);
    {
        os_threadId threadWait1;
        os_result result1;

        result = os_threadCreate (&thread_os_threadId, "threadToWaitFor", &thread_os_threadAttr, &threadId_thread, (void*) 1);
        CU_ASSERT (result == os_resultSuccess);
        result1 = os_threadCreate (&threadWait1, "waitingThread1", &thread_os_threadAttr, &get_threadExit_thread, &thread_os_threadId);
        CU_ASSERT (result1 == os_resultSuccess);

        if (result == os_resultSuccess && result1 == os_resultSuccess)
        {
          #ifdef _WRS_KERNEL
            sleepSeconds(1);
          #endif
            result1 = os_threadWaitExit (threadWait1, NULL);

            if (result1 != os_resultSuccess) {
              #if ENABLE_TRACING
                printf ("os_threadWaitExit 1 failed\n");
              #endif
                CU_ASSERT (result1 == os_resultSuccess);
            }
        } else {
          #if ENABLE_TRACING
            printf ("os_threadCreate failed.\n");
          #endif
        }
    }
  #else /* WIN32 */
  #if ENABLE_TRACING
    printf ("N.A - Not tested on Windows.\n");
  #endif
  #endif

  #if ENABLE_TRACING
    /* Wait for thread to terminate and pass NULL for the
       return value address - not interrested */
    printf ("Starting tc_os_threadWaitExit_005\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, &threadId_thread, NULL);
    CU_ASSERT  (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);
      #if ENABLE_TRACING
        if (result != os_resultSuccess)
            printf ("os_threadWaitExit failed.\n");
      #endif
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }

  #if ENABLE_TRACING
    printf ("Ending tc_threadWaitExit\n");
  #endif
}

CUnit_Test(thread, figure_identity)
{
#if !defined(_WIN32)
    os_threadId thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    char threadId[512];
    char thread_name[512];
    int result;
#endif /* WIN32 */

  #if ENABLE_TRACING
    /* Figure out the identity of the thread, where it's name is known */
    printf ("Starting tc_os_threadFigureIdentity_001\n");
  #endif
  #ifdef WIN32
    /* Untested because the identifier does not contain the name on Windows */
  #else
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadFigureIdentity", &thread_os_threadAttr, &threadIdentity_thread, threadId);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            uintmax_t threadNumeric = 0;
          #ifdef _WRS_KERNEL
            int dum;
            sscanf (threadId, "%s (%d %d)", thread_name, &threadNumeric, &dum);
          #else
            sscanf (threadId, "%s 0x%"SCNxMAX, thread_name, &threadNumeric);
          #endif
            CU_ASSERT (strcmp (thread_name, "threadFigureIdentity") == 0 && threadNumeric == os_threadIdToInteger(thread_os_threadId));
        } else {
          #if ENABLE_TRACING
            printf ("os_threadWaitExit failed.\n");
          #endif
        }
    } else {
      #if ENABLE_TRACING
        printf ("os_threadCreate failed.\n");
      #endif
    }
  #endif /* WIN32 */

  #if ENABLE_TRACING
    /* Figure out the identity of the thread, where it's name is unknown */
    printf ("Starting tc_os_threadFigureIdentity_002\n");
  #endif
  #if (defined _WRS_KERNEL || defined WIN32)
    {
        char threadId[512];
        int threadNumeric;

        os_threadFigureIdentity (threadId, sizeof(threadId));
      #if defined WIN32
        sscanf (threadId, "%d", &threadNumeric);
      #else /* VXWORKS */
        sscanf (index(threadId,'(') + 1, "%d", &threadNumeric);
      #endif
        CU_ASSERT (threadNumeric == os_threadIdToInteger(os_threadIdSelf()));
    }
  #else
    {
        char threadId[512];
        uintptr_t threadNumeric;

        os_threadFigureIdentity (threadId, sizeof(threadId));

      #ifdef WIN32
        (void)sscanf (threadId, "%d", &threadNumeric);
      #else
        (void)sscanf (threadId, "%"PRIxPTR, &threadNumeric);
      #endif

      #ifndef INTEGRITY
        CU_ASSERT (threadNumeric == (uintptr_t)os_threadIdToInteger(os_threadIdSelf()));
      #endif
    }
  #endif

  #if ENABLE_TRACING
    /* Figure out the identity of the thread, check the return parameter */
    printf ("Starting tc_os_threadFigureIdentity_003\n");
  #endif
  #ifdef _WRS_KERNEL
   {
       char threadId[512];
       char threadIdString[512];
       int threadNumeric;
       int threadIdLen;

       snprintf (threadIdString, sizeof(threadIdString), "%s (%d %d)", taskName(taskIdSelf()),os_threadIdSelf(),taskIdSelf());
       threadIdLen = os_threadFigureIdentity (threadId, sizeof(threadId));
       CU_ASSERT (threadIdLen == strlen(threadIdString));
   }
  #else
   {
       char threadId[512];
       char threadIdString[512];
       unsigned int threadIdLen;

       (void)snprintf (threadIdString, sizeof(threadIdString), "0x%"PRIxMAX, os_threadIdToInteger(os_threadIdSelf()));
       threadIdLen = os_threadFigureIdentity (threadId, sizeof(threadId));

       CU_ASSERT (threadIdLen == strlen(threadIdString));
   }
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_threadFigureIdentity\n");
  #endif
}

CUnit_Test(thread, attr_init)
{
    os_threadAttr thread_os_threadAttr;
  #if ENABLE_TRACING
    /* Check default attributes: schedClass */
    printf ("Starting tc_os_threadAttrInit_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    CU_ASSERT (thread_os_threadAttr.schedClass == OS_SCHED_DEFAULT);

  #if ENABLE_TRACING
    /* Check default attributes: schedPriority */
    printf ("Starting tc_os_threadAttrInit_002\n");
  #endif
  #if !(defined _WRS_KERNEL || defined WIN32 || defined __APPLE__)
    os_threadAttrInit (&thread_os_threadAttr);
    CU_ASSERT (thread_os_threadAttr.schedPriority == ((sched_get_priority_min (SCHED_OTHER) + sched_get_priority_max (SCHED_OTHER)) / 2 ));
  #else
    #if ENABLE_TRACING
    /* OSX priorities are different (min=15 and max=47) */
    printf ("N.A - Not tested for VxWorks, Windows and OSX\n");
    #endif
  #endif

  #if ENABLE_TRACING
    /* Check default attributes: stacksize */
    printf ("Starting tc_os_threadAttrInit_003\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    CU_ASSERT (thread_os_threadAttr.stackSize == 0);

  #if ENABLE_TRACING
    printf ("Ending tc_threadAttrInit\n");
  #endif
}

CUnit_Test(thread, memmalloc)
{
  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with success result for main thread */
    printf ("Starting tc_os_threadMemMalloc_001\n");
  #endif
    returnval = os_threadMemMalloc (3, 100);
    CU_ASSERT (returnval != NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with fail result for main thread
       for index already in use */
    printf ("Starting tc_os_threadMemMalloc_002\n");
  #endif
    returnval = os_threadMemMalloc (3, 100);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with fail result for main thread
       for index < 0 */
    printf ("Starting tc_os_threadMemMalloc_003\n");
  #endif
    returnval = os_threadMemMalloc (-1, 100);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemMalloc with fail result for main thread
       for index >= OS_THREAD_MEM_ARRAY_SIZE */
    printf ("Starting tc_os_threadMemMalloc_004\n");
  #endif
    returnval = os_threadMemMalloc (OS_THREAD_MEM_ARRAY_SIZE, 100);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    printf ("Ending tc_threadMemMalloc\n");
  #endif
}

CUnit_Test(thread, memget)
{
  #if ENABLE_TRACING
    /* Check os_threadMemGet for main thread and non allocated index */
    printf ("Starting tc_os_threadMemGet_001\n");
  #endif
    returnval = os_threadMemGet (OS_THREAD_WARNING);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemGet for main thread and allocated index */
    printf ("Starting tc_os_threadMemGet_002\n");
  #endif
    /* FIXME: This test is no good. Apart from the fact that a valid thread
              memory index should be used (os_threadMemoryIndex), this also
              does not work if the test is executed in a self-contained
              manner using the CUnit runner. For now just work around it by
              first doing a os_threadMemMalloc. */
    (void)os_threadMemMalloc(3, 100);
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval != NULL);

  #if ENABLE_TRACING
    printf ("Ending tc_threadMemGet\n");
  #endif
}

CUnit_Test(thread, memfree)
{
  #if ENABLE_TRACING
    /* Check os_threadMemFree for main thread and non allocated index */
    printf ("Starting tc_os_threadMemFree_001\n");
  #endif
    os_threadMemFree (OS_THREAD_WARNING);
    returnval = os_threadMemGet (OS_THREAD_WARNING);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    /* Check os_threadMemFree for main thread and allocated index */
    printf ("Starting tc_os_threadMemFree_002\n");
  #endif
    /* FIXME: See comments on memget test. */
    (void)os_threadMemMalloc(3, 100);
    returnval = os_threadMemGet(3);
    CU_ASSERT(returnval != NULL);
    os_threadMemFree (3);
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    printf ("Ending tc_threadMemFree\n");
  #endif
}

CUnit_Test(thread, module)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    os_threadHook hook;
    int           mainCount;
    int result;

    os_threadAttrInit (&thread_os_threadAttr);
    /* Run the following tests for child thread */
    result = os_threadCreate (&thread_os_threadId, "ThreadMemory", &thread_os_threadAttr, &threadMemory_thread, NULL);
    if (result == os_resultSuccess)
    {
#ifdef _WRS_KERNEL
        sleepSeconds(1);
#endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);
    }
    else
    {
       printf ("Child thread could not be started");
    }

  #if ENABLE_TRACING
    /* Check only startCb is called on created thread */
    printf ("Starting tc_os_threadModule_001\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = NULL;
    hook.stopCb = NULL;
    hook.stopArg = NULL;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 0 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check startCb and stopCb are called on created thread */
    printf ("Starting tc_os_threadModule_002\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = NULL;
    hook.stopCb = threadStopCallback;
    hook.stopArg = NULL;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 1 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check startCb and stopCb are called and startArg is passed */
    printf ("Starting tc_os_threadModule_003\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = &mainCount;
    hook.stopCb = threadStopCallback;
    hook.stopArg = NULL;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 0 && stopCallbackCount == 1 && mainCount == 1 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check startCb and stopCb are called and stopArg is passed */
    printf ("Starting tc_os_threadModule_004\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = NULL;
    hook.stopCb = threadStopCallback;
    hook.stopArg = &mainCount;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 0 && mainCount == 1 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check startCb and stopCb are called and startArg and stopArg are passed */
    printf ("Starting tc_os_threadModule_005\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = &mainCount;
    hook.stopCb = threadStopCallback;
    hook.stopArg = &mainCount;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 0 && stopCallbackCount == 0 && mainCount == 2 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check stopCb is called on created thread */
    printf ("Starting tc_os_threadModule_006\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = NULL;
    hook.startArg = NULL;
    hook.stopCb = threadStopCallback;
    hook.stopArg = &mainCount;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 0 && stopCallbackCount == 0 && mainCount == 1);

  #if ENABLE_TRACING
    /* Check startCb is called on created thread */
    printf ("Starting tc_os_threadModule_007\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = threadStartCallback;
    hook.startArg = NULL;
    hook.stopCb = NULL;
    hook.stopArg = &mainCount;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 0 && mainCount == 0 && threadCalled == 1);

  #if ENABLE_TRACING
    /* Check startCb is called, but thread main is not called */
    printf ("Starting tc_os_threadModule_008\n");
  #endif
    startCallbackCount = 0;
    stopCallbackCount = 0;
    threadCalled = 0;
    mainCount = 0;
    hook.startCb = threadStartCallbackFAIL;
    hook.startArg = NULL;
    hook.stopCb = NULL;
    hook.stopArg = NULL;
    os_threadModuleSetHook(&hook, NULL);
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, &threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 0 && mainCount == 0 && threadCalled == 0);

  #if ENABLE_TRACING
    printf ("Ending tc_threadModule\n");
  #endif
}

