#include <assert.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_config.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_status.h"
#include "kernel/dds_init.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"

/* List of created participants */

static dds_entity * dds_pp_head = NULL;

int dds_participant_create
(
  dds_entity_t * e,
  const dds_domainid_t domain,
  const dds_qos_t * qos,
  const dds_participantlistener_t * listener
)
{
  int ret;
  nn_guid_t guid;
  dds_participant * pp;
  nn_plist_t plist;
  dds_qos_t * new_qos = NULL;
  struct thread_state1 * thr;
  bool asleep;

  nn_plist_init_empty (&plist);

  /* Check domain id */

  ret = dds_init_impl (domain);
  if (ret != DDS_RETCODE_OK)
  {
    goto fail;
  }

  /* Validate qos */

  if (qos)
  {
    ret = dds_qos_validate (DDS_TYPE_PARTICIPANT, qos);
    if (ret != DDS_RETCODE_OK)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
    dds_qos_merge (&plist.qos, new_qos);
  }

  thr = lookup_thread_state ();
  asleep = !vtime_awake_p (thr->vtime);
  if (asleep)
  {
    thread_state_awake (thr);
  }
  ret = new_participant (&guid, 0, &plist);
  if (asleep)
  {
    thread_state_asleep (thr);
  }

  if (ret != 0)
  {
    ret = DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_KERNEL, DDS_ERR_M4);
    goto fail;
  }

  pp = dds_alloc (sizeof (*pp));
  dds_entity_init (&pp->m_entity, NULL, DDS_TYPE_PARTICIPANT, new_qos);
  pp->m_entity.m_guid = guid;
  *e = &pp->m_entity;
  pp->m_entity.m_domain = dds_domain_create (config.domainId);
  pp->m_entity.m_domainid = config.domainId;
  
  if (listener)
  {
    pp->m_listener = *listener;
  }

  /* Add participant to extent */

  os_mutexLock (&dds_global.m_mutex);
  pp->m_entity.m_next = dds_pp_head;
  dds_pp_head = &pp->m_entity;
  os_mutexUnlock (&dds_global.m_mutex);

fail:

  nn_plist_fini (&plist);
  
  return ret;
}

dds_entity_t dds_participant_lookup (dds_domainid_t domain_id)
{
  dds_entity_t pp = NULL;
 
  os_mutexLock (&dds_global.m_mutex);
  pp = dds_pp_head;
  while (pp && (pp->m_domainid != domain_id))
  {
    pp = pp->m_next;
  }
  os_mutexUnlock (&dds_global.m_mutex);
  return pp;
}

void dds_participant_remove (dds_entity_t e)
{
  dds_entity * iter;
  dds_entity * prev = NULL;
  
  assert (e);
  assert (e->m_kind == DDS_TYPE_PARTICIPANT);
  
  os_mutexLock (&dds_global.m_mutex);
  iter = dds_pp_head;
  while (iter)
  {
    if (iter == e)
    {
      if (prev)
      {
        prev->m_next = iter->m_next;
      }
      else
      {
        dds_pp_head = iter->m_next;
      }
      break;
    }
    prev = iter;
    iter = iter->m_next;
  }
  os_mutexUnlock (&dds_global.m_mutex);

  assert (iter);
}

dds_domainid_t dds_participant_get_domain_id (dds_entity_t pp)
{
  return pp->m_domainid;
}

dds_entity_t dds_participant_get (dds_entity_t entity)
{
  return &entity->m_pp->m_entity;
}
