#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(ts, test_1) {

  dds_entity_t participant, participant2, participant3;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant, NULL, "dds_participant_create");

  participant2 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant2, NULL, "dds_participant_create");

  dds_delete (participant);
  dds_delete (participant2);

  participant3 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_neq(participant3, NULL, "dds_participant_create");

  dds_delete (participant3);

}

