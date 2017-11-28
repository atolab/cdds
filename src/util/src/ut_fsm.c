#include <string.h>
#include <assert.h>
#include "os/os.h"
#include "util/ut_fsm.h"

/* TODO: Change polling for condition variable triggering.  */

struct ut_fsm
{
    const ut_fsm_transition *transitions;
    int cnt;
    void *arg;
    const ut_fsm_state *current;
};

typedef struct ut__fsm_event
{
    struct ut_fsm *fsm;
    int event_id;
    struct ut__fsm_event *next;
} ut__fsm_event;

typedef struct ut__fsm_timeout
{
    struct ut_fsm *fsm;
    os_time abs;
    struct ut__fsm_timeout *next;
} ut__fsm_timeout;

static os_threadId ut__fsm_tid;
static int ut__fsm_teardown = 0;
static ut__fsm_event *ut__fsm_queue = NULL;
static ut__fsm_timeout *ut__fsm_timeouts = NULL;
static os_mutex ut__fsm_mutex;


static void
ut__fsm_dispatch(
        struct ut_fsm *fsm,
        int event_id)
{
    ut__fsm_event *event;

    assert(fsm);

    event = os_malloc(sizeof(ut__fsm_event));
    event->fsm = fsm;
    event->event_id = event_id;
    event->next = NULL;

    /* Insert FIFO event. */
    if (ut__fsm_queue) {
        ut__fsm_event *last = ut__fsm_queue;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = event;
    } else {
        ut__fsm_queue = event;
    }
}

static void
ut__fsm_update_timeout(struct ut_fsm *fsm)
{
    int create = 1;
    ut__fsm_timeout *timeout;
    os_mutexLock(&ut__fsm_mutex);
    /* First, remove possible UT_FSM_TIMEOUT_EVENT from ut__fsm_queue. */
    /* TODO: do this removal. */

    /* Now, update possible timeout from the list. */
    timeout = ut__fsm_timeouts;
    while (timeout != NULL) {
        if (timeout->fsm == fsm) {
            create = 0;
            if (fsm->current == NULL) {
                /* TODO: Remove timeout from list (iso infinite). */
                timeout->abs.tv_sec  = OS_TIME_INFINITE_SEC;
                timeout->abs.tv_nsec = OS_TIME_INFINITE_NSEC;
            } else {
                if (fsm->current->timeout == NULL) {
                    timeout->abs.tv_sec  = OS_TIME_INFINITE_SEC;
                    timeout->abs.tv_nsec = OS_TIME_INFINITE_NSEC;
                } else {
                    timeout->abs = os_timeAdd(os_timeGet(), *(fsm->current->timeout));
                }
            }
            break;
        }
        timeout = timeout->next;
    }
    if ((create) && (fsm->current != NULL) && (fsm->current->timeout != NULL)) {
        /* Add a timeout. */
        timeout = os_malloc(sizeof(ut__fsm_timeout));
        timeout->fsm = fsm;
        timeout->abs = os_timeAdd(os_timeGet(), *(fsm->current->timeout));
        timeout->next = ut__fsm_timeouts;
        ut__fsm_timeouts = timeout;
    }
    os_mutexUnlock(&ut__fsm_mutex);
}

static void
ut__fsm_trigger_timeouts(void)
{
    os_time now = os_timeGet();
    ut__fsm_timeout *timeout;
    os_mutexLock(&ut__fsm_mutex);
    timeout = ut__fsm_timeouts;
    while (timeout != NULL) {
        if (!os_timeIsInfinite(timeout->abs)) {
            if (os_timeCompare(timeout->abs, now) < 0) {
                ut__fsm_dispatch(timeout->fsm, UT_FSM_EVENT_TIMEOUT);
            }
        }
        timeout = timeout->next;
    }
    os_mutexUnlock(&ut__fsm_mutex);
}

