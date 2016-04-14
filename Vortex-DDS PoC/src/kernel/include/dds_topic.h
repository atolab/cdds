#ifndef _DDS_TOPIC_H_
#define _DDS_TOPIC_H_

#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern struct sertopic * dds_topic_lookup (dds_domain * domain, const char * name);
extern void dds_topic_free (dds_domainid_t domainid, struct sertopic * st);

typedef bool (*dds_topic_intern_filter_fn) (const void * sample, void *ctx);

DDS_EXPORT void dds_topic_set_filter_with_ctx
  (dds_entity_t topic, dds_topic_intern_filter_fn filter, void *ctx);

DDS_EXPORT dds_topic_intern_filter_fn dds_topic_get_filter_with_ctx
  (dds_entity_t topic);

#if defined (__cplusplus)
}
#endif
#endif
