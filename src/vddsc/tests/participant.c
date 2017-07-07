#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

Test(vddsc_participant, create_and_delete) {

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

Test(vddsc_participant_1, lookup) {

  dds_entity_t participant;
  dds_entity_t participants[3];
  dds_domainid_t domain_id;
  dds_return_t status, num_of_found_pp;
  size_t size = 3;

  /* Create a participant */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  /* Get domain id */
  status = dds_get_domainid(participant, &domain_id);
  cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(participant, domain_id)");

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 1, "dds_lookup_participant(domain_id, participants, size)");

  dds_delete (participant);
}

Test(vddsc_participant_2, lookup) {

  dds_entity_t participant, participant2;
  dds_entity_t participants[2];
  dds_domainid_t domain_id;
  dds_return_t status, num_of_found_pp;
  size_t size = 2;

  /* Create participants */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  participant2 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant2, 0, "dds_participant_create");

  /* Get domain id */
  status = dds_get_domainid(participant, &domain_id);
  cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(participant, domain_id)");

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 2, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert(participants[0] == participant || participants[0] == participant2, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert(participants[1] == participant || participants[1] == participant2, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert_neq(participants[0], participants[1], "dds_lookup_participant(domain_id, participants, size)");

  dds_delete (participant2);
  dds_delete (participant);
}

Test(vddsc_participant_3, lookup) {

  dds_entity_t participant, participant2, participant3;
  dds_entity_t participants[2];
  dds_domainid_t domain_id;
  dds_return_t status, num_of_found_pp;
  size_t size = 2;

  /* Create participants */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  participant2 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant2, 0, "dds_participant_create");

  participant3 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant3, 0, "dds_participant_create");

  /* Get domain id */
  status = dds_get_domainid(participant, &domain_id);
  cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(participant, domain_id)");

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 3, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert(participants[0] == participant || participants[0] == participant2 || participants[0] == participant3, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert(participants[1] == participant || participants[1] == participant2 || participants[0] == participant3, "dds_lookup_participant(domain_id, participants, size)");
  cr_assert_neq(participants[0], participants[1], "dds_lookup_participant(domain_id, participants, size)");

  dds_delete (participant3);
  dds_delete (participant2);
  dds_delete (participant);
}
