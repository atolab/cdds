#include <assert.h>
#include "dds.h"
#include "ddsi/q_config.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_init.h"
#include "kernel/dds_tkmap.h"

#define DDS_WRITER_STATUS_MASK                                   \
                        DDS_LIVELINESS_LOST_STATUS              |\
                        DDS_OFFERED_DEADLINE_MISSED_STATUS      |\
                        DDS_OFFERED_INCOMPATIBLE_QOS_STATUS     |\
                        DDS_PUBLICATION_MATCHED_STATUS

static dds_return_t
dds_writer_instance_hdl(
        dds_entity *e,
        dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    *i = (dds_instance_handle_t)writer_instance_id(&e->m_guid);
    return DDS_RETCODE_OK;
}

static dds_return_t
dds_writer_status_validate(
        uint32_t mask)
{
    return (mask & ~(DDS_WRITER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER) :
                     DDS_RETCODE_OK;
}

/*
  Handler function for all write related status callbacks. May trigger status
  condition or call listener on writer. Each entity has a mask of
  supported status types. According to DDS specification, if listener is called
  then status conditions is not triggered.
*/

static void
dds_writer_status_cb(
        void *entity,
        const status_cb_data_t *data)
{
    dds_writer *wr;
    dds_retcode_t rc;
    void *metrics = NULL;

    /* When data is NULL, it means that the writer is deleted. */
    if (data == NULL) {
        /* Release the initial claim that was done during the create. This
         * will indicate that further API deletion is now possible. */
        ut_handle_release(((dds_entity*)entity)->m_hdl, ((dds_entity*)entity)->m_hdllink);
        return;
    }

    if (dds_writer_lock(((dds_entity*)entity)->m_hdl, &wr) != DDS_RETCODE_OK) {
        /* There's a deletion or closing going on. */
        return;
    }
    assert(wr == entity);

    /* Reset the status for possible Listener call.
     * When a listener is not called, the status will be set (again). */
    dds_entity_status_reset(entity, data->status);

    /* Update status metrics. */
    switch (data->status) {
        case DDS_OFFERED_DEADLINE_MISSED_STATUS: {
            wr->m_offered_deadline_missed_status.total_count++;
            wr->m_offered_deadline_missed_status.total_count_change++;
            wr->m_offered_deadline_missed_status.last_instance_handle = data->handle;
            metrics = (void*)&(wr->m_offered_deadline_missed_status);
            break;
        }
        case DDS_LIVELINESS_LOST_STATUS: {
            wr->m_liveliness_lost_status.total_count++;
            wr->m_liveliness_lost_status.total_count_change++;
            metrics = (void*)&(wr->m_liveliness_lost_status);
            break;
        }
        case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS: {
            wr->m_offered_incompatible_qos_status.total_count++;
            wr->m_offered_incompatible_qos_status.total_count_change++;
            wr->m_offered_incompatible_qos_status.last_policy_id = data->extra;
            metrics = (void*)&(wr->m_offered_incompatible_qos_status);
            break;
        }
        case DDS_PUBLICATION_MATCHED_STATUS: {
            if (data->add) {
                wr->m_publication_matched_status.total_count++;
                wr->m_publication_matched_status.total_count_change++;
                wr->m_publication_matched_status.current_count++;
                wr->m_publication_matched_status.current_count_change++;
            } else {
                wr->m_publication_matched_status.current_count--;
                wr->m_publication_matched_status.current_count_change--;
            }
            wr->m_publication_matched_status.last_subscription_handle = data->handle;
            metrics = (void*)&(wr->m_publication_matched_status);
            break;
        }
        default: assert (0);
    }

    /* The writer needs to be unlocked when propagating the (possible) listener
     * call because the application should be able to call this writer within
     * the callback function. */
    dds_writer_unlock(wr);

    /* Is anybody interested within the entity hierarchy through listeners? */
    rc = dds_entity_listener_propagation(entity, entity, data->status, metrics, true);

    if (rc == DDS_RETCODE_OK) {
        /* Event was eaten by a listener. */
        if (dds_writer_lock(((dds_entity*)entity)->m_hdl, &wr) == DDS_RETCODE_OK) {
            assert(wr == entity);

            /* Reset the status. */
            dds_entity_status_reset(entity, data->status);

            /* Reset the change counts of the metrics. */
            switch (data->status) {
                case DDS_OFFERED_DEADLINE_MISSED_STATUS: {
                    wr->m_offered_deadline_missed_status.total_count_change = 0;
                    break;
                }
                case DDS_LIVELINESS_LOST_STATUS: {
                    wr->m_liveliness_lost_status.total_count_change = 0;
                    break;
                }
                case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS: {
                    wr->m_offered_incompatible_qos_status.total_count_change = 0;
                    break;
                }
                case DDS_PUBLICATION_MATCHED_STATUS: {
                    wr->m_publication_matched_status.total_count_change = 0;
                    wr->m_publication_matched_status.current_count_change = 0;
                    break;
                }
                default: assert (0);
            }
            dds_writer_unlock(wr);
        } else {
            /* There's a deletion or closing going on. */
        }
    } else if (rc == DDS_RETCODE_NO_DATA) {
        /* Nobody was interested through a listener (NO_DATA == NO_CALL): set the status. */
        dds_entity_status_set(entity, data->status);
        /* Notify possible interested observers. */
        dds_entity_status_signal(entity);
    } else {
        /* Something went wrong up the hierarchy.
         * Likely, a parent is in the process of being deleted. */
    }
}

static uint32_t
get_bandwidth_limit(
        nn_transport_priority_qospolicy_t transport_priority)
{
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  struct config_channel_listelem *channel = find_channel (transport_priority);
  return channel->data_bandwidth_limit;
#else
  return 0;
#endif
}

static dds_return_t
dds_writer_close(
        dds_entity *e)
{
    dds_retcode_t rc = DDS_RETCODE_OK;
    dds_writer *wr = (dds_writer*)e;
    struct thread_state1 * const thr = lookup_thread_state();
    const bool asleep = thr ? !vtime_awake_p(thr->vtime) : false;

    assert(e);

    if (asleep) {
        thread_state_awake(thr);
    }
    if (thr) {
        nn_xpack_send (wr->m_xp, false);
    }
    if (delete_writer (&e->m_guid) != 0) {
        rc = DDS_RETCODE_ERROR;
    }
    if (asleep) {
        thread_state_asleep(thr);
    }
    return DDS_ERRNO(rc);
}

static dds_return_t
dds_writer_delete(
        dds_entity *e)
{
    dds_writer *wr = (dds_writer*)e;
    struct thread_state1 * const thr = lookup_thread_state();
    const bool asleep = thr ? !vtime_awake_p(thr->vtime) : false;
    dds_return_t ret;

    assert(e);
    assert(thr);

    if (asleep) {
        thread_state_awake(thr);
    }
    if (thr) {
        nn_xpack_free(wr->m_xp);
    }
    if (asleep) {
        thread_state_asleep(thr);
    }
    ret = dds_delete(wr->m_topic->m_entity.m_hdl);
    if(ret == DDS_RETCODE_OK){
        ret = dds_delete_impl(e->m_parent->m_hdl, true);
        if(dds_err_nr(ret) == DDS_RETCODE_ALREADY_DELETED){
            ret = DDS_RETCODE_OK;
        }
    }
    os_mutexDestroy(&wr->m_call_lock);
    return ret;
}


static dds_return_t
dds_writer_qos_validate(
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = DDS_RETCODE_OK;
    bool consistent = true;

    assert(qos);

    /* Check consistency. */
    if (dds_qos_validate_common(qos) != true
     || ((qos->present & QP_USER_DATA) && validate_octetseq(&qos->user_data) != true)
     || ((qos->present & QP_DURABILITY_SERVICE) && validate_durability_service_qospolicy(&qos->durability_service) != 0)
     || ((qos->present & QP_LIFESPAN) && validate_duration(&qos->lifespan.duration) != 0)
     || ((qos->present & QP_HISTORY) && (qos->present & QP_RESOURCE_LIMITS) && validate_history_and_resource_limits(&qos->history, &qos->resource_limits) != 0))
    {
        ret = DDS_ERRNO(DDS_RETCODE_INCONSISTENT_POLICY);
    } else if (enabled) {
        /* TODO: Improve/check immutable check. */
        if (!(qos->present & (QP_LATENCY_BUDGET | QP_OWNERSHIP_STRENGTH))) {
            ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY);
        }
    }

    return ret;
}

