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
#ifndef UT_FSM_H
#define UT_FSM_H

#include "os/os.h"
#include "util/ut_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

/********************************************************************************************
 *
 * TODO CHAM-520: This is only a proof of concept.
 *
 ********************************************************************************************/

#define UT_FSM_NEVER { OS_TIME_INFINITE_SEC, OS_TIME_INFINITE_NSEC }

#define UT_FSM_EVENT_AUTO    (-1)
#define UT_FSM_EVENT_TIMEOUT (-2)

struct ut_fsm;

/* It is allowed to call ut_fsm_dispatch() from within a dispatch function. */
typedef void (*ut_fsm_action) (struct ut_fsm *fsm, void *arg);

typedef struct ut_fsm_state {
    const ut_fsm_action func;
    const os_time *timeout;
} ut_fsm_state;

typedef struct ut_fsm_transition {
    const ut_fsm_state *begin;
    const int event_id;
    const ut_fsm_action func;
    const ut_fsm_state *end;
} ut_fsm_transition;

struct ut_fsm *
ut_fsm_create(const ut_fsm_transition *transitions, int cnt, void *arg);

void
ut_fsm_start(struct ut_fsm *fsm);

void /* Set global fsm timeout. */
ut_fsm_set_timeout(struct ut_fsm *fsm, ut_fsm_action func, os_time timeout);

void
ut_fsm_dispatch(struct ut_fsm *fsm, int event_id);

const ut_fsm_state*
ut_fsm_current_state(struct ut_fsm *fsm);

void
ut_fsm_cleanup(struct ut_fsm *fsm);

void /* Implicit cleanup. */
ut_fsm_free(struct ut_fsm *fsm);

#if defined (__cplusplus)
}
#endif

#endif /* UT_FSM_H */
