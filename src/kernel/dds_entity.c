#include <assert.h>
#include <string.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_xmsg.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_topic.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"
#include "kernel/dds_condition.h"
#include "os/os_report.h"



void dds_entity_add_ref (dds_entity * e)
{
    assert (e);
    os_mutexLock (&e->m_mutex);
    e->m_refc++;
    os_mutexUnlock (&e->m_mutex);
}

//dds_entity_hierarchy_cb_lock
bool dds_entity_hierarchy_busy_start(dds_entity *e)
{
    bool ok = true;
    if (e) {
        /* Propagate lock to make sure the top is handled first. */
        ok = dds_entity_hierarchy_busy_start(e->m_parent);

        if (ok) {
            /* Lock the listener. Unlock it when the callback is done.
             * This has the following effect:
             * - Entity deletion will wait while the cb is in progress.
             * - Setting a listener on the entity will wait while the cb is in progress.
             * - Setting functions on the listener will wait while the cb is in progress.
             */
            /* Indicate that I'm busy, so that a parrallel delete/set_listener will wait. */
            os_mutexLock(&e->m_mutex);
            if (e->m_flags |= DDS_ENTITY_DELETED) {
                /* Deletion in progress. */
                ok = false;
            } else {
                e->m_flags |= DDS_ENTITY_BUSY;
            }
            os_mutexUnlock(&e->m_mutex);
            if (!ok) {
                /* Un-busy the top of the hierarchy. */
                dds_entity_hierarchy_busy_end(e->m_parent);
            }
        }
    }
    return ok;
}

void dds_entity_hierarchy_busy_end(dds_entity *e)
{
    if (e) {
        /* We started at the top, so when resetting we should start at the bottom. */
        os_mutexLock(&e->m_mutex);
        e->m_flags &= ~DDS_ENTITY_BUSY;
        os_mutexUnlock(&e->m_mutex);

        /* If anybody is waiting, let them continue. */
        if (e->m_waiting > 0) {
            os_condBroadcast (&e->m_cond);
        }

        /* Continue up the hierarchy. */
        dds_entity_hierarchy_busy_end(e->m_parent);
    }
}

