#include <string.h>
#include <assert.h>
#include "kernel/dds_types.h"
#include "kernel/dds_rhc.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_report.h"
#include "kernel/dds_err.h"
#include "kernel/q_osplser.h"
#include "ddsi/q_radmin.h"
#include "ddsi/q_config.h"
#include "client_durability.h"
#include "os/os.h"

#define PARTITION_NAME "durabilityPartition"

#if OS_ENDIANNESS == OS_LITTLE_ENDIAN
#define DDS_ENDIAN true
#else
#define DDS_ENDIAN false
#endif

/* Durable data sample and associated info */

typedef struct dds_dur_data_t
{
  uint8_t * m_buff;
  uint32_t m_len;
  uint32_t m_seqno;
  DDS_MessageFlag_t m_kind;
  DDS_PayloadSerializationFormat_t m_format;
  nn_ddsi_time_t m_stamp;
  uint8_t m_hash[16];
  bool m_auto_dispose;
  int32_t m_ownership_strength;
  struct dds_dur_data_t * m_next;
} dds_dur_data_t;

/* Set of durable samples for a reader */

typedef struct dds_dur_entry_t
{
  DDS_RequestId_t m_rid;
  dds_reader * m_reader;
  struct rhc * m_rhc;
  dds_dur_data_t * m_data;
  struct dds_dur_entry_t * m_next;
} dds_dur_entry_t;

static os_atomic_uint32_t dds_dur_rid = OS_ATOMIC_UINT32_INIT (0);
static dds_dur_entry_t * dds_dur_cache = NULL;
static os_cond dds_dur_cond;
static os_mutex dds_dur_mutex;

static dds_dur_entry_t * dds_dur_find_entry (DDS_RequestId_t * rid, bool remove)
{
  dds_dur_entry_t * entry = dds_dur_cache;
  dds_dur_entry_t * prev = NULL;

  while (entry)
  {
    if
    (
      (entry->m_rid.requestId == rid->requestId) &&
      (entry->m_rid.clientId.prefix == rid->clientId.prefix) &&
      (entry->m_rid.clientId.suffix == rid->clientId.suffix)
    )
    {
      if (remove)
      {
        if (prev)
        {
          prev->m_next = entry->m_next;
        }
        else
        {
          dds_dur_cache = entry->m_next;
        }
      }
      break;
    }
    prev = entry;
    entry = entry->m_next;
  }
  return entry;
}

static void dds_dur_new_entry (dds_reader * reader, struct rhc * rhc, DDS_RequestId_t * rid)
{
  dds_dur_entry_t * entry = dds_alloc (sizeof (*entry));
  entry->m_rid = *rid;
  entry->m_reader = reader;
  entry->m_rhc = rhc;
  entry->m_next = dds_dur_cache;
  dds_dur_cache = entry;
}

static void dds_dur_free_entry (dds_dur_entry_t * entry)
{
  dds_dur_data_t * data;
  while (entry->m_data)
  {
    data = entry->m_data;
    entry->m_data = data->m_next;
    dds_free (data->m_buff);
    dds_free (data);
  }
  dds_free (entry);
}

static void dds_dur_add_entry_data (dds_dur_entry_t * entry, DDS_HistoricalDataBead_t * bead)
{
  dds_dur_data_t * iter;
  dds_dur_data_t * prev;
  dds_dur_data_t * data = dds_alloc (sizeof (*data));

  /* Copy bead into data (take rather than copy data buffer) */

  data->m_len = bead->payload._length;
  data->m_buff = bead->payload._buffer;
  bead->payload._length = 0;
  bead->payload._maximum = 0;
  bead->payload._buffer = NULL;

  data->m_seqno = bead->sequenceNumber;
  data->m_kind = bead->kind;
  data->m_format = bead->serializationFormat;
  memcpy (data->m_hash, bead->keyHash, 16);
  data->m_stamp.seconds = bead->sourceTimestamp.sec;
  data->m_stamp.fraction = bead->sourceTimestamp.nanosec;
  data->m_auto_dispose = bead->qos.writerLifecycleAutoDisposeUnregisteredInstances;

  /* Add data to list, ordered by sequence number (oldest data first) */

  if (entry->m_data)
  {
    prev = NULL;
    iter = entry->m_data;
    while (iter && (iter->m_seqno < data->m_seqno))
    {
      prev = iter;
      iter = iter->m_next;
    }
    if (prev)
    {
      prev->m_next = data;
    }
    else
    {
      entry->m_data = data;
    }
    data->m_next = iter;
  }
  else
  {
    entry->m_data = data;
  }
}

