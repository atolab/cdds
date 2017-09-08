#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>

/*
  The helloinstanceworld example is an extension of the helloquickworld
  example. If you haven't looked at that yet, please do.

  The helloinstanceworld explores multiple samples and instances.
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
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloInstanceWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Writer. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    writer = dds_create_writer (participant, topic, qos, NULL);
    DDS_ERR_CHECK (writer, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete(qos);

    /* Prepare to wait for subscriber. */
    ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    waitset = dds_create_waitset(participant);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    ret = dds_waitset_attach(waitset, writer, (dds_attach_t)NULL);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Wait for subscriber. */
    printf ("\n=== [Publisher]  Waiting for a reader ...\n");
    ret = dds_waitset_wait(waitset, NULL, 0, DDS_SECS(30));
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    if (ret > 0)
    {
        /* Write first message of first user. */
        msg.userID = 1;
        msg.message = "{id1|msg1}";
        printf ("=== [Publisher]  Writing : Message (%d, %s)\n", msg.userID, msg.message);
        ret = dds_write (writer, &msg);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        /* Normally, you shouldn't sleep in an application, but
         * here we want to make sure that the subscriber handled
         * the message and thus it's state was updated. */
        dds_sleepfor (DDS_SECS (1));

        /* Write first message of second user. */
        msg.userID = 2;
        msg.message = "{id2|msg1}";
        printf ("=== [Publisher]  Writing : Message (%d, %s)\n", msg.userID, msg.message);
        ret = dds_write (writer, &msg);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        dds_sleepfor (DDS_SECS (1));

        /* Write second message of first user. */
        msg.userID = 1;
        msg.message = "{id1|msg2}";
        printf ("=== [Publisher]  Writing : Message (%d, %s)\n", msg.userID, msg.message);
        ret = dds_write (writer, &msg);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        dds_sleepfor (DDS_SECS (1));

        /* Dispose instance of second user.
         * This'll indicate to the subscriber that the publisher has finished. */
        msg.userID = 2;
        msg.message = "don't care";
        printf ("=== [Publisher]  Dispose : Message (%d, %s)\n", msg.userID, msg.message);
        ret = dds_dispose (writer, &msg);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        dds_sleepfor (DDS_SECS (1));
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
