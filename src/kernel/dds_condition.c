#include <assert.h>
#include "kernel/dds_condition.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_guardcond.h"
#include "kernel/dds_statuscond.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_globals.h"

void dds_condition_delete (dds_condition * cond)
{
  dds_waitset * ws;

  /* status condition delete is handled as part of entity_delete */

  assert (cond);

  do
  {
    os_mutexLock (cond->m_lock);
    ws = cond->m_waitsets ? cond->m_waitsets->m_waitset : NULL;
    if (ws)
    {
      dds_waitset_remove_condition_locked (ws, cond);
    }
    os_mutexUnlock (cond->m_lock);
  }
  while (ws);

  if (cond->m_kind & DDS_TYPE_COND_READ)
  {
    dds_rhc_remove_readcondition ((dds_readcond*) cond);
  }

  /* Only guard condition actually uses it's own lock */

  if (cond->m_kind == DDS_TYPE_COND_GUARD)
  {
    os_mutexDestroy (cond->m_lock);
  }
  dds_free (cond);
}

/* 
  dds_cond_callback_signal: Called from reader/writer callback handler
  with condition already locked. If can't get global attach lock need to
  unlock condition until can, to avoid deadlock (typically caused by
  condition concurrently being detached from waitset).
*/
   
void dds_cond_callback_signal (dds_condition * cond)
{
  bool unlocked = false;

  if (cond->m_waitsets)
  {
    /* Need to get global attach lock or can deadlock */

    while (os_mutexTryLock (&gv.attach_lock) != os_resultSuccess)
    {
      if (! unlocked)
      {
        os_mutexUnlock (cond->m_lock);
        unlocked = true;
      }
      dds_sleepfor (DDS_MSECS (1));
    }
    if (unlocked)
    {
      os_mutexLock (cond->m_lock);
    }
    dds_cond_signal_waitsets_locked (cond);
    os_mutexUnlock (&gv.attach_lock);
  }
}

void dds_cond_signal_waitsets_locked (dds_condition * cond)
{
  dds_ws_cond_link * ws = cond->m_waitsets;
  while (ws)
  {
    dds_waitset_signal (ws->m_waitset);
    ws = ws->m_ws_next;
  }
}

bool dds_condition_triggered (dds_condition * cond)
{
  bool triggered = false;
  assert (cond);

  os_mutexLock (cond->m_lock);
  triggered = cond->m_trigger != 0;
  os_mutexUnlock (cond->m_lock);

  return triggered;
}

bool dds_condition_add_waitset (dds_condition * cond, dds_waitset * ws, dds_attach_t x)
{
  bool ret;
  os_mutexLock (cond->m_lock);
  ret = dds_waitset_add_condition_locked (ws, cond, x);
  os_mutexUnlock (cond->m_lock);
  return ret;
}

int dds_condition_remove_waitset (dds_condition * cond, dds_waitset * ws)
{
  int ret;
  os_mutexLock (cond->m_lock);
  ret = dds_waitset_remove_condition_locked (ws, cond);
  os_mutexUnlock (cond->m_lock);
  return ret;
}
