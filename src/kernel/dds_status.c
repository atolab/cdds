#include <assert.h>
#include "kernel/dds_status.h"

const uint32_t dds_status_masks [DDS_ENTITY_NUM] =
{
  DDS_TOPIC_STATUS_MASK, DDS_PARTICIPANT_STATUS_MASK,
  DDS_READER_STATUS_MASK, DDS_WRITER_STATUS_MASK,
  DDS_SUBSCRIBER_STATUS_MASK, DDS_PUBLISHER_STATUS_MASK
};

/* Check if status(s) supported on entity type */

int dds_status_check (dds_entity_kind_t kind, uint32_t mask)
{
  return (mask & ~(dds_status_masks[kind & DDS_TYPE_INDEX_MASK])) ?
    DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_STATUS, DDS_ERR_M1) :
    DDS_RETCODE_OK;
}

/* 
  Set boolean on readers that indicates state of DATA_ON_READERS
  status on parent subscriber
*/
static void dds_status_data_on_readers (dds_entity_t sub, bool set)
{
  dds_entity_t iter = sub->m_children;
  while (iter)
  {
    os_mutexLock (&iter->m_mutex);
    ((dds_reader*) iter)->m_data_on_readers = set;
    os_mutexUnlock (&iter->m_mutex);
    iter = iter->m_next;
  }
}

/* Read status condition based on mask */

int dds_status_read (dds_entity_t e, uint32_t * status, uint32_t mask)
{
  int ret;

  assert (e);
  assert (status);

  ret = dds_status_check (e->m_kind, mask);
  if (ret == DDS_RETCODE_OK)
  {
    os_mutexLock (&e->m_mutex);
    *status = e->m_scond->m_trigger & mask;
    os_mutexUnlock (&e->m_mutex);
  }
  return ret;
}

/* Take and clear status condition based on mask */

int dds_status_take (dds_entity_t e, uint32_t * status, uint32_t mask)
{
  int ret;

  assert (e);
  assert (status);

  ret = dds_status_check (e->m_kind, mask);
  if (ret == DDS_RETCODE_OK)
  {
    os_mutexLock (&e->m_mutex);
    *status = e->m_scond->m_trigger & mask;
    if (*status && (e->m_kind == DDS_TYPE_SUBSCRIBER))
    {
      dds_status_data_on_readers (e, false);
    }
    e->m_scond->m_trigger &= ~mask;
    os_mutexUnlock (&e->m_mutex);
  }
  return ret;
}

uint32_t dds_status_changes (dds_entity_t e)
{
  uint32_t change;
  os_mutexLock (&e->m_mutex);
  change = e->m_scond->m_trigger;
  os_mutexUnlock (&e->m_mutex);
  return change;
}

uint32_t dds_status_get_enabled (dds_entity_t e)
{
  uint32_t ret;
  assert (e);

  os_mutexLock (&e->m_mutex);
  ret = e->m_status_enable;
  os_mutexUnlock (&e->m_mutex);
  return ret;
}

int dds_status_set_enabled (dds_entity_t e, uint32_t mask)
{
  int ret;
  assert (e);

  ret = dds_status_check (e->m_kind, mask);
  if (ret == DDS_RETCODE_OK)
  {
    os_mutexLock (&e->m_mutex);
    e->m_status_enable = mask;
    e->m_scond->m_trigger &= mask;
    
    if (mask && (e->m_kind == DDS_TYPE_SUBSCRIBER))
    {
      dds_status_data_on_readers (e, true);
    }
    os_mutexUnlock (&e->m_mutex);
  }
  return ret;
}

