#include <assert.h>
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_config.h"
#include "kernel/q_osplser.h"
#include "kernel/dds_init.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_domain.h"
#include "kernel/dds_participant.h"

#define DDS_PARTICIPANT_STATUS_MASK    0

/* List of created participants */

static dds_entity * dds_pp_head = NULL;

static void dds_participant_delete(dds_entity_t e, bool recurse)
{
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);

    assert(e);
    assert(thr);

    if (asleep) {
      thread_state_awake(thr);
    }

    dds_domain_free (e->m_domain);
    dds_participant_remove (e);

    if (asleep) {
      thread_state_asleep(thr);
    }
}

static dds_return_t dds_participant_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    *i = (dds_instance_handle_t)participant_instance_id(&e->m_guid);
    return DDS_RETCODE_OK;
}

static dds_return_t dds_participant_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_PPANT, 0);
    bool consistent;
    assert(qos);
    /* Check consistency. */
    consistent = ! ((qos->present & QP_USER_DATA) && ! validate_octetseq (&qos->user_data));
    if (consistent) {
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            ret = DDS_ERRNO (DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_PPANT, 0);
        } else {
            ret = DDS_RETCODE_OK;
        }
    }
    return ret;
}


static dds_return_t dds_participant_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_participant_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, DDS_ERR_M1));
        }
    }
    return ret;
}

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
    ret = (int)dds_participant_qos_validate (qos, false);
    if (ret != DDS_RETCODE_OK)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
    dds_qos_merge (&plist.qos, new_qos);
  }
  else
  {
    /* Use default qos. */
    new_qos = dds_qos_create ();
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
  dds_entity_init (&pp->m_entity, NULL, DDS_TYPE_PARTICIPANT, new_qos, NULL, DDS_PARTICIPANT_STATUS_MASK);
  pp->m_entity.m_guid = guid;
  *e = &pp->m_entity;
  pp->m_entity.m_domain = dds_domain_create (config.domainId);
  pp->m_entity.m_domainid = config.domainId;
  pp->m_entity.m_deriver.delete = dds_participant_delete;
  pp->m_entity.m_deriver.set_qos = dds_participant_qos_set;
  pp->m_entity.m_deriver.get_instance_hdl = dds_participant_instance_hdl;

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
