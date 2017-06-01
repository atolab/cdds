#ifndef _DDS_WAITSET_H_
#define _DDS_WAITSET_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

_Check_return_
bool dds_waitset_add_condition_locked (_In_ dds_waitset * ws, _In_ dds_condition * cond, _In_ dds_attach_t x);
int dds_waitset_remove_condition_locked (_In_ dds_waitset * ws, _In_ dds_condition * cond);
void dds_waitset_signal (_In_ dds_waitset * ws);

#if defined (__cplusplus)
}
#endif
#endif
