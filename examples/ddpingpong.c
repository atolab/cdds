#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <inttypes.h>

#include "ddsi/q_entity.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_xmsg.h"
#include "ddsi/q_transmit.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_bswap.h"
#include "ddsi/q_radmin.h"
#include "ddsi/q_protocol.h"
#include "ddsi/q_misc.h"
#include "ddsi/q_config.h"
#include "ddsi/q_globals.h"
#include "ddsi/q_radmin.h"
#include "ddsi/ddsi_ser.h"

#include "RoundTrip.h"
#include "kernel/dds_reader.h"

#define TIME_STATS_SIZE_INCREMENT 50000
#define MAX_SAMPLES 100
#define US_IN_ONE_SEC 1000000LL

typedef struct ExampleTimeStats
{
  dds_time_t * values;
  unsigned long valuesSize;
  unsigned long valuesMax;
  double average;
  dds_time_t min;
  dds_time_t max;
  unsigned long count;
} ExampleTimeStats;

RoundTripModule_DataType sub_data[MAX_SAMPLES];
void *samples[MAX_SAMPLES];
dds_sample_info_t info[MAX_SAMPLES];

ExampleTimeStats roundTrip;
ExampleTimeStats roundTripOverall;
os_mutex statslock;

static void exampleInitTimeStats (ExampleTimeStats *stats)
{
  stats->values = (dds_time_t*) malloc (TIME_STATS_SIZE_INCREMENT * sizeof (dds_time_t));
  stats->valuesSize = 0;
  stats->valuesMax = TIME_STATS_SIZE_INCREMENT;
  stats->average = 0;
  stats->min = 0;
  stats->max = 0;
  stats->count = 0;
}

static void exampleResetTimeStats (ExampleTimeStats *stats)
{
  os_mutexLock (&statslock);
  memset (stats->values, 0, stats->valuesMax * sizeof (unsigned long));
  stats->valuesSize = 0;
  stats->average = 0;
  stats->min = 0;
  stats->max = 0;
  stats->count = 0;
  os_mutexUnlock(&statslock);
}

static void exampleDeleteTimeStats (ExampleTimeStats *stats)
{
  free (stats->values);
}

static ExampleTimeStats *exampleAddTimingToTimeStats
  (ExampleTimeStats *stats, dds_time_t timing)
{
  os_mutexLock(&statslock);
  if (stats->valuesSize > stats->valuesMax)
  {
    dds_time_t * temp = (dds_time_t*) realloc (stats->values, (stats->valuesMax + TIME_STATS_SIZE_INCREMENT) * sizeof (dds_time_t));
    stats->values = temp;
    stats->valuesMax += TIME_STATS_SIZE_INCREMENT;
  }
  if (stats->valuesSize < stats->valuesMax)
  {
    stats->values[stats->valuesSize++] = timing;
  }
  stats->average = (stats->count * stats->average + timing) / (stats->count + 1);
  stats->min = (stats->count == 0 || timing < stats->min) ? timing : stats->min;
  stats->max = (stats->count == 0 || timing > stats->max) ? timing : stats->max;
  stats->count++;
  os_mutexUnlock(&statslock);
  return stats;
}

static int exampleCompareT (const void* a, const void* b)
{
  dds_time_t ul_a = *((dds_time_t*)a);
  dds_time_t ul_b = *((dds_time_t*)b);

  if (ul_a < ul_b) return -1;
  if (ul_a > ul_b) return 1;
  return 0;
}

static double exampleGetMedianFromTimeStats (ExampleTimeStats *stats)
{
  double median = 0.0;

  os_mutexLock(&statslock);
  if (stats->valuesSize == 0)
    median = 1.0/0.0;
  else
  {
    qsort (stats->values, stats->valuesSize, sizeof (dds_time_t), exampleCompareT);

    if (stats->valuesSize % 2 == 0)
    {
      median = (double)(stats->values[stats->valuesSize / 2 - 1] + stats->values[stats->valuesSize / 2]) / 2;
    }
    else
    {
      median = (double)stats->values[stats->valuesSize / 2];
    }
  }
  os_mutexUnlock(&statslock);
  
  return median;
}

static dds_condition_t terminated;

#ifdef _WIN32
static bool CtrlHandler (DWORD fdwCtrlType)
{
  dds_guard_trigger (terminated);
  return true; //Don't let other handlers handle this key
}
#else
static void CtrlHandler (int fdwCtrlType)
{
  dds_guard_trigger (terminated);
}
#endif

