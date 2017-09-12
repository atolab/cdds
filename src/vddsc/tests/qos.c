#include "qos.h"
#include <criterion/logging.h>


#if 0
#else
/****************************************************************************
 * API tests
 ****************************************************************************/
Test(vddsc_qos, userdata, .init=qos_init, .fini=qos_fini)
{
    struct pol_userdata p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_userdata(NULL, g_pol_userdata.value, g_pol_userdata.sz);
    dds_qget_userdata(NULL, &p.value, &p.sz);
    dds_qget_userdata(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_userdata(g_qos, g_pol_userdata.value, g_pol_userdata.sz);
    dds_qget_userdata(g_qos, &p.value, &p.sz);
    cr_assert_eq(p.sz, g_pol_userdata.sz);
    cr_assert_str_eq(p.value, g_pol_userdata.value);

    dds_free(p.value);
}

Test(vddsc_qos, topicdata, .init=qos_init, .fini=qos_fini)
{
    struct pol_topicdata p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_topicdata(NULL, g_pol_topicdata.value, g_pol_topicdata.sz);
    dds_qget_topicdata(NULL, &p.value, &p.sz);
    dds_qget_topicdata(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_topicdata(g_qos, g_pol_topicdata.value, g_pol_topicdata.sz);
    dds_qget_topicdata(g_qos, &p.value, &p.sz);
    cr_assert_eq(p.sz, g_pol_topicdata.sz);
    cr_assert_str_eq(p.value, g_pol_topicdata.value);

    dds_free(p.value);
}

Test(vddsc_qos, groupdata, .init=qos_init, .fini=qos_fini)
{
    struct pol_groupdata p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_groupdata(NULL, g_pol_groupdata.value, g_pol_groupdata.sz);
    dds_qget_groupdata(NULL, &p.value, &p.sz);
    dds_qget_groupdata(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_groupdata(g_qos, g_pol_groupdata.value, g_pol_groupdata.sz);
    dds_qget_groupdata(g_qos, &p.value, &p.sz);
    cr_assert_eq(p.sz, g_pol_groupdata.sz);
    cr_assert_str_eq(p.value, g_pol_groupdata.value);

    dds_free(p.value);
}

Test(vddsc_qos, durability, .init=qos_init, .fini=qos_fini)
{
    struct pol_durability p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_durability(NULL, g_pol_durability.kind);
    dds_qget_durability(NULL, &p.kind);
    dds_qget_durability(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_durability(g_qos, g_pol_durability.kind);
    dds_qget_durability(g_qos, &p.kind);
    cr_assert_eq(p.kind, g_pol_durability.kind);
}

Test(vddsc_qos, history, .init=qos_init, .fini=qos_fini)
{
    struct pol_history p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_history(NULL, g_pol_history.kind, g_pol_history.depth);
    dds_qget_history(NULL, &p.kind, &p.depth);
    dds_qget_history(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_history(g_qos, g_pol_history.kind, g_pol_history.depth);
    dds_qget_history(g_qos, &p.kind, &p.depth);
    cr_assert_eq(p.kind, g_pol_history.kind);
    cr_assert_eq(p.depth, g_pol_history.depth);
}

Test(vddsc_qos, resource_limits, .init=qos_init, .fini=qos_fini)
{
    struct pol_resource_limits p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_resource_limits(NULL, g_pol_resource_limits.max_samples, g_pol_resource_limits.max_instances, g_pol_resource_limits.max_samples_per_instance);
    dds_qget_resource_limits(NULL, &p.max_samples, &p.max_instances, &p.max_samples_per_instance);
    dds_qget_resource_limits(g_qos, NULL, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_resource_limits(g_qos, g_pol_resource_limits.max_samples, g_pol_resource_limits.max_instances, g_pol_resource_limits.max_samples_per_instance);
    dds_qget_resource_limits(g_qos, &p.max_samples, &p.max_instances, &p.max_samples_per_instance);
    cr_assert_eq(p.max_samples, g_pol_resource_limits.max_samples);
    cr_assert_eq(p.max_instances, g_pol_resource_limits.max_instances);
    cr_assert_eq(p.max_samples_per_instance, g_pol_resource_limits.max_samples_per_instance);
}

Test(vddsc_qos, presentation, .init=qos_init, .fini=qos_fini)
{
    struct pol_presentation p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_presentation(NULL, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
    dds_qget_presentation(NULL, &p.access_scope, &p.coherent_access, &p.ordered_access);
    dds_qget_presentation(g_qos, NULL, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_presentation(g_qos, g_pol_presentation.access_scope, g_pol_presentation.coherent_access, g_pol_presentation.ordered_access);
    dds_qget_presentation(g_qos, &p.access_scope, &p.coherent_access, &p.ordered_access);
    cr_assert_eq(p.access_scope, g_pol_presentation.access_scope);
    cr_assert_eq(p.coherent_access, g_pol_presentation.coherent_access);
    cr_assert_eq(p.ordered_access, g_pol_presentation.ordered_access);
}

Test(vddsc_qos, lifespan, .init=qos_init, .fini=qos_fini)
{
    struct pol_lifespan p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_lifespan(NULL, g_pol_lifespan.lifespan);
    dds_qget_lifespan(NULL, &p.lifespan);
    dds_qget_lifespan(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_lifespan(g_qos, g_pol_lifespan.lifespan);
    dds_qget_lifespan(g_qos, &p.lifespan);
    cr_assert_eq(p.lifespan, g_pol_lifespan.lifespan);
}

Test(vddsc_qos, deadline, .init=qos_init, .fini=qos_fini)
{
    struct pol_deadline p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_deadline(NULL, g_pol_deadline.deadline);
    dds_qget_deadline(NULL, &p.deadline);
    dds_qget_deadline(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_deadline(g_qos, g_pol_deadline.deadline);
    dds_qget_deadline(g_qos, &p.deadline);
    cr_assert_eq(p.deadline, g_pol_deadline.deadline);
}

Test(vddsc_qos, latency_budget, .init=qos_init, .fini=qos_fini)
{
    struct pol_latency_budget p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_latency_budget(NULL, g_pol_latency_budget.duration);
    dds_qget_latency_budget(NULL, &p.duration);
    dds_qget_latency_budget(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_latency_budget(g_qos, g_pol_latency_budget.duration);
    dds_qget_latency_budget(g_qos, &p.duration);
    cr_assert_eq(p.duration, g_pol_latency_budget.duration);
}

Test(vddsc_qos, ownership, .init=qos_init, .fini=qos_fini)
{
    struct pol_ownership p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_ownership(NULL, g_pol_ownership.kind);
    dds_qget_ownership(NULL, &p.kind);
    dds_qget_ownership(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_ownership(g_qos, g_pol_ownership.kind);
    dds_qget_ownership(g_qos, &p.kind);
    cr_assert_eq(p.kind, g_pol_ownership.kind);
}

Test(vddsc_qos, ownership_strength, .init=qos_init, .fini=qos_fini)
{
    struct pol_ownership_strength p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_ownership_strength(NULL, g_pol_ownership_strength.value);
    dds_qget_ownership_strength(NULL, &p.value);
    dds_qget_ownership_strength(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_ownership_strength(g_qos, g_pol_ownership_strength.value);
    dds_qget_ownership_strength(g_qos, &p.value);
    cr_assert_eq(p.value, g_pol_ownership_strength.value);
}

Test(vddsc_qos, liveliness, .init=qos_init, .fini=qos_fini)
{
    struct pol_liveliness p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_liveliness(NULL, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
    dds_qget_liveliness(NULL, &p.kind, &p.lease_duration);
    dds_qget_liveliness(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_liveliness(g_qos, g_pol_liveliness.kind, g_pol_liveliness.lease_duration);
    dds_qget_liveliness(g_qos, &p.kind, &p.lease_duration);
    cr_assert_eq(p.kind, g_pol_liveliness.kind);
    cr_assert_eq(p.lease_duration, g_pol_liveliness.lease_duration);
}

Test(vddsc_qos, time_base_filter, .init=qos_init, .fini=qos_fini)
{
    struct pol_time_based_filter p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_time_based_filter(NULL, g_pol_time_based_filter.minimum_separation);
    dds_qget_time_based_filter(NULL, &p.minimum_separation);
    dds_qget_time_based_filter(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_time_based_filter(g_qos, g_pol_time_based_filter.minimum_separation);
    dds_qget_time_based_filter(g_qos, &p.minimum_separation);
    cr_assert_eq(p.minimum_separation, g_pol_time_based_filter.minimum_separation);
}

Test(vddsc_qos, partition, .init=qos_init, .fini=qos_fini)
{
    struct pol_partition p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_partition(NULL, g_pol_partition.n, c_partitions);
    dds_qget_partition(NULL, &p.n, &p.ps);
    dds_qget_partition(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_partition(g_qos, g_pol_partition.n, c_partitions);
    dds_qget_partition(g_qos, &p.n, &p.ps);
    cr_assert_eq(p.n, 2);
    cr_assert_eq(p.n, g_pol_partition.n);
    cr_assert_str_eq(p.ps[0], g_pol_partition.ps[0]);
    cr_assert_str_eq(p.ps[1], g_pol_partition.ps[1]);

    dds_free(p.ps[0]);
    dds_free(p.ps[1]);
    dds_free(p.ps);
}

Test(vddsc_qos, reliability, .init=qos_init, .fini=qos_fini)
{
    struct pol_reliability p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_reliability(NULL, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
    dds_qget_reliability(NULL, &p.kind, &p.max_blocking_time);
    dds_qget_reliability(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_reliability(g_qos, g_pol_reliability.kind, g_pol_reliability.max_blocking_time);
    dds_qget_reliability(g_qos, &p.kind, &p.max_blocking_time);
    cr_assert_eq(p.kind, g_pol_reliability.kind);
    cr_assert_eq(p.max_blocking_time, g_pol_reliability.max_blocking_time);
}

Test(vddsc_qos, transport_priority, .init=qos_init, .fini=qos_fini)
{
    struct pol_transport_priority p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_transport_priority(NULL, g_pol_transport_priority.value);
    dds_qget_transport_priority(NULL, &p.value);
    dds_qget_transport_priority(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_transport_priority(g_qos, g_pol_transport_priority.value);
    dds_qget_transport_priority(g_qos, &p.value);
    cr_assert_eq(p.value, g_pol_transport_priority.value);
}

Test(vddsc_qos, destination_order, .init=qos_init, .fini=qos_fini)
{
    struct pol_destination_order p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_destination_order(NULL, g_pol_destination_order.kind);
    dds_qget_destination_order(NULL, &p.kind);
    dds_qget_destination_order(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_destination_order(g_qos, g_pol_destination_order.kind);
    dds_qget_destination_order(g_qos, &p.kind);
    cr_assert_eq(p.kind, g_pol_destination_order.kind);
}

Test(vddsc_qos, writer_data_lifecycle, .init=qos_init, .fini=qos_fini)
{
    struct pol_writer_data_lifecycle p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_writer_data_lifecycle(NULL, g_pol_writer_data_lifecycle.autodispose);
    dds_qget_writer_data_lifecycle(NULL, &p.autodispose);
    dds_qget_writer_data_lifecycle(g_qos, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_writer_data_lifecycle(g_qos, g_pol_writer_data_lifecycle.autodispose);
    dds_qget_writer_data_lifecycle(g_qos, &p.autodispose);
    cr_assert_eq(p.autodispose, g_pol_writer_data_lifecycle.autodispose);
}

Test(vddsc_qos, reader_data_lifecycle, .init=qos_init, .fini=qos_fini)
{
    struct pol_reader_data_lifecycle p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_reader_data_lifecycle(NULL, g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay, g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay);
    dds_qget_reader_data_lifecycle(NULL, &p.autopurge_nowriter_samples_delay, &p.autopurge_disposed_samples_delay);
    dds_qget_reader_data_lifecycle(g_qos, NULL, NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_reader_data_lifecycle(g_qos, g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay, g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay);
    dds_qget_reader_data_lifecycle(g_qos, &p.autopurge_nowriter_samples_delay, &p.autopurge_disposed_samples_delay);
    cr_assert_eq(p.autopurge_nowriter_samples_delay, g_pol_reader_data_lifecycle.autopurge_nowriter_samples_delay);
    cr_assert_eq(p.autopurge_disposed_samples_delay, g_pol_reader_data_lifecycle.autopurge_disposed_samples_delay);
}

Test(vddsc_qos, durability_service, .init=qos_init, .fini=qos_fini)
{
    struct pol_durability_service p = { 0 };

    /* NULLs shouldn't crash and be a noops. */
    dds_qset_durability_service(NULL,
            g_pol_durability_service.service_cleanup_delay,
            g_pol_durability_service.history_kind,
            g_pol_durability_service.history_depth,
            g_pol_durability_service.max_samples,
            g_pol_durability_service.max_instances,
            g_pol_durability_service.max_samples_per_instance);
    dds_qget_durability_service(NULL,
            &p.service_cleanup_delay,
            &p.history_kind,
            &p.history_depth,
            &p.max_samples,
            &p.max_instances,
            &p.max_samples_per_instance);
    dds_qget_durability_service(g_qos,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);

    /* Getting after setting, should yield the original input. */
    dds_qset_durability_service(g_qos,
            g_pol_durability_service.service_cleanup_delay,
            g_pol_durability_service.history_kind,
            g_pol_durability_service.history_depth,
            g_pol_durability_service.max_samples,
            g_pol_durability_service.max_instances,
            g_pol_durability_service.max_samples_per_instance);
    dds_qget_durability_service(g_qos,
            &p.service_cleanup_delay,
            &p.history_kind,
            &p.history_depth,
            &p.max_samples,
            &p.max_instances,
            &p.max_samples_per_instance);
    cr_assert_eq(p.service_cleanup_delay, g_pol_durability_service.service_cleanup_delay);
    cr_assert_eq(p.history_kind, g_pol_durability_service.history_kind);
    cr_assert_eq(p.history_depth, g_pol_durability_service.history_depth);
    cr_assert_eq(p.max_samples, g_pol_durability_service.max_samples);
    cr_assert_eq(p.max_instances, g_pol_durability_service.max_instances);
    cr_assert_eq(p.max_samples_per_instance, g_pol_durability_service.max_samples_per_instance);
}

#endif


#pragma warning(pop)
