#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_write.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "ddsi/ddsi_ser.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_writedispose(
        dds_entity_t writer,
        const void *data)
{
  return dds_instance_writedispose_ts(writer, data, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_dispose(
        dds_entity_t writer,
        const void *data)
{
  return dds_instance_dispose_ts(writer, data, dds_time());
}

static struct tkmap_instance * dds_instance_find
(
  const dds_topic * topic,
  const void * data,
  const bool create
)
{
  serdata_t sd = serialize_key (gv.serpool, topic->m_stopic, data);
  struct tkmap_instance * inst = dds_tkmap_find (topic, sd, false, create);
  ddsi_serdata_unref (sd);
  return inst;
}

static void dds_instance_remove (const dds_topic * topic,  const void * data, dds_instance_handle_t handle)
{
  struct tkmap_instance * inst;

  if (handle != DDS_HANDLE_NIL)
  {
    inst = dds_tkmap_find_by_id (gv.m_tkmap, handle);
  }
  else
  {
    assert (data);
    inst = dds_instance_find (topic, data, false);
  }
  if (inst)
  {
    struct thread_state1 * const thr = lookup_thread_state ();
    const bool asleep = !vtime_awake_p (thr->vtime);
    if (asleep)
    {
     thread_state_awake (thr);
    }

    dds_tkmap_instance_unref (inst);
    if (asleep)
    {
     thread_state_asleep (thr);
    }
  }
}

static const dds_topic * dds_instance_info (dds_entity *e)
{
  const dds_topic * topic;

  assert (e);
  assert ((dds_entity_kind(e->m_hdl) == DDS_KIND_READER) || (dds_entity_kind(e->m_hdl) == DDS_KIND_WRITER));

  if (dds_entity_kind(e->m_hdl) == DDS_KIND_READER)
    topic = ((dds_reader*) e)->m_topic;
  else
    topic = ((dds_writer*) e)->m_topic;
  return topic;
}

static const dds_topic * dds_instance_info_by_hdl (dds_entity_t e)
{
    const dds_topic * topic = NULL;
    int ret = DDS_RETCODE_OK;
    dds_entity *w_or_r;
    ret = dds_entity_lock(e, DDS_KIND_WRITER, &w_or_r);
    if (ret == DDS_RETCODE_ILLEGAL_OPERATION) {
        ret = dds_entity_lock(e, DDS_KIND_READER, &w_or_r);
    }
    if (ret != DDS_RETCODE_OK) {
        topic = dds_instance_info(w_or_r);
        dds_entity_unlock(w_or_r);
    }
    return topic;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_instance_handle_t
dds_instance_register(
        dds_entity_t writer,
        const void *data)
{
  struct tkmap_instance * inst;
  dds_entity *wr;
  int ret = DDS_RETCODE_OK;

  assert (data);

  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO (ret, DDS_MOD_INST, DDS_ERR_M1);
  }
  inst = dds_instance_find (((dds_writer*) wr)->m_topic, data, true);
  dds_entity_unlock(wr);
  return inst->m_iid;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_unregister(
        dds_entity_t writer,
        const void *data,
        dds_instance_handle_t handle)
{
  return dds_instance_unregister_ts (writer, data, handle, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_unregister_ts(
        dds_entity_t writer,
        const void *data,
        dds_instance_handle_t handle,
        dds_time_t timestamp)
{
  int ret = DDS_RETCODE_OK;
  bool autodispose = true;
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  void * sample = (void*) data;
  const dds_topic * topic = NULL;
  dds_entity *wr;

  assert (data != NULL || handle != DDS_HANDLE_NIL);

  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO (ret, DDS_MOD_INST, DDS_ERR_M1);
  }

  /* If have handle but not sample, get sample from handle */

  if (sample == NULL)
  {
    struct tkmap * map = gv.m_tkmap;
    topic = dds_instance_info (wr);
    sample = dds_alloc (topic->m_descriptor->m_size);
    if (! dds_tkmap_get_key (map, handle, sample))
    {
      ret = DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_INST, DDS_ERR_M2);
    }
  }

  if (ret == DDS_RETCODE_OK)
  {
    if (wr->m_qos)
    {
      dds_qget_writer_data_lifecycle (wr->m_qos, &autodispose);
    }
    if (autodispose)
    {
      dds_instance_remove (((dds_writer*) wr)->m_topic, data, handle);
      action |= DDS_WR_DISPOSE_BIT;
    }

    ret = dds_write_impl ((dds_writer*)wr, sample, timestamp, action);
  }

  if (topic)
  {
    dds_sample_free (sample, topic->m_descriptor, DDS_FREE_ALL);
  }
  dds_entity_unlock(wr);
  return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_writedispose_ts(
        dds_entity_t writer,
        const void *data,
        dds_time_t timestamp)
{
  int ret;
  dds_entity *wr;
  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO (ret, DDS_MOD_INST, DDS_ERR_M1);
  }
  ret = dds_write_impl ((dds_writer*)wr, data, timestamp, DDS_WR_ACTION_WRITE_DISPOSE);
  dds_instance_remove (((dds_writer*)wr)->m_topic, data, DDS_HANDLE_NIL);
  dds_entity_unlock(wr);
  return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
int
dds_instance_dispose_ts(
        dds_entity_t writer,
        const void *data,
        dds_time_t timestamp)
{
  int ret;
  dds_entity *wr;
  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO (ret, DDS_MOD_INST, DDS_ERR_M1);
  }
  ret = dds_write_impl ((dds_writer*)wr, data, timestamp, DDS_WR_ACTION_DISPOSE);
  dds_instance_remove (((dds_writer*)wr)->m_topic, data, DDS_HANDLE_NIL);
  dds_entity_unlock(wr);
  return ret;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
dds_instance_handle_t
dds_instance_lookup(
        dds_entity_t entity,
        const void *data)
{
  dds_instance_handle_t ih;
  const dds_topic * topic;
  struct tkmap * map = gv.m_tkmap;
  serdata_t sd;

  assert (data);

  topic = dds_instance_info_by_hdl (entity);
  sd = serialize_key (gv.serpool, topic->m_stopic, data);
  ih = dds_tkmap_lookup (map, sd);
  ddsi_serdata_unref (sd);
  return ih;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
int
dds_instance_get_key(
        dds_entity_t entity,
        dds_instance_handle_t inst,
        void *data)
{
  const dds_topic * topic;
  struct tkmap * map = gv.m_tkmap;

  assert (data);

  topic = dds_instance_info_by_hdl (entity);
  memset (data, 0, topic->m_descriptor->m_size);
  return (dds_tkmap_get_key (map, inst, data)) ?
    DDS_RETCODE_OK : DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_INST, DDS_ERR_M1);
}
