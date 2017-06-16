#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(c99_basic, test)
{
  dds_entity_t participant;
  dds_return_t status;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);

  /* TODO: CHAM-108: Add some simple read/write test(s). */

  status = dds_delete(participant);
  cr_assert_eq(status, DDS_RETCODE_OK);
}

