#ifndef _DDS_ERR_H_
#define _DDS_ERR_H_

#include "os/os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* To construct return status
 * Use '+' instead of '|'. Otherwise, the SAL checking doesn't
 * understand when a return value is negative or positive and
 * complains a lot about "A successful path through the function
 * does not set the named _Out_ parameter." */
#if !defined(__FILE_ID__)
#define __FILE_ID__ (0)
#endif

#define DDS__FILE_ID__ (((__FILE_ID__ & 0x1ff)) << 22)
#define DDS__LINE__ ((__LINE__ & 0x3fff) << 8)

#define DDS_ERRNO_DEPRECATED(e) ((e <= 0) ? e : -(DDS__FILE_ID__ + DDS__LINE__ + (e)))

static VDDS_INLINE dds_return_t handle_dds_errno(int e, const char * context, const char * file, int line, const char * msg, ...)
{
  dds_return_t ret;
  if (e <= 0) {
    ret = e;
  } else {
    va_list args;
    va_start(args, msg);
    OS_REPORT_FROM_FILE(OS_REPORT_ERROR, context, file, line, e, msg, args);
    va_end(args);
    ret = -(DDS__FILE_ID__ + DDS__LINE__ + (e));
  }

  return ret;
}
#define DDS_ERRNO(e,msg,...) (handle_dds_errno(e,OS_FUNCTION,__FILE__,__LINE__,msg,##__VA_ARGS__))

#if defined (__cplusplus)
}
#endif
#endif
