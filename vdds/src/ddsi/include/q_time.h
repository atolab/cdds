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
#ifndef NN_TIME_H
#define NN_TIME_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DDSI2
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define T_NEVER 0x7fffffffffffffffll
#define T_MILLISECOND 1000000ll
#define T_SECOND (1000 * T_MILLISECOND)
#define T_MICROSECOND (T_MILLISECOND/1000)

typedef struct {
  int seconds;
  unsigned fraction;
} nn_ddsi_time_t;

#if DDSI_DURATION_ACCORDING_TO_SPEC /* what the spec says */
typedef struct { /* why different from ddsi_time_t? */
  int sec;
  int nanosec;
} nn_duration_t;
#else /* this is what I used to do & what wireshark does - probably right */
typedef nn_ddsi_time_t nn_duration_t;
#endif

typedef struct {
  int64_t v;
} nn_mtime_t;

typedef struct {
  int64_t v;
} nn_wctime_t;

typedef struct {
  int64_t v;
} nn_etime_t;

extern const nn_ddsi_time_t invalid_ddsi_timestamp;
extern const nn_ddsi_time_t ddsi_time_infinite;
extern const nn_duration_t duration_infinite;

int valid_ddsi_timestamp (nn_ddsi_time_t t);

OS_API nn_wctime_t now (void);       /* wall clock time */
nn_mtime_t now_mt (void);     /* monotonic time */
nn_etime_t now_et (void);     /* elapsed time */
void mtime_to_sec_usec (int *sec, int *usec, nn_mtime_t t);
void wctime_to_sec_usec (int *sec, int *usec, nn_wctime_t t);
void etime_to_sec_usec (int *sec, int *usec, nn_etime_t t);
nn_mtime_t mtime_round_up (nn_mtime_t t, int64_t round);
nn_mtime_t add_duration_to_mtime (nn_mtime_t t, int64_t d);
nn_wctime_t add_duration_to_wctime (nn_wctime_t t, int64_t d);
nn_etime_t add_duration_to_etime (nn_etime_t t, int64_t d);

nn_ddsi_time_t nn_wctime_to_ddsi_time (nn_wctime_t t);
OS_API nn_wctime_t nn_wctime_from_ddsi_time (nn_ddsi_time_t x);
OS_API nn_duration_t nn_to_ddsi_duration (int64_t t);
OS_API int64_t nn_from_ddsi_duration (nn_duration_t x);
#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* NN_TIME_H */
