#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "ddsi/q_entity.h"

#define DDS_PUBLISHER_STATUS_MASK   0

static dds_return_t dds_publisher_instance_hdl(dds_entity_t e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, 0);
}

static dds_return_t dds_publisher_qos_validate (const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = DDS_ERRNO (DDS_RETCODE_INCONSISTENT_POLICY, DDS_MOD_KERNEL, 0);
    bool consistent = true;
    assert(qos);
    /* Check consistency. */
    consistent &= (qos->present & QP_GROUP_DATA) ? validate_octetseq (&qos->group_data) : true;
    consistent &= (qos->present & QP_PRESENTATION) ? (validate_presentation_qospolicy (&qos->presentation) == 0) : true;
    consistent &= (qos->present & QP_PARTITION) ? validate_partition_qospolicy(&qos->partition) : true;
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

static dds_return_t dds_publisher_qos_set (dds_entity_t e, const dds_qos_t *qos, bool enabled)
{
    dds_return_t ret = dds_publisher_qos_validate(qos, enabled);
    if (ret == DDS_RETCODE_OK) {
        if (enabled) {
            /* TODO: CHAM-95: DDSI does not support changing QoS policies. */
            ret = (dds_return_t)(DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, DDS_ERR_M1));
        }
    }
    return ret;
}

dds_entity_t dds_create_publisher
(
  _In_ dds_entity_t pp,
  _In_opt_ const dds_qos_t * qos,
  _In_opt_ const dds_listener_t * listener
)
{
  dds_publisher * pub;
  dds_qos_t * new_qos = NULL;
  dds_return_t ret;


  /* Check participant */

  if ((pp == NULL) || (pp->m_kind != DDS_TYPE_PARTICIPANT))
  {
      /* TODO: Temporary implementation
       * It should actually return a handle indicating a BAD_PARAMETER, but as long
       * as there is no implementation of an handle server NULL is retruned instead.
       * ret = DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WRITER, DDS_ERR_M3);
        return (dds_entity_t)ret;
       */
      return NULL;
  }

  os_mutexLock (&pp->m_mutex);

  /* Validate qos */

  if (qos)
  {
    ret = dds_publisher_qos_validate(qos, false);
    if (ret != DDS_RETCODE_OK)
    {
      goto fail;
    }
    new_qos = dds_qos_create ();
    dds_qos_copy (new_qos, qos);
  }

  /* Create publisher */

  pub = dds_alloc (sizeof (*pub));
  dds_entity_init (&pub->m_entity, pp, DDS_TYPE_PUBLISHER, new_qos, listener, DDS_PUBLISHER_STATUS_MASK);
  pub->m_entity.m_deriver.set_qos = dds_publisher_qos_set;
  pub->m_entity.m_deriver.get_instance_hdl = dds_publisher_instance_hdl;
  os_mutexUnlock (&pp->m_mutex);
  return &pub->m_entity;

fail:

  os_mutexUnlock (&pp->m_mutex);
  /* TODO: return ret when handles have been implemented correctly */
  return NULL;
}


dds_return_t dds_suspend
(
  _In_ dds_entity_t pub
)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(pub);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}


dds_return_t dds_resume
(
  _In_ dds_entity_t pub
)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(pub);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}


dds_return_t dds_wait_for_acks
(
  _In_ dds_entity_t pub_or_w,
  _In_ dds_duration_t timeout
)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(pub_or_w);
  OS_UNUSED_ARG(timeout);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}
