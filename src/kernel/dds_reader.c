#include <assert.h>
#include <string.h>
#include "kernel/dds_reader.h"
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"

#include <string.h>
#include "os/os.h"

#define DDS_READER_STATUS_MASK                                   \
                        DDS_SAMPLE_REJECTED_STATUS              |\
                        DDS_LIVELINESS_CHANGED_STATUS           |\
                        DDS_REQUESTED_DEADLINE_MISSED_STATUS    |\
                        DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS   |\
                        DDS_DATA_AVAILABLE_STATUS               |\
                        DDS_SAMPLE_LOST_STATUS                  |\
                        DDS_SUBSCRIPTION_MATCHED_STATUS

static dds_return_t dds_reader_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    *i = (dds_instance_handle_t)reader_instance_id(&e->m_guid);
    return DDS_RETCODE_OK;
}

static void dds_reader_delete(dds_entity_t e, bool recurse)
{
    dds_reader * rd = (dds_reader*) e;
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);

    assert(e);
    assert(thr);

    if (asleep) {
      thread_state_awake(thr);
    }

    delete_reader (&e->m_guid);
    dds_entity_delete_wait (e, thr);
    dds_entity_delete_impl ((dds_entity*) rd->m_topic, false, recurse);
    dds_free (rd->m_loan);

    if (asleep) {
      thread_state_asleep(thr);
    }
}

static dds_return_t dds_reader_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_READER, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= dds_qos_validate_common(qos);
    consistent &= (qos->present & QP_USER_DATA)                       ? validate_octetseq (&qos->user_data) : true;
    consistent &= (qos->present & QP_PRISMTECH_READER_DATA_LIFECYCLE) ? validate_reader_data_lifecycle (&qos->reader_data_lifecycle) == 0 : true;
    consistent &= (qos->present & QP_TIME_BASED_FILTER)               ? validate_duration (&qos->time_based_filter.minimum_separation) == 0 : true;
    consistent &= (qos->present & QP_HISTORY)
               && (qos->present & QP_RESOURCE_LIMITS)                 ? validate_history_and_resource_limits (&qos->history, &qos->resource_limits) == 0 : true;
    consistent &= (qos->present & QP_TIME_BASED_FILTER)
               && (qos->present & QP_DEADLINE)                        ? validate_deadline_and_timebased_filter (qos->deadline.deadline, qos->time_based_filter.minimum_separation) : true;
    if (consistent) {
        ret = DDS_RETCODE_OK;
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            if (qos->present != QP_LATENCY_BUDGET) {
                ret = DDS_ERRNO(DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_READER, DDS_ERR_M1);
            }
        }
    }
    return ret;
}

static dds_return_t dds_reader_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_reader_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_READER, DDS_ERR_M1));
        }
    }
    return ret;
}

