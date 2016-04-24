#include <assert.h>
#include "kernel/dds_reader.h"
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_status.h"
#include "kernel/dds_statuscond.h"
#include "kernel/dds_init.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"

void dds_reader_status_cb (void * entity, const status_cb_data_t * data)
{
  dds_reader * rd = (dds_reader*) entity;
  bool call = false;
  dds_readerlistener_t listener;
  dds_requested_incompatible_qos_status_t requested_incompatible_qos;
  dds_sample_lost_status_t sample_lost;
  dds_sample_rejected_status_t sample_rejected;
  dds_liveliness_changed_status_t liveliness_changed;
  dds_subscription_matched_status_t subscription_matched;
  void (*on_data_readers) (dds_entity_t subscriber) = NULL;
  dds_subscriber * sub = NULL;
#if 0
  dds_requested_deadline_missed_status_t requested_deadline_missed;
#endif

  if (data == NULL)
  {
    dds_entity_delete_signal (entity);
    return;
  }

  /* Lock entity. Return if pending deletion or status not enabled. */

  if 
  ( 
    (! dds_entity_callback_lock (&rd->m_entity)) ||
    ((rd->m_entity.m_status_enable & data->status) == 0)
  )
  {
    goto done;
  }

  /* Update status metrics */

  listener = rd->m_listener;
  switch (data->status)
  {
#if 0
    /* TODO: NYI in DDSI */
    case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
    {
      rd->m_requested_deadline_missed_status.total_count++;
      rd->m_requested_deadline_missed_status.total_count_change++;
      rd->m_requested_deadline_missed_status.last_instance_handle = data->handle;
      call = listener.on_requested_deadline_missed != NULL;
      if (call)
      {
        requested_deadline_missed = rd->m_requested_deadline_missed_status;
        rd->m_requested_deadline_missed_status.total_count_change = 0;
      }
      break;
    }
#endif
    case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
    {
      rd->m_requested_incompatible_qos_status.total_count++;
      rd->m_requested_incompatible_qos_status.total_count_change++;
      rd->m_requested_incompatible_qos_status.last_policy_id = data->extra;
      call = listener.on_requested_incompatible_qos != NULL;
      if (call)
      {
        requested_incompatible_qos = rd->m_requested_incompatible_qos_status;
        rd->m_requested_incompatible_qos_status.total_count_change = 0;
      }
      break;
    }
    case DDS_SAMPLE_LOST_STATUS:
    {
      rd->m_sample_lost_status.total_count++;
      rd->m_sample_lost_status.total_count_change++;
      call = listener.on_sample_lost != NULL;
      if (call)
      {
        sample_lost = rd->m_sample_lost_status;
        rd->m_sample_lost_status.total_count_change = 0;
      }
      break;
    }
    case DDS_SAMPLE_REJECTED_STATUS:
    {
      rd->m_sample_rejected_status.total_count++;
      rd->m_sample_rejected_status.total_count_change++;
      rd->m_sample_rejected_status.last_reason = data->extra;
      rd->m_sample_rejected_status.last_instance_handle = data->handle;
      call = listener.on_sample_rejected != NULL;
      if (call)
      {
        sample_rejected = rd->m_sample_rejected_status;
        rd->m_sample_rejected_status.total_count_change = 0;
      }
      break;
    }
    case DDS_DATA_AVAILABLE_STATUS:
    {
      call = listener.on_data_available != NULL;
      break;
    }
    case DDS_LIVELINESS_CHANGED_STATUS:
    {
      if (data->add)
      {
        rd->m_liveliness_changed_status.alive_count++;
        rd->m_liveliness_changed_status.alive_count_change++;
	if (rd->m_liveliness_changed_status.not_alive_count > 0)
	{
	  rd->m_liveliness_changed_status.not_alive_count--;
	}
      }
      else
      {
	rd->m_liveliness_changed_status.alive_count--;
        rd->m_liveliness_changed_status.not_alive_count++;
        rd->m_liveliness_changed_status.not_alive_count_change++;
      }
      rd->m_liveliness_changed_status.last_publication_handle = data->handle;
      call = listener.on_liveliness_changed != NULL;
      if (call)
      {
        liveliness_changed = rd->m_liveliness_changed_status;
        rd->m_liveliness_changed_status.alive_count_change = 0;
        rd->m_liveliness_changed_status.not_alive_count_change = 0;
      }
      break;
    }
    case DDS_SUBSCRIPTION_MATCHED_STATUS:
    {
      if (data->add)
      {
        rd->m_subscription_matched_status.total_count++;
        rd->m_subscription_matched_status.total_count_change++;
        rd->m_subscription_matched_status.current_count++;
        rd->m_subscription_matched_status.current_count_change++;
      }
      else
      {
        rd->m_subscription_matched_status.current_count--;
        rd->m_subscription_matched_status.current_count_change--;
      }
      rd->m_subscription_matched_status.last_publication_handle = data->handle;
      call = listener.on_subscription_matched != NULL;
      if (call)
      {
        subscription_matched = rd->m_subscription_matched_status;
        rd->m_subscription_matched_status.total_count_change = 0;
        rd->m_subscription_matched_status.current_count_change = 0;
      }
      break;
    }
    default: assert (0);
  }

  /* DATA_AVAILABLE is handled differently to normal status changes */

  if (data->status != DDS_DATA_AVAILABLE_STATUS)
  {
    if (call)
    {
      /* Reset trigger and call listener unlocked */

      rd->m_entity.m_scond->m_trigger &= ~data->status;
      os_mutexUnlock (&rd->m_entity.m_mutex);
      switch (data->status)
      {
#if 0
    /* TODO: NYI in DDSI */
        case DDS_REQUESTED_DEADLINE_MISSED_STATUS:
          (listener.on_requested_deadline_missed) (&rd->m_entity, &requested_deadline_missed);
          break;
#endif
        case DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS:
          (listener.on_requested_incompatible_qos) (&rd->m_entity, &requested_incompatible_qos);
          break;
        case DDS_SAMPLE_LOST_STATUS:
          (listener.on_sample_lost) (&rd->m_entity, &sample_lost);
          break;
        case DDS_SAMPLE_REJECTED_STATUS:
          (listener.on_sample_rejected) (&rd->m_entity, &sample_rejected);
          break;
        case DDS_LIVELINESS_CHANGED_STATUS:
          (listener.on_liveliness_changed) (&rd->m_entity, &liveliness_changed);
          break;
        case DDS_SUBSCRIPTION_MATCHED_STATUS:
          (listener.on_subscription_matched) (&rd->m_entity, &subscription_matched);
          break;
        default: assert (0);
      }
      os_mutexLock (&rd->m_entity.m_mutex);
    }
    else
    {
      /* Set trigger and signal status condition */

      rd->m_entity.m_scond->m_trigger |= data->status;
      dds_cond_callback_signal (rd->m_entity.m_scond);
    }
    goto done;
  }

  /* 
    DDSI only generates reader DATA_AVAILABLE status which is
    transformed into subscriber DATA_ON_READERS status.
    Subscriber listeners are called before reader listeners.
    Subscriber/reader listeners are called with trigger enabled.
    Listeners are handled before any status condition updates.
    All listener callbacks are made with everything unlocked to
    avoid potential deadlock.
  */

  /* Subscriber listener callback */

  if (rd->m_entity.m_parent->m_kind == DDS_TYPE_SUBSCRIBER)
  {
    sub = (dds_subscriber*) rd->m_entity.m_parent;
      
    if (dds_entity_callback_lock (&sub->m_entity))
    {
      if (sub->m_entity.m_status_enable & DDS_DATA_ON_READERS_STATUS)
      {
        sub->m_entity.m_scond->m_trigger |= DDS_DATA_ON_READERS_STATUS;
        on_data_readers = sub->m_listener.on_data_readers;

        /* Call listener unlocked then reset status */

        if (on_data_readers)
        {
          /* Set data available on reader as subscriber may read/take */

          rd->m_entity.m_scond->m_trigger |= DDS_DATA_AVAILABLE_STATUS;
          os_mutexUnlock (&rd->m_entity.m_mutex);
          os_mutexUnlock (&sub->m_entity.m_mutex);
          (on_data_readers) (&sub->m_entity);
          os_mutexLock (&sub->m_entity.m_mutex);
          os_mutexLock (&rd->m_entity.m_mutex);
          sub->m_entity.m_scond->m_trigger &= ~DDS_DATA_ON_READERS_STATUS;
        }
      }
    }
    else
    {
      goto done;
    }
  }
  
  /* Reader listener callback */

  if (call)
  {
    /* If on_data_readers called may have done a read/take */

    if (on_data_readers == NULL)
    {
      rd->m_entity.m_scond->m_trigger |= DDS_DATA_AVAILABLE_STATUS;
    }
    if (rd->m_entity.m_scond->m_trigger & DDS_DATA_AVAILABLE_STATUS)
    {
      os_mutexUnlock (&rd->m_entity.m_mutex);
      if (sub)
      {
        os_mutexUnlock (&sub->m_entity.m_mutex);
      }
      (listener.on_data_available) (&rd->m_entity);
      os_mutexLock (&rd->m_entity.m_mutex);
      rd->m_entity.m_scond->m_trigger &= ~DDS_DATA_AVAILABLE_STATUS;
      if (sub)
      {
        os_mutexLock (&sub->m_entity.m_mutex);
        sub->m_entity.m_scond->m_trigger &= ~DDS_DATA_ON_READERS_STATUS;
      }
    }
  }

  /* Trigger subscriber status condition */

  if 
  (
    sub && (on_data_readers == NULL) &&
    (sub->m_entity.m_status_enable & DDS_DATA_ON_READERS_STATUS)
  )
  {  
    dds_cond_callback_signal (sub->m_entity.m_scond);
  }

  /* Trigger reader status condition */

  if (!call)
  {
    rd->m_entity.m_scond->m_trigger |= DDS_DATA_AVAILABLE_STATUS;
    dds_cond_callback_signal (rd->m_entity.m_scond);
  }

done:

  if (sub)
  {
    dds_entity_callback_unlock (&sub->m_entity);
  }
  dds_entity_callback_unlock (&rd->m_entity);
}

