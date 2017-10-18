#include "vddsc/dds.h"
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

Test(vddsc_participant_lookup, one) {

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
  cr_assert_eq(participants[0], participant,"dds_lookup_participant did not return the participant");

  dds_delete (participant);
}

Test(vddsc_participant_lookup, multiple) {

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
  cr_assert(participants[0] == participant || participants[0] == participant2);
  cr_assert(participants[1] == participant || participants[1] == participant2);
  cr_assert_neq(participants[0], participants[1], "dds_lookup_participant returned a participant twice");

  dds_delete (participant2);
  dds_delete (participant);
}

Test(vddsc_participant_lookup, array_too_small) {

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
  cr_assert(participants[0] == participant || participants[0] == participant2 || participants[0] == participant3);
  cr_assert(participants[1] == participant || participants[1] == participant2 || participants[1] == participant3);
  cr_assert_neq(participants[0], participants[1], "dds_lookup_participant returned a participant twice");

  dds_delete (participant3);
  dds_delete (participant2);
  dds_delete (participant);
}

Test(vddsc_participant_lookup, null_zero){

  dds_entity_t participant;
  dds_domainid_t domain_id;
  dds_return_t status, num_of_found_pp;
  size_t size = 0;

  /* Create a participant */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  /* Get domain id */
  status = dds_get_domainid(participant, &domain_id);
  cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(participant, domain_id)");

  num_of_found_pp = dds_lookup_participant( domain_id, NULL, size);
  cr_assert_eq(num_of_found_pp, 1, "dds_lookup_participant(domain_id, participants, size)");

  dds_delete (participant);
}

Test(vddsc_participant_lookup, null_nonzero){

  dds_entity_t participant;
  dds_domainid_t domain_id;
  dds_return_t status, num_of_found_pp;
  size_t size = 2;

  /* Create a participant */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  /* Get domain id */
  status = dds_get_domainid(participant, &domain_id);
  cr_assert_status_eq(status, DDS_RETCODE_OK, "dds_get_domainid(participant, domain_id)");

  num_of_found_pp = dds_lookup_participant( domain_id, NULL, size);
  cr_assert_status_eq(num_of_found_pp, DDS_RETCODE_BAD_PARAMETER, "dds_lookup_participant did not return bad parameter");

  dds_delete (participant);
}

Test(vddsc_participant_lookup, unknown_id) {

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
  domain_id ++;

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 0, "dds_lookup_participant(domain_id, participants, size)");

  dds_delete (participant);
}

Test(vddsc_participant_lookup, none) {

  dds_entity_t participants[2];
  dds_return_t num_of_found_pp;
  size_t size = 2;

  num_of_found_pp = dds_lookup_participant( 0, participants, size);
  cr_assert_eq(num_of_found_pp, 0, "dds_lookup_participant did not return 0");
}

Test(vddsc_participant_lookup, no_more) {

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

  dds_delete (participant);

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 0, "dds_lookup_participant did not return 0");
}

Test(vddsc_participant_lookup, deleted) {

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

  dds_delete (participant2);

  num_of_found_pp = dds_lookup_participant( domain_id, participants, size);
  cr_assert_eq(num_of_found_pp, 1, "dds_lookup_participant did not return one participant");
  cr_assert(participants[0] == participant);

  dds_delete (participant);
}
