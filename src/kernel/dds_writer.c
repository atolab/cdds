#include <assert.h>
#include "ddsi/q_config.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_status.h"
#include "kernel/dds_statuscond.h"
#include "kernel/dds_init.h"
#include "kernel/dds_tkmap.h"

#define DDS_STATUS_MASK 0x000000ff

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
  dds_writerlistener_t listener;
  dds_offered_incompatible_qos_status_t offered_incompatible_qos;
  dds_publication_matched_status_t publication_matched;

#if 0
  dds_liveliness_lost_status_t liveliness_lost;
  dds_offered_deadline_missed_status_t offered_deadline_missed;
#endif

  if (data == NULL)
  {
    dds_entity_delete_signal (entity);
    return;
  }

  /* Lock entity. Return if pending deletion or status not enabled. */

  if
  (
    (! dds_entity_callback_lock (&wr->m_entity)) ||
    ((wr->m_entity.m_status_enable & data->status) == 0)
  )
  {
    goto done;
  }

  /* Update status metrics */

  listener = wr->m_listener;
  switch (data->status)
  {
#if 0
    /* TODO: NYI in DDSI */
    case DDS_OFFERED_DEADLINE_MISSED_STATUS:
    {
      wr->m_offered_deadline_missed_status.total_count++;
      wr->m_offered_deadline_missed_status.total_count_change++;
      wr->m_offered_deadline_missed_status.last_instance_handle = data->handle;
      call = listener.on_offered_deadline_missed != NULL;
      if (call)
      {
        offered_deadline_missed = wr->m_offered_deadline_missed_status;
        wr->m_offered_deadline_missed_status.total_count_change = 0;
      }
      break;
    }
    case DDS_LIVELINESS_LOST_STATUS:
    {
      wr->m_liveliness_lost_status.total_count++;
      wr->m_liveliness_lost_status.total_count_change++;
      call = listener.on_liveliness_lost != NULL;
      if (call)
      {
        liveliness_lost = wr->m_liveliness_lost_status;
        wr->m_liveliness_lost_status.total_count_change = 0;
      }
      break;
    }
#endif
    case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS:
    {
      wr->m_offered_incompatible_qos_status.total_count++;
      wr->m_offered_incompatible_qos_status.total_count_change++;
      wr->m_offered_incompatible_qos_status.last_policy_id = data->extra;
      call = listener.on_offered_incompatible_qos != NULL;
      if (call)
      {
        offered_incompatible_qos = wr->m_offered_incompatible_qos_status;
        wr->m_offered_incompatible_qos_status.total_count_change = 0;
      }
      break;
    }
    case DDS_PUBLICATION_MATCHED_STATUS:
    {
      if (data->add)
      {
        wr->m_publication_matched_status.total_count++;
        wr->m_publication_matched_status.total_count_change++;
        wr->m_publication_matched_status.current_count++;
        wr->m_publication_matched_status.current_count_change++;
      }
      else
      {
        wr->m_publication_matched_status.current_count--;
        wr->m_publication_matched_status.current_count_change--;
      }
      wr->m_publication_matched_status.last_subscription_handle = data->handle;
      call = listener.on_publication_matched != NULL;
      if (call)
      {
        publication_matched = wr->m_publication_matched_status;
        wr->m_publication_matched_status.total_count_change = 0;
        wr->m_publication_matched_status.current_count_change = 0;
      }
      break;
    }
    default: assert (0);
  }

  /* Either call listener or signal status condition */

  if (call)
  {
    /* Reset trigger and call listener unlocked */

    wr->m_entity.m_scond->m_trigger &= ~data->status;
    os_mutexUnlock (&wr->m_entity.m_mutex);
    switch (data->status)
    {
#if 0
    /* TODO: NYI in DDSI */
      case DDS_OFFERED_DEADLINE_MISSED_STATUS:
        (listener.on_offered_deadline_missed) (&wr->m_entity, &offered_deadline_missed);
        break;
      case DDS_LIVELINESS_LOST_STATUS:
        (listener.on_liveliness_lost) (&wr->m_entity, &liveliness_lost);
        break;
#endif
      case DDS_OFFERED_INCOMPATIBLE_QOS_STATUS:
        (listener.on_offered_incompatible_qos) (&wr->m_entity, &offered_incompatible_qos);
        break;
      case DDS_PUBLICATION_MATCHED_STATUS:
        (listener.on_publication_matched) (&wr->m_entity, &publication_matched);
        break;
      default: assert (0);
    }
    os_mutexLock (&wr->m_entity.m_mutex);
  }
  else
  {
    /* Set trigger and signal status condition */

    wr->m_entity.m_scond->m_trigger |= data->status;
    dds_cond_callback_signal (wr->m_entity.m_scond);
  }

done:
  dds_entity_callback_unlock (&wr->m_entity);
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

int dds_writer_create
(
  dds_entity_t pp_or_pub,
  dds_entity_t * writer,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_writerlistener_t * listener
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

  ret = dds_qos_validate (DDS_TYPE_WRITER, wqos);
  if (ret != 0)
  {
    os_mutexUnlock (&pp_or_pub->m_mutex);
    return ret;
  }

  /* Create writer */
  wr = dds_alloc (sizeof (*wr));
  *writer = &wr->m_entity;
  dds_entity_init (&wr->m_entity, pp_or_pub, DDS_TYPE_WRITER, wqos);
  wr->m_topic = tp;
  dds_entity_add_ref (topic);
  wr->m_xp = nn_xpack_new (conn, get_bandwidth_limit(wqos->transport_priority), config.xpack_send_async);
  os_mutexInit (&wr->m_call_lock);

  /* Merge listener functions with those from parent */

  if (listener)
  {
    wr->m_listener = *listener;
  }
  if (pp_or_pub->m_kind == DDS_TYPE_PARTICIPANT)
  {
    dds_participantlistener_t l;
    dds_listener_get_unl (pp_or_pub, &l);
    dds_listener_merge (&wr->m_listener, &l.publisherlistener.writerlistener, DDS_TYPE_WRITER);
  }
  else
  {
    dds_publisherlistener_t l;
    dds_listener_get_unl (pp_or_pub, &l);
    dds_listener_merge (&wr->m_listener, &l.writerlistener, DDS_TYPE_WRITER);
  }
  os_mutexUnlock (&pp_or_pub->m_mutex);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  wr->m_wr = new_writer (&wr->m_entity.m_guid, NULL, &pp_or_pub->m_pp->m_entity.m_guid, tp->m_stopic, wqos,
  dds_writer_status_cb, wr);
  assert (wr->m_wr);
  if (asleep)
  {
    thread_state_asleep (thr);
  }
  return DDS_RETCODE_OK;
}
