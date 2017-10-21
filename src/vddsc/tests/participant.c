#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "config_env.h"
#include "os/os.h"

#define cr_assert_status_eq(s1, s2, ...) cr_assert_eq(dds_err_nr(s1), s2, __VA_ARGS__)

void test_create_participant_domain_values(_In_     const dds_domainid_t valid_domain);

void test_create_participant_domain_values(_In_     const dds_domainid_t valid_domain){
  dds_entity_t participant, participant2, participant3, participant4, participant5;

  //invalid domain
  participant = dds_create_participant (1, NULL, NULL);
  cr_assert_lt(participant, 0, "Invalid participant with invalid domain");

  //valid specific domain value
  participant2 = dds_create_participant (valid_domain, NULL, NULL);
  cr_assert_gt(participant2, 0, "Valid participant with valid specific domain value");

  //DDS_DOMAIN_DEFAULT with no domain environment variable
  participant3 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant3, 0, "Valid participant with DDS_DOMAIN_DEFAULT with no domain environment variable");

  //DDS_DOMAIN_DEFAULT with invalid domain environment variable
  putenv("VORTEX_DOMAIN=2");
  participant4 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_lt(participant4, 0, "Invalid participant with DDS_DOMAIN_DEFAULT with invalid domain environment variable");

  //DDS_DOMAIN_DEFAULT with valid domain environment variable
  char *env_domain_str;
  env_domain_str = os_malloc(strlen("VORTEX_DOMAIN") + strlen("=") + 3 + 1);
  (void) sprintf(env_domain_str, "%s=%d", "VORTEX_DOMAIN", valid_domain);
  putenv(env_domain_str);
  participant5 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant5, 0, "Valid participant with DDS_DOMAIN_DEFAULT with valid domain environment variable");

  dds_delete (participant2);
  dds_delete (participant3);
  dds_delete (participant5);

}

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



Test(vddsc_participant, create_with_no_conf, .init = os_osInit, .fini = os_osExit) {

  const char * env_uri = os_getenv("VORTEXDDS_URI");
  const char * env_domain = os_getenv("VORTEX_DOMAIN");
  cr_assert_eq(env_uri, NULL, "VORTEXDDS_URI must be NULL");
  cr_assert_eq(env_domain, NULL, "VORTEX_DOMAIN must be NULL");

  test_create_participant_domain_values(0); //0 is the default domain value for no config file
}



Test(vddsc_participant, create_with_conf, .init = os_osInit, .fini = os_osExit) {

  char *env_uri_str;
  env_uri_str = os_malloc(strlen("VORTEXDDS_URI") + strlen("=") + strlen(CONFIG_ENV_SIMPLE_UDP) + 1);
  (void) sprintf(env_uri_str, "%s=%s", "VORTEXDDS_URI", CONFIG_ENV_SIMPLE_UDP);
  os_putenv(env_uri_str);

  char *env_mp_str;
  env_mp_str = os_malloc(strlen("MAX_PARTICIPANTS") + strlen("=") + strlen(CONFIG_ENV_MAX_PARTICIPANTS) + 1);
  (void) sprintf(env_mp_str, "%s=%s", "MAX_PARTICIPANTS", CONFIG_ENV_MAX_PARTICIPANTS);
  os_putenv(env_mp_str);


  const char * env_uri = os_getenv("VORTEXDDS_URI");
  const char * env_domain = os_getenv("VORTEX_DOMAIN");

  cr_assert_neq(env_uri, NULL, "VORTEXDDS_URI must be set");
  cr_assert_eq(env_domain, NULL, "VORTEX_DOMAIN must be NULL");

  test_create_participant_domain_values(3); //3 is the value in config file
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