static int defragment (unsigned char **datap, const struct nn_rdata *fragchain, uint32_t sz)
{
  if (fragchain->nextfrag == NULL)
  {
    *datap = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
    return 0;
  }
  else
  {
    unsigned char *buf;
    if ((buf = malloc (sz)) != NULL)
    {
      uint32_t off = 0;
      while (fragchain)
      {
        assert (fragchain->min <= off);
        assert (fragchain->maxp1 <= sz);
        if (fragchain->maxp1 > off)
        {
          /* only copy if this fragment adds data */
          const unsigned char *payload = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
          memcpy (buf + off, payload + off - fragchain->min, fragchain->maxp1 - off);
          off = fragchain->maxp1;
        }
        fragchain = fragchain->nextfrag;
      }
    }
    *datap = buf;
    return 1;
  }
}

struct reader_cbarg {
  struct tkmap_instance *tk;
  struct sertopic *tp;
  struct nn_xpack *xp;
  struct writer *wr;
  ExampleTimeStats *roundTrip;
  ExampleTimeStats *roundTripOverall;
};

struct tkmap_instance *get_tkmap_instance(struct sertopic *tp)
{
  serstate_t st;
  serdata_t d;
  struct tkmap_instance *tk;
  uint32_t zero = 0;
  st = ddsi_serstate_new (gv.serpool, tp);
  ddsi_serstate_append_blob(st, 4, 4, &zero);
  ddsi_serstate_set_msginfo(st, 0, (nn_wctime_t){0}, NULL);
  d = ddsi_serstate_fix(st);
  thread_state_awake(lookup_thread_state());
  tk = ddsi_plugin.rhc_lookup_fn(d);
  thread_state_asleep(lookup_thread_state());
  ddsi_serdata_unref(d);
  return tk;
}

void reader_cb (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, void *varg)
{
  struct reader_cbarg *arg = varg;
  Data_DataFrag_common_t *msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  unsigned char data_smhdr_flags = normalize_data_datafrag_flags (&msg->smhdr, config.buggy_datafrag_flags_mode);
  if (sampleinfo->statusinfo == 0 && !sampleinfo->complex_qos && (data_smhdr_flags & DATA_FLAG_DATAFLAG))
  {
    unsigned char *datap;
    int mustfree;
    const struct CDRHeader *hdr;
    serstate_t st;
    serdata_t d;
    nn_wctime_t sourcetime;

    sourcetime = nn_wctime_from_ddsi_time(sampleinfo->timestamp);

    if (arg->roundTrip)
    {
      dds_time_t postTakeTime = dds_time ();
      dds_time_t difference = (postTakeTime - sourcetime.v)/DDS_NSECS_IN_USEC;
      exampleAddTimingToTimeStats (arg->roundTrip, difference);
      exampleAddTimingToTimeStats (arg->roundTripOverall, difference);
      sourcetime.v = postTakeTime;
    }

    mustfree = defragment (&datap, fragchain, sampleinfo->size);
    hdr = (const struct CDRHeader *) datap;
#if PLATFORM_IS_LITTLE_ENDIAN
    if (hdr->identifier != CDR_LE) abort();
#else
    if (hdr->identifier != CDR_BE) abort();
#endif
    st = ddsi_serstate_new (gv.serpool, arg->tp);
    ddsi_serstate_append_blob(st, 4, sampleinfo->size - 4, datap + 4);
    ddsi_serstate_set_msginfo(st, 0, sourcetime, NULL);
    d = ddsi_serstate_fix(st);
    write_sample(arg->xp, arg->wr, d, arg->tk);
    nn_xpack_send(arg->xp, true);
    if (mustfree) free (datap);
  }
}

dds_entity_t writer;
int isping = 1;
int isdd = 0;
int islistener = 0;

static void data_available_handler (dds_entity_t reader)
{
  dds_time_t postTakeTime, difference;
  int status = dds_take (reader, samples, MAX_SAMPLES, info, 0);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  postTakeTime = dds_time ();

  if (isping)
  {
    difference = (postTakeTime - info[0].source_timestamp)/DDS_NSECS_IN_USEC;
    exampleAddTimingToTimeStats (&roundTrip, difference);
    exampleAddTimingToTimeStats (&roundTripOverall, difference);
    info[0].source_timestamp = postTakeTime;
  }

  status = dds_write_ts (writer, &sub_data[0], info[0].source_timestamp);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
}

