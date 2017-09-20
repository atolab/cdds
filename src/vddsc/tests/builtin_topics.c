#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#include <stdio.h>

/* This is just a preliminary test to verify builtin-topic types are available,
 * tests will be extended to actually read topics once that part is implemented
 */

static dds_entity_t builtin_topic_handles[10];

#define STR(x) #x

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
#undef TEST_ALLOC
}

Test(vddsc_builtin_topics, create_reader)
{
    dds_entity_t participant;
    dds_entity_t t1;

    /* Create a participant */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(participant, 0, "dds_participant_create");

#define TEST_FIND(p, t) do { \
        t1 = dds_find_topic(p, t); \
        cr_expect_lt(t1, 0, "dds_find_topic(\"" t "\") returned a valid handle"); \
    } while(0);

    /* A builtin-topic proxy is created 'on demand' and should not exist before a reader is created for it */
    TEST_FIND(participant, "DCPSParticipant");
    TEST_FIND(participant, "CMParticipant");
    TEST_FIND(participant, "DCPSType");
    TEST_FIND(participant, "DCPSTopic");
    TEST_FIND(participant, "DCPSPublication");
    TEST_FIND(participant, "CMPublisher");
    TEST_FIND(participant, "DCPSSubscription");
    TEST_FIND(participant, "CMSubscriber");
    TEST_FIND(participant, "CMDataWriter");
    TEST_FIND(participant, "CMDataReader");
#undef TEST_FIND

    /* A reader is created by providing a special builtin-topic handle */
    {
        dds_entity_t readers[10];
        dds_entity_t builtin_subscriber, s;

        builtin_topic_handles[0] = DDS_BUILTIN_TOPIC_DCPSPARTICIPANT;
        builtin_topic_handles[1] = DDS_BUILTIN_TOPIC_CMPARTICIPANT;
        builtin_topic_handles[2] = DDS_BUILTIN_TOPIC_DCPSTYPE;
        builtin_topic_handles[3] = DDS_BUILTIN_TOPIC_DCPSTOPIC;
        builtin_topic_handles[4] = DDS_BUILTIN_TOPIC_DCPSPUBLICATION;
        builtin_topic_handles[5] = DDS_BUILTIN_TOPIC_CMPUBLISHER;
        builtin_topic_handles[6] = DDS_BUILTIN_TOPIC_DCPSSUBSCRIPTION;
        builtin_topic_handles[7] = DDS_BUILTIN_TOPIC_CMSUBSCRIBER;
        builtin_topic_handles[8] = DDS_BUILTIN_TOPIC_CMDATAWRITER;
        builtin_topic_handles[9] = DDS_BUILTIN_TOPIC_CMDATAREADER;


        for (int i = 0; i < 10; i++) {
            readers[i] = dds_create_reader(participant, builtin_topic_handles[i], NULL, NULL);
            cr_expect_gt(readers[i], 0, "Failed to created reader for builtin topic handle %d", builtin_topic_handles[i]);

            if (i == 0) {
                /* Check the parent of reader is a subscriber */
                builtin_subscriber = dds_get_parent(readers[i]);
                cr_assert_gt(builtin_subscriber, 0, "Failed to get parent of first builtin-reader (%s)", dds_err_str(builtin_subscriber));
                cr_assert_eq(builtin_subscriber & DDS_ENTITY_KIND_MASK, DDS_KIND_SUBSCRIBER, "Parent is not a subscriber");
            } else {
                /* Check the parent of reader equals parent of first reader */
                s = dds_get_parent(readers[i]);
                cr_assert_gt(s, 0, "Failed to get parent of builtin-reader (%s)", dds_err_str(s));
                cr_assert_eq(s, builtin_subscriber, "Parent subscriber of reader(%d) doesn't equal builtin-subscriber", i);
                //dds_delete(s);
            }
        }

        dds_delete(builtin_subscriber);
    }

#define TEST_FIND(p, t) do { \
        t1 = dds_find_topic(p, t); \
        cr_expect_gt(t1, 0, "dds_find_topic(\"" t "\") returned an invalid handle (%s)", dds_err_str(t1)); \
    } while(0);

    /* Builtin-topics proxies should now be created */
//    TEST_FIND(participant, "DCPSParticipant");
//    TEST_FIND(participant, "CMParticipant");
//    TEST_FIND(participant, "DCPSType");
//    TEST_FIND(participant, "DCPSTopic");
//    TEST_FIND(participant, "DCPSPublication");
//    TEST_FIND(participant, "CMPublisher");
//    TEST_FIND(participant, "DCPSSubscription");
//    TEST_FIND(participant, "CMSubscriber");
//    TEST_FIND(participant, "CMDataWriter");
//    TEST_FIND(participant, "CMDataReader");
#undef TEST_FIND

    dds_delete(participant);

}

