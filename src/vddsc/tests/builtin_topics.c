#include "dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

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
    /* TEST_ALLOC(CMTopic); */
    TEST_ALLOC(Publication);
    TEST_ALLOC(CMPublisher);
    TEST_ALLOC(Subscription);
    TEST_ALLOC(CMSubscriber);
    TEST_ALLOC(CMDataWriter);
    TEST_ALLOC(CMDataReader);
}

#if 0
Test(vddsc_builtin_topics, create_reader)
{

}
#endif

