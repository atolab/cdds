#include <assert.h>
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

static dds_return_t dds_writer_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    *i = (dds_instance_handle_t)writer_instance_id(&e->m_guid);
    return DDS_RETCODE_OK;
}

static dds_return_t dds_writer_status_validate (uint32_t mask)
{
    return (mask & ~(DDS_WRITER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, 0) :
                     DDS_RETCODE_OK;
}

/*
  Handler function for all write related status callbacks. May trigger status
  condition or call listener on writer. Each entity has a mask of
  supported status types. According to DDS specification, if listener is called
  then status conditions is not triggered.
*/

static void dds_writer_status_cb (void * entity, const status_cb_data_t * data)
{
    dds_writer * wr = (dds_writer*) entity;
    bool call = false;
    void *metrics = NULL;

    /* When data is NULL, it means that the writer is deleted. */
    if (data == NULL) {
        /* API deletion is possible waiting for this signal that
         * indicates that no more callbacks will be triggered. */
        dds_entity_delete_signal (entity);
        return;
    }

    os_mutexLock (&wr->m_entity.m_mutex);

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
    os_mutexUnlock (&wr->m_entity.m_mutex);


    /* Indicate to the entity hierarchy that we're busy with a callback.
     * This is done from the top to bottom to prevent possible deadlocks.
     * We can't really lock the entities because they have to be possibly
     * accessible from listener callbacks. */
    if (!dds_entity_cb_propagate_begin(entity)) {
        /* An entity in the hierarchy is probably being deleted. */
        return;
    }

    /* Is anybody interested within the entity hierarchy through listeners? */
    call = dds_entity_cp_propagate_call(entity, entity, data->status, metrics, true);

    /* Let possible waits continue. */
    dds_entity_cb_propagate_end(entity);

    if (call) {
        /* Event was eaten by a listener. */
        os_mutexLock (&wr->m_entity.m_mutex);

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
        os_mutexUnlock (&wr->m_entity.m_mutex);
    } else {
        /* Nobody was interested through a listener. Set the status to maybe force a trigger. */
        dds_entity_status_set(entity, data->status);
        dds_entity_status_signal(entity);
    }
}

static uint32_t get_bandwidth_limit (nn_transport_priority_qospolicy_t transport_priority)
{
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  struct config_channel_listelem *channel = find_channel (transport_priority);
  return channel->data_bandwidth_limit;
#else
  return 0;
#endif
}

static void dds_writer_delete(dds_entity_t e, bool recurse)
{
    dds_writer * wr = (dds_writer*) e;
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);

    assert(e);
    assert(thr);

    if (asleep) {
      thread_state_awake(thr);
    }

    nn_xpack_send (wr->m_xp, false);
    delete_writer (&e->m_guid);
    dds_entity_delete_wait (e, thr);
    nn_xpack_free (wr->m_xp);
    os_mutexDestroy (&wr->m_call_lock);
    dds_entity_delete_impl ((dds_entity*) wr->m_topic, false, recurse);

    if (asleep) {
      thread_state_asleep(thr);
    }
}

static dds_return_t dds_writer_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_WRITER, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= dds_qos_validate_common(qos);
    consistent &= (qos->present & QP_USER_DATA) && ! validate_octetseq (&qos->user_data);
    consistent &= (qos->present & QP_DURABILITY_SERVICE) && (validate_durability_service_qospolicy (&qos->durability_service) != 0);
    consistent &= ((qos->present & QP_LIFESPAN) && (validate_duration (&qos->lifespan.duration) != 0)) ||
                  ((qos->present & QP_HISTORY) && (qos->present & QP_RESOURCE_LIMITS) && (validate_history_and_resource_limits (&qos->history, &qos->resource_limits) != 0));
    if (consistent) {
        ret = DDS_RETCODE_OK;
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            if (!(qos->present & (QP_LATENCY_BUDGET | QP_OWNERSHIP_STRENGTH))) {
                ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_WRITER, DDS_ERR_M2);
            }
        }
    }
    return ret;
}

static dds_return_t dds_writer_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
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
                ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_ERROR, DDS_MOD_WRITER, 0));
            }
        } else {
            if (enabled) {
                ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, DDS_ERR_M1));
            }
        }
    }
    return ret;
}


