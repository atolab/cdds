#ifndef OS_PLATFORM_SYNC_H
#define OS_PLATFORM_SYNC_H

#include <stdint.h>
#include <pthread.h>
#if HAVE_LKST
#include "lkst.h"
#endif

#if defined (__cplusplus)
extern "C" {
#endif

    typedef struct os_cond {
#ifdef OSPL_STRICT_MEM
        /* Used to identify initialized cond when memory is freed -
           keep this first in the structure so its so its address is
           the same as the os_cond */
        uint64_t signature;
#endif
        pthread_cond_t cond;
    } os_cond;

    typedef struct os_mutex {
#ifdef OSPL_STRICT_MEM
        /* Used to identify initialized cond when memory is freed -
           keep this first in the structure so its so its address is
           the same as the os_cond */
        uint64_t signature;
#endif
        pthread_mutex_t mutex;
    } os_mutex;

    typedef struct os_rwlock {
        os_mutex mutex;
    } os_rwlock;

    void os_syncModuleInit(void);
    void os_syncModuleExit(void);

#if defined (__cplusplus)
}
#endif

#endif
