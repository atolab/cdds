#include <assert.h>
#include "os/os.h"
#include "kernel/dds_types.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_condition.h"
#include "kernel/dds_guardcond.h"
#include "kernel/dds_readcond.h"
#include "kernel/dds_statuscond.h"
#include "kernel/dds_rhc.h"

#include "ddsi/q_globals.h"
#include "ddsi/q_config.h"
#include "ddsi/q_log.h"

/*
  Waitsets and conditions form an N:M relationship. This is implemented with
  a dds_ws_cond_link which implements a single relation. The waitset has a list
  of conditions with which it is associated and the condition has a list of
  waitsets with which it is associated. Both relationships are supported with a single
  dds_ws_cond_link which acts as an entry in two separate linked lists (one for
  the condition and one for the waitset).
*/

static bool cond_is_attached_locked (const dds_condition * cond, const dds_waitset * ws)
{
  dds_ws_cond_link * wsl = cond->m_waitsets;

  while (wsl && (wsl->m_waitset != ws))
  {
    wsl = wsl->m_ws_next;
  }
  return (wsl != NULL);
}

static int dds_waitset_wait_impl_locked
(
  dds_waitset *waitset, dds_attach_t *xs, size_t nxs,
  const dds_time_t abstimeout, dds_time_t tnow
)
{
  /* set ret and break the loop when the attached condition is triggered or timedout */
  int ret = -1;
  dds_ws_cond_link * wsl;

  assert (waitset);
  while (true)
  {
    uint32_t n = 0;
    wsl = waitset->m_conds;
    while (wsl)
    {
      if (*((volatile uint32_t *) &wsl->m_cond->m_trigger))
      {
        if (n < nxs)
        {
          xs[n] = wsl->m_attach;
        }
        n++;
      }
      wsl = wsl->m_cond_next;
    }
    if (n > 0)
    {
      ret = n;
      waitset->timeout_counter = 0;
      break;
    }
    else if (abstimeout == DDS_NEVER)
    {
      if (waitset->trp_fd >= 0)
        break;
      waitset->timeout_counter++;
      os_condWait (&waitset->cv, &waitset->conds_lock);
    }
    else if (abstimeout <= tnow)
    {
      /* reset the counter */
      waitset->timeout_counter = 0;
      /* timeout is means the number of triggered conditions is 0, and
         isn't considered an error */
      ret = 0;
      break;
    }
    else
    {
      /* Do not assume the abstraction layer can handle timeouts >=
         2^31-1, given that we have at least some implementations
         that define os_timeSec as a 32-bit integer */
      if (waitset->trp_fd >= 0)
        break;
      waitset->timeout_counter++;
      dds_duration_t dt = abstimeout - tnow;
      os_time to;
      if ((sizeof (to.tv_sec) == 4) && ((dt / DDS_NSECS_IN_SEC) >= INT32_MAX))
      {
        to.tv_sec = INT32_MAX;
        to.tv_nsec = DDS_NSECS_IN_SEC - 1;
      }
      else
      {
        to.tv_sec = (os_timeSec) (dt / DDS_NSECS_IN_SEC);
        to.tv_nsec = (uint32_t) (dt % DDS_NSECS_IN_SEC);
      }
      os_condTimedWait (&waitset->cv, &waitset->conds_lock, &to);
      tnow = dds_time ();
    }
  }

  if (waitset->trp_fd >= 0 && ret >= 0)
  {
    waitset->trp_hdr.status = ret;
    if (write(waitset->trp_fd, &waitset->trp_hdr, sizeof(waitset->trp_hdr) + ret * sizeof(waitset->trp_xs[0])) != sizeof(waitset->trp_hdr) + ret * sizeof(waitset->trp_xs[0]))
      abort();
    waitset->trp_fd = -1;
  }
  return ret;
}

static int dds_waitset_wait_impl
(
 dds_waitset *waitset, dds_attach_t *xs, size_t nxs,
 const dds_time_t abstimeout, dds_time_t tnow,
 int trp_fd
 )
{
  int ret;

  assert(waitset);
  assert(nxs <= DDS_WAITSET_MAX);
  assert(trp_fd == -1 || xs == waitset->trp_xs);
  os_mutexLock (&waitset->conds_lock);
  assert(waitset->trp_fd == -1);
  waitset->trp_fd = trp_fd;
  waitset->trp_nxs = nxs;
  waitset->trp_abstimeout = abstimeout;
  ret = dds_waitset_wait_impl_locked(waitset, xs, nxs, abstimeout, tnow);
  os_mutexUnlock (&waitset->conds_lock);
  return ret;
}

