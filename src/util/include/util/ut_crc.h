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
#ifndef UT_CRC_H
#define UT_CRC_H

#include "os/os.h"
#include "util/ut_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

UTIL_EXPORT uint32_t ut_crcCalculate (const void *buf, size_t length) __nonnull_all__ __attribute_pure__;

#if defined (__cplusplus)
}
#endif

#endif /* UT_CRC_H */
