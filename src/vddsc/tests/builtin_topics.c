#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#include <stdio.h>

/* This is just a preliminary test to verify builtin-topic types are available,
 * tests will be extended to actually read topics once that part is implemented
 */

Test(vddsc_builtin_topics, types_allocation)
{
#define TEST_ALLOC(type) do { \
        DDS_##type##BuiltinTopicData *data = DDS_##type##BuiltinTopicData__alloc(); \
        cr_expect_not_null(data, "Failed to allocate DDS_" #type "BuiltinTopicData"); \
        dds_free(data); \
    } while(0)

    TEST_ALLOC(Participant);
    TEST_ALLOC(CMParticipant);
    TEST_ALLOC(Type);
    TEST_ALLOC(Topic);
    TEST_ALLOC(Publication);
    TEST_ALLOC(CMPublisher);
    TEST_ALLOC(Subscription);
    TEST_ALLOC(CMSubscriber);
    TEST_ALLOC(CMDataWriter);
    TEST_ALLOC(CMDataReader);
}

Test(vddsc_builtin_topics, create_reader)
{
    dds_entity_t participant;
    dds_entity_t reader;
    dds_entity_t t1, t2;

    /* Create a participant */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0, "dds_participant_create");

    /* A builtin-topic is created 'on demand' and doesn't exist before a reader is created for it */
    t1 = dds_find_topic(participant, "DCPSParticipant");
    cr_assert_lt(t1, 0, "dds_find_topic(\"DCPSParticipant\")");

    /* The builtin-topic handle can be used to refer to builtin-topics, and should result in a builtin reader */
    reader = dds_create_reader(participant, DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, NULL, NULL);
    printf("READER %d\n", reader);
    reader = dds_create_reader(participant, DDS_BUILTIN_TOPIC_DCPSPARTICIPANT, NULL, NULL);
    printf("READER %d\n", reader);
//
    t1 = dds_find_topic(participant, "DCPSParticipant");
    t2 = dds_find_topic(participant, "DCPSParticipant");
    printf("TOPIC t1(%d) t2(%d)\n", t1, t2);

}

