#ifndef _DDS_ERR_H_
#define _DDS_ERR_H_

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

#define DDS_ERR_NO(err) -(DDS__FILE_ID__ + DDS__LINE__ + (err))

#define DDS_ERRNO_DEPRECATED(e) ((e <= 0) ? e : DDS_ERR_NO(e))

dds_return_t handle_dds_errno(int dds_error, int e, const char * context, const char * file, int line, const char * msg, ...);

#define DDS_ERRNO(e,msg,...) (handle_dds_errno(DDS_ERR_NO(e),e,OS_FUNCTION,__FILE__,__LINE__,msg,##__VA_ARGS__))

#if defined (__cplusplus)
}
#endif
#endif
