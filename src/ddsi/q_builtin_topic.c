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

#include "ddsi/q_misc.h"
#include "ddsi/q_config.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_builtin_topic.h"

#include "dds_builtinTopics.h"

static void generate_user_data (_Out_ DDS_UserDataQosPolicy *a, _In_ const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_USER_DATA) || (xqos->user_data.length == 0))
  {
    a->value._maximum = 0;
    a->value._length  = 0;
    a->value._buffer  = NULL;
    a->value._release = false;
  } else {
    a->value._maximum = xqos->user_data.length;
    a->value._length  = xqos->user_data.length;
    a->value._buffer  = xqos->user_data.value;
    a->value._release = false;
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



static int qpc_topic_name (char *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TOPIC_NAME);
  //*a = c_stringNew_s (gv.ospl_base, xqos->topic_name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

/*
static int qpc_type_name (c_string *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TYPE_NAME);
  *a = c_stringNew_s (gv.ospl_base, xqos->type_name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int qpc_entity_name (c_string *a, const char *name)
{
  *a = c_stringNew_s (gv.ospl_base, name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int qpc_share_policy (struct v_sharePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_SHARE);
  a->enable = xqos->share.enable;
  a->name = c_stringNew_s (gv.ospl_base, xqos->share.name);
  return (a->name == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static void qpc_entity_factory (struct v_entityFactoryPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_ENTITY_FACTORY);
  a->autoenable_created_entities = xqos->entity_factory.autoenable_created_entities;
}

static void qpc_durability_policy (struct v_durabilityPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DURABILITY);
  switch (xqos->durability.kind)
  {
    case NN_VOLATILE_DURABILITY_QOS:
      a->kind = V_DURABILITY_VOLATILE;
      break;
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS:
      a->kind = V_DURABILITY_TRANSIENT_LOCAL;
      break;
    case NN_TRANSIENT_DURABILITY_QOS:
      a->kind = V_DURABILITY_TRANSIENT;
      break;
    case NN_PERSISTENT_DURABILITY_QOS:
      a->kind = V_DURABILITY_PERSISTENT;
      break;
  }
}

static void qpc_durability_service_policy (struct v_durabilityServicePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DURABILITY_SERVICE);
  switch (xqos->durability_service.history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      a->history_kind = V_HISTORY_KEEPLAST;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      a->history_kind = V_HISTORY_KEEPALL;
      break;
  }
  a->history_depth = xqos->durability_service.history.depth;
  a->max_instances = xqos->durability_service.resource_limits.max_instances;
  a->max_samples = xqos->durability_service.resource_limits.max_samples;
  a->max_samples_per_instance = xqos->durability_service.resource_limits.max_samples_per_instance;
  a->service_cleanup_delay = ddsi_duration_to_v_duration (xqos->durability_service.service_cleanup_delay);
}

static void qpc_deadline_policy (struct v_deadlinePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DEADLINE);
  a->period = ddsi_duration_to_v_duration (xqos->deadline.deadline);
}

static void qpc_latency_budget_policy (struct v_latencyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LATENCY_BUDGET);
  a->duration = ddsi_duration_to_v_duration (xqos->latency_budget.duration);
}

static void qpc_liveliness_policy (struct v_livelinessPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LIVELINESS);
  switch (xqos->liveliness.kind)
  {
    case NN_AUTOMATIC_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_AUTOMATIC;
      break;
    case NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_PARTICIPANT;
      break;
    case NN_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_TOPIC;
      break;
  }
  a->lease_duration = ddsi_duration_to_v_duration (xqos->liveliness.lease_duration);
}

static void qpc_reliability_policy (struct v_reliabilityPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_RELIABILITY);
  assert (xqos->present & QP_PRISMTECH_SYNCHRONOUS_ENDPOINT);
  switch (xqos->reliability.kind)
  {
    case NN_BEST_EFFORT_RELIABILITY_QOS:
      a->kind = V_RELIABILITY_BESTEFFORT;
      break;
    case NN_RELIABLE_RELIABILITY_QOS:
      a->kind = V_RELIABILITY_RELIABLE;
      break;
  }
  a->max_blocking_time = ddsi_duration_to_v_duration (xqos->reliability.max_blocking_time);
  a->synchronous = xqos->synchronous_endpoint.value;
}

static void qpc_transport_priority_policy (struct v_transportPolicy *a, const nn_xqos_t *xqos)
{
  a->value = xqos->transport_priority.value;
}

static void qpc_lifespan_policy (struct v_lifespanPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LIFESPAN);
  a->duration = ddsi_duration_to_v_duration (xqos->lifespan.duration);
}

static void qpc_destination_order_policy (struct v_orderbyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DESTINATION_ORDER);
  switch (xqos->destination_order.kind)
  {
    case NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
      a->kind = V_ORDERBY_RECEPTIONTIME;
      break;
    case NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
      a->kind = V_ORDERBY_SOURCETIME;
      break;
  }
}

static void qpc_history_policy (struct v_historyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_HISTORY);
  switch (xqos->history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      a->kind = V_HISTORY_KEEPLAST;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      a->kind = V_HISTORY_KEEPALL;
      break;
  }
  a->depth = xqos->history.depth;
}

static void qpc_resource_limits_policy (struct v_resourcePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_RESOURCE_LIMITS);
  a->max_instances = xqos->resource_limits.max_instances;
  a->max_samples = xqos->resource_limits.max_samples;
  a->max_samples_per_instance = xqos->resource_limits.max_samples_per_instance;
}


static int qpc_user_data_policy (struct v_builtinUserDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_USER_DATA) || xqos->user_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->user_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->user_data.value, xqos->user_data.length);
  return 0;
}

static void qpc_ownership_policy (struct v_ownershipPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_OWNERSHIP);
  switch (xqos->ownership.kind)
  {
    case NN_SHARED_OWNERSHIP_QOS:
      a->kind = V_OWNERSHIP_SHARED;
      break;
    case NN_EXCLUSIVE_OWNERSHIP_QOS:
      a->kind = V_OWNERSHIP_EXCLUSIVE;
      break;
  }
}

static void qpc_ownership_strength_policy (struct v_strengthPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_OWNERSHIP_STRENGTH);
  a->value = xqos->ownership_strength.value;
}

static void qpc_time_based_filter_policy (struct v_pacingPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TIME_BASED_FILTER);
  a->minSeperation = ddsi_duration_to_v_duration (xqos->time_based_filter.minimum_separation);
}

static void qpc_presentation_policy (struct v_presentationPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRESENTATION);
  switch (xqos->presentation.access_scope)
  {
    case NN_INSTANCE_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_INSTANCE;
      break;
    case NN_TOPIC_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_TOPIC;
      break;
    case NN_GROUP_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_GROUP;
      break;
  }
  a->coherent_access = xqos->presentation.coherent_access;
  a->ordered_access = xqos->presentation.ordered_access;
}

static int qpc_group_data_policy (struct v_builtinGroupDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_GROUP_DATA) || xqos->group_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->group_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->group_data.value, xqos->group_data.length);
  return 0;
}

static int qpc_topic_data_policy (struct v_builtinTopicDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_TOPIC_DATA) || xqos->topic_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->topic_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->topic_data.value, xqos->topic_data.length);
  return 0;
}

static int qpc_partition_policy (struct v_builtinPartitionPolicy *a, const nn_xqos_t *xqos)
{
  const int present = (xqos->present & QP_PARTITION) != 0;
  c_type type = c_string_t (gv.ospl_base);
  a->name = c_arrayNew_s (type, (!present || xqos->partition.n == 0) ? 1 : xqos->partition.n);
  if (a->name == NULL)
    return ERR_OUT_OF_MEMORY;
  else
  {
    c_string *ns = (c_string *) a->name;
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

static void qpc_writer_data_lifecycle_policy (struct v_writerLifecyclePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_WRITER_DATA_LIFECYCLE);
  a->autodispose_unregistered_instances = xqos->writer_data_lifecycle.autodispose_unregistered_instances;
  a->autopurge_suspended_samples_delay = ddsi_duration_to_v_duration (xqos->writer_data_lifecycle.autopurge_suspended_samples_delay);
  a->autounregister_instance_delay = ddsi_duration_to_v_duration (xqos->writer_data_lifecycle.autounregister_instance_delay);
}

static void qpc_reader_data_lifecycle_policy (struct v_readerLifecyclePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_READER_DATA_LIFECYCLE);
  a->autopurge_nowriter_samples_delay = ddsi_duration_to_v_duration (xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
  a->autopurge_disposed_samples_delay = ddsi_duration_to_v_duration (xqos->reader_data_lifecycle.autopurge_disposed_samples_delay);
  a->autopurge_dispose_all = xqos->reader_data_lifecycle.autopurge_dispose_all;
  a->enable_invalid_samples = xqos->reader_data_lifecycle.enable_invalid_samples;
  switch (xqos->reader_data_lifecycle.invalid_sample_visibility)
  {
    case NN_NO_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
      break;
    case NN_MINIMUM_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
      break;
    case NN_ALL_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_ALL_INVALID_SAMPLES;
      break;
  }
}

static void qpc_reader_lifespan_policy (struct v_readerLifespanPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_READER_LIFESPAN);
  a->used = xqos->reader_lifespan.use_lifespan;
  a->duration = ddsi_duration_to_v_duration (xqos->reader_lifespan.duration);
}

static int qpc_subscription_keys_policy (struct v_userKeyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_SUBSCRIPTION_KEYS);
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
void
propagate_builtin_topic_publication(
        _In_ const struct writer *writer,
        _In_ nn_wctime_t timestamp,
        _In_ int alive
  )
{



// never called for DDSI built-in writers /
  const nn_xqos_t *xqos = writer->xqos;


  DDS_PublicationBuiltinTopicData data;
  generate_key(&(data.key), &(writer->e.guid.prefix));
  //data.participant_key = writer->c.pp->e.guid.entityid; //TODO: Assign correct values


//  Note: topic gets set lazily, so may still be NULL, but the topic name is in the QoS
  /*
  if (qpc_topic_name(data.topic_name,xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_type_name(&to->type_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_durability_policy(&to->durability, xqos);
  qpc_deadline_policy(&to->deadline, xqos);
  qpc_latency_budget_policy(&to->latency_budget, xqos);
  qpc_liveliness_policy(&to->liveliness, xqos);
  qpc_reliability_policy(&to->reliability, xqos);
  qpc_lifespan_policy(&to->lifespan, xqos);
  qpc_destination_order_policy(&to->destination_order, xqos);
  if (qpc_user_data_policy(&to->user_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_ownership_policy(&to->ownership, xqos);
  qpc_ownership_strength_policy(&to->ownership_strength, xqos);
  qpc_presentation_policy(&to->presentation, xqos);
  if (qpc_partition_policy(&to->partition, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_topic_data_policy(&to->topic_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_group_data_policy(&to->group_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_writer_data_lifecycle_policy(&to->lifecycle, xqos);
  to->alive = 1; //  FIXME -- depends on full implementation of liveliness


  forward_builtin_publication(&data, timestamp, alive);
   */
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

to->
key = pwr->c.gid;
if (product_string_needed == 0)
to->product.
value = c_stringNew(gv.ospl_base, "");
else
{
char *p;
int n;
if ((to->product.
value = c_stringMalloc_s(gv.ospl_base, len)
) == NULL)
return
V_COPYIN_RESULT_OUT_OF_MEMORY;
p = to->product.value;
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
to->
publisher_key = pwr->c.group_gid;
if (qpc_entity_name (&to->name, pwr->e.name) < 0)
return
V_COPYIN_RESULT_OUT_OF_MEMORY;
qpc_history_policy (&to->history, xqos);
qpc_resource_limits_policy (&to->resource_limits, xqos);
qpc_writer_data_lifecycle_policy (&to->writer_data_lifecycle, xqos);
return
V_COPYIN_RESULT_OK;
}

*/