#include "dds.h"
#include "cunitrunner/runner.h"

void testcase_1 (void)
{
    dds_entity_t participant;
    int status;

    status = dds_init(0, NULL);
    CU_ASSERT(status == DDS_RETCODE_OK);
    DDS_ERR_CHECK(status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    status = dds_participant_create(&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    CU_ASSERT(status == DDS_RETCODE_OK);
    DDS_ERR_CHECK(status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_entity_delete(participant);

    dds_fini();
}

int main (int argc, char *argv[])
{
    CU_pSuite examples;

    if (runner_init(argc, argv)) {
        goto err_init;
    }

    /* add a suite to the registry */
    if ((examples = CU_add_suite("examples/helloworld", NULL, NULL)) == NULL) {
        goto err;
    }

    /* add test cases to the test suite */
    if (CU_add_test(examples, "dds_participant_create", testcase_1) == NULL) {
        goto err;
    }

    runner_run();

err:
    runner_fini();
err_init:
    return CU_get_error();
}

