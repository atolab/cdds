/** @file
 *
 * @brief DDS C99 QoS API
 *
 * @todo add copyright header?
 *
 * This header file defines the public API of QoS and Policies in the
 * VortexDDS C99 language binding.
 */
#ifndef DDS_QOS_H
#define DDS_QOS_H

#include "os/os_public.h"

#if defined (__cplusplus)
extern "C" {
#endif

#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* QoS identifiers */
/** @name QoS identifiers
  @{**/
#define DDS_INVALID_QOS_POLICY_ID 0
#define DDS_USERDATA_QOS_POLICY_ID 1
#define DDS_DURABILITY_QOS_POLICY_ID 2
#define DDS_PRESENTATION_QOS_POLICY_ID 3
#define DDS_DEADLINE_QOS_POLICY_ID 4
#define DDS_LATENCYBUDGET_QOS_POLICY_ID 5
#define DDS_OWNERSHIP_QOS_POLICY_ID 6
#define DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID 7
#define DDS_LIVELINESS_QOS_POLICY_ID 8
#define DDS_TIMEBASEDFILTER_QOS_POLICY_ID 9
#define DDS_PARTITION_QOS_POLICY_ID 10
#define DDS_RELIABILITY_QOS_POLICY_ID 11
#define DDS_DESTINATIONORDER_QOS_POLICY_ID 12
#define DDS_HISTORY_QOS_POLICY_ID 13
#define DDS_RESOURCELIMITS_QOS_POLICY_ID 14
#define DDS_ENTITYFACTORY_QOS_POLICY_ID 15
#define DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID 16
#define DDS_READERDATALIFECYCLE_QOS_POLICY_ID 17
#define DDS_TOPICDATA_QOS_POLICY_ID 18
#define DDS_GROUPDATA_QOS_POLICY_ID 19
#define DDS_TRANSPORTPRIORITY_QOS_POLICY_ID 20
#define DDS_LIFESPAN_QOS_POLICY_ID 21
#define DDS_DURABILITYSERVICE_QOS_POLICY_ID 22
/** @}*/


/* QoS structure is opaque */
/** QoS structure */
typedef struct nn_xqos dds_qos_t;

/* Durability QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Durability
 */
typedef enum dds_durability_kind
{
  DDS_DURABILITY_VOLATILE,
  DDS_DURABILITY_TRANSIENT_LOCAL,
  DDS_DURABILITY_TRANSIENT,
  DDS_DURABILITY_PERSISTENT
}
dds_durability_kind_t;

/* History QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_History
 */
typedef enum dds_history_kind
{
  DDS_HISTORY_KEEP_LAST,
  DDS_HISTORY_KEEP_ALL
}
dds_history_kind_t;

/* Ownership QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Ownership
 */
typedef enum dds_ownership_kind
{
  DDS_OWNERSHIP_SHARED,
  DDS_OWNERSHIP_EXCLUSIVE
}
dds_ownership_kind_t;

/* Liveliness QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Liveliness
 */
typedef enum dds_liveliness_kind
{
  DDS_LIVELINESS_AUTOMATIC,
  DDS_LIVELINESS_MANUAL_BY_PARTICIPANT,
  DDS_LIVELINESS_MANUAL_BY_TOPIC
}
dds_liveliness_kind_t;

/* Reliability QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Reliability
 */
typedef enum dds_reliability_kind
{
  DDS_RELIABILITY_BEST_EFFORT,
  DDS_RELIABILITY_RELIABLE
}
dds_reliability_kind_t;

/* DestinationOrder QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_DestinationOrder
 */
typedef enum dds_destination_order_kind
{
  DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP,
  DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP
}
dds_destination_order_kind_t;

/* History QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_History
 */
typedef struct dds_history_qospolicy
{
  dds_history_kind_t kind;
  int32_t depth;
}
dds_history_qospolicy_t;

/* ResourceLimits QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_ResourceLimits
 */
typedef struct dds_resource_limits_qospolicy
{
  int32_t max_samples;
  int32_t max_instances;
  int32_t max_samples_per_instance;
}
dds_resource_limits_qospolicy_t;

/* Presentation QoS: Applies to Publisher, Subscriber */
/**
 * \ref DCPS_QoS_Presentation
 */
typedef enum dds_presentation_access_scope_kind
{
  DDS_PRESENTATION_INSTANCE,
  DDS_PRESENTATION_TOPIC,
  DDS_PRESENTATION_GROUP
}
dds_presentation_access_scope_kind_t;

/**
 * @brief Allocate memory and initialize default QoS-policies
 *
 * @returns - Pointer to the initialized dds_qos_t structure, NULL if unsuccessful.
 */
_Ret_notnull_
OS_API dds_qos_t * dds_qos_create (void);

/**
 * @brief Delete memory allocated to QoS-policies structure
 *
 * @param[in] qos - Pointer to dds_qos_t structure
 */
OS_API void dds_qos_delete (_In_ _Post_invalid_ dds_qos_t * __restrict qos);

/**
 * @brief Reset a QoS-policies structure to default values
 *
 * @param[in,out] qos - Pointer to the dds_qos_t structure
 */
OS_API void dds_qos_reset (_Inout_ dds_qos_t * __restrict qos);

/**
 * @brief Copy all QoS-policies from one structure to another
 *
 * @param[in,out] dst - Pointer to the destination dds_qos_t structure
 * @param[in] src - Pointer to the source dds_qos_t structure
 */
OS_API dds_return_t dds_qos_copy (_Inout_ dds_qos_t * __restrict dst, _In_ const dds_qos_t * __restrict src);

/**
 * @brief Copy all QoS-policies from one structure to another, unless already set
 *
 * Policies are copied from src to dst, unless src already has the policy set to a non-default value.
 *
 * @param[in,out] dst - Pointer to the destination qos structure
 * @param[in] src - Pointer to the source qos structure
 */
OS_API void dds_qos_merge (_Inout_ dds_qos_t * __restrict dst, _In_ const dds_qos_t * __restrict src);

/**
 * Description : Retrieves the default value of the domain participant qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for participant
 */
OS_API void dds_get_default_participant_qos (dds_qos_t * __restrict qos);

/**
 * Description : Retrieves the default value of the topic qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for topic
 */
OS_API void dds_get_default_topic_qos (dds_qos_t * __restrict qos);

/**
 * Description : Retrieves the default value of the publisher qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for publisher
 */
OS_API void dds_get_default_publisher_qos (dds_qos_t * __restrict qos);

/**
 * Description : Retrieves the default value of the data writer qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for data writer
 */
OS_API void dds_get_default_writer_qos (dds_qos_t * __restrict qos);

/**
 * Description : Retrieves the default value of the data reader qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for data reader
 */
OS_API void dds_get_default_reader_qos (dds_qos_t * __restrict qos);

/**
 * @brief Set the userdata of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the userdata
 * @param[in] value - Pointer to the userdata
 * @param[in] sz - Size of userdata stored in value
 */
OS_API
void dds_qset_userdata
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ const void * __restrict value,
    _In_range_(>, 0) size_t sz
);

