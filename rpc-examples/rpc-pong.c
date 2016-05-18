#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RoundTrip.h"

#define MAX_SAMPLES 10

int main (int argc, char *argv[])
{
  dds_duration_t waitTimeout = DDS_INFINITY;
  unsigned int i;
  int status, samplecount, j;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_entity_t participant;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t topic;
  dds_entity_t publisher;
  dds_entity_t subscriber;
  dds_waitset_t waitSet;

  RoundTripModule_DataType data[MAX_SAMPLES];
  void * samples[MAX_SAMPLES];
  dds_sample_info_t info[MAX_SAMPLES];
  const char *pubPartitions[] = { "pong" };
  const char *subPartitions[] = { "ping" };
  dds_qos_t *qos;
  dds_condition_t readCond;

  /* Initialize sample data */

  memset (data, 0, sizeof (data));
  for (i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = &data[i];
  }

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* A DDS Topic is created for our sample type on the domain participant. */

  status = dds_topic_create
    (participant, &topic, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* A DDS Publisher is created on the domain participant. */

  qos = dds_qos_create ();
  dds_qset_partition (qos, 1, pubPartitions);
  status = dds_publisher_create (participant, &publisher, qos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (qos);

  /* A DDS Subscriber is created on the domain participant. */

  qos = dds_qos_create ();
  dds_qset_partition (qos, 1, subPartitions);
  status = dds_subscriber_create (participant, &subscriber, qos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (qos);

  /* A DDS DataReader is created on the Subscriber & Topic with a modified QoS. */

  qos = dds_qos_create ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  status = dds_reader_create (subscriber, &reader, topic, qos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (qos);

  printf("Let ping create reader, then press enter to continue ..."); fflush(stdout);getchar();

  /* A DDS DataWriter is created on the Publisher & Topic with a modififed Qos. */

  qos = dds_qos_create ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  dds_qset_writer_data_lifecycle (qos, false);
  status = dds_writer_create (publisher, &writer, topic, qos, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (qos);
  
  waitSet = dds_waitset_create ();
  readCond = dds_readcondition_create (reader, DDS_ANY_STATE);
  status = dds_waitset_attach (waitSet, readCond, reader);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  printf ("Waiting for samples from ping to send back...\n");
  fflush (stdout);

  while (true)
  {
    /* Wait for a sample from ping */

    status = dds_waitset_wait (waitSet, wsresults, wsresultsize, waitTimeout);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Take samples */
    samplecount = dds_take (reader, samples, MAX_SAMPLES, info, 0);
    DDS_ERR_CHECK (samplecount, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    for (j = 0; j < samplecount; j++)
    {
      if (info[j].valid_data)
      {
        /* If sample is valid, send it back to ping */

        RoundTripModule_DataType * valid_sample = &data[j];
        status = dds_write (writer, valid_sample);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
      }
    }
  }
#if 0
  /* Clean up */
  status = dds_waitset_detach (waitSet, readCond);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_condition_delete (readCond);
  status = dds_waitset_delete (waitSet);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_entity_delete (participant);

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    RoundTripModule_DataType_free (&data[i], DDS_FREE_CONTENTS);
  }

  dds_fini ();
  return 0;
#endif
}