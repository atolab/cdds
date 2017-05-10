#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "ddsi/q_entity.h"

#define DDS_PUBLISHER_STATUS_MASK   0

static dds_result_t dds_publisher_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, 0);
}

static dds_result_t dds_publisher_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_result_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_KERNEL, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= ((qos->present & QP_GROUP_DATA) && ! validate_octetseq (&qos->group_data));
    consistent &= ((qos->present & QP_PRESENTATION) && (validate_presentation_qospolicy (&qos->presentation) != 0));
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

int dds_publisher_create
(
  dds_entity_t pp,
  dds_entity_t * publisher,
  const dds_qos_t * qos,
  const dds_publisherlistener_t * listener
)
{
  int ret = DDS_RETCODE_OK;
  dds_publisher * pub;
  dds_qos_t * new_qos = NULL;
  dds_participantlistener_t l;

  assert (pp);
  assert (pp->m_kind == DDS_TYPE_PARTICIPANT);
  assert (publisher);

  /* Check participant */

  os_mutexLock (&pp->m_mutex);

  /* Validate qos */

  if (qos)
  {
    ret = (int)dds_publisher_qos_validate(qos, false);
    if (ret != 0)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create publisher */

  pub = dds_alloc (sizeof (*pub));
  dds_entity_init (&pub->m_entity, pp, DDS_TYPE_PUBLISHER, new_qos, NULL, DDS_PUBLISHER_STATUS_MASK);
  *publisher = &pub->m_entity;
  pub->m_entity.m_deriver.validate_qos = dds_publisher_qos_validate;
  pub->m_entity.m_deriver.get_instance_hdl = dds_publisher_instance_hdl;

  /* Merge listener functions with those from parent */

  if (listener)
  {
    pub->m_listener = *listener;
  }
  dds_listener_get_unl (pp, &l);
  dds_listener_merge (&pub->m_listener, &l.publisherlistener, DDS_TYPE_PUBLISHER);

fail:

  os_mutexUnlock (&pp->m_mutex);
  return ret;
}
