#include <assert.h>

#include "dds.h"
#include "os/os.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/theories.h>
#define MAX_SAMPLES 10

Test(vddsc_reader, create)
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

Test(vddsc_reader, read)
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
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

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

Test(vddsc_reader, read_with_loan)
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
  dds_return_t ret;
  int status, samplecount;

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

  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = NULL;
  }

  for (int j = 0;  j < MAX_SAMPLES; j++)
  {
    RoundTripModule_DataType valid_sample = { 0 };
    status = dds_write(writer, &valid_sample);
    cr_assert_eq(status, DDS_RETCODE_OK);
  }
  samplecount = dds_read_wl (reader, samples, info, MAX_SAMPLES);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);
  samplecount = dds_read_wl (reader, samples, info, MAX_SAMPLES);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_OLD);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_reader, read_mask)
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
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_NEW_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE;

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

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
  samplecount = dds_read_mask (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES, mask);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }
  samplecount = dds_read_mask (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES, mask);
  cr_assert_eq (samplecount, 0);
  mask = DDS_READ_SAMPLE_STATE | DDS_NOT_NEW_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE;
  samplecount = dds_read_mask (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES, mask);
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

Test(vddsc_reader, read_mask_with_loan)
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
  dds_return_t ret;
  int status, samplecount;
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_NEW_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE;

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = NULL;
  }

  for (int j = 0;  j < MAX_SAMPLES; j++)
  {
    RoundTripModule_DataType valid_sample = { 0 };
    status = dds_write(writer, &valid_sample);
    cr_assert_eq(status, DDS_RETCODE_OK);
  }
  samplecount = dds_read_mask_wl (reader, samples, info, MAX_SAMPLES, mask);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  samplecount = dds_read_mask_wl (reader, samples, info, MAX_SAMPLES, mask);
  cr_assert_eq (samplecount, 0);
  mask = DDS_READ_SAMPLE_STATE | DDS_NOT_NEW_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE;
  samplecount = dds_read_mask_wl (reader, samples, info, MAX_SAMPLES, mask);
  cr_assert_eq (samplecount, MAX_SAMPLES);
  for(int i = 0; i< samplecount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_OLD);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_reader, take)
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
  int status, samplesCount;
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
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

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
  samplesCount = dds_take (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
  cr_assert_eq (samplesCount, MAX_SAMPLES);
  for(int i = 0; i< samplesCount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }

  samplesCount = dds_take (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
  cr_assert_eq (samplesCount, 0);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_reader, take_with_loan)
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
  dds_return_t ret;
  int status, samplesCount;

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = NULL;
  }

  for (int j = 0;  j < MAX_SAMPLES; j++)
  {
    RoundTripModule_DataType valid_sample = { 0 };
    status = dds_write(writer, &valid_sample);
    cr_assert_eq(status, DDS_RETCODE_OK);
  }
  samplesCount = dds_take_wl (reader, samples, info, MAX_SAMPLES);
  cr_assert_eq (samplesCount, MAX_SAMPLES);
  for(int i = 0; i< samplesCount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  samplesCount = dds_take_wl (reader, samples, info, MAX_SAMPLES);
  cr_assert_eq (samplesCount, 0);
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_reader, take_mask)
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
  int status, samplesCount;
  RoundTripModule_DataType data[MAX_SAMPLES];
  uint32_t mask = DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

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
  samplesCount = dds_take_mask (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES, mask);
  cr_assert_eq (samplesCount, MAX_SAMPLES);
  for(int i = 0; i < samplesCount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }

  samplesCount = dds_take_mask (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES, mask);
  cr_assert_eq (samplesCount, 0);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_reader, take_mask_with_loan)
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
  int status, samplesCount;
  dds_return_t ret;
  uint32_t mask = DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;

  /* Create a reader */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();

  dds_qset_history(qos, DDS_HISTORY_KEEP_ALL, MAX_SAMPLES);
  reader = dds_create_reader (participant, topic, qos, NULL);

  /*Create a writer */
  publisher = dds_create_publisher (participant, qos, NULL);
  cr_assert_gt(publisher, 0, "publisher greater than 0");
  writer = dds_create_writer (publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0, "writer greater than 0");

  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = NULL;
  }

  for (int j = 0;  j < MAX_SAMPLES; j++)
  {
    RoundTripModule_DataType valid_sample = { 0 };
    status = dds_write(writer, &valid_sample);
    cr_assert_eq(status, DDS_RETCODE_OK);
  }
  samplesCount = dds_take_mask_wl (reader, samples, info, MAX_SAMPLES, mask);
  cr_assert_eq (samplesCount, MAX_SAMPLES);
  for(int i = 0; i < samplesCount; i++)
  {
    cr_assert_eq(info[i].valid_data, true);
    cr_assert_eq(info[i].instance_state, DDS_IST_ALIVE);
    cr_assert_eq(info[i].sample_state, DDS_SST_NOT_READ);
    cr_assert_eq(info[i].view_state, DDS_VST_NEW);
  }
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  samplesCount = dds_take_mask_wl (reader, samples, info, MAX_SAMPLES, mask);
  cr_assert_eq (samplesCount, 0);
  ret = dds_return_loan(reader, samples, MAX_SAMPLES);
  cr_assert_eq (ret, DDS_RETCODE_OK);

  dds_delete(writer);
  dds_delete(publisher);
  dds_delete (reader);
  dds_qos_delete (qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}