/**
 * @brief Set the topicdata of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the topicdata
 * @param[in] value - Pointer to the topicdata
 * @param[in] sz - Size of the topicdata stored in value
 */
OS_API
void dds_qset_topicdata
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ const void * __restrict value,
    _In_range_(>, 0) size_t sz
);

/**
 * @brief Set the groupdata of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the groupdata
 * @param[in] value - Pointer to the group data
 * @param[in] sz - Size of groupdata stored in value
 */
OS_API
void dds_qset_groupdata
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ const void * __restrict value,
    _In_range_(>, 0) size_t sz
);

/**
 * @brief Set the durability policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - Durability kind value \ref DCPS_QoS_Durability
 */
OS_API
void dds_qset_durability
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_DURABILITY_VOLATILE, DDS_DURABILITY_PERSISTENT) dds_durability_kind_t kind
);

/**
 * @brief Set the history policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - History kind value \ref DCPS_QoS_History
 * @param[in] depth - History depth value \ref DCPS_QoS_History
 */
OS_API void dds_qset_history
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_HISTORY_KEEP_LAST, DDS_HISTORY_KEEP_ALL) dds_history_kind_t kind,
    _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t depth
);

/**
 * @brief Set the resource limits policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] max_samples - Number of samples resource-limit value
 * @param[in] max_instances - Number of instances resource-limit value
 * @param[in] max_samples_per_instance - Number of samples per instance resource-limit value
 */
