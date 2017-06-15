#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define ASSERT_CALLBACK_EQUAL(fntype, listener, expected) \
    do { \
        dds_on_##fntype##_fn cb; \
        dds_lget_##fntype(listener, &cb); \
        cr_expect_eq(cb, expected, "Callback 'on_" #fntype "' matched expected value '" #expected "'"); \
    } while (0)

#define STR(fntype) #fntype##_cb

#define TEST_GET_SET(listener, fntype, cb) \
    do { \
        /* Initially expect DDS_LUNSET on a newly created listener */ \
        ASSERT_CALLBACK_EQUAL(fntype, listener, DDS_LUNSET); \
        /* Using listener NULL, shouldn't crash */ \
        dds_lset_##fntype(NULL, NULL); \
        dds_lget_##fntype(NULL, NULL); \
        /* Set to NULL, get to confirm it succeeds */ \
        dds_lset_##fntype(listener, NULL); \
        ASSERT_CALLBACK_EQUAL(fntype, listener, NULL); \
        /* Set to a proper cb method, get to confirm it succeeds */ \
        dds_lset_##fntype(listener, cb); \
        ASSERT_CALLBACK_EQUAL(fntype, listener, cb); \
    } while (0)

/* dummy callbacks */
static void inconsistent_topic_cb(dds_entity_t topic, const dds_inconsistent_topic_status_t status, void* arg) {}
static void liveliness_lost_cb(dds_entity_t writer, const dds_liveliness_lost_status_t status, void* arg) {}
static void offered_deadline_missed_cb(dds_entity_t writer, const dds_offered_deadline_missed_status_t status, void* arg) {}
static void offered_incompatible_qos_cb(dds_entity_t writer, const dds_offered_incompatible_qos_status_t status, void* arg) {}
static void data_on_readers_cb(dds_entity_t subscriber, void* arg) {}
static void sample_lost_cb(dds_entity_t reader, const dds_sample_lost_status_t status, void* arg) {}
static void data_available_cb(dds_entity_t reader, void* arg) {}
static void sample_rejected_cb(dds_entity_t reader, const dds_sample_rejected_status_t status, void* arg) {}
static void liveliness_changed_cb(dds_entity_t reader, const dds_liveliness_changed_status_t status, void* arg) {}
static void requested_deadline_missed_cb(dds_entity_t reader, const dds_requested_deadline_missed_status_t status, void* arg) {}
static void requested_incompatible_qos_cb(dds_entity_t reader, const dds_requested_incompatible_qos_status_t status, void* arg) {}
static void publication_matched_cb(dds_entity_t writer, const dds_publication_matched_status_t status, void* arg) {}
static void subscription_matched_cb(dds_entity_t reader, const dds_subscription_matched_status_t status, void* arg) {}


/* tests */
Test(c99_listener, create_and_delete)
{
    /* Verify create doesn't return null */
    dds_listener_t *listener;
    listener = dds_listener_create(NULL);
    cr_assert_not_null(listener);

    /* Check default cb's are set */
    ASSERT_CALLBACK_EQUAL(inconsistent_topic, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(liveliness_lost, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(offered_deadline_missed, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(offered_incompatible_qos, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(data_on_readers, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(sample_rejected, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(liveliness_changed, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(requested_deadline_missed, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(requested_incompatible_qos, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(publication_matched, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(subscription_matched, listener, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(data_available, listener, DDS_LUNSET);

    dds_listener_delete(listener);
    dds_listener_delete(NULL);
}

Test(c99_listener, reset)
{
    dds_listener_t *listener;
    listener = dds_listener_create(NULL);
    cr_assert_not_null(listener);

    /* Set a listener cb to a non-default value */
    dds_lset_data_available(listener, NULL);
    ASSERT_CALLBACK_EQUAL(data_available, listener, NULL);

    /* Listener cb should revert to default after reset */
    dds_listener_reset(listener);
    ASSERT_CALLBACK_EQUAL(data_available, listener, DDS_LUNSET);

    dds_listener_delete(listener);
}

Test(c99_listener, copy)
{
    dds_listener_t *listener1 = NULL, *listener2 = NULL;
    listener1 = dds_listener_create(NULL);
    listener2 = dds_listener_create(NULL);
    cr_assert_not_null(listener1);
    cr_assert_not_null(listener2);

    /* Set some listener1 callbacks to non-default values */
    dds_lset_data_available(listener1, NULL);
    dds_lset_sample_lost(listener1, sample_lost_cb);
    ASSERT_CALLBACK_EQUAL(data_available, listener1, NULL);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener1, sample_lost_cb);
    ASSERT_CALLBACK_EQUAL(data_available, listener2, DDS_LUNSET);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener2, DDS_LUNSET);

    /* Cb's should be copied to listener2 */
    dds_listener_copy(listener2, listener1);
    ASSERT_CALLBACK_EQUAL(data_available, listener1, NULL);
    ASSERT_CALLBACK_EQUAL(data_available, listener2, NULL);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener1, sample_lost_cb);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener2, sample_lost_cb);

    dds_listener_delete(listener1);
    dds_listener_delete(listener2);
}

Test(c99_listener, merge)
{
    dds_listener_t *listener1 = NULL, *listener2 = NULL;
    listener1 = dds_listener_create(NULL);
    listener2 = dds_listener_create(NULL);
    cr_assert_not_null(listener1);
    cr_assert_not_null(listener2);

    /* Set some listener1 callbacks to non-default values */
    dds_lset_data_available(listener1, NULL);
    dds_lset_sample_lost(listener1, sample_lost_cb);
    ASSERT_CALLBACK_EQUAL(data_available, listener1, NULL);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener1, sample_lost_cb);

    /* Set listener2 callback to non-default values (will not be overwritten by merge) */
    dds_lset_data_available(listener2, data_available_cb);
    ASSERT_CALLBACK_EQUAL(data_available, listener2, data_available_cb);

    /* Cb's should be merged to listener2 */
    dds_listener_merge(listener2, listener1);
    ASSERT_CALLBACK_EQUAL(data_available, listener2, data_available_cb);
    ASSERT_CALLBACK_EQUAL(sample_lost, listener2, sample_lost_cb);

    /* This shouldn't crash. */
    dds_listener_merge(listener2, NULL);
    dds_listener_merge(NULL, listener1);
    dds_listener_merge(NULL, NULL);

    dds_listener_delete(listener1);
    dds_listener_delete(listener2);
}

Test(c99_listener, getters_setters)
{
    /* test all individual cb get/set methods */
    dds_listener_t *listener = dds_listener_create(NULL);
    cr_assert_not_null(listener);

    TEST_GET_SET(listener, inconsistent_topic, inconsistent_topic_cb);
    TEST_GET_SET(listener, liveliness_lost, liveliness_lost_cb);
    TEST_GET_SET(listener, offered_deadline_missed, offered_deadline_missed_cb);
    TEST_GET_SET(listener, offered_incompatible_qos, offered_incompatible_qos_cb);
    TEST_GET_SET(listener, data_on_readers, data_on_readers_cb);
    TEST_GET_SET(listener, sample_lost, sample_lost_cb);
    TEST_GET_SET(listener, sample_rejected, sample_rejected_cb);
    TEST_GET_SET(listener, liveliness_changed, liveliness_changed_cb);
    TEST_GET_SET(listener, requested_deadline_missed, requested_deadline_missed_cb);
    TEST_GET_SET(listener, requested_incompatible_qos, requested_incompatible_qos_cb);
    TEST_GET_SET(listener, publication_matched, publication_matched_cb);
    TEST_GET_SET(listener, subscription_matched, subscription_matched_cb);
    TEST_GET_SET(listener, data_available, data_available_cb);

    dds_listener_delete(listener);
}

