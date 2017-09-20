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

#ifndef OS_ASSERT_H
#define OS_ASSERT_H

#include <assert.h>

#define OS_ASSERT(s) do { assert(s); (void)"LCOV_EXCL_BR_LINE"; } while (0)

#endif /* OS_ASSERT_H */
