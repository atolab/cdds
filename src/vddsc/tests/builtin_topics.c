#include "qos.h"
#include "RoundTrip.h"
#include "Space.h"
#include "dds_builtinTopics.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>


// typedef struct DDS_TopicBuiltinTopicData
// typedef struct DDS_TypeBuiltinTopicData
// typedef struct DDS_PublicationBuiltinTopicData
// typedef struct DDS_SubscriptionBuiltinTopicData
//
// typedef struct DDS_CMParticipantBuiltinTopicData
// typedef struct DDS_CMPublisherBuiltinTopicData
// typedef struct DDS_CMSubscriberBuiltinTopicData
// typedef struct DDS_CMDataWriterBuiltinTopicData
// typedef struct DDS_CMDataReaderBuiltinTopicData



#define MAX_SAMPLES 10

static dds_entity_t g_participant = 0;
static dds_entity_t g_subscriber  = 0;
static dds_entity_t g_publisher   = 0;
static dds_entity_t g_topic       = 0;

static dds_entity_t g_builtin_subscriber  = 0;

static DDS_SubscriptionBuiltinTopicData g_sub_data[MAX_SAMPLES];
static DDS_PublicationBuiltinTopicData g_pub_data[MAX_SAMPLES];
static DDS_TopicBuiltinTopicData g_tpc_data[MAX_SAMPLES];

static dds_sample_info_t g_info[MAX_SAMPLES];

