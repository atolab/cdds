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

/* Make sure we get the XSI compliant version of strerror_r */
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#undef _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <string.h>
#include <assert.h>

#include "os/os.h"

int
os_getErrno (void)
{
    return errno;
}

void
os_setErrno (int err)
{
    errno = err;
}

int
os_strerror_r (int err, char *str, size_t len)
{
    int ret;
    assert(str);
    assert(len > 0);
    len--;
    ret = strerror_r(err, str, len);
    str[len] = '\0';
    return ret;
}
