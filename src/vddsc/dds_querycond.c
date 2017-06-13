#include <assert.h>
#include "kernel/dds_types.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_querycond.h"
#include "kernel/dds_readcond.h"
#include "ddsi/ddsi_ser.h"

dds_condition_t dds_querycondition_create
  (dds_entity_t reader, uint32_t mask, dds_querycondition_filter_fn filter)
{
  dds_condition_t cond;
  dds_topic *topic;
  dds_entity *r;
  int32_t ret;

  assert (filter);
  cond = dds_readcondition_create (reader, mask);
  if (cond) {
      cond->m_kind = DDS_TYPE_COND_QUERY;
      ((dds_readcond*) cond)->m_query.u.m_filter = filter;
      ((dds_readcond*) cond)->m_query.m_cxx_ctx = NULL;

      ret = dds_entity_lock(reader, DDS_KIND_READER, &r);
      if (ret == DDS_RETCODE_OK) {
          topic = (dds_topic *)(((dds_reader *)r)->m_topic);
          dds_entity_unlock(r);
          os_mutexLock (&topic->m_entity.m_mutex);
          if (topic->m_stopic->filter_sample == NULL)
          {
            topic->m_stopic->filter_sample = dds_alloc (topic->m_descriptor->m_size);
          }
          os_mutexUnlock (&topic->m_entity.m_mutex);
      } else {
          dds_condition_delete(cond);
          cond = NULL;
      }
  }
  return cond;
}

void dds_querycondition_from_readcondition
   (dds_condition_t cond, dds_entity_t reader)
{
  dds_topic *topic;
  dds_entity *r;
  int32_t ret;

  cond->m_kind = DDS_TYPE_COND_QUERY;
  ((dds_readcond*) cond)->m_query.u.m_filter = NULL;
  ((dds_readcond*) cond)->m_query.m_cxx_ctx = NULL;

  ret = dds_entity_lock(reader, DDS_KIND_READER, &r);
  if (ret == DDS_RETCODE_OK) {
      topic = (dds_topic *)(((dds_reader *)r)->m_topic);
      dds_entity_unlock(r);
      os_mutexLock (&topic->m_entity.m_mutex);
      if (topic->m_stopic->filter_sample == NULL)
      {
        topic->m_stopic->filter_sample = dds_alloc (topic->m_descriptor->m_size);
      }
      os_mutexUnlock (&topic->m_entity.m_mutex);
  }
}


void dds_querycondition_set_filter_with_ctx(dds_querycondition_filter_with_ctx_fn filter, dds_condition_t cond, void *ctx)
{
  ((dds_readcond*) cond)->m_query.u.m_filter_with_ctx = filter;
  ((dds_readcond*) cond)->m_query.m_cxx_ctx = ctx;
}
