#ifndef OS_PLATFORM_SYNC_H
#define OS_PLATFORM_SYNC_H

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
        CONDITION_VARIABLE cond;
    } os_cond;

    typedef struct os_mutex {
#ifdef OSPL_STRICT_MEM
        /* Used to identify initialized cond when memory is freed -
           keep this first in the structure so its so its address is
           the same as the os_cond */
        uint64_t signature;
#endif
        SRWLOCK lock;
    } os_mutex;

    typedef struct os_rwlock {
#ifdef OSPL_STRICT_MEM
        /* Used to identify initialized cond when memory is freed -
           keep this first in the structure so its so its address is
           the same as the os_cond */
        uint64_t signature;
#endif
        SRWLOCK lock;
        int state; /* -1: exclusive, 0: free, 1: shared */
    } os_rwlock;

    
    typedef INIT_ONCE os_once_t;
    #define OS_ONCE_T_STATIC_INIT INIT_ONCE_STATIC_INIT

#if defined (__cplusplus)
}
#endif

#endif
