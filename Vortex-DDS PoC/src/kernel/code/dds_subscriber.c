#include <assert.h>
#include "dds_listener.h"
#include "dds_qos.h"
#include "dds_status.h"
#include "q_entity.h"

int dds_subscriber_create
(
  dds_entity_t pp,
  dds_entity_t * subscriber,
  const dds_qos_t * qos,
  const dds_subscriberlistener_t * listener
)
{
  int ret = DDS_RETCODE_OK;
  dds_subscriber * sub;
  dds_qos_t * new_qos = NULL;
  dds_participantlistener_t l;

  assert (pp);
  assert (pp->m_kind == DDS_TYPE_PARTICIPANT);
  assert (subscriber);

  os_mutexLock (&pp->m_mutex);

  /* Validate qos */

  if (qos)
  {
    ret = dds_qos_validate (DDS_TYPE_SUBSCRIBER, qos);
    if (ret != 0)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create subscriber */

  sub = dds_alloc (sizeof (*sub));
  dds_entity_init (&sub->m_entity, pp, DDS_TYPE_SUBSCRIBER, new_qos);
  *subscriber = &sub->m_entity;

  /* Merge listener with those from parent */

  if (listener)
  {
    sub->m_listener = *listener;
  }
  dds_listener_get_unl (pp, &l);
  dds_listener_merge (&sub->m_listener, &l.subscriberlistener, DDS_TYPE_SUBSCRIBER);

fail:

  os_mutexUnlock (&pp->m_mutex);
  return ret;
}
