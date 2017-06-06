#include "os/os.h"
#include "util/ut_handleserver.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */

typedef enum claiming_state {
    STARTING,
    CLAIMING,
    STOPPED
} claiming_state;


typedef struct claiming_arg {
    claiming_state state;
    ut_handleserver_t srv;
    ut_handle_t  hdl;
    int32_t      kind;
    void        *arg;
} claiming_arg;

void*
claiming_thread(void *a)
{
    claiming_arg *ca = (claiming_arg*)a;
    ut_handleclaim_t claim;
    ut_handle_t ret;
    void *argx;

    ca->state = CLAIMING;

    ret = ut_handle_claim(ca->srv, ca->hdl, ca->kind, &argx, &claim);
    cr_assert_eq(ret, ca->hdl, "ut_handle_claim(thr) ret");
    cr_assert_eq(argx, ca->arg, "ut_handle_claim(thr) arg");
    cr_assert_neq(claim, NULL, "ut_handle_claim(thr) claim");
    ut_handle_release(ca->srv, claim);

    ca->state = STOPPED;

    return NULL;
}

os_result
claiming_thread_reached_state(claiming_state *actual, claiming_state expected, int32_t msec)
{
    bool stopped = false;
    os_time msec10 = { 0, 10000000 };
    while ((msec > 0) && (*actual != expected)) {
        os_nanoSleep(msec10);
        msec -= 10;
    }
    return (*actual == expected) ? os_resultSuccess : os_resultTimeout;
}

