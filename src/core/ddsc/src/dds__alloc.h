#ifndef _DDS_ALLOC_H_
#define _DDS_ALLOC_H_

#include "dds__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_sample_free_contents (char * data, const uint32_t * ops);

#if defined (__cplusplus)
}
#endif
#endif
