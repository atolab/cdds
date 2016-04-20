#ifndef _DDS_WAITSET_H_
#define _DDS_WAITSET_H_

#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

int dds_waitset_attach_condition (dds_waitset * ws, dds_condition * cond);
int dds_waitset_detach_condition (dds_waitset * ws, dds_condition * cond);
bool dds_waitset_add_condition_locked (dds_waitset * ws, dds_condition * cond, dds_attach_t x);
int dds_waitset_remove_condition_locked (dds_waitset * ws, dds_condition * cond);
void dds_waitset_signal (dds_waitset * ws);

#if defined (__cplusplus)
}
#endif
#endif
