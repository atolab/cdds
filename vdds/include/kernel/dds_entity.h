#ifndef _DDS_ENTITY_H_
#define _DDS_ENTITY_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_entity_init
(
  dds_entity * e, dds_entity * parent,
  dds_entity_kind_t kind, dds_qos_t * qos
);

DDS_EXPORT void dds_entity_add_ref (dds_entity * e);

DDS_EXPORT bool dds_entity_callback_lock (dds_entity * e);
DDS_EXPORT void dds_entity_callback_unlock (dds_entity * e);
DDS_EXPORT void dds_entity_delete_signal (dds_entity_t e);
DDS_EXPORT void dds_entity_delete_impl (dds_entity_t e, bool child, bool recurse);

#if defined (__cplusplus)
}
#endif
#endif
