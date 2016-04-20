#include <assert.h>
#include "dds_writer.h"
#include "dds_write.h"
#include "dds_tkmap.h"
#include "q_error.h"
#include "q_thread.h"
#include "q_osplser.h"
#include "q_transmit.h"
#include "q_ephash.h"
#include "q_config.h"
#include "q_entity.h"

int dds_write (dds_entity_t wr, const void * data)
{
  return dds_write_impl (wr, data, dds_time (), 0);
}

int dds_writecdr (dds_entity_t wr, const void * cdr, size_t sz)
{
  return dds_writecdr_impl (wr, cdr, sz, dds_time (), 0);
}

int dds_write_ts (dds_entity_t wr, const void * data, dds_time_t tstamp)
{
  return dds_write_impl (wr, data, tstamp, 0);
}

int dds_write_impl
(
  dds_entity_t wr, const void * data,
  dds_time_t tstamp, dds_write_action action
)
{
  int ret = DDS_RETCODE_OK;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (data);

  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool writekey = action & DDS_WR_KEY_BIT;
  dds_writer * writer = (dds_writer*) wr;
  struct writer * ddsi_wr = writer->m_wr;
  serdata_t d;

  /* Check for topic filter */

  if (ddsi_wr->topic->filter_fn && ! writekey)
  {
    if (! (ddsi_wr->topic->filter_fn) (data, ddsi_wr->topic->filter_ctx))
    {
      goto filtered;
    }
  }

  if (asleep)
  {
    thread_state_awake (thr);
  }

  /* Serialize and write data or key */

  if (writekey)
  {
    d = serialize_key (gv.serpool, ddsi_wr->topic, data);
  }
  else
  {
    d = serialize (gv.serpool, ddsi_wr->topic, data);
  }

  /* Set if disposing or unregistering */

  d->v.msginfo.statusinfo = 
    ((action & DDS_WR_DISPOSE_BIT) ? NN_STATUSINFO_DISPOSE : 0) |
    ((action & DDS_WR_UNREGISTER_BIT) ? NN_STATUSINFO_UNREGISTER : 0);
  d->v.msginfo.timestamp.v = tstamp;
  os_mutexLock (&writer->m_call_lock);
  ret = write_sample (writer->m_xp, ddsi_wr, d);

  if (ret >= 0)
  {
    /* Flush out write unless configured to batch */

    if (! config.whc_batch)
    {
      nn_xpack_send (writer->m_xp);
    }

    ret = DDS_RETCODE_OK;
  }
  else if (ret == ERR_TIMEOUT)
  {
    ret = DDS_ERRNO (DDS_RETCODE_TIMEOUT, DDS_MOD_WRITER, DDS_ERR_M1);
  }
  else
  {
    ret = DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_WRITER, DDS_ERR_M2);
  }
  os_mutexUnlock (&writer->m_call_lock);

  if (asleep)
  {
    thread_state_asleep (thr);
  }

filtered:

  return ret;
}

#include "q_radmin.h"
#include <string.h>

static void deliver_locally (const nn_guid_t *wrguid, int64_t seq, size_t nrdguid, const nn_guid_t *rdguid, serdata_t data)
{
  struct reader *rd;
  size_t idx = 0;
  if ((rd = ephash_lookup_reader_guid (&rdguid[idx++])) != NULL)
  {
    struct nn_rsample_info sampleinfo;
    struct tkmap_instance * tk;
    tk = (ddsi_plugin.rhc_lookup_fn) (data);
    memset(&sampleinfo, 0, sizeof(sampleinfo));
    sampleinfo.bswap = 0;
    sampleinfo.complex_qos = 0;
    sampleinfo.hashash = 0;
    sampleinfo.seq = seq;
    sampleinfo.reception_timestamp = data->v.msginfo.timestamp;
    sampleinfo.statusinfo = data->v.msginfo.statusinfo;
    sampleinfo.pwr_info.iid = 1;
    sampleinfo.pwr_info.auto_dispose = 0;
    sampleinfo.pwr_info.guid = *wrguid;
    sampleinfo.pwr_info.ownership_strength = 0;

    do {
      while (rd && ! (ddsi_plugin.rhc_store_fn) (rd->rhc, &sampleinfo, data, tk))
        dds_sleepfor (DDS_MSECS (10));
    } while (idx < nrdguid && (rd = ephash_lookup_reader_guid (&rdguid[idx++])) != NULL);

    (ddsi_plugin.rhc_unref_fn) (tk);
  }
}

