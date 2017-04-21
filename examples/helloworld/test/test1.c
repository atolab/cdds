#include "dds.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#define XPASS(x)    (0)

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;
static int ac;
static char **av;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void)
{
   if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
      return -1;
   }
   else {
      return 0;
   }
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
   if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }
}



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
  CU_ASSERT(status == DDS_RETCODE_OK);
}

/* This test demonstrates an XPASS test */
void testcase_3(void) {

  int status;

  status = 1;
  CU_ASSERT(XPASS(status == DDS_RETCODE_OK));
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
   pSuite = CU_add_suite("HelloWorld test suite", init_suite1, clean_suite1);
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

   if (NULL == CU_add_test(pSuite, "bogus_xpass_testcase_3", testcase_3)) {
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
