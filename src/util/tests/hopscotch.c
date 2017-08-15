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


static uint32_t
hopscotch_node_hash_zero(
        const void *c)
{
    /* Force hash collisions. */
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

Test(util_hopscotch, hash_collision)
{
    hopscotch_node_t *node;
    int x;

    g_hash = ut_chhNew(1, hopscotch_node_hash_zero, hopscotch_node_eq, hopscotch_node_buckets);
    cr_assert_not_null(g_hash);

    for (uint64_t id = 1; id <= NR_TEST_ELEMENTS; id++) {
        node = os_malloc(sizeof(*node));
        node->id = id;
        node->next = NULL;

        /* Add new node to hopscotch. */
        cr_log_info("Adding %ld\n", id);
        x = ut_chhAdd(g_hash, node);
        cr_assert_gt(x, 0);
    }

    ut_chhFree(g_hash);
}
