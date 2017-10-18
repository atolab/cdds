#ifndef COMMON_H
#define COMMON_H

#include "vddsc/dds.h"
#include <assert.h>

#define DDS_USERDATA_QOS_POLICY_NAME                            "UserData"
#define DDS_DURABILITY_QOS_POLICY_NAME                          "Durability"
#define DDS_PRESENTATION_QOS_POLICY_NAME                        "Presentation"
#define DDS_DEADLINE_QOS_POLICY_NAME                            "Deadline"
#define DDS_LATENCYBUDGET_QOS_POLICY_NAME                       "LatencyBudget"
#define DDS_OWNERSHIP_QOS_POLICY_NAME                           "Ownership"
#define DDS_OWNERSHIPSTRENGTH_QOS_POLICY_NAME                   "OwnershipStrength"
#define DDS_LIVELINESS_QOS_POLICY_NAME                          "Liveliness"
#define DDS_TIMEBASEDFILTER_QOS_POLICY_NAME                     "TimeBasedFilter"
#define DDS_PARTITION_QOS_POLICY_NAME                           "Partition"
#define DDS_RELIABILITY_QOS_POLICY_NAME                         "Reliability"
#define DDS_DESTINATIONORDER_QOS_POLICY_NAME                    "DestinationOrder"
#define DDS_HISTORY_QOS_POLICY_NAME                             "History"
#define DDS_RESOURCELIMITS_QOS_POLICY_NAME                      "ResourceLimits"
#define DDS_ENTITYFACTORY_QOS_POLICY_NAME                       "EntityFactory"
#define DDS_WRITERDATALIFECYCLE_QOS_POLICY_NAME                 "WriterDataLifecycle"
#define DDS_READERDATALIFECYCLE_QOS_POLICY_NAME                 "ReaderDataLifecycle"
#define DDS_TOPICDATA_QOS_POLICY_NAME                           "TopicData"
#define DDS_GROUPDATA_QOS_POLICY_NAME                           "GroupData"
#define DDS_TRANSPORTPRIORITY_QOS_POLICY_NAME                   "TransportPriority"
#define DDS_LIFESPAN_QOS_POLICY_NAME                            "Lifespan"
#define DDS_DURABILITYSERVICE_QOS_POLICY_NAME                   "DurabilityService"
#define DDS_SUBSCRIPTIONKEY_QOS_POLICY_NAME                     "SubscriptionKey"
#define DDS_VIEWKEY_QOS_POLICY_NAME                             "ViewKey"
#define DDS_READERLIFESPAN_QOS_POLICY_NAME                      "ReaderLifespan"
#define DDS_SHARE_QOS_POLICY_NAME                               "Share"
#define DDS_SCHEDULING_QOS_POLICY_NAME                          "Scheduling"

#define DDS_SUBSCRIPTIONKEY_QOS_POLICY_ID                       23
#define DDS_VIEWKEY_QOS_POLICY_ID                               24
#define DDS_READERLIFESPAN_QOS_POLICY_ID                        25
#define DDS_SHARE_QOS_POLICY_ID                                 26
#define DDS_SCHEDULING_QOS_POLICY_ID                            27

extern dds_entity_t dp;
extern const dds_topic_descriptor_t *ts_KeyedSeq;
extern const dds_topic_descriptor_t *ts_Keyed32;
extern const dds_topic_descriptor_t *ts_Keyed64;
extern const dds_topic_descriptor_t *ts_Keyed128;
extern const dds_topic_descriptor_t *ts_Keyed256;
extern const dds_topic_descriptor_t *ts_OneULong;
extern const char *saved_argv0;
extern const char *qos_arg_usagestr;
struct qos;
//
////#define BINS_LENGTH (8 * sizeof (unsigned long long) + 1)
//
unsigned long long nowll (void);
//void nowll_as_ddstime (DDS_Time_t *t);
//
//void bindelta (unsigned long long *bins, unsigned long long d, unsigned repeat);
//void binprint (unsigned long long *bins, unsigned long long telapsed);
//
struct hist;
struct hist *hist_new (unsigned nbins, uint64_t binwidth, uint64_t bin0);
void hist_free (struct hist *h);
void hist_reset_minmax (struct hist *h);
void hist_reset (struct hist *h);
void hist_record (struct hist *h, uint64_t x, unsigned weight);
void hist_print (struct hist *h, uint64_t dt, int reset);

