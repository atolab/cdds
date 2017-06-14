#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_condition.h"
#include "kernel/dds_listener.h"
#include "os/os_report.h"

/* Sanity check. */
#if DDS_ENTITY_KIND_MASK != UT_HANDLE_KIND_MASK
#error "DDS_ENTITY_KIND_MASK != UT_HANDLE_KIND_MASK"
#endif


void
dds_entity_add_ref (_In_ dds_entity * e)
{
    assert (e);
    os_mutexLock (&e->m_mutex);
    e->m_refc++;
    os_mutexUnlock (&e->m_mutex);
}

_Check_return_ bool
dds_entity_cb_propagate_begin(_In_ dds_entity *e)
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

void
dds_entity_cb_propagate_end(_In_ dds_entity *e)
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

_Check_return_ bool
dds_entity_cp_propagate_call(
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
                    assert(metrics);
                    l->on_inconsistent_topic(src->m_hdl, *((dds_inconsistent_topic_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_OFFERED_DEADLINE_MISSED_STATUS:
                if (l->on_offered_deadline_missed != DDS_LUNSET) {
                    assert(metrics);
                    l->on_offered_deadline_missed(src->m_hdl, *((dds_offered_deadline_missed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
                if (l->on_requested_deadline_missed != DDS_LUNSET) {
                    assert(metrics);
                    l->on_requested_deadline_missed(src->m_hdl, *((dds_requested_deadline_missed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS:
                if (l->on_offered_incompatible_qos != DDS_LUNSET) {
                    assert(metrics);
                    l->on_offered_incompatible_qos(src->m_hdl, *((dds_offered_incompatible_qos_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
                if (l->on_requested_incompatible_qos != DDS_LUNSET) {
                    assert(metrics);
                    l->on_requested_incompatible_qos(src->m_hdl, *((dds_requested_incompatible_qos_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SAMPLE_LOST_STATUS:
                if (l->on_sample_lost != DDS_LUNSET) {
                    assert(metrics);
                    l->on_sample_lost(src->m_hdl, *((dds_sample_lost_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SAMPLE_REJECTED_STATUS:
                if (l->on_sample_rejected != DDS_LUNSET) {
                    assert(metrics);
                    l->on_sample_rejected(src->m_hdl, *((dds_sample_rejected_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_DATA_ON_READERS_STATUS:
                if (l->on_data_on_readers != DDS_LUNSET) {
                    l->on_data_on_readers(src->m_hdl, l->arg);
                    called = true;
                }
                break;
            case DDS_DATA_AVAILABLE_STATUS:
                if (l->on_data_available != DDS_LUNSET) {
                    l->on_data_available(src->m_hdl, l->arg);
                    called = true;
                }
                break;
            case DDS_LIVELINESS_LOST_STATUS:
                if (l->on_liveliness_lost != DDS_LUNSET) {
                    assert(metrics);
                    l->on_liveliness_lost(src->m_hdl, *((dds_liveliness_lost_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_LIVELINESS_CHANGED_STATUS:
                if (l->on_liveliness_changed != DDS_LUNSET) {
                    assert(metrics);
                    l->on_liveliness_changed(src->m_hdl, *((dds_liveliness_changed_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_PUBLICATION_MATCHED_STATUS:
                if (l->on_publication_matched != DDS_LUNSET) {
                    assert(metrics);
                    l->on_publication_matched(src->m_hdl, *((dds_publication_matched_status_t*)metrics), l->arg);
                    called = true;
                }
                break;
            case DDS_SUBSCRIPTION_MATCHED_STATUS:
                if (l->on_subscription_matched != DDS_LUNSET) {
                    assert(metrics);
                    l->on_subscription_matched(src->m_hdl, *((dds_subscription_matched_status_t*)metrics), l->arg);
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


static void
dds_entity_cb_wait (_In_ dds_entity *e)
{
    e->m_cb_waiting++;
    while (e->m_cb_count > 0) {
        os_condWait (&e->m_cond, &e->m_mutex);
    }
    e->m_cb_waiting--;
}

static void
dds_entity_cb_wait_signal (_In_ dds_entity *e)
{
    /* Wake possible others. */
    if (e->m_cb_waiting > 0) {
        os_condBroadcast (&e->m_cond);
    }
}

void
dds_entity_delete_signal (_In_ dds_entity *e)
{
    /* Signal that clean up complete */
    os_mutexLock (&e->m_mutex);
    e->m_flags |= DDS_ENTITY_DDSI_DELETED;
    os_condSignal (&e->m_cond);
    os_mutexUnlock (&e->m_mutex);
}

void
dds_entity_delete_wait (_In_ dds_entity *e, _In_ struct thread_state1 * const thr)
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

void
dds_entity_delete_impl (_In_ dds_entity *e, _In_ bool child, _In_ bool recurse)
{
    os_time    timeout = { 10, 0 };
    dds_entity *iter;
    dds_entity **iterp;
    dds_entity *prev = NULL;

    if (dds_entity_lock(e->m_hdl, UT_HANDLE_DONTCARE_KIND, &e) != DDS_RETCODE_OK) {
        /* TODO: Trace deletion error. */
        return;
    }

    if (--e->m_refc != 0) {
        dds_entity_unlock(e);
        return;
    }

    dds_entity_cb_wait(e);

    ut_handle_close(e->m_hdl, e->m_hdllink);
    e->m_flags |= DDS_ENTITY_DELETED;
    e->m_status_enable = 0;
    dds_listener_reset(&e->m_listener);

    dds_entity_unlock(e);
    dds_entity_cb_wait_signal(e);

    if (e->m_deriver.close) {
        e->m_deriver.close(e, recurse);
    }

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
            if (dds_entity_kind(prev->m_hdl) == DDS_KIND_TOPIC) {
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
            assert (dds_entity_kind(prev->m_hdl) == DDS_KIND_TOPIC);
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
    if (ut_handle_delete(e->m_hdl, e->m_hdllink, timeout) == UT_HANDLE_OK) {
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
    } else {
        /* TODO: report deletion timeout error. */
    }
}

_Check_return_ dds_entity_t
dds_entity_init(
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
        assert (kind == DDS_KIND_PARTICIPANT);
        e->m_participant = e;
    }

    if (listener) {
        dds_listener_copy(&e->m_listener, listener);
    } else {
        dds_listener_reset(&e->m_listener);
    }

    e->m_hdllink = NULL;
    e->m_hdl = ut_handle_create((int32_t)kind, e);
    if (e->m_hdl > 0) {
        e->m_hdllink = ut_handle_get_link(e->m_hdl);
        assert(e->m_hdllink);
    }

    /* An ut_handle_t is directly used as dds_entity_t. */
    return (dds_entity_t)(e->m_hdl);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
dds_return_t
dds_delete(
        _In_ dds_entity_t entity)
{
    dds_entity *e;
    dds_return_t ret;
    ret = dds_entity_lock(entity, UT_HANDLE_DONTCARE_KIND, &e);
    if (ret == DDS_RETCODE_OK) {
        /* Unlock to make sure that other actions will finish before
         * continuing the deletion. */
        dds_entity_unlock(e);
        dds_entity_delete_impl (e, false, true);
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, 0);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_entity_t
dds_get_parent(
        _In_ dds_entity_t entity)
{
    dds_entity *e;
    dds_entity_t hdl = entity;
    int32_t errnr = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (errnr == DDS_RETCODE_OK) {
        if (e->m_parent) {
            hdl = e->m_parent->m_hdl;
        } else {
            hdl = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_ENTITY, 0);
        }
        dds_entity_unlock(e);
    } else {
        hdl = DDS_ERRNO(errnr, DDS_MOD_ENTITY, 0);
    }
    return hdl;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_entity_t
dds_get_participant (
        _In_ dds_entity_t entity)
{
    dds_entity *e;
    dds_entity_t hdl = entity;
    int32_t errnr = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (errnr == DDS_RETCODE_OK) {
        if (e->m_participant) {
            hdl = e->m_participant->m_hdl;
        } else {
            hdl = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_ENTITY, 0);
        }
        dds_entity_unlock(e);
    } else {
        hdl = DDS_ERRNO(errnr, DDS_MOD_ENTITY, 0);
    }
    return hdl;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_children(
        _In_        dds_entity_t entity,
        _Inout_opt_ dds_entity_t *children,
        _In_        size_t size)
{
    dds_entity *e;
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (((children != NULL) && (size  > 0) && (size < INT32_MAX)) ||
        ((children == NULL) && (size == 0)) ){
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            ret = 0;
            dds_entity* iter = e->m_children;
            while (iter) {
                if ((size_t)ret < size) { /*To fix the warning of signed/unsigned mismatch, type casting is done for the variable 'ret'*/
                    children[ret] = iter->m_hdl;
                }
                ret++;
                iter = iter->m_next;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_qos(
        _In_    dds_entity_t entity,
        _Inout_ dds_qos_t *qos)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if ((entity > 0) && (qos != NULL)) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.set_qos) {
                ret = dds_qos_copy (qos, e->m_qos);
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

/* Interface called whenever a changeable qos is modified */
_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_set_qos(
        _In_ dds_entity_t entity,
        _In_ const dds_qos_t * qos)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (qos != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.set_qos) {
                ret = e->m_deriver.set_qos(e, qos, e->m_flags & DDS_ENTITY_ENABLED);
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            if (ret == DDS_RETCODE_OK) {
                /* Remember this QoS. */
                if (e->m_qos == NULL) {
                    e->m_qos = dds_qos_create();
                }
                ret = dds_qos_copy (e->m_qos, qos);
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_listener(
        _In_ dds_entity_t entity,
        _Inout_ dds_listener_t * listener)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (listener != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            dds_entity_cb_wait(e);
            dds_listener_copy (listener, &e->m_listener);
            dds_entity_cb_wait_signal(e);
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_set_listener(
        _In_     dds_entity_t entity,
        _In_opt_ const dds_listener_t * listener)
{
    dds_return_t ret;
    dds_entity *e;
    ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
        dds_entity_cb_wait(e);
        if (listener) {
            dds_listener_copy(&e->m_listener, listener);
        } else {
            dds_listener_reset(&e->m_listener);
        }
        dds_entity_cb_wait_signal(e);
        dds_entity_unlock(e);
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_enable(
        _In_ dds_entity_t entity)
{
    dds_return_t ret;
    dds_entity *e;
    ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
        if ((e->m_flags & DDS_ENTITY_ENABLED) == 0) {
            /* TODO: CHAM-96: Really enable. */
            e->m_flags |= DDS_ENTITY_ENABLED;
            ret = DDS_RETCODE_UNSUPPORTED;
        }
        dds_entity_unlock(e);
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}


_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_status_changes(
        _In_    dds_entity_t entity,
        _Inout_ uint32_t *status)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                *status = e->m_status->m_trigger;
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_enabled_status(
        _In_    dds_entity_t entity,
        _Inout_ uint32_t *status)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                *status = e->m_status_enable;
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}


_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
DDS_EXPORT dds_return_t
dds_set_enabled_status(
        _In_ dds_entity_t entity,
        _In_ uint32_t mask)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    dds_entity *e;
    ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
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
            ret = DDS_RETCODE_ILLEGAL_OPERATION;
        }
        dds_entity_unlock(e);
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}


/* Read status condition based on mask */
_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_read_status(
        _In_    dds_entity_t entity,
        _Inout_ uint32_t *status,
        _In_    uint32_t mask)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                ret = e->m_deriver.validate_status(mask);
                if (ret == DDS_RETCODE_OK) {
                    *status = e->m_status->m_trigger & mask;
                }
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

/* Take and clear status condition based on mask */
_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_take_status(
        _In_    dds_entity_t entity,
        _Inout_ uint32_t *status,
        _In_    uint32_t mask)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                ret = e->m_deriver.validate_status(mask);
                if (ret == DDS_RETCODE_OK) {
                    *status = e->m_status->m_trigger & mask;
                    if (e->m_deriver.propagate_status) {
                        ret = e->m_deriver.propagate_status(e, *status, false);
                    }
                    e->m_status->m_trigger &= ~mask;
                }
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

void
dds_entity_status_signal(_In_ dds_entity *e)
{
    assert(e);
    os_mutexLock (&e->m_mutex);
    dds_cond_callback_signal(e->m_status);
    os_mutexUnlock (&e->m_mutex);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_domainid(
        _In_    dds_entity_t entity,
        _Inout_ dds_domainid_t *id)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (id != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            *id = e->m_domainid;
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_instancehandle_get(
        _In_    dds_entity_t entity,
        _Inout_ dds_instance_handle_t *ihdl)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (ihdl != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.get_instance_hdl) {
                ret = e->m_deriver.get_instance_hdl(e, ihdl);
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Check_return_ int32_t
dds_entity_lock(_In_ dds_entity_t hdl, _In_ dds_entity_kind_t kind, _Out_ dds_entity **e)
{
    ut_handle_t utr;
    assert(e);
    utr = ut_handle_claim(hdl, NULL, kind, (void**)e);
    if (utr == UT_HANDLE_OK) {
        os_mutexLock(&((*e)->m_mutex));
        /* The handle could have been closed while we were waiting for the mutex. */
        if (ut_handle_is_closed(hdl, (*e)->m_hdllink)) {
            os_mutexUnlock(&((*e)->m_mutex));
            utr = UT_HANDLE_CLOSED;
        }
    }
    return((utr == UT_HANDLE_OK)           ? DDS_RETCODE_OK                :
           (utr == UT_HANDLE_UNEQUAL_KIND) ? DDS_RETCODE_ILLEGAL_OPERATION :
           (utr == UT_HANDLE_INVALID)      ? DDS_RETCODE_BAD_PARAMETER     :
           (utr == UT_HANDLE_DELETED)      ? DDS_RETCODE_ALREADY_DELETED   :
           (utr == UT_HANDLE_CLOSED)       ? DDS_RETCODE_ALREADY_DELETED   :
                                             DDS_RETCODE_ERROR             );
}

void dds_entity_unlock(_In_ dds_entity *e)
{
    assert(e);
    os_mutexUnlock(&e->m_mutex);
    ut_handle_release(e->m_hdl, e->m_hdllink);
}
