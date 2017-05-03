#include "CUnit/CUnit.h"
#include "CUnit/Automated.h"

#include "dds_listener.h"

#define ASSERT_CALLBACK_EQUAL(fntype, listener, expected) \
    { \
        dds_on_##fntype##_fn cb; \
        dds_lget_##fntype(listener, &cb); \
        if (cb == expected) { CU_PASS("Callback 'on_" #fntype " matched expected value '" #expected "'"); } \
        else { CU_FAIL("Callback 'on_" #fntype "' did not match expected value '" #expected "'"); } \
    }

/* dummy callbacks */

void sample_lost_cb(dds_entity_t reader, const dds_sample_lost_status_t status)
{
    printf("sample_lost_cb called\n");
}

void data_available_cb(dds_entity_t reader)
{
    printf("data_available_cb called\n");
}


/* tests */

void test_create_and_delete(void)
{
    /* Verify create doesn't return null */
    dds_listener_t *listener = NULL;
    listener = dds_listener_create();
    CU_ASSERT_PTR_NOT_NULL(listener);

    /* Check default cb's are set */
    ASSERT_CALLBACK_EQUAL(inconsistent_topic, listener, NULL);
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
}

void test_reset(void)
{
    dds_listener_t *listener = NULL;
    listener = dds_listener_create();
    CU_ASSERT_PTR_NOT_NULL(listener);

    /* Set a listener cb to a non-default value */
    dds_lset_data_available(listener, NULL);
    ASSERT_CALLBACK_EQUAL(data_available, listener, NULL);

    /* Listener cb should revert to default after reset */
    dds_listener_reset(listener);
    ASSERT_CALLBACK_EQUAL(data_available, listener, DDS_LUNSET);
}

void test_copy(void)
{
    dds_listener_t *listener1 = NULL, *listener2 = NULL;
    listener1 = dds_listener_create();
    listener2 = dds_listener_create();
    CU_ASSERT_PTR_NOT_NULL(listener1);
    CU_ASSERT_PTR_NOT_NULL(listener2);

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
}

void test_merge(void)
{
    dds_listener_t *listener1 = NULL, *listener2 = NULL;
    listener1 = dds_listener_create();
    listener2 = dds_listener_create();
    CU_ASSERT_PTR_NOT_NULL(listener1);
    CU_ASSERT_PTR_NOT_NULL(listener2);

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
}

void test_getters(void)
{

}

void test_setters(void)
{

}


int main (int argc, char **argv)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* add a suite to the registry */
    pSuite = CU_add_suite("C99::Listener test suite", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add test cases to the test suite */
    if (NULL == CU_add_test(pSuite, "Create and delete a dds_listener_t instance", test_create_and_delete)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Reset a dds_listener_t instance to its default values", test_reset)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Copy a dds_listener_t instance", test_copy)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Merge two dds_listener_t instances", test_merge)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
#if 0
    if (NULL == CU_add_test(pSuite, "Set dds_listener_t callbacks", test_getters)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Get dds_listener_t callbacks", test_setters)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
#endif

    /* Run all tests using the CUnit Automated interface */
    CU_set_output_filename ("cunit");
    CU_list_tests_to_file ();
    CU_automated_run_tests ();

    /* cleanup registry */
    CU_cleanup_registry();

    /* ctest requires the test executable to return 0 when succuss, non-null when fail */
    return CU_get_error();
}
