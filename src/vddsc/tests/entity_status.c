#include "dds.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

Test(vddsc_entity, matched_status)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_publication_matched_status_t publication_matched;
  dds_subscription_matched_status_t subscription_matched;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create reader and writer with default QoS */
  reader = dds_create_reader(participant, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  /*Set reader and writer status enabled*/
  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Attach waitset and status conditions */
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for publication  matched status */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_publication_matched_status(writer, &publication_matched);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(publication_matched.current_count,1);
  }
  /* Wait for subscription  matched status */
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_subscription_matched_status(reader, &subscription_matched);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(subscription_matched.current_count, 1);
  }
  /* Clean up */
  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(writer);
  dds_delete(publisher);
  dds_delete(reader);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, pub_unmatched_status)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status =0;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_publication_matched_status_t publication_matched;
  dds_subscription_matched_status_t subscription_matched;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create reader and writer with default QoS */
  reader = dds_create_reader(participant, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  /*Set reader and writer status enabled*/
  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Create and attach waitset and status conditions */
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for subscription  matched status */
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_subscription_matched_status(reader, &subscription_matched);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(subscription_matched.current_count, 1);
  }

  /* Wait for publication  matched status */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    /* Reset the status */
    ret = dds_take_status (writer, &status, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(status, DDS_PUBLICATION_MATCHED_STATUS);

    /* Wait for the publication (un)matched status */
    ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    if(ret > 0){
      ret = dds_get_publication_matched_status(writer, &publication_matched);
      cr_assert_status_eq(ret, DDS_RETCODE_OK);
      cr_assert_eq(publication_matched.total_count_change,1);
      cr_assert_eq(publication_matched.current_count_change, 0);
    }
  }

  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(reader);

  /* Clean up */
  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(writer);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, sub_unmatch_status)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status =0;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_publication_matched_status_t publication_matched;
  dds_subscription_matched_status_t subscription_matched;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create reader and writer with default QoS */
  reader = dds_create_reader(participant, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  /*Set reader and writer status enabled*/
  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Create and attach waitset and status conditions */
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for publication  matched status */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_publication_matched_status(writer, &publication_matched);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(publication_matched.current_count, 1);
  }
  /* Wait for subscription  matched status */
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    /* Reset the status */
    ret = dds_take_status (reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);

    /* Reset process */
    ret = dds_waitset_detach(waitSetwr, writer);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    dds_delete(writer);

    /*Wait for the subscription (un)matched status */
    ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    if(ret > 0){
      ret = dds_get_subscription_matched_status(reader, &subscription_matched);
      cr_assert_status_eq(ret, DDS_RETCODE_OK);
      cr_assert_eq(subscription_matched.total_count_change, 1);
      cr_assert_eq(subscription_matched.current_count_change, 0);
    }
  }
  /* Clean up */
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_delete(reader);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, incompatible_qos)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);

  dds_requested_incompatible_qos_status_t req_incompatible_qos = {0};
  dds_offered_incompatible_qos_status_t off_incompatible_qos = {0};

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();
  dds_qset_durability (qos, DDS_DURABILITY_PERSISTENT);

  /* Create a reader with persistent durability */
  reader = dds_create_reader(participant, topic, qos, NULL);
  cr_assert_gt(reader, 0);

  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);

  /* Create writer with default QoS */
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  /* Get reader and writer status conditions and attach to waitset */
  ret = dds_set_enabled_status(reader, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(writer, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait subscription requested incompatible status */
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_requested_incompatible_qos_status (reader, &req_incompatible_qos);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_neq(req_incompatible_qos.total_count, 0);
  }
  /* Wait for offered incompatible QoS status */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_offered_incompatible_qos_status (writer, &off_incompatible_qos);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_neq(off_incompatible_qos.total_count, 0);
  }

  /* Clean up */
  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(writer);
  dds_delete(publisher);
  dds_delete(reader);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, liveliness_changed)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status;
  uint32_t set_status = 0;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_publication_matched_status_t publication_matched;
  dds_liveliness_changed_status_t liveliness_changed;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create reader and writer with default QoS */
  reader = dds_create_reader(participant, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  /*Set reader and writer status enabled*/
  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_LIVELINESS_CHANGED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Get the status set */
  ret = dds_get_enabled_status (reader, &set_status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(set_status, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_LIVELINESS_CHANGED_STATUS);

  /* Attach waitset and status conditions */
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for publication  and subscription matched status */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  status = dds_get_publication_matched_status(writer, &publication_matched);
  cr_assert_status_eq(status, DDS_RETCODE_OK);
  cr_assert_eq(publication_matched.current_count, 1);

  /* Check status */
  ret = dds_take_status(reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);

  if(status & DDS_SUBSCRIPTION_MATCHED_STATUS){
    /* wait for LIVELINESS_CHANGED status */
    ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    if(ret > 0){
      ret = dds_get_liveliness_changed_status (reader, &liveliness_changed);
      cr_assert_status_eq(ret, DDS_RETCODE_OK);
      if(liveliness_changed.alive_count == 1){
        /* Reset writer */
        ret = dds_waitset_detach(waitSetwr, writer);
        cr_assert_status_eq(ret, DDS_RETCODE_OK);
        dds_delete(writer);

        /* wait for LIVELINESS_CHANGED when a writer is deleted */
        ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
        if(ret > 0){
          ret = dds_get_liveliness_changed_status (reader, &liveliness_changed);
          cr_assert_status_eq(ret, DDS_RETCODE_OK);
          cr_assert_eq(liveliness_changed.not_alive_count, 1);
        }
      }
    }
  }

  /* Clean up */
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_delete(reader);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, sample_rejected)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_sample_rejected_status_t sample_rejected = {0};
  dds_resource_limits_qospolicy_t resource_limits = {1,1,1};

  /* Topic instance */
  RoundTripModule_DataType sample = { 0 };

  /* Create QoS with limited resources */
  qos = dds_qos_create();
  dds_qset_resource_limits (qos, resource_limits.max_samples, resource_limits.max_instances, resource_limits.max_samples_per_instance);
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS (10));
  dds_qset_history (qos, DDS_HISTORY_KEEP_ALL, 0);

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);
  cr_assert_gt(participant, 0);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);

  /* Create writer with default QoS and reader with QoS */
  reader = dds_create_reader(participant, topic, qos, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_SAMPLE_REJECTED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for subscription matched and publication matched */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  /* Reset the publication matched status */
  ret = dds_get_publication_matched_status(writer, NULL);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  /* write data - write more than resource limits set by a data reader */
  for (int i = 0; i < 5; i++)
  {
    ret = dds_write (writer, &sample);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
  }

  /* Read status and reset */
  ret = dds_take_status(reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  if(status & DDS_SUBSCRIPTION_MATCHED_STATUS){
    /* wait for sample rejected status */
    ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    if(ret > 0)
    {
      ret = dds_get_sample_rejected_status (reader, &sample_rejected);
      cr_assert_status_eq(ret, DDS_RETCODE_OK);
      cr_assert_eq(sample_rejected.last_reason, DDS_REJECTED_BY_SAMPLES_LIMIT);
    }
  }
  /* Delete reader */
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(reader);

  /* Wait for reader to be deleted */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  /* Clean Up */

  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(writer);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, sample_lost)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t time1;
  dds_sample_lost_status_t sample_lost = {0};
  dds_time_t waitTimeout = DDS_SECS (2);

  /* Topic instance */
  RoundTripModule_DataType sample = { 0 };

  /* Create QoS with limited resources */
  qos = dds_qos_create();
  dds_qset_destination_order (qos, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);

  /* Create writer with default QoS and reader with QoS */
  reader = dds_create_reader(participant, topic, qos, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, qos, NULL);
  cr_assert_gt(writer, 0);

  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_SAMPLE_LOST_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for subscription matched and publication matched */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  /* Reset the publication matched status */
  ret = dds_get_publication_matched_status(writer, NULL);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Read status and reset */
  ret = dds_take_status(reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);

  /* get current time - subtraction ensures that this is truly historic on all platforms. */
  time1 = dds_time () - 1000000;

  /* write a sample with current timestamp */
  ret = dds_write_ts (writer, &sample, dds_time ());
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* second sample with older timestamp */
  ret = dds_write_ts (writer, &sample, time1);

  if(status & DDS_SUBSCRIPTION_MATCHED_STATUS){
    /* wait for sample lost status */
    ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    if(ret > 0)
    {
      ret = dds_get_sample_lost_status (reader, &sample_lost);
      cr_assert_status_eq(ret, DDS_RETCODE_OK);
      cr_assert_gt(sample_lost.total_count, 0);
    }
  }

  /* Delete reader */
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(reader);

  /* Wait for reader to be deleted */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  /* Clean Up */

  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(writer);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}
