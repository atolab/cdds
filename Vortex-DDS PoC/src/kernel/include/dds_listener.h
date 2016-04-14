#ifndef _DDS_LISTENER_H_
#define _DDS_LISTENER_H_

#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_listener_merge (dds_listener_t dst, const dds_listener_t src, dds_entity_kind_t kind);
void dds_listener_get_unl (dds_entity_t e, dds_listener_t l);
void dds_listener_set_unl (dds_entity_t e, const dds_listener_t l);

#if defined (__cplusplus)
}
#endif
#endif
