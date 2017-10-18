#ifndef _DDS_READER_H_
#define _DDS_READER_H_

#include "dds__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct status_cb_data;

void dds_reader_status_cb (void * entity, const struct status_cb_data * data);

/*
  dds_reader_lock_samples: Returns number of samples in read cache and locks the
  reader cache to make sure that the samples content doesn't change.
  Because the cache is locked, you should be able to read/take without having to
  lock first. This is done by passing the DDS_READ_WITHOUT_LOCK value to the
  read/take call as maxs. Then the read/take will not lock but still unlock.

  See also CHAM-287, CHAM-306 and LITE-1183.

  Used to support LENGTH_UNLIMITED in C++.
*/
#define DDS_READ_WITHOUT_LOCK (0xFFFFFFED)
DDS_EXPORT uint32_t dds_reader_lock_samples (dds_entity_t entity);

struct nn_rsample_info;
struct nn_rdata;
DDS_EXPORT void dds_reader_ddsi2direct (dds_entity_t entity, void (*cb) (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, void *arg), void *cbarg);

#define dds_reader_lock(hdl, obj) dds_entity_lock(hdl, DDS_KIND_READER, (dds_entity**)obj)
#define dds_reader_unlock(obj)    dds_entity_unlock((dds_entity*)obj);

#if defined (__cplusplus)
}
#endif
#endif
