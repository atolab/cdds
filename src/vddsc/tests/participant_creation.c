#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(ts, pc) {

  dds_entity_t participant, participant2, participant3;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  participant2 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant2, 0, "dds_participant_create");

  dds_delete (participant);
  dds_delete (participant2);

  participant3 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant3, 0, "dds_participant_create");

  dds_delete (participant3);

}

