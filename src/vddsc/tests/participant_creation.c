#include "dds.h"
#include "os/os.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(vddsc_participant, create_and_delete) {

  dds_entity_t participant, participant2, participant3;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant, 0, "dds_participant_create");

  participant2 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant2, 0, "dds_participant_create");

  dds_delete (participant);
  dds_delete (participant2);

  participant3 = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  cr_assert_gt(participant3, 0, "dds_participant_create");

  dds_delete (participant3);

}

static uint32_t
tmain(void *arg)
{
    dds_entity_t participant;

    /* Creation of the second participant will not result in a call to
       dds_init. Previously this meant that no thread state would be kept for
       the application thread, causing the following call to crash the
       application. Nowadays, application threads will automatically be
       registered if they are unknown. This test simply ensures that the
       mechanism will keep to function properly. */
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0);
    dds_delete(participant);
    return 0;
}

Test(vddsc_participant, create_and_delete_from_application_thread)
{
    dds_entity_t participant;
    os_threadId tid;
    os_threadAttr tattr;
    os_result res;

    /* Creation of the first participant will initialize DDS. Thread state is
       automatically kept for the thread that initializes it. */
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0);

    os_threadAttrInit(&tattr);
    res = os_threadCreate(&tid, "application", &tattr, &tmain, NULL);
    cr_assert_eq(res, os_resultSuccess);
    res = os_threadWaitExit(tid, NULL);
    cr_assert_eq(res, os_resultSuccess);
    dds_delete(participant);
}
