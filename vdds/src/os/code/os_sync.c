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

#include <assert.h>
#include "os_sync.h"

/** \file os/common/code/os_mutex_attr.c
 *  \brief Common mutual exclusion semaphore attributes
 *
 * Implements os_mutexAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

/** \brief Initialize mutex attribute
 *
 * Set \b mutexAttr->scopeAttr to \b OS_SCOPE_PRIVATE
 * Set \b mutexAttr->errorCheckingAttr to \b OS_ERRORCHECKING_DISABLED
 */
_Post_satisfies_(mutexAttr->scopeAttr == OS_SCOPE_PRIVATE)
_Post_satisfies_(mutexAttr->errorCheckingAttr == OS_ERRORCHECKING_DISABLED)
void
os_mutexAttrInit (
    _Out_ os_mutexAttr *mutexAttr)
{
    mutexAttr->scopeAttr = OS_SCOPE_PRIVATE;
    /* By setting errorCheckingAttr to OS_ERRORCHECKING_ENABLED or
     * OS_ERRORCHECKING_DISABLED, error-checking can easily be enabled/disabled
     * for all mutexes that don't explicitly set the option themselves. */
    mutexAttr->errorCheckingAttr = OS_ERRORCHECKING_DISABLED;
}
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

/** \file os/common/code/os_cond_attr.c
 *  \brief Common condition variable attributes
 *
 * Implements os_condAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>
#include "os.h"

/** \brief Initialize condition variable attribute
 *
 * Set \b condAttr->scopeAttr to \b OS_SCOPE_PRIVATE
 */
_Post_satisfies_(condAttr->scopeAttr == OS_SCOPE_PRIVATE)
void
os_condAttrInit (
    _Out_ os_condAttr *condAttr)
{
    assert (condAttr != NULL);
    condAttr->scopeAttr = OS_SCOPE_PRIVATE;
}

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

/** \file os/common/code/os_rwlock_attr.c
 *  \brief Common multiple reader writer lock attributes
 *
 * Implements os_rwlockAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_PRIVATE
 */

/** \brief Initialize rwlock attribute
 *
 * Set \b rwlockAttr->scopeAttr to \b OS_SCOPE_PRIVATE
 */
_Post_satisfies_(rwlockAttr->scopeAttr == OS_SCOPE_PRIVATE)
void
os_rwlockAttrInit (
    _Out_ os_rwlockAttr *rwlockAttr)
{
    rwlockAttr->scopeAttr = OS_SCOPE_PRIVATE;
}
