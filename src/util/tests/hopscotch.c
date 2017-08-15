#include "assert.h"
#include "os/os.h"
#include "util/ut_hopscotch.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */



/*****************************************************************************************/

#define NR_TEST_ELEMENTS ((uint64_t)100)

static struct ut_chh *g_hash = NULL;

typedef struct hopscotch_node_t {
    uint64_t id;
    struct hopscotch_node_t* next;
} hopscotch_node_t;


static os_atomic_voidp_t g_first;

static void
sleepMsec(int32_t msec)
{
    os_time delay;

    assert(msec > 0);
    assert(msec < 1000);

    delay.tv_sec = 0;
    delay.tv_nsec = msec*1000*1000;

    os_nanoSleep(delay);
}


static uint32_t
hopscotch_node_hash(
        const void *c)
{
    const hopscotch_node_t *node = (hopscotch_node_t*)c;
    assert(node);
    /* Force hash collisions. */
    //return (uint32_t)(node->id % (NR_TEST_ELEMENTS / 3));
    return 0;
}

static int
hopscotch_node_eq (const void *a, const void *b)
{
    const hopscotch_node_t *nodeA = (hopscotch_node_t*)a;
    const hopscotch_node_t *nodeB = (hopscotch_node_t*)b;
    assert(nodeA);
    assert(nodeB);
    return (nodeA->id == nodeB->id);
}

static void hopscotch_node_buckets (void *bs)
{
    /* I don't know what this is for and what to do with it... */
}


static uint32_t
hopscotch_thread(void *a)
{
    int x;
    hopscotch_node_t *node;

    for (uint64_t id = 1; id <= NR_TEST_ELEMENTS; id++) {
        sleepMsec(2);

        /* Create new node. */
        node = os_malloc(sizeof(hopscotch_node_hash));
        node->id = id;
        node->next = NULL;

        /* Add new node to hopscotch. */
        printf("Adding %ld vs %ld (%u)\n", id, NR_TEST_ELEMENTS, (uint32_t)(node->id % (NR_TEST_ELEMENTS / 3)));
        x = ut_chhAdd(g_hash, node);
        //printf("Added  %ld vs %ld (%d)\n", id, NR_TEST_ELEMENTS, x);
        cr_assert_gt(x, 0);

        /* Add new node to start of test list. */
        node->next = os_atomic_ldvoidp(&g_first);
        os_atomic_stvoidp(&g_first, node);

        //printf("Add %ld vs %ld\n", id, NR_TEST_ELEMENTS);
    }

    return 0;
}

Test(util_hopscotch, stress)
{
    const os_time zero  = {  0, 0 };
    os_threadId   thread_id;
    os_threadAttr thread_attr;
    hopscotch_node_t *found;
    hopscotch_node_t *node;
    os_result     osr;

    os_atomic_stvoidp(&g_first, NULL);

    g_hash = ut_chhNew(1, hopscotch_node_hash, hopscotch_node_eq, hopscotch_node_buckets);
    cr_assert_not_null(g_hash);

    os_threadAttrInit(&thread_attr);
    osr = os_threadCreate(&thread_id, "hopscotch_thread", &thread_attr, hopscotch_thread, NULL);

    /* Wait for the first added node. */
    while (os_atomic_ldvoidp(&g_first) == NULL) {
        sleepMsec(5);
    }

    /* Keep looking up nodes. */
    node = os_atomic_ldvoidp(&g_first);
    while (node->id < NR_TEST_ELEMENTS) {
        //printf("Lookup %ld\n", node->id);
        found = ut_chhLookup(g_hash, node);
        cr_assert_not_null(found);
        cr_assert_eq(found->id, node->id);
        sleepMsec(1);
        node = node->next;
        if (node == NULL) {
            /* Wrap. */
            node = os_atomic_ldvoidp(&g_first);
        }
    }

    os_threadWaitExit(thread_id, NULL);

    ut_chhFree(g_hash);
}
