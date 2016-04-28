#ifndef OS_PLATFORM_THREAD_H
#define OS_PLATFORM_THREAD_H

#if defined (__cplusplus)
extern "C" {
#endif

    typedef pthread_t os_threadId;

    void os_threadModuleInit (void);
    void os_threadModuleExit (void);

#if defined (__cplusplus)
}
#endif

#endif
