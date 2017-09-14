#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_report.h"
#include "kernel/dds_err.h"
#include "ddsi/q_entity.h"
#include "kernel/dds_report.h"

#define DDS_SUBSCRIBER_STATUS_MASK                               \
                        DDS_DATA_ON_READERS_STATUS

static dds_return_t
dds_subscriber_instance_hdl(
        dds_entity *e,
        dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Generating subscriber instance handle is not supported");
}

static dds_return_t
dds_subscriber_qos_validate(
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = DDS_RETCODE_OK;

    assert(qos);

    if((qos->present & QP_GROUP_DATA) && !validate_octetseq(&qos->group_data)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Group data policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PARTITION) && !validate_stringseq(&qos->partition)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Partition policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PRESENTATION) && validate_presentation_qospolicy(&qos->presentation)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Presentation policy is inconsistent and caused an error");
    }
    if((qos->present & QP_PRISMTECH_ENTITY_FACTORY) && !validate_entityfactory_qospolicy(&qos->entity_factory)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Prismtech entity factory policy is inconsistent and caused an error");
    }
    if(ret == DDS_RETCODE_OK && enabled && (qos->present & QP_PRESENTATION)) {
        /* TODO: Improve/check immutable check. */
        ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY, "Presentation QoS policy is immutable");
    }

    return ret;
}

static dds_return_t
dds_subscriber_qos_set(
        dds_entity *e,
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = dds_subscriber_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "VortexDDS does not support changing QoS policies yet");
        }
    }
    return ret;
}

static dds_return_t
dds_subscriber_status_validate(
        uint32_t mask)
{
    return (mask & ~(DDS_SUBSCRIBER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Invalid status mask") :
                     DDS_RETCODE_OK;
}

/*
  Set boolean on readers that indicates state of DATA_ON_READERS
  status on parent subscriber
*/
static dds_return_t
dds_subscriber_status_propagate(
        dds_entity *sub,
        uint32_t mask,
        bool set)
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

_Pre_satisfies_((participant & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT)
_Must_inspect_result_ dds_entity_t
dds_create_subscriber(
        _In_     dds_entity_t participant,
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
    dds_entity * par;
    dds_subscriber * sub;
    dds_entity_t hdl;
    dds_qos_t * new_qos = NULL;
    dds_return_t ret;
    dds_retcode_t rc;

    DDS_REPORT_STACK();

    rc = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, &par);
    if (rc != DDS_RETCODE_OK) {
        hdl = DDS_ERRNO(rc, "Error occurred on locking participant");
        goto lock_err;
    }

    /* Validate qos */
    if (qos) {
        ret = dds_subscriber_qos_validate(qos, false);
        if (ret != DDS_RETCODE_OK) {
            hdl = ret;
            goto qos_err;
        }
        new_qos = dds_qos_create();
        /* Only returns failure when one of the qos args is NULL, which
         * is not the case here. */
        (void)dds_qos_copy(new_qos, qos);
    }

    /* Create subscriber */
    sub = dds_alloc(sizeof(*sub));
    hdl = dds_entity_init(&sub->m_entity, par, DDS_KIND_SUBSCRIBER, new_qos, listener, DDS_SUBSCRIBER_STATUS_MASK);
    sub->m_entity.m_deriver.set_qos = dds_subscriber_qos_set;
    sub->m_entity.m_deriver.validate_status = dds_subscriber_status_validate;
    sub->m_entity.m_deriver.propagate_status = dds_subscriber_status_propagate;
    sub->m_entity.m_deriver.get_instance_hdl = dds_subscriber_instance_hdl;


qos_err:
    dds_entity_unlock(par);
lock_err:
    DDS_REPORT_FLUSH(hdl <= 0);
    return hdl;
}

_Pre_satisfies_((subscriber & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER)
dds_return_t
dds_notify_readers(
        _In_ dds_entity_t subscriber)
{
    dds_entity *iter;
    dds_entity *sub;
    dds_retcode_t errnr;
    dds_return_t ret;

    DDS_REPORT_STACK();

    errnr = dds_entity_lock(subscriber, DDS_KIND_SUBSCRIBER, &sub);
    if (errnr == DDS_RETCODE_OK) {
        errnr = DDS_RETCODE_UNSUPPORTED;
        ret = DDS_ERRNO(errnr, "Unsupported operation");
        iter = sub->m_children;
        while (iter) {
            os_mutexLock(&iter->m_mutex);
            // TODO: check if reader has data available, call listener
            os_mutexUnlock(&iter->m_mutex);
            iter = iter->m_next;
        }
        dds_entity_unlock(sub);
    } else {
        ret = DDS_ERRNO(errnr, "Error occurred on locking subscriber");
    }

    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
    return ret;
}

dds_return_t
dds_subscriber_begin_coherent(
        _In_ dds_entity_t e)
{
    /* TODO: CHAM-124 Currently unsupported. */
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Using coherency to get a coherent data set is not currently being supported");
}

dds_return_t
dds_subscriber_end_coherent(
        _In_ dds_entity_t e)
{
    /* TODO: CHAM-124 Currently unsupported. */
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Using coherency to get a coherent data set is not currently being supported");
}
