#include <assert.h>

#include "dds.h"
#include "os/os.h"
#include "util/ut_handleserver.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "RoundTrip.h"

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */

/*
 * This file is a basic setup of the waitset tests for two reasons.
 *   1) Check basic functionality of 'waitset and conditions are entities' prototype.
 *   2) Create a test file to possibly base some test guidelines discussions on.
 *
 * So, this is far from completed and a lot still needs to be added to the tests.
 *
 * TODO: Add whole bunch of invalid arg tests for other API functions, like dds_waitset_attach,
 *       dds_waitset_set_trigger, etc.
 * TODO: Add whole bunch of 'entity array too small for number of attached entities' in, for
 *       instance, dds_waitset_get_conditions().
 * TODO: Test the second attach of one entity, which should fail.
 * TODO: Create a separate test file for specific Condition API stuff.
 * TODO: etc.
 */

/**************************************************************************************************
 *
 * Some thread related convenience stuff.
 *
 *************************************************************************************************/

typedef enum thread_state_t {
    STARTING,
    WAITING,
    STOPPED
} thread_state_t;

typedef struct thread_arg_t {
    os_threadId    tid;
    thread_state_t state;
    dds_entity_t   expected;
} thread_arg_t;

static void         waiting_thread_start(struct thread_arg_t *arg, dds_entity_t expected);
static dds_return_t waiting_thread_expect_exit(struct thread_arg_t *arg);




/**************************************************************************************************
 *
 * Test fixtures
 *
 *************************************************************************************************/

static dds_entity_t participant = 0;
static dds_entity_t topic       = 0;
static dds_entity_t writer      = 0;
static dds_entity_t reader      = 0;
static dds_entity_t waitset     = 0;
static dds_entity_t publisher   = 0;
static dds_entity_t subscriber  = 0;
static dds_entity_t readcond    = 0;



static void
vddsc_waitset_basic_init(void)
{
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0, "Failed to create prerequisite participant");

    waitset = dds_create_waitset(participant);
    cr_assert_gt(waitset, 0, "Failed to create waitset");
}

static void
vddsc_waitset_basic_fini(void)
{
    /* It shouldn't matter if any of these entities were deleted previously.
     * dds_delete will just return an error, which we don't check. */
    dds_delete(waitset);
    dds_delete(participant);
}

static void
vddsc_waitset_init(void)
{
    uint32_t mask = DDS_ANY_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
    char name[100];

#if 1
    /* Get semi random topic name. */
    snprintf(name, 100,
            "vddsc_waitset_test_pid%"PRIprocId"_tid%d",
            os_procIdSelf(),
            (int)os_threadIdToInteger(os_threadIdSelf()));
#else
    /* Single topic name causes interference when tests are executed in parallel. */
    snprintf(name, 100, "%s", "vddsc_waitset_test");
#endif

    vddsc_waitset_basic_init();

    publisher = dds_create_publisher(participant, NULL, NULL);
    cr_assert_gt(publisher, 0, "Failed to create prerequisite publisher");

    subscriber = dds_create_subscriber(participant, NULL, NULL);
    cr_assert_gt(subscriber, 0, "Failed to create prerequisite subscriber");

    topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, name, NULL, NULL);
    cr_assert_gt(topic, 0, "Failed to create prerequisite topic");

    reader = dds_create_reader(subscriber, topic, NULL, NULL);
    cr_assert_gt(reader, 0, "Failed to create prerequisite reader");

    writer = dds_create_writer(publisher, topic, NULL, NULL);
    cr_assert_gt(reader, 0, "Failed to create prerequisite writer");

    readcond = dds_create_readcondition(reader, mask);
    cr_assert_gt(publisher, 0, "Failed to create prerequisite publisher");
}

static void
vddsc_waitset_fini(void)
{
    /* It shouldn't matter if any of these entities were deleted previously.
     * dds_delete will just return an error, which we don't check. */
    dds_delete(readcond);
    dds_delete(writer);
    dds_delete(reader);
    dds_delete(topic);
    dds_delete(publisher);
    dds_delete(subscriber);
    vddsc_waitset_basic_fini();
}




/**************************************************************************************************
 *
 * These will check the waitset creation in various ways.
 *
 *************************************************************************************************/