int dds_writecdr_impl
(
 dds_entity_t wr, const void * cdr, size_t sz,
 dds_time_t tstamp, dds_write_action action
 )
{
  static os_atomic_uint64_t fake_seq;
  int ret = DDS_RETCODE_OK;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (cdr);

  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool writekey = action & DDS_WR_KEY_BIT;
  dds_writer * writer = (dds_writer*) wr;
  struct writer * ddsi_wr = writer->m_wr;
  serdata_t d;

  /* Check for topic filter */

  if (ddsi_wr->topic->filter_fn && ! writekey)
  {
    abort();
  }

  if (asleep)
  {
    thread_state_awake (thr);
  }

  /* Serialize and write data or key */

  if (writekey)
  {
    abort();
    //d = serialize_key (gv.serpool, ddsi_wr->topic, data);
  }
  else
  {
    serstate_t st = ddsi_serstate_new (gv.serpool, ddsi_wr->topic);
    if (ddsi_wr->topic->nkeys)
    {
      abort();
      //dds_key_gen ((const dds_topic_descriptor_t*) tp->type, &st->data->v.keyhash, (char*) sample);
    }
    ddsi_serstate_append_blob(st, 1, sz, cdr);
    d = ddsi_serstate_fix(st);
  }

  /* Set if disposing or unregistering */

  d->v.msginfo.statusinfo =
  ((action & DDS_WR_DISPOSE_BIT) ? NN_STATUSINFO_DISPOSE : 0) |
  ((action & DDS_WR_UNREGISTER_BIT) ? NN_STATUSINFO_UNREGISTER : 0);
  d->v.msginfo.timestamp.v = tstamp;
  os_mutexLock (&writer->m_call_lock);
  ddsi_serdata_ref(d);
  ret = write_sample (writer->m_xp, ddsi_wr, d);

  if (ret >= 0)
  {
    /* Flush out write unless configured to batch */

    if (! config.whc_batch)
    {
      nn_xpack_send (writer->m_xp);
    }
    ret = DDS_RETCODE_OK;
  }
  else if (ret == ERR_TIMEOUT)
  {
    ret = DDS_ERRNO (DDS_RETCODE_TIMEOUT, DDS_MOD_WRITER, DDS_ERR_M1);
  }
  else
  {
    ret = DDS_ERRNO (DDS_RETCODE_ERROR, DDS_MOD_WRITER, DDS_ERR_M2);
  }
  os_mutexUnlock (&writer->m_call_lock);

  if (ret == DDS_RETCODE_OK)
    deliver_locally (&ddsi_wr->e.guid, os_atomic_inc64_nv(&fake_seq),
                     sizeof(ddsi_wr->local_reader_guid)/sizeof(ddsi_wr->local_reader_guid[0]),
                     ddsi_wr->local_reader_guid, d);
  ddsi_serdata_unref(d);

  if (asleep)
  {
    thread_state_asleep (thr);
  }

filtered:
  
  return ret;
}

void dds_write_set_batch (bool enable)
{
  config.whc_batch = enable ? 1 : 0;
}

void dds_write_flush (dds_entity_t wr)
{
  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);

  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  os_mutexLock (&wr->m_mutex);
  nn_xpack_send (((dds_writer*) wr)->m_xp);
  os_mutexUnlock (&wr->m_mutex);
  if (asleep)
  {
    thread_state_asleep (thr);
  }
}
