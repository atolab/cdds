#include "dds.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(c99_topic, pc) {
  dds_entity_t participant;
  dds_entity_t topic, topic2;
  dds_return_t retCode;
  char name[200];
  int size = sizeof(name);

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);

  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  cr_assert_neq(topic, NULL, "dds_create_topic");
  topic2 = dds_create_topic (participant, &RoundTripModule_Address_desc, "UDPRoundTrip", NULL, NULL);
  cr_assert_neq(topic2, NULL, "dds_create_topic");

  retCode = dds_get_type_name(topic, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_type_name");
  cr_assert_str_neq(name, "RoundTripModule::DataType", "Type name is expected to be RoundTripModule::DataType");

  retCode = dds_get_type_name(topic2, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_type_name");
  cr_assert_str_neq(name, "RoundTripModule::Address", "Type name is expected to be RoundTripModule::Address");

  retCode = dds_get_name(topic, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_name");
  cr_assert_str_eq(name, "RoundTrip", "Name is expected to be RoundTrip");
  retCode = dds_get_name(topic2, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_name");
  cr_assert_str_eq(name, "UDPRoundTrip", "Name is expected to be UDPRoundTrip");

  dds_delete (topic);
  dds_delete (topic2);
  dds_delete (participant);
}

