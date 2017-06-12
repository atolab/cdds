#include <assert.h>
#include <string.h>
#include "kernel/dds_qos.h"
#include "ddsi/q_config.h"

/* TODO: dd_duration_t is converted to nn_ddsi_time_t declared in q_time.h
   This structure contain seconds and fractions.
   Revisit on the conversion as default values are { 0x7fffffff, 0xffffffff }
*/

static void dds_qos_data_copy_in (nn_octetseq_t * data, const void * __restrict value, size_t sz)
{
  if (data->value)
  {
    dds_free (data->value);
    data->value = NULL;
  }
  data->length = (uint32_t) sz;
  if (sz)
  {
    data->value = dds_alloc (sz);
    memcpy (data->value, value, sz);
  }
}

static void dds_qos_data_copy_out (const nn_octetseq_t * data, void ** value, size_t * sz)
{
  if (value)
  {
    *sz = data->length;
    if (data->value)
    {
      *value = dds_alloc (data->length);
      memcpy (*value, data->value, data->length);
    }
  }
}

bool validate_octetseq (const nn_octetseq_t* seq)
{
  /* default value is NULL with length 0 */
  return (((seq->length == 0) && (seq->value == NULL)) || (seq->length > 0));
}

bool validate_partition_qospolicy (_In_ const nn_partition_qospolicy_t * partition)
{
  /* default value is NULL with length 0 */
  return (((partition->n == 0) && (partition->strs == NULL)) || (partition->n > 0));
}


bool validate_reliability_qospolicy (const nn_reliability_qospolicy_t * reliability)
{
  return
  (
    (reliability->kind == NN_BEST_EFFORT_RELIABILITY_QOS || reliability->kind == NN_RELIABLE_RELIABILITY_QOS) &&
    (validate_duration (&reliability->max_blocking_time) == 0)
  );
}

bool validate_deadline_and_timebased_filter
  (const nn_duration_t deadline, const nn_duration_t minimum_separation)
{
  return (nn_from_ddsi_duration (minimum_separation) <= nn_from_ddsi_duration (deadline));
}

bool dds_qos_validate_common (const dds_qos_t *qos)
{
  return !
  (
    ((qos->present & QP_DURABILITY) && (validate_durability_qospolicy (&qos->durability) != 0)) ||
    ((qos->present & QP_DEADLINE) && (validate_duration (&qos->deadline.deadline) != 0)) ||
    ((qos->present & QP_LATENCY_BUDGET) && (validate_duration (&qos->latency_budget.duration) != 0)) ||
    ((qos->present & QP_OWNERSHIP) && (validate_ownership_qospolicy (&qos->ownership) != 0)) ||
    ((qos->present & QP_LIVELINESS) && (validate_liveliness_qospolicy (&qos->liveliness) != 0)) ||
    ((qos->present & QP_RELIABILITY) && ! validate_reliability_qospolicy (&qos->reliability)) ||
    ((qos->present & QP_DESTINATION_ORDER) && (validate_destination_order_qospolicy (&qos->destination_order) != 0)) ||
    ((qos->present & QP_HISTORY) && (validate_history_qospolicy (&qos->history) != 0)) ||
    ((qos->present & QP_RESOURCE_LIMITS) && (validate_resource_limits_qospolicy (&qos->resource_limits) != 0))
  );
}


/* set qos to default values */

