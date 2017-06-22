#include <stdio.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "dds.h"
#include "RoundTrip.h"

/* Tests in this file only concern themselves with very basic api tests of
   dds_write and dds_write_ts */

static const int payloadSize = 32;
static RoundTripModule_DataType data = { 0 };

static dds_entity_t participant = 0;
static dds_entity_t topic = 0;
static dds_entity_t publisher = 0;
static dds_entity_t writer = 0;

static void
setup(void)
{
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0);
    topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
    cr_assert_gt(topic, 0);
    publisher = dds_create_publisher(participant, NULL, NULL);
    cr_assert_gt(publisher, 0);
    writer = dds_create_writer(participant, topic, NULL, NULL);
    cr_assert_gt(writer, 0);

    memset(&data, 0, sizeof(data));
    data.payload._length = payloadSize;
    data.payload._buffer = dds_alloc (payloadSize);
    memset(data.payload._buffer, 'a', payloadSize);
    data.payload._release = true;
    data.payload._maximum = 0;
}

static void
teardown(void)
{
    RoundTripModule_DataType_free (&data, DDS_FREE_CONTENTS);
    memset(&data, 0, sizeof(data));

    if (writer > 0) {
        dds_delete(writer);
        writer = 0;
    }
    if (publisher > 0) {
        dds_delete(publisher);
        publisher = 0;
    }
    if (topic > 0) {
        dds_delete(topic);
        topic = 0;
    }
    if (participant > 0) {
        dds_delete(participant);
        participant = 0;
    }
}

Test(dds_write, basic, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write(writer, &data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);
}

Test(dds_write, null_writer, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write(0, &data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_BAD_PARAMETER);
}

Test(dds_write, bad_writer)
{
    dds_return_t status;

    status = dds_write(publisher, &data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_ILLEGAL_OPERATION);
}

Test(dds_write, closed_writer, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_delete(writer);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);
    status = dds_write(writer, &data);
    writer = 0;
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_ALREADY_DELETED);
}

Test(dds_write, null_sample, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write(writer, NULL);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_BAD_PARAMETER);
}

Test(dds_write_ts, basic, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write_ts(writer, &data, dds_time());
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);
}

Test(dds_write_ts, null_sample, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write_ts(writer, NULL, dds_time());
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_BAD_PARAMETER);
}

Test(dds_write_ts, bad_timestamp, .init = setup, .fini = teardown)
{
    dds_return_t status;

    status = dds_write_ts(writer, &data, -1);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_BAD_PARAMETER);
}
