#include "qos.h"
#include "RoundTrip.h"
#include "Space.h"
#include <criterion/logging.h>

static dds_entity_t g_participant = 0;
static dds_entity_t g_subscriber  = 0;
static dds_entity_t g_publisher   = 0;
static dds_entity_t g_topic       = 0;

#define MAX_SAMPLES 1

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
}

static void
teardown(void)
{
    dds_free(g_qos);
    dds_delete(g_participant);
}

static void
check_default_qos_of_builtin_entity(dds_entity_t entity)
{
  dds_return_t ret;
  dds_qos_t *qos = dds_qos_create();
  cr_assert_not_null(qos);

  ret = dds_get_qos(entity, qos);
  cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to get QOS of builtin Subscriober.");
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
//   dds_entity_t topic;
//
//   topic = dds_find_topic(g_participant, "DCPSParticipant");
//   cr_assert_lt(topic, 0);
//   topic = dds_find_topic(g_participant, "DCPSTopic");
//   cr_assert_lt(topic, 0);
//   topic = dds_find_topic(g_participant, "DCPSType");
//   cr_assert_lt(topic, 0);
//   topic = dds_find_topic(g_participant, "DCPSSubscription");
//   cr_assert_lt(topic, 0);
//   topic = dds_find_topic(g_participant, "DCSPPublication");
//   cr_assert_lt(topic, 0);
}

