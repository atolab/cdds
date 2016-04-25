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

#define OS_HAVE_INLINE 0 /* override automatic determination of inlining */
#define VDDS_INLINE        /* no "inline" in function defs (not really needed) */
#define OS_ATOMICS_OMIT_FUNCTIONS 0 /* force inclusion of functions defs */

#include "os/os_atomics.h"
