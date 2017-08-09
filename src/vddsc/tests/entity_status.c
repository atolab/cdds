#include <stdlib.h>
#include "dds.h"
#include "os/os.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/theories.h>

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

/****************************************************************************
 * Test globals.
 ****************************************************************************/
static dds_entity_t    participant;
static dds_entity_t    subscriber;
static dds_entity_t    publisher;
static dds_entity_t    topic;
static dds_entity_t    writer;
static dds_entity_t    reader;
static dds_entity_t    waitSetwr;
static dds_entity_t    waitSetrd;
static dds_return_t    status, status2 = 0;

static dds_qos_t      *qos;
static dds_attach_t wsresults[1];
static dds_attach_t wsresults2[2];
static size_t wsresultsize = 1U;
static size_t wsresultsize2 = 2U;
static dds_time_t waitTimeout = DDS_SECS (2);
static dds_time_t shortTimeout = DDS_MSECS (10);
static dds_publication_matched_status_t publication_matched;
static dds_subscription_matched_status_t subscription_matched;
static dds_resource_limits_qospolicy_t resource_limits = {1,1,1};

static dds_instance_handle_t reader_i_hdl = 0;
static dds_instance_handle_t writer_i_hdl = 0;

/****************************************************************************
 * Test initializations and teardowns.
 ****************************************************************************/
static char*
create_topic_name(const char *prefix, char *name, size_t size)
{
    /* Get semi random g_topic name. */
    os_procId pid = os_procIdSelf();
    uintmax_t tid = os_threadIdToInteger(os_threadIdSelf());
    snprintf(name, size, "%s_pid%"PRIprocId"_tid%"PRIuMAX"", prefix, pid, tid);
    return name;
}

static void
init_entity_status(void)
{
    dds_return_t ret;
    char topicName[100];

    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0, "Failed to create prerequisite participant");

    topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, create_topic_name("vddsc_status_test", topicName, 100), NULL, NULL);
    cr_assert_gt(topic, 0, "Failed to create prerequisite topic");

    qos = dds_qos_create();
    cr_assert_not_null(qos, "Failed to create prerequisite qos");
    dds_qset_resource_limits (qos, resource_limits.max_samples, resource_limits.max_instances, resource_limits.max_samples_per_instance);
    dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, DDS_MSECS(100));
    dds_qset_history (qos, DDS_HISTORY_KEEP_ALL, 0);
    dds_qset_destination_order (qos, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);

    subscriber = dds_create_subscriber(participant, qos, NULL);
    cr_assert_gt(subscriber, 0);
    reader = dds_create_reader(subscriber, topic, qos, NULL);
    cr_assert_gt(reader, 0);
    publisher = dds_create_publisher(participant, qos, NULL);
    cr_assert_gt(publisher, 0);
    writer = dds_create_writer(publisher, topic, qos, NULL);
    cr_assert_gt(writer, 0);

    waitSetwr = dds_create_waitset(participant);
    status = dds_waitset_attach (waitSetwr, writer, (dds_attach_t)(intptr_t)writer);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    waitSetrd = dds_create_waitset(participant);
    status = dds_waitset_attach (waitSetrd, reader, (dds_attach_t)(intptr_t)reader);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Get reader/writer handles because they can be tested against. */
    ret = dds_instancehandle_get(reader, &reader_i_hdl);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to get prerequisite reader_i_hdl");
    ret = dds_instancehandle_get(writer, &writer_i_hdl);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to get prerequisite writer_i_hdl");
}

static void
fini_entity_status(void)
{
    dds_waitset_detach(waitSetrd, reader);
    dds_waitset_detach(waitSetwr, writer);

    dds_delete(waitSetrd);
    dds_delete(waitSetwr);
    dds_delete(writer);
    dds_delete(publisher);
    dds_delete(reader);
    dds_qos_delete(qos);
    dds_delete(topic);
    dds_delete(subscriber);
    dds_delete(participant);
}

/****************************************************************************
 * Triggering tests
 ****************************************************************************/
