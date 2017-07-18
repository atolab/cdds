#include "os/os.h"
#include "dds.h"
#include <CUnit/Runner.h>

static uint32_t test_thread(void *a);

CUnit_Test(ts, pc)
{
  os_result     osr;
  os_threadId   thread_id;
  os_threadAttr thread_attr;
  dds_entity_t  participant;
  /* First participant creation will initialize dds. The thread that initilialized dds is automatically added to the internal thread management.
   * This means that anything done in this thread will succeed (dds wise). */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  //cr_assert_gt(participant, 0, "dds_participant_create");
  os_threadAttrInit(&thread_attr);
  osr = os_threadCreate(&thread_id, "test_thread", &thread_attr, test_thread, NULL);
  //cr_assert_eq(osr, os_resultSuccess, "os_threadCreate");
  os_threadWaitExit(thread_id, NULL);
  dds_delete (participant);
}

static uint32_t
test_thread(void *a)
{
  dds_entity_t participant;
  //dds_thread_init("test_thread");
  /* This is a second participant creation. So, no dds init, which means that this new thread is not added to the internal thread management.
   * The end result is that the following call will crash. */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  //cr_assert_gt(participant, 0, "dds_participant_create");
  dds_delete (participant);
  //dds_thread_fini();
  return 0;
}

