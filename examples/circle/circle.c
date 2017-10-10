#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dds.h"
#include "ShapeType.h"

int main ()
{
  dds_entity_t participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  dds_entity_t subscriber = dds_create_subscriber(participant, NULL, NULL);
  dds_qos_t *qos;
  qos = dds_qos_create();
  dds_qset_durability(qos, DDS_DURABILITY_PERSISTENT);
  dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 100);
  dds_qset_resource_limits(qos, 8192, 4196, 8192);
  dds_qset_durability_service(qos, DDS_SECS(3600), DDS_HISTORY_KEEP_LAST, 1, DDS_LENGTH_UNLIMITED, DDS_LENGTH_UNLIMITED, DDS_LENGTH_UNLIMITED);
  dds_entity_t circleTopic = dds_create_topic(participant, &ShapeType_desc, "Circle", qos, NULL);
  dds_qos_delete(qos);
  qos = dds_qos_create();
  dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);
  dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT);
  dds_entity_t reader;
  reader = dds_create_reader(subscriber, circleTopic, qos, NULL);
  dds_delete(reader);
  dds_sleepfor(DDS_SECS(10));
  reader = dds_create_reader(subscriber, circleTopic, qos, NULL);
  dds_qos_delete(qos);

  while (1)
  {
    ShapeType data[100];
    void *samples[sizeof(data)/sizeof(*data)];
    dds_sample_info_t info[sizeof(data)/sizeof(*data)];
    memset(data, 0, sizeof(data));
    for (size_t i = 0; i < sizeof(data)/sizeof(*data); i++)
      samples[i] = &data[i];
    const int n = dds_take(reader, samples, info, sizeof(data)/sizeof(*data), sizeof(data)/sizeof(*data));
    for (int i = 0; i < n; i++)
    {
      if (!info[i].valid_data)
        continue;
      ShapeType const * const s = &data[i];
      printf ("%s {%d,%d} %d\n", s->color, s->x, s->y, s->shapesize);
    }
    dds_sleepfor (DDS_MSECS (10));
  }
  return 0;
}
