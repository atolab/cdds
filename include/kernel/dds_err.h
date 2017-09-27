#ifndef _DDS_ERR_H_
#define _DDS_ERR_H_

#include <assert.h>
#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* To construct return status
 * Use '+' instead of '|'. Otherwise, the SAL checking doesn't
 * understand when a return value is negative or positive and
 * complains a lot about "A successful path through the function
 * does not set the named _Out_ parameter." */
#if !defined(__FILE_ID__)
#error "__FILE_ID__ not defined"
#endif

#define DDS__FILE_ID__ (((__FILE_ID__ & 0x1ff)) << 22)
#define DDS__LINE__ ((__LINE__ & 0x3fff) << 8)

#define DDS_ERR_NO(err) -(DDS__FILE_ID__ + DDS__LINE__ + (err))

#define DDS_ERRNO(e,msg,...) (assert(e > DDS_RETCODE_OK), os_report(OS_REPORT_ERROR, OS_FUNCTION, __FILE__, __LINE__, DDS_ERR_NO(e), (msg), ##__VA_ARGS__), DDS_ERR_NO(e))

#if defined (__cplusplus)
}
#endif
#endif
