#include <criterion/criterion.h>
#include <criterion/abort.h>
#include "dds.h"
#include "os/os.h"

static dds_entity_t participant;
static bool handlerCalled;

static void
setup(void)
{
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0);
}

static void
putes(void)
{
    dds_delete(participant);
}

static void
vddsc_log__test_fail_fn(_In_z_ const char *msg_str, _In_z_ const char *where_str)
{
    handlerCalled = true;
}

/* Just tests basic API availability */
Test(vddsc_log, basic_info, .init = setup, .fini = putes)
{
    /* Test only for API availability and not crashing. TODO: Check actual log-output. */
    dds_log_info("Test dds_log_info after having created participant %d\n", participant);
}
Test(vddsc_log, basic_warn, .init = setup, .fini = putes)
{
    /* Test only for API availability and not crashing. TODO: Check actual log-output. */
    dds_log_warn("Test dds_log_warn after having created participant %d\n", participant);
}
Test(vddsc_log, basic_error, .init = setup, .fini = putes)
{
    /* Test only for API availability and not crashing. TODO: Check actual log-output. */
    dds_log_error("Test dds_log_error after having created participant %d\n", participant);
}
Test(vddsc_log, basic_fatal, .init = setup, .fini = putes)
{
    dds_fail_fn old;

    old = dds_fail_set(&vddsc_log__test_fail_fn);
    cr_expect_not_null(old, "The default failure handler should be set");
    old = dds_fail_get();
    cr_expect_eq(old, &vddsc_log__test_fail_fn, "The new failure handler should be set");
    /* Test only for API availability and not crashing. TODO: Check actual log-output. */
    dds_log_fatal("Test dds_log_fatal after having created participant %d\n", participant);
    cr_expect(handlerCalled, "The installed handler should be called after dds_log_fatal");
    old = dds_fail_set(NULL);
    cr_expect_eq(old, &vddsc_log__test_fail_fn, "The old failure handler should be vddsc_log__test_fail_fn");
    handlerCalled = false;
    old = dds_fail_get();
    cr_expect_null(old, "The new failure handler should be NULL");
    dds_log_fatal("Test dds_log_fatal after having created participant %d\n", participant);
    cr_expect_not(handlerCalled, "The old handler should not be called after dds_log_fatal");
}

Test(vddsc_log, basic_err_check, .exit_code = 1)
{
    dds_entity_t fake;
    uint32_t flags = DDS_CHECK_REPORT;

    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK(participant, flags);

    OS_WARNING_MSVC_OFF(28020);
    fake = dds_create_writer(0, 0, NULL, NULL); /* Supposed to fail */
    OS_WARNING_MSVC_ON(28020);

    (void) dds_fail_set(&vddsc_log__test_fail_fn);

    DDS_ERR_CHECK(fake, DDS_CHECK_REPORT);
    DDS_ERR_CHECK(fake, DDS_CHECK_FAIL);
    cr_expect(handlerCalled, "The installed handler should be called after DDS_ERR_CHECK(fake, DDS_CHECK_FAIL)");
    handlerCalled = false;
    DDS_ERR_CHECK(fake, DDS_CHECK_FAIL | DDS_CHECK_REPORT);
    cr_expect(handlerCalled, "The installed handler should be called after DDS_ERR_CHECK(fake, DDS_CHECK_FAIL | DDS_CHECK_REPORT)");
    DDS_ERR_CHECK(fake, DDS_CHECK_FAIL | DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* This line should not be reached. If it does, it is a test-failure. */
    criterion_abort_test();
}
