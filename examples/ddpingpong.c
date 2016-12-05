#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

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

#include "os/os.h"

#include "RoundTrip.h"
#include "kernel/dds_reader.h"

#define TIME_STATS_SIZE_INCREMENT 50000
#define MAX_SAMPLES 100
#define NS_IN_ONE_SEC 1000000000LL

typedef struct ExampleTimeStats
{
  dds_time_t * values;
  unsigned long valuesSize;
  unsigned long valuesMax;
  double average;
  dds_time_t min;
  dds_time_t max;
  unsigned long count;
  int skipcount; /* for warming up - certainly the first one typically takes a few ms */
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
  stats->skipcount = 10;
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
  if (stats->skipcount > 0)
    stats->skipcount--;
  else
  {
    if (stats->valuesSize >= stats->valuesMax)
    {
      dds_time_t * temp = (dds_time_t*) realloc (stats->values, (stats->valuesMax + TIME_STATS_SIZE_INCREMENT) * sizeof (dds_time_t));
      if (temp == NULL) abort();
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
  }
  os_mutexUnlock(&statslock);
  return stats;
}

static void writeStats(uint32_t payloadSize, const ExampleTimeStats *stats, FILE *fp)
{
  unsigned long long i;
  fprintf(fp,"{%u,{", payloadSize);
  for (i = 0; i < stats->valuesSize; i++) {
    fprintf(fp, "%s%lld", (i == 0) ? "" : ",", stats->values[i]);
  }
  fprintf(fp,"}}\n");
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
    median = NAN;
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
static volatile sig_atomic_t terminated_flag;

#ifdef _WIN32
static bool CtrlHandler (DWORD fdwCtrlType)
{
  terminated_flag = 1;
  dds_guard_trigger (terminated);
  return true; //Don't let other handlers handle this key
}
#else
static void CtrlHandler (int fdwCtrlType)
{
  terminated_flag = 1;
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
  dds_time_t tprint;
  dds_time_t tstart;
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
      dds_time_t difference = postTakeTime - sourcetime.v;
      exampleAddTimingToTimeStats (arg->roundTrip, difference);
      exampleAddTimingToTimeStats (arg->roundTripOverall, difference);

      if (postTakeTime >= arg->tprint)
      {
        int64_t k = (postTakeTime - arg->tstart) / DDS_NSECS_IN_SEC;
        printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", k, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip)/1000, roundTrip.min/1000);
        exampleResetTimeStats (&roundTrip);
        arg->tprint = postTakeTime + DDS_SECS(1);
      }

      sourcetime.v = dds_time ();
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

struct data_available_handler_arg {
  dds_entity_t writer;
  int isping;
  dds_time_t tstart;
  dds_time_t tprint;
};

static void data_available_handler (dds_entity_t reader, void *varg)
{
  struct data_available_handler_arg *arg = varg;
  dds_time_t postTakeTime, difference;
  int status = dds_take (reader, samples, MAX_SAMPLES, info, 0);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  postTakeTime = dds_time ();

  if (arg->isping)
  {
    difference = postTakeTime - info[0].source_timestamp;
    exampleAddTimingToTimeStats (&roundTrip, difference);
    exampleAddTimingToTimeStats (&roundTripOverall, difference);

    if (postTakeTime >= arg->tprint)
    {
      int64_t k = (postTakeTime - arg->tstart) / DDS_NSECS_IN_SEC;
      printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", k, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip)/1000, roundTrip.min/1000);
      exampleResetTimeStats (&roundTrip);
      arg->tprint = postTakeTime + DDS_SECS(1);
    }

    postTakeTime = dds_time ();
    info[0].source_timestamp = postTakeTime;
  }

  status = dds_write_ts (arg->writer, &sub_data[0], info[0].source_timestamp);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
}

void usage (const char *argv0)
{
  fprintf (stderr, "%s {ping|pong|ddping|ddpong|lping|lpong} [payloadSize]\n", argv0);
  exit (1);
}

int main (int argc, char *argv[])
{
  dds_entity_t participant;
  dds_entity_t topic, addrtopic;
  dds_entity_t reader, addrreader;
  dds_entity_t writer, addrwriter;
  dds_entity_t publisher;
  dds_entity_t subscriber;
  dds_waitset_t waitSet;
  enum mode { WAITSET, LISTENER, DIRECT, UDP } mode = LISTENER;
  int isping = 0;

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
  dds_condition_t readCond;
  dds_readerlistener_t rd_listener;
  int opt;
  const char *logfile = NULL;
  struct data_available_handler_arg dah_arg;

  int udpsock;
  struct sockaddr_in udpaddr;
  socklen_t udpaddrsize = sizeof (udpaddr);

  while ((opt = getopt (argc, argv, "f:")) != EOF)
  {
    switch (opt)
    {
      case 'f':
        logfile = optarg;
        break;
      default:
        usage (argv[0]);
        break;
    }
  }

  if (argc - optind < 1 || argc - optind > 2 || strlen (argv[optind]) < 4)
    usage (argv[0]);

  {
    const size_t modelen = strlen (argv[optind]) - 4;
    const char *dirstr = argv[optind] + modelen;
    if (strcmp (dirstr, "ping") == 0)
      isping = 1;
    else if (strcmp (dirstr, "pong") == 0)
      isping = 0;
    else
      usage (argv[0]);
    if (modelen == 0)
      mode = WAITSET;
    else if (modelen == 1 && strncmp (argv[optind], "l", modelen) == 0)
      mode = LISTENER;
    else if (modelen == 2 && strncmp (argv[optind], "dd", modelen) == 0)
      mode = DIRECT;
    else if (modelen == 3 && strncmp (argv[optind], "udp", modelen) == 0)
      mode = UDP;
    else
      usage (argv[0]);
  }

  if (argc - optind >= 2 && (payloadSize = (uint32_t)atol (argv[2])) > 65536)
    usage(argv[0]);

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

  if ((udpsock = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror ("create udp socket"); exit (1);
  }
  memset (&udpaddr, 0, sizeof (udpaddr));
  udpaddr.sin_addr = ((struct sockaddr_in *) &gv.ownip)->sin_addr;
  udpaddr.sin_family = AF_INET;
  if (bind (udpsock, (struct sockaddr *)&udpaddr, sizeof (udpaddr)) == -1) {
    perror ("bind udp socket"); exit (1);
  }
  if (getsockname (udpsock, (struct sockaddr *)&udpaddr, &udpaddrsize) == -1) {
    perror ("getsockname on udp socket"); exit (1);
  }

  /* A DDS_Topic is created for our sample type on the domain participant. */
  status = dds_topic_create (participant, &topic, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  status = dds_topic_create (participant, &addrtopic, &RoundTripModule_Address_desc, "UDPRoundTrip", NULL, NULL);
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
  rd_listener.arg = &dah_arg;
  status = dds_reader_create (subscriber, &reader, topic, drQos, (mode == LISTENER) ? &rd_listener : NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  dds_qset_durability (drQos, DDS_DURABILITY_TRANSIENT_LOCAL);
  status = dds_reader_create (subscriber, &addrreader, addrtopic, drQos, NULL);
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

  dds_qset_writer_data_lifecycle (dwQos, true);
  dds_qset_durability (dwQos, DDS_DURABILITY_TRANSIENT_LOCAL);
  status = dds_writer_create (publisher, &addrwriter, addrtopic, dwQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (dwQos);
  
  terminated = dds_guardcondition_create ();
  waitSet = dds_waitset_create ();

  if (mode != LISTENER)
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

  if (mode == UDP)
  {
    static char buf[65536];
    struct RoundTripModule_Address addrsample;
    struct sockaddr_in peeraddr;
    socklen_t peeraddrlen = sizeof (peeraddr);
    ssize_t sz;
    if (isping)
    {
      void *addrsamples[] = { &addrsample };
      dds_sample_info_t infos[1];
      dds_time_t tprint, tsend;
      printf ("# payloadSize: %"PRIu32" | numSamples: %llu | timeOut: %" PRIi64 "\n\n", payloadSize, numSamples, timeOut);
      memset (buf, 0, sizeof (buf));
      do {
        status = dds_take (addrreader, addrsamples, 1, infos, 0);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
      } while (status != 1);
      memset(&peeraddr, 0, sizeof(peeraddr));
      peeraddr.sin_family = AF_INET;
      if (!inet_pton(AF_INET, addrsample.ip, &peeraddr.sin_addr))
        abort();

      printf("# Round trip measurements (in us)\n");
      printf("#             Round trip time [us]\n");
      printf("# Seconds     Count   median      min\n");

      peeraddr.sin_port = htons(addrsample.port);
      tsend = dds_time();
      tprint = tsend + DDS_SECS(1);
      sendto(udpsock, buf, payloadSize, 0, (struct sockaddr *)&peeraddr, peeraddrlen);
      while ((sz = recvfrom(udpsock, buf, sizeof(buf), 0, (struct sockaddr *)&peeraddr, &peeraddrlen)) == payloadSize && !terminated_flag)
      {
        dds_time_t trecv = dds_time();
        exampleAddTimingToTimeStats (&roundTrip, trecv - tsend);
        exampleAddTimingToTimeStats (&roundTripOverall, trecv - tsend);
        if (trecv >= tprint)
        {
          printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", elapsed + 1, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip)/1000, roundTrip.min/1000);
          exampleResetTimeStats (&roundTrip);
          tprint = trecv + DDS_SECS(1);
          elapsed++;
        }
        tsend = dds_time();
        sendto(udpsock, buf, payloadSize, 0, (struct sockaddr *)&peeraddr, peeraddrlen);
      }
    }
    else
    {
      char ipstr[256];
      inet_ntop(AF_INET, &udpaddr.sin_addr, ipstr, sizeof(ipstr));
      addrsample.ip = ipstr;
      addrsample.port = ntohs(udpaddr.sin_port);
      dds_write(addrwriter, &addrsample);
      while ((sz = recvfrom(udpsock, buf, sizeof(buf), 0, (struct sockaddr *)&peeraddr, &peeraddrlen)) >= 0 && !terminated_flag)
        sendto(udpsock, buf, (size_t)sz, 0, (struct sockaddr *)&peeraddr, peeraddrlen);
    }
  }
  else
  {
    dah_arg.isping = isping;
    dah_arg.writer = writer;
    dah_arg.tstart = dds_time();
    dah_arg.tprint = dah_arg.tstart + DDS_SECS(1);

    if (mode == DIRECT)
    {
      static struct reader_cbarg arg;
      arg.tp = (struct sertopic *) ((struct dds_reader *)reader)->m_rd->topic;
      arg.tk = get_tkmap_instance(arg.tp);
      arg.xp = ((struct dds_writer *)writer)->m_xp;
      arg.wr = ((struct dds_writer *)writer)->m_wr;
      arg.tstart = dds_time();
      arg.tprint = arg.tstart + DDS_SECS(1);
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
        status = dds_waitset_wait (waitSet, wsresults, wsresultsize, (mode != WAITSET) ? DDS_INFINITY : waitTimeout);
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
          if (status > 0)
          {
            difference = postTakeTime - info[0].source_timestamp;
            exampleAddTimingToTimeStats (&roundTrip, difference);
            exampleAddTimingToTimeStats (&roundTripOverall, difference);
          }

          /* Print stats each second */
          difference = postTakeTime - startTime;
          if (difference > NS_IN_ONE_SEC || (i && i == numSamples))
          {
            printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", elapsed + 1, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip)/1000, roundTrip.min/1000);
            exampleResetTimeStats (&roundTrip);
            startTime = dds_time ();
            elapsed++;
          }
        }
        else
        {
          elapsed ++;
        }

        /* Write a sample that pong can send back */
        if (mode == WAITSET)
        {
          status = dds_write (writer, &pub_data);
          DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
        }
        else
        {
          dds_time_t tnow = dds_time();
          difference = tnow - startTime;
          if (difference > NS_IN_ONE_SEC)
          {
            //printf("%9" PRIi64 " %9lu %8.0f %8"PRId64"\n", elapsed + 1, roundTrip.count, exampleGetMedianFromTimeStats (&roundTrip)/1000, roundTrip.min/1000);
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

      printf("\n%9s %9lu xxx %8"PRId64"\n", "# Overall", roundTripOverall.count, roundTripOverall.min);
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
  }

  /* Disable callbacks */

  dds_status_set_enabled (reader, 0);
  if (mode == DIRECT)
  {
    dds_reader_ddsi2direct (reader, 0, NULL);
    dds_sleepfor(DDS_MSECS(100));
  }

  /* Clean up */

  if (isping && logfile)
  {
    FILE *fp = fopen(logfile, "a");
    if (fp) {
      writeStats(payloadSize, &roundTripOverall, fp);
      fclose (fp);
    }
  }

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
