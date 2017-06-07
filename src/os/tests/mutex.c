#include "dds.h"
#include "cunitrunner/runner.h"
#include "os/os.h"
#include "os/os_process.h"

#ifdef __VXWORKS__
#   ifdef _WRS_KERNEL
#       define FORCE_SCHEDULING() taskDelay(1)
#   else
#       define FORCE_SCHEDULING() sched_yield()
#   endif
#else
#   define FORCE_SCHEDULING()
#endif

#define ENABLE_TRACING 0
#define BUSYLOOP       (100000)
#define MAX_LOOPS      (20)

typedef struct {
    os_mutex global_mutex;
    os_threadId global_data;
    int nolock_corrupt_count;
    int nolock_loop_count;
    int lock_corrupt_count;
    int lock_loop_count;
    int trylock_corrupt_count;
    int trylock_loop_count;
    int trylock_busy_count;
    int stop;
} shared_data;

os_threadAttr       mutex_os_threadAttr;
os_threadId         mutex_os_threadId[4];
os_time             delay1 = { 5, 0 };
os_time             pdelay = { 1, 0 };
os_procId           mutex_os_procId;
os_procId           mutex_os_procId1;
os_procId           mutex_os_procId2;
os_result           result;
char                buffer[512];
int                 supported_resultBusy;
int                 loop;
static shared_data *sd;
char                filePath[255];

uint32_t concurrent_lock_thread (_In_opt_ void *arg)
{
    int j;
    int loopc = 0;
    int printed = 0;

    while (!sd->stop)
    {
       if (arg) os_mutexLock (&sd->global_mutex);
       sd->global_data = os_threadIdSelf();

       FORCE_SCHEDULING();

       for (j = 0; j < BUSYLOOP; j++);
       if (os_threadIdToInteger(sd->global_data) !=
           os_threadIdToInteger(os_threadIdSelf()))
       {
          if (arg)
          {
             sd->lock_corrupt_count++;
          }
          else
          {
             sd->nolock_corrupt_count++;
          }
          if (!printed) {
              printed++;
          }
       }
       if (arg)
       {
          sd->lock_loop_count++;
          os_mutexUnlock (&sd->global_mutex);
       }
       else
       {
          sd->nolock_loop_count++;
       }

       FORCE_SCHEDULING();

       for (j = 0; j < BUSYLOOP; j++);
       loopc++;
    }
    return 0;
}

uint32_t concurrent_trylock_thread (_In_opt_ void *arg)
{
    int j;
    int loopc = 0;
    int printed = 0;
    os_result result;

    while (!sd->stop)
    {
       if (arg)
       {
          while ((result = os_mutexTryLock (&sd->global_mutex))
                 != os_resultSuccess)
          {
             if (result == os_resultBusy)
             {
                sd->trylock_busy_count++;
             }
             FORCE_SCHEDULING();
          }
       }
       sd->global_data = os_threadIdSelf();

       FORCE_SCHEDULING();

       for (j = 0; j < BUSYLOOP; j++);
       if (os_threadIdToInteger(sd->global_data) !=
           os_threadIdToInteger(os_threadIdSelf()))
       {
          if (arg)
          {
             sd->trylock_corrupt_count++;
          }
          else
          {
             sd->nolock_corrupt_count++;
          }
          if (!printed) {
              printed++;
          }
       }
       if (arg)
       {
          sd->trylock_loop_count++;
          os_mutexUnlock (&sd->global_mutex);
       }
       else
       {
          sd->nolock_loop_count++;
       }

       FORCE_SCHEDULING();

       for (j = 0; j < BUSYLOOP; j++);
       loopc++;
    }
    return 0;
}

static int  suite_abstraction_mutex_init (void)
{
    int result = 0;
    os_osInit();
  #if ENABLE_TRACING
    printf("Run suite_abstraction_mutex_init\n");
  #endif
  #ifdef OS_LINUX_MUTEX_H149C
    supported_resultBusy = 1;
  #else
    supported_resultBusy = 0;
  #endif

    return result;
}