static void
setup(void)
{
  qos_init();

  g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(g_participant, 0, "Failed to create prerequisite g_participant");
  g_topic = dds_create_topic(g_participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  cr_assert_gt(g_topic, 0, "Failed to create prerequisite g_topic");
  g_subscriber = dds_create_subscriber(g_participant, NULL, NULL);
  cr_assert_gt(g_subscriber, 0, "Failed to create prerequisite g_subscriber");
  g_publisher = dds_create_publisher(g_participant, NULL, NULL);
  cr_assert_gt(g_publisher, 0, "Failed to create prerequisite g_publisher");

//   g_builtin_subscriber = g_participant.get_builtin_subscriber();
//   dds_entity_t g_builtin_subscriber = dds_get_builtin_subscriber(g_participant);
//   cr_assert_gt(g_builtin_subscriber, 0, "Failed to retrieve the current builtin subscriber");


}

static void
teardown(void)
{
    dds_free(g_qos);
    dds_delete(g_subscriber);
    dds_delete(g_topic);
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

Test(vddsc_builtin_topics, builtin_qos, .init = setup, .fini = teardown)
{
  check_default_qos_of_builtin_entity(g_builtin_subscriber);
}

Test(vddsc_builtin_topics, builtin_qos_subscription_reader, .init = setup, .fini = teardown)
{
//   dds_entity_t builtin_sub_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSSubscription");
//   check_default_qos_of_builtin_entity(builtin_sub_reader);
}

Test(vddsc_builtin_topics, builtin_qos_publication_reader, .init = setup, .fini = teardown)
{
//   dds_entity_t builtin_pub_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSPublication");
//   check_default_qos_of_builtin_entity(builtin_pub_reader);
}

Test(vddsc_builtin_topics, builtin_qos_topic_reader, .init = setup, .fini = teardown)
{
//   dds_entity_t builtin_tpc_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSTopic");
//   check_default_qos_of_builtin_entity(builtin_tpc_reader);
}

Test(vddsc_builtin_topics, create_datareader, .init = setup, .fini = teardown)
{
  dds_entity_t rdr;
  dds_entity_t builtin_reader;

  //  Set some qos' which differ from the default
  dds_qset_durability(g_qos, g_pol_durability.kind);
  dds_qset_deadline(g_qos, g_pol_deadline.deadline);
  dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
  dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
  dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
  dds_qset_ownership(g_qos, g_pol_ownership.kind);
  dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
  dds_qset_userdata(g_qos, g_pol_userdata.value, g_pol_userdata.sz);
  dds_qset_time_based_filter(g_qos, g_pol_time_based_filter.minimum_separation);
  dds_qset_presentation(g_qos, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
  dds_qset_partition(g_qos, g_pol_partition.n, (const char **)g_pol_partition.ps);
  dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);
  dds_qset_groupdata(g_qos, g_pol_groupdata.value, g_pol_groupdata.sz);

  rdr = dds_create_reader(g_subscriber, g_topic, g_qos, NULL);

//   dds_entity_t builtin_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSSubscription");
//   cr_assert_gt(g_builtin_subscriber, 0, "Failed to retrieve built-in datareader for DCPSSubscription");
//   dds_return_t ret = dds_read(builtin_reader, g_sub_data, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert(ret, 0, "Failed to read Subscription data");

  // Check the QOS settings of the 'remote' qos'
  // TODO
  DDS_SubscriptionBuiltinTopicData subscription_data = g_sub_data[0];

  cr_assert_str_eq(subscription_data.topic_name, "RoundTrip");
  cr_assert_str_eq(subscription_data.type_name, "RoundTripModule::DataType");
  cr_assert_eq(subscription_data.durability.kind, g_pol_durability.kind);
  cr_assert_eq(subscription_data.deadline.period.sec, g_pol_deadline.deadline);
  cr_assert_eq(subscription_data.latency_budget.duration.sec, g_pol_latency_budget.duration);
  cr_assert_eq(subscription_data.liveliness.kind, g_pol_liveliness.kind);
  cr_assert_eq(subscription_data.liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
  cr_assert_eq(subscription_data.reliability.kind, g_pol_reliability.kind);
  cr_assert_eq(subscription_data.reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
  cr_assert_eq(subscription_data.ownership.kind, g_pol_ownership.kind);
  cr_assert_eq(subscription_data.destination_order.kind, g_pol_destination_order.kind);
  cr_assert_eq(subscription_data.user_data.value._buffer, g_pol_userdata.value);
  cr_assert_eq(subscription_data.user_data.value._length, g_pol_userdata.sz);
  cr_assert_eq(subscription_data.time_based_filter.minimum_separation.sec, g_pol_time_based_filter.minimum_separation);
  cr_assert_eq(subscription_data.presentation.access_scope, g_pol_presentation.access_scope);
  cr_assert_eq(subscription_data.presentation.coherent_access, g_pol_presentation.coherent_access);

  cr_assert_eq(subscription_data.presentation.ordered_access, g_pol_presentation.ordered_access);
  cr_assert_str_eq(subscription_data.partition.name._buffer, g_pol_partition.n);
  cr_assert_str_eq(subscription_data.topic_data.value._buffer, g_pol_topicdata.value);
  cr_assert_str_eq(subscription_data.topic_data.value._length, g_pol_topicdata.sz);
  cr_assert_str_eq(subscription_data.group_data.value._buffer, g_pol_groupdata.value);

  // TODO : freeing stuff?
}

Test(vddsc_builtin_topics, create_datawriter, .init = setup, .fini = teardown)
{
  dds_entity_t wrtr;
  dds_entity_t builtin_reader;

  dds_qset_durability(g_qos, g_pol_durability.kind);
  dds_qset_deadline(g_qos, g_pol_deadline.deadline);
  dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
  dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
  dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
  dds_qset_lifespan(g_qos, g_pol_lifespan.lifespan);
  dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
  dds_qset_userdata(g_qos, g_pol_userdata.value, g_pol_userdata.sz);
  dds_qset_ownership(g_qos, g_pol_ownership.kind);
  dds_qset_ownership_strength(g_qos, g_pol_ownership_strength.value);
  dds_qset_presentation(g_qos, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
  dds_qset_partition(g_qos, g_pol_partition.n, (const char **)g_pol_partition.ps);
  dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);

  wrtr = dds_create_writer(g_publisher, g_topic, g_qos, NULL);

//   dds_entity_t builtin_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSPublication");
//   cr_assert_gt(g_builtin_subscriber, 0, "Failed to retrieve built-in datareader for DCPSPublication");
//   dds_return_t ret = dds_read(builtin_reader, g_pub_data, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert(ret, 0, "Failed to read Publication data");

  // TODO
  DDS_PublicationBuiltinTopicData publication_data = g_pub_data[0];

  cr_assert_str_eq(publication_data.topic_name, "RoundTrip");
  cr_assert_str_eq(publication_data.type_name, "RoundTripModule::DataType");
  cr_assert_eq(publication_data.durability.kind, g_pol_durability.kind);
  cr_assert_eq(publication_data.deadline.period.sec, g_pol_deadline.deadline);
  cr_assert_eq(publication_data.latency_budget.duration.sec, g_pol_latency_budget.duration);
  cr_assert_eq(publication_data.liveliness.kind, g_pol_liveliness.kind);
  cr_assert_eq(publication_data.liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
  cr_assert_eq(publication_data.reliability.kind, g_pol_reliability.kind);
  cr_assert_eq(publication_data.reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
  cr_assert_eq(publication_data.lifespan.duration.sec, g_pol_lifespan.lifespan);
  cr_assert_eq(publication_data.destination_order.kind, g_pol_destination_order.kind);
  cr_assert_eq(publication_data.user_data.value._buffer, g_pol_userdata.value);
  cr_assert_eq(publication_data.user_data.value._length, g_pol_userdata.sz);
  cr_assert_eq(publication_data.ownership.kind, g_pol_ownership.kind);
  cr_assert_eq(publication_data.ownership_strength.value, g_pol_ownership_strength.value);
  cr_assert_eq(publication_data.presentation.access_scope, g_pol_presentation.access_scope);
  cr_assert_eq(publication_data.presentation.coherent_access, g_pol_presentation.coherent_access);
  cr_assert_eq(publication_data.presentation.ordered_access, g_pol_presentation.ordered_access);
  cr_assert_str_eq(publication_data.partition.name._buffer, g_pol_partition.n);
  cr_assert_str_eq(publication_data.topic_data.value._buffer, g_pol_topicdata.value);
  cr_assert_str_eq(publication_data.topic_data.value._length, g_pol_topicdata.sz);
  cr_assert_str_eq(publication_data.group_data.value._buffer, g_pol_groupdata.value);

  // TODO : freeing stuff
}

Test(vddsc_builtin_topics, create_topic, .init = setup, .fini = teardown)
{
  dds_entity_t tpc;
  dds_entity_t builtin_reader;

  dds_qset_durability(g_qos, g_pol_durability.kind);
  dds_qset_durability_service(g_qos,
                              g_pol_durability_service.service_cleanup_delay,
                              g_pol_durability_service.history_kind,
                              g_pol_durability_service.history_depth,
                              g_pol_durability_service.max_samples,
                              g_pol_durability_service.max_instances,
                              g_pol_durability_service.max_samples_per_instance);
  dds_qset_deadline(g_qos, g_pol_deadline.deadline);
  dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
  dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
  dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
  dds_qset_transport_priority(g_qos, g_pol_transport_priority.value);
  dds_qset_lifespan(g_qos, g_pol_lifespan.lifespan);
  dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
  dds_qset_history(g_qos, g_pol_history.kind, g_pol_history.depth);
  dds_qset_resource_limits(g_qos, g_pol_resource_limits.max_samples, g_pol_resource_limits.max_instances,
                           g_pol_resource_limits.max_samples_per_instance);
  dds_qset_ownership(g_qos, g_pol_ownership.kind);
  dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);


  tpc = dds_create_topic(g_participant, &Space_Type1_desc, "SpaceType1", g_qos, NULL);

//   dds_entity_t builtin_reader = dds_lookup_datareader(g_builtin_subscriber, "DCPSTopic");
//   cr_assert_gt(g_builtin_subscriber, 0, "Failed to retrieve built-in datareader for DCPSTopic");
//   dds_return_t ret = dds_read(builtin_reader, g_tpc_data, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert(ret, 0, "Failed to read Topic data");

  // TODO
  DDS_TopicBuiltinTopicData topic_data = g_tpc_data[0];

  cr_assert_str_eq(topic_data.name, "SpaceType1");
  cr_assert_str_eq(topic_data.type_name, "RoundTripModule::DataType");
  cr_assert_eq(topic_data.durability.kind, g_pol_durability.kind);
  cr_assert_eq(topic_data.durability_service.service_cleanup_delay.sec, g_pol_durability_service.service_cleanup_delay);
  cr_assert_eq(topic_data.durability_service.history_kind, g_pol_durability_service.history_kind);
  cr_assert_eq(topic_data.durability_service.history_depth, g_pol_durability_service.history_depth);
  cr_assert_eq(topic_data.durability_service.max_samples, g_pol_durability_service.max_samples);
  cr_assert_eq(topic_data.durability_service.max_instances, g_pol_durability_service.max_instances);
  cr_assert_eq(topic_data.durability_service.max_samples_per_instance, g_pol_durability_service.max_samples_per_instance);
  cr_assert_eq(topic_data.deadline.period.sec, g_pol_deadline.deadline);
  cr_assert_eq(topic_data.latency_budget.duration.sec, g_pol_latency_budget.duration);
  cr_assert_eq(topic_data.liveliness.kind, g_pol_liveliness.kind);
  cr_assert_eq(topic_data.liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
  cr_assert_eq(topic_data.reliability.kind, g_pol_reliability.kind);
  cr_assert_eq(topic_data.reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
  cr_assert_eq(topic_data.transport_priority.value, g_pol_transport_priority.value);
  cr_assert_eq(topic_data.lifespan.duration.sec, g_pol_lifespan.lifespan);
  cr_assert_eq(topic_data.destination_order.kind, g_pol_destination_order.kind);
  cr_assert_eq(topic_data.history.kind, g_pol_history.kind);
  cr_assert_eq(topic_data.history.depth, g_pol_history.depth);
  cr_assert_eq(topic_data.resource_limits.max_samples, g_pol_resource_limits.max_samples);
  cr_assert_eq(topic_data.resource_limits.max_instances, g_pol_resource_limits.max_instances);
  cr_assert_eq(topic_data.resource_limits.max_samples_per_instance, g_pol_resource_limits.max_samples_per_instance);
  cr_assert_eq(topic_data.ownership.kind, g_pol_ownership.kind);
  cr_assert_str_eq(topic_data.topic_data.value._buffer, g_pol_topicdata.value);
  cr_assert_str_eq(topic_data.topic_data.value._length, g_pol_topicdata.sz);

  // TODO : freeing stuff
}
