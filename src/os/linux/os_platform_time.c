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

/** \file os/darwin/code/os_time.c
 *  \brief Darwin time management
 *
 * Implements time management for Darwin
 */
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "os/os.h"

os_time os__timeDefaultTimeGet(void)
{
  static int timeshift = INT_MAX;
  struct timespec t;
  os_time rt;

  if(timeshift == INT_MAX) {
    const char *p = getenv("OSPL_TIMESHIFT");
    timeshift = (p == NULL) ? 0 : atoi(p);
  }

  (void) clock_gettime (CLOCK_REALTIME, &t);

  rt.tv_sec = (os_timeSec) t.tv_sec + timeshift;
  rt.tv_nsec = t.tv_nsec;

  return rt;
}

os_time os_timeGetMonotonic (void)
{
  struct timespec t;
  os_time rt;
  (void) clock_gettime (CLOCK_MONOTONIC, &t);

  rt.tv_sec = (os_timeSec) t.tv_sec;
  rt.tv_nsec = t.tv_nsec;

  return rt;
}

os_time os_timeGetElapsed (void)
{
  /* Elapsed time clock not (yet) supported on this platform. */
  return os_timeGetMonotonic();
}

os_result os_nanoSleep (os_time delay)
{
  struct timespec t;
  struct timespec r;
  int result;
  os_result rv;

  if( delay.tv_sec >= 0 && delay.tv_nsec >= 0) {
    /* Time should be normalized */
    assert (delay.tv_nsec < 1000000000);
    t.tv_sec = delay.tv_sec;
    t.tv_nsec = delay.tv_nsec;
    result = nanosleep (&t, &r);
    while (result && os_getErrno() == EINTR) {
      t = r;
      result = nanosleep (&t, &r);
    }
    if (result == 0) {
      rv = os_resultSuccess;
    } else {
      rv = os_resultFail;
    }
  } else {
    rv = os_resultFail;
  }
  return rv;
}
