#include <assert.h>
#include <string.h>
#include "dds.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_write.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_err.h"
#include "ddsi/ddsi_ser.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"

/* TODO: dds_retcode_t */

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_writedispose(
       _In_ dds_entity_t writer,
       _In_ const void *data)
{
    return dds_writedispose_ts(writer, data, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose(
       _In_ dds_entity_t writer,
       _In_ const void *data)
{
    return dds_dispose_ts(writer, data, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ih(
       _In_ dds_entity_t writer,
       _In_ dds_instance_handle_t handle)
{
    return dds_dispose_ih_ts(writer, handle, dds_time());
}

static struct tkmap_instance*
dds_instance_find(
        _In_ const dds_topic *topic,
        _In_ const void *data,
        _In_ const bool create)
{
    serdata_t sd = serialize_key (gv.serpool, topic->m_stopic, data);
    struct tkmap_instance * inst = dds_tkmap_find (topic, sd, false, create);
    ddsi_serdata_unref (sd);
    return inst;
}

static void
dds_instance_remove(
        _In_     const dds_topic *topic,
        _In_opt_ const void *data,
        _In_     dds_instance_handle_t handle)
{
    struct tkmap_instance * inst;

    if (handle != DDS_HANDLE_NIL) {
        inst = dds_tkmap_find_by_id (gv.m_tkmap, handle);
    } else {
        assert (data);
        inst = dds_instance_find (topic, data, false);
    }

    if (inst) {
        struct thread_state1 * const thr = lookup_thread_state();
        const bool asleep = thr ? !vtime_awake_p(thr->vtime) : false;
        if (asleep) {
            thread_state_awake(thr);
        }
        dds_tkmap_instance_unref (inst);
        if (asleep) {
            thread_state_asleep(thr);
        }
    }
}

static const dds_topic*
dds_instance_info(
        _In_ dds_entity *e)
{
    const dds_topic *topic = NULL;

    assert (e);
    assert ((dds_entity_kind(e->m_hdl) == DDS_KIND_READER) || (dds_entity_kind(e->m_hdl) == DDS_KIND_WRITER));

    if (dds_entity_kind(e->m_hdl) == DDS_KIND_READER) {
        topic = ((dds_reader*)e)->m_topic;
    } else {
        topic = ((dds_writer*)e)->m_topic;
    }
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
    if (ret == DDS_RETCODE_OK) {
        topic = dds_instance_info(w_or_r);
        dds_entity_unlock(w_or_r);
    }
    return topic;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_register_instance(
        _In_ dds_entity_t writer,
        _Out_ dds_instance_handle_t *handle,
        _In_ const void *data)
{
  struct tkmap_instance * inst;
  dds_entity *wr;
  int ret = DDS_RETCODE_BAD_PARAMETER;
  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);

  if (ret == DDS_RETCODE_OK) {
    if(data != NULL && handle != NULL){
      inst = dds_instance_find (((dds_writer*) wr)->m_topic, data, true);
      if(inst != NULL){
        *handle = inst->m_iid;
      } else{
        ret = DDS_RETCODE_ERROR;
      }
    }
    else{
      ret = DDS_RETCODE_BAD_PARAMETER;
    }
    dds_entity_unlock(wr);
  }
  return DDS_ERRNO_DEPRECATED(ret);
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance(
        _In_ dds_entity_t writer,
        _In_opt_ const void *data)
{
  return dds_unregister_instance_ts (writer, data, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ih(
       _In_ dds_entity_t writer,
       _In_opt_ dds_instance_handle_t handle)
{
  return dds_unregister_instance_ih_ts(writer, handle, dds_time());
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ts(
       _In_ dds_entity_t writer,
       _In_opt_ const void *data,
       _In_ dds_time_t timestamp)
{
  int ret = DDS_RETCODE_OK;
  bool autodispose = true;
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  void * sample = (void*) data;
  dds_entity *wr;

  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO_DEPRECATED(ret);
  }
  if (data == NULL){
    ret = DDS_ERRNO_DEPRECATED(DDS_RETCODE_BAD_PARAMETER);
  }

  if (ret == DDS_RETCODE_OK)
  {
    if (wr->m_qos)
    {
      dds_qget_writer_data_lifecycle (wr->m_qos, &autodispose);
    }
    if (autodispose)
    {
      dds_instance_remove (((dds_writer*) wr)->m_topic, data, DDS_HANDLE_NIL);
      action |= DDS_WR_DISPOSE_BIT;
    }

    ret = dds_write_impl ((dds_writer*)wr, sample, timestamp, action);
  }

  dds_entity_unlock(wr);
  return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_unregister_instance_ih_ts(
       _In_ dds_entity_t writer,
       _In_opt_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
  int ret = DDS_RETCODE_OK;
  bool autodispose = true;
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  dds_entity *wr;

  ret = dds_entity_lock(writer, DDS_KIND_WRITER, &wr);
  if (ret != DDS_RETCODE_OK) {
      return DDS_ERRNO_DEPRECATED(ret);
  }

  if (wr->m_qos)
  {
    dds_qget_writer_data_lifecycle (wr->m_qos, &autodispose);
  }
  if (autodispose)
  {
    dds_instance_remove (((dds_writer*) wr)->m_topic, NULL, handle);
    action |= DDS_WR_DISPOSE_BIT;
  }
  struct tkmap *map = gv.m_tkmap;
  const dds_topic *topic = dds_instance_info((dds_entity*)wr);
  void *sample = dds_alloc (topic->m_descriptor->m_size);
  if (dds_tkmap_get_key (map, handle, sample)) {
    ret = dds_write_impl ((dds_writer*)wr, sample, timestamp, action);
  }
  else{
    ret = DDS_ERRNO_DEPRECATED(DDS_RETCODE_BAD_PARAMETER);
  }
  dds_sample_free (sample, topic->m_descriptor, DDS_FREE_ALL);

  dds_entity_unlock(wr);
  return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_writedispose_ts(
       _In_ dds_entity_t writer,
       _In_ const void *data,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_writer *wr;
    ret = dds_writer_lock(writer, &wr);
    if (ret == DDS_RETCODE_OK) {
        ret = dds_write_impl (wr, data, timestamp, DDS_WR_ACTION_WRITE_DISPOSE);
        if (ret == DDS_RETCODE_OK) {
            dds_instance_remove (wr->m_topic, data, DDS_HANDLE_NIL);
        }
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO_DEPRECATED(ret);
    }
    return ret;
}

dds_return_t
dds_dispose_impl(
       _In_ dds_writer *wr,
       _In_ const void *data,
       _In_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    assert(wr);
    ret = dds_write_impl(wr, data, timestamp, DDS_WR_ACTION_DISPOSE);
    if (ret == DDS_RETCODE_OK) {
        dds_instance_remove (wr->m_topic, data, handle);
    }
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ts(
       _In_ dds_entity_t writer,
       _In_ const void *data,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_writer *wr;
    ret = dds_writer_lock(writer, &wr);
    if (ret == DDS_RETCODE_OK) {
        ret = dds_dispose_impl(wr, data, DDS_HANDLE_NIL, timestamp);
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO_DEPRECATED(ret);
    }
    return ret;
}

_Pre_satisfies_((writer & DDS_ENTITY_KIND_MASK) == DDS_KIND_WRITER)
dds_return_t
dds_dispose_ih_ts(
       _In_ dds_entity_t writer,
       _In_ dds_instance_handle_t handle,
       _In_ dds_time_t timestamp)
{
    dds_return_t ret;
    dds_writer *wr;
    ret = dds_writer_lock(writer, &wr);
    if (ret == DDS_RETCODE_OK) {
        struct tkmap *map = gv.m_tkmap;
        const dds_topic *topic = dds_instance_info((dds_entity*)wr);
        void *sample = dds_alloc (topic->m_descriptor->m_size);
        if (dds_tkmap_get_key (map, handle, sample)) {
            ret = dds_dispose_impl(wr, sample, handle, timestamp);
        } else {
            ret = DDS_ERRNO_DEPRECATED(DDS_RETCODE_PRECONDITION_NOT_MET);
        }
        dds_free(sample);
        dds_writer_unlock(wr);
    } else {
        ret = DDS_ERRNO_DEPRECATED(ret);
    }
    return ret;
}

_Pre_satisfies_(entity & DDS_ENTITY_KIND_MASK)
dds_instance_handle_t
dds_instance_lookup(
        dds_entity_t entity,
        const void *data)
{
  dds_instance_handle_t ih = DDS_HANDLE_NIL;
  const dds_topic * topic;
  struct tkmap * map = gv.m_tkmap;
  serdata_t sd;

  assert (data);

  topic = dds_instance_info_by_hdl (entity);
  if (topic) {
      sd = serialize_key (gv.serpool, topic->m_stopic, data);
      ih = dds_tkmap_lookup (map, sd);
      ddsi_serdata_unref (sd);
  }
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
    DDS_RETCODE_OK : DDS_ERRNO_DEPRECATED(DDS_RETCODE_BAD_PARAMETER);
}