Test(vddsc_entity_status, publication_matched, .init=init_entity_status, .fini=fini_entity_status)
{
    /* We're interested in publication matched status. */
    status = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for publication matched status */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_publication_matched_status(writer, &publication_matched);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(publication_matched.current_count,        1);
    cr_assert_eq(publication_matched.current_count_change, 1);
    cr_assert_eq(publication_matched.total_count,          1);
    cr_assert_eq(publication_matched.total_count_change,   1);
    cr_assert_eq(publication_matched.last_subscription_handle, reader_i_hdl);

    /* Getting the status should have reset the trigger,
     * meaning that the wait should timeout. */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    /* Un-match the publication by deleting the reader. */
    dds_delete(reader);

    /* Wait for publication matched status */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_publication_matched_status(writer, &publication_matched);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(publication_matched.current_count,         0);
    cr_assert_eq(publication_matched.current_count_change, -1);
    cr_assert_eq(publication_matched.total_count,           1);
    cr_assert_eq(publication_matched.total_count_change,    0);
    cr_assert_eq(publication_matched.last_subscription_handle, reader_i_hdl);
}

Test(vddsc_entity_status, subscription_matched, .init=init_entity_status, .fini=fini_entity_status)
{
    /* We're interested in subscription matched status. */
    status = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for subscription  matched status */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_subscription_matched_status(reader, &subscription_matched);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(subscription_matched.current_count,        1);
    cr_assert_eq(subscription_matched.current_count_change, 1);
    cr_assert_eq(subscription_matched.total_count,          1);
    cr_assert_eq(subscription_matched.total_count_change,   1);
    cr_assert_eq(subscription_matched.last_publication_handle, writer_i_hdl);

    /* Getting the status should have reset the trigger,
     * meaning that the wait should timeout. */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    /* Un-match the subscription by deleting the writer. */
    dds_delete(writer);

    /* Wait for subscription  matched status */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_subscription_matched_status(reader, &subscription_matched);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(subscription_matched.current_count,         0);
    cr_assert_eq(subscription_matched.current_count_change, -1);
    cr_assert_eq(subscription_matched.total_count,           1);
    cr_assert_eq(subscription_matched.total_count_change,    0);
    cr_assert_eq(subscription_matched.last_publication_handle, writer_i_hdl);
}

