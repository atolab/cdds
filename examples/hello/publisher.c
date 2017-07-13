#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>

/* 
  HelloWorldData Example creates a msg with an UserId and
  publishes it. The subscriber picks up the msg
*/

int main (int argc, char ** argv)
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t publisher;
  dds_entity_t writer;
  int status = 0;
  HelloWorldData_Msg msg;
  dds_qos_t * wqos;
  const char * partitions[1];

  /* Create a Participant */

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create a Topic */

  topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
  DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create a Publisher */

  publisher = dds_create_publisher (participant, NULL, NULL);
  DDS_ERR_CHECK (publisher, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Set QoS for Writer */

  wqos = dds_qos_create ();
  dds_qset_durability (wqos, DDS_DURABILITY_TRANSIENT);
  dds_qset_reliability (wqos, DDS_RELIABILITY_RELIABLE, DDS_SECS (1));
  partitions[0] = "HelloWorld example";
  dds_qset_partition (wqos, 1, partitions);

  /* Create a Writer */

  writer = dds_create_writer (publisher, topic, wqos, NULL);
  DDS_ERR_CHECK (writer, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (wqos);

  /* Create a msg to write */

  msg.userID = 1;
  msg.message = "Hello World";
  
  printf ("\n=== [Publisher] Writing : ");
  printf ("Message (%d, %s)\n", msg.userID, msg.message);

  status = dds_write (writer, &msg);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Sleep to allow delivery */

  dds_sleepfor (DDS_SECS (1));

  /* Clean up */

  status = dds_instance_dispose (writer, &msg);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_delete (participant);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  return 0;
}
