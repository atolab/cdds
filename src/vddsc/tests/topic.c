#include "dds.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(vddsc, topic_creation)
{
  dds_entity_t participant;
  dds_entity_t topic, topic2;
  dds_return_t retCode;
  dds_listener_t *listener;
  dds_qos_t *qos;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);

  /* Creating initial topic (with listener) should succeed. */
  listener = dds_listener_create(NULL);
  cr_assert_neq(listener, NULL, "dds_listener_create(NULL)");
  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_gt(topic, 0, "dds_create_topic(RoundTrip) 1st");

  /* Creating the same topic (without listener) should fail.  */
  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  cr_assert_eq(dds_err_nr(topic2), DDS_RETCODE_PRECONDITION_NOT_MET, "dds_create_topic(RoundTrip) 2nd");


  /* Creating the same topic (same type, different name) should succeed.  */
  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "TrippedyTrip", NULL, NULL);
  cr_assert_gt(topic2, 0, "dds_create_topic(TrippedyTrip) 1st");
  dds_delete(topic2);

  /* Re-creating previously created topic should succeed.  */
  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "TrippedyTrip", NULL, NULL);
  cr_assert_gt(topic2, 0, "dds_create_topic(TrippedyTrip) 2nd");
  dds_delete(topic2);

  /* Creating the same topic (with default QoS) should succeed.  */
  qos = dds_qos_create();
  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "TrippedyTrip2", qos, NULL);
  cr_assert_gt(topic2, 0, "dds_create_topic(Compatible)");
  dds_delete(topic2);
  dds_qos_delete(qos);

  /* Creating the same topic (with inconsistent QoS) should fail.  */
  qos = dds_qos_create();
  dds_qset_lifespan(qos, -2000000000);
  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "Inconsistent", qos, NULL);
  cr_assert_eq(dds_err_nr(topic2), DDS_RETCODE_INCONSISTENT_POLICY, "dds_create_topic(Inconsistent)");
  dds_qos_delete(qos);

  /* Creating the different topic with same name should fail.  */
  topic2 = dds_create_topic (participant, &RoundTripModule_Address_desc, "RoundTrip", NULL, NULL);
  cr_assert_eq(dds_err_nr(topic2), DDS_RETCODE_PRECONDITION_NOT_MET, "dds_create_topic(RoundTrip) other");

  dds_delete (topic);
  dds_listener_delete(listener);
  dds_delete (participant);
}

Test(vddsc, topic_find)
{
    dds_entity_t participant;
    dds_entity_t topic, found;

    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);

    found = dds_find_topic(participant, "RoundTripFind");
    cr_assert_eq(dds_err_nr(found), DDS_RETCODE_PRECONDITION_NOT_MET, "dds_find_topic(RoundTripFind) 1st");

    topic = dds_create_topic(participant, &RoundTripModule_DataType_desc, "RoundTripFind", NULL, NULL);
    cr_assert_gt(topic, 0, "dds_create_topic(RoundTripFind)");

    found = dds_find_topic(participant, "RoundTripFind");
    cr_assert_gt(found, 0, "dds_find_topic(RoundTripFind) 2nd");
    dds_delete(found);

    dds_delete(topic);

    found = dds_find_topic(participant, "RoundTripFind");
    cr_assert_eq(dds_err_nr(found), DDS_RETCODE_PRECONDITION_NOT_MET, "dds_find_topic(RoundTripFind) 3nd");

    dds_delete(participant);
}

Test(vddsc, topic_names)
{
  dds_entity_t participant;
  dds_entity_t topic, topic2;
  dds_return_t retCode;
  int size = 200;
  char name[size];

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);

  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, NULL);
  cr_assert_gt(topic, 0, "dds_create_topic");
  topic2 = dds_create_topic (participant, &RoundTripModule_Address_desc, "UDPRoundTrip", NULL, NULL);
  cr_assert_gt(topic2, 0, "dds_create_topic");

  retCode = dds_get_type_name(topic, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_type_name");
  cr_assert_str_eq(name, "RoundTripModule::DataType", "Type name is expected to be RoundTripModule::DataType");

  retCode = dds_get_type_name(topic2, name, size);
  cr_assert_eq(retCode, DDS_RETCODE_OK, "dds_get_type_name");
  cr_assert_str_eq(name, "RoundTripModule::Address", "Type name is expected to be RoundTripModule::Address");

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
