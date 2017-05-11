#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* Note:
   By default, cr_log_info() messages are not displayed.
   To have these messages displayed you can set the environment variable CRITERION_VERBOSITY_LEVEL.
   A value 0 or 1 should enable the display of these messages.
*/


/* Placeholder */
void setup(void) {
  cr_log_info("Runs before the test");
}

/* Placeholder */
void teardown(void) {
    cr_log_info("Runs after the test");
}


Test(ts, test_2, .init = setup, .fini = teardown) {

  dds_entity_t participant;
  int status;

  cr_log_info("Starting test %d", 2);
  status = dds_init (0, NULL);
  cr_assert_eq(status, DDS_RETCODE_OK, "dds_init");
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_eq(status, DDS_RETCODE_OK, "dds_participant_create");
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  cr_log_info("Stopping test %d", 2);


  dds_delete (participant);

  dds_fini ();
}

#if 1
Test(ts, test_3, .init = setup, .fini = teardown) {

  dds_entity_t participant;
  int status;

  cr_log_info("Starting test %d", 3);
  status = dds_init (0, NULL);
  cr_assert_eq(status, DDS_RETCODE_OK, "dds_init");
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_eq(status, DDS_RETCODE_OK, "dds_participant_create");
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  cr_log_info("Stopping test %d", 3);


  dds_delete (participant);

  dds_fini ();
}
#endif

