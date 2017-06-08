#include <assert.h>
#include <string.h>
#include "kernel/dds_write.h"
#include "kernel/dds_writer.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "ddsi/ddsi_ser.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"

int dds_instance_writedispose (dds_entity_t wr, const void *data)
{
  return dds_instance_writedispose_ts (wr, data, dds_time ());
}

int dds_instance_dispose (dds_entity_t wr, const void *data)
{
  return dds_instance_dispose_ts (wr, data, dds_time ());
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
    dds_tkmap_instance_unref (inst);
  }
}

static const dds_topic * dds_instance_info (dds_entity_t e)
{
  const dds_topic * topic;

  assert (e);
  assert (e->m_kind & DDS_IS_RD_OR_WR);

  if (e->m_kind == DDS_TYPE_READER)
    topic = ((dds_reader*) e)->m_topic;
  else
    topic = ((dds_writer*) e)->m_topic;
  return topic;
}

dds_instance_handle_t dds_instance_register (dds_entity_t wr, const void * data)
{
  struct tkmap_instance * inst;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (data);

  inst = dds_instance_find (((dds_writer*) wr)->m_topic, data, true);
  return inst->m_iid;
}

int dds_instance_unregister (dds_entity_t wr, const void * data, dds_instance_handle_t handle)
{
  return dds_instance_unregister_ts (wr, data, handle, dds_time ());
}

int dds_instance_unregister_ts (dds_entity_t wr, const void * data, dds_instance_handle_t handle, dds_time_t tstamp)
{
  int ret = DDS_RETCODE_OK;
  bool autodispose = true;
  dds_write_action action = DDS_WR_ACTION_UNREGISTER;
  void * sample = (void*) data;
  const dds_topic * topic = NULL;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (data != NULL || handle != DDS_HANDLE_NIL);

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

    ret = dds_write_impl (wr, sample, tstamp, action);
  }

  if (topic)
  {
    dds_sample_free (sample, topic->m_descriptor, DDS_FREE_ALL);
  }

  return ret;
}

int dds_instance_writedispose_ts (dds_entity_t wr, const void *data, dds_time_t tstamp)
{
  int ret = dds_write_impl (wr, data, tstamp, DDS_WR_ACTION_WRITE_DISPOSE);
  dds_instance_remove (((dds_writer*) wr)->m_topic, data, DDS_HANDLE_NIL);
  return ret;
}

int dds_instance_dispose_ts (dds_entity_t wr, const void *data, dds_time_t tstamp)
{
  int ret = dds_write_impl (wr, data, tstamp, DDS_WR_ACTION_DISPOSE);
  dds_instance_remove (((dds_writer*) wr)->m_topic, data, DDS_HANDLE_NIL);
  return ret;
}

dds_instance_handle_t dds_instance_lookup (dds_entity_t e, const void * data)
{
  dds_instance_handle_t ih;
  const dds_topic * topic;
  struct tkmap * map = gv.m_tkmap;
  serdata_t sd;

  assert (e);
  assert (data);

  topic = dds_instance_info (e);
  sd = serialize_key (gv.serpool, topic->m_stopic, data);
  ih = dds_tkmap_lookup (map, sd);
  ddsi_serdata_unref (sd);
  return ih;
}

int dds_instance_get_key
(
  dds_entity_t e,
  dds_instance_handle_t inst,
  void * data
)
{
  const dds_topic * topic;
  struct tkmap * map = gv.m_tkmap;

  assert (data);

  topic = dds_instance_info (e);
  memset (data, 0, topic->m_descriptor->m_size);
  return (dds_tkmap_get_key (map, inst, data)) ?
    DDS_RETCODE_OK : DDS_ERRNO (DDS_RETCODE_BAD_PARAMETER, DDS_MOD_INST, DDS_ERR_M1);
}
