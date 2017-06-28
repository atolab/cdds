#include "dds.h"
#include "os/os.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* We are deliberately testing some bad arguments that SAL will complain about.
 * So, silence SAL regarding these issues. */
#pragma warning(push)
#pragma warning(disable: 6387 28020)

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



static dds_entity_t    g_participant = 0;
static dds_entity_t    g_topic  = 0;
static dds_listener_t *g_listener = NULL;
static dds_qos_t      *g_qos = NULL;
static os_mutex        g_mutex;
static os_cond         g_cond;

static uint32_t        cb_called     = 0;
static dds_entity_t    cb_topic      = 0;
static dds_entity_t    cb_writer     = 0;
static dds_entity_t    cb_reader     = 0;
static dds_entity_t    cb_subscriber = 0;

static dds_inconsistent_topic_status_t          cb_inconsistent_topic_status        = { 0 };
static dds_liveliness_lost_status_t             cb_liveliness_lost_status           = { 0 };
static dds_offered_deadline_missed_status_t     cb_offered_deadline_missed_status   = { 0 };
static dds_offered_incompatible_qos_status_t    cb_offered_incompatible_qos_status  = { 0 };
static dds_sample_lost_status_t                 cb_sample_lost_status               = { 0 };
static dds_sample_rejected_status_t             cb_sample_rejected_status           = { 0 };
static dds_liveliness_changed_status_t          cb_liveliness_changed_status        = { 0 };
static dds_requested_deadline_missed_status_t   cb_requested_deadline_missed_status = { 0 };
static dds_requested_incompatible_qos_status_t  cb_requested_incompatible_qos_status= { 0 };
static dds_publication_matched_status_t         cb_publication_matched_status       = { 0 };
static dds_subscription_matched_status_t        cb_subscription_matched_status      = { 0 };


static void
inconsistent_topic_cb(
        dds_entity_t topic,
        const dds_inconsistent_topic_status_t status, void* arg)
{
    cb_topic = topic;
    cb_inconsistent_topic_status = status;
    cb_called |= DDS_INCONSISTENT_TOPIC_STATUS;
    os_condBroadcast(&g_cond);
}

static void
liveliness_lost_cb(
        dds_entity_t writer,
        const dds_liveliness_lost_status_t status,
        void* arg)
{
    cb_writer = writer;
    cb_liveliness_lost_status = status;
    cb_called |= DDS_LIVELINESS_LOST_STATUS;
    os_condBroadcast(&g_cond);
}

static void
offered_deadline_missed_cb(
        dds_entity_t writer,
        const dds_offered_deadline_missed_status_t status,
        void* arg)
{
    cb_writer = writer;
    cb_offered_deadline_missed_status = status;
    cb_called |= DDS_OFFERED_DEADLINE_MISSED_STATUS;
    os_condBroadcast(&g_cond);
}

static void
offered_incompatible_qos_cb(
        dds_entity_t writer,
        const dds_offered_incompatible_qos_status_t status,
        void* arg)
{
    cb_writer = writer;
    cb_offered_incompatible_qos_status = status;
    cb_called |= DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
    os_condBroadcast(&g_cond);
}

static void
data_on_readers_cb(
        dds_entity_t subscriber,
        void* arg)
{
    cb_subscriber = subscriber;
    cb_called |= DDS_DATA_ON_READERS_STATUS;
    os_condBroadcast(&g_cond);
}

static void
sample_lost_cb(
        dds_entity_t reader,
        const dds_sample_lost_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_sample_lost_status = status;
    cb_called |= DDS_SAMPLE_LOST_STATUS;
    os_condBroadcast(&g_cond);
}

static void
data_available_cb(
        dds_entity_t reader,
        void* arg)
{
    cb_reader = reader;
    cb_called |= DDS_DATA_AVAILABLE_STATUS;
    os_condBroadcast(&g_cond);
}

static void
sample_rejected_cb(
        dds_entity_t reader,
        const dds_sample_rejected_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_sample_rejected_status = status;
    cb_called |= DDS_SAMPLE_REJECTED_STATUS;
    os_condBroadcast(&g_cond);
}

static void
liveliness_changed_cb(
        dds_entity_t reader,
        const dds_liveliness_changed_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_liveliness_changed_status = status;
    cb_called |= DDS_LIVELINESS_CHANGED_STATUS;
    os_condBroadcast(&g_cond);
}

static void
requested_deadline_missed_cb(
        dds_entity_t reader,
        const dds_requested_deadline_missed_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_requested_deadline_missed_status = status;
    cb_called |= DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    os_condBroadcast(&g_cond);
}

static void
requested_incompatible_qos_cb(
        dds_entity_t reader,
        const dds_requested_incompatible_qos_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_requested_incompatible_qos_status = status;
    cb_called |= DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
    os_condBroadcast(&g_cond);
}

