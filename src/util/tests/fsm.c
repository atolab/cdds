#include "os/os.h"
#include "util/ut_fsm.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>



/*****************************************************************************************
 *                                                                                       *
 *            PLUGIN SIMULATION                                                          *
 *                                                                                       *
 *****************************************************************************************/

typedef enum {
    VALIDATION_PENDING_RETRY,
    VALIDATION_FAILED,
    VALIDATION_OK,
    VALIDATION_OK_FINAL_MESSAGE,
    VALIDATION_PENDING_HANDSHAKE_MESSAGE,
    VALIDATION_PENDING_HANDSHAKE_REQUEST,
    PluginReturn_MAX
} PluginReturn;

static PluginReturn
validate_remote_identity(void)
{
    static int first = 1;
    if (first) {
        first = 0;
        return VALIDATION_PENDING_RETRY;
    }
    return VALIDATION_PENDING_HANDSHAKE_MESSAGE;
}

static PluginReturn
begin_handshake_reply(void)
{
    static int first = 1;
    if (first) {
        first = 0;
        return VALIDATION_PENDING_RETRY;
    }
    return VALIDATION_OK_FINAL_MESSAGE;
}

static PluginReturn
get_shared_secret(void)
{
    return VALIDATION_OK;
}




/*****************************************************************************************
 *                                                                                       *
 *            PART HANDSHAKE STATE-MACHINE (based on french state-diagram)               *
 *                                                                                       *
 *****************************************************************************************/

/* State actions. */
static void
fsm_validate_remote_identity(struct ut_fsm *fsm, void *arg)
{
    PluginReturn ret;
    ret = validate_remote_identity();
    ut_fsm_dispatch(fsm, ret);
}

static void
fsm_begin_handshake_reply(struct ut_fsm *fsm, void *arg)
{
    PluginReturn ret;
    ret = begin_handshake_reply();
    if (ret == VALIDATION_OK_FINAL_MESSAGE) {
        ret = get_shared_secret();
    }
    ut_fsm_dispatch(fsm, ret);
}



static const os_time msec100 = { 0, 100000000 };

/* A few states from the handshake state-machine. */
static ut_fsm_state StateValidateRemoteIdentity   = { fsm_validate_remote_identity, NULL     };
static ut_fsm_state StateValRemIdentityRetryWait  = { NULL,                         &msec100 };
static ut_fsm_state StateHandshakeInitMessageWait = { NULL,                         NULL     };
static ut_fsm_state StateBeginHandshakeReply      = { fsm_begin_handshake_reply,    NULL     };
static ut_fsm_state StateBeginHsReplyWait         = { NULL,                         &msec100 };


/* Transition debug functions. */
static uint32_t visited = 0;
static void a(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x01; }
static void b(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x02; }
static void c(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x04; }
static void d(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x08; }
static void e(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x10; }
static void f(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x20; }
static void g(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x40; }
static void h(struct ut_fsm *fsm, void *arg) { cr_log_info("Transition %s", __FUNCTION__); visited |= 0x80; }


/* We're using the plugin return values directly as event ids in this example. */
#define SHM_MSG_RECEIVED (PluginReturn_MAX + 1)
ut_fsm_transition HandshakeTransistions [] =
{
    { NULL,                           UT_FSM_EVENT_AUTO,                    a, &StateValidateRemoteIdentity   }, // NULL state is the start state
    { &StateValidateRemoteIdentity,   VALIDATION_PENDING_RETRY,             b, &StateValRemIdentityRetryWait  },
    { &StateValidateRemoteIdentity,   VALIDATION_PENDING_HANDSHAKE_MESSAGE, c, &StateHandshakeInitMessageWait },
    { &StateValRemIdentityRetryWait,  UT_FSM_EVENT_TIMEOUT,                 d, &StateValidateRemoteIdentity   },
    { &StateHandshakeInitMessageWait, SHM_MSG_RECEIVED,                     e, &StateBeginHandshakeReply      },
    { &StateBeginHandshakeReply,      VALIDATION_PENDING_RETRY,             f, &StateBeginHsReplyWait         },
    { &StateBeginHandshakeReply,      VALIDATION_OK,                        g, NULL                           }, // Reaching NULL means end of state-diagram
    { &StateBeginHsReplyWait,         UT_FSM_EVENT_TIMEOUT,                 h, &StateBeginHandshakeReply      },
};




/*****************************************************************************************
 *                                                                                       *
 *            TEST(S)                                                                    *
 *                                                                                       *
 *****************************************************************************************/

/* Add --verbose command line argument to get the cr_log_info traces (if there are any). */

/*****************************************************************************************/
Test(util_fsm, handshake)
{
    struct ut_fsm *fsm;

    fsm = ut_fsm_create(HandshakeTransistions, sizeof(HandshakeTransistions)/sizeof(HandshakeTransistions[0]), NULL);
    cr_assert_not_null(fsm, "ut_fsm_create");

    ut_fsm_start(fsm);

    /* Simulate message received when entered StateValRemIdentityRetryWait. */
    while (ut_fsm_current_state(fsm) != &StateHandshakeInitMessageWait) {
        os_nanoSleep(msec100);
    }
    ut_fsm_dispatch(fsm, SHM_MSG_RECEIVED);

    /* Wait until state-machine has finished. */
    while (ut_fsm_current_state(fsm) != NULL) {
        os_nanoSleep(msec100);
    }

    /* All transitions should have been visited. */
    cr_assert_eq(visited, 0xFF, "Not all transitions visited");

    ut_fsm_free(fsm);
}

