#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "ddsi/q_entity.h"

#define DDS_SUBSCRIBER_STATUS_MASK                               \
                        DDS_DATA_ON_READERS_STATUS

static dds_return_t dds_subscriber_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, 0);
}

static dds_return_t dds_subscriber_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_KERNEL, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= (qos->present & QP_GROUP_DATA) ? validate_octetseq (&qos->group_data) : true;
    consistent &= (qos->present & QP_PRESENTATION) ? (validate_presentation_qospolicy (&qos->presentation) == 0) : true;
    if (consistent) {
        if (enabled) {
            /* TODO: Improve/check immutable check. */
            ret = DDS_ERRNO (DDS_RETCODE_IMMUTABLE_POLICY, DDS_MOD_KERNEL, 0);
        } else {
            ret = DDS_RETCODE_OK;
        }
    }
    return ret;
}

static dds_return_t dds_subscriber_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_subscriber_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, DDS_ERR_M1));
        }
    }
    return ret;
}

static dds_return_t dds_subscriber_status_validate (uint32_t mask)
{
    return (mask & ~(DDS_SUBSCRIBER_STATUS_MASK)) ?
                     DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_KERNEL, 0) :
                     DDS_RETCODE_OK;
}

/*
  Set boolean on readers that indicates state of DATA_ON_READERS
  status on parent subscriber
*/
static dds_return_t dds_subscriber_status_propagate (dds_entity_t sub, uint32_t mask, bool set)
{
    if (mask & DDS_DATA_ON_READERS_STATUS) {
        dds_entity_t iter = sub->m_children;
        while (iter) {
            os_mutexLock (&iter->m_mutex);
            ((dds_reader*) iter)->m_data_on_readers = set;
            os_mutexUnlock (&iter->m_mutex);
            iter = iter->m_next;
        }
    }
    return DDS_RETCODE_OK;
}

int dds_subscriber_create
(
  dds_entity_t pp,
  dds_entity_t * subscriber,
  const dds_qos_t * qos,
  const dds_listener_t * listener
)
{
  int ret = DDS_RETCODE_OK;
  dds_subscriber * sub;
  dds_qos_t * new_qos = NULL;

  assert (pp);
  assert (pp->m_kind == DDS_TYPE_PARTICIPANT);
  assert (subscriber);

  os_mutexLock (&pp->m_mutex);

  /* Validate qos */

  if (qos)
  {
    ret = (int)dds_subscriber_qos_validate (qos, false);
    if (ret != 0)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create subscriber */

  sub = dds_alloc (sizeof (*sub));
  dds_entity_init (&sub->m_entity, pp, DDS_TYPE_SUBSCRIBER, new_qos, listener, DDS_SUBSCRIBER_STATUS_MASK);
  *subscriber = &sub->m_entity;
  sub->m_entity.m_deriver.set_qos = dds_subscriber_qos_set;
  sub->m_entity.m_deriver.validate_status = dds_subscriber_status_validate;
  sub->m_entity.m_deriver.propagate_status = dds_subscriber_status_propagate;
  sub->m_entity.m_deriver.get_instance_hdl = dds_subscriber_instance_hdl;

fail:

  os_mutexUnlock (&pp->m_mutex);
  return ret;
}
