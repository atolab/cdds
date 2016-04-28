c/*
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

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "os/os.h"

os_result os_mutexInit (os_mutex *mutex, const os_mutexAttr *mutexAttr)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif
  assert (mutexAttr == NULL || mutexAttr->scopeAttr == OS_SCOPE_PRIVATE);
  InitializeSRWLock (&mutex->lock);
#ifdef OSPL_STRICT_MEM
  mutex->signature = OS_MUTEX_MAGIC_SIG;
#endif
  return os_resultSuccess;
}

void os_mutexDestroy (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
  mutex->signature = 0;
#endif
}

void os_mutexLock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  AcquireSRWLockExclusive (&mutex->lock);
}

os_result os_mutexLock_s (os_mutex *mutex)
{
  os_mutexLock (mutex);
  return os_resultSuccess;
}

os_result os_mutexTryLock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  return TryAcquireSRWLockExclusive (&mutex->lock) ? os_resultSuccess : os_resultBusy;
}

void os_mutexUnlock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  ReleaseSRWLockExclusive (&mutex->lock);
}

os_result os_condInit (os_cond *cond, os_mutex *dummymtx, const os_condAttr *condAttr)
{
  assert (cond != NULL);
  assert (dummymtx != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature != OS_COND_MAGIC_SIG);
#endif
  assert (condAttr == NULL || condAttr->scopeAttr != OS_SCOPE_PRIVATE);
  (void)dummymtx;
  InitializeConditionVariable (&cond->cond);
#ifdef OSPL_STRICT_MEM
  cond->signature = OS_COND_MAGIC_SIG;
#endif
  return os_resultSuccess;
}

void os_condDestroy (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature == OS_COND_MAGIC_SIG);
  cond->signature = 0;
#endif
}

void os_condWait (os_cond *cond, os_mutex *mutex)
{
  assert (cond != NULL);
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif
  if (SleepConditionVariableSRW (&cond->cond, &mutex->lock, INFINITE, 0))
    return os_resultSuccess;
  else
    abort ();
}

os_result os_condTimedWait (os_cond *cond, os_mutex *mutex, const os_time *time)
{
  DWORD timems;
  assert (cond != NULL);
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif
  timems = time->tv_sec * 1000 + (time->tv_nsec + 999999999) / 1000000;
  if (SleepConditionVariableSRW (&cond->cond, &mutex->lock, timems, 0))
    return os_resultSuccess;
  else if (GetLastError () != ERROR_TIMEOUT)
    abort ();
  else if (timems != INFINITE)
    return os_resultTimeout;
  else
    return os_resultSuccess;
}

void os_condSignal (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  WakeConditionVariable (&cond->cond);
}

void os_condBroadcast (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  WakeAllConditionVariable (&cond->cond);
}

os_result os_rwlockInit (os_rwlock *rwlock, const os_rwlockAttr *rwlockAttr)
{
  assert (rwlock);
  assert (rwlockAttr == NULL || rwlockAttr->scopeAttr == OS_SCOPE_PRIVATE);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  InitializeSRWLock (&rwlock->lock);
  rwlock->state = 0;
#ifdef OSPL_STRICT_MEM
  rwlock->signature = OS_RWLOCK_MAGIC_SIG;
#endif
  return os_resultSuccess;
}

void os_rwlockDestroy (os_rwlock *rwlock)
{
  assert (rwlock);
  assert (rwlock->state == 0);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
  rwlock->signature = 0;
#endif
}

void os_rwlockRead (os_rwlock *rwlock)
{
  assert (rwlock);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  AcquireSRWLockShared (&rwlock->lock);
  rwlock->state = 1;
}

void os_rwlockWrite (os_rwlock *rwlock)
{
  assert (rwlock);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  AcquireSRWLockExclusive (&rwlock->lock);
  rwlock->state = -1;
}

os_result os_rwlockTryRead (os_rwlock *rwlock)
{
  assert (rwlock);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  if (TryAcquireSRWLockShared (&rwlock->lock)) {
    rwlock->state = 1;
  } else {
    return os_resultBusy;
  }
}

os_result os_rwlockTryWrite (os_rwlock *rwlock)
{
  assert (rwlock);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  if (TryAcquireSRWLockExclusive (&rwlock->lock)) {
    rwlock->state = -1;
    return os_resultSuccess;
  } else {
    return os_resultBusy;
  }
}

void os_rwlockUnlock (os_rwlock *rwlock)
{
  assert (rwlock);
#ifdef OSPL_STRICT_MEM
  assert (rwlock->signature != OS_RWLOCK_MAGIC_SIG);
#endif
  assert (rwlock->state != 0);
  if (rwlock->state > 0) {
    ReleaseSRWLockShared (&rwlock->lock);
  } else {
    ReleaseSRWLockExclusive (&rwlock->lock);
  }
}
