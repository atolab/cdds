#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_listener.h"
#include "os/os_report.h"

/* Sanity check. */
#if DDS_ENTITY_KIND_MASK != UT_HANDLE_KIND_MASK
#error "DDS_ENTITY_KIND_MASK != UT_HANDLE_KIND_MASK"
#endif

static void
dds_entity_observers_delete(
        _In_ dds_entity *observed);

static void
dds_entity_observers_signal(
        _In_ dds_entity *observed,
        _In_ uint32_t status);

void
dds_entity_add_ref(_In_ dds_entity * e)
{
    assert (e);
    os_mutexLock (&e->m_mutex);
    e->m_refc++;
    os_mutexUnlock (&e->m_mutex);
}

void
dds_entity_add_ref_nolock(_In_ dds_entity *e)
{
    assert(e);
    e->m_refc++;
}

_Check_return_ dds_return_t
dds_entity_listener_propagation(
        _In_ dds_entity *e,
        _In_ dds_entity *src,
        _In_ uint32_t status,
        _In_opt_ void *metrics,
        _In_ bool propagate)
{
    dds_return_t ret = DDS_RETCODE_NO_DATA; /* Mis-use NO_DATA as NO_CALL. */
    dds_entity *dummy;
    /* e will be NULL when reaching the top of the entity hierarchy. */
    if (e) {
        ret = dds_entity_lock(e->m_hdl, DDS_KIND_DONTCARE, &dummy);
        if (ret == DDS_RETCODE_OK) {
            dds_listener_t *l = (dds_listener_t *)(&e->m_listener);

            assert(e == dummy);

            /* Indicate that a callback will be in progress, so that a parallel
             * delete/set_listener will wait. */
            e->m_cb_count++;

            /* Calling the actual listener should be done unlocked. */
            dds_entity_unlock(e);

            /* Now, perform the callback when available. */
            ret = DDS_RETCODE_NO_DATA;
            switch (status) {
                case DDS_INCONSISTENT_TOPIC_STATUS:
                    if (l->on_inconsistent_topic != DDS_LUNSET) {
                        assert(metrics);
                        l->on_inconsistent_topic(src->m_hdl, *((dds_inconsistent_topic_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_OFFERED_DEADLINE_MISSED_STATUS:
                    if (l->on_offered_deadline_missed != DDS_LUNSET) {
                        assert(metrics);
                        l->on_offered_deadline_missed(src->m_hdl, *((dds_offered_deadline_missed_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
                    if (l->on_requested_deadline_missed != DDS_LUNSET) {
                        assert(metrics);
                        l->on_requested_deadline_missed(src->m_hdl, *((dds_requested_deadline_missed_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS:
                    if (l->on_offered_incompatible_qos != DDS_LUNSET) {
                        assert(metrics);
                        l->on_offered_incompatible_qos(src->m_hdl, *((dds_offered_incompatible_qos_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
                    if (l->on_requested_incompatible_qos != DDS_LUNSET) {
                        assert(metrics);
                        l->on_requested_incompatible_qos(src->m_hdl, *((dds_requested_incompatible_qos_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_SAMPLE_LOST_STATUS:
                    if (l->on_sample_lost != DDS_LUNSET) {
                        assert(metrics);
                        l->on_sample_lost(src->m_hdl, *((dds_sample_lost_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_SAMPLE_REJECTED_STATUS:
                    if (l->on_sample_rejected != DDS_LUNSET) {
                        assert(metrics);
                        l->on_sample_rejected(src->m_hdl, *((dds_sample_rejected_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_DATA_ON_READERS_STATUS:
                    if (l->on_data_on_readers != DDS_LUNSET) {
                        l->on_data_on_readers(src->m_hdl, l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_DATA_AVAILABLE_STATUS:
                    if (l->on_data_available != DDS_LUNSET) {
                        l->on_data_available(src->m_hdl, l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_LIVELINESS_LOST_STATUS:
                    if (l->on_liveliness_lost != DDS_LUNSET) {
                        assert(metrics);
                        l->on_liveliness_lost(src->m_hdl, *((dds_liveliness_lost_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_LIVELINESS_CHANGED_STATUS:
                    if (l->on_liveliness_changed != DDS_LUNSET) {
                        assert(metrics);
                        l->on_liveliness_changed(src->m_hdl, *((dds_liveliness_changed_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_PUBLICATION_MATCHED_STATUS:
                    if (l->on_publication_matched != DDS_LUNSET) {
                        assert(metrics);
                        l->on_publication_matched(src->m_hdl, *((dds_publication_matched_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                case DDS_SUBSCRIPTION_MATCHED_STATUS:
                    if (l->on_subscription_matched != DDS_LUNSET) {
                        assert(metrics);
                        l->on_subscription_matched(src->m_hdl, *((dds_subscription_matched_status_t*)metrics), l->arg);
                        ret = DDS_RETCODE_OK;
                    }
                    break;
                default: assert (0);
            }
            if ((ret == DDS_RETCODE_NO_DATA) && propagate) {
                /* See if the parent is interested. */
                ret = dds_entity_listener_propagation(e->m_parent, src, status, metrics, propagate);
            }

            os_mutexLock(&(e->m_mutex));
            /* We are done with our callback. */
            e->m_cb_count--;
            /* Wake up possible waiting threads. */
            os_condBroadcast(&e->m_cond);
            os_mutexUnlock(&(e->m_mutex));
        }
    }
    return ret;
}


static void
dds_entity_cb_wait (_In_ dds_entity *e)
{
    while (e->m_cb_count > 0) {
        os_condWait (&e->m_cond, &e->m_mutex);
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
    e->m_cb_count = 0;
    e->m_observers = NULL;
    e->m_trigger = 0;

    /* TODO: CHAM-96: Implement dynamic enabling of entity. */
    e->m_flags |= DDS_ENTITY_ENABLED;

    /* set the status enable based on kind */
    e->m_status_enable = mask | DDS_INTERNAL_STATUS_MASK;

    os_mutexInit (&e->m_mutex);
    os_condInit (&e->m_cond, &e->m_mutex);

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
    os_time    timeout = { 10, 0 };
    dds_entity *e;
    dds_entity *child;
    dds_entity *prev = NULL;
    dds_entity *next = NULL;
    dds_return_t ret;

    ret = dds_entity_lock(entity, UT_HANDLE_DONTCARE_KIND, &e);
    if (ret != DDS_RETCODE_OK) {
        return DDS_ERRNO(ret, DDS_MOD_ENTITY, 0);
    }

    if (--e->m_refc != 0) {
        dds_entity_unlock(e);
        return DDS_RETCODE_OK;
    }

    dds_entity_cb_wait(e);

    ut_handle_close(e->m_hdl, e->m_hdllink);
    e->m_status_enable = 0;
    dds_listener_reset(&e->m_listener);
    e->m_trigger |= DDS_DELETING_STATUS;

    dds_entity_unlock(e);

    /* Signal observers that this entity will be deleted. */
    dds_entity_status_signal(e);

    /* Recursively delete children */
    child = e->m_children;
    while ((child != NULL) && (ret == DDS_RETCODE_OK)) {
        next = child->m_next;
        /* This will probably delete the child entry from
         * the current childrens list */
        ret = dds_delete(child->m_hdl);
        /* Next child. */
        child = next;
    }

    if (ret == DDS_RETCODE_OK) {
        /* Close the entity. This can terminate threads or kick of
         * other destroy stuff that takes a while. */
        if (e->m_deriver.close) {
            ret = e->m_deriver.close(e);
        }
    }

    if (ret == DDS_RETCODE_OK) {
        /* The ut_handle_delete will wait until the last active claim on that handle
         * is released. It is possible that this last release will be done by a thread
         * that was kicked during the close(). */
        if (ut_handle_delete(e->m_hdl, e->m_hdllink, timeout) != UT_HANDLE_OK) {
            ret = DDS_RETCODE_TIMEOUT;
        }
    }

    if (ret == DDS_RETCODE_OK) {
        /* Remove all possible observers. */
        dds_entity_observers_delete(e);

        /* Remove from parent */
        if (e->m_parent) {
            os_mutexLock (&e->m_parent->m_mutex);
            child = e->m_parent->m_children;
            while (child) {
                if (child == e) {
                    if (prev) {
                        prev->m_next = e->m_next;
                    } else {
                        e->m_parent->m_children = e->m_next;
                    }
                    break;
                }
                prev = child;
                child = child->m_next;
            }
            os_mutexUnlock (&e->m_parent->m_mutex);
        }

        /* Do some specific deletion when needed. */
        if (e->m_deriver.delete) {
            ret = e->m_deriver.delete(e);
        }
    }

    if (ret == DDS_RETCODE_OK) {
        /* Destroy last few things. */
        dds_qos_delete (e->m_qos);
        os_condDestroy (&e->m_cond);
        os_mutexDestroy (&e->m_mutex);
        dds_free (e);
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
        _Out_opt_   dds_entity_t *children,
        _In_        size_t size)
{
    dds_entity *e;
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, DDS_ERR_M2);
    if (((children != NULL) && (size  > 0) && (size < INT32_MAX)) ||
        ((children == NULL) && (size == 0)) ){
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            dds_entity* iter;
            /* Initialize first child to satisfy SAL. */
            if (children) {
                children[0] = 0;
            }
            ret = 0;
            iter = e->m_children;
            while (iter) {
                if ((size_t)ret < size) { /*To fix the warning of signed/unsigned mismatch, type casting is done for the variable 'ret'*/
                    children[ret] = iter->m_hdl;
                }
                ret++;
                iter = iter->m_next;
            }
            dds_entity_unlock(e);
        } else {
            ret = DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
        }
    }
    return ret;
}



_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_qos(
        _In_  dds_entity_t entity,
        _Out_ dds_qos_t *qos)
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
        _In_  dds_entity_t   entity,
        _Out_ dds_listener_t *listener)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (listener != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            dds_entity_cb_wait(e);
            dds_listener_copy (listener, &e->m_listener);
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
        _Out_   uint32_t *status)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                *status = e->m_trigger;
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
        _Out_   uint32_t *status)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.validate_status) {
                *status = (e->m_status_enable & ~DDS_INTERNAL_STATUS_MASK);
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
                /* Don't block internal status triggers. */
                mask |= DDS_INTERNAL_STATUS_MASK;
                e->m_status_enable = mask;
                e->m_trigger &= mask;
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



_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_read_status(
        _In_    dds_entity_t entity,
        _Out_   uint32_t *status,
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
                    *status = e->m_trigger & mask;
                }
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
dds_take_status(
        _In_    dds_entity_t entity,
        _Out_   uint32_t *status,
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
                    *status = e->m_trigger & mask;
                    if (e->m_deriver.propagate_status) {
                        ret = e->m_deriver.propagate_status(e, *status, false);
                    }
                    e->m_trigger &= ~mask;
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
dds_entity_status_signal(
        _In_ dds_entity *e)
{
    assert(e);
    /* Signal the observers in an unlocked state.
     * This is safe because we use handles and the current entity
     * will not be deleted while we're in this signaling state
     * due to a claimed handle. */
    dds_entity_observers_signal(e, e->m_trigger);
}



_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
_Check_return_ dds_return_t
dds_get_domainid(
        _In_    dds_entity_t entity,
        _Out_   dds_domainid_t *id)
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
        _Out_   dds_instance_handle_t *ihdl)
{
    dds_return_t ret = DDS_RETCODE_BAD_PARAMETER;
    if (ihdl != NULL) {
        dds_entity *e;
        ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
        if (ret == DDS_RETCODE_OK) {
            if (e->m_deriver.get_instance_hdl) {
                ret = e->m_deriver.get_instance_hdl(e, ihdl);
            } else {
                ret = DDS_RETCODE_ILLEGAL_OPERATION;
            }
            dds_entity_unlock(e);
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M2);
}

_Check_return_ dds_return_t
dds_valid_hdl(
        _In_ dds_entity_t hdl,
        _In_ dds_entity_kind_t kind)
{
    dds_return_t ret = hdl;
    ut_handle_t utr;
    if (hdl >= 0) {
        utr = ut_handle_status(hdl, NULL, kind);
        ret = ((utr == UT_HANDLE_OK)           ? DDS_RETCODE_OK                :
               (utr == UT_HANDLE_UNEQUAL_KIND) ? DDS_RETCODE_ILLEGAL_OPERATION :
               (utr == UT_HANDLE_INVALID)      ? DDS_RETCODE_BAD_PARAMETER     :
               (utr == UT_HANDLE_DELETED)      ? DDS_RETCODE_ALREADY_DELETED   :
               (utr == UT_HANDLE_CLOSED)       ? DDS_RETCODE_ALREADY_DELETED   :
                                                 DDS_RETCODE_ERROR             );
    }
    return ret;
}

_Check_return_ dds_return_t
dds_entity_lock(
        _In_ dds_entity_t hdl,
        _In_ dds_entity_kind_t kind,
        _Out_ dds_entity **e)
{
    dds_return_t ret = hdl;
    ut_handle_t utr;
    assert(e);
    if (hdl >= 0) {
        utr = ut_handle_claim(hdl, NULL, kind, (void**)e);
        if (utr == UT_HANDLE_OK) {
            os_mutexLock(&((*e)->m_mutex));
            /* The handle could have been closed while we were waiting for the mutex. */
            if (ut_handle_is_closed(hdl, (*e)->m_hdllink)) {
                os_mutexUnlock(&((*e)->m_mutex));
                utr = UT_HANDLE_CLOSED;
            }
        }
        ret = ((utr == UT_HANDLE_OK)           ? DDS_RETCODE_OK                :
               (utr == UT_HANDLE_UNEQUAL_KIND) ? DDS_RETCODE_ILLEGAL_OPERATION :
               (utr == UT_HANDLE_INVALID)      ? DDS_RETCODE_BAD_PARAMETER     :
               (utr == UT_HANDLE_DELETED)      ? DDS_RETCODE_ALREADY_DELETED   :
               (utr == UT_HANDLE_CLOSED)       ? DDS_RETCODE_ALREADY_DELETED   :
                                                 DDS_RETCODE_ERROR             );
    }
    return ret;
}



void
dds_entity_unlock(
        _In_ dds_entity *e)
{
    assert(e);
    os_mutexUnlock(&e->m_mutex);
    ut_handle_release(e->m_hdl, e->m_hdllink);
}



_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
dds_return_t
dds_triggered(
        _In_ dds_entity_t entity)
{
    dds_return_t ret;
    dds_entity *e;
    ret = dds_entity_lock(entity, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
        ret = (e->m_trigger != 0);
        dds_entity_unlock(e);
    } else {
        ret = DDS_ERRNO(ret, DDS_MOD_ENTITY, DDS_ERR_M1);
    }
    return ret;
}



_Check_return_ dds_return_t
dds_entity_observer_register_nl(
        _In_ dds_entity*  observed,
        _In_ dds_entity_t observer,
        _In_ dds_entity_callback cb)
{
    dds_return_t ret = DDS_RETCODE_OK;
    dds_entity_observer *o = os_malloc(sizeof(dds_entity_observer));
    assert(observed);
    o->m_cb = cb;
    o->m_observer = observer;
    o->m_next = NULL;
    if (observed->m_observers == NULL) {
        observed->m_observers = o;
    } else {
        dds_entity_observer *last;
        dds_entity_observer *idx = observed->m_observers;
        while ((idx != NULL) && (o != NULL)) {
            if (idx->m_observer == observer) {
                os_free(o);
                o = NULL;
                ret = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            last = idx;
            idx = idx->m_next;
        }
        if (o != NULL) {
            last->m_next = o;
        }
    }

    return ret;
}



_Check_return_ dds_return_t
dds_entity_observer_register(
        _In_ dds_entity_t observed,
        _In_ dds_entity_t observer,
        _In_ dds_entity_callback cb)
{
    dds_return_t ret;
    dds_entity *e;
    assert(cb);
    ret = dds_entity_lock(observed, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
        ret = dds_entity_observer_register_nl(e, observer, cb);
        dds_entity_unlock(e);
    }
    return ret;
}



dds_return_t
dds_entity_observer_unregister_nl(
        _In_ dds_entity*  observed,
        _In_ dds_entity_t observer)
{
    dds_return_t ret = DDS_RETCODE_PRECONDITION_NOT_MET;
    dds_entity_observer *prev = NULL;
    dds_entity_observer *idx  = observed->m_observers;
    while (idx != NULL) {
        if (idx->m_observer == observer) {
            if (prev == NULL) {
                observed->m_observers = idx->m_next;
            } else {
                prev->m_next = idx->m_next;
            }
            os_free(idx);
            idx = NULL;
            ret = DDS_RETCODE_OK;
        } else {
            prev = idx;
            idx = idx->m_next;
        }
    }
    return ret;
}



dds_return_t
dds_entity_observer_unregister(
        _In_ dds_entity_t observed,
        _In_ dds_entity_t observer)
{
    dds_return_t ret;
    dds_entity *e;
    ret = dds_entity_lock(observed, DDS_KIND_DONTCARE, &e);
    if (ret == DDS_RETCODE_OK) {
        ret = dds_entity_observer_unregister_nl(e, observer);
        dds_entity_unlock(e);
    }
    return ret;
}



static void
dds_entity_observers_delete(
        _In_ dds_entity *observed)
{
    dds_entity_observer *next;
    dds_entity_observer *idx = observed->m_observers;
    while (idx != NULL) {
        next = idx->m_next;
        os_free(idx);
        idx = next;
    }
    observed->m_observers = NULL;
}



static void
dds_entity_observers_signal(
        _In_ dds_entity *observed,
        _In_ uint32_t status)
{
    dds_entity_observer *idx = observed->m_observers;
    while (idx != NULL) {
        idx->m_cb(idx->m_observer, observed->m_hdl, status);
        idx = idx->m_next;
    }
}