int main (int argc, char *argv[])
{
  dds_entity_t reader;
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t publisher;
  dds_entity_t subscriber;
  dds_waitset_t waitSet;
  
  const char *pubPartitions[1] = { "ping" };
  const char *subPartitions[1] = { "pong" };
  dds_qos_t *pubQos;
  dds_qos_t *dwQos;
  dds_qos_t *drQos;
  dds_qos_t *subQos;

  uint32_t payloadSize = 0;
  unsigned long long numSamples = 0;
  dds_time_t timeOut = 0;
  dds_time_t startTime;
  dds_time_t postTakeTime;
  dds_time_t difference = 0;
  dds_time_t elapsed = 0;

  RoundTripModule_DataType pub_data;

  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (1);
  unsigned long i;
  int status;
  bool invalid = false;
  dds_condition_t readCond;
  dds_readerlistener_t rd_listener;

  if (argc != 2 && argc != 3)
    invalid = true;
  if (strcmp (argv[1], "ping") == 0)
    isping = 1;
  else if (strcmp (argv[1], "pong") == 0)
    isping = 0;
  else if (strcmp (argv[1], "ddping") == 0)
    isping = 1, isdd = 1;
  else if (strcmp (argv[1], "ddpong") == 0)
    isping = 0, isdd = 1;
  else if (strcmp (argv[1], "lping") == 0)
    isping = 1, islistener = 1;
  else if (strcmp (argv[1], "lpong") == 0)
    isping = 0, islistener = 1;
  else
    invalid = true;
  if (argc >= 3 && (payloadSize = (uint32_t)atol (argv[1])) > 65536)
    invalid = true;
  if (invalid)
  {
    fprintf (stderr, "%s {ping|pong|ddping|ddpong|lping|lpong} [payloadSize]\n", argv[0]);
    return 1;
  }

  if (isping) {
    pubPartitions[0] = "ping";
    subPartitions[0] = "pong";
  } else {
    pubPartitions[0] = "pong";
    subPartitions[0] = "ping";
  }

  /* Register handler for Ctrl-C */
#ifdef _WIN32
  SetConsoleCtrlHandler ((PHANDLER_ROUTINE)CtrlHandler, TRUE);
#else
  struct sigaction sat, oldAction;
  sat.sa_handler = CtrlHandler;
  sigemptyset (&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
#endif

  exampleInitTimeStats (&roundTrip);
  exampleInitTimeStats (&roundTripOverall);

  memset (&sub_data, 0, sizeof (sub_data));
  memset (&pub_data, 0, sizeof (pub_data));

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = &sub_data[i];
  }

  os_mutexInit(&statslock, NULL);

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* A DDS_Topic is created for our sample type on the domain participant. */
  status = dds_topic_create
    (participant, &topic, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* A DDS_Publisher is created on the domain participant. */
  pubQos = dds_qos_create ();
  dds_qset_partition (pubQos, 1, pubPartitions);

  /* A DDS_Subscriber is created on the domain participant. */
  subQos = dds_qos_create ();
  dds_qset_partition (subQos, 1, subPartitions);

  status = dds_subscriber_create (participant, &subscriber, subQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (subQos);
  /* A DDS_DataReader is created on the Subscriber & Topic with a modified QoS. */
  drQos = dds_qos_create ();
  dds_qset_reliability (drQos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  memset (&rd_listener, 0, sizeof (rd_listener));
  rd_listener.on_data_available = data_available_handler;
  status = dds_reader_create (subscriber, &reader, topic, drQos, islistener ? &rd_listener : NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (drQos);

  status = dds_publisher_create (participant, &publisher, pubQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (pubQos);

  /* A DDS_DataWriter is created on the Publisher & Topic with a modified Qos. */
  dwQos = dds_qos_create ();
  dds_qset_reliability (dwQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  dds_qset_writer_data_lifecycle (dwQos, false);
  status = dds_writer_create (publisher, &writer, topic, dwQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (dwQos);
  
  terminated = dds_guardcondition_create ();
  waitSet = dds_waitset_create ();

  if (!islistener)
  {
    readCond = dds_readcondition_create (reader, DDS_ANY_STATE);
    status = dds_waitset_attach (waitSet, readCond, reader);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  }
  else
  {
    readCond = 0;
  }

  status = dds_waitset_attach (waitSet, terminated, terminated);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  setvbuf(stdout, NULL, _IONBF, 0);

  if (isdd)
  {
    static struct reader_cbarg arg;
    arg.tp = (struct sertopic *) ((struct dds_reader *)reader)->m_rd->topic;
    arg.tk = get_tkmap_instance(arg.tp);
    arg.xp = ((struct dds_writer *)writer)->m_xp;
    arg.wr = ((struct dds_writer *)writer)->m_wr;
    if (isping)
    {
      arg.roundTrip = &roundTrip;
      arg.roundTripOverall = &roundTripOverall;
    }
    else
    {
      arg.roundTrip = NULL;
      arg.roundTripOverall = NULL;
    }
    dds_reader_ddsi2direct (reader, reader_cb, &arg);
  }

  if (isping)
  {
    printf ("# payloadSize: %"PRIu32" | numSamples: %llu | timeOut: %" PRIi64 "\n\n", payloadSize, numSamples, timeOut);

    pub_data.payload._length = payloadSize;
    pub_data.payload._buffer = payloadSize ? dds_alloc (payloadSize) : NULL;
    pub_data.payload._release = true;
    pub_data.payload._maximum = 0;
    for (i = 0; i < payloadSize; i++)
    {
      pub_data.payload._buffer[i] = 'a';
    }

    printf("# Round trip measurements (in us)\n");
    printf("#             Round trip time [us]\n");
    printf("# Seconds     Count   median      min\n");

    startTime = dds_time ();
    /* Write a sample that pong can send back */
    status = dds_write (writer, &pub_data);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    for (i = 0; !dds_condition_triggered (terminated) && (!numSamples || i < numSamples); i++)
    {
      /* Wait for response from pong */
      status = dds_waitset_wait (waitSet, wsresults, wsresultsize, waitTimeout);
      DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
      if (status != 0)
      {
        /* Take sample and check that it is valid */
        status = dds_take (reader, samples, MAX_SAMPLES, info, 0);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
        postTakeTime = dds_time ();

        if (!dds_condition_triggered (terminated))
        {
          if (status != 1)
          {
            fprintf (stdout, "%s%d%s", "ERROR: Ping received ", status,
                     " samples but was expecting 1. Are multiple pong applications running?\n");

            return (0);
          }
          else if (!info[0].valid_data)
          {
            printf ("ERROR: Ping received an invalid sample. Has pong terminated already?\n");
            return (0);
          }
        }

        /* Update stats */
        difference = (postTakeTime - info[0].source_timestamp)/DDS_NSECS_IN_USEC;
        exampleAddTimingToTimeStats (&roundTrip, difference);
        exampleAddTimingToTimeStats (&roundTripOverall, difference);

        /* Print stats each second */
        difference = (postTakeTime - startTime)/DDS_NSECS_IN_USEC;
        if (difference > US_IN_ONE_SEC || (i && i == numSamples))
        {
          printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", elapsed + 1, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip), roundTrip.min);
          exampleResetTimeStats (&roundTrip);
          startTime = dds_time ();
          elapsed++;
        }
      }
      else
      {
        elapsed += waitTimeout / DDS_NSECS_IN_SEC;
      }

      /* Write a sample that pong can send back */
      if (!isdd && !islistener)
      {
        status = dds_write (writer, &pub_data);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
      }
      else
      {
        dds_time_t tnow = dds_time();
        difference = (tnow - startTime)/DDS_NSECS_IN_USEC;
        if (difference > US_IN_ONE_SEC)
        {
          printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", elapsed + 1, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip), roundTrip.min);
          exampleResetTimeStats (&roundTrip);
          startTime = tnow;
          elapsed++;
        }
      }

      if (timeOut && elapsed == timeOut)
      {
        dds_guard_trigger (terminated);
      }
    }

    printf("\n%9s %9lu %8.0f %8"PRId64"\n", "# Overall", roundTripOverall.count, exampleGetMedianFromTimeStats (&roundTripOverall), roundTripOverall.min);
  }
  else
  {
    printf ("Waiting for samples from ping to send back...\n");
    fflush (stdout);

    while (!dds_condition_triggered (terminated))
    {
      /* Wait for a sample from ping */
      int samplecount;
      int j;

      status = dds_waitset_wait (waitSet, wsresults, wsresultsize, waitTimeout);
      DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

      /* Take samples */
      samplecount = dds_take (reader, samples, MAX_SAMPLES, info, 0);
      DDS_ERR_CHECK (samplecount, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
      for (j = 0; !dds_condition_triggered (terminated) && j < samplecount; j++)
      {
        /* If writer has been disposed terminate pong */

        if (info[j].instance_state == DDS_IST_NOT_ALIVE_DISPOSED)
        {
          printf ("Received termination request. Terminating.\n");
          dds_guard_trigger (terminated);
          break;
        }
        else if (info[j].valid_data)
        {
          /* If sample is valid, send it back to ping */

          RoundTripModule_DataType * valid_sample = &sub_data[j];
          status = dds_write_ts (writer, valid_sample, info[j].source_timestamp);
          DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
        }
      }
    }
  }

  /* Disable callbacks */

  dds_status_set_enabled (reader, 0);
  if (isdd)
  {
    dds_reader_ddsi2direct (reader, 0, NULL);
    usleep(100000);
  }

  /* Clean up */

  exampleDeleteTimeStats (&roundTrip);
  exampleDeleteTimeStats (&roundTripOverall);

  if (readCond)
  {
    status = dds_waitset_detach (waitSet, readCond);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  }
  status = dds_waitset_detach (waitSet, terminated);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  if (readCond)
    dds_condition_delete (readCond);
  dds_condition_delete (terminated);
  status = dds_waitset_delete (waitSet);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_entity_delete (participant);

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    RoundTripModule_DataType_free (&sub_data[i], DDS_FREE_CONTENTS);
  }
  RoundTripModule_DataType_free (&pub_data, DDS_FREE_CONTENTS);

  dds_fini ();

  return 0;
}
