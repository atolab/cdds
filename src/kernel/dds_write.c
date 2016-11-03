#include <assert.h>
#include "kernel/dds_writer.h"
#include "kernel/dds_write.h"
#include "kernel/dds_tkmap.h"
#include "ddsi/q_error.h"
#include "ddsi/q_thread.h"
#include "kernel/q_osplser.h"
#include "ddsi/q_transmit.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_config.h"
#include "ddsi/q_entity.h"

#if OS_ATOMIC64_SUPPORT
typedef os_atomic_uint64_t fake_seq_t;
uint64_t fake_seq_next (fake_seq_t *x) { return os_atomic_inc64_nv (x); }
#else /* HACK */
typedef os_atomic_uint32_t fake_seq_t;
uint64_t fake_seq_next (fake_seq_t *x) { return os_atomic_inc32_nv (x); }
#endif

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

#include "ddsi/q_radmin.h"
#include <string.h>

static void init_sampleinfo (struct nn_rsample_info *sampleinfo, struct writer *wr, int64_t seq, serdata_t payload)
{
  memset(sampleinfo, 0, sizeof(*sampleinfo));
  sampleinfo->bswap = 0;
  sampleinfo->complex_qos = 0;
  sampleinfo->hashash = 0;
  sampleinfo->seq = seq;
  sampleinfo->reception_timestamp = payload->v.msginfo.timestamp;
  sampleinfo->statusinfo = payload->v.msginfo.statusinfo;
  sampleinfo->pwr_info.iid = 1;
  sampleinfo->pwr_info.auto_dispose = 0;
  sampleinfo->pwr_info.guid = wr->e.guid;
  sampleinfo->pwr_info.ownership_strength = 0;
}

static void deliver_locally (struct writer *wr, int64_t seq, serdata_t payload, struct tkmap_instance * tk)
{
  os_mutexLock (&wr->rdary.rdary_lock);
  if (wr->rdary.fastpath_ok)
  {
    struct reader ** const rdary = wr->rdary.rdary;
    if (rdary[0])
    {
      struct nn_rsample_info sampleinfo;
      unsigned i;
      init_sampleinfo(&sampleinfo, wr, seq, payload);
      for (i = 0; rdary[i]; i++)
      {
        TRACE (("reader %x:%x:%x:%x\n", PGUID (rdary[i]->e.guid)));
        if (! (ddsi_plugin.rhc_store_fn) (rdary[i]->rhc, &sampleinfo, payload, tk))
          abort();
      }
    }
    os_mutexUnlock (&wr->rdary.rdary_lock);
  }
  else
  {
    /* When deleting, pwr is no longer accessible via the hash
     tables, and consequently, a reader may be deleted without
     it being possible to remove it from rdary. The primary
     reason rdary exists is to avoid locking the proxy writer
     but this is less of an issue when we are deleting it, so
     we fall back to using the GUIDs so that we can deliver all
     samples we received from it. As writer being deleted any
     reliable samples that are rejected are simply discarded. */
    ut_avlIter_t it;
    struct pwr_rd_match *m;
    struct nn_rsample_info sampleinfo;
    os_mutexUnlock (&wr->rdary.rdary_lock);
    init_sampleinfo(&sampleinfo, wr, seq, payload);
    os_mutexLock (&wr->e.lock);
    for (m = ut_avlIterFirst (&wr_local_readers_treedef, &wr->local_readers, &it); m != NULL; m = ut_avlIterNext (&it))
    {
      struct reader *rd;
      if ((rd = ephash_lookup_reader_guid (&m->rd_guid)) != NULL)
      {
        TRACE (("reader-via-guid %x:%x:%x:%x\n", PGUID (rd->e.guid)));
        if (! (ddsi_plugin.rhc_store_fn) (rd->rhc, &sampleinfo, payload, tk))
          abort();
      }
    }
    os_mutexUnlock (&wr->e.lock);
  }
}

int dds_write_impl
(
  dds_entity_t wr, const void * data,
  dds_time_t tstamp, dds_write_action action
)
{
  static fake_seq_t fake_seq;
  int ret = DDS_RETCODE_OK;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (data);

  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool writekey = action & DDS_WR_KEY_BIT;
  dds_writer * writer = (dds_writer*) wr;
  struct writer * ddsi_wr = writer->m_wr;
  struct tkmap_instance * tk;
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
  ddsi_serdata_ref(d);
  tk = (ddsi_plugin.rhc_lookup_fn) (d);
  ret = write_sample (writer->m_xp, ddsi_wr, d, tk);

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
    deliver_locally (ddsi_wr, fake_seq_next(&fake_seq), d, tk);
  ddsi_serdata_unref(d);
  (ddsi_plugin.rhc_unref_fn) (tk);

  if (asleep)
  {
    thread_state_asleep (thr);
  }

filtered:

  return ret;
}

int dds_writecdr_impl
(
 dds_entity_t wr, const void * cdr, size_t sz,
 dds_time_t tstamp, dds_write_action action
 )
{
  static fake_seq_t fake_seq;
  int ret = DDS_RETCODE_OK;

  assert (wr);
  assert (wr->m_kind == DDS_TYPE_WRITER);
  assert (cdr);

  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool writekey = action & DDS_WR_KEY_BIT;
  dds_writer * writer = (dds_writer*) wr;
  struct writer * ddsi_wr = writer->m_wr;
  struct tkmap_instance * tk;
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
  tk = (ddsi_plugin.rhc_lookup_fn) (d);
  ret = write_sample (writer->m_xp, ddsi_wr, d, tk);

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
    deliver_locally (ddsi_wr, fake_seq_next(&fake_seq), d, tk);
  ddsi_serdata_unref(d);
  (ddsi_plugin.rhc_unref_fn) (tk);

  if (asleep)
  {
    thread_state_asleep (thr);
  }

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
