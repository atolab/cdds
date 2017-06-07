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
#ifndef OS_ERRNO_H
#define OS_ERRNO_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os/os_defs.h"

#if (defined(WIN32) || defined(WINCE))
#include <winerror.h>
#endif

#include <errno.h> /* Required on Windows platforms too */

#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

    /** \brief Get error code set by last operation that failed
     *
     * @return Error code
     */
    OS_API int
    os_getErrno (
                 void);

    /** \brief Set error code to specified value
     *
     * @return void
     * @param err Error code
     */
    OS_API void
    os_setErrno (
                 int err);

    /**
     * \brief Get description for specified error code
     *
     * @return 0 success. On error a (positive) error number is returned
     * @param err Error number
     * @param buf Buffer to store description in
     * @oaram bufsz Number of bytes available in buf
     */
    OS_API int
    os_strerror_r (
        _In_ int err,
        _Out_writes_z_(bufsz) char *buf,
        _In_ size_t bufsz);

    /**
     * \brief Get description for specified error code
     *
     * @return Pointer to string allocated in thread specific memory
     * @param err Error number
     */
    OS_API const char *
    os_strerror (
        _In_ int err);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_ERRNO_H */
