#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>
#include <string.h>

/*
  The helloinstanceworld example is an extension of the helloquickworld
  example. If you haven't looked at that yet, please do.

  The helloinstanceworld explores multiple samples and instances.
*/

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 10

void print_msg(HelloWorldData_Msg *msg, dds_sample_info_t *info);

int main (int argc, char ** argv)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;
    dds_entity_t waitset;
    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];
    dds_sample_info_t *info;
    dds_return_t ret;
    dds_qos_t *qos;
    bool terminate = false;

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloInstanceWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Reader. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    reader = dds_create_reader (participant, topic, qos, NULL);
    DDS_ERR_CHECK (reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete(qos);

    /* Prepare array of samples. */
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        samples[i] = HelloWorldData_Msg__alloc ();
    }

    /* Prepare to wait for data. */
    ret = dds_set_enabled_status(reader, DDS_DATA_AVAILABLE_STATUS);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    waitset = dds_create_waitset(participant);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    ret = dds_waitset_attach(waitset, reader, (dds_attach_t)NULL);
    DDS_ERR_CHECK (waitset, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    while (!terminate)
    {
        printf ("\n=== [Subscriber] Waiting for message(s) ...\n");
        ret = dds_waitset_wait(waitset, NULL, 0, DDS_SECS(30));
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        if (ret > 0)
        {
            /* Do the actual read.
             * The return value contains the number of read samples. */
            ret = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
            DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

            for (int i = 0; i < ret; i++)
            {
                msg  = (HelloWorldData_Msg*) samples[i];
                info = &(infos[i]);

                /* Print Message. */
                printf ("=== [Subscriber] Received : ");
                print_msg(msg, info);

                if (info->instance_state != DDS_IST_ALIVE)
                {
                    /* Not alive indicates that the publisher quit. */
                    printf ("=== [Subscriber] Received : Not alive message -> terminate\n");
                    terminate = true;
                }
            }
        }
        else
        {
            printf ("=== [Subscriber] Wait timeout.\n");
            terminate = true;
        }
    }

    /* Clear array of samples. */
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        HelloWorldData_Msg_free (samples[i], DDS_FREE_ALL);
    }

    /* Deleting the participant will delete all its children recursively as well. */
    ret = dds_delete (participant);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    return 0;
}

void print_msg(HelloWorldData_Msg *msg, dds_sample_info_t *info)
{
    char *sst = "sample read";
    char *vst = "instance new";
    char *ist = "instance alive";
    char *str = "<invalid data>";

    if (info->sample_state == DDS_SST_NOT_READ)
    {
        sst = "sample not read";
    }

    if (info->view_state == DDS_VST_OLD)
    {
        vst = "instance old";
    }

    if (info->instance_state == DDS_IST_NOT_ALIVE_DISPOSED)
    {
        ist = "instance disposed";
    }
    if (info->instance_state == DDS_IST_NOT_ALIVE_NO_WRITERS)
    {
        ist = "instance no writers";
    }

    if (info->valid_data)
    {
        str = msg->message;
    }

    printf ("Message (%d, %s) [ %15s, %s, %18s ]\n",
             msg->userID, str, sst, vst, ist);
}