static void dds_qos_init_defaults (dds_qos_t * __restrict qos)
{
  assert (qos);
  memset (qos, 0, sizeof (*qos));
  qos->durability.kind = (nn_durability_kind_t) DDS_DURABILITY_VOLATILE;
  qos->deadline.deadline = nn_to_ddsi_duration (DDS_INFINITY);
  qos->durability_service.service_cleanup_delay = nn_to_ddsi_duration (0);
  qos->durability_service.history.kind = (nn_history_kind_t) DDS_HISTORY_KEEP_LAST;
  qos->durability_service.history.depth = 1;
  qos->durability_service.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
  qos->durability_service.resource_limits.max_instances = DDS_LENGTH_UNLIMITED;
  qos->durability_service.resource_limits.max_samples_per_instance = DDS_LENGTH_UNLIMITED;
  qos->presentation.access_scope = (nn_presentation_access_scope_kind_t) DDS_PRESENTATION_INSTANCE;
  qos->latency_budget.duration = nn_to_ddsi_duration (0);
  qos->ownership.kind = (nn_ownership_kind_t) DDS_OWNERSHIP_SHARED;
  qos->liveliness.kind = (nn_liveliness_kind_t) DDS_LIVELINESS_AUTOMATIC;
  qos->liveliness.lease_duration = nn_to_ddsi_duration (DDS_INFINITY);
  qos->time_based_filter.minimum_separation = nn_to_ddsi_duration (0);
  qos->reliability.kind = (nn_reliability_kind_t) DDS_RELIABILITY_BEST_EFFORT;
  qos->reliability.max_blocking_time = nn_to_ddsi_duration (DDS_MSECS (100));
  qos->lifespan.duration = nn_to_ddsi_duration (DDS_INFINITY);
  qos->destination_order.kind = (nn_destination_order_kind_t) DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP;
  qos->history.kind = (nn_history_kind_t) DDS_HISTORY_KEEP_LAST;
  qos->history.depth = 1;
  qos->resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
  qos->resource_limits.max_instances = DDS_LENGTH_UNLIMITED;
  qos->resource_limits.max_samples_per_instance = DDS_LENGTH_UNLIMITED;
  qos->writer_data_lifecycle.autodispose_unregistered_instances = true;
  qos->reader_data_lifecycle.autopurge_nowriter_samples_delay = nn_to_ddsi_duration (DDS_INFINITY);
  qos->reader_data_lifecycle.autopurge_disposed_samples_delay = nn_to_ddsi_duration (DDS_INFINITY);
}

dds_qos_t * dds_qos_create (void)
{
  dds_qos_t *qos;
  qos = (dds_qos_t*) dds_alloc (sizeof (dds_qos_t));
  dds_qos_init_defaults (qos);
  return qos;
}

void dds_qos_reset (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_fini (qos);
  dds_qos_init_defaults (qos);
}

void dds_qos_delete (dds_qos_t * __restrict qos)
{
  if (qos)
  {
    dds_qos_reset (qos);
    dds_free (qos);
  }
}

dds_return_t dds_qos_copy (dds_qos_t * __restrict dst, const dds_qos_t * __restrict src)
{
  assert (src);
  assert (dst);

  nn_xqos_copy (dst, src);
  return DDS_RETCODE_OK;
}

void dds_qos_merge (dds_qos_t * __restrict dst, const dds_qos_t * __restrict src)
{
  assert (src);
  assert (dst);

  /* Copy qos from source to destination unless already set */
  nn_xqos_mergein_missing (dst, src);
}

void dds_get_default_participant_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  dds_qos_init_defaults (qos);

  qos->present |= QP_USER_DATA;

  /* chk - set as in ddsi, though the default value as per spec is TRUE */
  qos->present |= QP_PRISMTECH_ENTITY_FACTORY;
  qos->entity_factory.autoenable_created_entities = 0;
}

void dds_get_default_topic_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_init_default_topic (qos);
}

void dds_get_default_publisher_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_init_default_publisher (qos);
}

void dds_get_default_subscriber_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_init_default_subscriber (qos);
}

void dds_get_default_writer_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_init_default_writer (qos);
}

void dds_get_default_reader_qos (dds_qos_t * __restrict qos)
{
  assert (qos);
  nn_xqos_init_default_reader (qos);
}

/* set qos could be called during creation or at run time */
/* NOTE: Lite Impln: userdata, topicdata and groupdata should contain valid value and size
   Deviation from Spec, which says default value is empty zero-sized sequence !!
*/
void dds_qset_userdata (dds_qos_t * __restrict qos, const void * __restrict value, size_t sz)
{
  assert (qos);
  assert (value);
  assert (sz);

  dds_qos_data_copy_in (&qos->user_data, value, sz);
  qos->present |= QP_USER_DATA;
}

void dds_qset_topicdata (dds_qos_t * __restrict qos, const void * __restrict value, size_t sz)
{
  assert (qos);
  assert (value);
  assert (sz);

  dds_qos_data_copy_in (&qos->topic_data, value, sz);
  qos->present |= QP_TOPIC_DATA;
}

void dds_qset_groupdata (dds_qos_t * __restrict qos, const void * __restrict value, size_t sz)
{
  assert (qos);
  assert (value);
  assert (sz);

  dds_qos_data_copy_in (&qos->group_data, value, sz);
  qos->present |= QP_GROUP_DATA;
}

void dds_qset_durability (dds_qos_t *qos, dds_durability_kind_t kind)
{
  assert (qos);

  qos->durability.kind = (nn_durability_kind_t) kind;
  qos->present |= QP_DURABILITY;
}

void dds_qset_history (dds_qos_t *qos, dds_history_kind_t kind, int32_t depth)
{
  assert (qos);

  qos->history.kind = (nn_history_kind_t) kind;
  qos->history.depth = depth;
  qos->present |= QP_HISTORY;
}

