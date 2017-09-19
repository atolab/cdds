#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "os/os.h"

Test(vddsc_err, unique_file_id)
{
  dds_entity_t participant, reader, writer;

  participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0);

  /* Disable SAL warning on intentional misuse of the API */
  OS_WARNING_MSVC_OFF(28020);
  reader = dds_create_reader(0, 0, NULL, NULL);
  cr_assert_lt(reader, 0);

  writer = dds_create_writer(0, 0, NULL, NULL);
  cr_assert_lt(writer, 0);

  OS_WARNING_MSVC_ON(28020);
  cr_log_info("file_id for dds_create_reader: %d", dds_err_file_id(reader));
  cr_log_info("file_id for dds_create_writer: %d", dds_err_file_id(writer));

  cr_assert_neq(dds_err_file_id(reader), dds_err_file_id(writer));

  dds_delete(participant);
}
