#ifndef _DDS_KEY_H_
#define _DDS_KEY_H_

#include "kernel/dds_types.h"

struct dds_key_hash;

#if defined (__cplusplus)
extern "C" {
#endif

void dds_key_md5 (struct dds_key_hash * kh);

void dds_key_gen
(
  const dds_topic_descriptor_t * const desc,
  struct dds_key_hash * kh,
  const char * sample
);

#if defined (__cplusplus)
}
#endif
#endif
