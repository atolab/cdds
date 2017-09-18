#include "dds.h"
#include "HelloWorldData.h"
#include "dds_builtinTopics.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1

#define CM_SAMPLES 10

int main (int argc, char ** argv)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;
    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];
    dds_return_t ret;
    dds_qos_t *qos;

    dds_entity_t dcps_topic;
    dds_entity_t dcps_reader;
    void *dcps_samples[CM_SAMPLES];
    dds_sample_info_t dcps_infos[CM_SAMPLES];
    for (int i = 0; i < CM_SAMPLES; i++)
    {
        dcps_samples[i] = DDS_ParticipantBuiltinTopicData__alloc ();
    }

    dds_entity_t cm_topic;
    dds_entity_t cm_reader;
    void *cm_samples[CM_SAMPLES];
    dds_sample_info_t cm_infos[CM_SAMPLES];
    for (int i = 0; i < CM_SAMPLES; i++)
    {
        cm_samples[i] = DDS_CMParticipantBuiltinTopicData__alloc ();
    }

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    //cm_topic = dds_create_topic (participant, &DDS_CMParticipantBuiltinTopicData_desc, "CMParticipant", NULL, NULL);
    dcps_topic = dds_find_topic(participant, "DCPSParticipant");
    DDS_ERR_CHECK (dcps_topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    {
        char name[100];
        dds_get_type_name(dcps_topic, name, 100);
        printf("Typename: %s\n", name);
    }

    cm_topic = dds_find_topic(participant, "CMParticipant");
    DDS_ERR_CHECK (cm_topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    {
        char name[100];
        dds_get_type_name(cm_topic, name, 100);
        printf("Typename: %s\n", name);
    }

    /* Create a Reader. */
    dcps_reader = dds_create_reader (participant, dcps_topic, NULL, NULL);
    DDS_ERR_CHECK (dcps_reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    cm_reader = dds_create_reader (participant, cm_topic, NULL, NULL);
    DDS_ERR_CHECK (cm_reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a Topic. */
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc,
                              "HelloWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Reader. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    reader = dds_create_reader (participant, topic, qos, NULL);
    DDS_ERR_CHECK (reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    printf ("\n=== [Subscriber] Waiting for a sample ...\n");

    /* Initialize sample buffer, by pointing the void pointer within
     * the buffer array to a valid sample memory location. */
    samples[0] = HelloWorldData_Msg__alloc ();

    /* Poll until data has been read. */
    while (true)
    {
        /* Do the actual read.
         * The return value contains the number of read samples. */
        ret = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        /* Check if we read some data and it is valid. */
        if ((ret > 0) && (infos[0].valid_data))
        {
            /* Print Message. */
            msg = (HelloWorldData_Msg*) samples[0];
            printf ("=== [Subscriber] Received : ");
            printf ("Message (%d, %s)\n", msg->userID, msg->message);
            break;
        }
        else
        {
            /* Polling sleep. */
            dds_sleepfor (DDS_MSECS (20));
        }
    }
    dds_qos_delete(qos);

    ret = dds_read (cm_reader, cm_samples, cm_infos, CM_SAMPLES, CM_SAMPLES);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    printf ("ret : %d\n", ret);
    for (int i = 0; i < ret; i++) {
        if (cm_infos[i].valid_data) {
            DDS_CMParticipantBuiltinTopicData *cm = (DDS_CMParticipantBuiltinTopicData*)cm_samples[i];
            printf("cm  : %x.%x.%x\n", cm->key[0], cm->key[1], cm->key[2]);
            printf("prod: %s\n", cm->product.value);
        }
    }

    ret = dds_read (dcps_reader, dcps_samples, dcps_infos, CM_SAMPLES, CM_SAMPLES);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    printf ("ret : %d\n", ret);
    for (int i = 0; i < ret; i++) {
        if (dcps_infos[i].valid_data) {
            DDS_ParticipantBuiltinTopicData *dcps = (DDS_ParticipantBuiltinTopicData*)dcps_samples[i];
            printf("dcps: %x.%x.%x\n", dcps->key[0], dcps->key[1], dcps->key[2]);
            printf("user: %s\n", dcps->user_data.value._buffer);
        }
    }


    dds_sleepfor (DDS_SECS (5));

    /* Free the data location. */
    HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);

    for (int i = 0; i < CM_SAMPLES; i++)
    {
        DDS_CMParticipantBuiltinTopicData_free (dcps_samples[i], DDS_FREE_ALL);
    }

    /* Clear array of samples. */
    for (int i = 0; i < CM_SAMPLES; i++)
    {
        DDS_CMParticipantBuiltinTopicData_free (cm_samples[i], DDS_FREE_ALL);
    }

    /* Deleting the participant will delete all its children recursively as well. */
    ret = dds_delete (participant);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    return EXIT_SUCCESS;
}
