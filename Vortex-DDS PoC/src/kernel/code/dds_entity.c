#include <assert.h>
#include <string.h>
#include "q_entity.h"
#include "q_thread.h"
#include "q_xmsg.h"
#include "q_osplser.h"
#include "dds_entity.h"
#include "dds_status.h"
#include "dds_statuscond.h"
#include "dds_rhc.h"
#include "dds_tkmap.h"
#include "dds_topic.h"
#include "dds_domain.h"
#include "dds_participant.h"
#include "os_report.h"

void dds_entity_add_ref (dds_entity * e)
{
  assert (e);
  os_mutexLock (&e->m_mutex);
  e->m_refc++;
  os_mutexUnlock (&e->m_mutex);
}

bool dds_entity_callback_lock (dds_entity * e)
{
  bool ok;
  os_mutexLock (&e->m_mutex);
  e->m_flags |= DDS_ENTITY_IN_USE;
  ok = (e->m_flags & DDS_ENTITY_DELETED) == 0;
  if (! ok && (e->m_status_enable != 0))
  {
    dds_log_warn ("dds_entity_delete - Entity deleted with active listeners");
  }
  return ok;
}

void dds_entity_callback_unlock (dds_entity * e)
{
  e->m_flags &= ~DDS_ENTITY_IN_USE;
  if (e->m_flags & DDS_ENTITY_DELETED)
  {
    os_condBroadcast (&e->m_cond);
  }
  os_mutexUnlock (&e->m_mutex);
}

void dds_entity_delete_signal (dds_entity_t e)
{
  /* Signal that clean up complete */

  os_mutexLock (&e->m_mutex);
  e->m_flags |= DDS_ENTITY_DDSI_DELETED;
  os_condSignal (&e->m_cond);
  os_mutexUnlock (&e->m_mutex);
}

static void dds_entity_delete_wait (dds_entity_t e, struct thread_state1 * const thr)
{
  /* Wait for DDSI clean up to complete */

  os_mutexLock (&e->m_mutex);
  if ((e->m_flags & DDS_ENTITY_DDSI_DELETED) == 0)
  {
    thread_state_asleep (thr);
    do
    {
      os_condWait (&e->m_cond, &e->m_mutex);
    }
    while ((e->m_flags & DDS_ENTITY_DDSI_DELETED) == 0);
    thread_state_awake (thr);
  }
  os_mutexUnlock (&e->m_mutex);
}

