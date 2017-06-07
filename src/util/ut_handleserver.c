/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <string.h>
#include <assert.h>
#include "os/os.h"
#include "util/ut_handleserver.h"

/* Arbitrarily number of max handles. Should be enough for the mock. */
#define MAX_NR_OF_HANDLES (1000)

typedef struct ut_handleinfo {
    ut_handle_t hdl;
    os_mutex *mutex;
    void *arg;
} ut_handleinfo;

typedef struct ut_handleserver {
    ut_handleinfo *hdls[MAX_NR_OF_HANDLES];
    int32_t last;
    os_mutex mutex;
} ut_handleserver;


_Check_return_ static ut_handle_t check_handle(_In_ ut_handleserver *hs, _In_ ut_handle_t hdl, _In_ int32_t kind);
static void delete_handle(_In_ ut_handleserver *hs, _In_ int32_t idx);

_Check_return_ ut_handleserver_t
ut_handleserver_new()
{
    ut_handleserver *hs = (ut_handleserver*)os_malloc(sizeof(ut_handleserver));
    memset(hs, 0, sizeof(ut_handleserver));
    hs->last = 0;
    os_mutexInit(&hs->mutex);
    return hs;
}

void
ut_handleserver_free(_Inout_ _Post_invalid_ ut_handleserver_t srv)
{
    int i;
    ut_handleserver *hs = srv;

    assert(hs);

    for (i = 0; i < hs->last; i++) {
        /* Every handle should have been deleted. */
        assert(hs->hdls[i] == NULL);
        if (hs->hdls[i] != NULL) {
            os_free(hs->hdls[i]);
        }
    }
    os_mutexDestroy(&hs->mutex);
    os_free(hs);
}


_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Post_satisfies_((return & UT_HANDLE_KIND_MASK) == kind)
_Check_return_ ut_handle_t
ut_handle_create(_In_     ut_handleserver_t srv,
                 _In_     int32_t kind,
                 _In_opt_ void *arg,
                 _In_opt_ os_mutex *mtx)
{
    ut_handleserver *hs = srv;
    ut_handle_t hdl = UT_HANDLE_ERROR;

    assert(hs);
    /* A kind is obligatory. */
    assert(kind & UT_HANDLE_KIND_MASK);
    /* The kind should extent outside its boundaries. */
    assert(!(kind & ~UT_HANDLE_KIND_MASK));

    os_mutexLock(&hs->mutex);

    if (hs->last < MAX_NR_OF_HANDLES) {
        hdl  = hs->last;
        hdl |= kind;
        hs->hdls[hs->last] = (ut_handleinfo*)os_malloc(sizeof(ut_handleinfo));
        hs->hdls[hs->last]->mutex = mtx;
        hs->hdls[hs->last]->arg   = arg;
        hs->hdls[hs->last]->hdl   = hdl;
        hs->last++;
    }

    os_mutexUnlock(&hs->mutex);

    return hdl;
}

void
ut_handle_delete(_In_                ut_handleserver_t srv,
                 _In_ _Post_invalid_ ut_handle_t hdl)
{
    ut_handleserver *hs = srv;
    assert(hs);
    os_mutexLock(&hs->mutex);
    hdl = check_handle(hs, hdl, 0);
    if (hdl > 0) {
        delete_handle(hs, (hdl & UT_HANDLE_IDX_MASK));
    }
    os_mutexUnlock(&hs->mutex);
}

void
ut_handle_delete_by_claim(_In_                   ut_handleserver_t srv,
                          _Inout_ _Post_invalid_ ut_handleclaim_t claim)
{
    ut_handleserver *hs = srv;
    ut_handleinfo *info = claim;
    os_mutex *info_mutex;

    assert(hs);
    assert(info);

    os_mutexLock(&hs->mutex);
    info_mutex = info->mutex;
    delete_handle(hs, (info->hdl & UT_HANDLE_IDX_MASK));
    os_mutexUnlock(info_mutex);
    os_mutexUnlock(&hs->mutex);
}

_Check_return_ ut_handle_t
ut_handle_is_valid(_In_ ut_handleserver_t srv,
                   _In_ ut_handle_t hdl)
{
    ut_handleserver *hs = srv;
    assert(hs);
    os_mutexLock(&hs->mutex);
    hdl = check_handle(hs, hdl, 0);
    os_mutexUnlock(&hs->mutex);
    return hdl;
}

