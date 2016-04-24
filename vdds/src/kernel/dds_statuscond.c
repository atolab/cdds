#include <assert.h>
#include "os/os.h"
#include "ddsi/q_globals.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_statuscond.h"
#include "kernel/dds_condition.h"

dds_condition_t dds_statuscondition_get (dds_entity_t e)
{
  assert (e);
  return (e->m_scond);
}

dds_statuscond * dds_statuscond_create (void)
{
  dds_statuscond * cond = dds_alloc (sizeof (*cond));
  cond->m_kind = DDS_TYPE_COND_STATUS;
  return cond;
}
