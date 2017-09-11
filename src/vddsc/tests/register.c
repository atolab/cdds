#include <assert.h>

#include "dds.h"
#include "os/os.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/theories.h>
#include "Space.h"


/**************************************************************************************************
 *
 * Test fixtures
 *
 *************************************************************************************************/
#define MAX_SAMPLES                 7
#define INITIAL_SAMPLES             2


static dds_entity_t g_participant = 0;
static dds_entity_t g_topic       = 0;
static dds_entity_t g_reader      = 0;
static dds_entity_t g_writer      = 0;
static dds_entity_t g_waitset     = 0;

static dds_time_t   g_past        = 0;
static dds_time_t   g_present     = 0;

static void*             g_samples[MAX_SAMPLES];
static Space_Type1       g_data[MAX_SAMPLES];
static dds_sample_info_t g_info[MAX_SAMPLES];

static char*
create_topic_name(const char *prefix, char *name, size_t size)
{
    /* Get semi random g_topic name. */
    os_procId pid = os_procIdSelf();
    uintmax_t tid = os_threadIdToInteger(os_threadIdSelf());
    snprintf(name, size, "%s_pid%"PRIprocId"_tid%"PRIuMAX"", prefix, pid, tid);
    return name;
}

static void
registering_init(void)
{
    Space_Type1 sample = { 0 };
    dds_qos_t *qos = dds_qos_create ();
    dds_attach_t triggered;
    dds_return_t ret;
    char name[100];

    /* Use by source timestamp to be able to check the time related funtions. */
    dds_qset_destination_order(qos, DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP);

    g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(g_participant, 0, "Failed to create prerequisite g_participant");

    g_waitset = dds_create_waitset(g_participant);
    cr_assert_gt(g_waitset, 0, "Failed to create g_waitset");

    g_topic = dds_create_topic(g_participant, &Space_Type1_desc, create_topic_name("vddsc_registering_test", name, 100), qos, NULL);
    cr_assert_gt(g_topic, 0, "Failed to create prerequisite g_topic");

    /* Create a reader that keeps one sample on three instances. */
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));
    dds_qset_resource_limits(qos, DDS_LENGTH_UNLIMITED, 3, 1);
    g_reader = dds_create_reader(g_participant, g_topic, qos, NULL);
    cr_assert_gt(g_reader, 0, "Failed to create prerequisite g_reader");

    /* Create a writer that will not automatically dispose unregistered samples. */
    dds_qset_writer_data_lifecycle(qos, false);
    g_writer = dds_create_writer(g_participant, g_topic, qos, NULL);
    cr_assert_gt(g_writer, 0, "Failed to create prerequisite g_writer");

    /* Sync g_writer to g_reader. */
    ret = dds_set_enabled_status(g_writer, DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to set prerequisite g_writer status");
    ret = dds_waitset_attach(g_waitset, g_writer, g_writer);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to attach prerequisite g_writer");
    ret = dds_waitset_wait(g_waitset, &triggered, 1, DDS_SECS(1));
    cr_assert_eq(ret, 1, "Failed prerequisite dds_waitset_wait g_writer r");
    cr_assert_eq(g_writer, (dds_entity_t)(intptr_t)triggered, "Failed prerequisite dds_waitset_wait g_writer a");
    ret = dds_waitset_detach(g_waitset, g_writer);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to detach prerequisite g_writer");

    /* Sync g_reader to g_writer. */
    ret = dds_set_enabled_status(g_reader, DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to set prerequisite g_reader status");
    ret = dds_waitset_attach(g_waitset, g_reader, g_reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to attach prerequisite g_reader");
    ret = dds_waitset_wait(g_waitset, &triggered, 1, DDS_SECS(1));
    cr_assert_eq(ret, 1, "Failed prerequisite dds_waitset_wait g_reader r");
    cr_assert_eq(g_reader, (dds_entity_t)(intptr_t)triggered, "Failed prerequisite dds_waitset_wait g_reader a");
    ret = dds_waitset_detach(g_waitset, g_reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to detach prerequisite g_reader");

    /* Write initial samples. */
    for (int i = 0; i < INITIAL_SAMPLES; i++) {
        sample.long_1 = i;
        sample.long_2 = i*2;
        sample.long_3 = i*3;
        ret = dds_write(g_writer, &sample);
        cr_assert_eq(ret, DDS_RETCODE_OK, "Failed prerequisite write");
    }

    /* Initialize reading buffers. */
    memset (g_data, 0, sizeof (g_data));
    for (int i = 0; i < MAX_SAMPLES; i++) {
        g_samples[i] = &g_data[i];
    }

    /* Initialize times. */
    g_present = dds_time();
    g_past    = g_present - DDS_SECS(1);

    dds_qos_delete(qos);
}

static void
registering_fini(void)
{
    dds_delete(g_reader);
    dds_delete(g_writer);
    dds_delete(g_waitset);
    dds_delete(g_topic);
    dds_delete(g_participant);
}


#if 0
#else
/**************************************************************************************************
 *
 * These will check the dds_register_instance() in various ways.
 *
 *************************************************************************************************/
/*************************************************************************************************/
Test(vddsc_register_instance, deleted_entity, .init=registering_init, .fini=registering_fini)
{
    dds_return_t ret;
    dds_instance_handle_t handle;
    dds_delete(g_writer);
    ret = dds_register_instance(g_writer, &handle, g_data);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ALREADY_DELETED, "returned %d", dds_err_nr(ret));
}

static dds_instance_handle_t hndle = 0;
static Space_Type1           data;
TheoryDataPoints(vddsc_register_instance, invalid_params) = {
        DataPoints(dds_instance_handle_t *, &hndle, NULL),
        DataPoints(void*, &data, NULL)
};
Theory((dds_instance_handle_t *hndl2, void *data), vddsc_register_instance, invalid_params/*, .init=registering_init, .fini=registering_fini*/)
{
    dds_return_t exp = DDS_RETCODE_BAD_PARAMETER * -1;
    dds_return_t ret;

    /* Only test when the combination of parameters is actually invalid.*/
    cr_assume((hndl2 == NULL) || (data == NULL));

    ret = dds_register_instance(g_writer, hndl2, data);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER, "returned %d != expected %d", dds_err_nr(ret), DDS_RETCODE_BAD_PARAMETER);
}

TheoryDataPoints(vddsc_register_instance, invalid_writers) = {
        DataPoints(dds_entity_t, -2, -1, 0, 1, 100, INT_MAX, INT_MIN),
};
Theory((dds_entity_t writer), vddsc_register_instance, invalid_writers, .init=registering_init, .fini=registering_fini)
{
    dds_entity_t exp = DDS_RETCODE_BAD_PARAMETER * -1;
    dds_return_t ret;
    dds_instance_handle_t handle;

    ret = dds_register_instance(writer, &handle, g_data);
    cr_assert_eq(dds_err_nr(ret), dds_err_nr(exp), "returned %d != expected %d", dds_err_nr(ret), dds_err_nr(exp));
}

TheoryDataPoints(vddsc_register_instance, non_writers) = {
        DataPoints(dds_entity_t*, &g_waitset, &g_reader, &g_topic, &g_participant),
};
Theory((dds_entity_t *writer), vddsc_register_instance, non_writers, .init=registering_init, .fini=registering_fini)
{
    dds_return_t ret;
    dds_instance_handle_t handle;
    ret = dds_register_instance(*writer, &handle, g_data);
    cr_assert_eq(dds_err_nr(ret), DDS_RETCODE_ILLEGAL_OPERATION, "returned %d", dds_err_nr(ret));
}

Test(vddsc_register_instance, registering_new_instance, .init=registering_init, .fini=registering_fini)
{
    dds_instance_handle_t instHndl, instHndl2;
    dds_return_t ret;
    Space_Type1 newInstance = { INITIAL_SAMPLES, 0, 0 };
    instHndl = dds_instance_lookup(g_writer, &newInstance);
    cr_assert_eq(instHndl, DDS_HANDLE_NIL);
    ret = dds_register_instance(g_writer, &instHndl2, &newInstance);
    cr_assert_eq(ret, DDS_RETCODE_OK);
    instHndl = dds_instance_lookup(g_writer, &newInstance);
    cr_assert_eq(instHndl, instHndl2);
}

Test(vddsc_register_instance, data_already_available, .init=registering_init, .fini=registering_fini)
{
    dds_instance_handle_t instHndl, instHndl2;
    dds_return_t ret;
    instHndl = dds_instance_lookup(g_writer, &g_data);
    cr_assert_neq(instHndl, DDS_HANDLE_NIL);
    ret = dds_register_instance(g_writer, &instHndl2, &g_data);
    cr_assert_eq(ret, DDS_RETCODE_OK);
    cr_assert_eq(instHndl2, instHndl);

}

#endif
