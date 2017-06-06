#ifndef OS_PLATFORM_PROCESS_H
#define OS_PLATFORM_PROCESS_H

#if defined (__cplusplus)
extern "C" {
#endif
    void os_processModuleInit(void);
    void os_processModuleExit(void);
#if defined (__cplusplus)
}
#endif

#endif
