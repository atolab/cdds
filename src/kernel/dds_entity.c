#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_condition.h"
#include "kernel/dds_listener.h"
#include "os/os_report.h"



void dds_entity_add_ref (_In_ dds_entity * e)
{
    assert (e);
    os_mutexLock (&e->m_mutex);
    e->m_refc++;
    os_mutexUnlock (&e->m_mutex);
}

_Check_return_
bool dds_entity_cb_propagate_begin(_In_ dds_entity *e)
{
    bool ok = true;
    if (e) {
        /* Propagate lock to make sure the top is handled first. */
        ok = dds_entity_cb_propagate_begin(e->m_parent);

        if (ok) {
            os_mutexLock(&e->m_mutex);
            if (e->m_flags & DDS_ENTITY_DELETED) {
                /* Entity deletion in progress: break off the callback process. */
                ok = false;
            } else {
                /* Indicate that a callback will be in progress, so that a parallel
                 * delete/set_listener will wait. */
                e->m_cb_count++;
            }
            os_mutexUnlock(&e->m_mutex);
            if (!ok) {
                /* Un-busy the top of the hierarchy. */
                dds_entity_cb_propagate_end(e->m_parent);
            }
        }
    }
    return ok;
}

void dds_entity_cb_propagate_end(_In_ dds_entity *e)
{
    if (e) {
        /* We started at the top, so when resetting we should start at the bottom. */
        os_mutexLock(&e->m_mutex);
        e->m_cb_count--;
        os_mutexUnlock(&e->m_mutex);

        /* If anybody is waiting, let them continue. */
        if ((e->m_cb_count == 0) && (e->m_cb_waiting > 0)) {
            os_condBroadcast (&e->m_cond);
        }

        /* Continue up the hierarchy. */
        dds_entity_cb_propagate_end(e->m_parent);
    }
}