int dds_get_publication_matched_status (dds_entity_t entity, dds_publication_matched_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M6);
  dds_writer * wr = (dds_writer*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_WRITER);

  os_mutexLock (&entity->m_mutex);

  /* check for status enabled flag */

  if (entity->m_status_enable & DDS_PUBLICATION_MATCHED_STATUS)
  {
    /* status = NULL, application do not need the status, but reset the counter & triggered bit */
    if (status)
    {
      *status = wr->m_publication_matched_status;
    }
    wr->m_publication_matched_status.total_count_change = 0;
    wr->m_publication_matched_status.current_count_change = 0;

    /* reset the bit corresponding to status after read */
    entity->m_scond->m_trigger &= ~DDS_PUBLICATION_MATCHED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_liveliness_lost_status (dds_entity_t entity, dds_liveliness_lost_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M7);
  dds_writer * wr = (dds_writer*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_WRITER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_LIVELINESS_LOST_STATUS)
  {
    if (status)
    {
      *status = wr->m_liveliness_lost_status;
    }
    wr->m_liveliness_lost_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_LIVELINESS_LOST_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_offered_deadline_missed_status (dds_entity_t entity, dds_offered_deadline_missed_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M8);
  dds_writer * wr = (dds_writer*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_WRITER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_OFFERED_DEADLINE_MISSED_STATUS)
  {
    if (status)
    {
      *status = wr->m_offered_deadline_missed_status;
    }
    wr->m_offered_deadline_missed_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_OFFERED_DEADLINE_MISSED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_offered_incompatible_qos_status (dds_entity_t entity, dds_offered_incompatible_qos_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M9);
  dds_writer * wr = (dds_writer*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_WRITER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)
  {
    if (status)
    {
      *status = wr->m_offered_incompatible_qos_status;
    }
    wr->m_offered_incompatible_qos_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_subscription_matched_status (dds_entity_t entity, dds_subscription_matched_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M10);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&rd->m_entity.m_mutex);
  if (rd->m_entity.m_status_enable & DDS_SUBSCRIPTION_MATCHED_STATUS)
  {
    if (status)
    {
      *status = rd->m_subscription_matched_status;
    }
    rd->m_subscription_matched_status.total_count_change = 0;
    rd->m_subscription_matched_status.current_count_change = 0;
    rd->m_entity.m_scond->m_trigger &= ~DDS_SUBSCRIPTION_MATCHED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&rd->m_entity.m_mutex);

  return ret;
}

int dds_get_liveliness_changed_status (dds_entity_t entity, dds_liveliness_changed_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M11);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_LIVELINESS_CHANGED_STATUS)
  {
    if (status)
    {
      *status = rd->m_liveliness_changed_status;
    }
    rd->m_liveliness_changed_status.alive_count_change = 0;
    rd->m_liveliness_changed_status.not_alive_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_LIVELINESS_CHANGED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_sample_rejected_status (dds_entity_t entity, dds_sample_rejected_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M12);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_SAMPLE_REJECTED_STATUS)
  {
    if (status)
    {
      *status = rd->m_sample_rejected_status;
    }
    rd->m_sample_rejected_status.total_count_change = 0;
    rd->m_sample_rejected_status.last_reason = DDS_NOT_REJECTED;
    entity->m_scond->m_trigger &= ~DDS_SAMPLE_REJECTED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);
  
  return ret;
}

int dds_get_sample_lost_status (dds_entity_t entity, dds_sample_lost_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M13);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_SAMPLE_LOST_STATUS)
  {
    if (status)
    {
      *status = rd->m_sample_lost_status;
    }
    rd->m_sample_lost_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_SAMPLE_LOST_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);
  
  return ret;
}

int dds_get_requested_deadline_missed_status (dds_entity_t entity, dds_requested_deadline_missed_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M14);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_REQUESTED_DEADLINE_MISSED_STATUS)
  {
    if (status)
    {
      *status = rd->m_requested_deadline_missed_status;
    }
    rd->m_requested_deadline_missed_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);
  
  return ret; 
}

int dds_get_requested_incompatible_qos_status (dds_entity_t entity, dds_requested_incompatible_qos_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M15);
  dds_reader * rd = (dds_reader*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_READER);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)
  {
    if (status)
    {
      *status = rd->m_requested_incompatible_qos_status;
    }
    rd->m_requested_incompatible_qos_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}

int dds_get_inconsistent_topic_status (dds_entity_t entity, dds_inconsistent_topic_status_t * status)
{
  int ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_STATUS, DDS_ERR_M16);
  dds_topic * tp = (dds_topic*) entity;

  assert (entity);
  assert (entity->m_kind == DDS_TYPE_TOPIC);

  os_mutexLock (&entity->m_mutex);
  if (entity->m_status_enable & DDS_INCONSISTENT_TOPIC_STATUS)
  {
    if (status)
    {
      *status = tp->m_inconsistent_topic_status;
    }
    tp->m_inconsistent_topic_status.total_count_change = 0;
    entity->m_scond->m_trigger &= ~DDS_INCONSISTENT_TOPIC_STATUS;
    ret = DDS_RETCODE_OK;
  }
  os_mutexUnlock (&entity->m_mutex);

  return ret;
}
