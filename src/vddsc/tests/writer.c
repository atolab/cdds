#include <stdio.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "dds.h"
#include "RoundTrip.h"

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
}

static void
teardown(void)
{
    dds_delete(writer);
    dds_delete(publisher);
    dds_delete(topic);
    dds_delete(participant);
}

Test(vddsc_create_writer, basic, .init = setup, .fini = teardown)
{
    dds_return_t result;

    writer = dds_create_writer(participant, topic, NULL, NULL);
    cr_assert_gt(writer, 0);
    result = dds_delete(writer);
    cr_assert_eq(result, DDS_RETCODE_OK);

}

Test(vddsc_create_writer, null_parent, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(0, topic, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_create_writer, bad_parent, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(topic, topic, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_ILLEGAL_OPERATION);
}

Test(vddsc_create_writer, participant, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(participant, topic, NULL, NULL);
    cr_assert_gt(writer, 0);
}

Test(vddsc_create_writer, publisher, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(publisher, topic, NULL, NULL);
    cr_assert_gt(writer, 0);
}

Test(vddsc_create_writer, deleted_publisher, .init = setup, .fini = teardown)
{
    dds_delete(publisher);

    writer = dds_create_writer(publisher, topic, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_ALREADY_DELETED);
}

Test(vddsc_create_writer, null_topic, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(publisher, 0, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_BAD_PARAMETER);
}

Test(vddsc_writer_create, bad_topic, .init = setup, .fini = teardown)
{
    writer = dds_create_writer(publisher, publisher, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_ILLEGAL_OPERATION);
}

Test(vddsc_create_writer, deleted_topic, .init = setup, .fini = teardown)
{
    dds_delete(topic);

    writer = dds_create_writer(publisher, topic, NULL, NULL);
    cr_assert_eq(dds_err_nr(writer), DDS_RETCODE_ALREADY_DELETED);
}