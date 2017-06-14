#include <assert.h>
#include "kernel/dds_reader.h"
#include "kernel/dds_readcond.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_entity.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER)
dds_condition_t
dds_readcondition_create(
        dds_entity_t reader,
        uint32_t mask)
{
  dds_readcond * cond = NULL;
  dds_reader * rd;
  int32_t ret;

  ret = dds_reader_lock(reader, &rd);
  if (ret == DDS_RETCODE_OK) {
      cond = dds_alloc (sizeof (*cond));
      cond->m_cond.m_kind = DDS_TYPE_COND_READ;
      cond->m_rhc = rd->m_rd->rhc;
      cond->m_sample_states = mask & DDS_ANY_SAMPLE_STATE;
      cond->m_view_states = mask & DDS_ANY_VIEW_STATE;
      cond->m_instance_states = mask & DDS_ANY_INSTANCE_STATE;
      cond->m_rd_guid = ((dds_entity*)rd)->m_guid;
      dds_rhc_add_readcondition (cond);
      dds_reader_unlock(rd);
  }

  return (dds_condition_t) cond;
}

_Pre_satisfies_(((readcond & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((readcond & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY) )
dds_entity_t
dds_get_datareader(
        _In_ dds_entity_t readcond)
{
    if (readcond > 0) {
#if 0
        /* TODO: CHAM-106: Return actual reader and errors when conditions are entities. */
        if (dds_entity_kind(readcond) == DDS_KIND_COND_READ) {
            return dds_get_parent(readcond);
        } else if (dds_entity_kind(readcond) == DDS_KIND_COND_QUERY) {
            return dds_get_parent(readcond);
        } else {
            return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION, DDS_MOD_READER, DDS_ERR_M1);
        }
#else
        return (dds_entity_t)DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_READER, DDS_ERR_M1);
#endif
    }
    return readcond;
}