Test(vddsc_builtin_topics, read_publication_data, .init = setup, .fini = teardown)
{
//   dds_entity_t reader;
//   dds_return_t ret;
//   void * samples[MAX_SAMPLES];
//   DDS_PublicationBuiltinTopicData *data;
//
//   reader = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, NULL);
//   cr_assert_gt(reader, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPUBLICATION.");
//
//   samples[0] = DDS_PublicationBuiltinTopicData__alloc();
//
//   ret = dds_read(reader, samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read samples DCPSPublication");
//
//   data = (DDS_PublicationBuiltinTopicData *)samples;
//   cr_assert_str_eq(data->topic_name, "DCPSPublication");
//
//   DDS_PublicationBuiltinTopicData_free(samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, read_subscription_data, .init = setup, .fini = teardown)
{
//   dds_entity_t reader;
//   dds_return_t ret;
//   void * samples[MAX_SAMPLES];
//   DDS_SubscriptionBuiltinTopicData *data;
//
//   reader = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
//   cr_assert_gt(reader, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");
//
//   samples[0] = DDS_SubscriptionBuiltinTopicData__alloc();
//
//   ret = dds_read(reader, samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read samples DCPSSubscription");
//
//   data = (DDS_SubscriptionBuiltinTopicData *)samples;
//   cr_assert_str_eq(data->topic_name, "DCPSSubscription");
//
//   DDS_SubscriptionBuiltinTopicData_free(samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, read_participant_data, .init = setup, .fini = teardown)
{
//   dds_entity_t reader;
//   dds_return_t ret;
//   void * samples[MAX_SAMPLES];
//
//   reader = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, NULL, NULL);
//   cr_assert_gt(reader, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPARTICIPANT.");
//
//   samples[0] = DDS_ParticipantBuiltinTopicData__alloc();
//
//   ret = dds_read(reader, samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read samples DCPSParticipant");
//
//   DDS_ParticipantBuiltinTopicData_free(samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, read_topic_data, .init = setup, .fini = teardown)
{
//   dds_entity_t reader;
//   dds_return_t ret;
//   void * samples[MAX_SAMPLES];
//   DDS_TopicBuiltinTopicData *data;
//
//   reader = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSTOPIC, NULL, NULL);
//   cr_assert_gt(reader, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSTOPIC.");
//
//   samples[0] = DDS_TopicBuiltinTopicData__alloc();
//
//   ret = dds_read(reader, samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read samples DCPSParticipant");
//
//   data = (DDS_TopicBuiltinTopicData *)samples;
//   cr_assert_str_eq(data->name, "DCPSSubscription");
//
//   DDS_ParticipantBuiltinTopicData_free(samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, read_type_data, .init = setup, .fini = teardown)
{
//   dds_entity_t reader;
//   dds_return_t ret;
//   void * samples[MAX_SAMPLES];
//   DDS_TypeBuiltinTopicData *data;
//
//   reader = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSTYPE, NULL, NULL);
//   cr_assert_gt(reader, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSTYPE.");
//
//   samples[0] = DDS_TypeBuiltinTopicData__alloc();
//
//   ret = dds_read(reader, samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read samples DCPSType");
//
//   data = (DDS_TypeBuiltinTopicData *)samples;
//   cr_assert_str_eq(data->name, "DCPSType");
//
//   DDS_TypeBuiltinTopicData_free(samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, same_subscriber, .init = setup, .fini = teardown)
{
//   dds_entity_t subscription_rdr;
//   dds_entity_t subscription_subscriber;
//
//   dds_entity_t publication_rdr;
//   dds_entity_t publication_subscriber;
//
//   dds_entity_t participant_rdr;
//   dds_entity_t participant_subscriber;
//
//   dds_entity_t topic_rdr;
//   dds_entity_t topic_subscriber;
//
//   dds_entity_t type_rdr;
//   dds_entity_t type_subscriber;
//
//   subscription_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
//   cr_assert_gt(subscription_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");
//   subscription_subscriber = dds_get_parent(subscription_rdr);
//   cr_assert_gt(subscription_subscriber, 0, "Could not find builtin subscriber for DSCPSSubscription-reader.");
//
//   publication_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, NULL);
//   cr_assert_gt(publication_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPUBLICATION.");
//   publication_subscriber = dds_get_parent(publication_rdr);
//   cr_assert_gt(publication_subscriber, 0, "Could not find builtin subscriber for DSCPSPublication-reader.");
//
//   cr_assert_eq(subscription_subscriber, publication_subscriber);
//
//   participant_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, NULL, NULL);
//   cr_assert_gt(participant_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSPARTICIPANT.");
//   participant_subscriber = dds_get_parent(participant_rdr);
//   cr_assert_gt(participant_subscriber, 0, "Could not find builtin subscriber for DSCPSParticipant-reader.");
//
//   cr_assert_eq(publication_subscriber, participant_subscriber);
//
//   topic_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSTOPIC, NULL, NULL);
//   cr_assert_gt(topic_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSTOPIC.");
//   topic_subscriber = dds_get_parent(topic_rdr);
//   cr_assert_gt(topic_subscriber, 0, "Could not find builtin subscriber for DSCPSTopic-reader.");
//
//   cr_assert_eq(participant_subscriber, topic_subscriber);
//
//   type_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSTYPE, NULL, NULL);
//   cr_assert_gt(type_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSTYPE.");
//   type_subscriber = dds_get_parent(type_rdr);
//   cr_assert_gt(type_subscriber, 0, "Could not find builtin subscriber for DSCPSType-reader.");
//
//   cr_assert_eq(topic_subscriber, type_subscriber);
}

Test(vddsc_builtin_topics, builtin_qos, .init = setup, .fini = teardown)
{
//   dds_entity_t dds_sub_rdr;
//   dds_entity_t dds_sub_subscriber;
//
//   dds_sub_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
//   cr_assert_gt(dds_sub_rdr, 0, "Failed to create a data reader for DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION.");
//   check_default_qos_of_builtin_entity(dds_sub_rdr);
//
//   dds_sub_subscriber = dds_get_parent(dds_sub_rdr);
//   cr_assert_gt(dds_sub_subscriber, 0, "Could not find builtin subscriber for DSCPSSubscription-reader.");
//   check_default_qos_of_builtin_entity(dds_sub_subscriber);
}

Test(vddsc_builtin_topics, datareader_qos, .init = setup, .fini = teardown)
{
//   dds_entity_t rdr;
//   dds_entity_t subscription_rdr;
//   dds_return_t ret;
//   void * subscription_samples[MAX_SAMPLES];
//   DDS_SubscriptionBuiltinTopicData *subscription_data;
//
//   //  Set some qos' which differ from the default
//   dds_qset_durability(g_qos, g_pol_durability.kind);
//   dds_qset_deadline(g_qos, g_pol_deadline.deadline);
//   dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
//   dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
//   dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
//   dds_qset_ownership(g_qos, g_pol_ownership.kind);
//   dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
//   dds_qset_userdata(g_qos, g_pol_userdata.value, g_pol_userdata.sz);
//   dds_qset_time_based_filter(g_qos, g_pol_time_based_filter.minimum_separation);
//   dds_qset_presentation(g_qos, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
//   dds_qset_partition(g_qos, g_pol_partition.n, (const char **)g_pol_partition.ps);
//   dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);
//   dds_qset_groupdata(g_qos, g_pol_groupdata.value, g_pol_groupdata.sz);
//
//   rdr = dds_create_reader(g_subscriber, g_topic, g_qos, NULL);
//
//   subscription_samples[0] = DDS_SubscriptionBuiltinTopicData__alloc();
//
//   subscription_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION, NULL, NULL);
//   cr_assert_gt(subscription_rdr, 0, "Failed to retrieve built-in datareader for DCPSSubscription");
//   ret = dds_read(subscription_rdr, subscription_samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read Subscription data");
//
//   // Check the QOS settings of the 'remote' qos'
//   subscription_data = (DDS_SubscriptionBuiltinTopicData *)subscription_samples[0];
//
//   cr_assert_str_eq(subscription_data->topic_name, "RoundTrip");
//   cr_assert_str_eq(subscription_data->type_name, "RoundTripModule::DataType");
//   cr_assert_eq(subscription_data->durability.kind, g_pol_durability.kind);
//   cr_assert_eq(subscription_data->deadline.period.sec, g_pol_deadline.deadline);
//   cr_assert_eq(subscription_data->latency_budget.duration.sec, g_pol_latency_budget.duration);
//   cr_assert_eq(subscription_data->liveliness.kind, g_pol_liveliness.kind);
//   cr_assert_eq(subscription_data->liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
//   cr_assert_eq(subscription_data->reliability.kind, g_pol_reliability.kind);
//   cr_assert_eq(subscription_data->reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
//   cr_assert_eq(subscription_data->ownership.kind, g_pol_ownership.kind);
//   cr_assert_eq(subscription_data->destination_order.kind, g_pol_destination_order.kind);
//   cr_assert_eq(subscription_data->user_data.value._buffer, g_pol_userdata.value);
//   cr_assert_eq(subscription_data->user_data.value._length, g_pol_userdata.sz);
//   cr_assert_eq(subscription_data->time_based_filter.minimum_separation.sec, g_pol_time_based_filter.minimum_separation);
//   cr_assert_eq(subscription_data->presentation.access_scope, g_pol_presentation.access_scope);
//   cr_assert_eq(subscription_data->presentation.coherent_access, g_pol_presentation.coherent_access);
//
//   cr_assert_eq(subscription_data->presentation.ordered_access, g_pol_presentation.ordered_access);
//   cr_assert_str_eq(subscription_data->partition.name._buffer, g_pol_partition.n);
//   cr_assert_str_eq(subscription_data->topic_data.value._buffer, g_pol_topicdata.value);
//   cr_assert_str_eq(subscription_data->topic_data.value._length, g_pol_topicdata.sz);
//   cr_assert_str_eq(subscription_data->group_data.value._buffer, g_pol_groupdata.value);
//
//   DDS_SubscriptionBuiltinTopicData_free(subscription_samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, datawriter_qos, .init = setup, .fini = teardown)
{
//   dds_entity_t wrtr;
//   dds_entity_t publication_rdr;
//   dds_return_t ret;
//   void * publication_samples[MAX_SAMPLES];
//   DDS_PublicationBuiltinTopicData *publication_data;
//
//   dds_qset_durability(g_qos, g_pol_durability.kind);
//   dds_qset_deadline(g_qos, g_pol_deadline.deadline);
//   dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
//   dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
//   dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
//   dds_qset_lifespan(g_qos, g_pol_lifespan.lifespan);
//   dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
//   dds_qset_userdata(g_qos, g_pol_userdata.value, g_pol_userdata.sz);
//   dds_qset_ownership(g_qos, g_pol_ownership.kind);
//   dds_qset_ownership_strength(g_qos, g_pol_ownership_strength.value);
//   dds_qset_presentation(g_qos, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
//   dds_qset_partition(g_qos, g_pol_partition.n, (const char **)g_pol_partition.ps);
//   dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);
//
//   wrtr = dds_create_writer(g_publisher, g_topic, g_qos, NULL);
//
//   publication_samples[0] = DDS_PublicationBuiltinTopicData__alloc();
//
//   publication_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSPUBLICATION, NULL, NULL);
//   cr_assert_gt(publication_rdr, 0, "Failed to retrieve built-in datareader for DCPSPublication");
//   ret = dds_read(publication_rdr, publication_samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read Publication data");
//
//   // Check the QOS settings of the 'remote' qos'
//   publication_data = (DDS_PublicationBuiltinTopicData *)publication_samples[0];
//
//   cr_assert_str_eq(publication_data->topic_name, "RoundTrip");
//   cr_assert_str_eq(publication_data->type_name, "RoundTripModule::DataType");
//   cr_assert_eq(publication_data->durability.kind, g_pol_durability.kind);
//   cr_assert_eq(publication_data->deadline.period.sec, g_pol_deadline.deadline);
//   cr_assert_eq(publication_data->latency_budget.duration.sec, g_pol_latency_budget.duration);
//   cr_assert_eq(publication_data->liveliness.kind, g_pol_liveliness.kind);
//   cr_assert_eq(publication_data->liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
//   cr_assert_eq(publication_data->reliability.kind, g_pol_reliability.kind);
//   cr_assert_eq(publication_data->reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
//   cr_assert_eq(publication_data->lifespan.duration.sec, g_pol_lifespan.lifespan);
//   cr_assert_eq(publication_data->destination_order.kind, g_pol_destination_order.kind);
//   cr_assert_eq(publication_data->user_data.value._buffer, g_pol_userdata.value);
//   cr_assert_eq(publication_data->user_data.value._length, g_pol_userdata.sz);
//   cr_assert_eq(publication_data->ownership.kind, g_pol_ownership.kind);
//   cr_assert_eq(publication_data->ownership_strength.value, g_pol_ownership_strength.value);
//   cr_assert_eq(publication_data->presentation.access_scope, g_pol_presentation.access_scope);
//   cr_assert_eq(publication_data->presentation.coherent_access, g_pol_presentation.coherent_access);
//   cr_assert_eq(publication_data->presentation.ordered_access, g_pol_presentation.ordered_access);
//   cr_assert_str_eq(publication_data->partition.name._buffer, g_pol_partition.n);
//   cr_assert_str_eq(publication_data->topic_data.value._buffer, g_pol_topicdata.value);
//   cr_assert_str_eq(publication_data->topic_data.value._length, g_pol_topicdata.sz);
//   cr_assert_str_eq(publication_data->group_data.value._buffer, g_pol_groupdata.value);
//
//   DDS_PublicationBuiltinTopicData_free(publication_samples[0], DDS_FREE_ALL);
}

Test(vddsc_builtin_topics, topic_qos, .init = setup, .fini = teardown)
{
//   dds_entity_t tpc;
//   dds_entity_t topic_rdr;
//   dds_return_t ret;
//   void * topic_samples[MAX_SAMPLES];
//   DDS_TopicBuiltinTopicData *topic_data;
//
//   dds_qset_durability(g_qos, g_pol_durability.kind);
//   dds_qset_durability_service(g_qos,
//                               g_pol_durability_service.service_cleanup_delay,
//                               g_pol_durability_service.history_kind,
//                               g_pol_durability_service.history_depth,
//                               g_pol_durability_service.max_samples,
//                               g_pol_durability_service.max_instances,
//                               g_pol_durability_service.max_samples_per_instance);
//   dds_qset_deadline(g_qos, g_pol_deadline.deadline);
//   dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
//   dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
//   dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
//   dds_qset_transport_priority(g_qos, g_pol_transport_priority.value);
//   dds_qset_lifespan(g_qos, g_pol_lifespan.lifespan);
//   dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
//   dds_qset_history(g_qos, g_pol_history.kind, g_pol_history.depth);
//   dds_qset_resource_limits(g_qos, g_pol_resource_limits.max_samples, g_pol_resource_limits.max_instances,
//                            g_pol_resource_limits.max_samples_per_instance);
//   dds_qset_ownership(g_qos, g_pol_ownership.kind);
//   dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);
//
//
//   tpc = dds_create_topic(g_participant, &Space_Type1_desc, "SpaceType1", g_qos, NULL);
//
//   topic_samples[0] = DDS_PublicationBuiltinTopicData__alloc();
//
//   topic_rdr = dds_create_reader(g_participant, DDS_BUILTIN_TOPIC_DCPSTOPIC, NULL, NULL);
//   cr_assert_gt(topic_rdr, 0, "Failed to retrieve built-in datareader for DCPSPublication");
//   ret = dds_read(topic_rdr, topic_samples, g_info, MAX_SAMPLES, MAX_SAMPLES);
//   cr_assert_gt(ret, 0, "Failed to read Topic data");
//
//   topic_data = (DDS_TopicBuiltinTopicData *)topic_samples[0];
//
//   cr_assert_str_eq(topic_data->name, "SpaceType1");
//   cr_assert_str_eq(topic_data->type_name, "RoundTripModule::DataType");
//   cr_assert_eq(topic_data->durability.kind, g_pol_durability.kind);
//   cr_assert_eq(topic_data->durability_service.service_cleanup_delay.sec, g_pol_durability_service.service_cleanup_delay);
//   cr_assert_eq(topic_data->durability_service.history_kind, g_pol_durability_service.history_kind);
//   cr_assert_eq(topic_data->durability_service.history_depth, g_pol_durability_service.history_depth);
//   cr_assert_eq(topic_data->durability_service.max_samples, g_pol_durability_service.max_samples);
//   cr_assert_eq(topic_data->durability_service.max_instances, g_pol_durability_service.max_instances);
//   cr_assert_eq(topic_data->durability_service.max_samples_per_instance, g_pol_durability_service.max_samples_per_instance);
//   cr_assert_eq(topic_data->deadline.period.sec, g_pol_deadline.deadline);
//   cr_assert_eq(topic_data->latency_budget.duration.sec, g_pol_latency_budget.duration);
//   cr_assert_eq(topic_data->liveliness.kind, g_pol_liveliness.kind);
//   cr_assert_eq(topic_data->liveliness.lease_duration.sec, g_pol_liveliness.lease_duration);
//   cr_assert_eq(topic_data->reliability.kind, g_pol_reliability.kind);
//   cr_assert_eq(topic_data->reliability.max_blocking_time.sec, g_pol_reliability.max_blocking_time);
//   cr_assert_eq(topic_data->transport_priority.value, g_pol_transport_priority.value);
//   cr_assert_eq(topic_data->lifespan.duration.sec, g_pol_lifespan.lifespan);
//   cr_assert_eq(topic_data->destination_order.kind, g_pol_destination_order.kind);
//   cr_assert_eq(topic_data->history.kind, g_pol_history.kind);
//   cr_assert_eq(topic_data->history.depth, g_pol_history.depth);
//   cr_assert_eq(topic_data->resource_limits.max_samples, g_pol_resource_limits.max_samples);
//   cr_assert_eq(topic_data->resource_limits.max_instances, g_pol_resource_limits.max_instances);
//   cr_assert_eq(topic_data->resource_limits.max_samples_per_instance, g_pol_resource_limits.max_samples_per_instance);
//   cr_assert_eq(topic_data->ownership.kind, g_pol_ownership.kind);
//   cr_assert_str_eq(topic_data->topic_data.value._buffer, g_pol_topicdata.value);
//   cr_assert_str_eq(topic_data->topic_data.value._length, g_pol_topicdata.sz);
//
//   DDS_TopicBuiltinTopicData_free(topic_samples[0], DDS_FREE_ALL);
}
