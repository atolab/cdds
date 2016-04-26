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

int main (int argc, char **argv)
{
  unsigned long i;
  int status;
  int result = EXIT_SUCCESS;
  uint32_t payloadSize = 8192;
  unsigned int burstInterval = 0;
  unsigned int timeOut = 0;
  uint32_t maxSamples = 100;
  int burstSize = 1;
  bool timedOut = false;
  char * partitionName = "Throughput example";
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t publisher;
  dds_entity_t writer;
  dds_time_t pubStart;
  dds_time_t now;
  dds_time_t deltaTv;
  ThroughputModule_DataType sample;
  dds_publication_matched_status_t pms;
  const char *parts[1];
  dds_qos_t *pubQos;
  dds_qos_t *dwQos;
  int localReaders = 0;
  dds_entity_t rds[16];

  /* Register handler for Ctrl-C */

#ifdef _WIN32
  SetConsoleCtrlHandler ((PHANDLER_ROUTINE) CtrlHandler, true);
#else
  struct sigaction sat;
  sat.sa_handler = CtrlHandler;
  sigemptyset (&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
#endif

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /*
   * Get the program parameters
   * Parameters: publisher [payloadSize] [burstInterval] [burstSize] [timeOut] [partitionName]
   */
  if (argc == 2 && (strcmp (argv[1], "-h") == 0 || strcmp (argv[1], "--help") == 0))
  {
    printf ("Usage (parameters must be supplied in order):\n");
    printf ("./publisher [payloadSize (bytes)] [burstInterval (ms)] [burstSize (samples)] [timeOut (seconds)] [partitionName]\n");
    printf ("Defaults:\n");
    printf ("./publisher 8192 0 1 0 \"Throughput example\"\n");
    return result;
  }
  if (argc > 1)
  {
    payloadSize = atoi (argv[1]); /* The size of the payload in bytes */
  }
  if (argc > 2)
  {
    burstInterval = atoi (argv[2]); /* The time interval between each burst in ms */
  }
  if (argc > 3)
  {
    burstSize = atoi (argv[3]); /* The number of samples to send each burst */
  }
  if (argc > 4)
  {
    timeOut = atoi (argv[4]); /* The number of seconds the publisher should run for (0 = infinite) */
  }
  if (argc > 5)
  {
    partitionName = argv[5]; /* The name of the partition */
  }
  if (argc > 6)
  {
    localReaders = atoi(argv[6]);
    if (localReaders < 0 || localReaders > (int)(sizeof(rds)/sizeof(rds[0])))
    {
      printf("number of local readers out of range\n");
      return EXIT_FAILURE;
    }
  }

  printf ("payloadSize: %i bytes burstInterval: %u ms burstSize: %d timeOut: %u seconds partitionName: %s\n",
    payloadSize, burstInterval, burstSize, timeOut, partitionName);
  parts[0] = partitionName;

  /* A domain participant is created for the default domain. */

  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  terminated = dds_guardcondition_create ();

  /* A topic is created for our sample type on the domain participant. */

  status = dds_topic_create (participant, &topic, &ThroughputModule_DataType_desc, "Throughput", NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  if (localReaders)
  {
    dds_qos_t *qos = dds_qos_create ();
    dds_entity_t sub;
    dds_qset_partition (qos, 1, parts);
    status = dds_subscriber_create (participant, &sub, qos, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete (qos);
    qos = dds_qos_create ();
    for (i = 0; i < localReaders; i++)
    {
      status = dds_reader_create (sub, &rds[i], topic, qos, NULL);
      DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    }
    dds_qos_delete (qos);
  }
  
  /* A publisher is created on the domain participant. */

  pubQos = dds_qos_create ();
  dds_qset_partition (pubQos, 1, parts);
  status = dds_publisher_create (participant, &publisher, pubQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (pubQos);

  /* A DataWriter is created on the publisher. */

  dwQos = dds_qos_create ();
  dds_qset_reliability (dwQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  dds_qset_history (dwQos, DDS_HISTORY_KEEP_ALL, 0);
  dds_qset_resource_limits (dwQos, maxSamples, DDS_LENGTH_UNLIMITED, DDS_LENGTH_UNLIMITED);
  status = dds_writer_create (publisher, &writer, topic, dwQos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (dwQos);

  /* Enable write batching */

  dds_write_set_batch (true);

  /* Fill the sample payload with data */

  sample.count = 0;
  sample.payload._buffer = dds_alloc (payloadSize);
  sample.payload._length = payloadSize;
  sample.payload._release = true;
  for (i = 0; i < payloadSize; i++)
  {
    sample.payload._buffer[i] = 'a';
  }

  /* Wait until have a reader */

  pubStart = dds_time ();
  if (!localReaders)
  {
    do
    {
      dds_sleepfor (DDS_MSECS (100));
      status = dds_get_publication_matched_status (writer, &pms);
      if (timeOut)
      {
        now = dds_time ();
        deltaTv = now - pubStart;
        if ((deltaTv) > DDS_SECS (timeOut))
        {
          timedOut = true;
        }
      }
    }
    while (pms.total_count == 0 && !timedOut);
  }

  /* Register the sample instance and write samples repeatedly or until time out */
  {
    dds_time_t burstStart;
    int burstCount = 0;

    printf ("Writing samples...\n");
    burstStart = pubStart;

    while (!dds_condition_triggered (terminated) && !timedOut)
    {
      /* Write data until burst size has been reached */

      if (burstCount < burstSize)
      {
        status = dds_write (writer, &sample);
        if (dds_err_no (status) == DDS_RETCODE_TIMEOUT)
        {
          timedOut = true;
        }
        else
        {
          DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
          sample.count++;
          burstCount++;
        }
      }
      else if (burstInterval)
      {
        /* Sleep until burst interval has passed */

        dds_time_t time = dds_time ();
        deltaTv = time - burstStart;
        if (deltaTv < DDS_MSECS (burstInterval))
        {
          dds_write_flush (writer);
          dds_sleepfor (DDS_MSECS (burstInterval) - deltaTv);
        }
        burstStart = dds_time ();
        burstCount = 0;
      }
      else
      {
        burstCount = 0;
      }

      if (timeOut)
      {
        now = dds_time ();
        deltaTv = now - pubStart;
        if ((deltaTv) > DDS_SECS (timeOut))
        {
          timedOut = true;
        }
      }
    }
    dds_write_flush (writer);

    if (dds_condition_triggered (terminated))
    {
      printf ("Terminated, %llu samples written.\n", (long long) sample.count);
    }
    else
    {
      printf ("Timed out, %llu samples written.\n", (long long) sample.count);
    }
  }

  for (i = 0; i < localReaders; i++)
  {
    ThroughputModule_DataType s;
    void *ms[1] = { &s };
    dds_sample_info_t is[1];
    int n = dds_take (rds[i], ms, 1, is, 0);
    DDS_ERR_CHECK (n, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    if (n == 0)
      s.count = 0;
    if (s.count + 1 != sample.count)
      printf("reader: %llu, writer: %llu - huh?\n", (long long) s.count + 1, (long long) sample.count);
  }

  /* Cleanup */
#if 0
  status = dds_instance_dispose (writer, &sample);
  if (dds_err_no (status) != DDS_RETCODE_TIMEOUT)
  {
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  }
#endif
  dds_free (sample.payload._buffer);

  dds_condition_delete (terminated);
  dds_entity_delete (participant);
  dds_fini ();

#ifdef _WIN32
  SetConsoleCtrlHandler (0, false);
#else
  sigaction (SIGINT, &oldAction, 0);
#endif
  return result;
}
