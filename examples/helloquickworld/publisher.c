#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>

/*
  The helloquickworld example is an extension of the helloworld
  example. If you haven't looked at that yet, please do.

  The helloworld contains a few sleeps and polling. These are
  removed and improved upon in this helloquickworld example.
*/

int main (int argc, char ** argv)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;
    dds_entity_t waitset;
    dds_return_t ret;
    dds_qos_t *qos;
    HelloWorldData_Msg msg;

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloQuickWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Writer. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    writer = dds_create_writer (participant, topic, qos, NULL);
    DDS_ERR_CHECK (writer, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete(qos);

    /*
     * Before writing, the writer should have discovered the reader.
     * This can be done by waiting for the publication matched event.
     * For that to happen, we need to:
     *   - Indicate our interest for that on the writer.
     *   - Create a waitset.
     *   - Attach the writer to the waitset.
     *   - The waitset.wait will unblock when the publication is matched,
     *     indicating that a reader has been discovered.
     */
    ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    waitset = dds_create_waitset(participant);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    ret = dds_waitset_attach(waitset, writer, (dds_attach_t)NULL);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* The dds_waitset_wait returns the number of triggered entities,
     * which can only be '1' because only 1 was attached. Returning
     * 0 means that within the timeout, no entities were triggered.
     * Returning a negative value indicates an error. */
    printf ("\n=== [Publisher]  Waiting for a reader ...\n");
    ret = dds_waitset_wait(waitset, NULL, 0, DDS_SECS(30));
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    if (ret > 0)
    {
        /* Create a message to write. */
        msg.userID = 1;
        msg.message = "Hello Quick World";

        printf ("=== [Publisher]  Writing : ");
        printf ("Message (%d, %s)\n", msg.userID, msg.message);

        ret = dds_write (writer, &msg);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    }
    else
    {
        printf ("=== [Publisher]  Did not discover a reader. Exiting\n");
    }

    /* Deleting the participant will delete all its children recursively as well. */
    ret = dds_delete (participant);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    return 0;
}