OS_API
void dds_qset_resource_limits
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_samples,
    _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_instances,
    _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_samples_per_instance
);


/**
 * @brief Set the presentation policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] access_scope - Access-scope kind
 * @param[in] coherent_access - Coherent access enable value
 * @param[in] ordered_access - Ordered access enable value
 */
OS_API void dds_qset_presentation
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_PRESENTATION_INSTANCE, DDS_PRESENTATION_GROUP) dds_presentation_access_scope_kind_t access_scope,
    _In_ bool coherent_access,
    _In_ bool ordered_access
);

/**
 * @brief Set the lifespan policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] lifespan - Lifespan duration (expiration time relative to source timestamp of a sample)
 */
OS_API
void dds_qset_lifespan
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(0, DDS_INFINITY) dds_duration_t lifespan
);

/**
 * @brief Set the deadline policy of a qos structure.
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] deadline - Deadline duration
 */
OS_API
void dds_qset_deadline
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(0, DDS_INFINITY) dds_duration_t deadline
);

/**
 * @brief Set the latency-budget policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] duration - Latency budget duration
 */
OS_API
void dds_qset_latency_budget
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(0, DDS_INFINITY) dds_duration_t duration
);

/**
 * @brief Set the ownership policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - Ownership kind
 */
OS_API
void dds_qset_ownership
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_OWNERSHIP_SHARED, DDS_OWNERSHIP_EXCLUSIVE) dds_ownership_kind_t kind
);

/**
 * @brief Set the ownership strength policy of a qos structure
 *
 * param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * param[in] value - Ownership strength value
 */
OS_API
void dds_qset_ownership_strength
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ int32_t value
);

/**
 * @brief Set the liveliness policy of a qos structure
 *
 * param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * param[in] kind - Liveliness kind
 * param[in[ lease_duration - Lease duration
 */
OS_API
void dds_qset_liveliness
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_LIVELINESS_AUTOMATIC, DDS_LIVELINESS_MANUAL_BY_TOPIC) dds_liveliness_kind_t kind,
    _In_range_(0, DDS_INFINITY) dds_duration_t lease_duration
);

/**
 * @brief Set the time-based filter policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] minimum_separation - Minimum duration between sample delivery for an instance
 */
OS_API
void dds_qset_time_based_filter
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(0, DDS_INFINITY) dds_duration_t minimum_separation
);

/**
 * @brief Set the partition policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] n - Number of partitions stored in ps
 * @param[in[ ps - Pointer to string(s) storing partition name(s)
 */
OS_API
void dds_qset_partition
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ uint32_t n,
    _In_reads_z_(n) const char ** __restrict ps
);

/**
 * @brief Set the reliability policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - Reliability kind
 * @param[in] max_blocking_time - Max blocking duration applied when kind is reliable.
 */
OS_API
void dds_qset_reliability
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_RELIABILITY_BEST_EFFORT, DDDS_RELIABILITY_RELIABLE) dds_reliability_kind_t kind,
    _In_range_(0, DDS_INFINITY) dds_duration_t max_blocking_time
);

/**
 * @brief Set the transport-priority policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] value - Priority value
 */
OS_API
void dds_qset_transport_priority
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ int32_t value
);

/**
 * @brief Set the destination-order policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] kind - Destination-order kind
 */
OS_API
void dds_qset_destination_order
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP,
        DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP) dds_destination_order_kind_t kind
);

/**
 * @brief Set the writer data-lifecycle policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] autodispose_unregistered_instances - Automatic disposal of unregistered instances
 */
OS_API
void dds_qset_writer_data_lifecycle
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_ bool autodispose
);

/**
 * @brief Set the reader data-lifecycle policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] autopurge_nowriter_samples_delay - Delay for purging of samples from instances in a no-writers state
 * @param[in] autopurge_disposed_samples_delay - Delay for purging of samples from disposed instances
 */
