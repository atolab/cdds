#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>
#include <string.h>

#define MAX_SAMPLES 1

int main (int argc, char ** argv)
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t subscriber;
  dds_entity_t reader;
  HelloWorldData_Msg * msg;
  HelloWorldData_Msg sample;
  void * samples[MAX_SAMPLES];
  dds_sample_info_t info[MAX_SAMPLES];
  int ret = 0;
  dds_qos_t * rqos;
  const char * partitions[1];

  /* Initialize sample */

  memset (&sample, 0, sizeof (sample));

  /* Create a Participant */

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (participant, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create a Topic */

  topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
  DDS_ERR_CHECK (topic, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Create a Subscriber */

  subscriber = dds_create_subscriber (participant, NULL, NULL);
  DDS_ERR_CHECK (subscriber, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  /* Set QoS for Reader */

  rqos = dds_qos_create ();
  dds_qset_durability (rqos, DDS_DURABILITY_TRANSIENT);
  dds_qset_reliability (rqos, DDS_RELIABILITY_RELIABLE, DDS_SECS (1));
  partitions[0] = "HelloWorld example";
  dds_qset_partition (rqos, 1, partitions);

  /* Create a Reader */

  reader = dds_create_reader (subscriber, topic, rqos, NULL);
  DDS_ERR_CHECK (reader, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_qos_delete (rqos);

  printf ("\n=== [Reader] waiting for a message ...\n");

  samples[0] = &sample;
  while (true)
  {
    ret = dds_read (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
    DDS_ERR_CHECK (ret, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Print Message */

    if ((ret > 0) && info[0].valid_data)
    {
      msg = (HelloWorldData_Msg*) samples[0];
      printf ("=== [Subscriber] Received : ");
      printf ("Message (%d, %s)\n", msg->userID, msg->message);
      break;
    }
    else
    {
      dds_sleepfor (DDS_MSECS (20));
    }
  }

  /* Clean up */

  HelloWorldData_Msg_free (msg, DDS_FREE_CONTENTS);
  dds_delete (participant);

  return 0;
}