/* Load complete set of durable data for a reader into it's read cache */

static void dds_dur_load_rhc (dds_dur_entry_t * entry)
{
  struct tkmap_instance * inst;
  serstate_t st;
  serdata_t sample;
  struct nn_rsample_info info;
  unsigned statusinfo = 0;
  nn_wctime_t timestamp;
  dds_dur_data_t * data;
  DDS_PayloadSerializationFormat_t format;

  /* Determine native endian */

  format = dds_stream_endian () ?
    DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_LE :
    DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_BE;

  /* Iterate samples (already ordered by sequence number) */

  data = entry->m_data;
  if (!data)
  {
    dds_log_info ("No durable data to load\n");
  }
  while (data)
  {
    memset (&info, 0, sizeof (info));
    info.seq = data->m_seqno;
    info.pwr_info.iid = 0;
    info.pwr_info.auto_dispose = data->m_auto_dispose;
    info.pwr_info.ownership_strength = data->m_ownership_strength;
    info.reception_timestamp = now ();
    info.hashash = 1;
    memcpy (info.keyhash.value, data->m_hash, 16);
    info.bswap = (data->m_format != format);

    st = ddsi_serstate_new (gv.serpool, entry->m_reader->m_topic->m_stopic);
    switch (data->m_kind)
    {
      case DDS_MESSAGE_FLAG_WRITE:
      {
        st->kind = STK_DATA;
        break;
      }
      case DDS_MESSAGE_FLAG_DISPOSE:
      {
        statusinfo = NN_STATUSINFO_DISPOSE;
        st->kind = STK_KEY;
        break;
      }
      case DDS_MESSAGE_FLAG_UNREGISTER:
      {
        statusinfo = NN_STATUSINFO_UNREGISTER;
        st->kind = STK_KEY;
        break;
      }
      case DDS_MESSAGE_FLAG_WRITE_DISPOSE:
      {
        st->kind = STK_DATA;
        statusinfo = NN_STATUSINFO_DISPOSE;
        break;
      }
      case DDS_MESSAGE_FLAG_REGISTER:
      {
        st->kind = STK_KEY;
        break;
      }
      default: assert (0);
    }

    /* Deliver data to read cache */

    timestamp = nn_wctime_from_ddsi_time (data->m_stamp);
    ddsi_serstate_append_blob (st, 4, data->m_len, data->m_buff);
    ddsi_serstate_set_msginfo (st, statusinfo, timestamp, NULL);
    sample = ddsi_serstate_fix (st);
    inst = dds_tkmap_lookup_instance_ref (sample);
    dds_rhc_store (entry->m_rhc, &info, sample, inst);
    ddsi_serdata_unref (sample);
    dds_tkmap_instance_unref (inst);
    data = data->m_next;
  }
}

#define DDS_DURATION_INFINITE_SEC 0x7fffffff
#define DDS_DURATION_INFINITE_NSEC 0x7fffffffU

static void dds_durability_set_duration (DDS_Duration_t * out, nn_duration_t in)
{
  int64_t i64 = nn_from_ddsi_duration (in);
  if (i64 != T_NEVER)
  {
    out->sec = (int32_t) (i64 / DDS_NSECS_IN_SEC);
    out->nanosec = i64 % DDS_NSECS_IN_SEC;
  }
  else
  {
    out->sec = DDS_DURATION_INFINITE_SEC;
    out->nanosec = DDS_DURATION_INFINITE_NSEC;
  }
}

/* Listener callback routine for historic data samples */

