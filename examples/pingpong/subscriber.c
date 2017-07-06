#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "Throughput.h"

/*
 * The Throughput example measures data throughput in bytes per second. The publisher
 * allows you to specify a payload size in bytes as well as allowing you to specify
 * whether to send data in bursts. The publisher will continue to send data forever
 * unless a time out is specified. The subscriber will receive data and output the
 * total amount received and the data rate in bytes per second. It will also indicate
 * if any samples were received out of order. A maximum number of cycles can be
 * specified and once this has been reached the subscriber will terminate and output
 * totals and averages.
 */

#define BYTES_PER_SEC_TO_MEGABITS_PER_SEC 125000.0
#define MAX_SAMPLES 1000

typedef struct HandleEntry
{
  dds_instance_handle_t handle;
  unsigned long long count;
  struct HandleEntry * next;
} HandleEntry;

typedef struct HandleMap
{
  HandleEntry *entries;
} HandleMap;

static unsigned long cycles = 0;
static dds_instance_handle_t ph = 0;

static HandleMap * imap;
static HandleEntry * current = NULL;
static volatile unsigned long long outOfOrder = 0;
static volatile unsigned long long total_bytes = 0;
static volatile unsigned long long total_samples = 0;

static volatile dds_time_t startTime = 0;
static dds_time_t time_now = 0;

static volatile unsigned long payloadSize = 0;

static ThroughputModule_DataType data [MAX_SAMPLES];
static void * samples[MAX_SAMPLES];
static long pollingDelay = 0;

static dds_condition_t terminated;

/* Functions to handle Ctrl-C presses. */

#ifdef _WIN32
static int CtrlHandler (DWORD fdwCtrlType)
{
  dds_guard_trigger (terminated);
  return true; /* Don't let other handlers handle this key */
}
#else
struct sigaction oldAction;
static void CtrlHandler (int fdwCtrlType)
{
  dds_guard_trigger (terminated);
}
#endif
/*
 * This struct contains all of the entities used in the publisher and subscriber.
 */

static HandleMap * HandleMap__alloc (void)
{
  HandleMap * map = malloc (sizeof (*map));
  memset (map, 0, sizeof (*map));
  return map;
}

static void HandleMap__free (HandleMap *map)
{
  HandleEntry * entry;

  while (map->entries)
  {
    entry = map->entries;
    map->entries = entry->next;
    free (entry);
  }
  free (map);
}

static HandleEntry* store_handle (HandleMap *map, dds_instance_handle_t key)
{
  HandleEntry * entry = malloc (sizeof (*entry));
  memset (entry, 0, sizeof (*entry));

  entry->handle = key;
  entry->next = map->entries;
  map->entries = entry;

  return entry;
}

static HandleEntry* retrieve_handle (HandleMap *map, dds_instance_handle_t key)
{
  HandleEntry * entry = map->entries;

  while (entry)
  {
    if (entry->handle == key)
    {
      break;
    }
    entry = entry->next;
  }
  return entry;
}