int dds_writer_create
(
  dds_entity_t pp_or_pub,
  dds_entity_t * writer,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_listener_t * listener
)
{
  dds_qos_t * wqos;
  dds_publisher * pub;
  dds_writer * wr;
  dds_topic * tp = (dds_topic*) topic;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  ddsi_tran_conn_t conn = gv.data_conn_mc ? gv.data_conn_mc : gv.data_conn_uc;

  assert (pp_or_pub);
  assert (writer);
  assert (topic);
  assert (tp->m_stopic);
  assert ((pp_or_pub->m_kind & DDS_IS_PP_OR_PUB) != 0);
  assert (topic->m_kind == DDS_TYPE_TOPIC);
  assert (pp_or_pub->m_domain == topic->m_domain);
  int ret = DDS_RETCODE_OK;

  os_mutexLock (&pp_or_pub->m_mutex);

  pub = (pp_or_pub->m_kind == DDS_TYPE_PUBLISHER) ? (dds_publisher*) pp_or_pub : NULL;

  /* Merge Topic & Publisher qos */

  wqos = dds_qos_create ();
  if (qos)
  {
    dds_qos_copy (wqos, qos);
  }
  if (pub && pub->m_entity.m_qos)
  {
    dds_qos_merge (wqos, pub->m_entity.m_qos);
  }

  if (tp->m_entity.m_qos)
  {
    /* merge topic qos data to writer qos */
    dds_qos_merge (wqos, tp->m_entity.m_qos);
  }
  nn_xqos_mergein_missing (wqos, &gv.default_xqos_wr);

  ret = (int)dds_writer_qos_validate (wqos, false);
  if (ret != 0)
  {
    os_mutexUnlock (&pp_or_pub->m_mutex);
    return ret;
  }

  /* Create writer */
  wr = dds_alloc (sizeof (*wr));
  *writer = &wr->m_entity;
  dds_entity_init (&wr->m_entity, pp_or_pub, DDS_TYPE_WRITER, wqos, listener, DDS_WRITER_STATUS_MASK);
  wr->m_topic = tp;
  dds_entity_add_ref (topic);
  wr->m_xp = nn_xpack_new (conn, get_bandwidth_limit(wqos->transport_priority), config.xpack_send_async);
  os_mutexInit (&wr->m_call_lock);
  wr->m_entity.m_deriver.delete = dds_writer_delete;
  wr->m_entity.m_deriver.set_qos = dds_writer_qos_set;
  wr->m_entity.m_deriver.validate_status = dds_writer_status_validate;
  wr->m_entity.m_deriver.get_instance_hdl = dds_writer_instance_hdl;

  os_mutexUnlock (&pp_or_pub->m_mutex);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  wr->m_wr = new_writer (&wr->m_entity.m_guid, NULL, &pp_or_pub->m_participant->m_guid, tp->m_stopic, wqos,
  dds_writer_status_cb, wr);
  assert (wr->m_wr);
  if (asleep)
  {
    thread_state_asleep (thr);
  }
  return DDS_RETCODE_OK;
}

dds_entity_t dds_get_publisher(dds_entity_t wr)
{
    /* TODO: CHAM-104: Return actual errors when dds_entity_t became an handle iso a pointer (see header). */
    if (dds_entity_is_a(wr, DDS_TYPE_WRITER)) {
        return dds_get_parent(wr);
    }
    return NULL;
}

dds_return_t dds_get_publication_matched_status (dds_entity_t entity, dds_publication_matched_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_WRITER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WRITER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_PUBLICATION_MATCHED_STATUS) {
            dds_writer *wr = (dds_writer*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_publication_matched_status;
            }
            wr->m_publication_matched_status.total_count_change = 0;
            wr->m_publication_matched_status.current_count_change = 0;
            dds_entity_status_reset(entity, DDS_PUBLICATION_MATCHED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_liveliness_lost_status (dds_entity_t entity, dds_liveliness_lost_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_WRITER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WRITER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_LIVELINESS_LOST_STATUS) {
            dds_writer *wr = (dds_writer*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_liveliness_lost_status;
            }
            wr->m_liveliness_lost_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_LIVELINESS_LOST_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_offered_deadline_missed_status (dds_entity_t entity, dds_offered_deadline_missed_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_WRITER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WRITER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_OFFERED_DEADLINE_MISSED_STATUS) {
            dds_writer *wr = (dds_writer*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_offered_deadline_missed_status;
            }
            wr->m_offered_deadline_missed_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_OFFERED_DEADLINE_MISSED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_offered_incompatible_qos_status (dds_entity_t entity, dds_offered_incompatible_qos_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_WRITER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WRITER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS) {
            dds_writer *wr = (dds_writer*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = wr->m_offered_incompatible_qos_status;
            }
            wr->m_offered_incompatible_qos_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}