static dds_return_t dds_reader_status_validate (uint32_t mask)
{
    return (mask & ~(DDS_READER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0) :
                     DDS_RETCODE_OK;
}

void dds_reader_status_cb (void * entity, const status_cb_data_t * data)
{
    dds_reader * rd = (dds_reader*) entity;
    bool call = false;
    void *metrics = NULL;

    /* When data is NULL, it means that the reader is deleted. */
    if (data == NULL) {
        /* API deletion is possible waiting for this signal that
         * indicates that no more callbacks will be triggered. */
        dds_entity_delete_signal (entity);
        return;
    }

    os_mutexLock (&rd->m_entity.m_mutex);

    /* Reset the status for possible Listener call.
     * When a listener is not called, the status will be set (again). */
    dds_entity_status_reset(entity, data->status);

    /* Update status metrics. */
    switch (data->status) {
        case DDS_REQUESTED_DEADLINE_MISSED_STATUS: {
            rd->m_requested_deadline_missed_status.total_count++;
            rd->m_requested_deadline_missed_status.total_count_change++;
            rd->m_requested_deadline_missed_status.last_instance_handle = data->handle;
            metrics = (void*)&(rd->m_requested_deadline_missed_status);
            break;
        }
        case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS: {
            rd->m_requested_incompatible_qos_status.total_count++;
            rd->m_requested_incompatible_qos_status.total_count_change++;
            rd->m_requested_incompatible_qos_status.last_policy_id = data->extra;
            metrics = (void*)&(rd->m_requested_incompatible_qos_status);
            break;
        }
        case DDS_SAMPLE_LOST_STATUS: {
            rd->m_sample_lost_status.total_count++;
            rd->m_sample_lost_status.total_count_change++;
            metrics = (void*)&(rd->m_sample_lost_status);
            break;
        }
        case DDS_SAMPLE_REJECTED_STATUS: {
            rd->m_sample_rejected_status.total_count++;
            rd->m_sample_rejected_status.total_count_change++;
            rd->m_sample_rejected_status.last_reason = data->extra;
            rd->m_sample_rejected_status.last_instance_handle = data->handle;
            metrics = (void*)&(rd->m_sample_rejected_status);
            break;
        }
        case DDS_DATA_AVAILABLE_STATUS: {
            metrics = NULL;
            break;
        }
        case DDS_LIVELINESS_CHANGED_STATUS: {
            if (data->add) {
                rd->m_liveliness_changed_status.alive_count++;
                rd->m_liveliness_changed_status.alive_count_change++;
                if (rd->m_liveliness_changed_status.not_alive_count > 0) {
                    rd->m_liveliness_changed_status.not_alive_count--;
                }
            } else {
                rd->m_liveliness_changed_status.alive_count--;
                rd->m_liveliness_changed_status.not_alive_count++;
                rd->m_liveliness_changed_status.not_alive_count_change++;
            }
            rd->m_liveliness_changed_status.last_publication_handle = data->handle;
            metrics = (void*)&(rd->m_liveliness_changed_status);
            break;
        }
        case DDS_SUBSCRIPTION_MATCHED_STATUS: {
            if (data->add) {
                rd->m_subscription_matched_status.total_count++;
                rd->m_subscription_matched_status.total_count_change++;
                rd->m_subscription_matched_status.current_count++;
                rd->m_subscription_matched_status.current_count_change++;
            } else {
                rd->m_subscription_matched_status.current_count--;
                rd->m_subscription_matched_status.current_count_change--;
            }
            rd->m_subscription_matched_status.last_publication_handle = data->handle;
            metrics = (void*)&(rd->m_subscription_matched_status);
            break;
        }
        default: assert (0);
    }
    os_mutexUnlock (&rd->m_entity.m_mutex);

    /* Indicate to the entity hierarchy that we're busy with a callback.
     * This is done from the top to bottom to prevent possible deadlocks.
     * We can't really lock the entities because they have to be possibly
     * accessible from listener callbacks. */
    if (!dds_entity_cb_propagate_begin(entity)) {
        /* An entity in the hierarchy is probably being deleted. */
        return;
    }

    /* DATA_AVAILABLE is handled differently to normal status changes. */
    if (data->status == DDS_DATA_AVAILABLE_STATUS) {
        /* First, try to ship it off to its parent subscriber or participant as DDS_DATA_ON_READERS_STATUS. */
        call = dds_entity_cp_propagate_call(rd->m_entity.m_parent, entity, DDS_DATA_ON_READERS_STATUS, NULL, true);

        if (!call) {
            /* No parent was interested. What about myself with DDS_DATA_AVAILABLE_STATUS? */
            call = dds_entity_cp_propagate_call(entity, entity, DDS_DATA_AVAILABLE_STATUS, NULL, false);
        }

        if (!call) {
            /* Nobody was interested. Set the status to maybe force a trigger on the subscriber. */
            dds_entity_status_set(rd->m_entity.m_parent, DDS_DATA_ON_READERS_STATUS);
            dds_entity_status_signal(rd->m_entity.m_parent);
        }
    } else {
        /* Is anybody interested within the entity hierarchy through listeners? */
        call = dds_entity_cp_propagate_call(entity, entity, data->status, metrics, true);
    }

    /* Let possible waits continue. */
    dds_entity_cb_propagate_end(entity);

    if (call) {
        /* Event was eaten by a listener. */
        os_mutexLock (&rd->m_entity.m_mutex);

        /* Reset the change counts of the metrics. */
        switch (data->status) {
            case DDS_REQUESTED_DEADLINE_MISSED_STATUS: {
                rd->m_requested_deadline_missed_status.total_count_change = 0;
                break;
            }
            case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS: {
                rd->m_requested_incompatible_qos_status.total_count_change = 0;
                break;
            }
            case DDS_SAMPLE_LOST_STATUS: {
                rd->m_sample_lost_status.total_count_change = 0;
                break;
            }
            case DDS_SAMPLE_REJECTED_STATUS: {
                rd->m_sample_rejected_status.total_count_change = 0;
                break;
            }
            case DDS_DATA_AVAILABLE_STATUS: {
                /* Nothing to reset. */;
                break;
            }
            case DDS_LIVELINESS_CHANGED_STATUS: {
                rd->m_liveliness_changed_status.alive_count_change = 0;
                rd->m_liveliness_changed_status.not_alive_count_change = 0;
                break;
            }
            case DDS_SUBSCRIPTION_MATCHED_STATUS: {
                rd->m_subscription_matched_status.total_count_change = 0;
                rd->m_subscription_matched_status.current_count_change = 0;
                break;
            }
            default: assert (0);
        }
        os_mutexUnlock (&rd->m_entity.m_mutex);
    } else {
        /* Nobody was interested through a listener. Set the status to maybe force a trigger. */
        dds_entity_status_set(entity, data->status);
        dds_entity_status_signal(entity);
    }
}

int dds_reader_create
(
  dds_entity_t pp_or_sub,
  dds_entity_t * reader,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_listener_t * listener
)
{
  dds_qos_t * rqos;
  dds_subscriber * sub;
  dds_reader * rd;
  struct rhc * rhc;
  dds_topic * tp = (dds_topic*) topic;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);

  assert (pp_or_sub);
  assert (reader);
  assert (topic);
  assert (tp->m_stopic);
  assert ((pp_or_sub->m_kind & DDS_IS_PP_OR_SUB) != 0);
  assert (topic->m_kind == DDS_TYPE_TOPIC);
  assert (pp_or_sub->m_domain == topic->m_domain);
  int ret = DDS_RETCODE_OK;

  os_mutexLock (&pp_or_sub->m_mutex);

  sub = (pp_or_sub->m_kind == DDS_TYPE_SUBSCRIBER) ? (dds_subscriber*) pp_or_sub : NULL;

  /* Merge qos from topic and subscriber */
  rqos = dds_qos_create ();
  if (qos)
  {
    dds_qos_copy (rqos, qos);
  }

  if (sub && sub->m_entity.m_qos)
  {
    dds_qos_merge (rqos, sub->m_entity.m_qos);
  }
  if (tp->m_entity.m_qos)
  {
    dds_qos_merge (rqos, tp->m_entity.m_qos);

    /* reset the following qos policies if set during topic qos merge as they aren't applicable for reader */
    rqos->present &= ~(QP_DURABILITY_SERVICE | QP_TRANSPORT_PRIORITY | QP_LIFESPAN);
  }
  nn_xqos_mergein_missing (rqos, &gv.default_xqos_rd);

  ret = (int)dds_reader_qos_validate (rqos, false);
  if (ret != 0)
  {
    os_mutexUnlock (&pp_or_sub->m_mutex);
    return ret;
  }

  /* Create reader and associated read cache */
  rd = dds_alloc (sizeof (*rd));
  *reader = &rd->m_entity;
  dds_entity_init (&rd->m_entity, pp_or_sub, DDS_TYPE_READER, rqos, listener, DDS_READER_STATUS_MASK);
  rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
  rd->m_topic = tp;
  dds_entity_add_ref (topic);
  rhc = dds_rhc_new (rd, tp->m_stopic);
  rd->m_entity.m_deriver.delete = dds_reader_delete;
  rd->m_entity.m_deriver.set_qos = dds_reader_qos_set;
  rd->m_entity.m_deriver.validate_status = dds_reader_status_validate;
  rd->m_entity.m_deriver.get_instance_hdl = dds_reader_instance_hdl;

  os_mutexUnlock (&pp_or_sub->m_mutex);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  rd->m_rd = new_reader
  (
    &rd->m_entity.m_guid, NULL, &pp_or_sub->m_participant->m_guid, tp->m_stopic,
    rqos, rhc, dds_reader_status_cb, rd
  );
  assert (rd->m_rd);
  if (asleep)
  {
    thread_state_asleep (thr);
  }

  /* For persistent data register reader with durability */
  if
  (
    dds_global.m_dur_reader &&
    (rd->m_entity.m_qos->durability.kind > NN_TRANSIENT_LOCAL_DURABILITY_QOS)
  )
  {
    (dds_global.m_dur_reader) (rd, rhc);
  }

  return DDS_RETCODE_OK;
}

