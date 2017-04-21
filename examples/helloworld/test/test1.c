#include "dds.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;
static int ac;
static char **av;


void testcase_1(void) {

  dds_entity_t participant;
  int status;

  status = dds_init (ac, av);
  CU_ASSERT(status == DDS_RETCODE_OK);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  CU_ASSERT(status == DDS_RETCODE_OK);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  dds_entity_delete (participant);

  dds_fini ();
}


/* This test demonstrates a failing test */
void testcase_2(void) {

  int status;

  status = 1;
  CU_ASSERT(status == 3);
}


int main (int argc, char *argv[])
{

   CU_pSuite pSuite = NULL;

   ac = argc;
   av = argv;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   /* add a suite to the registry */
   pSuite = CU_add_suite("HelloWorld test suite", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add test cases to the test suite */ 
   if (NULL == CU_add_test(pSuite, "dds_participant_create", testcase_1)) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if (NULL == CU_add_test(pSuite, "bogus_failing_testcase_2", testcase_2)) {
      CU_cleanup_registry();
      return CU_get_error();
   }


   /* Run all tests using the CUnit Basic interface */ 
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
 
   /* cleanup registry */
   CU_cleanup_registry();

  /* ctest requires the test executable to return 0 when succuss, non-null when fail */
  return CU_get_error();
}
