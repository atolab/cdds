#include "dds.h"
#include "cunitrunner/runner.h"
#include "os/os.h"

#define ENABLE_TRACING 0

char          arg_result[30];
int           threadCalled;
int           startCallbackCount;
int           stopCallbackCount;
void          *returnval;

static void sleepSeconds (int seconds)
{
  os_time sdelay;
  sdelay.tv_sec = seconds;
  sdelay.tv_nsec = 0;
  os_nanoSleep (sdelay);
}

void *new_thread (void *args)
{
  static char tr_result[] = "os_threadExit";

  snprintf (arg_result, sizeof (arg_result), "%s", (char *)args);
  sleepSeconds (3);
  return tr_result;
}

void *threadExit_thread (void *args)
{
#ifdef _WRS_KERNEL
  taskDelay(1*sysClkRateGet());
#endif
  os_threadExit (args);
  return NULL;
}

static unsigned long thread_id_from_thread = 0;

void * threadId_thread (void *args)
{
  if (args != NULL)
  {
    sleepSeconds (3);
  }
  thread_id_from_thread = os_threadIdToInteger (os_threadIdSelf ());
  return NULL;
}

void * get_threadExit_thread (void *args)
{
  os_threadId * threadId = args;
  unsigned long * id = NULL;
  int ret = os_threadWaitExit (*threadId, (void**) &id);
  if (ret == os_resultSuccess)
  {
    free (id);
  }
  return (ret == os_resultSuccess) ? args : NULL;
}

void *threadIdentity_thread (void *args)
{
  char *identity = args;
  os_threadFigureIdentity (identity, 512);
  return NULL;
}

static void *threadMain(void *args)
{
  OS_UNUSED_ARG(args);
  threadCalled = 1;
  sleepSeconds(1);
  return NULL;
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

void *threadMemory_thread (void *args)
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

    return returnval;
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

static int  suite_abstraction_thread_init (void)
{
    int result = 0;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_thread_init\n");
  #endif

    return result;
}

static int suite_abstraction_thread_clean (void)
{
    int result = DDS_RETCODE_OK;

  #if ENABLE_TRACING
    printf("Run suite_abstraction_thread_clean\n");
  #endif
    os_osExit();
    return result;
}

static void tc_os_threadCreate (void)
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
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate1", &thread_os_threadAttr, new_thread, "os_threadCreate");
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
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate3", &thread_os_threadAttr, new_thread, "os_threadCreate");
    CU_ASSERT (result == os_resultSuccess);
  #if !(defined _WRS_KERNEL || defined WIN32)
    if (result == os_resultSuccess) {
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
        CU_ASSERT (result_int == DDS_RETCODE_OK);

        if (result_int != DDS_RETCODE_OK) {
          #if ENABLE_TRACING
            printf ("pthread_getschedparam failed");
          #endif
        } else
            CU_ASSERT (policy == SCHED_OTHER);
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
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate4", &thread_os_threadAttr, new_thread, "os_threadCreate");
    CU_ASSERT (result == os_resultSuccess);
    if (result == os_resultSuccess) {
      #ifndef WIN32
        int policy;
        struct sched_param sched_param;

        result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate5", &thread_os_threadAttr, new_thread, "os_threadCreate");
        CU_ASSERT (result == os_resultSuccess);
        if (result == os_resultSuccess) {
            int policy;
            struct sched_param sched_param;

            result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate6", &thread_os_threadAttr, new_thread, "os_threadCreate");
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

        result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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
    result = os_threadCreate (&thread_os_threadId, "ThreadCreate7", &thread_os_threadAttr, new_thread, "os_threadCreate");
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

        result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate8", &thread_os_threadAttr, new_thread, "os_threadCreate");
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

            result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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
        result = os_threadCreate (&thread_os_threadId, "ThreadCreate9", &thread_os_threadAttr, new_thread, "os_threadCreate");
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

            result_int = pthread_getschedparam (thread_os_threadId, &policy, &sched_param);
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

static void tc_os_threadExit (void)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    unsigned long  value;
    unsigned long  return_value;
    int result;

  #if ENABLE_TRACING
    /* Check for correct return value when terminating thread via os_threadExit */
    printf ("Starting tc_os_threadExit_001\n");
  #endif
    value = 12345678;
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadExit", &thread_os_threadAttr, threadExit_thread, (void *)value);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, (void **)&return_value);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            CU_ASSERT (return_value == value);
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
    printf ("Ending tc_threadExit\n");
  #endif
}

static void tc_os_threadIdSelf (void)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    int result;

  #if ENABLE_TRACING
    /* Check if own thread ID is correctly provided */
    printf ("Starting tc_os_threadIdSelf_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "OwnThreadId", &thread_os_threadAttr, threadId_thread, NULL);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            unsigned long tmp_thread_os_threadId = os_threadIdToInteger(thread_os_threadId);
            CU_ASSERT (thread_id_from_thread == tmp_thread_os_threadId);
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

static void tc_os_threadWaitExit (void)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    int result;

  #if ENABLE_TRACING
    /* Wait for thread to terminate and get the return value with Success result,
       while thread is still running */
    printf ("Starting tc_os_threadWaitExit_001\n");
  #endif
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, threadId_thread, (void *)1);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
            CU_ASSERT (thread_id_from_thread == os_threadIdToInteger(thread_os_threadId));
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
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, threadId_thread, NULL);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT(result == os_resultSuccess);

        if (result == os_resultSuccess) {
            CU_ASSERT (thread_id_from_thread == os_threadIdToInteger(thread_os_threadId));
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
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, threadId_thread, NULL);
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
        void * returnResult1;

        result = os_threadCreate (&thread_os_threadId, "threadToWaitFor", &thread_os_threadAttr, threadId_thread, (void*) 1);
        CU_ASSERT (result == os_resultSuccess);
        result1 = os_threadCreate (&threadWait1, "waitingThread1", &thread_os_threadAttr, get_threadExit_thread, &thread_os_threadId);
        CU_ASSERT (result1 == os_resultSuccess);

        if (result == os_resultSuccess && result1 == os_resultSuccess)
        {
          #ifdef _WRS_KERNEL
            sleepSeconds(1);
          #endif
            result1 = os_threadWaitExit (threadWait1, &returnResult1);

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
    result = os_threadCreate (&thread_os_threadId, "threadWaitExit", &thread_os_threadAttr, threadId_thread, NULL);
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

static void tc_os_threadFigureIdentity (void)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    char threadId[512];
    char thread_name[512];
    int result;
    uintptr_t threadNumeric = 0;

  #if ENABLE_TRACING
    /* Figure out the identity of the thread, where it's name is known */
    printf ("Starting tc_os_threadFigureIdentity_001\n");
  #endif
  #ifdef WIN32
    /* Untested because the identifier does not contain the name on Windows */
  #else
    os_threadAttrInit (&thread_os_threadAttr);
    result = os_threadCreate (&thread_os_threadId, "threadFigureIdentity", &thread_os_threadAttr, threadIdentity_thread, threadId);
    CU_ASSERT (result == os_resultSuccess);

    if (result == os_resultSuccess) {
      #ifdef _WRS_KERNEL
        sleepSeconds(1);
      #endif
        result = os_threadWaitExit (thread_os_threadId, NULL);
        CU_ASSERT (result == os_resultSuccess);

        if (result == os_resultSuccess) {
          #ifdef _WRS_KERNEL
            int dum;
            sscanf (threadId, "%s (%d %d)", thread_name, &threadNumeric, &dum);
          #else
            sscanf (threadId, "%s %"SCNxPTR, thread_name, &threadNumeric);
          #endif
            CU_ASSERT (strcmp (thread_name, "threadFigureIdentity") == 0 && threadNumeric == (uintptr_t)thread_os_threadId);
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
        sscanf (threadId, "%d", &threadNumeric);
      #else
        sscanf (&threadId[12], PRIxPTR, &threadNumeric);
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

     #ifdef WIN32
       snprintf (threadIdString, sizeof(threadIdString), "%d",
                  (uintptr_t)os_threadIdToInteger(os_threadIdSelf()));
     #else
       snprintf (threadIdString, sizeof(threadIdString), PRIxPTR, (uintptr_t)os_threadIdSelf());
     #endif
       threadIdLen = os_threadFigureIdentity (threadId, sizeof(threadId));

     #ifdef WIN32
       CU_ASSERT (threadIdLen == strlen(threadIdString));
     #else
       CU_ASSERT (threadIdLen == (strlen(threadIdString) + strlen("main thread ")));
     #endif
   }
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_threadFigureIdentity\n");
  #endif
}

static void tc_os_threadAttrInit (void)
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

static void tc_os_threadMemMalloc (void)
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

static void tc_os_threadMemGet (void)
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
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval != NULL);

  #if ENABLE_TRACING
    printf ("Ending tc_threadMemGet\n");
  #endif
}

static void tc_os_threadMemFree (void)
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
    os_threadMemFree (3);
    returnval = os_threadMemGet (3);
    CU_ASSERT (returnval == NULL);

  #if ENABLE_TRACING
    printf ("Ending tc_threadMemFree\n");
  #endif
}

