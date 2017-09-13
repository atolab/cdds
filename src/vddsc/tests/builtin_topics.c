#include "qos.h"
#include "RoundTrip.h"
#include <criterion/logging.h>

static dds_entity_t g_participant = 0;

static void
setup(void)
{

  g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(g_participant, 0, "Failed to create prerequisite g_participant");
}

static void
teardown(void)
{
  dds_delete(g_participant);
}

static void
check_default_qos_of_builtin_entity(dds_entity_t entity)
{
  dds_return_t ret;
  dds_qos_t *qos = dds_qos_create();
  cr_assert_not_null(qos);

  ret = dds_get_qos(entity, qos);
//   cr_assert(ret, DDS_RETCODE_OK, "Failed to get QOS of builtin Subscriober.");
  dds_qget_durability(qos, &g_pol_durability.kind);
  dds_qget_presentation(qos, &g_pol_presentation.access_scope, &g_pol_presentation.coherent_access, &g_pol_presentation.ordered_access);
  dds_qget_deadline(qos, &g_pol_deadline.deadline);
  dds_qget_ownership(qos, &g_pol_ownership.kind);
  dds_qget_liveliness(qos, &g_pol_liveliness.kind, &g_pol_liveliness.lease_duration);
  dds_qget_time_based_filter(qos, &g_pol_time_based_filter.minimum_separation);
  dds_qget_reliability(qos, &g_pol_reliability.kind, &g_pol_reliability.max_blocking_time);
  dds_qget_destination_order(qos, &g_pol_destination_order.kind);
  dds_qget_history(qos, &g_pol_history.kind, &g_pol_history.depth);
  dds_qget_resource_limits(qos, &g_pol_resource_limits.max_samples, &g_pol_resource_limits.max_instances, &g_pol_resource_limits.max_samples_per_instance);
  dds_qget_reader_data_lifecycle(qos, &g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay, &g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay);
  // no getter for ENTITY_FACTORY
  cr_assert_eq(g_pol_durability.kind, DDS_TRANSIENT_LOCAL_DURABILITY_QOS);
  cr_assert_eq(g_pol_presentation.access_scope, DDS_TOPIC_PRESENTATION_QOS);
  cr_assert_eq(g_pol_presentation.coherent_access, false);
  cr_assert_eq(g_pol_presentation.ordered_access, false);
  cr_assert_eq(g_pol_deadline.deadline, DDS_INFINITY);
  cr_assert_eq(g_pol_ownership.kind, DDS_SHARED_OWNERSHIP_QOS);
  cr_assert_eq(g_pol_liveliness.kind, DDS_AUTOMATIC_LIVELINESS_QOS);
  cr_assert_eq(g_pol_time_based_filter.minimum_separation, 0);
  cr_assert_eq(g_pol_reliability.kind, DDS_RELIABLE_RELIABILITY_QOS);
  cr_assert_eq(g_pol_reliability.max_blocking_time, DDS_MSECS(100));
  cr_assert_eq(g_pol_destination_order.kind, DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS);
  cr_assert_eq(g_pol_history.kind, DDS_KEEP_LAST_HISTORY_QOS);
  cr_assert_eq(g_pol_history.depth, 1);
  cr_assert_eq(g_pol_resource_limits.max_instances, DDS_INFINITY);
  cr_assert_eq(g_pol_resource_limits.max_samples, DDS_INFINITY);
  cr_assert_eq(g_pol_resource_limits.max_samples_per_instance, DDS_INFINITY);
  cr_assert_eq(g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay, DDS_INFINITY);
  cr_assert_eq(g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay, DDS_INFINITY);
}

/* This is just a preliminary test to verify builtin-topic types are available,
 * tests will be extended to actually read topics once that part is implemented
 */
Test(vddsc_builtin_topics, types_allocation)
{
#define TEST_ALLOC(type) do { \
        DDS_##type##BuiltinTopicData *data = DDS_##type##BuiltinTopicData__alloc(); \
        cr_expect_not_null(data, "Failed to allocate DDS_" #type "BuiltinTopicData"); \
        dds_free(data); \
    } while(0)

    TEST_ALLOC(Participant);
    TEST_ALLOC(CMParticipant);
    TEST_ALLOC(Type);
    TEST_ALLOC(Topic);
    /* TEST_ALLOC(CMTopic); */
    TEST_ALLOC(Publication);
    TEST_ALLOC(CMPublisher);
    TEST_ALLOC(Subscription);
    TEST_ALLOC(CMSubscriber);
    TEST_ALLOC(CMDataWriter);
    TEST_ALLOC(CMDataReader);
}