static dds_return_t
dds_writer_qos_set(
        dds_entity *e,
        const dds_qos_t *qos,
        bool enabled)
{
    dds_return_t ret = dds_writer_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        /*
         * TODO: CHAM-95: DDSI does not support changing QoS policies.
         *
         * Only Ownership is required for the minimum viable product. This seems
         * to be the only QoS policy that DDSI supports changes on.
         */
        if (qos->present & QP_OWNERSHIP_STRENGTH) {
            dds_ownership_kind_t kind;
            /* check for ownership before updating, ownership strength is applicable only if
             * writer is exclusive */
            dds_qget_ownership (e->m_qos, &kind);

            if (kind == DDS_OWNERSHIP_EXCLUSIVE) {
                struct thread_state1 * const thr = lookup_thread_state ();
                const bool asleep = !vtime_awake_p (thr->vtime);
                struct writer * ddsi_wr = ((dds_writer*)e)->m_wr;

                dds_qset_ownership_strength (e->m_qos, qos->ownership_strength.value);

                if (asleep) {
                    thread_state_awake (thr);
                }

                os_mutexLock (&((dds_writer*)e)->m_call_lock);
                if (qos->ownership_strength.value != ddsi_wr->xqos->ownership_strength.value) {
                    ddsi_wr->xqos->ownership_strength.value = qos->ownership_strength.value;
                }
                os_mutexUnlock (&((dds_writer*)e)->m_call_lock);

                if (asleep) {
                    thread_state_asleep (thr);
                }
            }
            else
            {
                ret = DDS_ERRNO(DDS_RETCODE_ERROR);
            }
        } else {
            if (enabled) {
                ret = DDS_ERRNO(DDS_RETCODE_UNSUPPORTED);
            }
        }
    }
    return ret;
}


