#ifndef _DDS_ENTITY_H_
#define _DDS_ENTITY_H_

#include "kernel/dds_types.h"
#include "ddsi/q_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_entity_init (
        _In_     dds_entity * e,
        _In_opt_ dds_entity * parent,
        _In_     dds_entity_kind_t kind,
        _In_opt_ dds_qos_t * qos,
        _In_opt_ const dds_listener_t *listener,
        _In_     uint32_t mask);

void dds_entity_add_ref (_In_ dds_entity * e);

_Check_return_
bool dds_entity_cb_propagate_begin(_In_ dds_entity *e);
void dds_entity_cb_propagate_end(_In_ dds_entity *e);
bool dds_entity_cp_propagate_call(_In_ dds_entity *e, _In_ dds_entity *src, _In_ uint32_t status, _In_opt_ void *metrics, _In_ bool propagate);

void dds_entity_delete_signal (_In_ dds_entity_t e);
void dds_entity_delete_impl (_In_ dds_entity_t e, _In_ bool child, _In_ bool recurse);
void dds_entity_delete_wait (_In_ dds_entity_t e, _In_ struct thread_state1 * const thr);

#define dds_entity_is_a(e, k) ((e > 0) && (((dds_entity*)e)->m_kind == k))

#define dds_entity_is_enabled(e, k)   (((dds_entity*)e)->m_flags & DDS_ENTITY_ENABLED)

#define dds_entity_status_set(e, t)   (((dds_entity*)e)->m_status->m_trigger |= (((dds_entity*)e)->m_status_enable & t))
#define dds_entity_status_reset(e,t)  (((dds_entity*)e)->m_status->m_trigger &= ~t)
#define dds_entity_status_match(e,t)  (((dds_entity*)e)->m_status->m_trigger &   t)
void dds_entity_status_signal(_In_ dds_entity_t e);

bool dds_entity_is_kind (_In_opt_ dds_entity_t e, _In_ dds_entity_kind_t kind);

#if defined (__cplusplus)
}
#endif
#endif