void dds_reader_ddsi2direct (dds_entity_t entity, ddsi2direct_directread_cb_t cb, void *cbarg)
{
  dds_reader *dds_rd = (dds_reader*) entity;
  struct reader *rd = dds_rd->m_rd;

  {
    nn_guid_t pwrguid;
    struct proxy_writer *pwr;
    struct rd_pwr_match *m;
    memset (&pwrguid, 0, sizeof (pwrguid));
    os_mutexLock (&rd->e.lock);

    rd->ddsi2direct_cb = cb;
    rd->ddsi2direct_cbarg = cbarg;
    while ((m = ut_avlLookupSuccEq (&rd_writers_treedef, &rd->writers, &pwrguid)) != NULL)
    {
      /* have to be careful walking the tree -- pretty is different, but
       I want to check this before I write a lookup_succ function. */
      struct rd_pwr_match *m_next;
      nn_guid_t pwrguid_next;
      pwrguid = m->pwr_guid;
      if ((m_next = ut_avlFindSucc (&rd_writers_treedef, &rd->writers, m)) != NULL)
        pwrguid_next = m_next->pwr_guid;
      else
      {
        memset (&pwrguid_next, 0xff, sizeof (pwrguid_next));
        pwrguid_next.entityid.u = (pwrguid_next.entityid.u & ~0xff) | NN_ENTITYID_KIND_WRITER_NO_KEY;
      }
      os_mutexUnlock (&rd->e.lock);
      if ((pwr = ephash_lookup_proxy_writer_guid (&pwrguid)) != NULL)
      {
        os_mutexLock (&pwr->e.lock);
        pwr->ddsi2direct_cb = cb;
        pwr->ddsi2direct_cbarg = cbarg;
        os_mutexUnlock (&pwr->e.lock);
      }
      pwrguid = pwrguid_next;
      os_mutexLock (&rd->e.lock);
    }
    os_mutexUnlock (&rd->e.lock);
  }
}

