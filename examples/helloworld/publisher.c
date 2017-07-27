#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>

/*
  The helloworld example sends a HelloWorldData_Msg from a publisher
  to a subscriber. For the example to work, the subscriber should
  already be running when executing the publisher application.

  The publisher creates a HelloWorldData_Msg sample with an UserId
  and a string message and publishes it.

  The subscriber picks up that sample and displays it.
*/

int main (int argc, char ** argv)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;
    dds_return_t ret;
    HelloWorldData_Msg msg;

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Writer. */
    writer = dds_create_writer (participant, topic, NULL, NULL);
    DDS_ERR_CHECK (writer, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Sleep to allow discovery of reader by writer and vice versa. */
    dds_sleepfor (DDS_SECS (1));

    /* Create a message to write. */
    msg.userID = 1;
    msg.message = "Hello World";

    printf ("\n=== [Publisher] Writing : ");
    printf ("Message (%d, %s)\n", msg.userID, msg.message);

    ret = dds_write (writer, &msg);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Sleep to allow delivery. */
    dds_sleepfor (DDS_SECS (1));

    /* Deleting the participant will delete all its children recursively as well. */
    ret = dds_delete (participant);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    return 0;
}