bool dds_entity_hierarchy_callback(dds_entity *e, dds_entity *src, uint32_t status, void *metrics, bool propagate)
{
    bool called = false;
    if (e) {
        /* We should be busy at this point. */
        assert(e->m_flags & DDS_ENTITY_BUSY);

        if (e->m_listener) {
            switch (status) {
                case DDS_INCONSISTENT_TOPIC_STATUS: {
                    dds_on_inconsistent_topic_fn cb = e->m_listener->on_inconsistent_topic;
                    if (cb) {
                        cb(src, *((dds_inconsistent_topic_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_OFFERED_DEADLINE_MISSED_STATUS: {
                    dds_on_offered_deadline_missed_fn cb = e->m_listener->on_offered_deadline_missed;
                    if (cb) {
                        cb(src, *((dds_offered_deadline_missed_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_REQUESTED_DEADLINE_MISSED_STATUS: {
                    dds_on_requested_deadline_missed_fn cb = e->m_listener->on_requested_deadline_missed;
                    if (cb) {
                        cb(src, *((dds_requested_deadline_missed_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS: {
                    dds_on_offered_incompatible_qos_fn cb = e->m_listener->on_offered_incompatible_qos;
                    if (cb) {
                        cb(src, *((dds_offered_incompatible_qos_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS: {
                    dds_on_requested_incompatible_qos_fn cb = e->m_listener->on_requested_incompatible_qos;
                    if (cb) {
                        cb(src, *((dds_requested_incompatible_qos_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_SAMPLE_LOST_STATUS: {
                    dds_on_sample_lost_fn cb = e->m_listener->on_sample_lost;
                    if (cb) {
                        cb(src, *((dds_sample_lost_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_SAMPLE_REJECTED_STATUS: {
                    dds_on_sample_rejected_fn cb = e->m_listener->on_sample_rejected;
                    if (cb) {
                        cb(src, *((dds_sample_rejected_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_DATA_ON_READERS_STATUS: {
                    dds_on_data_on_readers_fn cb = e->m_listener->on_data_on_readers;
                    if (cb) {
                        cb(src);
                        called = true;
                    }
                    break;
                }
                case DDS_DATA_AVAILABLE_STATUS: {
                    dds_on_data_available_fn cb = e->m_listener->on_data_available;
                    if (cb) {
                        cb(src);
                        called = true;
                    }
                    break;
                }
                case DDS_LIVELINESS_LOST_STATUS: {
                    dds_on_liveliness_lost_fn cb = e->m_listener->on_liveliness_lost;
                    if (cb) {
                        cb(src, *((dds_liveliness_lost_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_LIVELINESS_CHANGED_STATUS: {
                    dds_on_liveliness_changed_fn cb = e->m_listener->on_liveliness_changed;
                    if (cb) {
                        cb(src, *((dds_liveliness_changed_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_PUBLICATION_MATCHED_STATUS: {
                    dds_on_publication_matched_fn cb = e->m_listener->on_publication_matched;
                    if (cb) {
                        cb(src, *((dds_publication_matched_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                case DDS_SUBSCRIPTION_MATCHED_STATUS: {
                    dds_on_subscription_matched_fn cb = e->m_listener->on_subscription_matched;
                    if (cb) {
                        cb(src, *((dds_subscription_matched_status_t*)metrics));
                        called = true;
                    }
                    break;
                }
                default: assert (0);
            }
        }

        if (!called && propagate) {
            /* See if the parent is interrested. */
            called = dds_entity_hierarchy_callback(e->m_parent, src, status, metrics, propagate);
        }
    }
    return called;
}

void dds_entity_wait_lock (dds_entity_t e)
{
    os_mutexLock (&e->m_mutex);
    e->m_waiting++;
    while (e->m_flags & DDS_ENTITY_BUSY) {
        os_condWait (&e->m_cond, &e->m_mutex);
    }
    e->m_waiting--;
}

void dds_entity_wait_unlock (dds_entity_t e)
{
    os_mutexUnlock(&e->m_mutex);
    if (e->m_waiting > 0) {
        os_condBroadcast (&e->m_cond);
    }
}

void dds_entity_delete_signal (dds_entity_t e)
{
    /* Signal that clean up complete */
    os_mutexLock (&e->m_mutex);
    e->m_flags |= DDS_ENTITY_DDSI_DELETED;
    os_condSignal (&e->m_cond);
    os_mutexUnlock (&e->m_mutex);
}

void dds_entity_delete_wait (dds_entity_t e, struct thread_state1 * const thr)
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

void dds_entity_delete_impl (dds_entity_t e, bool child, bool recurse)
{
    dds_entity_t iter;
    dds_entity_t *iterp;
    dds_entity_t prev = NULL;

    dds_entity_wait_lock(e);

    if (--e->m_refc != 0) {
        dds_entity_wait_unlock(e);
        return;
    }

    e->m_flags |= DDS_ENTITY_DELETED;
    e->m_status_enable = 0;
    e->m_listener = NULL;

    dds_entity_wait_unlock(e);

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

void dds_entity_delete (dds_entity_t e)
{
    if (e) {
        dds_entity_delete_impl (e, false, true);
    }
}

void dds_entity_init
(
  dds_entity * e, dds_entity * parent,
  dds_entity_kind_t kind, dds_qos_t * qos,
  dds_listener_cham65_t * listener,
  uint32_t mask
)
{
    assert (e);

    e->m_refc = 1;
    e->m_parent = parent;
    e->m_kind = kind;
    e->m_qos = qos;
    e->m_listener = listener;
    e->m_waiting = 0;

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
}

dds_entity_t dds_get_parent(dds_entity_t e)
{
    if (e > 0) {
        return (dds_entity_t)(e->m_parent);
    }
    return NULL;
}

dds_entity_t dds_get_participant(dds_entity_t e)
{
    if (e > 0) {
        return (dds_entity_t)(e->m_participant);
    }
    return NULL;
}

dds_result_t dds_get_children(dds_entity_t e, dds_entity_t *children, size_t size)
{
    dds_result_t ret = (dds_result_t)(DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0));
    if ((e > 0) && (children != NULL) && (size > 0)) {
        os_mutexLock(&e->m_mutex);
        ret = 0;
        dds_entity* iter = e->m_children;
        while (iter) {
            if (ret < size) {
                children[ret] = (dds_entity_t)iter;
            }
            ret++;
            iter = iter->m_next;
        }
        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}

dds_result_t dds_get_qos (dds_entity_t e, dds_qos_t * qos)
{
    dds_result_t ret = (dds_result_t)(DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0));
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
dds_result_t dds_set_qos (dds_entity_t e, const dds_qos_t * qos)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (qos != NULL)) {
        ret = DDS_RETCODE_OK;
        os_mutexLock(&e->m_mutex);
        if (e->m_qos) {
            if (e->m_deriver.validate_qos) {
                ret = e->m_deriver.validate_qos(qos, e->m_flags & DDS_ENTITY_ENABLED);
            } else {
                ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_ENTITY, DDS_ERR_M1);
            }

            if (ret == DDS_RETCODE_OK) {
                if (e->m_flags & DDS_ENTITY_ENABLED) {
                    /* Set QoS in DDSI. */
                    /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
                    ret = (dds_result_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_ENTITY, 0));
                }
            }

            if (ret == DDS_RETCODE_OK) {
                /* Remember this QoS. */
                ret = dds_qos_copy (e->m_qos, qos);
            }
        }
        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}

dds_result_t dds_get_listener (dds_entity_t e, dds_listener_cham65_t ** listener)
{
    dds_result_t ret = (dds_result_t)(DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0));
    if ((e > 0) && (listener != NULL)) {
        ret = DDS_RETCODE_OK;
        os_mutexLock(&e->m_mutex);
        *listener = e->m_listener;
        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}

dds_result_t dds_set_listener (dds_entity_t e, dds_listener_cham65_t *listener)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (listener != NULL)) {
        ret = DDS_RETCODE_OK;
        dds_entity_wait_lock(e);
        /* TODO: CHAM-65: Actually set listeners, not just copy it (or is setting them enough?). */
        e->m_listener = listener;
        dds_entity_wait_unlock(e);
    }
    return ret;
}

dds_result_t dds_enable(dds_entity_t e)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if (e > 0) {
        os_mutexLock(&e->m_mutex);
        if (e->m_flags & DDS_ENTITY_ENABLED) {
            /* Already enabled. */
            ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_ENTITY, 0);
        } else {
            /* TODO: CHAM-96: Really enable. */
            e->m_flags |= DDS_ENTITY_ENABLED;
            ret = DDS_RETCODE_OK;
        }

        os_mutexUnlock(&e->m_mutex);
    }
    return ret;
}







dds_result_t dds_get_status_changes (dds_entity_t e, uint32_t *mask)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (mask != NULL)) {
        os_mutexLock (&e->m_mutex);
        *mask = e->m_status->m_trigger;
        os_mutexUnlock (&e->m_mutex);
        ret = DDS_RETCODE_OK;
    }
    return ret;
}

dds_result_t dds_get_enabled_status(dds_entity_t e, uint32_t *mask)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (mask != NULL)) {
        os_mutexLock (&e->m_mutex);
        *mask = e->m_status_enable;
        os_mutexUnlock (&e->m_mutex);
        ret = DDS_RETCODE_OK;
    }
    return ret;
}


dds_result_t dds_set_enabled_status (dds_entity_t e, uint32_t mask)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
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
dds_result_t dds_read_status (dds_entity_t e, uint32_t * status, uint32_t mask)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
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
dds_result_t dds_take_status (dds_entity_t e, uint32_t * status, uint32_t mask)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
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

void dds_entity_status_signal(dds_entity_t e)
{
    assert(e);
    dds_cond_callback_signal(e->m_status);
}

dds_result_t dds_get_domainid (dds_entity_t e, dds_domainid_t *id)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (id != NULL)) {
        ret = DDS_RETCODE_OK;
        os_mutexLock (&e->m_mutex);
        *id = e->m_domainid;
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}

dds_result_t dds_instancehandle_get(dds_entity_t e, dds_instance_handle_t *i)
{
    dds_result_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_ENTITY, 0);
    if ((e > 0) && (i != NULL)) {
        os_mutexLock (&e->m_mutex);
        if (e->m_deriver.get_instance_hdl) {
            ret = e->m_deriver.get_instance_hdl(e, i);
        }
        os_mutexUnlock (&e->m_mutex);
    }
    return ret;
}
