#include <assert.h>
#include <string.h>
#include "kernel/dds_listener.h"
#include "kernel/dds_qos.h"
#include "ddsi/q_entity.h"

#define DDS_PUBLISHER_STATUS_MASK   0

static dds_return_t dds_publisher_instance_hdl(dds_entity *e, dds_instance_handle_t *i)
{
    assert(e);
    assert(i);
    /* TODO: Get/generate proper handle. */
    return DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_KERNEL, 0);
}

static dds_return_t dds_publisher_qos_validate (_In_ const dds_qos_t *qos, _In_ bool enabled)
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

static dds_return_t dds_publisher_qos_set (dds_entity *e, const dds_qos_t *qos, bool enabled)
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

_Pre_satisfies_((participant & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT)
_Must_inspect_result_ dds_entity_t
dds_create_publisher(
        _In_     dds_entity_t participant,
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener)
{
  dds_entity * par;
  dds_publisher * pub;
  dds_entity_t hdl;
  dds_qos_t * new_qos = NULL;
  dds_return_t ret;
  int32_t errnr;

  errnr = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, &par);
  if (errnr != DDS_RETCODE_OK) {
      return DDS_ERRNO(errnr, DDS_MOD_KERNEL, DDS_ERR_M2);
  }

  /* Validate qos */
  if (qos) {
    ret = dds_publisher_qos_validate(qos, false);
    if (ret != DDS_RETCODE_OK) {
      dds_entity_unlock(par);
      return ret;
    }
    new_qos = dds_qos_create ();
    /* Only returns failure when one of the qos args is NULL, which
     * is not the case here. */
    (void)dds_qos_copy(new_qos, qos);
  }

  /* Create publisher */
  pub = dds_alloc (sizeof (*pub));
  hdl = dds_entity_init (&pub->m_entity, par, DDS_KIND_PUBLISHER, new_qos, listener, DDS_PUBLISHER_STATUS_MASK);
  pub->m_entity.m_deriver.set_qos = dds_publisher_qos_set;
  pub->m_entity.m_deriver.get_instance_hdl = dds_publisher_instance_hdl;
  dds_entity_unlock(par);

  return hdl;
}


_Pre_satisfies_((publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PUBLISHER)
DDS_EXPORT dds_return_t
dds_suspend(
        _In_ dds_entity_t publisher)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(publisher);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}


_Pre_satisfies_((publisher & DDS_ENTITY_KIND_MASK) == DDS_KIND_PUBLISHER)
dds_return_t
dds_resume(
        _In_ dds_entity_t publisher)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(publisher);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}


_Pre_satisfies_(((publisher_or_writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER   ) ||\
                ((publisher_or_writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_PUBLISHER) )
dds_return_t
dds_wait_for_acks(
        _In_ dds_entity_t publisher_or_writer,
        _In_ dds_duration_t timeout)
{
  dds_return_t ret;

  /* TODO: Currently unsupported. */
  OS_UNUSED_ARG(publisher_or_writer);
  OS_UNUSED_ARG(timeout);

  ret = DDS_ERRNO (DDS_RETCODE_UNSUPPORTED, DDS_MOD_WRITER, 0);
  return ret;
}

dds_return_t
dds_publisher_begin_coherent
(
    _In_ dds_entity_t e
)
{
    return DDS_RETCODE_UNSUPPORTED;
}

dds_return_t
dds_publisher_end_coherent
(
    _In_ dds_entity_t e
)
{
    return DDS_RETCODE_UNSUPPORTED;
}

