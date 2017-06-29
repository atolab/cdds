#ifndef _DDS_ENTITY_H_
#define _DDS_ENTITY_H_

#include "kernel/dds_types.h"
#include "ddsi/q_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

_Check_return_
dds_entity_t dds_entity_init (
        _In_     dds_entity * e,
        _In_opt_ dds_entity * parent,
        _In_     dds_entity_kind_t kind,
        _In_opt_ dds_qos_t * qos,
        _In_opt_ const dds_listener_t *listener,
        _In_     uint32_t mask);

void dds_entity_add_ref(_In_ dds_entity * e);
void dds_entity_add_ref_nolock(_In_ dds_entity *e);

_Check_return_
dds_return_t dds_entity_listener_propagation(_In_ dds_entity *e, _In_ dds_entity *src, _In_ uint32_t status, _In_opt_ void *metrics, _In_ bool propagate);

#define dds_entity_is_enabled(e, k)   (((dds_entity*)e)->m_flags & DDS_ENTITY_ENABLED)

#define dds_entity_status_set(e, t)   (((dds_entity*)e)->m_status->m_trigger |= (((dds_entity*)e)->m_status_enable & t))
#define dds_entity_status_reset(e,t)  (((dds_entity*)e)->m_status->m_trigger &= ~t)
#define dds_entity_status_match(e,t)  (((dds_entity*)e)->m_status->m_trigger &   t)
void dds_entity_status_signal(_In_ dds_entity *e);

_Check_return_
int32_t dds_entity_lock(_In_ dds_entity_t hdl, _In_ dds_entity_kind_t kind, _Out_ dds_entity **e);
void dds_entity_unlock(_In_ dds_entity *e);

#define dds_entity_kind(hdl) (hdl & DDS_ENTITY_KIND_MASK)



#if defined (__cplusplus)
}
#endif
#endif