Test(util, handleserver)
{
    ut_handleserver_t srv;
    ut_handle_t       hdl1;
    ut_handle_t       hdl2;
    ut_handle_t       hdl3;
    ut_handle_t       ret;
    int arg1 = 1;
    int arg2 = 2;
    void *argx;
    int32_t kind1 = 0x10000000;
    int32_t kind2 = 0x20000000;
    int32_t kind3 = 0x40000000;
    os_mutex mtx1;
    os_mutex mtx3;
    ut_handleclaim_t claim1;
    ut_handleclaim_t claim2;
    ut_handleclaim_t claim3;
    os_threadId   thread_id;
    claiming_arg  thread_arg;
    os_threadAttr thread_attr;
    os_result     osr;

    os_osInit();
    os_mutexInit(&mtx1);
    os_mutexInit(&mtx3);

    os_threadAttrInit(&thread_attr);

    /* Create the server. */
    srv = ut_handleserver_new();
    cr_assert_neq(srv, NULL, "ut_handleserver_new");

    /* Create various handles with different arguments. */
    hdl1 = ut_handle_create(srv, kind1, (void*)&arg1, &mtx1);
    cr_assert(hdl1 > 0, "ut_handle_create(1)");
    ret = ut_handle_is_valid(srv, hdl1);
    cr_assert_eq(ret, hdl1, "ut_handle_is_valid(1)");
    hdl2 = ut_handle_create(srv, kind2, (void*)&arg2, NULL);
    cr_assert(hdl2 > 0, "ut_handle_create(2)");
    ret = ut_handle_is_valid(srv, hdl2);
    cr_assert_eq(ret, hdl2, "ut_handle_is_valid(2)");
    hdl3 = ut_handle_create(srv, kind3, NULL, &mtx3);
    cr_assert(hdl3 > 0, "ut_handle_create(3)");
    ret = ut_handle_is_valid(srv, hdl3);
    cr_assert_eq(ret, hdl3, "ut_handle_is_valid(3)");

    /* Try getting an arg with an handle with invalid kind. */
    ret = ut_handle_get_arg(srv, hdl1, kind2, &argx);
    cr_assert_eq(ret, UT_HANDLE_ERROR_KIND_NOT_EQUAL, "ut_handle_get_arg(invalid kind)");

    /* Try claiming an handle with invalid kind. */
    ret = ut_handle_claim(srv, hdl1, kind3, NULL, &claim1);
    cr_assert_eq(ret, UT_HANDLE_ERROR_KIND_NOT_EQUAL, "ut_handle_claim(invalid kind)");

    /* Try claiming an unlockable handle. */
    ret = ut_handle_claim(srv, hdl2, kind2, NULL, &claim2);
    cr_assert_eq(ret, UT_HANDLE_ERROR_NOT_LOCKABLE, "ut_handle_claim(unlockable)");

    /* Get arg from handle. */
    ret = ut_handle_get_arg(srv, hdl2, kind2, &argx);
    cr_assert_eq(ret, hdl2, "ut_handle_get_arg(2) ret");
    cr_assert_eq(argx, &arg2, "ut_handle_get_arg(2) arg");

    /* Claim handles. */
    ret = ut_handle_claim(srv, hdl1, kind1, &argx, &claim1);
    cr_assert_eq(ret, hdl1, "ut_handle_claim(1) ret");
    cr_assert_eq(argx, &arg1, "ut_handle_claim(1) arg");
    cr_assert_neq(claim1, NULL, "ut_handle_claim(1) claim");
    ret = ut_handle_claim(srv, hdl3, kind3, &argx, &claim3);
    cr_assert_eq(ret, hdl3, "ut_handle_claim(3) ret");
    cr_assert_eq(argx, NULL, "ut_handle_claim(3) arg");
    cr_assert_neq(claim3, NULL, "ut_handle_claim(3) claim");

    /* Release and reclaim. */
    ut_handle_release(srv, claim1);
    ret = ut_handle_claim(srv, hdl1, kind1, &argx, &claim1);
    cr_assert_eq(ret, hdl1, "ut_handle_claim(1) ret");
    cr_assert_eq(argx, &arg1, "ut_handle_claim(1) arg");
    cr_assert_neq(claim1, NULL, "ut_handle_claim(1) claim");

    /* Try re-claiming in other thread, which should block. */
    thread_arg.arg = (void*)&arg1;
    thread_arg.hdl = hdl1;
    thread_arg.kind = kind1;
    thread_arg.srv = srv;
    thread_arg.state = STARTING;
    os_threadAttrInit(&thread_attr);
    osr = os_threadCreate(&thread_id, "claiming_thread", &thread_attr, claiming_thread, (void*)&thread_arg);
    cr_assert_eq(osr, os_resultSuccess, "os_threadCreate");
    osr = claiming_thread_reached_state(&thread_arg.state, CLAIMING, 1000);
    cr_assert_eq(osr, os_resultSuccess, "claiming");
    osr = claiming_thread_reached_state(&thread_arg.state, STOPPED, 500);
    cr_assert_eq(osr, os_resultTimeout, "claiming");

    /* Releasing the hdl should unblock the thread. */
    ut_handle_release(srv, claim1);
    osr = claiming_thread_reached_state(&thread_arg.state, STOPPED, 1000);
    cr_assert_eq(osr, os_resultSuccess, "claiming");
    os_threadWaitExit(thread_id, NULL);

    /* Delete the handles. */
    ut_handle_delete(srv, hdl1);
    ret = ut_handle_is_valid(srv, hdl1);
    cr_assert_eq(ret, UT_HANDLE_ERROR_DELETED, "ut_handle_delete_by_claim(1)");
    ut_handle_delete(srv, hdl2);
    ret = ut_handle_is_valid(srv, hdl2);
    cr_assert_eq(ret, UT_HANDLE_ERROR_DELETED, "ut_handle_delete(2)");
    ut_handle_delete_by_claim(srv, claim3);
    ret = ut_handle_is_valid(srv, hdl3);
    cr_assert_eq(ret, UT_HANDLE_ERROR_DELETED, "ut_handle_delete(3)");

    ut_handleserver_free(srv);

    os_mutexDestroy(&mtx1);
    os_mutexDestroy(&mtx3);
    os_osExit();
}

