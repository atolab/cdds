#include <assert.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "kernel/dds_status.h"
#include "ddsi/q_entity.h"

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
    ret = dds_qos_validate (DDS_TYPE_PUBLISHER, qos);
    if (ret != 0)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create publisher */

  pub = dds_alloc (sizeof (*pub));
  dds_entity_init (&pub->m_entity, pp, DDS_TYPE_PUBLISHER, new_qos);
  *publisher = &pub->m_entity;

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