#if 0
/* This is basically the same as the Lite test, but inconsistent topic is not triggered.
 * That is actually what I would expect, because the code doesn't seem to be the way
 * to go to test for inconsistent topic. */
Test(vddsc_entity, inconsistent_topic)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic_wr, topic_rd;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_inconsistent_topic_status_t topic_status = {0};

  /*Create topics */
  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic_wr = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic_wr, 0);
  topic_rd = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip2", NULL, listener);
  cr_assert_gt(topic_rd, 0, "fails %d", dds_err_nr(topic_rd));
  qos = dds_qos_create();

  /* Create writer and reader with default QoS */
  reader = dds_create_reader(participant, topic_rd, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic_wr, NULL, NULL);
  cr_assert_gt(writer, 0);

  /*Set reader topic and writer topic statuses enabled*/
  ret = dds_set_enabled_status(topic_wr, DDS_INCONSISTENT_TOPIC_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(topic_rd, DDS_INCONSISTENT_TOPIC_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for pub inconsistent topic status callback */
  ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_inconsistent_topic_status (topic_wr, &topic_status);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_gt(topic_status.total_count, 0);
  }

  /* Wait for sub inconsistent topic status callback */
  ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
  if(ret > 0){
    ret = dds_get_inconsistent_topic_status (topic_rd, &topic_status);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_gt(topic_status.total_count, 0);
  }

  /* Clean up */
  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(reader);
  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(writer);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic_rd);
  dds_delete(topic_wr);
  dds_listener_delete(listener);
  dds_delete(participant);
}
#endif