bool dds_entity_cp_propagate_call(
        _In_ dds_entity *e,
        _In_ dds_entity *src,
        _In_ uint32_t status,
        _In_opt_ void *metrics,
        _In_ bool propagate)
{
    bool called = false;
    if (e) {
        dds_listener_t *l = (dds_listener_t *)(&e->m_listener);

        /* We should be busy at this point. */
        assert(e->m_cb_count > 0);

        switch (status) {
            case DDS_INCONSISTENT_TOPIC_STATUS:
                if (l->on_inconsistent_topic != DDS_LUNSET) {
                    l->on_inconsistent_topic(src, *((dds_inconsistent_topic_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_OFFERED_DEADLINE_MISSED_STATUS:
                if (l->on_offered_deadline_missed != DDS_LUNSET) {
                    l->on_offered_deadline_missed(src, *((dds_offered_deadline_missed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
                if (l->on_requested_deadline_missed != DDS_LUNSET) {
                    l->on_requested_deadline_missed(src, *((dds_requested_deadline_missed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS:
                if (l->on_offered_incompatible_qos != DDS_LUNSET) {
                    l->on_offered_incompatible_qos(src, *((dds_offered_incompatible_qos_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
                if (l->on_requested_incompatible_qos != DDS_LUNSET) {
                    l->on_requested_incompatible_qos(src, *((dds_requested_incompatible_qos_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SAMPLE_LOST_STATUS:
                if (l->on_sample_lost != DDS_LUNSET) {
                    l->on_sample_lost(src, *((dds_sample_lost_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SAMPLE_REJECTED_STATUS:
                if (l->on_sample_rejected != DDS_LUNSET) {
                    l->on_sample_rejected(src, *((dds_sample_rejected_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_DATA_ON_READERS_STATUS:
                if (l->on_data_on_readers != DDS_LUNSET) {
                    l->on_data_on_readers(src, l->arg);
                    called = true;
                }
                break;
            case DDS_DATA_AVAILABLE_STATUS:
                if (l->on_data_available != DDS_LUNSET) {
                    l->on_data_available(src, l->arg);
                    called = true;
                }
                break;
            case DDS_LIVELINESS_LOST_STATUS:
                if (l->on_liveliness_lost != DDS_LUNSET) {
                    l->on_liveliness_lost(src, *((dds_liveliness_lost_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_LIVELINESS_CHANGED_STATUS:
                if (l->on_liveliness_changed != DDS_LUNSET) {
                    l->on_liveliness_changed(src, *((dds_liveliness_changed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_PUBLICATION_MATCHED_STATUS:
                if (l->on_publication_matched != DDS_LUNSET) {
                    l->on_publication_matched(src, *((dds_publication_matched_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SUBSCRIPTION_MATCHED_STATUS:
                if (l->on_subscription_matched != DDS_LUNSET) {
                    l->on_subscription_matched(src, *((dds_subscription_matched_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            default: assert (0);
        }

        if (!called && propagate) {
            /* See if the parent is interested. */
            called = dds_entity_cp_propagate_call(e->m_parent, src, status, metrics, propagate);
        }
    }
    return called;
}


void dds_entity_cb_wait_lock (_In_ dds_entity_t e)
{
    os_mutexLock (&e->m_mutex);
    e->m_cb_waiting++;
    while (e->m_cb_count > 0) {
        os_condWait (&e->m_cond, &e->m_mutex);
    }
    e->m_cb_waiting--;
}

void dds_entity_cb_wait_unlock (_In_ dds_entity_t e)
{
    os_mutexUnlock(&e->m_mutex);
    /* Wake possible others. */
    if (e->m_cb_waiting > 0) {
        os_condBroadcast (&e->m_cond);
    }
}

void dds_entity_delete_signal (_In_ dds_entity_t e)
{
    /* Signal that clean up complete */
    os_mutexLock (&e->m_mutex);
    e->m_flags |= DDS_ENTITY_DDSI_DELETED;
    os_condSignal (&e->m_cond);
    os_mutexUnlock (&e->m_mutex);
}

void dds_entity_delete_wait (_In_ dds_entity_t e, _In_ struct thread_state1 * const thr)
{
    /* Wait for DDSI clean up to complete */
    os_mutexLock (&e->m_mutex);
    if ((e->m_flags & DDS_ENTITY_DDSI_DELETED) == 0) {
        thread_state_asleep (thr);
        do {
            os_condWait (&e->m_cond, &e->m_mutex);
        }
        while ((e->m_flags & DDS_ENTITY_DDSI_DELETED) == 0);
        thread_state_awake (thr);
    }
    os_mutexUnlock (&e->m_mutex);
}

void dds_entity_delete_impl (_In_ dds_entity_t e, _In_ bool child, _In_ bool recurse)
{
    dds_entity_t iter;
    dds_entity_t *iterp;
    dds_entity_t prev = NULL;

    dds_entity_cb_wait_lock(e);

    if (--e->m_refc != 0) {
        dds_entity_cb_wait_unlock(e);
        return;
    }

    e->m_flags |= DDS_ENTITY_DELETED;
    e->m_status_enable = 0;
    dds_listener_reset(&e->m_listener);

    dds_entity_cb_wait_unlock(e);

    /* Remove from parent */
    if (!child && e->m_parent) {
        os_mutexLock (&e->m_parent->m_mutex);
        iter = e->m_parent->m_children;
        while (iter) {
            if (iter == e) {
                /* Remove topic from participant extent */
                if (prev) {
                    prev->m_next = e->m_next;
                } else {
                    e->m_parent->m_children = e->m_next;
                }
                break;
            }
            prev = iter;
            iter = iter->m_next;
        }
        os_mutexUnlock (&e->m_parent->m_mutex);
    }

    /* Recursively delete children */
    if (recurse) {
        iterp = &e->m_children;
        while (*iterp != NULL) {
            prev = (*iterp);
            if (prev->m_kind == DDS_TYPE_TOPIC) {
                /* Skip the topic element */
                iterp = &(prev->m_next);
            } else {
                /* Remove entity from list */
                *iterp = prev->m_next;
                /* clear m_parent, otherwise prev may try to delete this object later */
                os_mutexLock (&prev->m_mutex);
                prev->m_parent = NULL;
                os_mutexUnlock (&prev->m_mutex);
                dds_entity_delete_impl (prev, true, true);
            }
        }

        /* TODO: Get proper reason and comment why the topics are deleted last. */
        while (e->m_children) {
            prev = e->m_children;
            assert (prev->m_kind == DDS_TYPE_TOPIC);
            e->m_children = prev->m_next;
            os_mutexLock (&prev->m_mutex);
            prev->m_parent = NULL;
            os_mutexUnlock (&prev->m_mutex);
            dds_entity_delete_impl (prev, true, true);
        }
    } else {
        /* Delete myself as parent from my children. */
        for (iter = e->m_children; iter != NULL; iter = iter->m_next) {
            os_mutexLock (&iter->m_mutex);
            iter->m_parent = NULL;
            os_mutexUnlock (&iter->m_mutex);
        }
    }

    /* Clean up */
    if (e->m_deriver.delete) {
        e->m_deriver.delete(e, recurse);
    }
    dds_qos_delete (e->m_qos);
    if (e->m_status) {
        dds_condition_delete (e->m_status);
    }
    os_condDestroy (&e->m_cond);
    os_mutexDestroy (&e->m_mutex);
    dds_free (e);
}

void dds_entity_init(
        _In_     dds_entity * e,
        _In_opt_ dds_entity * parent,
        _In_     dds_entity_kind_t kind,
        _In_opt_ dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener,
        _In_     uint32_t mask)
{
    assert (e);

    e->m_refc = 1;
    e->m_parent = parent;
    e->m_kind = kind;
    e->m_qos = qos;
    e->m_cb_waiting = 0;
    e->m_cb_count = 0;

    /* TODO: CHAM-96: Implement dynamic enabling of entity. */
    e->m_flags |= DDS_ENTITY_ENABLED;

    /* set the status enable based on kind */
    e->m_status_enable = mask;

    os_mutexInit (&e->m_mutex);
    os_condInit (&e->m_cond, &e->m_mutex);

    /* alloc status condition */
    e->m_status = dds_alloc(sizeof(dds_condition));
    e->m_status->m_kind = DDS_TYPE_COND_STATUS;
    e->m_status->m_lock = &e->m_mutex;

    if (parent) {
        e->m_domain = parent->m_domain;
        e->m_domainid = parent->m_domainid;
        e->m_participant = parent->m_participant;
        e->m_next = parent->m_children;
        parent->m_children = e;
    } else {
        assert (kind == DDS_TYPE_PARTICIPANT);
        e->m_participant = e;
    }

    if (listener) {
        dds_listener_copy(&e->m_listener, listener);
    } else {
        dds_listener_reset(&e->m_listener);
    }
}

dds_return_t dds_delete (_In_ dds_entity_t e)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    if (e) {
        dds_entity_delete_impl (e, false, true);
        return DDS_RETCODE_OK;
    }
    return DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
}

dds_entity_t dds_get_parent(_In_ dds_entity_t e)
{
    /* TODO: CHAM-104: Return actual errors when dds_entity_t became an handle iso a pointer (see header). */
    if (e > 0) {
        return (dds_entity_t)(e->m_parent);
    }
    return NULL;
}

dds_entity_t dds_get_participant(_In_ dds_entity_t e)
{
    /* TODO: CHAM-104: Return actual errors when dds_entity_t became an handle iso a pointer (see header). */
    if (e > 0) {
        return (dds_entity_t)(e->m_participant);
    }
    return NULL;
}

dds_return_t dds_get_children(
        _In_ dds_entity_t e,
        _Out_opt_ dds_entity_t *children,
        _In_ size_t size)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if (e > 0) {
        if (((children != NULL) && (size  > 0) && (size < INT32_MAX)) ||
            ((children == NULL) && (size == 0)) ){

            os_mutexLock(&e->m_mutex);
            ret = 0;
            dds_entity* iter = e->m_children;
            while (iter) {
                if ((size_t)ret < size) { /*To fix the warning of signed/unsigned mismatch, type casting is done for the variable 'ret'*/
                    children[ret] = (dds_entity_t)iter;
                }
                ret++;
                iter = iter->m_next;
            }
            os_mutexUnlock(&e->m_mutex);
        }
    }
    return ret;
}

dds_return_t dds_get_qos (_In_ dds_entity_t e, _Out_ dds_qos_t * qos)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (qos != NULL)) {
        os_mutexLock(&e->m_mutex);
        if (e->m_qos) {
            ret = dds_qos_copy (qos, e->m_qos);
        }
        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}

/* Interface called whenever a changeable qos is modified */
dds_return_t dds_set_qos (_In_ dds_entity_t e, _In_ const dds_qos_t * qos)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (qos != NULL)) {
        ret = DDS_RETCODE_OK;
        os_mutexLock(&e->m_mutex);
        if (e->m_deriver.set_qos) {
            ret = e->m_deriver.set_qos(e, qos, e->m_flags & DDS_ENTITY_ENABLED);
        } else {
            ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_ENTITY, DDS_ERR_M1);
        }
        if (ret == DDS_RETCODE_OK) {
            /* Remember this QoS. */
            if (e->m_qos == NULL) {
                e->m_qos = dds_qos_create();
            }
            ret = dds_qos_copy (e->m_qos, qos);
        }
        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_listener (_In_ dds_entity_t e, _Out_ dds_listener_t * listener)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (listener != NULL)) {
        ret = DDS_RETCODE_OK;
        dds_entity_cb_wait_lock(e);
        dds_listener_copy (listener, &e->m_listener);
        dds_entity_cb_wait_unlock(e);
    }
    return ret;
}

dds_return_t dds_set_listener (_In_ dds_entity_t e, _In_opt_ const dds_listener_t * listener)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if (e > 0) {
        ret = DDS_RETCODE_OK;
        dds_entity_cb_wait_lock(e);
        if (listener) {
            dds_listener_copy(&e->m_listener, listener);
        } else {
            dds_listener_reset(&e->m_listener);
        }
        dds_entity_cb_wait_unlock(e);
    }
    return ret;
}

dds_return_t dds_enable(_In_ dds_entity_t e)
{
    /* TODO: CHAM-104: Return more different errors when dds_entity_t became an handle iso a pointer (see header). */
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if (e > 0) {
        os_mutexLock(&e->m_mutex);
        ret = DDS_RETCODE_OK;
        if (e->m_flags & DDS_ENTITY_ENABLED) {
            /* Already enabled. */
        } else {
            /* TODO: CHAM-96: Really enable. */
            e->m_flags |= DDS_ENTITY_ENABLED;
            ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_ENTITY, 0);
        }

        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}







dds_return_t dds_get_status_changes (_In_ dds_entity_t e, _Out_ uint32_t *mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (mask != NULL)) {
        os_mutexLock (&e->m_mutex);
        *mask = e->m_status->m_trigger;
        os_mutexUnlock (&e->m_mutex);
        ret = DDS_RETCODE_OK;
    }
    return ret;
}

dds_return_t dds_get_enabled_status(_In_ dds_entity_t e, _Out_ uint32_t *mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (mask != NULL)) {
        os_mutexLock (&e->m_mutex);
        *mask = e->m_status_enable;
        os_mutexUnlock (&e->m_mutex);
        ret = DDS_RETCODE_OK;
    }
    return ret;
}


dds_return_t dds_set_enabled_status (_In_ dds_entity_t e, _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if (e > 0) {
        os_mutexLock (&e->m_mutex);
        if (e->m_deriver.validate_status) {
            ret = e->m_deriver.validate_status(mask);
            if (ret == DDS_RETCODE_OK) {
                e->m_status_enable = mask;
                e->m_status->m_trigger &= mask;
                if (e->m_deriver.propagate_status) {
                    ret = e->m_deriver.propagate_status(e, mask, true);
                }
            }
        } else {
            if (mask == 0) {
                ret = DDS_RETCODE_OK;
            }
        }
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}


/* Read status condition based on mask */
dds_return_t dds_read_status (_In_ dds_entity_t e, _Out_ uint32_t * status, _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (status != NULL)) {
        os_mutexLock (&e->m_mutex);
        if (e->m_deriver.validate_status) {
            ret = e->m_deriver.validate_status(mask);
            if (ret == DDS_RETCODE_OK) {
                *status = e->m_status->m_trigger & mask;
            }
        }
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}

/* Take and clear status condition based on mask */
dds_return_t dds_take_status (_In_ dds_entity_t e, _Out_ uint32_t * status, _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (status != NULL)) {
        os_mutexLock (&e->m_mutex);
        if (e->m_deriver.validate_status) {
            ret = e->m_deriver.validate_status(mask);
            if (ret == DDS_RETCODE_OK) {
                *status = e->m_status->m_trigger & mask;
                if (e->m_deriver.propagate_status) {
                    ret = e->m_deriver.propagate_status(e, *status, false);
                }
                e->m_status->m_trigger &= ~mask;
            }
        }
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}

void dds_entity_status_signal(_In_ dds_entity_t e)
{
    assert(e);
    os_mutexLock (&e->m_mutex);
    dds_cond_callback_signal(e->m_status);
    os_mutexUnlock (&e->m_mutex);
}

dds_return_t dds_get_domainid (_In_ dds_entity_t e, _Out_ dds_domainid_t *id)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (id != NULL)) {
        ret = DDS_RETCODE_OK;
        os_mutexLock (&e->m_mutex);
        *id = e->m_domainid;
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}

dds_return_t dds_instancehandle_get(_In_ dds_entity_t e, _Out_ dds_instance_handle_t *i)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (i != NULL)) {
        os_mutexLock (&e->m_mutex);
        if (e->m_deriver.get_instance_hdl) {
            ret = e->m_deriver.get_instance_hdl(e, i);
        }
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}
