#ifndef _DDS_IID_H_
#define _DDS_IID_H_

#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_iid_init (void);
void dds_iid_fini (void);
uint64_t dds_iid_gen (void);

#if defined (__cplusplus)
}
#endif
#endif
