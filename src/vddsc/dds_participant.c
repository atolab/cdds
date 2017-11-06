#include <assert.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_config.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_init.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"
#include "kernel/dds_err.h"
#include "kernel/dds_report.h"

#define DDS_PARTICIPANT_STATUS_MASK    0

/* List of created participants */

static dds_entity * dds_pp_head = NULL;

static dds_return_t
dds_participant_status_validate(
        uint32_t mask)
{
    return (mask & ~(DDS_PARTICIPANT_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Argument mask is invalid") :
                     DDS_RETCODE_OK;
}

static dds_return_t
dds_participant_delete(
        dds_entity *e)
{
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);
    dds_entity *prev = NULL;
    dds_entity *iter;

    assert(e);
    assert(thr);
    assert(dds_entity_kind(e->m_hdl) == DDS_KIND_PARTICIPANT);

    if (asleep) {
      thread_state_awake(thr);
    }

    dds_domain_free (e->m_domain);

    os_mutexLock (&dds_global.m_mutex);
    iter = dds_pp_head;
    while (iter) {
        if (iter == e) {
            if (prev) {
                prev->m_next = iter->m_next;
            } else {
                  dds_pp_head = iter->m_next;
            }
            break;
        }
        prev = iter;
        iter = iter->m_next;
    }
    os_mutexUnlock (&dds_global.m_mutex);

    assert (iter);

    if (asleep) {
      thread_state_asleep(thr);
    }

    /* Finalize the dds layer when this was the last participant. */
    if(dds_pp_head == NULL){
        dds_fini();
    }

    return DDS_RETCODE_OK;
}

static dds_return_t
dds_participant_instance_hdl(
        dds_entity *e,
        dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    *i = (dds_instance_handle_t)participant_instance_id(&e->m_guid);
    return DDS_RETCODE_OK;
}

static dds_return_t
dds_participant_qos_validate(
        const dds_qos_t *qos,
        OS_UNUSED_PAR(bool enabled))
{
    dds_return_t ret = DDS_RETCODE_OK;
    assert(qos);

    /* Check consistency. */
    if ((qos->present & QP_USER_DATA) && !validate_octetseq(&qos->user_data)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "User data QoS policy is inconsistent and caused an error");
    }
    if ((qos->present & QP_PRISMTECH_ENTITY_FACTORY) && !validate_entityfactory_qospolicy(&qos->entity_factory)) {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY, "Prismtech entity factory QoS policy is inconsistent and caused an error");
    }
    return ret;
}


static dds_return_t
dds_participant_qos_set(
        OS_UNUSED_PAR(dds_entity *e),
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = dds_participant_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, "Changing the participant QoS is not supported.");
        }
    }
    return ret;
}

_Must_inspect_result_ dds_entity_t
dds_create_participant(
        _In_     const dds_domainid_t domain,
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
    int q_rc;
    dds_return_t ret;
    dds_entity_t e;
    nn_guid_t guid;
    dds_participant * pp;
    nn_plist_t plist;
    dds_qos_t * new_qos = NULL;
    struct thread_state1 * thr;
    bool asleep;

    /* Initialize the dds layer when this is the first participant. */
    if (dds_pp_head == NULL) {
        ret = dds_init();
        if (ret != DDS_RETCODE_OK) {
            e = DDS_ERRNO(ret, "Initialization of DDS layer is failed");
            goto fail;
        }
    }

    /* Report stack is only useful after dds (and thus os) init. */
    DDS_REPORT_STACK();

    nn_plist_init_empty (&plist);

    /* Check domain id */
    ret = dds_init_impl (domain);
    if (ret != DDS_RETCODE_OK) {
        e = (dds_entity_t)ret;
        goto fail;
    }

    /* Validate qos */
    if (qos) {
        ret = dds_participant_qos_validate (qos, false);
        if (ret != DDS_RETCODE_OK) {
            e = (dds_entity_t)ret;
            goto fail;
        }
        new_qos = dds_qos_create ();
        /* Only returns failure when one of the qos args is NULL, which
         * is not the case here. */
        (void)dds_qos_copy(new_qos, qos);
        dds_qos_merge (&plist.qos, new_qos);
    } else {
        /* Use default qos. */
        new_qos = dds_qos_create ();
    }

    thr = lookup_thread_state ();
    asleep = !vtime_awake_p (thr->vtime);
    if (asleep) {
        thread_state_awake (thr);
    }
    q_rc = new_participant (&guid, 0, &plist);
    if (asleep) {
        thread_state_asleep (thr);
    }

    if (q_rc != 0) {
        dds_qos_delete(new_qos);
        e = DDS_ERRNO(DDS_RETCODE_ERROR, "Internal error");
        goto fail;
    }

    pp = dds_alloc (sizeof (*pp));
    e = dds_entity_init (&pp->m_entity, NULL, DDS_KIND_PARTICIPANT, new_qos, listener, DDS_PARTICIPANT_STATUS_MASK);
    if (e < 0) {
        dds_qos_delete(new_qos);
        goto fail;
    }

    pp->m_entity.m_guid = guid;
    pp->m_entity.m_domain = dds_domain_create (config.domainId);
    pp->m_entity.m_domainid = config.domainId;
    pp->m_entity.m_deriver.delete = dds_participant_delete;
    pp->m_entity.m_deriver.set_qos = dds_participant_qos_set;
    pp->m_entity.m_deriver.get_instance_hdl = dds_participant_instance_hdl;
    pp->m_entity.m_deriver.validate_status = dds_participant_status_validate;
    pp->m_builtin_subscriber = 0;

    /* Add participant to extent */
    os_mutexLock (&dds_global.m_mutex);
    pp->m_entity.m_next = dds_pp_head;
    dds_pp_head = &pp->m_entity;
    os_mutexUnlock (&dds_global.m_mutex);

fail:
    nn_plist_fini (&plist);
    if (dds_pp_head == NULL) {
        dds_fini();
    }
    DDS_REPORT_FLUSH( e <= 0);
    return e;
}

_Check_return_ dds_return_t
dds_lookup_participant(
        _In_        dds_domainid_t domain_id,
        _Out_opt_   dds_entity_t *participants,
        _In_        size_t size)
{
    dds_return_t ret = 0;

    DDS_REPORT_STACK();

    if ((participants != NULL) && ((size <= 0) || (size >= INT32_MAX))) {
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Array is given, but with invalid size");
        goto err;
    }
    if ((participants == NULL) && (size != 0)) {
        ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, "Size is given, but no array");
        goto err;
    }

    dds_entity* iter;
    if(participants){
        participants[0] = 0;
    }
    iter = dds_pp_head;
    while (iter) {
        if(iter->m_domainid == domain_id) {
            if((size_t)ret < size) {
                participants[ret] = iter->m_hdl;
            }
            ret ++;
        }
        iter = iter->m_next;
    }

err:
    DDS_REPORT_FLUSH(ret != DDS_RETCODE_OK);
    return ret;
}
