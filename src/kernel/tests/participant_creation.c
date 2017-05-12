#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */


Test(c99, participant_creation)
{
    dds_entity_t participant;
    int status;

    /* TODO: Replace with proper tests. */

    cr_log_info("Starting test %d", 2);
    status = dds_init (0, NULL);
    cr_assert_eq(status, DDS_RETCODE_OK, "dds_init");
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_eq(status, DDS_RETCODE_OK, "dds_participant_create");
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    cr_log_info("Stopping test %d", 2);
    status = dds_delete (participant);
    cr_assert_eq(status, DDS_RETCODE_OK, "dds_participant_delete");
    dds_fini ();
}


