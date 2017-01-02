#ifndef OS_PLATFORM_THREAD_H
#define OS_PLATFORM_THREAD_H

#include "os/os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

    typedef struct os_threadInfo_s {
		DWORD threadId;
		HANDLE handle;
	} os_threadId;

    os_result os_threadModuleInit (void);
    void os_threadModuleExit (void);

#if defined (__cplusplus)
}
#endif

#endif