static void
publication_matched_cb(
        dds_entity_t writer,
        const dds_publication_matched_status_t status,
        void* arg)
{
    cb_writer = writer;
    cb_publication_matched_status = status;
    cb_called |= DDS_PUBLICATION_MATCHED_STATUS;
    os_condBroadcast(&g_cond);
}

static void
subscription_matched_cb(
        dds_entity_t reader,
        const dds_subscription_matched_status_t status,
        void* arg)
{
    cb_reader = reader;
    cb_subscription_matched_status = status;
    cb_called |= DDS_SUBSCRIPTION_MATCHED_STATUS;
    os_condBroadcast(&g_cond);
}


/* tests */
void test_create_and_delete(void)
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

void test_reset(void)
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

void test_copy(void)
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

void test_merge(void)
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

void test_getters_setters(void)
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

Test(c99_listener, test)
{
    test_create_and_delete();
    test_reset();
    test_copy();
    test_merge();
    test_getters_setters();
}


static void
vddsc_listener_cb_init(void)
{
    char name[100];

    os_osInit();

    /* Get semi random g_topic name. */
    snprintf(name, 100,
            "vddsc_listener_test_pid%"PRIprocId"_tid%d",
            os_procIdSelf(),
            (int)os_threadIdToInteger(os_threadIdSelf()));

    os_mutexInit(&g_mutex);
    os_condInit(&g_cond, &g_mutex);

    g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(g_participant, 0, "Failed to create prerequisite g_participant");

    g_topic = dds_create_topic(g_participant, &RoundTripModule_DataType_desc, name, NULL, NULL);
    cr_assert_gt(g_topic, 0, "Failed to create prerequisite g_topic");

    g_listener = dds_listener_create(NULL);
    cr_assert_not_null(g_listener, "Failed to create prerequisite g_listener");

    g_qos = dds_qos_create();
    cr_assert_not_null(g_qos, "Failed to create prerequisite g_qos");

    dds_qset_reliability(g_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
    dds_qset_history(g_qos, DDS_HISTORY_KEEP_ALL, 0);
}

static void
vddsc_listener_cb_fini(void)
{
    dds_qos_delete(g_qos);
    dds_listener_delete(g_listener);
    dds_delete(g_participant);
    os_condDestroy(&g_cond);
    os_mutexDestroy(&g_mutex);
    os_osExit();
}

static uint32_t
vddsc_listener_cb_waitfor(uint32_t expected)
{
    os_time timeout = { 1, 0 };
    os_result osr = os_resultSuccess;
    os_mutexLock(&g_mutex);
    while (((cb_called & expected) != expected) && (osr == os_resultSuccess)) {
        osr = os_condTimedWait(&g_cond, &g_mutex, &timeout);
    }
    os_mutexUnlock(&g_mutex);
    return cb_called;
}


Test(c99_listener, matched, .init=vddsc_listener_cb_init, .fini=vddsc_listener_cb_fini)
{
    dds_entity_t writer;
    dds_entity_t reader;
    uint32_t triggered;

    /* We are interested in matched notifications. */
    dds_lset_publication_matched(g_listener, publication_matched_cb);
    dds_lset_subscription_matched(g_listener, subscription_matched_cb);

    /* Create reader and writer with proper listeners.
     * The creation order is deliberately different from publication_matched and subscription_matched. */
    reader = dds_create_reader(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(reader, 0, "Failed to create prerequisite reader");
    writer = dds_create_writer(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(writer, 0, "Failed to create prerequisite writer");

    /* Both matched should be triggered on the right entities. */
    triggered = vddsc_listener_cb_waitfor(DDS_PUBLICATION_MATCHED_STATUS | DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_eq(triggered & DDS_SUBSCRIPTION_MATCHED_STATUS, DDS_SUBSCRIPTION_MATCHED_STATUS, "DDS_SUBSCRIPTION_MATCHED_STATUS not triggered");
    cr_assert_eq(triggered & DDS_PUBLICATION_MATCHED_STATUS,  DDS_PUBLICATION_MATCHED_STATUS,  "DDS_PUBLICATION_MATCHED_STATUS not triggered");
    cr_assert_eq(cb_writer, writer);
    cr_assert_eq(cb_reader, reader);

    dds_delete(reader);
    dds_delete(writer);
}

Test(c99_listener, publication_matched, .init=vddsc_listener_cb_init, .fini=vddsc_listener_cb_fini)
{
    dds_instance_handle_t reader_hdl;
    dds_entity_t writer;
    dds_entity_t reader;
    dds_return_t ret;
    uint32_t triggered;

    /* We are interested in publication matched notifications. */
    dds_lset_publication_matched(g_listener, publication_matched_cb);

    /* Create reader and writer with proper listeners. */
    writer = dds_create_writer(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(writer, 0, "Failed to create prerequisite writer");
    reader = dds_create_reader(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(reader, 0, "Failed to create prerequisite reader");
    ret = dds_instancehandle_get(reader, &reader_hdl);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to get prerequisite reader_hdl");

    /* Publication matched should be triggered with the right status. */
    triggered = vddsc_listener_cb_waitfor(DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_eq(triggered & DDS_PUBLICATION_MATCHED_STATUS,  DDS_PUBLICATION_MATCHED_STATUS,  "DDS_PUBLICATION_MATCHED_STATUS not triggered");
    cr_assert_eq(cb_writer, writer);
    cr_assert_eq(cb_publication_matched_status.current_count, 1);
    cr_assert_eq(cb_publication_matched_status.current_count_change, 1);
    cr_assert_eq(cb_publication_matched_status.total_count, 1);
    cr_assert_eq(cb_publication_matched_status.total_count_change, 1);
    cr_assert_eq(cb_publication_matched_status.last_subscription_handle, reader_hdl);

    /* Reset the trigger flags. */
    cb_called = 0;

    /* Un-match the publication by deleting the reader. */
    dds_delete(reader);

    /* Publication matched should be triggered with the right status. */
    triggered = vddsc_listener_cb_waitfor(DDS_PUBLICATION_MATCHED_STATUS);
    cr_assert_eq(triggered & DDS_PUBLICATION_MATCHED_STATUS,  DDS_PUBLICATION_MATCHED_STATUS,  "DDS_PUBLICATION_MATCHED_STATUS not triggered");
    cr_assert_eq(cb_writer, writer);
    cr_assert_eq(cb_publication_matched_status.current_count, 0);
    cr_assert_eq(cb_publication_matched_status.current_count_change, -1);
    cr_assert_eq(cb_publication_matched_status.total_count, 1);
    cr_assert_eq(cb_publication_matched_status.total_count_change, 0);
    cr_assert_eq(cb_publication_matched_status.last_subscription_handle, reader_hdl);

    dds_delete(writer);
}


Test(c99_listener, subscription_matched, .init=vddsc_listener_cb_init, .fini=vddsc_listener_cb_fini)
{
    dds_instance_handle_t writer_hdl;
    dds_entity_t writer;
    dds_entity_t reader;
    dds_return_t ret;
    uint32_t triggered;

    /* We are interested in subscription matched notifications. */
    dds_lset_subscription_matched(g_listener, subscription_matched_cb);

    /* Create reader and writer with proper listeners. */
    writer = dds_create_writer(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(writer, 0, "Failed to create prerequisite writer");
    reader = dds_create_reader(g_participant, g_topic, g_qos, g_listener);
    cr_assert_gt(reader, 0, "Failed to create prerequisite reader");
    ret = dds_instancehandle_get(writer, &writer_hdl);
    cr_assert_eq(ret, DDS_RETCODE_OK, "Failed to get prerequisite writer_hdl");

    /* Subscription matched should be triggered with the right status. */
    triggered = vddsc_listener_cb_waitfor(DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_eq(triggered & DDS_SUBSCRIPTION_MATCHED_STATUS,  DDS_SUBSCRIPTION_MATCHED_STATUS,  "DDS_SUBSCRIPTION_MATCHED_STATUS not triggered");
    cr_assert_eq(cb_reader, reader);
    cr_assert_eq(cb_subscription_matched_status.current_count, 1);
    cr_assert_eq(cb_subscription_matched_status.current_count_change, 1);
    cr_assert_eq(cb_subscription_matched_status.total_count, 1);
    cr_assert_eq(cb_subscription_matched_status.total_count_change, 1);
    cr_assert_eq(cb_subscription_matched_status.last_publication_handle, writer_hdl);

    /* Reset the trigger flags. */
    cb_called = 0;

    /* Un-match the subscription by deleting the writer. */
    dds_delete(writer);

    /* Subscription matched should be triggered with the right status. */
    triggered = vddsc_listener_cb_waitfor(DDS_SUBSCRIPTION_MATCHED_STATUS);
    cr_assert_eq(triggered & DDS_SUBSCRIPTION_MATCHED_STATUS,  DDS_SUBSCRIPTION_MATCHED_STATUS,  "DDS_SUBSCRIPTION_MATCHED_STATUS not triggered");
    cr_assert_eq(cb_reader, reader);
    cr_assert_eq(cb_subscription_matched_status.current_count, 0);
    cr_assert_eq(cb_subscription_matched_status.current_count_change, -1);
    cr_assert_eq(cb_subscription_matched_status.total_count, 1);
    cr_assert_eq(cb_subscription_matched_status.total_count_change, 0);
    cr_assert_eq(cb_subscription_matched_status.last_publication_handle, writer_hdl);

    dds_delete(reader);
}

#pragma warning(pop)