Test(vddsc_entity, data_available)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader;
  dds_entity_t writer;
  dds_entity_t publisher;
  dds_return_t ret, status;
  dds_entity_t waitSetwr, waitSetrd;
  dds_attach_t p_wsresults[1];
  size_t p_wsresultsize = 1U;
  dds_attach_t s_wsresults[2];
  size_t s_wsresultsize = 2U;
  dds_time_t time1;
  dds_time_t waitTimeout = DDS_SECS (2);

  RoundTripModule_DataType sample = { 0 };

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create writer and reader with default QoS*/
  reader = dds_create_reader(participant, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for subscription and publication matched status */
  ret = dds_waitset_wait(waitSetwr, p_wsresults, p_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  ret = dds_waitset_wait(waitSetrd, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  /* Write the sample */
  ret = dds_write (writer, &sample);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* wait for data available */
  ret = dds_take_status(reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);

  ret = dds_waitset_wait(waitSetrd, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  ret = dds_get_status_changes (reader, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  dds_delete(reader);

  /* Wait for reader to be deleted */
  ret = dds_waitset_wait(waitSetwr, p_wsresults, p_wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  /* Clean Up */

  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(writer);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, all_data_available)
{
  dds_entity_t participant;
  dds_listener_t *listener;
  dds_entity_t topic;
  dds_qos_t *qos;
  dds_entity_t reader, reader2;
  dds_entity_t writer;
  dds_entity_t subscriber;
  dds_entity_t publisher;
  dds_return_t ret, status;
  dds_entity_t waitSetwr, waitSetrd, waitSetrd2;
  dds_attach_t p_wsresults[1];
  size_t p_wsresultsize = 1U;
  dds_attach_t s_wsresults[2];
  size_t s_wsresultsize = 2U;
  dds_time_t waitTimeout = DDS_SECS (2);
  dds_sample_info_t info;

  /* Topic instance */
  RoundTripModule_DataType p_sample = { 0 };
  void * s_samples[1];
  RoundTripModule_DataType s_sample;

  memset (&s_sample, 0, sizeof (s_sample));
  s_samples[0] = &s_sample;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL);
  topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0);
  qos = dds_qos_create();

  /* Create writer, subscriber and 2 readers with default QoS */
  subscriber = dds_create_subscriber(participant, qos, NULL);
  cr_assert_gt(subscriber, 0);
  reader = dds_create_reader(subscriber, topic, NULL, NULL);
  cr_assert_gt(reader, 0);
  reader2 = dds_create_reader(subscriber, topic, NULL, NULL);
  cr_assert_gt(reader2, 0);
  publisher = dds_create_publisher(participant, qos, NULL);
  cr_assert_gt(publisher, 0);
  writer = dds_create_writer(publisher, topic, NULL, NULL);
  cr_assert_gt(writer, 0);

  ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_set_enabled_status(reader2, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  waitSetwr = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  waitSetrd2 = dds_create_waitset(participant);
  ret = dds_waitset_attach (waitSetrd2, reader2, (dds_attach_t)(intptr_t)reader2);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Wait for publication matched status */
  ret = dds_waitset_wait(waitSetwr, p_wsresults, p_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  /* Wait for subscription matched status on both readers */
  ret = dds_waitset_wait(waitSetrd, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);
  ret = dds_waitset_wait(waitSetrd2, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_gt(ret, 0);

  ret = dds_write (writer, &p_sample);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Reset the publication and subscription matched status */
  ret = dds_get_publication_matched_status(writer, NULL);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_take_status (reader, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  ret = dds_take_status (reader2, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_SUBSCRIPTION_MATCHED_STATUS);

  /* wait for data */
  ret = dds_waitset_wait(waitSetrd, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  ret = dds_waitset_wait(waitSetrd2, s_wsresults, s_wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  ret = dds_waitset_detach(waitSetrd, reader);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_waitset_detach(waitSetrd2, reader2);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  ret = dds_delete(waitSetrd2);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);

  /* Get DATA_ON_READERS status*/
  ret = dds_get_status_changes (subscriber, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_DATA_ON_READERS_STATUS);

  /* Get DATA_AVAILABLE status */
  ret = dds_get_status_changes (reader, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_DATA_AVAILABLE_STATUS);

  /* Get DATA_AVAILABLE status */
  ret = dds_get_status_changes (reader2, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, DDS_DATA_AVAILABLE_STATUS);

  /* Read 1 data sample from reader1 */
  ret = dds_take (reader, s_samples, &info, 1, 1);
  cr_assert_eq(ret, 1);

  /* status after taking the data should be reset */
  ret = dds_get_status_changes (reader, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_neq(status, ~DDS_DATA_AVAILABLE_STATUS);

  /* status from reader2 */
  ret = dds_get_status_changes (reader2, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_neq(status, ~DDS_DATA_AVAILABLE_STATUS);

  /* status from subscriber */
  ret = dds_get_status_changes (subscriber, &status);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  cr_assert_eq(status, 0);

  RoundTripModule_DataType_free (&s_sample, DDS_FREE_CONTENTS);

  dds_delete(reader);
  dds_delete(reader2);
  dds_delete(subscriber);

  /* Wait for reader to be deleted */
  ret = dds_waitset_wait(waitSetwr, p_wsresults, p_wsresultsize, waitTimeout);
  cr_assert_neq(ret, 0);

  ret = dds_waitset_detach(waitSetwr, writer);
  cr_assert_status_eq(ret, DDS_RETCODE_OK);
  dds_delete(writer);

  dds_delete(waitSetrd);
  dds_delete(waitSetwr);
  dds_delete(publisher);
  dds_qos_delete(qos);
  dds_delete(topic);
  dds_listener_delete(listener);
  dds_delete(participant);
}

Test(vddsc_entity, get_enabled_status_invalid_params)
{
  dds_return_t status;
  uint32_t s = 0;

  status = dds_get_enabled_status(-1, &s);
  cr_assert_eq(status, -1);
  status = dds_get_enabled_status(0, &s);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_enabled_status(0, NULL)");
  status = dds_get_enabled_status(1, &s);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_entity, set_enabled_status_invalid_params)
{
  dds_return_t status;

  status = dds_set_enabled_status(-1, DDS_DATA_AVAILABLE_STATUS);
  cr_assert_eq(status, -1);
  status = dds_set_enabled_status(0, DDS_DATA_AVAILABLE_STATUS);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_set_enabled_status(0, DDS_DATA_AVAILABLE_STATUS)");
  status = dds_set_enabled_status(1, DDS_DATA_AVAILABLE_STATUS);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_entity, read_status_invalid_params)
{
  dds_return_t status;
  uint32_t s = 0;

  status = dds_read_status (-1, &s, 0);
  cr_assert_eq(status, -1);
  status = dds_read_status (0, &s, 0);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_read_status(0, &s, 0)");
  status = dds_read_status (1, &s, 0);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_entity, take_status_invalid_params)
{
  dds_return_t status;
  uint32_t s = 0;

  status = dds_take_status (-1, &s, 0);
  cr_assert_eq(status, -1);
  status = dds_take_status (0, &s, 0);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_read_status(0, &s, 0)");
  status = dds_take_status (1, &s, 0);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);

}

Test(vddsc_entity, get_status_changes_invalid_params)
{
  dds_return_t status;
  uint32_t s = 0;

  status = dds_get_status_changes (-1, &s);
  cr_assert_eq(status, -1);
  status = dds_get_status_changes (0, &s);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_get_status_changes(0, 0)");
  status = dds_get_status_changes (1, &s);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_entity, triggered_invalid_params)
{
  dds_return_t status;

  status = dds_triggered (-1);
  cr_assert_eq(status, -1);
  status = dds_triggered (0);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER, "dds_triggered(0)");
  //cr_assert_eq(ret, 0, "dds_triggered: Invalid return code %d", dds_err_nr(ret));
  status = dds_triggered (1);
  cr_assert_status_eq(status, DDS_RETCODE_BAD_PARAMETER);
}