uint32_t dds_reader_lock_samples (dds_entity_t entity)
{
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  return (dds_rhc_lock_samples (rd->m_rd->rhc));
}

int dds_reader_wait_for_historical_data
(
  dds_entity_t reader,
  dds_duration_t max_wait
)
{
  int ret = DDS_RETCODE_OK;

  assert (reader);
  assert (reader->m_kind == DDS_TYPE_READER);

  if (reader->m_qos->durability.kind > NN_TRANSIENT_LOCAL_DURABILITY_QOS)
  {
    ret = DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_READER, DDS_ERR_M1);
    if (dds_global.m_dur_wait)
    {
      ret = (dds_global.m_dur_wait) ((dds_reader*) reader, max_wait);
    }
  }

  return ret;
}

dds_entity_t dds_get_subscriber(dds_entity_t rd)
{
    /* TODO: CHAM-104: Return actual errors when dds_entity_t became an handle iso a pointer (see header). */
    if (dds_entity_is_a(rd, DDS_TYPE_READER)) {
        return dds_get_parent(rd);
    }
    if (dds_entity_is_a(rd, DDS_TYPE_COND_READ)) {
        return dds_get_parent(dds_get_parent(rd));
    }
    return NULL;
}

dds_return_t dds_get_subscription_matched_status (dds_entity_t entity, dds_subscription_matched_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_SUBSCRIPTION_MATCHED_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_subscription_matched_status;
            }
            rd->m_subscription_matched_status.total_count_change = 0;
            rd->m_subscription_matched_status.current_count_change = 0;
            dds_entity_status_reset(entity, DDS_SUBSCRIPTION_MATCHED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_liveliness_changed_status (dds_entity_t entity, dds_liveliness_changed_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_LIVELINESS_CHANGED_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_liveliness_changed_status;
            }
            rd->m_liveliness_changed_status.alive_count_change = 0;
            rd->m_liveliness_changed_status.not_alive_count_change = 0;
            dds_entity_status_reset(entity, DDS_LIVELINESS_CHANGED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_sample_rejected_status (dds_entity_t entity, dds_sample_rejected_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_SAMPLE_REJECTED_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_sample_rejected_status;
            }
            rd->m_sample_rejected_status.total_count_change = 0;
            rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
            dds_entity_status_reset(entity, DDS_SAMPLE_REJECTED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_sample_lost_status (dds_entity_t entity, dds_sample_lost_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_SAMPLE_LOST_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_sample_lost_status;
            }
            rd->m_sample_lost_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_SAMPLE_LOST_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_requested_deadline_missed_status (dds_entity_t entity, dds_requested_deadline_missed_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_REQUESTED_DEADLINE_MISSED_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_requested_deadline_missed_status;
            }
            rd->m_requested_deadline_missed_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_REQUESTED_DEADLINE_MISSED_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}

dds_return_t dds_get_requested_incompatible_qos_status (dds_entity_t entity, dds_requested_incompatible_qos_status_t * status)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, 0);
    if (dds_entity_is_a(entity, DDS_TYPE_READER) && (status != NULL)) {
        ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_READER, 0);

        os_mutexLock (&entity->m_mutex);
        if (entity->m_status_enable & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS) {
            dds_reader *rd = (dds_reader*)entity;
            /* status = NULL, application do not need the status, but reset the counter & triggered bit */
            if (status) {
                *status = rd->m_requested_incompatible_qos_status;
            }
            rd->m_requested_incompatible_qos_status.total_count_change = 0;
            dds_entity_status_reset(entity, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
            ret = DDS_RETCODE_OK;
        }
        os_mutexUnlock (&entity->m_mutex);
    }
    return ret;
}
