/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <assert.h>
#include <string.h>
#include <ddsi/q_error.h>
#include <os/os.h>
#include <os/os_stdlib.h>

#include "ddsi/q_misc.h"
#include "ddsi/q_config.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_builtin_topic.h"

#include "dds_builtinTopics.h"

const DDS_Duration_t DDS_Duration_TIME_ZERO         = {0, 0};
const DDS_Duration_t DDS_Duration_INFINITE     = {0x7fffffff, 0x7fffffffU};
const DDS_Duration_t DDS_Duration_MIN_INFINITE = {-0x7fffffff,  0x7fffffffU};
const DDS_Duration_t DDS_Duration_TIME_INVALID      = {-1,  0xffffffffU};

static void generate_user_data (_Out_ DDS_UserDataQosPolicy *target, _In_ const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_USER_DATA) || (xqos->user_data.length == 0))
  {
    target->value._maximum = 0;
    target->value._length  = 0;
    target->value._buffer  = NULL;
    target->value._release = false;
  } else {
    target->value._maximum = xqos->user_data.length;
    target->value._length  = xqos->user_data.length;
    target->value._buffer  = xqos->user_data.value;
    target->value._release = false;
  }
}

static void generate_group_data (_Out_ DDS_GroupDataQosPolicy *target, _In_ const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_GROUP_DATA) || (xqos->group_data.length == 0))
  {
    target->value._maximum = 0;
    target->value._length  = 0;
    target->value._buffer  = NULL;
    target->value._release = false;
  } else {
    target->value._maximum = xqos->group_data.length;
    target->value._length  = xqos->group_data.length;
    target->value._buffer  = xqos->group_data.value;
    target->value._release = false;
  }
}

static void generate_topic_data (_Out_ DDS_TopicDataQosPolicy *target, _In_ const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_TOPIC_DATA) || (xqos->topic_data.length == 0))
  {
    target->value._maximum = 0;
    target->value._length  = 0;
    target->value._buffer  = NULL;
    target->value._release = false;
  } else {
    target->value._maximum = xqos->topic_data.length;
    target->value._length  = xqos->topic_data.length;
    target->value._buffer  = xqos->topic_data.value;
    target->value._release = false;
  }
}


static void generate_partition_data (_Out_ DDS_PartitionQosPolicy *target, _In_ const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_PARTITION) || (xqos->topic_data.length == 0))
  {
    target->name._maximum = 0;
    target->name._length  = 0;
    target->name._buffer  = NULL;
    target->name._release = false;
  } else {
    target->name._maximum = xqos->partition.n;
    target->name._length  = xqos->partition.n;
    target->name._buffer  = xqos->partition.strs;
    target->name._release = false;
  }
}


