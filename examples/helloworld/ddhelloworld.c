#include "dds.h"

int main (int argc, char *argv[])
{
  dds_entity_t participant;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  dds_delete (participant);

  return 0;
}