static bool data_available_handler (dds_entity_t reader)
{
  int samples_received;
  dds_sample_info_t info [MAX_SAMPLES];
  int i;
  dds_time_t tnow = dds_time ();

  if (startTime == 0)
  {
    startTime = tnow;
  }

  /* Take samples and iterate through them */

  if (pollingDelay == -1) {
    samples_received = dds_read (reader, samples, info, MAX_SAMPLES, 0);
  } else {
    samples_received = dds_take (reader, samples, info, MAX_SAMPLES, 0);
  }
  DDS_ERR_CHECK (samples_received, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  for (i = 0; i < samples_received; i++)
  {
    if (info[i].valid_data)
    {
      ph = info[i].publication_handle;
      current = retrieve_handle (imap, ph);
      ThroughputModule_DataType * this_sample = &data[i];

      if (current == NULL)
      {
        current = store_handle (imap, ph);
        current->count = this_sample->count;
      }

      if (this_sample->count != current->count)
      {
        outOfOrder++;
      }
      current->count = this_sample->count + 1;

      /* Add the sample payload size to the total received */

      payloadSize = this_sample->payload._length;
      total_bytes += payloadSize + 8;
      total_samples++;
    }
  }

  return samples_received > 0;
}

static void data_available_handler_listener (dds_entity_t reader, void *arg)
{
    (void)arg;
    (void)data_available_handler (reader);
}

int main (int argc, char **argv)
{
  int status;
  unsigned long i;
  int result = EXIT_SUCCESS;
  unsigned long long maxCycles = 0;
  char *partitionName = "Throughput example";

  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t subscriber;
  dds_entity_t reader;
  dds_waitset_t waitSet;
  dds_condition_t rdcond = NULL;
  dds_listener_t rd_listener;

  unsigned long long prev_samples = 0;
  unsigned long long prev_bytes = 0;
  double deltaTime;
  dds_time_t printT = 0, lastPrintT = 0;

  setvbuf (stdout, (char *) NULL, _IOLBF, 0);

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Register handler for Ctrl-C */
#ifdef _WIN32
  SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, true);
#else
  struct sigaction sat;
  sat.sa_handler = CtrlHandler;
  sigemptyset(&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
#endif

  /*
   * Get the program parameters
   * Parameters: subscriber [maxCycles] [pollingDelay] [partitionName]
   */
  if (argc == 2 && (strcmp (argv[1], "-h") == 0 || strcmp (argv[1], "--help") == 0))
  {
    printf ("Usage (parameters must be supplied in order):\n");
    printf ("./subscriber [maxCycles (0 = infinite)] [pollingDelay (ms, 0: listener; -1: listener keep-last-1; <-1: waitset)] [partitionName]\n");
    printf ("Defaults:\n");
    printf ("./subscriber 0 0 \"Throughput example\"\n");
    return result;
  }

  if (argc > 1)
  {
    maxCycles = atoi (argv[1]); /* The number of times to output statistics before terminating */
  }
  if (argc > 2)
  {
    pollingDelay = atoi (argv[2]); /* The number of ms to wait between reads (0, -1: listener based, <-1: waitset) */
  }
  if (argc > 3)
  {
    partitionName = argv[3]; /* The name of the partition */
  }

  printf ("Cycles: %llu | PollingDelay: %ld | Partition: %s\n",
    maxCycles, pollingDelay, partitionName);

  /* Initialise entities */

  {
    const char *subParts[1];
    dds_qos_t *subQos = dds_qos_create ();
    dds_qos_t *drQos = dds_qos_create ();
    uint32_t maxSamples = 4000;
    dds_attach_t wsresults[2];
    size_t wsresultsize = 1U;
    int nreads = 0;

    /* A Participant is created for the default domain. */

    status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* A Topic is created for our sample type on the domain participant. */

    topic = dds_create_topic (participant, &ThroughputModule_DataType_desc, "Throughput", NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* A Subscriber is created on the domain participant. */

    subParts[0] = partitionName;
    dds_qset_partition (subQos, 1, subParts);
    status = dds_create_subscriber (participant, &subscriber, subQos, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete (subQos);

    /* A Reader is created on the Subscriber & Topic with a modified Qos. */

    dds_qset_reliability (drQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    if (pollingDelay != -1) {
      dds_qset_history (drQos, DDS_HISTORY_KEEP_ALL, 0);
    }
    dds_qset_resource_limits (drQos, maxSamples, DDS_LENGTH_UNLIMITED, DDS_LENGTH_UNLIMITED);

    memset (&rd_listener, 0, sizeof (rd_listener));
    rd_listener.on_data_available = data_available_handler_listener;

    reader = dds_create_reader (subscriber, topic, drQos, pollingDelay < -1 || pollingDelay > 0 ? NULL : &rd_listener);
    DDS_ERR_CHECK (reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete (drQos);

    /* A Read Condition is created which is triggered when data is available to read */

    waitSet = dds_waitset_create ();

    terminated = dds_guardcondition_create ();
    status = dds_waitset_attach (waitSet, terminated, terminated);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    if (pollingDelay < -1)
    {
      rdcond = dds_readcondition_create(reader, DDS_ANY_STATE);
      status = dds_waitset_attach (waitSet, rdcond, rdcond);
      DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    }

    /* Read samples until the maxCycles has been reached (0 = infinite) */

    imap = HandleMap__alloc ();

    memset (data, 0, sizeof (data));
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
      samples[i] = &data[i];
    }

    printf ("Waiting for samples...\n");

    while (!dds_condition_triggered (terminated) && (maxCycles == 0 || cycles < maxCycles))
    {
      if (pollingDelay == -1 || pollingDelay == 0)
      {
        status = dds_waitset_wait (waitSet, wsresults, wsresultsize, DDS_SECS (1));
      }
      else if (pollingDelay > 0)
      {
        dds_sleepfor (DDS_MSECS (pollingDelay));
        while (data_available_handler (reader)) { }
      }
      else
      {
        status = dds_waitset_wait (waitSet, wsresults, wsresultsize, DDS_SECS (1));
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
        for (int i = 0; i < status; i++)
        {
          if (wsresults[i] == rdcond)
          {
            data_available_handler(reader);
            break;
          }
        }
      }

      time_now = dds_time();
      if (time_now > printT) {
        if (lastPrintT) {
          /* FIXME: should do this atomically */
          unsigned long long ts = total_samples;
          unsigned long long tb = total_bytes;
          unsigned long long oo = outOfOrder;
          deltaTime = (time_now - lastPrintT) / 1e9;
          if (ts != prev_samples)
          {
            printf ("Payload size: %lu | Total received: %llu samples, %llu bytes | Out of order: %llu samples Transfer rate: %.2lf samples/s, %.2lf Mbit/s | nreads %d\n",
                    payloadSize, ts, tb, oo,
                    (ts - prev_samples) / deltaTime,
                    ((tb - prev_bytes) / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime,
                    nreads);
          }
          cycles++;

          /* Update the previous values for next iteration */

          prev_bytes = tb;
          prev_samples = ts;
        }
        lastPrintT = time_now;
        printT = time_now + DDS_SECS(1);
      }
    }

    /* Disable callbacks */

    dds_set_enabled_status (reader, 0);

    /* Output totals and averages */

    deltaTime = (double) (time_now - startTime) / 1e9;
    printf ("\nTotal received: %llu samples, %llu bytes\n", total_samples, total_bytes);
    printf ("Out of order: %llu samples\n", outOfOrder);
    printf ("Average transfer rate: %.2lf samples/s, ", total_samples / deltaTime);
    printf ("%.2lf Mbit/s\n", (total_bytes / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime);

    HandleMap__free (imap);
  }

  /* Clean up */

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    ThroughputModule_DataType_free (&data[i], DDS_FREE_CONTENTS);
  }

  status = dds_waitset_detach (waitSet, terminated);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_condition_delete (terminated);
  status = dds_waitset_delete (waitSet);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_delete (participant);
  dds_fini ();

#ifdef _WIN32
  SetConsoleCtrlHandler (0, FALSE);
#else
  sigaction (SIGINT, &oldAction, 0);
#endif
  return result;
}