static void
generate_product_data(
        _Out_ DDS_ProductDataQosPolicy *a,
        _In_ const struct entity_common *participant,
        _In_ const nn_plist_t *plist)
{
  /* replicate format generated in v_builtinCreateCMParticipantInfo() */
  static const char product_tag[] = "Product";
  static const char exec_name_tag[] = "ExecName";
  static const char participant_name_tag[] = "ParticipantName";
  static const char process_id_tag[] = "PID";
  static const char node_name_tag[] = "NodeName";
  static const char federation_id_tag[] = "FederationId";
  static const char vendor_id_tag[] = "VendorId";
  static const char service_type_tag[] = "ServiceType";
  const size_t cdata_overhead = 12; /* <![CDATA[ and ]]> */
  const size_t tag_overhead = 5; /* <> and </> */
  char pidstr[11]; /* unsigned 32-bits, so max < 5e9, or 10 chars + terminator */
  char federationidstr[20]; /* max 2 * unsigned 32-bits hex + separator, terminator */
  char vendoridstr[22]; /* max 2 * unsigned 32-bits + seperator, terminator */
  char servicetypestr[11]; /* unsigned 32-bits */
  unsigned servicetype;
  size_t len = 1 + 2*(sizeof(product_tag)-1) + tag_overhead;

  if (plist->present & PP_PRISMTECH_EXEC_NAME)
    len += 2*(sizeof(exec_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->exec_name);
  if (plist->present & PP_ENTITY_NAME)
    len += 2*(sizeof(participant_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->entity_name);
  if (plist->present & PP_PRISMTECH_PROCESS_ID)
  {
    int n = snprintf (pidstr, sizeof (pidstr), "%u", plist->process_id);
    assert (n > 0 && (size_t) n < sizeof (pidstr));
    len += 2*(sizeof(process_id_tag)-1) + tag_overhead + (size_t) n;
  }
  if (plist->present & PP_PRISMTECH_NODE_NAME)
    len += 2*(sizeof(node_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->node_name);

  {
    int n = snprintf (vendoridstr, sizeof (vendoridstr), "%u.%u", plist->vendorid.id[0], plist->vendorid.id[1]);
    assert (n > 0 && (size_t) n < sizeof (vendoridstr));
    len += 2*(sizeof(vendor_id_tag)-1) + tag_overhead + (size_t) n;
  }

  {
    int n;
    if (vendor_is_opensplice (plist->vendorid))
      n = snprintf (federationidstr, sizeof (federationidstr), "%x", participant->guid.prefix.u[0]);
    else
      n = snprintf (federationidstr, sizeof (federationidstr), "%x:%x", participant->guid.prefix.u[0], participant->guid.prefix.u[1]);
    assert (n > 0 && (size_t) n < sizeof (federationidstr));
    len += 2*(sizeof(federation_id_tag)-1) + tag_overhead + (size_t) n;
  }

  if (plist->present & PP_PRISMTECH_SERVICE_TYPE)
    servicetype = plist->service_type;
  else
    servicetype = 0;

  {
    int n = snprintf (servicetypestr, sizeof (servicetypestr), "%u", (unsigned) servicetype);
    assert (n > 0 && (size_t) n < sizeof (servicetypestr));
    len += 2*(sizeof(service_type_tag)-1) + tag_overhead + (size_t) n;
  }

  a->value = os_malloc(len);

  {
    char *p = a->value;
    int n;
    n = snprintf (p, len, "<%s>", product_tag); assert (n >= 0 && (size_t) n < len); p += n; len -= (size_t) n;
    if (plist->present & PP_PRISMTECH_EXEC_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", exec_name_tag, plist->exec_name, exec_name_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_ENTITY_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", participant_name_tag, plist->entity_name, participant_name_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_PRISMTECH_PROCESS_ID)
    {
      n = snprintf (p, len, "<%s>%s</%s>", process_id_tag, pidstr, process_id_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_PRISMTECH_NODE_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", node_name_tag, plist->node_name, node_name_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    n = snprintf (p, len, "<%s>%s</%s>", federation_id_tag, federationidstr, federation_id_tag);
    assert (n >= 0 && (size_t) n < len);
    p += n; len -= (size_t) n;
    n = snprintf (p, len, "<%s>%s</%s>", vendor_id_tag, vendoridstr, vendor_id_tag);
    assert (n >= 0 && (size_t) n < len);
    p += n; len -= (size_t) n;

    {
      n = snprintf (p, len, "<%s>%s</%s>", service_type_tag, servicetypestr, service_type_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }

    n = snprintf (p, len, "</%s>", product_tag);
    assert (n >= 0 && (size_t) n == len-1);
    (void) n;
  }
}

static void generate_key (_Out_ DDS_BuiltinTopicKey_t *a, _In_ const nn_guid_prefix_t *gid)
{
  (*a)[0] = gid->u[0];
  (*a)[1] = gid->u[1];
  (*a)[2] = gid->u[2];
}

void
propagate_builtin_topic_participant(
        _In_ const struct entity_common *participant,
        _In_ const nn_plist_t *plist,
        _In_ nn_wctime_t timestamp,
        _In_ int alive)
{
  DDS_ParticipantBuiltinTopicData data;
  generate_key(&(data.key), &(participant->guid.prefix));
  generate_user_data(&(data.user_data), &(plist->qos));
  forward_builtin_participant(&data, timestamp, alive);
}

void
propagate_builtin_topic_cmparticipant(
        _In_ const struct entity_common *participant,
        _In_ const nn_plist_t *plist,
        _In_ nn_wctime_t timestamp,
        _In_ int alive)
{
  DDS_CMParticipantBuiltinTopicData data;
  generate_key(&(data.key), &(participant->guid.prefix));
  generate_product_data(&(data.product), participant, plist);
  forward_builtin_cmparticipant(&data, timestamp, alive);
  os_free(data.product.value);
}




static int copy_topic_name (_Out_ char **target, const nn_xqos_t *xqos)
{
  printf("QP_TOPIC_NAME: %lu",xqos->present & QP_TOPIC_NAME);
  *target = os_strdup(xqos->topic_name);
  return (*target == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int copy_type_name (_Out_ char **target, const nn_xqos_t *xqos)
{
  printf("QP_TYPE_NAME: %lu",xqos->present & QP_TYPE_NAME);
  *target = os_strdup(xqos->type_name);
  return (*target == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int copy_entity_name (_Out_ char **target, const char *name)
{
  *target = os_strdup (name);
  return (*target == NULL) ? ERR_OUT_OF_MEMORY : 0;
}


static DDS_Duration_t ddsi_duration_to_dds_duration (nn_duration_t ddsi_time)
{
  const DDS_Duration_t inf = DDS_Duration_INFINITE;
  int64_t dds_duration = nn_from_ddsi_duration (ddsi_time);
  if (dds_duration == T_NEVER)
    return inf;
  else
  {
    DDS_Duration_t builtin_topic_duration;
    builtin_topic_duration.sec = (long) (dds_duration / T_SECOND);
    builtin_topic_duration.nanosec = (ulong) (dds_duration % T_SECOND);
    return builtin_topic_duration;
  }
}

static int copy_share_policy (_Out_ struct DDS_ShareQosPolicy *target, const nn_xqos_t *xqos)
{
  //printf("QP_PR: %lu",xqos->present & QP_PR);
  target->enable = xqos->share.enable;
  target->name = os_strdup( xqos->share.name);
  return (target->name == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static void copy_entity_factory (_Out_ struct DDS_EntityFactoryQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_PRISMTECH_ENTITY_FACTORY: %lu",xqos->present & QP_PRISMTECH_ENTITY_FACTORY);
  target->autoenable_created_entities = xqos->entity_factory.autoenable_created_entities;
}

static void copy_durability_policy (_Out_ struct DDS_DurabilityQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_DURABILITY: %lu",xqos->present & QP_DURABILITY);
  switch (xqos->durability.kind)
  {
    case NN_VOLATILE_DURABILITY_QOS:
      target->kind = DDS_VOLATILE_DURABILITY_QOS;
      break;
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS:
      target->kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case NN_TRANSIENT_DURABILITY_QOS:
      target->kind = DDS_TRANSIENT_DURABILITY_QOS;
      break;
    case NN_PERSISTENT_DURABILITY_QOS:
      target->kind = DDS_PERSISTENT_DURABILITY_QOS;
      break;
  }
}

static void copy_durability_service_policy (_Out_ struct DDS_DurabilityServiceQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_DURABILITY_SERVICE: %lu",xqos->present & QP_DURABILITY_SERVICE);
  switch (xqos->durability_service.history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      target->history_kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      target->history_kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
  }
  target->history_depth = xqos->durability_service.history.depth;
  target->max_instances = xqos->durability_service.resource_limits.max_instances;
  target->max_samples = xqos->durability_service.resource_limits.max_samples;
  target->max_samples_per_instance = xqos->durability_service.resource_limits.max_samples_per_instance;
  target->service_cleanup_delay = ddsi_duration_to_dds_duration (xqos->durability_service.service_cleanup_delay);
}

static void copy_deadline_policy (_Out_ struct DDS_DeadlineQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_DEADLINE: %lu",xqos->present & QP_DEADLINE);
  target->period = ddsi_duration_to_dds_duration (xqos->deadline.deadline);
}

static void copy_latency_budget_policy (_Out_ struct DDS_LatencyBudgetQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_LATENCY_BUDGET: %lu",xqos->present & QP_LATENCY_BUDGET);
  target->duration = ddsi_duration_to_dds_duration (xqos->latency_budget.duration);
}

static void copy_liveliness_policy (_Out_ struct DDS_LivelinessQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_LIVELINESS: %lu",xqos->present & QP_LIVELINESS);
  switch (xqos->liveliness.kind)
  {
    case NN_AUTOMATIC_LIVELINESS_QOS:
      target->kind = DDS_AUTOMATIC_LIVELINESS_QOS;
      break;
    case NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      target->kind = DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      break;
    case NN_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      target->kind = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
  }
  target->lease_duration = ddsi_duration_to_dds_duration (xqos->liveliness.lease_duration);
}

static void copy_reliability_policy (_Out_ struct DDS_ReliabilityQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_RELIABILITY: %lu",xqos->present & QP_RELIABILITY);
  printf("QP_PRISMTECH_SYNCHRONOUS_ENDPOINT: %lu",xqos->present & QP_PRISMTECH_SYNCHRONOUS_ENDPOINT);
  switch (xqos->reliability.kind)
  {
    case NN_BEST_EFFORT_RELIABILITY_QOS:
      target->kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case NN_RELIABLE_RELIABILITY_QOS:
      target->kind = DDS_RELIABLE_RELIABILITY_QOS;
      break;
  }
  target->max_blocking_time = ddsi_duration_to_dds_duration (xqos->reliability.max_blocking_time);
  target->synchronous = xqos->synchronous_endpoint.value;
}

static void copy_transport_priority_policy (_Out_ struct DDS_TransportPriorityQosPolicy *target, const nn_xqos_t *xqos)
{
  target->value = xqos->transport_priority.value;
}

static void copy_lifespan_policy (_Out_ struct DDS_LifespanQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_LIFESPAN: %lu",xqos->present & QP_LIFESPAN);
    target->duration = ddsi_duration_to_dds_duration (xqos->lifespan.duration);
}

static void copy_destination_order_policy (_Out_ struct DDS_DestinationOrderQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_DESTINATION_ORDER: %lu",xqos->present & QP_DESTINATION_ORDER);
  switch (xqos->destination_order.kind)
  {
    case NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
      target->kind = DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
      break;
    case NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
      target->kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
      break;
  }
}

static void copy_history_policy (_Out_ struct DDS_HistoryQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_HISTORY: %lu",xqos->present & QP_HISTORY);
  switch (xqos->history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      target->kind = DDS_KEEP_LAST_HISTORY_QOS;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      target->kind = DDS_KEEP_ALL_HISTORY_QOS;
      break;
  }
  target->depth = xqos->history.depth;
}

static void copy_resource_limits_policy (_Out_ struct DDS_ResourceLimitsQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_RESOURCE_LIMITS: %lu",xqos->present & QP_RESOURCE_LIMITS);
  target->max_instances = xqos->resource_limits.max_instances;
  target->max_samples = xqos->resource_limits.max_samples;
  target->max_samples_per_instance = xqos->resource_limits.max_samples_per_instance;
}

static void copy_ownership_policy (_Out_ struct DDS_OwnershipQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_OWNERSHIP: %lu",xqos->present & QP_OWNERSHIP);
  switch (xqos->ownership.kind)
  {
    case NN_SHARED_OWNERSHIP_QOS:
      target->kind = DDS_SHARED_OWNERSHIP_QOS;
      break;
    case NN_EXCLUSIVE_OWNERSHIP_QOS:
      target->kind = DDS_EXCLUSIVE_OWNERSHIP_QOS;
      break;
  }
}

static void copy_ownership_strength_policy (_Out_ struct DDS_OwnershipStrengthQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_OWNERSHIP_STRENGTH: %lu",xqos->present & QP_OWNERSHIP_STRENGTH);
  target->value = xqos->ownership_strength.value;
}

/*
static void copy_time_based_filter_policy (struct v_pacingPolicy *a, const nn_xqos_t *xqos)
{
  printf("QP_TIME_BASED_FILTER: %lu",xqos->present & QP_TIME_BASED_FILTER);
  a->minSeperation = ddsi_duration_to_dds_duration (xqos->time_based_filter.minimum_separation);
}
*/
static void copy_presentation_policy (_Out_ struct DDS_PresentationQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_PRESENTATION: %lu",xqos->present & QP_PRESENTATION);
  switch (xqos->presentation.access_scope)
  {
    case NN_INSTANCE_PRESENTATION_QOS:
      target->access_scope = DDS_INSTANCE_PRESENTATION_QOS;
      break;
    case NN_TOPIC_PRESENTATION_QOS:
      target->access_scope = DDS_TOPIC_PRESENTATION_QOS;
      break;
    case NN_GROUP_PRESENTATION_QOS:
      target->access_scope = DDS_GROUP_PRESENTATION_QOS;
      break;
  }
  target->coherent_access = xqos->presentation.coherent_access;
  target->ordered_access = xqos->presentation.ordered_access;
}


/*
static int copy_partition_policy (_Out_ struct DDS_PartitionQosPolicy *target, const nn_xqos_t *xqos)
{
  const int present = (xqos->present & QP_PARTITION) != 0;
  c_type type = c_string_t (gv.ospl_base);
  target->name = c_arrayNew_s (type, (!present || xqos->partition.n == 0) ? 1 : xqos->partition.n);
  if (target->name == NULL)
    return ERR_OUT_OF_MEMORY;
  else
  {
    c_string *ns = (c_string *) target->name;
    if (!present || xqos->partition.n == 0)
    {
      if ((ns[0] = c_stringNew_s (gv.ospl_base, "")) == NULL)
        return ERR_OUT_OF_MEMORY;
    }
    else
    {
      unsigned i;
      for (i = 0; i < xqos->partition.n; i++)
      {
        if ((ns[i] = c_stringNew_s (gv.ospl_base, xqos->partition.strs[i])) == NULL)
          return ERR_OUT_OF_MEMORY;
      }
    }
  }
  return 0;
}
*/
static void copy_writer_data_lifecycle_policy (_Out_ struct DDS_WriterDataLifecycleQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_PRISMTECH_WRITER_DATA_LIFECYCLE: %lu",xqos->present & QP_PRISMTECH_WRITER_DATA_LIFECYCLE);
  target->autodispose_unregistered_instances = xqos->writer_data_lifecycle.autodispose_unregistered_instances;
  target->autopurge_suspended_samples_delay = ddsi_duration_to_dds_duration (xqos->writer_data_lifecycle.autopurge_suspended_samples_delay);
  target->autounregister_instance_delay = ddsi_duration_to_dds_duration (xqos->writer_data_lifecycle.autounregister_instance_delay);
}

static void copy_reader_data_lifecycle_policy (_Out_ struct DDS_ReaderDataLifecycleQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_PRISMTECH_READER_DATA_LIFECYCLE: %lu",xqos->present & QP_PRISMTECH_READER_DATA_LIFECYCLE);
  target->autopurge_nowriter_samples_delay = ddsi_duration_to_dds_duration (xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
  target->autopurge_disposed_samples_delay = ddsi_duration_to_dds_duration (xqos->reader_data_lifecycle.autopurge_disposed_samples_delay);
  target->autopurge_dispose_all = xqos->reader_data_lifecycle.autopurge_dispose_all;
  target->enable_invalid_samples = xqos->reader_data_lifecycle.enable_invalid_samples;
  switch (xqos->reader_data_lifecycle.invalid_sample_visibility)
  {
    case NN_NO_INVALID_SAMPLE_VISIBILITY_QOS:
      target->invalid_sample_visibility.kind = DDS_NO_INVALID_SAMPLES;
      break;
    case NN_MINIMUM_INVALID_SAMPLE_VISIBILITY_QOS:
      target->invalid_sample_visibility.kind = DDS_MINIMUM_INVALID_SAMPLES;
      break;
    case NN_ALL_INVALID_SAMPLE_VISIBILITY_QOS:
      target->invalid_sample_visibility.kind = DDS_ALL_INVALID_SAMPLES;
      break;
  }
  //TODO: test this function!
}

static void copy_reader_lifespan_policy (_Out_ struct DDS_ReaderLifespanQosPolicy *target, const nn_xqos_t *xqos)
{
  printf("QP_PRISMTECH_READER_LIFESPAN: %lu",xqos->present & QP_PRISMTECH_READER_LIFESPAN);
  target->use_lifespan = xqos->reader_lifespan.use_lifespan;
  target->duration = ddsi_duration_to_dds_duration (xqos->reader_lifespan.duration);
}
/*
static int copy_subscription_keys_policy (struct v_userKeyPolicy *a, const nn_xqos_t *xqos)
{
  printf("QP_PRISMTECH_SUBSCRIPTION_KEYS: %lu",xqos->present & QP_PRISMTECH_SUBSCRIPTION_KEYS);
  if (!xqos->subscription_keys.use_key_list || xqos->subscription_keys.key_list.n == 0)
  {
    a->enable = 0;
    a->expression = c_stringNew_s (gv.ospl_base, "");
    if (a->expression == NULL)
      return ERR_OUT_OF_MEMORY;
  }
  else
  {
    os_size_t len;
    char *p;
    unsigned i;
    a->enable = 1;
    // subscription keys in topicInfo is a comma-separated list of keys, terminated by a 0, so string length is simply the length of each key increased by 1
    len = 0;
    for (i = 0; i < xqos->subscription_keys.key_list.n; i++)
      len += strlen (xqos->subscription_keys.key_list.strs[i]) + 1;

    a->expression = p = c_stringMalloc(gv.ospl_base, len);
    if (a->expression == NULL)
      return ERR_OUT_OF_MEMORY;
    for (i = 0; i < xqos->subscription_keys.key_list.n; i++)
    {
      os_size_t sz = strlen (xqos->subscription_keys.key_list.strs[i]);
      memcpy (p, xqos->subscription_keys.key_list.strs[i], sz);
      p += sz;
      *p++ = ','; // for last one still be replaced with terminating 0
    }
    p[-1] = 0;
  }
  return 0;
}
*/

int
propagate_builtin_topic_publication(
        _In_ const struct entity_common *entity,
        _In_ const struct endpoint_common *endpoint,
        _In_ const nn_xqos_t *xqos,
        _In_ nn_wctime_t timestamp,
        _In_ int alive
)
{

// never called for DDSI built-in writers /
  //const nn_xqos_t *xqos = writer->xqos;

  int return_value;

  DDS_PublicationBuiltinTopicData data;
  generate_key(&(data.key), &(entity->guid.prefix));
  //generate_key(&(data.participant_key), &(endpoint->pp->e.guid.prefix)); //TODO: Assign correct values

  /*
    printf("QP_TOPIC_NAME: %lu\n",xqos->present & QP_TOPIC_NAME);
    printf("QP_TYPE_NAME: %lu\n",xqos->present & QP_TYPE_NAME);
    printf("QP_PRISMTECH_ENTITY_FACTORY: %lu\n",xqos->present & QP_PRISMTECH_ENTITY_FACTORY);
    printf("QP_DURABILITY_SERVICE: %lu\n",xqos->present & QP_DURABILITY_SERVICE);
    printf("QP_DEADLINE: %lu\n",xqos->present & QP_DEADLINE);
    printf("QP_LATENCY_BUDGET: %lu\n",xqos->present & QP_LATENCY_BUDGET);
    printf("QP_LIVELINESS: %lu\n",xqos->present & QP_LIVELINESS);
    printf("QP_RELIABILITY: %lu\n",xqos->present & QP_RELIABILITY);
    printf("QP_PRISMTECH_SYNCHRONOUS_ENDPOINT: %lu\n",xqos->present & QP_PRISMTECH_SYNCHRONOUS_ENDPOINT);
    printf("QP_LIFESPAN: %lu\n",xqos->present & QP_LIFESPAN);
    printf("QP_DESTINATION_ORDER: %lu\n",xqos->present & QP_DESTINATION_ORDER);
    printf("QP_RESOURCE_LIMITS: %lu\n",xqos->present & QP_RESOURCE_LIMITS);
    printf("QP_OWNERSHIP: %lu\n\n",xqos->present & QP_OWNERSHIP);
*/
/*
//  Note: topic gets set lazily, so may still be NULL, but the topic name is in the QoS
  if (copy_topic_name(&data.topic_name, xqos) < 0)
  {
    return_value = ERR_OUT_OF_MEMORY;
  } else if (copy_type_name(&data.type_name, xqos) < 0)
  {
    return_value = ERR_OUT_OF_MEMORY;
  } else
  {


    generate_partition_data(&data.partition, xqos);
    copy_durability_policy(&data.durability, xqos);
    copy_deadline_policy(&data.deadline, xqos);
    copy_latency_budget_policy(&data.latency_budget, xqos);
    copy_liveliness_policy(&data.liveliness, xqos);
    copy_reliability_policy(&data.reliability, xqos);
    copy_lifespan_policy(&data.lifespan, xqos);
    copy_destination_order_policy(&data.destination_order, xqos);

    generate_user_data(&data.user_data, xqos);
    generate_group_data(&data.group_data, xqos);
    generate_topic_data(&data.topic_data, xqos);

    copy_ownership_policy(&data.ownership, xqos);
    copy_ownership_strength_policy(&data.ownership_strength, xqos);
    copy_presentation_policy(&data.presentation, xqos);

    forward_builtin_publication(&data, timestamp, alive);
  }

    */
  return return_value;
}
/*
void propagate_builtin_topic_cmdatawriter(
        _In_ const struct entity_common *participant,
        _In_ const nn_plist_t *plist,
        _In_ nn_wctime_t timestamp,
        _In_ int alive
)
{
//  never called for DDSI built-in writers
const struct proxy_writer *pwr = vpwr;
const nn_xqos_t *xqos = pwr->c.xqos;
struct v_dataWriterCMInfo *to = vto;

static const char product_tag[] = "Product";
static const char transport_priority_tag[] = "transport_priority";
static const char rti_typecode_tag[] = "rti_typecode";
const size_t tag_overhead = 5; //  <> and </>
char tpriostr[12]; //  signed 32-bits, so min ~= -2e9, or 11 chars + terminator
size_t len = 1 + 2 * (sizeof(product_tag) - 1) + tag_overhead;
int product_string_needed = 0;

if (xqos->transport_priority.value != 0) {
int n = snprintf(tpriostr, sizeof(tpriostr), "%d", (int) xqos->transport_priority.value);
assert (n > 0 && (size_t) n < sizeof(tpriostr));
len += 2*(sizeof(transport_priority_tag)-1) + tag_overhead + (size_t)
n;
product_string_needed = 1;
}

if (xqos->present & QP_RTI_TYPECODE) {
len += 2*(sizeof(rti_typecode_tag)-1) + tag_overhead +
base64_length(xqos
->rti_typecode.length);
product_string_needed = 1;
}

data.
key = pwr->c.gid;
if (product_string_needed == 0)
data.product.
value = c_stringNew(gv.ospl_base, "");
else
{
char *p;
int n;
if ((data.product.
value = c_stringMalloc_s(gv.ospl_base, len)
) == NULL)
return
V_COPYIN_RESULT_OUT_OF_MEMORY;
p = data.product.value;
n = snprintf(p, len, "<%s>", product_tag); assert (n >= 0 && (size_t) n < len); p +=
n;
len -= (size_t)
n;
if (xqos->transport_priority.value != 0) {
n = snprintf(p, len, "<%s>%s</%s>", transport_priority_tag, tpriostr, transport_priority_tag);
assert (n >= 0 && (size_t) n < len);
p +=
n;
len -= (size_t)
n;
}
if (xqos->present & QP_RTI_TYPECODE)
{
n = snprintf(p, len, "<%s>", rti_typecode_tag);
assert (n >= 0 && (size_t) n < len);
p +=
n;
len -= (size_t)
n;
n = base64_encode(p, len, xqos->rti_typecode.value, xqos->rti_typecode.length);
assert (n >= 0 && (size_t) n < len);
p +=
n;
len -= (size_t)
n;
n = snprintf(p, len, "</%s>", rti_typecode_tag);
assert (n >= 0 && (size_t) n < len);
p +=
n;
len -= (size_t)
n;
}
n = snprintf(p, len, "</%s>", product_tag);
assert (n >= 0 && (os_size_t) n == len - 1);
(void)
n;
}
data.
publisher_key = pwr->c.group_gid;
if (copy_entity_name (&data.name, pwr->e.name) < 0)
return
V_COPYIN_RESULT_OUT_OF_MEMORY;
copy_history_policy (&data.history, xqos);
copy_resource_limits_policy (&data.resource_limits, xqos);
copy_writer_data_lifecycle_policy (&data.writer_data_lifecycle, xqos);
return
V_COPYIN_RESULT_OK;
}

*/