#include <assert.h>
#include "kernel/dds_condition.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_guardcond.h"
#include "kernel/dds_waitset.h"
#include "ddsi/q_globals.h"

dds_condition_t dds_guardcondition_create (void)
{
  dds_guardcond * guard = dds_alloc (sizeof (*guard));
  guard->m_cond.m_kind = DDS_TYPE_COND_GUARD;
  os_mutexInit (&guard->m_lock);
  guard->m_cond.m_lock = &guard->m_lock;
  return (dds_condition_t) guard;
}

static void dds_guard_set_trigger (dds_condition_t cond, bool value)
{
  assert (cond);
  assert (cond->m_kind == DDS_TYPE_COND_GUARD);

  os_mutexLock (&gv.attach_lock);
  os_mutexLock (cond->m_lock);
  if (!value)
  {
    cond->m_trigger = 0;
  }
  else if (!cond->m_trigger)
  {
    cond->m_trigger = 1;
    dds_cond_signal_waitsets_locked (cond);
  }
  os_mutexUnlock (cond->m_lock);
  os_mutexUnlock (&gv.attach_lock);
}

void dds_guard_trigger (dds_condition_t guard)
{
  dds_guard_set_trigger (guard, true);
}

void dds_guard_reset (dds_condition_t guard)
{
  dds_guard_set_trigger (guard, false);
}