static bool dds_waitset_add_condition (dds_waitset * ws, dds_condition *cond, dds_attach_t x)
{
  return (cond->m_kind & DDS_TYPE_COND_READ) ?
    dds_rhc_add_waitset ((dds_readcond*) cond, ws, x) :
    dds_condition_add_waitset (cond, ws, x);
}

static int dds_waitset_remove_condition (dds_waitset * ws, dds_condition * cond)
{
  return (cond->m_kind & DDS_TYPE_COND_READ) ?
    dds_rhc_remove_waitset ((dds_readcond*) cond, ws) :
    dds_condition_remove_waitset (cond, ws);
}

bool dds_waitset_add_condition_locked (dds_waitset * ws, dds_condition * cond, dds_attach_t x)
{
  dds_ws_cond_link * wsl = NULL;

  if (! cond_is_attached_locked (cond, ws))
  {
    wsl = dds_alloc (sizeof (*wsl));
    wsl->m_attach = x;
    wsl->m_waitset = ws;
    wsl->m_cond = cond;
    wsl->m_ws_next = cond->m_waitsets;
    wsl->m_cond_next = ws->m_conds;
    cond->m_waitsets = wsl;
    ws->m_conds = wsl;
    ws->m_nconds++;
  }
  return (wsl != NULL);
}

int dds_waitset_remove_condition_locked (dds_waitset * ws, dds_condition * cond)
{
  dds_ws_cond_link * iter;
  dds_ws_cond_link * prev;

  if (!cond_is_attached_locked (cond, ws) || (ws->m_nconds == 0))
  {
    return DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WAITSET, DDS_ERR_M3);
  }

  iter = cond->m_waitsets;
  prev = NULL;
  while (iter)
  {
    if (iter->m_waitset == ws)
    {
      if (prev)
      {
        prev->m_ws_next = iter->m_ws_next;
      }
      else
      {
        cond->m_waitsets = iter->m_ws_next;
      }
      break;
    }
    prev = iter;
    iter = iter->m_ws_next;
  }
  assert (iter);
  iter = ws->m_conds;
  prev = NULL;
  while (iter)
  {
    if (iter->m_cond == cond)
    {
      if (prev)
      {
        prev->m_cond_next = iter->m_cond_next;
      }
      else
      {
        ws->m_conds = iter->m_cond_next;
      }
      break;
    }
    prev = iter;
    iter = iter->m_cond_next;
  }
  assert (iter);
  ws->m_nconds--;

  dds_free (iter);
  return DDS_RETCODE_OK;
}

dds_waitset_t dds_waitset_create (void)
{
  dds_waitset * waitset = dds_alloc (sizeof (*waitset));
  os_mutexInit (&waitset->conds_lock, NULL);
  os_condInit (&waitset->cv, &waitset->conds_lock, NULL);
  waitset->trp_fd = -1;
  waitset->trp_hdr.code = 5;
  return waitset;
}

int dds_waitset_delete (dds_waitset_t ws)
{
  dds_condition * cond = NULL;
  int ret = DDS_RETCODE_OK;

  assert (ws);

  do
  {
    os_mutexLock (&gv.attach_lock);
    os_mutexLock (&ws->conds_lock);
    if (ws->timeout_counter > 0 || ws->trp_fd >= 0)
    {
      ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WAITSET, DDS_ERR_M5);
    }
    else
    {
      cond = ws->m_conds ? ws->m_conds->m_cond : NULL;
      if (cond)
      {
        ret = dds_waitset_remove_condition (ws, cond);
      }
    }
    os_mutexUnlock (&ws->conds_lock);
    os_mutexUnlock (&gv.attach_lock);
  }
  while (cond && (ret == DDS_RETCODE_OK));
  if (ret == DDS_RETCODE_OK)
  {
    os_condDestroy (&ws->cv);
    os_mutexDestroy (&ws->conds_lock);
    dds_free (ws);
  }
  return ret;
}