static int suite_abstraction_mutex_clean (void)
{
    int result = DDS_RETCODE_OK;

  #if ENABLE_TRACING
    printf("Run suite_abstraction_mutex_clean\n");
  #endif
    os_free (sd);
    os_osExit();
    return result;
}

static void tc_os_mutexInit (void)
{
  #if ENABLE_TRACING
    /* Initialize mutex with PRIVATE scope and Success result  */
    printf ("Starting tc_os_mutexInit_001\n");
  #endif
    sd = os_malloc (sizeof (*sd));
    os_mutexInit (&sd->global_mutex);

  #if ENABLE_TRACING
    /* Initialize mutex with PRIVATE scope and Fail result  */
    printf ("Starting tc_os_mutexInit_002\n");
    printf ("N.A - Failure cannot be forced.\n");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_mutexInit\n");
  #endif
}

static void tc_os_mutexLock (void)
{
  #if ENABLE_TRACING
    /* Test critical section access with locking and PRIVATE scope  */
    printf ("Starting tc_os_mutexLock_001\n");
  #endif
    os_threadAttrInit (&mutex_os_threadAttr);

    FORCE_SCHEDULING();

    delay1.tv_sec = 3;
  #if ENABLE_TRACING
    printf ("Testing for %d.%9.9d seconds without lock\n", delay1.tv_sec, delay1.tv_nsec);
  #endif
    sd->stop = 0;
    sd->nolock_corrupt_count = 0;
    sd->nolock_loop_count = 0;
    sd->lock_corrupt_count = 0;
    sd->lock_loop_count = 0;
    sd->trylock_corrupt_count = 0;
    sd->trylock_loop_count = 0;
    sd->trylock_busy_count = 0;
    os_threadCreate (&mutex_os_threadId[0], "thr0", &mutex_os_threadAttr, &concurrent_lock_thread, NULL);
    os_threadCreate (&mutex_os_threadId[1], "thr1", &mutex_os_threadAttr, &concurrent_lock_thread, NULL);
    os_threadCreate (&mutex_os_threadId[2], "thr2", &mutex_os_threadAttr, &concurrent_trylock_thread, NULL);
    os_threadCreate (&mutex_os_threadId[3], "thr3", &mutex_os_threadAttr, &concurrent_trylock_thread, NULL);
    os_nanoSleep (delay1);
    sd->stop = 1;
    os_threadWaitExit (mutex_os_threadId[0], NULL);
    os_threadWaitExit (mutex_os_threadId[1], NULL);
    os_threadWaitExit (mutex_os_threadId[2], NULL);
    os_threadWaitExit (mutex_os_threadId[3], NULL);
  #if ENABLE_TRACING
    printf ("All threads stopped\n");
  #endif

    delay1.tv_sec = 3;
  #if ENABLE_TRACING
    printf ("Testing for %d.%9.9d seconds with lock\n", delay1.tv_sec, delay1.tv_nsec);
  #endif
    sd->stop = 0;
    sd->nolock_corrupt_count = 0;
    sd->nolock_loop_count = 0;
    sd->lock_corrupt_count = 0;
    sd->lock_loop_count = 0;
    sd->trylock_corrupt_count = 0;
    sd->trylock_loop_count = 0;
    sd->trylock_busy_count = 0;
    os_threadCreate (&mutex_os_threadId[0], "thr0", &mutex_os_threadAttr, &concurrent_lock_thread, (void *)1);
    os_threadCreate (&mutex_os_threadId[1], "thr1", &mutex_os_threadAttr, &concurrent_lock_thread, (void *)1);
    os_threadCreate (&mutex_os_threadId[2], "thr2", &mutex_os_threadAttr, &concurrent_trylock_thread, (void *)1);
    os_threadCreate (&mutex_os_threadId[3], "thr3", &mutex_os_threadAttr, &concurrent_trylock_thread, (void *)1);
    os_nanoSleep (delay1);
    sd->stop = 1;
    os_threadWaitExit (mutex_os_threadId[0], NULL);
    os_threadWaitExit (mutex_os_threadId[1], NULL);
    os_threadWaitExit (mutex_os_threadId[2], NULL);
    os_threadWaitExit (mutex_os_threadId[3], NULL);
  #if ENABLE_TRACING
    printf ("All threads stopped\n");
  #endif

    CU_ASSERT (sd->lock_corrupt_count == 0 || sd->lock_loop_count > 0);

  #if ENABLE_TRACING
    /* Lock mutex with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutexLock_002\n");
  #endif
    os_mutexLock (&sd->global_mutex); //Cannot be checked
    os_mutexUnlock (&sd->global_mutex);

  #if ENABLE_TRACING
    /* Lock mutex with PRIVATE scope and Fail result */
    printf ("Starting tc_os_mutexLock_003\n");
    printf ("N.A - Failure cannot be forced\n");
  #endif

  #if ENABLE_TRACING
    /* mutexLock_s with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutexLock_004\n");
  #endif
    CU_ASSERT (os_mutexLock_s (&sd->global_mutex) == os_resultSuccess);
    os_mutexUnlock (&sd->global_mutex);

  #if ENABLE_TRACING
    printf ("Ending tc_mutexLock\n");
  #endif
}

static void tc_os_mutexTryLock (void)
{
  #if ENABLE_TRACING
    /* Test critical section access with trylocking and PRIVATE scope */
    printf ("Starting tc_os_mutexTryLock_001\n");
  #endif
    CU_ASSERT (sd->trylock_corrupt_count == 0 || sd->trylock_loop_count > 0);

  #if ENABLE_TRACING
    /* TryLock mutex with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutexTryLock_002\n");
  #endif
    result = os_mutexTryLock (&sd->global_mutex);
    CU_ASSERT (result == os_resultSuccess);

  #if ENABLE_TRACING
    /* TryLock mutex with PRIVATE scope and Busy result */
    printf ("Starting tc_os_mutexTryLock_003\n");
  #if defined(__VXWORKS__) && !defined(_WRS_KERNEL)
    printf ("N.A - Mutexes are recursive on VxWorks RTP so this test is  disabled\n");
  #endif
  #endif

    result = os_mutexTryLock (&sd->global_mutex);
    CU_ASSERT (result == os_resultBusy);

  #if ENABLE_TRACING
    printf ("Ending tc_mutexTryLock\n");
  #endif
}

