#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>
#include <string.h>

/*
  The helloquickworld example is an extension of the helloworld
  example. If you haven't looked at that yet, please do.

  The helloworld contains a few sleeps and polling. These are
  removed and improved upon in this helloquickworld example.
*/

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1

int main (int argc, char ** argv)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;
    dds_entity_t waitset;
    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];
    dds_return_t ret;
    dds_qos_t *qos;

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloQuickWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Reader. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    reader = dds_create_reader (participant, topic, qos, NULL);
    DDS_ERR_CHECK (reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete(qos);

    /*
     * The writer waited for a reader to be discovered before starting
     * to write. This isn't really needed for the reader. The reader
     * can just wait for data to arrive.
     * For that to happen, we need to:
     *   - Indicate our interest for that on the reader.
     *   - Create a waitset.
     *   - Attach the reader to the waitset.
     *   - The waitset.wait will unblock when data is available.
     */
    ret = dds_set_enabled_status(reader, DDS_DATA_AVAILABLE_STATUS);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    waitset = dds_create_waitset(participant);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    ret = dds_waitset_attach(waitset, reader, (dds_attach_t)NULL);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* The dds_waitset_wait returns the number of triggered entities,
     * which can only be '1' because only 1 was attached. Returning
     * 0 means that within the timeout, no entities were triggered.
     * Returning a negative value indicates an error. */
    printf ("\n=== [Subscriber] Waiting for a message ...\n");
    ret = dds_waitset_wait(waitset, NULL, 0, DDS_SECS(30));
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    if (ret > 0)
    {
        /* Initialize sample buffer, by pointing the void pointer within
         * the buffer array to a valid sample memory location. */
        samples[0] = HelloWorldData_Msg__alloc ();

        /* Do the actual read.
         * The return value contains the number of read samples. */
        ret = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        /* Check if we really read some data and it is valid. */
        if (ret > 0)
        {
            msg = (HelloWorldData_Msg*) samples[0];
            if (infos[0].valid_data)
            {
                /* Print Message. */
                printf ("=== [Subscriber] Received : ");
                printf ("Message (%d, %s)\n", msg->userID, msg->message);
            }
            else
            {
                /* Only print the key part of the Message, because the rest
                 * of the data is invalid. */
                printf ("=== [Subscriber] Received : invalid data (instance was disposed or unregistered)");
                printf ("Message (%d, *)\n", msg->userID);
            }
        }
        else
        {
            printf ("=== [Subscriber] No data was read, which is unexpected.\n");
        }

        /* Free the data location. */
        HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);
    }
    else
    {
        printf ("=== [Subscriber] No data was read, within the timeout.\n");
    }

    /* Deleting the participant will delete all its children recursively as well. */
    ret = dds_delete (participant);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    return 0;
}
