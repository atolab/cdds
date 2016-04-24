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

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API uint32_t ut_crcCalculate (const void *buf, size_t length) __nonnull_all__ __attribute_pure__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_CRC_H */
