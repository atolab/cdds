#ifndef OS_PLATFORM_PROCESS_H
#define OS_PLATFORM_PROCESS_H

#if defined (__cplusplus)
extern "C" {
#endif
	void os_processModuleInit(void);
	void os_processModuleExit(void);
	void os_procCallExitHandlers(void);
	HANDLE os_procIdToHandle(os_procId);
	os_procId os_handleToProcId(HANDLE);

#if defined (__cplusplus)
}
#endif

#endif
