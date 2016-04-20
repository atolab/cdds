#ifndef _DDS_TKMAP_H_
#define _DDS_TKMAP_H_

#include "dds_types.h"
#include "os_atomics.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct tkmap;
struct serdata;
struct dds_topic;

struct tkmap_instance
{
  struct serdata * m_sample;
  struct tkmap * m_map;
  uint64_t m_iid;
  os_atomic_uint32_t m_refc;
};

struct tkmap * dds_tkmap_new (void);
void dds_tkmap_free (struct tkmap *tkmap);
void dds_tkmap_instance_ref (struct tkmap_instance *tk);
uint64_t dds_tkmap_lookup (struct tkmap *tkmap, struct serdata *serdata);
bool dds_tkmap_get_key (struct tkmap * map, uint64_t iid, void * sample);
struct tkmap_instance * dds_tkmap_find
(
  const struct dds_topic * topic,
  struct serdata * sd,
  const bool rd,
  const bool create
);
struct tkmap_instance * dds_tkmap_find_by_id (struct tkmap * map, uint64_t iid);

DDS_EXPORT struct tkmap_instance * dds_tkmap_lookup_instance_ref (struct serdata * sd);
DDS_EXPORT void dds_tkmap_instance_unref (struct tkmap_instance * tk);

#if defined (__cplusplus)
}
#endif
#endif