void dds_waitset_get_conditions (dds_waitset_t ws, dds_condition_seq * seq)
{
  assert (ws);
  assert (seq);
  dds_ws_cond_link * iter;
  unsigned i = 0;

  os_mutexLock (&ws->conds_lock);
  seq->_length = ws->m_nconds;
  seq->_release = (ws->m_nconds > 0);
  seq->_buffer = ws->m_nconds ? dds_alloc (sizeof (dds_condition*) * seq->_length) : NULL;

  assert ((ws->m_nconds == 0) == (ws->m_conds == NULL));
  iter = ws->m_conds;
  while (iter)
  {
    seq->_buffer[i++] = iter->m_cond;
    iter = iter->m_cond_next;
  }
  os_mutexUnlock (&ws->conds_lock);
}

int dds_waitset_attach (dds_waitset_t ws, dds_condition_t cond, dds_attach_t x)
{
  bool ret;

  assert (ws);
  assert (cond);

    /* Global attach/detach lock prevents attach and detach from running
     in parallel and causing trouble */
  os_mutexLock (&gv.attach_lock);
  TRACE (("attach_condition (waitset %p, cond %p)", (void*) ws, (void*) cond));

  /* First add waitset to condition: this has no effect other than
     cause an (apparently) spurious wakeup of the waitset's condition
     variable if the condition happens to be triggered between adding
     the waitset to the condition and adding the condition to the
     waitset */

  os_mutexLock (&ws->conds_lock);
  ret = dds_waitset_add_condition (ws, cond, x);
  os_mutexUnlock (&ws->conds_lock);

  if (! ret)
  {
    TRACE ((" - already attached\n"));
    goto done;
  }

  /* The number of attached conditions never changes without
     attach_lock being held. So the lock analyzers should all be
     happy about these accesses. */

  if (cond->m_trigger)
  {
    TRACE ((" - triggering"));
    os_mutexLock (cond->m_lock);
    dds_cond_signal_waitsets_locked (cond);
    os_mutexUnlock (cond->m_lock);
  }

done:

  TRACE (("\n"));
  os_mutexUnlock (&gv.attach_lock);
  return DDS_RETCODE_OK;
}

int dds_waitset_detach (dds_waitset_t ws, dds_condition_t cond)
{
  int ret;

  assert (ws);
  assert (cond);

  /* Reverse order of attach */

  os_mutexLock (&gv.attach_lock);
  TRACE (("dds_waitset_detach (waitset %p, cond %p)", (void*) ws, (void*) cond));

  os_mutexLock (&ws->conds_lock);
  ret = dds_waitset_remove_condition (ws, cond);
  os_mutexUnlock (&ws->conds_lock);

  if (ret != DDS_RETCODE_OK)
  {
    TRACE ((" - not attached\n"));
    ret = DDS_ERRNO (DDS_RETCODE_PRECONDITION_NOT_MET, DDS_MOD_WAITSET, DDS_ERR_M4);
  }

  os_mutexUnlock (&gv.attach_lock);
  return ret;
}

int dds_waitset_wait_until (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t abstimeout)
{
  return dds_waitset_wait_impl (ws, xs, nxs, abstimeout, dds_time (), -1);
}

int dds_waitset_wait (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t reltimeout)
{
  dds_time_t tnow = dds_time ();
  dds_time_t abstimeout = (DDS_INFINITY - reltimeout <= tnow) ? DDS_NEVER : (tnow + reltimeout);
  return dds_waitset_wait_impl (ws, xs, nxs, abstimeout, tnow, -1);
}

int dds_waitset_wait_async (dds_waitset_t ws, size_t nxs, dds_time_t reltimeout, int trp_fd)
{
  dds_time_t tnow = dds_time ();
  dds_time_t abstimeout = (DDS_INFINITY - reltimeout <= tnow) ? DDS_NEVER : (tnow + reltimeout);
  assert(nxs <= DDS_WAITSET_MAX);
  return dds_waitset_wait_impl (ws, ws->trp_xs, nxs, abstimeout, tnow, trp_fd);
}

void dds_waitset_signal (dds_waitset * ws)
{
  os_mutexLock (&ws->conds_lock);
  ws->triggered++;
  os_condBroadcast (&ws->cv);
  if (ws->trp_fd >= 0)
    dds_waitset_wait_impl_locked(ws, ws->trp_xs, ws->trp_nxs, ws->trp_abstimeout, dds_time());
  os_mutexUnlock (&ws->conds_lock);
}
