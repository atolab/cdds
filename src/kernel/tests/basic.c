#include "dds.h"
#include "CUnit/Runner.h"

CUnit_Test(c99_basic, test)
{
  dds_entity_t participant;
  int status;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  CU_ASSERT(participant != NULL);

  /* TODO: CHAM-108: Add some simple read/write test(s). */

  status = dds_delete(participant);
  CU_ASSERT(status == DDS_RETCODE_OK);
}

