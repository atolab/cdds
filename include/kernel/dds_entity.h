#ifndef _DDS_ENTITY_H_
#define _DDS_ENTITY_H_

#include "kernel/dds_types.h"
#include "ddsi/q_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

_Check_return_ dds_entity_t
dds_entity_init(
        _In_     dds_entity * e,
        _In_opt_ dds_entity * parent,
        _In_     dds_entity_kind_t kind,
        _In_opt_ dds_qos_t * qos,
        _In_opt_ const dds_listener_t *listener,
        _In_     uint32_t mask);

void
dds_entity_inc_refc(
        _In_ dds_entity_t entity);
void
dds_entity_inc_usage(
        _In_ dds_entity *e);

dds_return_t
dds_entity_dec_usage(
        _In_ dds_entity_t entity);

_Check_return_ dds_retcode_t
dds_entity_listener_propagation(
        _In_ dds_entity *e,
        _In_ dds_entity *src,
        _In_ uint32_t status,
        _In_opt_ void *metrics,
        _In_ bool propagate);

#define dds_entity_is_enabled(e, k)   (((dds_entity*)e)->m_flags & DDS_ENTITY_ENABLED)

#define dds_entity_status_set(e, t)   (((dds_entity*)e)->m_trigger |= (((dds_entity*)e)->m_status_enable & t))
#define dds_entity_status_reset(e,t)  (((dds_entity*)e)->m_trigger &= ~t)
#define dds_entity_status_match(e,t)  (((dds_entity*)e)->m_trigger &   t)

/* The mutex needs to be unlocked when calling this because the entity can be called
 * within the signal callback from other contexts. That shouldn't deadlock. */
void
dds_entity_status_signal(
        _In_ dds_entity *e);

_Check_return_ dds_retcode_t
dds_valid_hdl(
        _In_ dds_entity_t hdl,
        _In_ dds_entity_kind_t kind);

_Check_return_ dds_retcode_t
dds_entity_lock(
        _In_ dds_entity_t hdl,
        _In_ dds_entity_kind_t kind,
        _Out_ dds_entity **e);

void
dds_entity_unlock(
        _In_ dds_entity *e);

#define dds_entity_kind(hdl) (hdl & DDS_ENTITY_KIND_MASK)

_Check_return_ dds_retcode_t
dds_entity_observer_register_nl(
        _In_ dds_entity*  observed,
        _In_ dds_entity_t observer,
        _In_ dds_entity_callback cb);

_Check_return_ dds_retcode_t
dds_entity_observer_register(
        _In_ dds_entity_t observed,
        _In_ dds_entity_t observer,
        _In_ dds_entity_callback cb);

dds_retcode_t
dds_entity_observer_unregister_nl(
        _In_ dds_entity*  observed,
        _In_ dds_entity_t observer);

dds_retcode_t
dds_entity_observer_unregister(
        _In_ dds_entity_t observed,
        _In_ dds_entity_t observer);


#if defined (__cplusplus)
}
#endif
#endif