void dds_qset_resource_limits
  (dds_qos_t *qos, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
{
  assert (qos);

  qos->resource_limits.max_samples = max_samples;
  qos->resource_limits.max_instances = max_instances;
  qos->resource_limits.max_samples_per_instance = max_samples_per_instance;
  qos->present |= QP_RESOURCE_LIMITS;
}

void dds_qset_presentation
  (dds_qos_t *qos, dds_presentation_access_scope_kind_t access_scope, bool coherent_access, bool ordered_access)
{
  assert (qos);

  qos->presentation.access_scope = (nn_presentation_access_scope_kind_t) access_scope;
  qos->presentation.coherent_access = coherent_access;
  qos->presentation.ordered_access = ordered_access;
  qos->present |= QP_PRESENTATION;
}

void dds_qset_lifespan (dds_qos_t *qos, dds_duration_t lifespan)
{
  assert (qos);
  /* convert int64_t to nn_duration_t */
  qos->lifespan.duration = nn_to_ddsi_duration (lifespan);
  qos->present |= QP_LIFESPAN;
}

void dds_qset_deadline (dds_qos_t * qos, dds_duration_t deadline)
{
  assert (qos);
  /* convert int64_t to nn_duration_t */
  qos->deadline.deadline = nn_to_ddsi_duration (deadline);
  qos->present |= QP_DEADLINE;
}

void dds_qset_latency_budget (dds_qos_t * qos, dds_duration_t duration)
{
  assert (qos);
  /* convert int64_t to nn_duration_t */
  qos->latency_budget.duration = nn_to_ddsi_duration (duration);
  qos->present |= QP_LATENCY_BUDGET;
}

void dds_qset_ownership (dds_qos_t *qos, dds_ownership_kind_t kind)
{
  assert (qos);

  qos->ownership.kind = (nn_ownership_kind_t) kind;
  qos->present |= QP_OWNERSHIP;
}

void dds_qset_ownership_strength (dds_qos_t * qos, int32_t value)
{
  assert (qos);

  qos->ownership_strength.value = value;
  qos->present |= QP_OWNERSHIP_STRENGTH;
}

void dds_qset_liveliness (dds_qos_t *qos, dds_liveliness_kind_t kind, dds_duration_t lease_duration)
{
  assert (qos);

  qos->liveliness.kind = (nn_liveliness_kind_t) kind;
  /* convert int64_t to nn_duration_t */
  qos->liveliness.lease_duration = nn_to_ddsi_duration (lease_duration);
  qos->present |= QP_LIVELINESS;
}

void dds_qset_time_based_filter (dds_qos_t *qos, dds_duration_t minimum_separation)
{
  assert (qos);

  /* convert int64_t to struct */
  qos->time_based_filter.minimum_separation = nn_to_ddsi_duration (minimum_separation);
  qos->present |= QP_TIME_BASED_FILTER;
}

void dds_qset_partition (dds_qos_t * __restrict qos, uint32_t n, const char ** ps)
{
  uint32_t i;
  size_t len;

  assert (qos);
  assert (ps);
  if (qos->partition.strs != NULL){
	  for (uint32_t i = 0; i < qos->partition.n; i++)
	  {
	    dds_free(qos->partition.strs[i]);
	  }
	  dds_free(qos->partition.strs);
	  qos->partition.strs == NULL;
  }
  qos->partition.n = n;
  qos->partition.strs = dds_alloc (sizeof (char*) * n);

  for (i = 0; i < n; i++)
  {
    len = strlen (ps[i]) + 1;
    qos->partition.strs[i] = dds_alloc (len);
    strncpy (qos->partition.strs[i], ps[i], len);
  }
  qos->present |= QP_PARTITION;
}

void dds_qset_reliability (dds_qos_t *qos, dds_reliability_kind_t kind, dds_duration_t max_blocking_time)
{
  assert (qos);

  qos->reliability.kind = (nn_reliability_kind_t) kind;
  /* convert int64_t to nn_duration_t */
  qos->reliability.max_blocking_time = nn_to_ddsi_duration (max_blocking_time);
  qos->present |= QP_RELIABILITY;
}

void dds_qset_transport_priority (dds_qos_t * qos, int32_t value)
{
  assert (qos);

  qos->transport_priority.value = value;
  qos->present |= QP_TRANSPORT_PRIORITY;
}

void dds_qset_destination_order (dds_qos_t * qos, dds_destination_order_kind_t kind)
{
  assert (qos);

  qos->destination_order.kind = (nn_destination_order_kind_t) kind;
  qos->present |= QP_DESTINATION_ORDER;
}

void dds_qset_writer_data_lifecycle (dds_qos_t * qos, bool autodispose)
{
  assert (qos);

  qos->writer_data_lifecycle.autodispose_unregistered_instances = autodispose;
  qos->present |= QP_PRISMTECH_WRITER_DATA_LIFECYCLE;
}

void dds_qset_reader_data_lifecycle
  (dds_qos_t * qos, dds_duration_t autopurge_nowriter_samples, dds_duration_t autopurge_disposed_samples_delay)
{
  assert (qos);
  /* convert int64_t to nn_duration_t */
  qos->reader_data_lifecycle.autopurge_nowriter_samples_delay = \
    nn_to_ddsi_duration (autopurge_nowriter_samples);
  qos->reader_data_lifecycle.autopurge_disposed_samples_delay = \
    nn_to_ddsi_duration (autopurge_disposed_samples_delay);
  qos->present |= QP_PRISMTECH_READER_DATA_LIFECYCLE;
}

void dds_qset_durability_service
(
  dds_qos_t * qos,
  dds_duration_t service_cleanup_delay,
  dds_history_kind_t history_kind,
  int32_t history_depth,
  int32_t max_samples,
  int32_t max_instances,
  int32_t max_samples_per_instance
)
{
  assert (qos);
  qos->durability_service.service_cleanup_delay = nn_to_ddsi_duration (service_cleanup_delay);
  qos->durability_service.history.kind = (nn_history_kind_t) history_kind;
  qos->durability_service.history.depth = history_depth;
  qos->durability_service.resource_limits.max_samples = max_samples;
  qos->durability_service.resource_limits.max_instances = max_instances;
  qos->durability_service.resource_limits.max_samples_per_instance = max_samples_per_instance;
  qos->present |= QP_DURABILITY_SERVICE;
}

void dds_qget_userdata (const dds_qos_t * qos, void ** value, size_t * sz)
{
  assert (qos);
  dds_qos_data_copy_out (&qos->user_data, value, sz);
}

void dds_qget_topicdata (const dds_qos_t * qos, void ** value, size_t * sz)
{
  assert (qos);
  dds_qos_data_copy_out (&qos->topic_data, value, sz);
}

void dds_qget_groupdata (const dds_qos_t * qos, void ** value, size_t * sz)
{
  assert (qos);
  dds_qos_data_copy_out (&qos->group_data, value, sz);
}

void dds_qget_durability (const dds_qos_t * qos, dds_durability_kind_t *kind)
{
  assert (qos);
  if (kind)
  {
    *kind = (dds_durability_kind_t) qos->durability.kind;
  }
}

void dds_qget_history (const dds_qos_t * qos, dds_history_kind_t * kind, int32_t *depth)
{
  assert (qos);
  if (kind) *kind = (dds_history_kind_t) qos->history.kind;
  if (depth) *depth = qos->history.depth;
}

void dds_qget_resource_limits
  (const dds_qos_t * qos, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance)
{
  assert (qos);
  if (max_samples) *max_samples = qos->resource_limits.max_samples;
  if (max_instances) *max_instances = qos->resource_limits.max_instances;
  if (max_samples_per_instance)
  {
    *max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
  }
}

void dds_qget_presentation
  (const dds_qos_t * qos, dds_presentation_access_scope_kind_t *access_scope, bool *coherent_access, bool *ordered_access)
{
  assert (qos);
  if (access_scope) *access_scope = (dds_presentation_access_scope_kind_t) qos->presentation.access_scope;
  if (coherent_access) *coherent_access = qos->presentation.coherent_access;
  if (ordered_access) *ordered_access = qos->presentation.ordered_access;
}

void dds_qget_lifespan (const dds_qos_t *qos, dds_duration_t * lifespan)
{
  assert (qos);
  /* convert nn_duration_t to int64_t */
  if (lifespan)
  {
    *lifespan = nn_from_ddsi_duration (qos->lifespan.duration);
  }
}

void dds_qget_deadline (const dds_qos_t * qos, dds_duration_t * deadline)
{
  assert (qos);
  /* convert nn_duration_t to int64_t */
  if (deadline)
  {
    *deadline = nn_from_ddsi_duration (qos->deadline.deadline);
  }
}

void dds_qget_latency_budget (const dds_qos_t *qos, dds_duration_t *duration)
{
  assert (qos);
  /* convert nn_duration_t to int64_t */
  if (duration)
  {
    *duration = nn_from_ddsi_duration (qos->latency_budget.duration);
  }
}

void dds_qget_ownership (const dds_qos_t *qos, dds_ownership_kind_t *kind)
{
  assert (qos);
  if (kind)
  {
    *kind = (dds_ownership_kind_t) qos->ownership.kind;
  }
}

void dds_qget_ownership_strength (const dds_qos_t *qos, int32_t *value)
{
  assert (qos);
  if (value) *value = qos->ownership_strength.value;
}

void dds_qget_liveliness (const dds_qos_t *qos, dds_liveliness_kind_t *kind, dds_duration_t *lease_duration)
{
  assert (qos);
  if (kind) *kind = (dds_liveliness_kind_t) qos->liveliness.kind;
  /* convert nn_duration_t to int64_t */
  if (lease_duration)
  {
    *lease_duration = nn_from_ddsi_duration (qos->liveliness.lease_duration);
  }
}

void dds_qget_time_based_filter (const dds_qos_t *qos, dds_duration_t *minimum_separation)
{
  assert (qos);
  if (minimum_separation)
  {
    /* convert nn_duration_t to int64_t */
    *minimum_separation = nn_from_ddsi_duration (qos->time_based_filter.minimum_separation);
  }
}

void dds_qget_partition (const dds_qos_t * qos, uint32_t *n, char *** ps)
{
  assert (qos);
  size_t len;
  if (n && ps)
  {
    uint32_t i;
    *n = qos->partition.n;
    if (qos->partition.n != 0)
    {
      *ps = dds_alloc (sizeof (char*) * qos->partition.n);
      for (i = 0; i < qos->partition.n; i++)
      {
        len = strlen (qos->partition.strs[i]) + 1;
        (*ps)[i] = dds_alloc (len);
        strncpy ((*ps)[i], qos->partition.strs[i], len);
      }
    }
  }
}

void dds_qget_reliability (const dds_qos_t *qos, dds_reliability_kind_t *kind, dds_duration_t *max_blocking_time)
{
  assert (qos);
  if (kind) *kind = (dds_reliability_kind_t) qos->reliability.kind;
  /* convert from nn_duration_t to int64_t */
  if (max_blocking_time)
  {
    *max_blocking_time = nn_from_ddsi_duration (qos->reliability.max_blocking_time);
  }
}

void dds_qget_transport_priority (const dds_qos_t *qos, int32_t *value)
{
  assert (qos);
  if (value) *value = qos->transport_priority.value;
}

void dds_qget_destination_order (const dds_qos_t *qos, dds_destination_order_kind_t *value)
{
  assert (qos);
  if (value) *value = (dds_destination_order_kind_t) qos->destination_order.kind;
}

void dds_qget_writer_data_lifecycle (const dds_qos_t *qos, bool * autodispose)
{
  assert (qos);
  if (autodispose)
  {
    *autodispose = qos->writer_data_lifecycle.autodispose_unregistered_instances;
  }
}

void dds_qget_reader_data_lifecycle
  (const dds_qos_t *qos, dds_duration_t *autopurge_nowriter_samples, dds_duration_t *autopurge_disposed_delay)
{
  assert (qos);
  /* convert from nn_duration_t to int64_t */
  if (autopurge_nowriter_samples)
  {
    *autopurge_nowriter_samples = \
     nn_from_ddsi_duration (qos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
  }
  if (autopurge_disposed_delay)
  {
    *autopurge_disposed_delay = \
    nn_from_ddsi_duration (qos->reader_data_lifecycle.autopurge_disposed_samples_delay);
  }
}

void dds_qget_durability_service
(
  const dds_qos_t * qos,
  dds_duration_t * service_cleanup_delay,
  dds_history_kind_t * history_kind,
  int32_t * history_depth,
  int32_t * max_samples,
  int32_t * max_instances,
  int32_t * max_samples_per_instance
)
{
  assert (qos);
  if (service_cleanup_delay) *service_cleanup_delay = nn_from_ddsi_duration (qos->durability_service.service_cleanup_delay);
  if (history_kind) *history_kind = (dds_history_kind_t) qos->durability_service.history.kind;
  if (history_depth) *history_depth = qos->durability_service.history.depth;
  if (max_samples) *max_samples = qos->durability_service.resource_limits.max_samples;
  if (max_instances) *max_instances = qos->durability_service.resource_limits.max_instances;
  if (max_samples_per_instance) *max_samples_per_instance = qos->durability_service.resource_limits.max_samples_per_instance;
}