OS_API
void dds_qset_reader_data_lifecycle
(
    _Inout_ dds_qos_t * __restrict qos,
    _In_range_(0, DDS_INFINITY) dds_duration_t autopurge_nowriter_samples_delay,
    _In_range_(0, DDS_INFINITY) dds_duration_t autopurge_disposed_samples_delay
);

/**
 * @brief Set the durability-service policy of a qos structure
 *
 * @param[in,out] qos - Pointer to a dds_qos_t structure that will store the policy
 * @param[in] service_cleanup_delay - Delay for purging of abandoned instances from the durability service
 * @param[in] history_kind - History policy kind applied by the durability service
 * @param[in] history_depth - History policy depth applied by the durability service
 * @param[in] max_samples - Number of samples resource-limit policy applied by the durability service
 * @param[in] max_instances - Number of instances resource-limit policy applied by the durability service
 * @param[in] max_samples_per_instance - Number of samples per instance resource-limit policy applied by the durability service
 */
OS_API
void dds_qset_durability_service
(
  _Inout_ dds_qos_t * __restrict qos,
  _In_range_(0, DDS_INFINITY) dds_duration_t service_cleanup_delay,
  _In_range_(DDS_HISTORY_KEEP_LAST, DDS_HISTORY_KEEP_ALL) dds_history_kind_t history_kind,
  _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t history_depth,
  _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_samples,
  _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_instances,
  _In_range_(>=, DDS_LENGTH_UNLIMITED) int32_t max_samples_per_instance
);

/**
 * @brief Get the userdata from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] value - Pointer that will store the userdata
 * @param[in,out] sz - Pointer that will store the size of userdata
 */
OS_API
void dds_qget_userdata
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ void ** value,
    _Inout_ size_t * sz
);

/**
 * @brief Get the topicdata from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] value - Pointer that will store the topicdata
 * @param[in,out] sz - Pointer that will store the size of topicdata
 */
OS_API
void dds_qget_topicdata
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ void ** value,
    _Inout_ size_t * sz
);

/**
 * @brief Get the groupdata from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] value - Pointer that will store the groupdata
 * @param[in,out] sz - Pointer that will store the size of groupdata
 */
OS_API
void dds_qget_groupdata
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ void ** value,
    _Inout_ size_t * sz
);

/**
 * @brief Get the durability policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the durability kind
 */
OS_API
void dds_qget_durability
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_durability_kind_t *kind
);

/**
 * @brief Get the history policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the history kind (optional)
 * @param[in,out] depth - Pointer that will store the history depth (optional)
 */
OS_API
void dds_qget_history
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ dds_history_kind_t * kind,
    _Inout_opt_ int32_t *depth
);

/**
 * @brief Get the resource-limits policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] max_samples - Pointer that will store the number of samples resource-limit (optional)
 * @param[in,out] max_instances - Pointer that will store the number of instances resource-limit (optional)
 * @param[in,out] max_samples_per_instance - Pointer that will store the number of samples per instance resource-limit (optional)
 */
OS_API
void dds_qget_resource_limits
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ int32_t *max_samples,
    _Inout_opt_ int32_t *max_instances,
    _Inout_opt_ int32_t *max_samples_per_instance
);

/**
 * @brief Get the presentation policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] access_scope - Pointer that will store access scope kind (optional)
 * @param[in,out] coherent_access - Pointer that will store coherent access enable value (optional)
 * @param[in,out] ordered_access - Pointer that will store orderede access enable value (optional)
 */
OS_API
void dds_qget_presentation
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ dds_presentation_access_scope_kind_t *access_scope,
    _Inout_opt_ bool *coherent_access,
    _Inout_opt_ bool *ordered_access
);

/**
 * @brief Get the lifespan policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] lifespan - Pointer that will store lifespan duration
 */
OS_API
void dds_qget_lifespan
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_duration_t * lifespan
);

/**
 * @brief Get the deadline policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] deadline - Pointer that will store deadline duration
 */
OS_API
void dds_qget_deadline
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_duration_t * deadline
);

/**
 * @brief Get the latency-budget policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] duration - Pointer that will store latency-budget duration
 */