void dds_entity_delete_impl (dds_entity_t e, bool child, bool recurse)
{
  dds_entity_t iter;
  dds_entity_t *iterp;
  dds_entity_t prev = NULL;

  os_mutexLock (&e->m_mutex);

  if (--e->m_refc != 0)
  {
    os_mutexUnlock (&e->m_mutex);
    return;
  }

  e->m_flags |= DDS_ENTITY_DELETED;
  e->m_status_enable = 0;

  /* If has active callback, wait until complete */

  if (e->m_flags & DDS_ENTITY_IN_USE)
  {
    do
    {
      os_condWait (&e->m_cond, &e->m_mutex);
    }
    while ((e->m_flags & DDS_ENTITY_IN_USE) != 0);
  }
  os_mutexUnlock (&e->m_mutex);

  /* Remove from parent */

  if (!child && e->m_parent)
  {
    os_mutexLock (&e->m_parent->m_mutex);
    iter = e->m_parent->m_children;
    while (iter)
    {
      if (iter == e)
      {
        /* Remove topic from participant extent */

        if (prev)
        {
          prev->m_next = e->m_next;
        }
        else
        {
          e->m_parent->m_children = e->m_next;
        }
        break;
      }
      prev = iter;
      iter = iter->m_next;
    }
    os_mutexUnlock (&e->m_parent->m_mutex);
  }

  /* Recursively delete children */
  if (recurse)
  {
    iterp = &e->m_children;
    while (*iterp != NULL)
    {
      prev = (*iterp);
      if (prev->m_kind == DDS_TYPE_TOPIC)
      {
         /* Skip the topic element */
         iterp = &(prev->m_next);
      }
      else
      {
        /* Remove entity from list */
        *iterp = prev->m_next;
        /* clear m_parent,
           otherwise prev may try to delete this object later */
        os_mutexLock (&prev->m_mutex);
        prev->m_parent = NULL;
        os_mutexUnlock (&prev->m_mutex);
        dds_entity_delete_impl (prev, true, true);
      }
    }
    while (e->m_children)
    {
      prev = e->m_children;
      assert (prev->m_kind == DDS_TYPE_TOPIC);
      e->m_children = prev->m_next;
      os_mutexLock (&prev->m_mutex);
      prev->m_parent = NULL;
      os_mutexUnlock (&prev->m_mutex);
      dds_entity_delete_impl (prev, true, true);
    }
  }
  else
  {
    for (iter = e->m_children; iter != NULL; iter = iter->m_next)
    {
      os_mutexLock (&iter->m_mutex);
      iter->m_parent = NULL;
      os_mutexUnlock (&iter->m_mutex);
    }

  }
  /* Delete from DDSI */

  if (e->m_kind & DDS_IS_MAPPED)
  {
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);

    if (asleep)
    {
      thread_state_awake (thr);
    }
    switch (e->m_kind)
    {
      case DDS_TYPE_PARTICIPANT:
      {
        dds_domain_free (e->m_domain);
        dds_participant_remove (e);
        break;
      }
      case DDS_TYPE_READER:
      {
        dds_reader * rd = (dds_reader*) e;
        delete_reader (&e->m_guid);
        dds_entity_delete_wait (e, thr);
        dds_entity_delete_impl ((dds_entity*) rd->m_topic, false, recurse);
        dds_free (rd->m_loan);
        break;
      }
      case DDS_TYPE_WRITER:
      {
        dds_writer * wr = (dds_writer*) e;
        nn_xpack_send (wr->m_xp);
        delete_writer (&e->m_guid);
        dds_entity_delete_wait (e, thr);
        nn_xpack_free (wr->m_xp);
        os_mutexDestroy (&wr->m_call_lock);
        dds_entity_delete_impl ((dds_entity*) wr->m_topic, false, recurse);
        break;
      }
      default: assert (0); break;
    }
    if (asleep)
    {
      thread_state_asleep (thr);
    }
  }

  /* Clean up */

  if (e->m_kind == DDS_TYPE_TOPIC)
  {
    dds_topic_free (e->m_domainid, ((dds_topic*) e)->m_stopic);
  }
  dds_qos_delete (e->m_qos);
  if (e->m_scond)
  {
    dds_condition_delete (e->m_scond);
  }
  os_condDestroy (&e->m_cond);
  os_mutexDestroy (&e->m_mutex);
  os_atomic_dec32 (&dds_global.m_entity_count[e->m_kind & DDS_TYPE_INDEX_MASK]);
  dds_free (e);
}

void dds_entity_delete (dds_entity_t e)
{
  if (e)
  {
    dds_entity_delete_impl (e, false, true);
  }
}

void dds_entity_init
(
  dds_entity * e, dds_entity * parent,
  dds_entity_kind_t kind, dds_qos_t * qos
)
{
  assert (e);
  assert (parent || kind == DDS_TYPE_PARTICIPANT);

  e->m_refc = 1;
  e->m_parent = parent;
  e->m_kind = kind;
  e->m_qos = qos;

  /* set the status enable based on kind */

  e->m_status_enable = dds_status_masks [kind & DDS_TYPE_INDEX_MASK];

  os_mutexInit (&e->m_mutex, NULL);
  os_condInit (&e->m_cond, &e->m_mutex, NULL);

  /* alloc status condition */

  if (kind & DDS_HAS_SCOND)
  {
    e->m_scond = dds_statuscond_create ();
    e->m_scond->m_lock = &e->m_mutex;
  }

  if (parent)
  {
    e->m_domain = parent->m_domain;
    e->m_domainid = parent->m_domainid;
    e->m_pp = parent->m_pp;
    e->m_next = parent->m_children;
    parent->m_children = e;
  }
  else
  {
    e->m_pp = (dds_participant*) e;
  }
  os_atomic_inc32 (&dds_global.m_entity_count[kind & DDS_TYPE_INDEX_MASK]);
}
