#ifndef OS_PLATFORM_THREAD_H
#define OS_PLATFORM_THREAD_H

#include <pthread.h>

#if defined (__cplusplus)
extern "C" {
#endif

    /* Wrapped in a struct to help programmers conform to the abstraction. */
    typedef struct os_threadId_s {
        pthread_t v; /* Don't touch directly (except for maybe a test or the os-abstraction implementation itself). */
    } os_threadId;

    void os_threadModuleInit (void);
    void os_threadModuleExit (void);

#if defined (__cplusplus)
}
#endif

#endif