_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Check_return_ ut_handle_t
ut_handle_get_arg(_In_  ut_handleserver_t srv,
                  _In_  ut_handle_t hdl,
                  _In_  int32_t kind,
                  _Out_ void **arg)
{
    ut_handleserver *hs = srv;
    assert(hs);
    assert(arg);
    *arg = NULL;
    os_mutexLock(&hs->mutex);
    hdl = check_handle(hs, hdl, kind);
    if (hdl > 0) {
        int32_t idx = (hdl & UT_HANDLE_IDX_MASK);
        assert(idx < MAX_NR_OF_HANDLES);
        *arg = hs->hdls[idx]->arg;
    }
    os_mutexUnlock(&hs->mutex);
    return hdl;
}

_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Check_return_ ut_handle_t
ut_handle_claim(_In_      ut_handleserver_t srv,
                _In_      ut_handle_t hdl,
                _In_      int32_t kind,
                _Out_opt_ void **arg,
                _Out_     ut_handleclaim_t *claim)
{
    ut_handleserver *hs = srv;
    ut_handleinfo *info;

    assert(hs);
    assert(claim);

    *claim = NULL;
    if (arg) {
        *arg = NULL;
    }

    os_mutexLock(&hs->mutex);

    /* Check if handle is valid, expected and lockable. */
    hdl = check_handle(hs, hdl, kind);
    if (hdl > 0) {
        int32_t idx = (hdl & UT_HANDLE_IDX_MASK);
        assert(idx < MAX_NR_OF_HANDLES);
        info = hs->hdls[idx];
        if (info->mutex == NULL) {
            hdl = UT_HANDLE_ERROR_NOT_LOCKABLE;
        } else {

            /* Do while handle is not deleted. */
            do {
                os_result osr = os_mutexTryLock(info->mutex);
                if (osr == os_resultSuccess) {
                    *claim = info;
                    if (arg) {
                        *arg = info->arg;
                    }
                } else if (osr == os_resultBusy) {
                    /* TODO: Replace with condition wait when this is not a mock anymore. */
                    const os_time delay = { 0, 1000000 };
                    os_mutexUnlock(&hs->mutex);
                    os_nanoSleep(delay);
                    os_mutexLock(&hs->mutex);
                    /* Check if handle was deleted. */
                    hdl = check_handle(hs, hdl, kind);
                } else {
                    assert(false);
                }
            } while ((*claim == NULL) && (hdl > 0));
        }
    }
    os_mutexUnlock(&hs->mutex);
    return hdl;
}

void
ut_handle_release(_In_                   ut_handleserver_t srv,
                  _Inout_ _Post_invalid_ ut_handleclaim_t claim)
{
    ut_handleserver *hs = srv;
    ut_handleinfo *info = claim;

    assert(hs);
    assert(info);

    OS_UNUSED_ARG(hs);

    os_mutexUnlock(info->mutex);
}

_Check_return_ static ut_handle_t
check_handle(_In_ ut_handleserver *hs,
             _In_ ut_handle_t hdl,
             _In_ int32_t kind)
{
    if (hdl > 0) {
        int32_t idx = (hdl & UT_HANDLE_IDX_MASK);
        if (idx < hs->last) {
            assert(idx < MAX_NR_OF_HANDLES);
            ut_handleinfo *info = hs->hdls[idx];
            if (info != NULL) {
                if ((info->hdl & UT_HANDLE_KIND_MASK) == (hdl & UT_HANDLE_KIND_MASK)) {
                    if ((kind != 0) && (kind != (hdl & UT_HANDLE_KIND_MASK))) {
                        /* It's a valid handle, but the caller expected a different kind. */
                        hdl = UT_HANDLE_ERROR_KIND_NOT_EQUAL;
                    }
                } else {
                    hdl = UT_HANDLE_ERROR_KIND_NOT_EQUAL;
                }
            } else {
                hdl = UT_HANDLE_ERROR_DELETED;
            }
        } else {
            hdl = UT_HANDLE_ERROR_INVALID;
        }
    } else {
        hdl = UT_HANDLE_ERROR_INVALID;
    }
    return hdl;
}

static void
delete_handle(_In_ ut_handleserver *hs,
              _In_ int32_t idx)
{
    assert(hs);
    assert(idx < MAX_NR_OF_HANDLES);
    os_free(hs->hdls[idx]);
    hs->hdls[idx] = NULL;
}