Test(vddsc_builtin_topics, availability_builtin_topics, .init = setup, .fini = teardown)
{
  dds_entity_t topic;

  topic = dds_find_topic(g_participant, "DCPSParticipant");
  cr_assert_lt(topic, 0);
  topic = dds_find_topic(g_participant, "DCPSTopic");
  cr_assert_lt(topic, 0);
  topic = dds_find_topic(g_participant, "DCPSSubscription");
  cr_assert_lt(topic, 0);
  topic = dds_find_topic(g_participant, "DCSPPublication");
  cr_assert_lt(topic, 0);
}

Test(vddsc_builtin_topics, create_datareader, .init = setup, .fini = teardown)
{
  dds_entity_t rdr;
  dds_entity_t topic;

  rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
  cr_assert_gt(rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");

  topic = dds_find_topic(g_participant, "DCPSSubscription");
  cr_assert_gt(topic, 0, "Could not find a builtin topic for DCPSSubscription");

  topic = dds_find_topic(g_participant, "DCSPPublication");
  cr_assert_eq(topic, 0, "Found builtin topic for DCPSPublication");
}

Test(vddsc_builtin_topics, create_datawriter, .init = setup, .fini = teardown)
{
  dds_entity_t rdr;
  dds_entity_t wrt;
  dds_entity_t topic;
  dds_entity_t dds_builtin_topic;
  dds_entity_t dds_sub_topic;
  dds_entity_t dds_pub_topic;

  // Create a topic and a writer to trigger the DCPSPublication topic
  topic = dds_create_topic(g_participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  wrt = dds_create_writer(g_participant, topic, NULL, NULL);

  rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, NULL);
  cr_assert_gt(rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPUBLICATION.");

  dds_builtin_topic = dds_find_topic(g_participant, "DCPSTopic");
  cr_assert_gt(dds_builtin_topic, 0, "Could not find a builtin topic for DCPSTopic-reader");

  dds_sub_topic = dds_find_topic(g_participant, "DCPSSubscription");
  cr_assert_gt(dds_sub_topic, 0, "Could not find a builtin topic for DCPSSubscription");

  dds_pub_topic = dds_find_topic(g_participant, "DCSPPublication");
  cr_assert_gt(dds_pub_topic, 0, "Could not find a builtin topic for DCPSPublication");
}

Test(vddsc_builtin_topics, same_subscriber, .init = setup, .fini = teardown)
{
  dds_entity_t dds_pub_rdr;
  dds_entity_t dds_sub_rdr;
  dds_entity_t dds_sub_subscriber;
  dds_entity_t dds_pub_subscriber;

  dds_sub_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
  cr_assert_gt(dds_sub_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");

  dds_sub_subscriber = dds_get_parent(dds_sub_rdr);
  cr_assert_gt(dds_sub_subscriber, 0, "Could nog find builtin subscriber for DSCPSSubscription-reader.");

  dds_pub_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, NULL);
  cr_assert_gt(dds_pub_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPUBLICATION.");

  dds_pub_subscriber = dds_get_parent(dds_pub_rdr);
  cr_assert_gt(dds_pub_subscriber, 0, "Could nog find builtin subscriber for DSCPPublication-reader.");

  cr_assert_eq(dds_pub_subscriber, dds_sub_subscriber);
}

Test(vddsc_builtin_topics, qos, .init = setup, .fini = teardown)
{
  dds_entity_t dds_sub_rdr;
  dds_entity_t dds_sub_subscriber;

  dds_sub_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
  cr_assert_gt(dds_sub_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");
  check_default_qos_of_builtin_entity(dds_sub_rdr);

  dds_sub_subscriber = dds_get_parent(dds_sub_rdr);
  cr_assert_gt(dds_sub_subscriber, 0, "Could nog find builtin subscriber for DSCPSSubscription-reader.");
  check_default_qos_of_builtin_entity(dds_sub_subscriber);
}
