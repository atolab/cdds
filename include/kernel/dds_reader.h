#ifndef _DDS_READER_H_
#define _DDS_READER_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct status_cb_data;

void dds_reader_status_cb (void * entity, const struct status_cb_data * data);

/* 
  dds_reader_lock_samples: Returns number of samples in read cache.
  Locks cache iif > 0. Subsequent read/take required to unlock cache.
  Used to support LENGTH_UNLIMITED in C++.
*/

DDS_EXPORT uint32_t dds_reader_lock_samples (dds_entity_t entity);

struct nn_rsample_info;
struct nn_rdata;
DDS_EXPORT void dds_reader_ddsi2direct (dds_entity_t entity, void (*cb) (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, void *arg), void *cbarg);

#if defined (__cplusplus)
}
#endif
#endif
