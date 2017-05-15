#include "dds.h"

int main (int argc, char *argv[])
{
  dds_entity_t participant;
  int status;

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  //DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  dds_delete (participant);

  dds_fini ();

  return 0;
}
