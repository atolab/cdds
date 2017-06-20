#include "dds.h"
#include "RoundTrip.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(vddsc, reader_creation)
{
  dds_entity_t participant;
  dds_entity_t reader, reader2, reader3;
  dds_entity_t subscriber;
  dds_entity_t topic, topic2;
  dds_listener_t *listener;
  dds_qos_t *qos, *qos2;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  listener = dds_listener_create(NULL);

  /*Creating reader with participant (and with qos) */

  topic = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  qos = dds_qos_create ();
  reader = dds_create_reader (participant, topic, qos, NULL);
  cr_assert_gt(reader, 0, "dds_create_reader() 1st");
  dds_delete(reader);

  /* Creating reader with subscriber (and with qos)*/
  qos2 = dds_qos_create ();
  subscriber = dds_create_subscriber (participant, qos2, NULL);
  reader2 = dds_create_reader (subscriber, topic, qos2, NULL);
  cr_assert_gt(reader2, 0, "dds_create_reader() 2nd");
  dds_delete(reader2);
  dds_delete(topic);


  /*Creating reader with participant (and without qos) */

  topic2 = dds_create_topic (participant, &RoundTripModule_DataType_desc, "RoundTrip", NULL, listener);
  reader3 = dds_create_reader (participant, topic2, NULL, NULL);
  cr_assert_gt(reader3, 0, "dds_create_reader() 3rd");
  dds_delete(reader3);
  dds_delete(topic2);

  dds_qos_delete(qos2);
  dds_qos_delete(qos);
  dds_listener_delete(listener);
  dds_delete(subscriber);
  dds_delete(participant);
}