Test(vddsc_waitset_create, second, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_entity_t ws;
    dds_return_t ret;

    /* Basically, vddsc_waitset_basic_init() already tested the creation of a waitset. But
     * just see if we can create a second one. */
    ws = dds_create_waitset(participant);
    cr_assert_gt(ws, 0, "dds_create_waitset(): returned %d", dds_err_nr(ws));

    /* Also, we should be able to delete this second one. */
    ret = dds_delete(ws);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_delete(): returned %d", dds_err_nr(ret));

    /* And, of course, be able to delete the first one (return code isn't checked in the test fixtures). */
    ret = dds_delete(waitset);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_delete(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_create, invalid_arg_zero, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_entity_t ws;
    ws = dds_create_waitset(0);
    cr_assert_eq(dds_err_nr(ws), DDS_RETCODE_BAD_PARAMETER, "dds_create_waitset(): returned %d", dds_err_nr(ws));
}

Test(vddsc_waitset_create, invalid_arg_negative)
{
    dds_entity_t ws;
    ws = dds_create_waitset(-1);
    cr_assert_eq(ws, -1, "dds_create_waitset(): returned %d", ws);
}

Test(vddsc_waitset_create, invalid_arg_deleted, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_entity_t ws;
    dds_entity_t deleted;
    deleted = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    dds_delete(deleted);
    ws = dds_create_waitset(deleted);
    cr_assert_eq(dds_err_nr(ws), DDS_RETCODE_ALREADY_DELETED, "dds_create_waitset(): returned %d", dds_err_nr(ws));
}

Test(vddsc_waitset_create, invalid_arg_self, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_entity_t ws;
    ws = dds_create_waitset(waitset);
    cr_assert_eq(dds_err_nr(ws), DDS_RETCODE_ILLEGAL_OPERATION, "dds_create_waitset(): returned %d", dds_err_nr(ws));
}

Test(vddsc_waitset_create, invalid_arg_non_pariticpant, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    dds_entity_t ws;
    ws = dds_create_waitset(publisher);
    cr_assert_eq(dds_err_nr(ws), DDS_RETCODE_ILLEGAL_OPERATION, "dds_create_waitset(): returned %d", dds_err_nr(ws));
}




/**************************************************************************************************
 *
 * These will check the waitset attach/detach in various ways. We will use NULL as attach argument
 * because it will be properly used in 'waitset behaviour' tests anyway.
 *
 *************************************************************************************************/
Test(vddsc_waitset_attach_detach, itself, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, waitset, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_waitset_detach(waitset, waitset);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_detach(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_attach_detach, participant, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, participant, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_waitset_detach(waitset, participant);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_detach(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_attach_detach, reader, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, reader, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_waitset_detach(waitset, reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_detach(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_attach_detach, readcondition, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, readcond, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_waitset_detach(waitset, readcond);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_detach(): returned %d", dds_err_nr(ret));
}




/**************************************************************************************************
 *
 * These will check entities can be deleted while attached to the waitset. We will use NULL as
 * attach argument because it will be properly used in 'waitset behaviour' tests anyway.
 * 1) Test if the waitset can be deleted when attached to itself.
 * 2) Test if the waitset parent can be deleted when attached.
 * 3) Test if an 'ordinary' entity can be deleted when attached.
 * 4) Test if an condition can be deleted when attached.
 *
 *************************************************************************************************/
Test(vddsc_waitset_delete_attached, self, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, waitset, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_delete(waitset);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_delete(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_delete_attached, participant, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, participant, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_delete(participant);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_delete(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_delete_attached, reader, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, reader, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_delete(reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_delete(): returned %d", dds_err_nr(ret));
}

Test(vddsc_waitset_delete_attached, readcondition, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    dds_return_t ret;

    ret = dds_waitset_attach(waitset, readcond, NULL);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    ret = dds_delete(readcond);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_delete(): returned %d", dds_err_nr(ret));
}




/**************************************************************************************************
 *
 * This will check if waitset will wake up when it is attached to itself and triggered.
 *
 * In short:
 * 1) Attach the waitset to itself
 * 2) A different thread will call dds_waitset_wait. This should block because the waitset
 *    hasn't been triggered yet. We also want it to block to know for sure that it'll wake up.
 * 3) Trigger the waitset. This should unblock the other thread that was waiting on the waitset.
 * 4) A new dds_waitset_wait should return immediately because the trigger value hasn't been
 *    reset (dds_waitset_set_trigger(waitset, false)) after the waitset woke up.
 *
 *************************************************************************************************/
Test(vddsc_waitset_triggering, on_self, .init=vddsc_waitset_basic_init, .fini=vddsc_waitset_basic_fini)
{
    dds_attach_t triggered;
    thread_arg_t arg;
    dds_return_t ret;

    /* The waitset should not have been triggered. */
    ret = dds_triggered(waitset);
    cr_assert_eq(ret, 0, "dds_triggered(): returned %d", dds_err_nr(ret));

    /* Attach waitset to itself. */
    ret = dds_waitset_attach(waitset, waitset, (dds_attach_t)(intptr_t)waitset);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    /* Start a thread that'll wait because no entity attached to the waitset
     * has been triggered (for instance by a status change). */
    waiting_thread_start(&arg, waitset);

    /* Triggering of the waitset should unblock the thread. */
    ret = dds_waitset_set_trigger(waitset, true);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_set_trigger(): returned %d", dds_err_nr(ret));
    ret = waiting_thread_expect_exit(&arg);
    cr_assert_eq(ret, DDS_RETCODE_OK, "waiting thread did not unblock");

    /* Now the trigger state should be true. */
    ret = dds_triggered(waitset);
    cr_assert_gt(ret, 0, "dds_triggered(): returned %d", dds_err_nr(ret));

    /* Waitset shouldn't wait, but immediately return our waitset. */
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(1));
    cr_assert_eq(ret, 1, "dds_waitset_wait(): returned %d", ret);
    cr_assert_eq(waitset, (dds_entity_t)(intptr_t)triggered);

    /* Reset waitset trigger. */
    ret = dds_waitset_set_trigger(waitset, false);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_set_trigger(): returned %d", dds_err_nr(ret));
    ret = dds_triggered(waitset);
    cr_assert_eq(ret, 0, "dds_triggered(): returned %d", dds_err_nr(ret));

    /* Detach waitset from itself. */
    ret = dds_waitset_detach(waitset, waitset);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));
}




/**************************************************************************************************
 *
 * This will check if waitset will wake up when data is written related to an attached reader.
 *
 * In short:
 * 1) Attach the reader to the waitset
 * 2) A different thread will call dds_waitset_wait. This should block because the reader
 *    hasn't got data yet. We also want it to block to know for sure that it'll wake up.
 * 3) Write data. This should unblock the other thread that was waiting on the waitset.
 * 4) A new dds_waitset_wait should return immediatly because the status of the reader hasn't
 *    changed after the first trigger (it didn't read the data).
 *
 *************************************************************************************************/
Test(vddsc_waitset_triggering, on_reader, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    RoundTripModule_DataType sample;
    dds_attach_t triggered;
    thread_arg_t arg;
    dds_return_t ret;

    memset(&sample, 0, sizeof(RoundTripModule_DataType));

    /* Only interested in data_available for this test. */
    ret = dds_set_enabled_status(reader, DDS_DATA_AVAILABLE_STATUS);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_set_enabled_status(): returned %d", dds_err_nr(ret));

    /* The reader should not have been triggered. */
    ret = dds_triggered(reader);
    cr_assert_eq(ret, 0, "dds_triggered(): returned %d", dds_err_nr(ret));

    /* Attach reader to the waitset. */
    ret = dds_waitset_attach(waitset, reader, (dds_attach_t)(intptr_t)reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    /* Start a thread that'll wait because no entity attached to the waitset
     * has been triggered (for instance by a status change). */
    waiting_thread_start(&arg, reader);

    /* Writing data should unblock the thread. */
    ret = dds_write(writer, &sample);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_write(): returned %d", dds_err_nr(ret));
    ret = waiting_thread_expect_exit(&arg);
    cr_assert_eq(ret, DDS_RETCODE_OK, "waiting thread did not unblock");

    /* Now the trigger state should be true. */
    ret = dds_triggered(reader);
    cr_assert_gt(ret, 0, "dds_triggered: Invalid return code %d", dds_err_nr(ret));

    /* Waitset shouldn't wait, but immediately return our reader. */
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(1));
    cr_assert_eq(ret, 1, "dds_waitset_wait ret");
    cr_assert_eq(reader, (dds_entity_t)(intptr_t)triggered, "dds_waitset_wait attachment");

    /* Detach reader. */
    ret = dds_waitset_detach(waitset, reader);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach: Invalid return code %d", dds_err_nr(ret));
}




/**************************************************************************************************
 *
 * This will check if waitset will wake up when data is written related to an attached condition.
 *
 * In short:
 * 1) Attach the readcondition to the waitset
 * 2) A different thread will call dds_waitset_wait. This should block because the related reader
 *    hasn't got data yet. We also want it to block to know for sure that it'll wake up.
 * 3) Write data. This should unblock the other thread that was waiting on the readcondition.
 * 4) A new dds_waitset_wait should return immediately because the status of the related reader
 *    (and thus the readcondition) hasn't changed after the first trigger (it didn't read the data).
 *
 *************************************************************************************************/
Test(vddsc_waitset_triggering, on_readcondition, .init=vddsc_waitset_init, .fini=vddsc_waitset_fini)
{
    RoundTripModule_DataType sample;
    dds_attach_t triggered;
    thread_arg_t arg;
    dds_return_t ret;

    memset(&sample, 0, sizeof(RoundTripModule_DataType));

    /* Make sure that we start un-triggered. */
    ret = dds_triggered(readcond);
    cr_assert_eq(ret, 0, "dds_triggered: Invalid return code %d", dds_err_nr(ret));

    /* Attach condition to the waitset. */
    ret = dds_waitset_attach(waitset, readcond, (dds_attach_t)(intptr_t)readcond);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach(): returned %d", dds_err_nr(ret));

    /* Start a thread that'll wait because no entity attached to the waitset
     * has been triggered (for instance by a status change). */
    waiting_thread_start(&arg, readcond);

    /* Writing data should unblock the thread. */
    ret = dds_write(writer, &sample);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_write(): returned %d", dds_err_nr(ret));
    ret = waiting_thread_expect_exit(&arg);
    cr_assert_eq(ret, DDS_RETCODE_OK, "waiting thread did not unblock");

    /* Now the trigger state should be true. */
    ret = dds_triggered(readcond);
    cr_assert_gt(ret, 0, "dds_triggered: Invalid return code %d", dds_err_nr(ret));

    /* Waitset shouldn't wait, but immediately return our reader. */
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(1));
    cr_assert_eq(ret, 1, "dds_waitset_wait ret");
    cr_assert_eq(readcond, (dds_entity_t)(intptr_t)triggered, "dds_waitset_wait attachment");

    /* Detach condition. */
    ret = dds_waitset_detach(waitset, readcond);
    cr_assert_eq(ret, DDS_RETCODE_OK, "dds_waitset_attach: Invalid return code %d", dds_err_nr(ret));
}






/**************************************************************************************************
 *
 * Convenience support functions.
 *
 *************************************************************************************************/

static uint32_t
waiting_thread(void *a)
{
    thread_arg_t *arg = (thread_arg_t*)a;
    dds_attach_t triggered;
    dds_return_t ret;

    arg->state = WAITING;
    /* This should block until the main test released all claims. */
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(1000));
    cr_assert_eq(ret, 1, "dds_waitset_wait returned %d", ret);
    cr_assert_eq(arg->expected, (dds_entity_t)(intptr_t)triggered, "dds_waitset_wait attachment");
    arg->state = STOPPED;

    return 0;
}

static os_result
thread_reached_state(thread_state_t *actual, thread_state_t expected, int32_t msec)
{
    /* Convenience function. */
    bool stopped = false;
    os_time msec10 = { 0, 10000000 };
    while ((msec > 0) && (*actual != expected)) {
        os_nanoSleep(msec10);
        msec -= 10;
    }
    return (*actual == expected) ? os_resultSuccess : os_resultTimeout;
}

static void
waiting_thread_start(struct thread_arg_t *arg, dds_entity_t expected)
{
    os_threadId   thread_id;
    os_threadAttr thread_attr;
    os_result     osr;

    assert(arg);

    /* Create an other thread that will blocking wait on the waitset. */
    arg->expected = expected;
    arg->state   = STARTING;
    os_threadAttrInit(&thread_attr);
    osr = os_threadCreate(&thread_id, "waiting_thread", &thread_attr, waiting_thread, arg);
    cr_assert_eq(osr, os_resultSuccess, "os_threadCreate");

    /* The thread should reach 'waiting' state. */
    osr = thread_reached_state(&(arg->state), WAITING, 1000);
    cr_assert_eq(osr, os_resultSuccess, "waiting returned %d", osr);

    /* But thread should block and thus NOT reach 'stopped' state. */
    osr = thread_reached_state(&(arg->state), STOPPED, 100);
    cr_assert_eq(osr, os_resultTimeout, "waiting returned %d", osr);

    arg->tid = thread_id;
}

static dds_return_t
waiting_thread_expect_exit(struct thread_arg_t *arg)
{
    os_result osr;
    assert(arg);
    osr = thread_reached_state(&(arg->state), STOPPED, 5000);
    if (osr == os_resultSuccess) {
        os_threadWaitExit(arg->tid, NULL);
        return DDS_RETCODE_OK;
    }
    return DDS_RETCODE_TIMEOUT;
}