_Pre_satisfies_(((participant_or_publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PUBLISHER) || \
                ((participant_or_publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT))
_Pre_satisfies_((topic & DDS_ENTITY_KIND_MASK) == DDS_KIND_TOPIC)
dds_entity_t
dds_create_writer(
        _In_ dds_entity_t participant_or_publisher,
        _In_ dds_entity_t topic,
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
    dds_retcode_t rc;
    dds_qos_t * wqos;
    dds_writer * wr;
    dds_entity_t writer = (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ERROR);
    dds_entity * pub = NULL;
    dds_entity * tp;
    dds_entity_t publisher;
    struct thread_state1 * const thr = lookup_thread_state();
    const bool asleep = !vtime_awake_p(thr->vtime);
    ddsi_tran_conn_t conn = gv.data_conn_mc ? gv.data_conn_mc : gv.data_conn_uc;
    int ret = DDS_RETCODE_OK;

    /* Try claiming a participant. If that's not working, then it could be a subscriber. */
    if(dds_entity_kind(participant_or_publisher) == DDS_KIND_PARTICIPANT){
        publisher = dds_create_publisher(participant_or_publisher, qos, NULL);
    } else{
        publisher = participant_or_publisher;
    }
    rc = dds_entity_lock(publisher, DDS_KIND_PUBLISHER, &pub);

    if (rc != DDS_RETCODE_OK) {
        writer = (dds_entity_t)DDS_ERRNO(rc);
        goto err_pub_lock;
    }

    if (publisher != participant_or_publisher) {
        pub->m_flags |= DDS_ENTITY_IMPLICIT;
    }

    rc = dds_entity_lock(topic, DDS_KIND_TOPIC, &tp);
    if (rc != DDS_RETCODE_OK) {
        writer = (dds_entity_t)DDS_ERRNO(rc);
        goto err_tp_lock;
    }
    assert(((dds_topic*)tp)->m_stopic);
    assert(pub->m_domain == tp->m_domain);

    /* Merge Topic & Publisher qos */
    wqos = dds_qos_create();
    if (qos) {
        /* Only returns failure when one of the qos args is NULL, which
         * is not the case here. */
        (void)dds_qos_copy(wqos, qos);
    }

    if (pub->m_qos) {
        dds_qos_merge(wqos, pub->m_qos);
    }

    if (tp->m_qos) {
        /* merge topic qos data to writer qos */
        dds_qos_merge(wqos, tp->m_qos);
    }
    nn_xqos_mergein_missing(wqos, &gv.default_xqos_wr);

    ret = (int)dds_writer_qos_validate(wqos, false);
    if (ret != 0) {
        dds_qos_delete(wqos);
        writer = (dds_entity_t)ret;
        goto err_bad_qos;
    }

    /* Create writer */
    wr = dds_alloc(sizeof (*wr));
    writer = dds_entity_init(&wr->m_entity, pub, DDS_KIND_WRITER, wqos, listener, DDS_WRITER_STATUS_MASK);

    wr->m_topic = (dds_topic*)tp;
    dds_entity_add_ref_nolock(tp);
    wr->m_xp = nn_xpack_new(conn, get_bandwidth_limit(wqos->transport_priority), config.xpack_send_async);
    os_mutexInit (&wr->m_call_lock);
    wr->m_entity.m_deriver.close = dds_writer_close;
    wr->m_entity.m_deriver.delete = dds_writer_delete;
    wr->m_entity.m_deriver.set_qos = dds_writer_qos_set;
    wr->m_entity.m_deriver.validate_status = dds_writer_status_validate;
    wr->m_entity.m_deriver.get_instance_hdl = dds_writer_instance_hdl;

    /* Extra claim of this writer to make sure that the delete waits until DDSI
     * has deleted its writer as well. This can be known through the callback. */
    if (ut_handle_claim(wr->m_entity.m_hdl, wr->m_entity.m_hdllink, DDS_KIND_WRITER, NULL) != UT_HANDLE_OK) {
        assert(0);
    }

    os_mutexUnlock(&tp->m_mutex);
    os_mutexUnlock(&pub->m_mutex);

    if (asleep) {
        thread_state_awake(thr);
    }
    wr->m_wr = new_writer(&wr->m_entity.m_guid, NULL, &pub->m_participant->m_guid, ((dds_topic*)tp)->m_stopic,
                          wqos, dds_writer_status_cb, wr);
    os_mutexLock(&pub->m_mutex);
    os_mutexLock(&tp->m_mutex);
    assert(wr->m_wr);
    if (asleep) {
        thread_state_asleep(thr);
    }
    dds_entity_unlock(tp);
    dds_entity_unlock(pub);
    return writer;

err_bad_qos:
    dds_entity_unlock(tp);
err_tp_lock:
    dds_entity_unlock(pub);
    if((pub->m_flags & DDS_ENTITY_IMPLICIT) != 0){
        dds_delete(publisher);
    }
err_pub_lock:
    return writer;
}



_Pre_satisfies_(((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER))
dds_entity_t
dds_get_publisher(
        _In_ dds_entity_t writer)
{
    if (writer >= 0) {
        if (dds_entity_kind(writer) == DDS_KIND_WRITER) {
            return dds_get_parent(writer);
        } else {
            dds_retcode_t rc = dds_valid_hdl(writer, DDS_KIND_DONTCARE);
            if (rc == DDS_RETCODE_OK) {
                return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION);
            } else {
                return (dds_entity_t)DDS_ERRNO(rc);
            }
        }
    }
    return writer;
}

dds_return_t
dds_get_publication_matched_status(
        dds_entity_t entity,
        dds_publication_matched_status_t *status)
{
    dds_retcode_t rc;
    dds_writer *wr;

    rc = dds_writer_lock(entity, &wr);
    if (rc == DDS_RETCODE_OK) {
        if (((dds_entity*)wr)->m_status_enable & DDS_PUBLICATION_MATCHED_STATUS) {
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_publication_matched_status;
            }
            wr->m_publication_matched_status.total_count_change = 0;
            wr->m_publication_matched_status.current_count_change = 0;
            dds_entity_status_reset(wr, DDS_PUBLICATION_MATCHED_STATUS);
        }
        dds_writer_unlock(wr);
    }
    return DDS_ERRNO(rc);
}

dds_return_t
dds_get_liveliness_lost_status(
        dds_entity_t entity,
        dds_liveliness_lost_status_t *status)
{
    dds_retcode_t rc;
    dds_writer *wr;

    rc = dds_writer_lock(entity, &wr);
    if (rc == DDS_RETCODE_OK) {
        if (((dds_entity*)wr)->m_status_enable & DDS_LIVELINESS_LOST_STATUS) {
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_liveliness_lost_status;
            }
            wr->m_liveliness_lost_status.total_count_change = 0;
            dds_entity_status_reset(wr, DDS_LIVELINESS_LOST_STATUS);
        }
        dds_writer_unlock(wr);
    }
    return DDS_ERRNO(rc);
}

