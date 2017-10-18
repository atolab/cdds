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

/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/code/os_init.c
 *  \brief Initialization / Deinitialization
 *
 * Initialization / Deinitialization provides routines for
 * initializing the OS layer claiming required resources
 * and routines to deinitialize the OS layer, releasing
 * all resources still claimed.
 */

#include "os/os.h"

const char *
os_versionString(void)
{
    return OSPL_VERSION_STR;
}
