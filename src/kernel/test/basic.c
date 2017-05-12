#include "dds.h"
#include "CUnit/CUnit.h"
#include "CUnit/Automated.h"


void test(void) {

  dds_entity_t participant;
  int status;

  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  CU_ASSERT(status == DDS_RETCODE_OK);

  /* TODO: CHAM-108: Add some simple read/write test(s). */

  status = dds_delete (participant);
  CU_ASSERT(status == DDS_RETCODE_OK);
}



int main (int argc, char *argv[])
{
    CU_pSuite pSuite = NULL;

    /* TODO: Remove dds_init (deprecated). */
    dds_init(0, NULL);

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* add a suite to the registry */
    pSuite = CU_add_suite("Basic C99 interface test", NULL, NULL);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add test cases to the test suite */
    if (NULL == CU_add_test(pSuite, "C99 Interface", test)) {
        CU_cleanup_registry();
        return CU_get_error();
    }


    /* Run all tests using the CUnit Automated interface */
    CU_set_output_filename ("cunit");
    CU_list_tests_to_file ();
    CU_automated_run_tests ();

    /* cleanup registry */
    CU_cleanup_registry();

    /* TODO: Remove dds_fini (deprecated). */
    dds_fini();

    /* ctest requires the test executable to return 0 when succuss, non-null when fail */
    return CU_get_error();
}