Test(vddsc_entity, incompatible_qos, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_entity_t reader2;
    dds_requested_incompatible_qos_status_t req_incompatible_qos = {0};
    dds_offered_incompatible_qos_status_t off_incompatible_qos = {0};
    dds_qset_durability (qos, DDS_DURABILITY_PERSISTENT);

    /* Create a reader with persistent durability */
    reader2 = dds_create_reader(participant, topic, qos, NULL);
    cr_assert_gt(reader2, 0);
    status = dds_waitset_attach (waitSetrd, reader2, (dds_attach_t)(intptr_t)reader2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Get reader and writer status conditions and attach to waitset */
    status = dds_set_enabled_status(reader, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader2, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(writer, DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for subscription requested incompatible status, which should only be
     * triggered on reader2. */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, 1);
    cr_assert_eq(reader2,  (dds_entity_t)(intptr_t)wsresults[0]);

    /* Get and check the status. */
    status = dds_get_requested_incompatible_qos_status (reader2, &req_incompatible_qos);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(req_incompatible_qos.total_count,           1);
    cr_assert_eq(req_incompatible_qos.total_count_change,    1);
    cr_assert_eq(req_incompatible_qos.last_policy_id, DDS_DURABILITY_QOS_POLICY_ID);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    /* Wait for offered incompatible QoS status */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_offered_incompatible_qos_status (writer, &off_incompatible_qos);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(off_incompatible_qos.total_count,           1);
    cr_assert_eq(off_incompatible_qos.total_count_change,    1);
    cr_assert_eq(off_incompatible_qos.last_policy_id, DDS_DURABILITY_QOS_POLICY_ID);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    status = dds_waitset_detach(waitSetrd, reader2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    dds_delete(reader2);
}

Test(vddsc_entity, liveliness_changed, .init=init_entity_status, .fini=fini_entity_status)
{
    uint32_t set_status = 0;
    dds_liveliness_changed_status_t liveliness_changed;

    status = dds_set_enabled_status(reader, DDS_LIVELINESS_CHANGED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Get the status set */
    status = dds_get_enabled_status (reader, &set_status);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(set_status, DDS_LIVELINESS_CHANGED_STATUS);

    /* wait for LIVELINESS_CHANGED status */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_get_liveliness_changed_status (reader, &liveliness_changed);
    cr_assert_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(liveliness_changed.alive_count,           1);
    cr_assert_eq(liveliness_changed.alive_count_change,    1);
    cr_assert_eq(liveliness_changed.not_alive_count,       0);
    cr_assert_eq(liveliness_changed.not_alive_count_change,0);
    cr_assert_eq(liveliness_changed.last_publication_handle, writer_i_hdl);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    /* Reset writer */
    status = dds_waitset_detach(waitSetwr, writer);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    dds_delete(writer);

    /* wait for LIVELINESS_CHANGED when a writer is deleted */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_get_liveliness_changed_status (reader, &liveliness_changed);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(liveliness_changed.alive_count,           0);
    cr_assert_eq(liveliness_changed.alive_count_change,    0);
    cr_assert_eq(liveliness_changed.not_alive_count,       1);
    cr_assert_eq(liveliness_changed.not_alive_count_change,1);
    cr_assert_eq(liveliness_changed.last_publication_handle, writer_i_hdl);
}

Test(vddsc_entity, sample_rejected, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_sample_rejected_status_t sample_rejected = {0};

    /* Topic instance */
    RoundTripModule_DataType sample = { 0 };

    status = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_SAMPLE_REJECTED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for subscription matched and publication matched */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_set_enabled_status(reader, DDS_SAMPLE_REJECTED_STATUS);
    cr_assert_eq(status, DDS_RETCODE_OK);

    /* write data - write more than resource limits set by a data reader */
    for (int i = 0; i < 5; i++)
    {
      status = dds_write (writer, &sample);
      cr_assert_status_eq(status, DDS_RETCODE_OK);
    }

    /* wait for sample rejected status */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_sample_rejected_status (reader, &sample_rejected);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(sample_rejected.total_count,        4);
    cr_assert_eq(sample_rejected.total_count_change, 4);
    cr_assert_eq(sample_rejected.last_reason, DDS_REJECTED_BY_SAMPLES_LIMIT);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));
}

#if 0
/* This is basically the same as the Lite test, but inconsistent topic is not triggered.
 * That is actually what I would expect, because the code doesn't seem to be the way
 * to go to test for inconsistent topic. */
Test(vddsc_entity, inconsistent_topic)
{
    dds_inconsistent_topic_status_t topic_status = {0};

    topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip1", NULL, NULL);
    cr_assert_gt(topic, 0, "fails %d", dds_err_nr(topic));

    /*Set reader topic and writer topic statuses enabled*/
    ret = dds_set_enabled_status(topic, DDS_INCONSISTENT_TOPIC_STATUS);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    ret = dds_set_enabled_status(topic, DDS_INCONSISTENT_TOPIC_STATUS);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);

    /* Wait for pub inconsistent topic status callback */
    ret = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(ret, wsresultsize);
    ret = dds_get_inconsistent_topic_status (topic, &topic_status);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_gt(topic_status.total_count, 0);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    /* Wait for sub inconsistent topic status callback */
    ret = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    ret = dds_get_inconsistent_topic_status (topic, &topic_status);
    cr_assert_status_eq(ret, DDS_RETCODE_OK);
    cr_assert_gt(topic_status.total_count, 0);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

    dds_delete(topic);
}
#endif

Test(vddsc_entity, sample_lost, .init=init_entity_status, .fini=fini_entity_status)
{

    dds_sample_lost_status_t sample_lost = {0};
    dds_time_t time1;
    /* Topic instance */
    RoundTripModule_DataType sample = { 0 };

    status = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_SAMPLE_LOST_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for subscription matched and publication matched */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_set_enabled_status(reader, DDS_SAMPLE_LOST_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* get current time - subtraction ensures that this is truly historic on all platforms. */
    time1 = dds_time () - 1000000;

    /* write a sample with current timestamp */
    status = dds_write_ts (writer, &sample, dds_time ());
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* second sample with older timestamp */
    status = dds_write_ts (writer, &sample, time1);

    /* wait for sample lost status */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_get_sample_lost_status (reader, &sample_lost);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(sample_lost.total_count,          1);
    cr_assert_eq(sample_lost.total_count_change, 1);

    /*Getting the status should have reset the trigger, waitset should timeout */
    status = dds_waitset_wait(waitSetrd, wsresults, wsresultsize, shortTimeout);
    cr_assert_eq(dds_err_nr(status), 0, "returned %d", dds_err_nr(status));

}

Test(vddsc_entity, data_available, .init=init_entity_status, .fini=fini_entity_status)
{
    RoundTripModule_DataType sample = { 0 };

    status = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for subscription and publication matched status */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_waitset_wait(waitSetrd, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    /* Write the sample */
    status = dds_write (writer, &sample);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* wait for data available */
    status = dds_take_status(reader, &status2, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_SUBSCRIPTION_MATCHED_STATUS);

    status = dds_waitset_wait(waitSetrd, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_get_status_changes (reader, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    status = dds_waitset_detach(waitSetrd, reader);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    dds_delete(reader);

    /* Wait for reader to be deleted */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_neq(status, 0);
}

Test(vddsc_entity, all_data_available, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_entity_t reader2;
    dds_entity_t waitSetrd2;
    dds_sample_info_t info;

    /* Topic instance */
    RoundTripModule_DataType p_sample = { 0 };
    void * s_samples[1];
    RoundTripModule_DataType s_sample;

    memset (&s_sample, 0, sizeof (s_sample));
    s_samples[0] = &s_sample;

    reader2 = dds_create_reader(subscriber, topic, NULL, NULL);
    cr_assert_gt(reader2, 0);

    status = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status(reader2, DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    waitSetrd2 = dds_create_waitset(participant);
    status = dds_waitset_attach (waitSetrd2, reader2, (dds_attach_t)(intptr_t)reader2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Wait for publication matched status */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    /* Wait for subscription matched status on both readers */
    status = dds_waitset_wait(waitSetrd, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_eq(status, wsresultsize);
    status = dds_waitset_wait(waitSetrd2, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_eq(status, wsresultsize);

    status = dds_write (writer, &p_sample);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Reset the publication and subscription matched status */
    status = dds_get_publication_matched_status(writer, NULL);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_take_status (reader, &status2, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_SUBSCRIPTION_MATCHED_STATUS);
    status = dds_take_status (reader2, &status2, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_SUBSCRIPTION_MATCHED_STATUS);

    /* wait for data */
    status = dds_waitset_wait(waitSetrd, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_neq(status, 0);

    status = dds_waitset_wait(waitSetrd2, wsresults2, wsresultsize2, waitTimeout);
    cr_assert_neq(status, 0);

    status = dds_waitset_detach(waitSetrd, reader);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_waitset_detach(waitSetrd2, reader2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    status = dds_delete(waitSetrd2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);

    /* Get DATA_ON_READERS status*/
    status = dds_get_status_changes (subscriber, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_DATA_ON_READERS_STATUS);

    /* Get DATA_AVAILABLE status */
    status = dds_get_status_changes (reader, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_DATA_AVAILABLE_STATUS);

    /* Get DATA_AVAILABLE status */
    status = dds_get_status_changes (reader2, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, DDS_DATA_AVAILABLE_STATUS);

    /* Read 1 data sample from reader1 */
    status = dds_take (reader, s_samples, &info, 1, 1);
    cr_assert_eq(status, 1);

    /* status after taking the data should be reset */
    status = dds_get_status_changes (reader, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_neq(status2, ~DDS_DATA_AVAILABLE_STATUS);

    /* status from reader2 */
    status = dds_get_status_changes (reader2, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_neq(status2, ~DDS_DATA_AVAILABLE_STATUS);

    /* status from subscriber */
    status = dds_get_status_changes (subscriber, &status2);
    cr_assert_status_eq(status, DDS_RETCODE_OK);
    cr_assert_eq(status2, 0);

    RoundTripModule_DataType_free (&s_sample, DDS_FREE_CONTENTS);

    dds_delete(reader2);

    /* Wait for reader to be deleted */
    status = dds_waitset_wait(waitSetwr, wsresults, wsresultsize, waitTimeout);
    cr_assert_neq(status, 0);
}

/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_enabled_status, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_get_enabled_status, bad_param)
{
    uint32_t s = 0;
    dds_return_t ret;
    dds_return_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_get_enabled_status(e, &s);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_get_enabled_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t s = 0;
    dds_delete(reader);
    ret = dds_get_enabled_status(reader, &s);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_get_enabled_status(): returned %d", dds_err_nr(ret));
}

Test(vddsc_get_enabled_status, illegal, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t s = 0;
    ret = dds_get_enabled_status(waitSetrd, &s);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}

TheoryDataPoints(vddsc_get_enabled_status, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber),
};
Theory((dds_entity_t *e), vddsc_get_enabled_status, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t status;
    uint32_t s = 0;
    status = dds_get_enabled_status (*e, &s);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_enabled_status(entity, status)");
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_set_enabled_status, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_set_enabled_status, bad_param)
{
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_set_enabled_status(e, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_set_enabled_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_set_enabled_status(reader, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_set_enabled_status(): returned %d", dds_err_nr(ret));
}

Test(vddsc_set_enabled_status, illegal, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_entity_t ret;
    ret = dds_set_enabled_status(waitSetrd, 0);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "dds_set_enabled_status(): returned %d", dds_err_nr(ret));
}

TheoryDataPoints(vddsc_set_enabled_status, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber),
};
Theory((dds_entity_t *entity), vddsc_set_enabled_status, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t status;
    status = dds_set_enabled_status (*entity, 0);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_set_enabled_status(entity, mask)");
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_read_status, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_read_status, bad_param)
{
    uint32_t status = 0;
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_read_status(e, &status, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_read_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t status = 0;
    dds_delete(reader);
    ret = dds_read_status(reader, &status, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_read_status(): returned %d", dds_err_nr(ret));
}

Test (vddsc_read_status, illegal, .init=init_entity_status, .fini=fini_entity_status)
{
    uint32_t status = 0;
    dds_return_t ret;
    ret = dds_read_status(waitSetrd, &status, 0);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "dds_read_status(): returned %d", dds_err_nr(ret));
}
TheoryDataPoints(vddsc_read_status, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber),
};
Theory((dds_entity_t *e), vddsc_read_status, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t status;
    status = dds_read_status (*e, &status, 0);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_read_status(entity, status, mask)");
}

Test (vddsc_read_status, invalid_status_on_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    uint32_t status = 0;
    dds_return_t ret;
    ret = dds_read_status(reader, &status, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER, "dds_read_status(): returned %d", dds_err_nr(ret));
}

Test (vddsc_read_status, invalid_status_on_writer, .init=init_entity_status, .fini=fini_entity_status)
{
    uint32_t status = 0;
    dds_return_t ret;
    ret = dds_read_status(writer, &status, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER, "dds_read_status(): returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_take_status, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_take_status, bad_param)
{
    uint32_t status = 0;
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_take_status(e, &status, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_take_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t status = 0;
    dds_delete(reader);
    ret = dds_take_status(reader, &status, 0 /*mask*/);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_take_status(): returned %d", dds_err_nr(ret));
}
Test(vddsc_take_status, illegal, .init=init_entity_status, .fini=fini_entity_status)
{
    uint32_t status = 0;
    dds_return_t ret;
    ret = dds_take_status(waitSetrd, &status, 0);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "dds_take_status(): returned %d", dds_err_nr(ret));
}

TheoryDataPoints(vddsc_take_status, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber),
};
Theory((dds_entity_t *e), vddsc_take_status, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t status = 0;
    ret = dds_take_status (*e, &status, 0 /*mask*/);
    cr_assert_status_eq(ret, DDS_RETCODE_OK, "dds_take_status(entity, status, mask)");
}

/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_status_changes, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_get_status_changes, bad_param)
{
    uint32_t s = 0;
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_get_status_changes(e, &s);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_get_status_changes, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t s = 0;
    dds_delete(reader);
    ret = dds_get_status_changes(reader, &s);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_get_status_changes(): returned %d", dds_err_nr(ret));
}

Test(vddsc_get_status_changes, illegal, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    uint32_t s = 0;
    ret = dds_get_status_changes(waitSetrd, &s);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "dds_get_status_changes(): returned %d", dds_err_nr(ret));
}

TheoryDataPoints(vddsc_get_status_changes, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber),
};
Theory((dds_entity_t *e), vddsc_get_status_changes, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t status;
    uint32_t s = 0;
    status = dds_get_status_changes (*e, &s);
    cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_status_changes(entity, status)");
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_triggered, bad_param) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t e), vddsc_triggered, bad_param)
{
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (e < 0) {
        /* Entering the API with an error should return the same error. */
        exp = e;
    }

    ret = dds_triggered(e);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "dds_triggered(): returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

Test(vddsc_triggered, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_triggered(reader);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "dds_triggered(): returned %d", dds_err_nr(ret));
}

TheoryDataPoints(vddsc_triggered, status_ok) = {
        DataPoints(dds_entity_t *,&reader, &writer, &participant, &topic, &publisher, &subscriber, &waitSetrd),
};
Theory((dds_entity_t *e), vddsc_triggered, status_ok, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t status;
    status = dds_triggered (*e);
    cr_assert_geq(status, 0, "dds_triggered(entity)");
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_inconsistent_topic_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t topic), vddsc_get_inconsistent_topic_status, bad_params)
{
    dds_inconsistent_topic_status_t topic_status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (topic < 0) {
        /* Entering the API with an error should return the same error. */
        exp = topic;
    }

    ret = dds_get_inconsistent_topic_status(topic, &topic_status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_inconsistent_topic_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_inconsistent_topic_status(topic, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_inconsistent_topic_status, non_topics) = {
        DataPoints(dds_entity_t*, &reader, &writer, &participant),
};
Theory((dds_entity_t *topic), vddsc_get_inconsistent_topic_status, non_topics, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_inconsistent_topic_status(*topic, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_inconsistent_topic_status, deleted_topic, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(topic);
    ret = dds_get_inconsistent_topic_status(topic, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %s", dds_err_str(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_publication_matched_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t writer), vddsc_get_publication_matched_status, bad_params)
{
	dds_publication_matched_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (writer < 0) {
        /* Entering the API with an error should return the same error. */
        exp = writer;
    }

    ret = dds_get_publication_matched_status(writer, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_publication_matched_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_publication_matched_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_publication_matched_status, non_writers) = {
        DataPoints(dds_entity_t*, &reader, &topic, &participant),
};
Theory((dds_entity_t *writer), vddsc_get_publication_matched_status, non_writers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_publication_matched_status(*writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_publication_matched_status, deleted_writer, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(writer);
    ret = dds_get_publication_matched_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_liveliness_lost_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t writer), vddsc_get_liveliness_lost_status, bad_params)
{
	dds_liveliness_lost_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (writer < 0) {
        /* Entering the API with an error should return the same error. */
        exp = writer;
    }

    ret = dds_get_liveliness_lost_status(writer, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_liveliness_lost_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_liveliness_lost_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_liveliness_lost_status, non_writers) = {
        DataPoints(dds_entity_t*, &reader, &topic, &participant),
};
Theory((dds_entity_t *writer), vddsc_get_liveliness_lost_status, non_writers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_liveliness_lost_status(*writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_liveliness_lost_status, deleted_writer, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(writer);
    ret = dds_get_liveliness_lost_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_offered_deadline_missed_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t writer), vddsc_get_offered_deadline_missed_status, bad_params)
{
	dds_offered_deadline_missed_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (writer < 0) {
        /* Entering the API with an error should return the same error. */
        exp = writer;
    }

    ret = dds_get_offered_deadline_missed_status(writer, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_offered_deadline_missed_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_offered_deadline_missed_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_offered_deadline_missed_status, non_writers) = {
        DataPoints(dds_entity_t*, &reader, &topic, &participant),
};
Theory((dds_entity_t *writer), vddsc_get_offered_deadline_missed_status, non_writers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_offered_deadline_missed_status(*writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_offered_deadline_missed_status, deleted_writer, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(writer);
    ret = dds_get_offered_deadline_missed_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_offered_incompatible_qos_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t writer), vddsc_get_offered_incompatible_qos_status, bad_params)
{
	dds_offered_incompatible_qos_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (writer < 0) {
        /* Entering the API with an error should return the same error. */
        exp = writer;
    }

    ret = dds_get_offered_incompatible_qos_status(writer, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_offered_incompatible_qos_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_offered_incompatible_qos_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_offered_incompatible_qos_status, non_writers) = {
        DataPoints(dds_entity_t*, &reader, &topic, &participant),
};
Theory((dds_entity_t *writer), vddsc_get_offered_incompatible_qos_status, non_writers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_offered_incompatible_qos_status(*writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_offered_incompatible_qos_status, deleted_writer, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(writer);
    ret = dds_get_offered_incompatible_qos_status(writer, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_subscription_matched_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_subscription_matched_status, bad_params)
{
	dds_subscription_matched_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_subscription_matched_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_subscription_matched_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_subscription_matched_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %s", dds_err_str(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_subscription_matched_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_subscription_matched_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_subscription_matched_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_subscription_matched_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_subscription_matched_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_liveliness_changed_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_liveliness_changed_status, bad_params)
{
	dds_liveliness_changed_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_liveliness_changed_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_liveliness_changed_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_liveliness_changed_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_liveliness_changed_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_liveliness_changed_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_liveliness_changed_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_liveliness_changed_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_liveliness_changed_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_sample_rejected_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_sample_rejected_status, bad_params)
{
	dds_sample_rejected_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_sample_rejected_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_sample_rejected_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_sample_rejected_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_sample_rejected_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_sample_rejected_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_sample_rejected_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_sample_rejected_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_sample_rejected_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_sample_lost_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_sample_lost_status, bad_params)
{
	dds_sample_lost_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_sample_lost_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_sample_lost_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_sample_lost_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_sample_lost_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_sample_lost_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_sample_lost_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_sample_lost_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_sample_lost_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_requested_deadline_missed_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_requested_deadline_missed_status, bad_params)
{
	dds_requested_deadline_missed_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_requested_deadline_missed_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_requested_deadline_missed_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_sample_lost_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_requested_deadline_missed_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_requested_deadline_missed_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_requested_deadline_missed_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_requested_deadline_missed_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_requested_deadline_missed_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_requested_incompatible_qos_status, bad_params) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t reader), vddsc_get_requested_incompatible_qos_status, bad_params)
{
	dds_requested_incompatible_qos_status_t status = {0};
    dds_return_t ret;
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;

    if (reader < 0) {
        /* Entering the API with an error should return the same error. */
        exp = reader;
    }

    ret = dds_get_requested_incompatible_qos_status(reader, &status);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_requested_incompatible_qos_status, null, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_requested_incompatible_qos_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_OK, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
TheoryDataPoints(vddsc_get_requested_incompatible_qos_status, non_readers) = {
        DataPoints(dds_entity_t*, &writer, &topic, &participant),
};
Theory((dds_entity_t *reader), vddsc_get_requested_incompatible_qos_status, non_readers, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    ret = dds_get_requested_incompatible_qos_status(*reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
Test(vddsc_get_requested_incompatible_qos_status, deleted_reader, .init=init_entity_status, .fini=fini_entity_status)
{
    dds_return_t ret;
    dds_delete(reader);
    ret = dds_get_requested_incompatible_qos_status(reader, NULL);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}
/*************************************************************************************************/

/*************************************************************************************************/
