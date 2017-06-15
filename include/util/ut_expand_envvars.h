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
#ifndef UT_EXPAND_ENVVARS_H
#define UT_EXPAND_ENVVARS_H

#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

    /* Expands ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms, but not $X */
    OSAPI_EXPORT char *ut_expand_envvars(const char *string);

    /* Expands $X, ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms, $ and \ can be escaped with \ */
    OSAPI_EXPORT char *ut_expand_envvars_sh(const char *string);

#if defined (__cplusplus)
}
#endif

#endif
