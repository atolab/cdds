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

/** \file os/darwin/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include <assert.h>

#include "os/os.h"

/** \brief Counter that keeps track of number of times os-layer is initialized */
static os_atomic_uint32_t _ospl_osInitCount = OS_ATOMIC_UINT32_INIT(0);

/** \brief OS layer initialization
 *
 * \b os_osInit calls:
 * - \b os_sharedMemoryInit
 * - \b os_threadInit
 */
void os_osInit (void)
{
  uint32_t initCount;

  initCount = os_atomic_inc32_nv(&_ospl_osInitCount);

  if (initCount == 1) {
    os_processModuleInit();
    os_threadModuleInit();
    os_timeModuleInit();
    os_reportInit(false);
    os_socketModuleInit();
  }

  return;
}

/** \brief OS layer deinitialization
 *
 * \b os_osExit calls:
 * - \b os_sharedMemoryExit
 * - \b os_threadExit
 */
void os_osExit (void)
{
  uint32_t initCount;

  initCount = os_atomic_dec32_nv(&_ospl_osInitCount);

  if (initCount == 0) {
    os_socketModuleExit();
    os_reportExit();
    os_timeModuleExit();
    os_threadModuleExit();
    os_processModuleExit();
  } else if ((initCount + 1) < initCount){
    /* The 0 boundary is passed, so os_osExit is called more often than
     * os_osInit. Therefore undo decrement as nothing happened and warn. */
    os_atomic_inc32(&_ospl_osInitCount);
    OS_REPORT(OS_WARNING, "os_osExit", 1, "OS-layer not initialized");
    /* Fail in case of DEV, as it is incorrect API usage */
    assert(0);
  }
  return;
}

/* We need this on windows to make sure the main thread of MFC applications
 * calls os_osInit().
 */
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  /* handle to DLL module */
    DWORD fdwReason,     /* reason for calling function */
    LPVOID lpReserved )  /* reserved */
{
    /* Perform actions based on the reason for calling.*/
    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        /* Initialize once for each new process.
         * Return FALSE to fail DLL load.
         */
        os_osInit();
    break;
    case DLL_THREAD_ATTACH:
         /* Do thread-specific initialization.
          */
    break;
    case DLL_THREAD_DETACH:
         /* Do thread-specific cleanup.
          */
    break;
    case DLL_PROCESS_DETACH:
        /* Perform any necessary cleanup.
         */
        os_osExit();
    break;
    }
    return TRUE;  /* Successful DLL_PROCESS_ATTACH.*/
}
