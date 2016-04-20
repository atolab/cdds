#include <assert.h>
#include "dds_readcond.h"
#include "dds_waitset.h"
#include "dds_rhc.h"
#include "q_ephash.h"
#include "q_entity.h"
#include "q_thread.h"

dds_condition_t dds_readcondition_create (dds_entity_t rd, uint32_t mask)
{
  dds_readcond * cond = dds_alloc (sizeof (*cond));
  dds_reader * reader = (dds_reader*) rd;

  assert (rd);
  assert (rd->m_kind == DDS_TYPE_READER);
  assert (reader->m_rd);

  cond->m_cond.m_kind = DDS_TYPE_COND_READ;
  cond->m_rhc = reader->m_rd->rhc;
  cond->m_sample_states = mask & DDS_ANY_SAMPLE_STATE;
  cond->m_view_states = mask & DDS_ANY_VIEW_STATE;
  cond->m_instance_states = mask & DDS_ANY_INSTANCE_STATE;
  cond->m_rd_guid = rd->m_guid;
  dds_rhc_add_readcondition (cond);

  return (dds_condition_t) cond;
}
