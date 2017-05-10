#include <assert.h>
#include "CUnit/CUnit.h"
#include "CUnit/Automated.h"

#include "dds.h"


static dds_entity_t entity = NULL;
static bool force_assert = false;

#define TEST_CHECK_STATUS(s, e) \
    if (dds_err_nr(s) != e) {    \
        CU_ASSERT(dds_err_nr(s) == e); \
        assert(!force_assert); \
    }

#define TEST_CHECK_BOOL(b) \
    if (!(b)) {    \
        CU_ASSERT(b); \
        assert(!force_assert); \
    }


/* tests */
void test_entity_creation(void)
{
    dds_result_t status;

    /* Use participant as entity in the tests. */
    status = dds_participant_create (&entity, DDS_DOMAIN_DEFAULT, NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
}

void test_entity_enabling(void)
{
    dds_result_t status;

    /* Check enabling with bad parameters. */
    status = dds_enable(NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Check actual enabling. */
    /* TODO: CHAM-96: Check enabling.
    status = dds_enable(&entity);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    */

    /* Check re-enabling. */
    status = dds_enable(entity);
    TEST_CHECK_STATUS(status, DDS_RETCODE_PRECONDITION_NOT_MET);
}

void test_entity_qos(void)
{
    dds_result_t status;
    dds_qos_t *qos1 = dds_qos_create();
    dds_qos_t *qos2 = dds_qos_create();

    /* Don't check inconsistent and immutable policies. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting QoS with bad parameters. */
    status = dds_get_qos (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_qos (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_qos (NULL, qos1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get QoS. */
    status = dds_get_qos (entity, qos1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);

    /* Check setting QoS with bad parameters. */
    status = dds_set_qos (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_set_qos (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_set_qos (NULL, qos2);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Entity (partitition) is enabled, so we shouldn't be able to set QoS. */
    /* Checking all QoS internals (also related to enabled/disabled) should be
     * done by a QoS test and specific 'child' entities. */
    status = dds_set_qos (entity, qos2);
    TEST_CHECK_STATUS(status, DDS_RETCODE_IMMUTABLE_POLICY);

    dds_qos_delete(qos1);
    dds_qos_delete(qos2);
}

void test_entity_listeners(void)
{
    dds_result_t status;
    c99_listener_cham65_t *l1 = NULL;
    c99_listener_cham65_t *l2 = dds_listener_create();

    /* Don't check actual workings of the listeners. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting Listener with bad parameters. */
    status = dds_get_listener (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_listener (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_listener (NULL, &l1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get Listener, which should be NULL. */
    status = dds_get_listener (entity, &l1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(l1 == NULL);

    /* Check setting Listener with bad parameters. */
    status = dds_set_listener (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_set_listener (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_set_listener (NULL, l2);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Getting after setting should return set listener. */
    status = dds_set_listener (entity, l2);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    status = dds_get_listener (entity, &l1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(l1 == l2);

    dds_free(l2);
}

void test_entity_status(void)
{
    dds_result_t status;
    uint32_t s1 = 0;

    /* Don't check actual bad statusses. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting Status with bad parameters. */
    status = dds_get_enabled_status (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_enabled_status (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_enabled_status (NULL, &s1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get Status, which should be 0 for a participant. */
    status = dds_get_enabled_status (entity, &s1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(s1 == 0);

    /* Check setting Status with bad parameters. */
    status = dds_set_enabled_status (NULL, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* I shouldn't be able to set statusses on a participant. */
    status = dds_set_enabled_status (entity, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    status = dds_set_enabled_status (entity, DDS_DATA_AVAILABLE_STATUS);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Check getting Status changes with bad parameters. */
    status = dds_get_status_changes (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_status_changes (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_status_changes (NULL, &s1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get Status change, which should be 0 for a participant. */
    status = dds_get_status_changes (entity, &s1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(s1 == 0);

    /* Status read and take shouldn't work on participant. */
    status = dds_read_status (entity, &s1, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_take_status (entity, &s1, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
}


void test_entity_handle(void)
{
    dds_result_t status;
    dds_instance_handle_t hdl;

    /* Don't check actual handle contents. That's a job
     * for the specific entity children, not for the generic part. */

    /* Check getting Handle with bad parameters. */
    status = dds_instancehandle_get (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_instancehandle_get (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_instancehandle_get (NULL, &hdl);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get Instance Handle, which should not be 0 for a participant. */
    status = dds_instancehandle_get (entity, &hdl);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(hdl != 0);
}

void test_entity_get_entities(void)
{
    dds_result_t status;
    dds_entity_t par = NULL;
    dds_entity_t child;

    /* ---------- Get Parent ------------ */

    /* Check getting Parent with bad parameters. */
    par = dds_get_parent (NULL);
    TEST_CHECK_BOOL(par == NULL);

    /* Get Parent, a participant doesn't have a parent. */
    par = dds_get_parent (entity);
    TEST_CHECK_BOOL(par == NULL);

    /* ---------- Get Participant ------------ */

    /* Check getting Participant with bad parameters. */
    par = dds_get_participant (NULL);
    TEST_CHECK_BOOL(par == NULL);

    /* Get Participant, a participants' participant is itself. */
    par = dds_get_participant (entity);
    TEST_CHECK_BOOL(par == entity);

    /* ---------- Get Children ------------ */

    /* Check getting Children with bad parameters. */
    status = dds_get_children (NULL, &child, 1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_children (entity, NULL, 1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_children (entity, &child, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_children (NULL, NULL, 1);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_children (NULL, &child, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_children (entity, NULL, 0);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get Childrent, of which there are currently none. */
    status = dds_get_children (entity, &child, 1);
    TEST_CHECK_BOOL(status == 0);
}

void test_entity_get_domainid(void)
{
    int status;
    dds_domainid_t id;

    /* Check getting ID with bad parameters. */
    status = dds_get_domainid (NULL, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_domainid (entity, NULL);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);
    status = dds_get_domainid (NULL, &id);
    TEST_CHECK_STATUS(status, DDS_RETCODE_BAD_PARAMETER);

    /* Get and check the domain id. */
    status = dds_get_domainid (entity, &id);
    TEST_CHECK_STATUS(status, DDS_RETCODE_OK);
    TEST_CHECK_BOOL(id == 0);
}

void test_entity_deletion(void)
{
    /* TODO: Deletion will be different?? */
    dds_entity_delete (entity);
    entity = NULL;
}


int main (int argc, char **argv)
{
    CU_pSuite pSuite = NULL;
    int err = 0;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* add a suite to the registry */
    pSuite = CU_add_suite("C99::Entity API test suite", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* TODO: This should be removed... */
    dds_init (argc, argv);

    /* add test cases to the test suite */
    if (NULL == CU_add_test(pSuite, "Check generic entity QoS creation", test_entity_creation)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity enabling", test_entity_enabling)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity QoS handling", test_entity_qos)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity Listener handling", test_entity_listeners)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity Status handling", test_entity_status)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity Instance handle", test_entity_handle)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity get contained entities", test_entity_get_entities)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity get domain id", test_entity_get_domainid)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite, "Check generic entity QoS deletion", test_entity_deletion)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Automated interface */
    CU_set_output_filename ("cunit");
    CU_list_tests_to_file ();
    CU_automated_run_tests ();

    err = (int)CU_get_number_of_asserts();

    /* cleanup registry */
    CU_cleanup_registry();

    /* TODO: This should be removed... */
    dds_fini ();

    /* ctest requires the test executable to return 0 when succuss, non-null when fail */

    return err;
    //return CU_get_error();
}


