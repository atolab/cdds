#include "dds.h"
#include "CUnit/Runner.h"
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

CUnit_Suite_Initialize(os_mutex)
{
    int result = 0;
    os_osInit();
    printf("Run os_mutex_Initialize\n");
  #ifdef OS_LINUX_MUTEX_H149C
    supported_resultBusy = 1;
  #else
    supported_resultBusy = 0;
  #endif

    return result;
}

CUnit_Suite_Cleanup(os_mutex)
{
    int result = DDS_RETCODE_OK;

    printf("Run os_mutex_Cleanup\n");
    os_free (sd);
    os_osExit();
    return result;
}

CUnit_Test(os_mutex, init)
{
    /* Initialize mutex with PRIVATE scope and Success result  */
    printf ("Starting os_mutex_init_001\n");
    sd = os_malloc (sizeof (*sd));
    os_mutexInit (&sd->global_mutex);

    /* Initialize mutex with PRIVATE scope and Fail result  */
    printf ("Starting os_mutex_init_002\n");
    printf ("N.A - Failure cannot be forced.\n");

    printf ("Ending os_mutex_init\n");
}

CUnit_Test(os_mutex, lock, false)
{
    /* Test critical section access with locking and PRIVATE scope  */
    printf ("Starting tc_os_mutex_lock_001\n");
    os_threadAttrInit (&mutex_os_threadAttr);

    FORCE_SCHEDULING();

    delay1.tv_sec = 3;
    printf ("Testing for %d.%9.9d seconds without lock\n", delay1.tv_sec, delay1.tv_nsec);
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
    printf ("All threads stopped\n");

    delay1.tv_sec = 3;
    printf ("Testing for %d.%9.9d seconds with lock\n", delay1.tv_sec, delay1.tv_nsec);
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
    printf ("All threads stopped\n");

    CU_ASSERT (sd->lock_corrupt_count == 0 || sd->lock_loop_count > 0);

    /* Lock mutex with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutex_lock_002\n");
    os_mutexLock (&sd->global_mutex); //Cannot be checked
    os_mutexUnlock (&sd->global_mutex);

    /* Lock mutex with PRIVATE scope and Fail result */
    printf ("Starting tc_os_mutex_lock_003\n");
    printf ("N.A - Failure cannot be forced\n");

    /* mutexLock_s with PRIVATE scope and Success result */
    printf ("Starting tc_os_mutex_lock_004\n");
    CU_ASSERT (os_mutexLock_s (&sd->global_mutex) == os_resultSuccess);
    os_mutexUnlock (&sd->global_mutex);

    printf ("Ending os_mutex_lock\n");
}

CUnit_Test(os_mutex, trylock, false)
{
    os_result result;

    /* Test critical section access with trylocking and PRIVATE scope */
    printf ("Starting os_mutex_trylock_001\n");
    CU_ASSERT (sd->trylock_corrupt_count == 0 || sd->trylock_loop_count > 0);

    /* TryLock mutex with PRIVATE scope and Success result */
    printf ("Starting os_mutex_trylock_002\n");
    result = os_mutexTryLock (&sd->global_mutex);
    CU_ASSERT (result == os_resultSuccess);

    /* TryLock mutex with PRIVATE scope and Busy result */
    printf ("Starting os_mutex_trylock_003\n");
  #if defined(__VXWORKS__) && !defined(_WRS_KERNEL)
    printf ("N.A - Mutexes are recursive on VxWorks RTP so this test is  disabled\n");
  #endif

    result = os_mutexTryLock (&sd->global_mutex);
    CU_ASSERT (result == os_resultBusy);

    printf ("Ending os_mutex_trylock\n");
}

CUnit_Test(os_mutex, unlock, false)
{
    /* Unlock mutex with PRIVATE scope and Success result */
    printf ("Starting os_mutex_unlock_001\n");
    os_mutexUnlock (&sd->global_mutex); // Cannot be checked directly - Success is assumed

    /* Unlock mutex with PRIVATE scope and Fail result */
    printf ("Starting os_mutex_unlock_002\n");
    printf ("N.A - Failure cannot be forced\n");

    printf ("Ending os_mutex_unlock\n");
}

CUnit_Test(os_mutex, destroy, false)
{
    /* Deinitialize mutex with PRIVATE scope and Success result */
    printf ("Starting os_mutex_destroy_001\n");
    os_mutexDestroy(&sd->global_mutex); // Cannot be checked directly - Success is assumed

    /* Deinitialize mutex with PRIVATE scope and Fail result */
    printf ("Starting os_mutex_destroy_002\n");
    printf ("N.A - Failure cannot be forced\n");

    printf ("Ending os_mutex_destroy\n");
}