static void dds_durability_on_data_available (dds_entity_t reader, void *arg)
{
  int ret;
  uint32_t index = 0;
  uint32_t i;
  bool signal;
  dds_sample_info_t info[1];
  void * samples[1];
  uint32_t mask = DDS_ALIVE_INSTANCE_STATE;
  DDS_HistoricalData sa;
  dds_dur_entry_t * entry;

  memset (&sa, 0, sizeof (sa));
  samples[0] = &sa;

  while (true)
  {
    ret = dds_take_mask (reader, samples, info, 1, 1, mask);
    if (ret <= 0)
    {
      break;
    }
    if (info[index++].valid_data)
    {
      signal = false;
      os_mutexLock (&dds_dur_mutex);
      for (i = 0; i < sa.requestIds._length; i++)
      {
        if (sa.content._d == DDS_HISTORICAL_DATA_KIND_BEAD)
        {
          entry = dds_dur_find_entry (&sa.requestIds._buffer[i], false);
          if (entry)
          {
            dds_dur_add_entry_data (entry, &sa.content._u.bead);
          }
          else
          {
            dds_log_error ("Durability could not find entity for sample\n");
            break;
          }
        }
        else /* DDS_HISTORICAL_DATA_KIND_LINK */
        {
          entry = dds_dur_find_entry (&sa.requestIds._buffer[i], true);
          if (entry)
          {
            if
            (
              (sa.content._u.link.errorCode == 0) &&
              (sa.content._u.link.completeness == DDS_COMPLETENESS_COMPLETE)
            )
            {
              /* Load samples into read cache */

              dds_dur_load_rhc (entry);
            }
            else
            {
              if (sa.content._u.link.errorCode != 0)
              {
                dds_log_error ("Durability error: %d\n", sa.content._u.link.errorCode);
                entry->m_reader->m_entity.m_flags |= DDS_ENTITY_FAILED;
                break;
              }
              else
              {
                if (entry->m_data)
                {
                  /* Failure due to incomplete sample set */

                  dds_log_error ("Durability received incomplete set of samples\n");
                  entry->m_reader->m_entity.m_flags |= DDS_ENTITY_FAILED;
                  break;
                }
              }
            }

            /* Flag reader as no longer waiting */

            entry->m_reader->m_entity.m_flags &= ~DDS_ENTITY_WAITING;
            dds_dur_free_entry (entry);
            signal = true;
          }
        }
      }

      /* Signal waiting readers if sample set delivered or a failure */

      if (signal)
      {
        os_condBroadcast (&dds_dur_cond);
      }
      os_mutexUnlock (&dds_dur_mutex);
    }
    DDS_HistoricalData_free (&sa, DDS_FREE_CONTENTS);
  }
}

static void dds_durability_on_pub_matched (dds_entity_t writer, const struct dds_publication_matched_status status, void *arg)
{
  dds_log_info ("DDS::HistoricalDataRequest writer publication matched\n");
}

static void dds_durability_on_sub_matched (dds_entity_t reader, const struct dds_subscription_matched_status status, void *arg)
{
  dds_log_info ("DDS::HistoricalData reader subscription matched\n");
}

static void get_client_id(const dds_participant * pp, DDS_Gid_t *clientId)
{
  uint32_t * u32ptr;
  u32ptr = (uint32_t*) &clientId->prefix;
  u32ptr[0] = pp->m_entity.m_guid.prefix.u[0];
  u32ptr[1] = pp->m_entity.m_guid.prefix.u[1];
  u32ptr  = (uint32_t*) &clientId->suffix;
  u32ptr[0] = pp->m_entity.m_guid.prefix.u[2];
  u32ptr[1] = pp->m_entity.m_guid.entityid.u;
}

static dds_entity_t dds_durability_register (dds_entity_t pphdl, DDS_Gid_t *clientId)
{
  dds_entity_t rtopic;
  dds_entity_t wtopic;
  dds_qos_t * qos;
  dds_listener_t rlistener;
  dds_listener_t wlistener;
  dds_entity_t dur_rd, dur_wr;
  const char * partitions[1];

  memset (&wlistener, 0, sizeof (wlistener));
  memset (&rlistener, 0, sizeof (rlistener));

  /* Set listeners */

  rlistener.on_data_available = dds_durability_on_data_available;
  rlistener.on_subscription_matched = dds_durability_on_sub_matched;
  wlistener.on_publication_matched = dds_durability_on_pub_matched;

  /* Create topic for historic data samples */

  qos = dds_qos_create ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS (100));
  dds_qset_history (qos, DDS_HISTORY_KEEP_ALL, DDS_LENGTH_UNLIMITED);
  dds_qset_destination_order (qos, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);
