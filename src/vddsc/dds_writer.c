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

static dds_return_t dds_writer_instance_hdl(dds_entity *e, dds_instance_handle_t *i)
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
    dds_writer *wr;
    dds_return_t ret;
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
    ret = dds_entity_listener_propagation(entity, entity, data->status, metrics, true);

    if (ret == DDS_RETCODE_OK) {
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
    } else if (ret == DDS_RETCODE_NO_DATA) {
        /* Nobody was interested through a listener (NO_DATA == NO_CALL): set the status. */
        dds_entity_status_set(entity, data->status);
        /* Notify possible interested observers. */
        dds_entity_status_signal(entity);
    } else {
        /* Something went wrong up the hierarchy.
         * Likely, a parent is in the process of being deleted. */
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

static dds_return_t dds_writer_close(dds_entity *e)
{
    dds_return_t ret = DDS_RETCODE_OK;
    dds_writer *wr = (dds_writer*)e;
    struct thread_state1 * const thr = lookup_thread_state();
    const bool asleep = !vtime_awake_p(thr->vtime);

    assert(e);
    assert(thr);

    if (asleep) {
      thread_state_awake(thr);
    }
    nn_xpack_send (wr->m_xp, false);
    if (delete_writer (&e->m_guid) != 0) {
        ret = DDS_RETCODE_ERROR;
    }
    if (asleep) {
      thread_state_asleep(thr);
    }
    return ret;
}

static dds_return_t dds_writer_delete(dds_entity *e)
{
    dds_writer *wr = (dds_writer*)e;
    struct thread_state1 * const thr = lookup_thread_state();
    const bool asleep = !vtime_awake_p(thr->vtime);
    dds_return_t ret;

    assert(e);
    assert(thr);

    if (asleep) {
      thread_state_awake(thr);
    }
    nn_xpack_free(wr->m_xp);
    if (asleep) {
      thread_state_asleep(thr);
    }
    ret = dds_delete(wr->m_topic->m_entity.m_hdl);
    os_mutexDestroy(&wr->m_call_lock);
    return ret;
}


static dds_return_t dds_writer_qos_validate (const dds_qos_t *qos, bool enabled)
{
#if 1
    return DDS_RETCODE_OK; //TODO: CHAM-170 Properly implement
#else
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
#endif
}

static dds_return_t dds_writer_qos_set (dds_entity *e, const dds_qos_t *qos, bool enabled)
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


_Pre_satisfies_(((participant_or_publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PUBLISHER  ) ||\
                ((participant_or_publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT) )
_Pre_satisfies_( (topic & DDS_ENTITY_KIND_MASK) == DDS_KIND_TOPIC )
dds_entity_t
dds_create_writer(
        _In_ dds_entity_t participant_or_publisher,
        _In_ dds_entity_t topic,
        _In_opt_ const dds_qos_t * qos,
        _In_opt_ const dds_listener_t * listener)
{
  int32_t errnr;
  dds_qos_t * wqos;
  dds_publisher * pub = NULL;
  dds_writer * wr;
  dds_entity * parent = NULL;
  dds_entity * tp;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  ddsi_tran_conn_t conn = gv.data_conn_mc ? gv.data_conn_mc : gv.data_conn_uc;
  int ret = DDS_RETCODE_OK;
  dds_entity_t writer;

  /* Try claiming a participant. If that's not working, then it could be a subscriber. */
  errnr = dds_entity_lock(participant_or_publisher, DDS_KIND_PARTICIPANT, &parent);
  if (errnr != DDS_RETCODE_OK) {
      if (errnr == DDS_RETCODE_ILLEGAL_OPERATION) {
          errnr = dds_entity_lock(participant_or_publisher, DDS_KIND_PUBLISHER, &parent);
          if (errnr != DDS_RETCODE_OK) {
              return (int)DDS_ERRNO(errnr, DDS_MOD_WRITER, DDS_ERR_M1);
          }
          pub = (dds_publisher*)parent;
      } else {
          return (int)DDS_ERRNO(errnr, DDS_MOD_WRITER, DDS_ERR_M2);
      }
  }

  errnr = dds_entity_lock(topic, DDS_KIND_TOPIC, &tp);
  if (errnr != DDS_RETCODE_OK) {
      dds_entity_unlock(parent);
      return (int)DDS_ERRNO(errnr, DDS_MOD_WRITER, DDS_ERR_M3);
  }
  assert (((dds_topic*)tp)->m_stopic);
  assert (parent->m_domain == tp->m_domain);

  /* Merge Topic & Publisher qos */

  wqos = dds_qos_create ();
  if (qos)
  {
    /* Only returns failure when one of the qos args is NULL, which
     * is not the case here. */
    (void)dds_qos_copy(wqos, qos);
  }
  if (pub && pub->m_entity.m_qos)
  {
    dds_qos_merge (wqos, pub->m_entity.m_qos);
  }

  if (tp->m_qos)
  {
    /* merge topic qos data to writer qos */
    dds_qos_merge (wqos, tp->m_qos);
  }
  nn_xqos_mergein_missing (wqos, &gv.default_xqos_wr);

  ret = (int)dds_writer_qos_validate (wqos, false);
  if (ret != 0)
  {
    dds_entity_unlock(tp);
    dds_entity_unlock(parent);
    return ret;
  }

  /* Create writer */
  wr = dds_alloc (sizeof (*wr));
  writer = dds_entity_init (&wr->m_entity, parent, DDS_KIND_WRITER, wqos, listener, DDS_WRITER_STATUS_MASK);
  wr->m_topic = (dds_topic*)tp;
  dds_entity_unlock(tp);
  dds_entity_add_ref (tp);
  wr->m_xp = nn_xpack_new (conn, get_bandwidth_limit(wqos->transport_priority), config.xpack_send_async);
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

  /* TODO: When calling new_writer, we should unlock the parent.
   *       The reason for that is that it is possible that this new_writer call generates
   *       a call to dds_writer_status_cb (when it notices a publication match f.i.),
   *       which will try to lock the parent when handling that event in the listeners.
   *       Causing a deadlock off course.
   *
   *       In other tickets, this piece of code has changed, so we probably will have some
   *       merge conflicts. We should come up with a proper solution after the merge.
   *       Probably just unlock the parent and topic mutex, without releasing their handles,
   *       which will delay their possible deletions until we release their handles.
   *       This means that their content will not be deleted but they're still unlocked.
   */
  dds_entity_unlock(parent);

  if (asleep)
  {
    thread_state_awake (thr);
  }

//TODO: CHAM-170 Fix unsafe tp usage
  wr->m_wr = new_writer (&wr->m_entity.m_guid, NULL, &parent->m_participant->m_guid, ((dds_topic*)tp)->m_stopic,
                         wqos, dds_writer_status_cb, wr);
  assert (wr->m_wr);
  if (asleep)
  {
    thread_state_asleep (thr);
  }
  return writer;
}

_Pre_satisfies_(((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER))
dds_entity_t
dds_get_publisher(
        _In_ dds_entity_t writer)
{
    if (writer > 0) {
        if (dds_entity_kind(writer) == DDS_KIND_WRITER) {
            return dds_get_parent(writer);
        } else {
            return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_WRITER, DDS_ERR_M1);
        }
    }
    return writer;
}

dds_return_t dds_get_publication_matched_status (dds_entity_t entity, dds_publication_matched_status_t * status)
{
    int32_t errnr;
    dds_writer *wr;

    errnr = dds_writer_lock(entity, &wr);
    if (errnr == DDS_RETCODE_OK) {
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
    return DDS_ERRNO(errnr, DDS_MOD_WRITER, 0);
}

dds_return_t dds_get_liveliness_lost_status (dds_entity_t entity, dds_liveliness_lost_status_t * status)
{
    int32_t errnr;
    dds_writer *wr;

    errnr = dds_writer_lock(entity, &wr);
    if (errnr == DDS_RETCODE_OK) {
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
    return DDS_ERRNO(errnr, DDS_MOD_WRITER, 0);
}

dds_return_t dds_get_offered_deadline_missed_status (dds_entity_t entity, dds_offered_deadline_missed_status_t * status)
{
    int32_t errnr;
    dds_writer *wr;

    errnr = dds_writer_lock(entity, &wr);
    if (errnr == DDS_RETCODE_OK) {
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
    return DDS_ERRNO(errnr, DDS_MOD_WRITER, 0);
}

dds_return_t dds_get_offered_incompatible_qos_status (dds_entity_t entity, dds_offered_incompatible_qos_status_t * status)
{
    int32_t errnr;
    dds_writer *wr;

    errnr = dds_writer_lock(entity, &wr);
    if (errnr == DDS_RETCODE_OK) {
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
    return DDS_ERRNO(errnr, DDS_MOD_WRITER, 0);
}