static void
ut__fsm_state_change(ut__fsm_event *event)
{
    struct ut_fsm *fsm = event->fsm;
    int event_id = event->event_id;

    for (int i = 0; i < fsm->cnt; i++) {
        if ((fsm->transitions[i].begin == fsm->current) &&
            (fsm->transitions[i].event_id == event_id ) ){

            /* Transition. */
            if (fsm->transitions[i].func) {
                fsm->transitions[i].func(fsm, fsm->arg);
            }

            /* New state. */
            fsm->current = fsm->transitions[i].end;
            if (fsm->current && fsm->current->func) {
                fsm->current->func(fsm, fsm->arg);
            }

            /* Set new timeout when needed. */
            ut__fsm_update_timeout(fsm);
        }
    }
}


static uint32_t
ut__fsm_thread(void *a)
{
    ut__fsm_event *event;
    os_time msec10 = { 0, 10000000 };
    while (!ut__fsm_teardown) {
        event = NULL;

        os_mutexLock(&ut__fsm_mutex);
        /* Extract FIFO event. */
        if (ut__fsm_queue) {
            event = ut__fsm_queue;
            ut__fsm_queue = ut__fsm_queue->next;
        }
        os_mutexUnlock(&ut__fsm_mutex);

        if (event) {
            ut__fsm_state_change(event);
            os_free(event);
        } else {
            /* TODO: Change this poll into condition variable triggering. */
            os_nanoSleep(msec10);
        }

        ut__fsm_trigger_timeouts();
    }
    return 0;
}

static void
ut__fsm_fini_once(void)
{
    ut__fsm_teardown = 1;
    os_threadWaitExit(ut__fsm_tid, NULL);
    os_mutexDestroy(&ut__fsm_mutex);
    os_osExit();
}

static void
ut__fsm_init_once(void)
{
    os_osInit();
    os_result osr;
    os_threadAttr attr;

    os_mutexInit(&ut__fsm_mutex);

    os_threadAttrInit(&attr);
    osr = os_threadCreate (&ut__fsm_tid, "ut_fsm_thread", &attr, &ut__fsm_thread, NULL);
    if (osr == os_resultSuccess) {
        os_procAtExit(ut__fsm_fini_once);
    }
}

static int /* 1 = ok, other = error */
ut__fsm_validate(const ut_fsm_transition *transitions, int cnt)
{
    int i;

    for (i = 0; i < cnt; i++) {
        /* It needs to have a start. */
        if ((transitions[i].begin == NULL) &&
            (transitions[i].event_id == UT_FSM_EVENT_AUTO)) {
            return 1;
        }
    }

    return 0;
}

struct ut_fsm*
ut_fsm_create(
        const ut_fsm_transition *transitions,
        int cnt,
        void *arg)
{
    struct ut_fsm* fsm = NULL;

    assert(transitions);
    assert(cnt > 0);

    ut__fsm_init_once();

    if (ut__fsm_validate) {
        fsm = os_malloc(sizeof(struct ut_fsm));
        fsm->transitions = transitions;
        fsm->cnt = cnt;
        fsm->arg = arg;
        fsm->current = NULL;
    }

    return fsm;
}

void
ut_fsm_start(
        struct ut_fsm *fsm)
{
    assert(fsm);
    ut_fsm_dispatch(fsm, UT_FSM_EVENT_AUTO);
}


void
ut_fsm_set_timeout(struct ut_fsm *fsm, ut_fsm_action func, os_time timeout)
{
    /* TODO: implement. */
}

void
ut_fsm_dispatch(
        struct ut_fsm *fsm,
        int event_id)
{
    assert(fsm);
    os_mutexLock(&ut__fsm_mutex);
    ut__fsm_dispatch(fsm, event_id);
    os_mutexUnlock(&ut__fsm_mutex);
}

const ut_fsm_state*
ut_fsm_current_state(struct ut_fsm *fsm)
{
    assert(fsm);
    return fsm->current;
}

void
ut_fsm_cleanup(struct ut_fsm *fsm)
{
    assert(fsm);
    /* TODO: Make sure all events of this fsm are removed from the queue. */
}

void
ut_fsm_free(struct ut_fsm *fsm)
{
    assert(fsm);
    ut_fsm_cleanup(fsm);
    os_free(fsm);
}