/*  dds_qset_liveliness (qos, DDS_LIVELINESS_AUTOMATIC, DDS_INFINITY); */
  dds_qset_liveliness (qos, DDS_LIVELINESS_AUTOMATIC, 0);
  rtopic = dds_create_topic (pphdl, &DDS_HistoricalData_desc, "d_historicalData", qos, NULL);
  DDS_ERR_CHECK (rtopic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create reader for historic data samples */

  partitions[0] = PARTITION_NAME;
  dds_qset_partition (qos, 1, partitions);
  dur_rd = dds_create_reader (pphdl, rtopic, qos, &rlistener);
  DDS_ERR_CHECK (dur_rd, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (qos);

  /* Create topic for historic data request */

  qos = dds_qos_create ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS (100));
  dds_qset_history (qos, DDS_HISTORY_KEEP_ALL, DDS_LENGTH_UNLIMITED);
  dds_qset_destination_order (qos, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);
/*  dds_qset_liveliness (qos, DDS_LIVELINESS_AUTOMATIC, DDS_INFINITY); */
  dds_qset_liveliness (qos, DDS_LIVELINESS_AUTOMATIC, 0);
  wtopic = dds_create_topic (pphdl, &DDS_HistoricalDataRequest_desc, "d_historicalDataRequest", qos, NULL);
  DDS_ERR_CHECK (wtopic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create writer to notify durability server of durable data publication */

  partitions[0] = PARTITION_NAME;
  dds_qset_partition (qos, 1, partitions);
  dur_wr = dds_create_writer (pphdl, wtopic, qos, &wlistener);
  DDS_ERR_CHECK (dur_wr, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  dds_qos_delete (qos);

  /* TODO: Wait for sub/pub matched within timeout (configurable) */

  /* If timeout, log error, delete reader/writer, unplug facility and carry on ... */

  {
    dds_participant * pp;
    dds__retcode_t rc;
    if ((rc = dds_entity_lock (pphdl, DDS_KIND_PARTICIPANT, (dds_entity**)&pp)) != DDS_RETCODE_OK) {
      DDS_ERROR (rc, "get_dur_writer: failed to lock participant.");
      return (dds_entity_t)rc;
    } else {
      pp->m_dur_reader = dur_rd;
      pp->m_dur_writer = dur_wr;
      get_client_id(pp, clientId);
      dds_entity_unlock ((dds_entity*)pp);
      return dur_wr;
    }
  }
}

static dds_entity_t get_dur_writer_and_clientid (dds_entity_t pphdl, DDS_Gid_t *clientId)
{
  dds_participant * pp;
  dds__retcode_t rc;
  dds_entity_t wr = DDS_HANDLE_NIL;
  os_mutexLock (&dds_dur_mutex);
  if ((rc = dds_entity_lock (pphdl, DDS_KIND_PARTICIPANT, (dds_entity**)&pp)) != DDS_RETCODE_OK) {
    os_mutexUnlock (&dds_dur_mutex);
    DDS_ERROR (rc, "get_dur_writer: failed to lock participant.");
    return (dds_entity_t)rc;
  } else {
    if (pp->m_dur_writer != DDS_HANDLE_NIL) {
      wr = pp->m_dur_writer;
      get_client_id(pp, clientId);
      dds_entity_unlock ((dds_entity*)pp);
    } else {
      dds_entity_unlock ((dds_entity*)pp);
      wr = dds_durability_register (pphdl, clientId);
    }
    os_mutexUnlock (&dds_dur_mutex);
    return wr;
  }
}

static void dds_durability_reader (dds_reader * reader, struct rhc * rhc)
{
  dds_entity_t durwr;
  DDS_HistoricalDataRequest request;
  DDS_DataReaderQos * rq = &request.dataReaderQos;
  dds_qos_t * qos = reader->m_entity.m_qos;
  const char * partitions[1];
  const char * data_partitions[1];
  int status;

  partitions[0] = "*";
  data_partitions[0] = PARTITION_NAME;
  memset (&request, 0, sizeof (request));

  /* Initialise participant durability reader/writer on demand */

  durwr = get_dur_writer_and_clientid (dds_get_participant(reader->m_entity.m_hdl), &request.requestId.clientId);

  /* Initialize historic data request */

  request.version.major = 1;
  request.version.vendorId.vendorId[0] = 0x01;
  request.version.vendorId.vendorId[1] = 0x0d;
  request.requestId.requestId = os_atomic_inc32_nv (&dds_dur_rid);
  request.topic = reader->m_topic->m_stopic->name;
  request.partitions._maximum = 1;
  request.partitions._length = 1;
  request.partitions._buffer = (uint8_t*) partitions;
  request.startTime.sec = 0;
  request.startTime.nanosec = 0;
  request.endTime.sec = -1;
  request.endTime.nanosec = 0xffffffffU;
  request.sqlFilter = "";
  request.maxSamples = -1;
  request.maxInstances = -1;
  request.maxSamplesPerInstance = -1;
  request.alignmentPartition._length = 1;
  request.alignmentPartition._buffer = (uint8_t*) data_partitions;
  request.timeout.nanosec = 200000000;  /* TODO: Make configurable when supported in OSPL */
  request.timeout.sec = 0;

  /* Requested serialization format is configurable */

  switch (config.durability_cdr)
  {
    case DUR_CDR_LE:
      request.serializationFormat = DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_LE;
      break;
    case DUR_CDR_BE:
      request.serializationFormat = DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_BE;
      break;
    case DUR_CDR_SERVER:
      request.serializationFormat = DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY;
      break;
    default:
      request.serializationFormat = dds_stream_endian () ?
        DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_LE : DDS_PAYLOAD_SERIALIZATION_FORMAT_CDR_BE;
      break;
  }

  /* Copy QoS (will have had default reader qos values merged) */

  rq->durability.kind = (DDS_DurabilityQosPolicyKind) qos->durability.kind;
  dds_durability_set_duration (&rq->deadline.period, qos->deadline.deadline);
  dds_durability_set_duration (&rq->latency_budget.duration, qos->latency_budget.duration);
  rq->liveliness.kind = (DDS_LivelinessQosPolicyKind) qos->liveliness.kind;
  dds_durability_set_duration (&rq->liveliness.lease_duration, qos->liveliness.lease_duration);
  rq->reliability.kind = (DDS_ReliabilityQosPolicyKind) qos->reliability.kind;
  dds_durability_set_duration (&rq->reliability.max_blocking_time, qos->reliability.max_blocking_time);
  rq->reliability.synchronous = false;
  rq->destination_order.kind = (DDS_DestinationOrderQosPolicyKind) qos->destination_order.kind;
  rq->history.kind = (DDS_HistoryQosPolicyKind) qos->history.kind;
  rq->history.depth = qos->history.depth;
  rq->resource_limits.max_samples = qos->resource_limits.max_samples;
  rq->resource_limits.max_instances = qos->resource_limits.max_instances;
  rq->resource_limits.max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
  rq->ownership.kind = (DDS_OwnershipQosPolicyKind) qos->ownership.kind;
  rq->reader_lifespan.use_lifespan = qos->reader_lifespan.use_lifespan;
  dds_durability_set_duration (&rq->reader_lifespan.duration, qos->reader_lifespan.duration);

  /* Create new entry for reader */

  dds_dur_new_entry (reader, rhc, &request.requestId);

  /* Request durable data */

  status = dds_write (durwr, &request);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Flag reader as waiting */

  reader->m_entity.m_flags |= DDS_ENTITY_WAITING;
}

static int dds_durability_wait (dds_reader * reader, dds_duration_t timeout)
{
  int ret = DDS_RETCODE_OK;
  os_result res;
  os_time tout =
  {
    (os_timeSec) (timeout / DDS_NSECS_IN_SEC),
    (uint32_t) (timeout % DDS_NSECS_IN_SEC)
  };

  os_mutexLock (&dds_dur_mutex);
  while (reader->m_entity.m_flags & DDS_ENTITY_WAITING)
  {
    do
    {
      res = os_condTimedWait (&dds_dur_cond, &dds_dur_mutex, &tout);
    }
    while (res == os_resultFail);

    /* Check for timeout */
    if (res == os_resultTimeout)
    {
      dds_log_warn ("Durability timeout waiting for samples\n");
      ret = DDS_ERRNO (DDS_RETCODE_TIMEOUT, "Durability timeout waiting for samples");
      break;
    }
    if (reader->m_entity.m_flags & DDS_ENTITY_FAILED)
    {
      ret = DDS_ERRNO (DDS_RETCODE_ERROR, "Retrieving of historical data failed");
      break;
    }
  }
  os_mutexUnlock (&dds_dur_mutex);

  return ret;
}

static void dds_durability_init (void)
{
  os_mutexInit (&dds_dur_mutex);
  os_condInit (&dds_dur_cond, &dds_dur_mutex);
}

static void dds_durability_fini (void)
{
  os_condDestroy (&dds_dur_cond);
  os_mutexDestroy (&dds_dur_mutex);
}

void dds_durability_plugin (void)
{
  dds_global.m_dur_reader = dds_durability_reader;
  dds_global.m_dur_wait = dds_durability_wait;
  dds_global.m_dur_fini = dds_durability_fini;
  dds_global.m_dur_init = dds_durability_init;
}
