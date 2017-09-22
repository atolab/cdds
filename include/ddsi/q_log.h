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
#ifndef NN_LOG_H
#define NN_LOG_H

#include <stdarg.h>

#include "os/os.h"
#include "ddsi/q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define LC_FATAL 1u
#define LC_ERROR 2u
#define LC_WARNING 4u
#define LC_CONFIG 8u
#define LC_INFO 16u
#define LC_DISCOVERY 32u
#define LC_DATA 64u
#define LC_TRACE 128u
#define LC_RADMIN 256u
#define LC_TIMING 512u
#define LC_TRAFFIC 1024u
#define LC_TOPIC 2048u
#define LC_TCP 4096u
#define LC_PLIST 8192u
#define LC_WHC 16384u
#define LC_THROTTLE 32768u
#define LC_ALLCATS (LC_FATAL | LC_ERROR | LC_WARNING | LC_CONFIG | LC_INFO | LC_DISCOVERY | LC_DATA | LC_TRACE | LC_TIMING | LC_TRAFFIC | LC_TCP | LC_THROTTLE)

typedef unsigned logcat_t;

typedef struct logbuf {
  char buf[2048];
  size_t bufsz;
  size_t pos;
  nn_wctime_t tstamp;
} *logbuf_t;

logbuf_t logbuf_new (void);
void logbuf_init (logbuf_t lb);
void logbuf_free (logbuf_t lb);

int nn_vlog (logcat_t cat, const char *fmt, va_list ap);
OSAPI_EXPORT int nn_log (_In_ logcat_t cat, _In_z_ _Printf_format_string_ const char *fmt, ...) __attribute_format__((printf,2,3));
OSAPI_EXPORT int nn_trace (_In_z_ _Printf_format_string_ const char *fmt, ...) __attribute_format__((printf,1,2));
void nn_log_set_tstamp (nn_wctime_t tnow);

#define TRACE(args) ((config.enabled_logcats & LC_TRACE) ? (nn_trace args) : 0)

#define LOG_THREAD_CPUTIME(guard) do {                                  \
    if (config.enabled_logcats & LC_TIMING)                             \
    {                                                                   \
      nn_mtime_t tnowlt = now_mt ();                                      \
      if (tnowlt.v >= (guard).v)                                          \
      {                                                                 \
        int64_t ts = get_thread_cputime ();                            \
        nn_log (LC_TIMING, "thread_cputime %d.%09d\n",                  \
                (int) (ts / T_SECOND), (int) (ts % T_SECOND));          \
        (guard).v = tnowlt.v + T_SECOND;                                  \
      }                                                                 \
    }                                                                   \
  } while (0)


#define NN_WARNING(/* fmt, <- C'99 doesn't allow empty VA_ARGS, so include fmt in the VA_ARGS */...) nn_log (LC_WARNING, "<Warning> " __VA_ARGS__)
#define NN_ERROR(/* fmt, <- C'99 doesn't allow empty VA_ARGS, so include fmt in the VA_ARGS */...) nn_log (LC_ERROR, "<Error> " __VA_ARGS__)
#define NN_FATAL(/* fmt, <- C'99 doesn't allow empty VA_ARGS, so include fmt in the VA_ARGS */...) nn_log (LC_FATAL, "<Fatal> " __VA_ARGS__)

#if defined (__cplusplus)
}
#endif

#endif /* NN_LOG_H */
