#include "dds.h"
#include "os/os.h"
#include <criterion/criterion.h>

/* We are deliberately testing some bad arguments that SAL will complain about.
 * So, silence SAL regarding these issues. */
#pragma warning(push)
#pragma warning(disable: 6387 28020)



/****************************************************************************
 * Convenience global policies
 ****************************************************************************/
struct pol_userdata {
    void *value;
    size_t sz;
};

struct pol_topicdata {
    void *value;
    size_t sz;
};

struct pol_groupdata {
    void *value;
    size_t sz;
};

struct pol_durability {
    dds_durability_kind_t kind;
};

struct pol_history {
    dds_history_kind_t kind;
    int32_t depth;
};

struct pol_resource_limits {
    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
};

struct pol_presentation {
    dds_presentation_access_scope_kind_t access_scope;
    bool coherent_access;
    bool ordered_access;
};

struct pol_lifespan {
    dds_duration_t lifespan;
};

struct pol_deadline {
    dds_duration_t deadline;
};

struct pol_latency_budget {
    dds_duration_t duration;
};

struct pol_ownership {
    dds_ownership_kind_t kind;
};

struct pol_ownership_strength {
    int32_t value;
};

struct pol_liveliness {
    dds_liveliness_kind_t kind;
    dds_duration_t lease_duration;
};

struct pol_time_based_filter {
    dds_duration_t minimum_separation;
};

struct pol_partition {
    uint32_t n;
    char **ps;
};

struct pol_reliability {
    dds_reliability_kind_t kind;
    dds_duration_t max_blocking_time;
};

struct pol_transport_priority {
    int32_t value;
};

struct pol_destination_order {
    dds_destination_order_kind_t kind;
};

struct pol_writer_data_lifecycle {
    bool autodispose;
};

struct pol_reader_data_lifecycle {
    dds_duration_t autopurge_nowriter_samples_delay;
    dds_duration_t autopurge_disposed_samples_delay;
};

struct pol_durability_service {
    dds_duration_t service_cleanup_delay;
    dds_history_kind_t history_kind;
    int32_t history_depth;
    int32_t max_samples;
    int32_t max_instances;
    int32_t max_samples_per_instance;
};



static struct pol_userdata g_pol_userdata;
static struct pol_topicdata g_pol_topicdata;
static struct pol_groupdata g_pol_groupdata;
static struct pol_durability g_pol_durability;
static struct pol_history g_pol_history;
static struct pol_resource_limits g_pol_resource_limits;
static struct pol_presentation g_pol_presentation;
static struct pol_lifespan g_pol_lifespan;
static struct pol_deadline g_pol_deadline;
static struct pol_latency_budget g_pol_latency_budget;
static struct pol_ownership g_pol_ownership;
static struct pol_ownership_strength g_pol_ownership_strength;
static struct pol_liveliness g_pol_liveliness;
static struct pol_time_based_filter g_pol_time_based_filter;
static struct pol_partition g_pol_partition;
static struct pol_reliability g_pol_reliability;
static struct pol_transport_priority g_pol_transport_priority;
static struct pol_destination_order g_pol_destination_order;
static struct pol_writer_data_lifecycle g_pol_writer_data_lifecycle;
static struct pol_reader_data_lifecycle g_pol_reader_data_lifecycle;
static struct pol_durability_service g_pol_durability_service;



static const char* c_userdata  = "user_key";
static const char* c_topicdata = "topic_key";
static const char* c_groupdata = "group_key";
static const char* c_partitions[] = {"Partition1", "Partition2"};



/****************************************************************************
 * Test initializations and teardowns.
 ****************************************************************************/
static dds_qos_t *g_qos = NULL;

static void
qos_init(void)
{
    g_qos = dds_qos_create();
    cr_assert_not_null(g_qos);

    g_pol_userdata.value = (void*)c_userdata;
    g_pol_userdata.sz = strlen((char*)g_pol_userdata.value) + 1;

    g_pol_topicdata.value = (void*)c_topicdata;
    g_pol_topicdata.sz = strlen((char*)g_pol_topicdata.value) + 1;

    g_pol_groupdata.value = (void*)c_groupdata;
    g_pol_groupdata.sz = strlen((char*)g_pol_groupdata.value) + 1;

    g_pol_durability.kind = DDS_DURABILITY_TRANSIENT;

    g_pol_history.kind  = DDS_HISTORY_KEEP_LAST;
    g_pol_history.depth = 1;

    g_pol_resource_limits.max_samples = 1;
    g_pol_resource_limits.max_instances = 1;
    g_pol_resource_limits.max_samples_per_instance = 1;

    g_pol_presentation.access_scope = DDS_PRESENTATION_INSTANCE;
    g_pol_presentation.coherent_access = true;
    g_pol_presentation.ordered_access = true;

    g_pol_lifespan.lifespan = 10000;

    g_pol_deadline.deadline = 20000;

    g_pol_latency_budget.duration = 30000;

    g_pol_ownership.kind = DDS_OWNERSHIP_EXCLUSIVE;

    g_pol_ownership_strength.value = 10;

    g_pol_liveliness.kind = DDS_LIVELINESS_AUTOMATIC;
    g_pol_liveliness.lease_duration = 40000;

    g_pol_time_based_filter.minimum_separation = 50000;

    g_pol_partition.ps = (char**)c_partitions;
    g_pol_partition.n  = 2;

    g_pol_reliability.kind = DDS_RELIABILITY_RELIABLE;
    g_pol_reliability.max_blocking_time = 60000;

    g_pol_transport_priority.value = 42;

    g_pol_destination_order.kind = DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP;

    g_pol_writer_data_lifecycle.autodispose = true;

    g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay = 70000;
    g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay = 80000;

    g_pol_durability_service.history_depth = 1;
    g_pol_durability_service.history_kind = DDS_HISTORY_KEEP_LAST;
    g_pol_durability_service.max_samples = 2;
    g_pol_durability_service.max_instances = 3;
    g_pol_durability_service.max_samples_per_instance = 4;
    g_pol_durability_service.service_cleanup_delay = 90000;


}

static void
qos_fini(void)
{
    dds_qos_delete(g_qos);
}
