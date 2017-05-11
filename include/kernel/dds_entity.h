#ifndef _DDS_ENTITY_H_
#define _DDS_ENTITY_H_

#include "kernel/dds_types.h"
#include "ddsi/q_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_entity_init
(
  dds_entity * e, dds_entity * parent,
  dds_entity_kind_t kind, dds_qos_t * qos,
  dds_listener_t listener,
  uint32_t mask
);

void dds_entity_add_ref (dds_entity * e);

bool dds_entity_cb_propagate_begin(dds_entity *e);
void dds_entity_cb_propagate_end(dds_entity *e);
bool dds_entity_cp_propagate_call(dds_entity *e, dds_entity *src, uint32_t status, void *metrics, bool propagate);

void dds_entity_delete_signal (dds_entity_t e);
void dds_entity_delete_impl (dds_entity_t e, bool child, bool recurse);
void dds_entity_delete_wait (dds_entity_t e, struct thread_state1 * const thr);

#define dds_entity_is_a(e, k) ((e > 0) && (((dds_entity*)e)->m_kind == k))

#define dds_entity_is_enabled(e, k)   (((dds_entity*)e)->m_flags & DDS_ENTITY_ENABLED)

#define dds_entity_status_set(e, t)   (((dds_entity*)e)->m_status->m_trigger |= (((dds_entity*)e)->m_status_enable & t))
#define dds_entity_status_reset(e,t)  (((dds_entity*)e)->m_status->m_trigger &= ~t)
#define dds_entity_status_match(e,t)  (((dds_entity*)e)->m_status->m_trigger &   t)
void dds_entity_status_signal(dds_entity_t e);

#if defined (__cplusplus)
}
#endif
#endif