int dds_reader_create
(
  dds_entity_t pp_or_sub,
  dds_entity_t * reader,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_readerlistener_t * listener
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
  
  ret = dds_qos_validate (DDS_TYPE_READER, rqos);
  if (ret != 0)
  {
    os_mutexUnlock (&pp_or_sub->m_mutex);
    return ret;
  }
  
  /* Create reader and associated read cache */
  rd = dds_alloc (sizeof (*rd));
  *reader = &rd->m_entity;
  dds_entity_init (&rd->m_entity, pp_or_sub, DDS_TYPE_READER, rqos);
  rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
  rd->m_topic = tp;
  dds_entity_add_ref (topic);
  rhc = dds_rhc_new (rd, tp->m_stopic);

  /* Merge listener with those from parent */
  if (listener)
  {
    rd->m_listener = *listener;
  }
  if (sub)
  {
    rd->m_data_on_readers = (sub->m_entity.m_status_enable & DDS_DATA_ON_READERS_STATUS);
    dds_subscriberlistener_t l;
    dds_listener_get_unl (pp_or_sub, &l);
    dds_listener_merge (&rd->m_listener, &l.readerlistener, DDS_TYPE_READER);
  }
  else
  {
    dds_participantlistener_t l;
    dds_listener_get_unl (pp_or_sub, &l);
    dds_listener_merge (&rd->m_listener, &l.subscriberlistener.readerlistener, DDS_TYPE_READER);
  }
  os_mutexUnlock (&pp_or_sub->m_mutex);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  rd->m_rd = new_reader 
  (
    &rd->m_entity.m_guid, NULL, &pp_or_sub->m_pp->m_entity.m_guid, tp->m_stopic,
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
