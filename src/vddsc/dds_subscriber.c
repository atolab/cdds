#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "ddsi/q_entity.h"

#define DDS_SUBSCRIBER_STATUS_MASK                               \
                        DDS_DATA_ON_READERS_STATUS

static dds_return_t dds_subscriber_instance_hdl(dds_entity *e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, 0);
}

static dds_return_t dds_subscriber_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_KERNEL, 0);
    bool consistent = true;

    assert(qos);
    consistent &= (qos->present & QP_GROUP_DATA) ? validate_octetseq(&qos->group_data) : true;
    consistent &= (qos->present & QP_PARTITION) ? validate_stringseq(&qos->partition) : true;
    consistent &= (qos->present & QP_PRESENTATION) ? !validate_presentation_qospolicy(&qos->presentation) : true;
    consistent &= (qos->present & QP_PRISMTECH_ENTITY_FACTORY) ? \
        validate_entityfactory_qospolicy(&qos->entity_factory) : true;

    if (consistent) {
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            ret = DDS_ERRNO (DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_KERNEL, 0);
        } else {
            ret = DDS_RETCODE_OK;
        }
    }
    return ret;
}

static dds_return_t dds_subscriber_qos_set (dds_entity *e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_subscriber_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, DDS_ERR_M1));
        }
    }
    return ret;
}

static dds_return_t dds_subscriber_status_validate (uint32_t mask)
{
    return (mask & ~(DDS_SUBSCRIBER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_KERNEL, 0) :
                     DDS_RETCODE_OK;
}

/*
  Set boolean on readers that indicates state of DATA_ON_READERS
  status on parent subscriber
*/
static dds_return_t dds_subscriber_status_propagate (dds_entity *sub, uint32_t mask, bool set)
{
    if (mask & DDS_DATA_ON_READERS_STATUS) {
        dds_entity *iter = sub->m_children;
        while (iter) {
            os_mutexLock (&iter->m_mutex);
            ((dds_reader*) iter)->m_data_on_readers = set;
            os_mutexUnlock (&iter->m_mutex);
            iter = iter->m_next;
        }
    }
    return DDS_RETCODE_OK;
}

dds_entity_t dds_create_subscriber
(
  _In_ dds_entity_t pp,
  _In_opt_ const dds_qos_t * qos,
  _In_opt_ const dds_listener_t * listener
)
{
    dds_entity * participant;
    dds_subscriber * sub;
    dds_entity_t hdl;
    dds_qos_t * new_qos = NULL;
    dds_return_t ret;
    int32_t errnr;

    errnr = dds_entity_lock(pp, DDS_KIND_PARTICIPANT, &participant);
    if (errnr != DDS_RETCODE_OK) {
        return DDS_ERRNO(errnr, DDS_MOD_KERNEL, DDS_ERR_M2);
    }

    /* Validate qos */
    if (qos) {
        ret = dds_subscriber_qos_validate(qos, false);
        if (ret != DDS_RETCODE_OK) {
            dds_entity_unlock(participant);
            return ret;
        }
        new_qos = dds_qos_create();
        dds_qos_copy(new_qos, qos);
    }

    /* Create subscriber */
    sub = dds_alloc(sizeof(*sub));
    hdl = dds_entity_init(&sub->m_entity, participant, DDS_KIND_SUBSCRIBER, new_qos, listener, DDS_SUBSCRIBER_STATUS_MASK);
    sub->m_entity.m_deriver.set_qos = dds_subscriber_qos_set;
    sub->m_entity.m_deriver.validate_status = dds_subscriber_status_validate;
    sub->m_entity.m_deriver.propagate_status = dds_subscriber_status_propagate;
    sub->m_entity.m_deriver.get_instance_hdl = dds_subscriber_instance_hdl;
    dds_entity_unlock(participant);

    return hdl;
}

dds_return_t dds_notify_readers(_In_ dds_entity_t subscriber)
{
    dds_entity *iter;
    dds_entity *sub;
    int32_t errnr;

    errnr = dds_entity_lock(subscriber, DDS_KIND_SUBSCRIBER, &sub);
    if (errnr == DDS_RETCODE_OK) {
        errnr = DDS_RETCODE_UNSUPPORTED;
        iter = sub->m_children;
        while (iter) {
            os_mutexLock(&iter->m_mutex);
            // TODO: check if reader has data available, call listener
            os_mutexUnlock(&iter->m_mutex);
            iter = iter->m_next;
        }
        dds_entity_unlock(sub);
    }

    return DDS_ERRNO(errnr, DDS_MOD_KERNEL, 0);
}

dds_return_t
dds_subscriber_begin_coherent
(
    dds_entity_t e
)
{
    return DDS_RETCODE_UNSUPPORTED;
}

dds_return_t
dds_subscriber_end_coherent
(
    dds_entity_t e
)
{
    return DDS_RETCODE_UNSUPPORTED;
}