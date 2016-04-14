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

#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

    /* Expands ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms, but not $X */
    OS_API char *ut_expand_envvars(const char *string);

    /* Expands $X, ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms, $ and \ can be escaped with \ */
    OS_API char *ut_expand_envvars_sh(const char *string);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