Test(vddsc_reader, wait_for_historical_data)
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t reader;
  dds_qos_t *qos;
  dds_return_t ret;
  dds_duration_t zeroSec = ((dds_duration_t)DDS_SECS(0));
  dds_duration_t oneSec = ((dds_duration_t)DDS_SECS(1));
  dds_duration_t minusOneSec = ((dds_duration_t)DDS_SECS(-1));
  dds_duration_t infinite = DDS_INFINITY;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  qos = dds_qos_create ();
  dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", qos, NULL);
  cr_assert_gt(topic, 0, "dds_create_topic(Roundtrip)");

  /* Call wait_for_historical_data with various reader arguments */
  ret = dds_wait_for_historical_data(0, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER, "dds_wait_for_historial_data(NULL, oneSec)");
  ret = dds_wait_for_historical_data(participant, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "dds_wait_for_historial_data(participant, oneSec)");

  /* Call wait_for_historical_data with a transient-local reader and various max_wait arguments */
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "transient-local reader created");
  ret = dds_wait_for_historical_data(reader, minusOneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER, "dds_wait_for_historial_data(reader, minusOneSec)");
  ret = dds_wait_for_historical_data(reader, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "dds_wait_for_historial_data(reader, oneSec)");
  ret = dds_wait_for_historical_data(reader, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "dds_wait_for_historial_data(reader, oneSec) again");
  ret = dds_wait_for_historical_data(reader, zeroSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "dds_wait_for_historial_data(reader, zeroSec)");
  ret = dds_wait_for_historical_data(reader, infinite);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "dds_wait_for_historial_data(reader, infinite)");
  dds_delete(reader);

  /* Call wait_for_historical_data on volatile, transient and persistent readers */
  dds_qset_durability(qos, DDS_DURABILITY_VOLATILE);
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "volatile reader created");
  ret = dds_wait_for_historical_data(reader, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_UNSUPPORTED, "dds_wait_for_historial_data(volatile_reader, oneSec)");
  dds_delete(reader);

  dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT);
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "transient reader created");
  ret = dds_wait_for_historical_data(reader, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_UNSUPPORTED, "dds_wait_for_historial_data(transient_reader, oneSec)");
  dds_delete(reader);

  dds_qset_durability(qos, DDS_DURABILITY_PERSISTENT);
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "persistent reader created");
  ret = dds_wait_for_historical_data(reader, oneSec);
  cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_UNSUPPORTED, "dds_wait_for_historial_data(persistent_reader, oneSec)");
  dds_delete(reader);

  dds_qos_delete (qos);
  dds_delete(reader);
  dds_delete(topic);
  dds_delete(participant);
}

static dds_entity_t g_participant = 0;
static void reader_env_init(void)
{
    /* We need a participant for initialization. */
    g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(g_participant, 0, "Failed to create prerequisite participant");
}
static void reader_env_fini(void)
{
    dds_delete(g_participant);
}

TheoryDataPoints(vddsc_reader, read_invalid_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t rdr), vddsc_reader, read_invalid_params, .init=reader_env_init, .fini=reader_env_fini)
{
    dds_entity_t exp = -DDS_RETCODE_BAD_PARAMETER;
    dds_sample_info_t info[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    dds_return_t ret;

    if (rdr < 0) {
        /* Entering the API with an error should return the same error. */
        exp = rdr;
    }

    ret = dds_read (rdr, samples, info, MAX_SAMPLES, MAX_SAMPLES);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