static void tc_os_mutexUnlock (void)
{
  #if ENABLE_TRACING
    /* Unlock mutex with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutexUnlock_001\n");
  #endif
    os_mutexUnlock (&sd->global_mutex); // Cannot be checked directly - Success is assumed

  #if ENABLE_TRACING
    /* Unlock mutex with PRIVATE scope and Fail result */
    printf ("Starting tc_os_mutexUnlock_002\n");
    printf ("N.A - Failure cannot be forced\n");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_mutexUnlock\n");
  #endif
}

static void tc_os_mutexDestroy (void)
{
  #if ENABLE_TRACING
    /* Deinitialize mutex with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutexDestroy_001\n");
  #endif
    os_mutexDestroy(&sd->global_mutex); // Cannot be checked directly - Success is assumed

  #if ENABLE_TRACING
    /* Deinitialize mutex with PRIVATE scope and Fail result */
    printf ("Starting tc_os_mutexDestroy_002\n");
    printf ("N.A - Failure cannot be forced\n");
  #endif

  #if ENABLE_TRACING
    printf ("Ending tc_mutexDestroy\n");
  #endif
}

int main (int argc, char *argv[])
{
    CU_pSuite suite;

    if (runner_init(argc, argv)){
        goto err_init;
    }

    if ((suite = CU_add_suite ("abstraction_mutex", suite_abstraction_mutex_init, suite_abstraction_mutex_clean)) == NULL){
        goto err;
    }
    if (CU_add_test (suite, "tc_os_mutexInit", tc_os_mutexInit) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_mutexLock", tc_os_mutexLock) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_mutexTryLock", tc_os_mutexTryLock) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_mutexUnlock", tc_os_mutexUnlock) == NULL) {
        goto err;
    }
    if (CU_add_test (suite, "tc_os_mutexDestroy", tc_os_mutexDestroy) == NULL) {
        goto err;
    }
    runner_run();
err:
    runner_fini();
err_init:
    return CU_get_error();
}
