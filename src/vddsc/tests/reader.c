#include "dds.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "os/os.h"

Test(vddsc, reader_creation)
{
  dds_entity_t participant;
  dds_entity_t reader, reader2, reader3;
  dds_entity_t subscriber;
  dds_entity_t topic, topic2;
  dds_listener_t *listener;
  dds_qos_t *qos, *qos2;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);

  /*Creating reader with participant (and with qos) */

  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "dds_create_reader() 1st");
  dds_delete(reader);

  /* Creating reader with subscriber (and with qos)*/
  qos2 = dds_qos_create ();
  subscriber = dds_create_subscriber (participant, qos2, NULL);
  reader2 = dds_create_reader (subscriber, topic, qos2, NULL);
  cr_assert_gt(reader2, 0, "dds_create_reader() 2nd");
  dds_delete(reader2);
  dds_delete(topic);


  /*Creating reader with participant (and without qos) */

  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  reader3 = dds_create_reader (participant, topic2, NULL, NULL);
  cr_assert_gt(reader3, 0, "dds_create_reader() 3rd");
  dds_delete(reader3);
  dds_delete(topic2);

  dds_qos_delete(qos2);
  dds_qos_delete(qos);
  dds_listener_delete(listener);
  dds_delete(subscriber);
  dds_delete(participant);
}
#define MAX_SAMPLES 10
Test(vddsc, reader_read)
{
  int sample_received = 0;
  dds_entity_t reader;
  void * samples[MAX_SAMPLES];
  dds_sample_info_t info[MAX_SAMPLES];
  dds_entity_t participant;
  dds_entity_t topic;
  dds_listener_t *listener;
  dds_qos_t *qos;
  dds_entity_t publisher;
  dds_entity_t writer;
  int status, samplecount;
  RoundTripModule_DataType data[MAX_SAMPLES];

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  writer = dds_create_writer (publisher, topic, NULL, NULL);

  memset (data, 0, sizeof (data));
  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = &data[i];
  }

  for (int j = 0;  j < MAX_SAMPLES; j++)
  {
      RoundTripModule_DataType * valid_sample = &data[j];
      status = dds_write(writer, valid_sample);
      cr_assert_eq(status, DDS_RETCODE_OK);
  }
  samplecount = dds_read (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }

  samplecount = dds_read (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_OLD);
  }

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}
