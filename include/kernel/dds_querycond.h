#ifndef _DDS_QUERYCOND_H_
#define _DDS_QUERYCOND_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C"
{
#endif
DDS_EXPORT void dds_querycondition_from_readcondition(dds_condition_t cond, dds_entity_t rd);

DDS_EXPORT void dds_querycondition_set_filter_with_ctx(dds_querycondition_filter_with_ctx_fn filter, dds_condition_t cond, void *ctx);

#if defined (__cplusplus)
}
#endif
#endif
