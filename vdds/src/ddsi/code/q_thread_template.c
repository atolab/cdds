/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* -*- c -*- */

#include "sysdeps.h"
#include "os_atomics.h"
#include "q_static_assert.h"

#if defined SUPPRESS_THREAD_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE struct thread_state1 *lookup_thread_state (void)
{
#if OS_HAS_TSD_USING_THREAD_KEYWORD
  if (tsd_thread_state == NULL)
    tsd_thread_state = lookup_thread_state_real ();
  return tsd_thread_state;
#else
  return lookup_thread_state_real ();
#endif
}

NN_C99_INLINE int vtime_awake_p (vtime_t vtime)
{
  return (vtime % 2) == 0;
}

NN_C99_INLINE int vtime_asleep_p (vtime_t vtime)
{
  return (vtime % 2) == 1;
}

NN_C99_INLINE int vtime_gt (vtime_t vtime1, vtime_t vtime0)
{
  Q_STATIC_ASSERT_CODE (sizeof (vtime_t) == sizeof (svtime_t));
  return (svtime_t) (vtime1 - vtime0) > 0;
}

NN_C99_INLINE void thread_state_asleep (struct thread_state1 *ts1)
{
  vtime_t vt = ts1->vtime;
  vtime_t wd = ts1->watchdog;
  if (vtime_awake_p (vt))
  {
    os_atomic_fence_rel ();
    ts1->vtime = vt + 1;
  }
  else
  {
    os_atomic_fence_rel ();
    ts1->vtime = vt + 2;
    os_atomic_fence_acq ();
  }

  if ( wd % 2 ){
    ts1->watchdog = wd + 2;
  } else {
    ts1->watchdog = wd + 1;
  }
 }

NN_C99_INLINE void thread_state_awake (struct thread_state1 *ts1)
{
  vtime_t vt = ts1->vtime;
  vtime_t wd = ts1->watchdog;
  if (vtime_asleep_p (vt))
    ts1->vtime = vt + 1;
  else
  {
    os_atomic_fence_rel ();
    ts1->vtime = vt + 2;
  }
  os_atomic_fence_acq ();

  if ( wd % 2 ){
    ts1->watchdog = wd + 1;
  } else {
    ts1->watchdog = wd + 2;
  }

}

NN_C99_INLINE void thread_state_blocked (struct thread_state1 *ts1)
{
  vtime_t wd = ts1->watchdog;
  if ( wd % 2 ){
    ts1->watchdog = wd + 2;
  } else {
    ts1->watchdog = wd + 1;
  }
}

NN_C99_INLINE void thread_state_unblocked (struct thread_state1 *ts1)
{
  vtime_t wd = ts1->watchdog;
  if ( wd % 2 ){
    ts1->watchdog = wd + 1;
  } else {
    ts1->watchdog = wd + 2;
  }
}


