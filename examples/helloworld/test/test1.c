#include "dds.h"
#include <stdio.h>

int main (int argc, char *argv[])
{
  dds_entity_t participant;
  int status;

  printf("This is an example test whith the purposes to demonstrate how ctest can be used.");
  printf("This test has been created on conto of CHAM-69\n");
  printf("This tests only tests the successful creation of a participant\n");

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  dds_entity_delete (participant);

  dds_fini ();

  /* ctest requires the test executable to return 0 when succuss, non-null when fail */
  return status;
}