dds_return_t
dds_get_offered_deadline_missed_status(
        dds_entity_t entity,
        dds_offered_deadline_missed_status_t *status)
{
    dds_retcode_t rc;
    dds_writer *wr;

    rc = dds_writer_lock(entity, &wr);
    if (rc == DDS_RETCODE_OK) {
        if (((dds_entity*)wr)->m_status_enable & DDS_OFFERED_DEADLINE_MISSED_STATUS) {
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_offered_deadline_missed_status;
            }
            wr->m_offered_deadline_missed_status.total_count_change = 0;
            dds_entity_status_reset(wr, DDS_OFFERED_DEADLINE_MISSED_STATUS);
        }
        dds_writer_unlock(wr);
    }
    return DDS_ERRNO(rc);
}

dds_return_t
dds_get_offered_incompatible_qos_status(
        dds_entity_t entity,
        dds_offered_incompatible_qos_status_t *status)
{
    dds_retcode_t rc;
    dds_writer *wr;

    rc = dds_writer_lock(entity, &wr);
    if (rc == DDS_RETCODE_OK) {
        if (((dds_entity*)wr)->m_status_enable & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS) {
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_offered_incompatible_qos_status;
            }
            wr->m_offered_incompatible_qos_status.total_count_change = 0;
            dds_entity_status_reset(wr, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
        }
        dds_writer_unlock(wr);
    }
    return DDS_ERRNO(rc);
}