OS_API
void dds_qget_latency_budget
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_duration_t *duration
);

/**
 * @brief Get the ownership policy from a qos structure
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the ownership kind
 */
OS_API
void dds_qget_ownership
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_ownership_kind_t *kind
);

/**
 * @brief Get the ownership strength qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] value - Pointer that will store the ownership strength value
 */
OS_API
void dds_qget_ownership_strength
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ int32_t *value
);

/**
 * @brief Get the liveliness qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the liveliness kind (optional)
 * @param[in,out] lease_duration - Pointer that will store the liveliness lease duration (optional)
 */
OS_API
void dds_qget_liveliness
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ dds_liveliness_kind_t *kind,
    _Inout_opt_ dds_duration_t *lease_duration
);

/**
 * @brief Get the time-based filter qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] minimum_separation - Pointer that will store the minimum separation duration (optional)
 */
OS_API
void dds_qget_time_based_filter
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_duration_t *minimum_separation
);

/**
 * @brief Get the partition qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] n - Pointer that will store the number of partitions (optional)
 * @param[in,out] ps - Pointer that will store the string(s) containing partition name(s) (optional)
 */
OS_API
void dds_qget_partition
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ uint32_t *n,
    _Inout_ char *** ps
);

/**
 * @brief Get the reliability qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the reliability kind (optional)
 * @param[in,out] max_blocking_time - Pointer that will store the max blocking time for reliable reliability (optional)
 */
OS_API
void dds_qget_reliability
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ dds_reliability_kind_t *kind,
    _Inout_opt_ dds_duration_t *max_blocking_time
);

/**
 * @brief Get the transport priority qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] value - Pointer that will store the transport priority value
 */
OS_API
void dds_qget_transport_priority
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ int32_t *value
);

/**
 * @brief Get the destination-order qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] kind - Pointer that will store the destination-order kind
 */
OS_API
void dds_qget_destination_order
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ dds_destination_order_kind_t *kind
);

/**
 * @brief Get the writer data-lifecycle qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] autodispose_unregistered_instances - Pointer that will store the autodispose unregistered instances enable value
 */
OS_API
void dds_qget_writer_data_lifecycle
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_ bool * autodispose
);

/**
 * @brief Get the reader data-lifecycle qos policy
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out] autopurge_nowriter_samples_delay - Pointer that will store the delay for auto-purging samples from instances in a no-writer state (optional)
 * @param[in,out] autopurge_disposed_samples_delay - Pointer that will store the delay for auto-purging of disposed instances (optional)
 */
OS_API
void dds_qget_reader_data_lifecycle
(
    _In_ const dds_qos_t * __restrict qos,
    _Inout_opt_ dds_duration_t *autopurge_nowriter_samples_delay,
    _Inout_opt_ dds_duration_t *autopurge_disposed_samples_delay
);

/**
 * @brief Get the durability-service qos policy values.
 *
 * @param[in] qos - Pointer to a dds_qos_t structure storing the policy
 * @param[in,out]  service_cleanup_delay - Pointer that will store the delay for purging of abandoned instances from the durability service (optional)
 * @param[in,out] history_kind - Pointer that will store history policy kind applied by the durability service (optional)
 * @param[in,out] history_depth - Pointer that will store history policy depth applied by the durability service (optional)
 * @param[in,out] max_samples - Pointer that will store number of samples resource-limit policy applied by the durability service (optional)
 * @param[in,out] max_instances - Pointer that will store number of instances resource-limit policy applied by the durability service (optional)
 * @param[in,out] max_samples_per_instance - Pointer that will store number of samples per instance resource-limit policy applied by the durability service (optional)
 */
OS_API void dds_qget_durability_service
(
  _In_ const dds_qos_t * qos,
  _Inout_opt_ dds_duration_t * service_cleanup_delay,
  _Inout_opt_ dds_history_kind_t * history_kind,
  _Inout_opt_ int32_t * history_depth,
  _Inout_opt_ int32_t * max_samples,
  _Inout_opt_ int32_t * max_instances,
  _Inout_opt_ int32_t * max_samples_per_instance
);

#undef OS_API

#if defined (__cplusplus)
}
#endif
#endif