static void tc_os_threadModule (void)
{
    os_threadId   thread_os_threadId;
    os_threadAttr thread_os_threadAttr;
    os_threadHook hook;
    int           mainCount;
    int result;

    os_threadAttrInit (&thread_os_threadAttr);
    /* Run the following tests for child thread */
    result = os_threadCreate (&thread_os_threadId, "ThreadMemory", &thread_os_threadAttr, threadMemory_thread, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
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
    result = os_threadCreate(&thread_os_threadId, "threadHook", &thread_os_threadAttr, threadMain, NULL);
    CU_ASSERT (result == os_resultSuccess);
    os_threadWaitExit(thread_os_threadId, NULL);
    CU_ASSERT (startCallbackCount == 1 && stopCallbackCount == 0 && mainCount == 0 && threadCalled == 0);

  #if ENABLE_TRACING
    printf ("Ending tc_threadModule\n");
  #endif
}

int main (int argc, char *argv[])
{
    CU_pSuite suite;

    if (runner_init(argc, argv)){
        goto err_init;
    }
    if ((suite = CU_add_suite ("abstraction_thread", suite_abstraction_thread_init, suite_abstraction_thread_clean)) == NULL){
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadCreate", tc_os_threadCreate) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadExit", tc_os_threadExit) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadIdSelf", tc_os_threadIdSelf) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadWaitExit", tc_os_threadWaitExit) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadFigureIdentity", tc_os_threadFigureIdentity) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadAttrInit", tc_os_threadAttrInit) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadMemMalloc", tc_os_threadMemMalloc) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadMemGet", tc_os_threadMemGet) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadMemFree", tc_os_threadMemFree) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_threadModule", tc_os_threadModule) == NULL) {
        goto err;
    }
    runner_run();
err:
    runner_fini();
err_init:
    return CU_get_error();
}