void error (const char *fmt, ...);
#define error_abort(rc, fmt, ...) if (rc < DDS_SUCCESS) { error(fmt); DDS_ERR_CHECK(rc, DDS_CHECK_FAIL); }
#define error_report(rc, fmt, ...) if (rc < DDS_SUCCESS) { error(fmt); DDS_ERR_CHECK(rc, DDS_CHECK_REPORT); }
#define error_return(rc, fmt, ...) if (rc < DDS_SUCCESS) { error_report(rc, fmt); return; }
#define error_exit(fmt, ...) { error(fmt); exit(2); }
#define os_error_exit(osres, fmt, ...) if (osres != os_resultSuccess) { error(fmt); exit(2); }

void save_argv0 (const char *argv0);
int common_init (const char *argv0);
void common_fini (void);
int change_publisher_partitions (dds_entity_t pub, unsigned npartitions, const char *partitions[]);
int change_subscriber_partitions (dds_entity_t sub, unsigned npartitions, const char *partitions[]);
dds_entity_t new_publisher (const struct qos *a, unsigned npartitions, const char **partitions);
dds_entity_t new_subscriberNew (dds_qos_t *a, unsigned npartitions, const char **partitions);
dds_entity_t new_subscriber (const struct qos *a, unsigned npartitions, const char **partitions);
struct qos *new_tqos (void);
struct qos *new_pubqos (void);
struct qos *new_subqos (void);
struct qos *new_rdqos (dds_entity_t s, dds_entity_t t);
struct qos *new_wrqos (dds_entity_t p, dds_entity_t t);
void free_qos (struct qos *a);
void set_infinite_dds_duration (dds_duration_t *dd);
int double_to_dds_duration (dds_duration_t *dd, double d);
dds_entity_t new_topic (const char *name, const dds_topic_descriptor_t *topicDesc, const struct qos *a);
dds_entity_t new_topic_KeyedSeq (const char *name, const struct qos *a);
dds_entity_t new_topic_Keyed32 (const char *name, const struct qos *a);
dds_entity_t new_topic_Keyed64 (const char *name, const struct qos *a);
dds_entity_t new_topic_Keyed128 (const char *name, const struct qos *a);
dds_entity_t new_topic_Keyed256 (const char *name, const struct qos *a);
dds_entity_t new_topic_OneULong (const char *name, const struct qos *a);
dds_entity_t new_datawriter (const struct qos *a);
dds_entity_t new_datareader (const struct qos *a);
dds_entity_t new_datawriter_listener (const struct qos *a, const dds_listener_t *l);
dds_entity_t new_datareader_listener (const struct qos *a, const dds_listener_t *l);
const dds_qos_t *qos_datawriter(const struct qos *a);
void qos_livelinessNew (dds_qos_t *a, const char *arg);
void qos_liveliness (struct qos *a, const char *arg);
void qos_deadline (struct qos *a, const char *arg);
void qos_durability (struct qos *a, const char *arg);
void qos_history (struct qos *a, const char *arg);
void qos_destination_order (struct qos *a, const char *arg);
void qos_ownership (struct qos *a, const char *arg);
void qos_transport_priority (struct qos *a, const char *arg);
void qos_reliability (struct qos *a, const char *arg);
void qos_resource_limits (struct qos *a, const char *arg);
void qos_user_data (struct qos *a, const char *arg);
void qos_latency_budget (struct qos *a, const char *arg);
void qos_lifespan (struct qos *a, const char *arg);
void qos_autodispose_unregistered_instances (struct qos *a, const char *arg);
void qos_subscription_keys (struct qos *a, const char *arg);
void set_qosprovider (const char *arg);
void setqos_from_args (struct qos *q, int n, const char *args[]);
void setqos_from_argsNew (dds_qos_t *q, int n, const char *args[]);

#endif
