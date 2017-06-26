#include <assert.h>
#include "os/os.h"
#include "kernel/dds_types.h"
#include "kernel/dds_waitset.h"
#include "kernel/dds_entity.h"
#include "kernel/dds_querycond.h"
#include "kernel/dds_readcond.h"
#include "kernel/dds_rhc.h"

#include "ddsi/q_globals.h"
#include "ddsi/q_config.h"
#include "ddsi/q_log.h"

#define dds_waitset_lock(hdl, obj) dds_entity_lock(hdl, DDS_KIND_WAITSET, (dds_entity**)obj)
#define dds_waitset_unlock(obj)    dds_entity_unlock((dds_entity*)obj);


static void
dds_waitset_swap(
        _Inout_   dds_attachment **dst,
        _In_      dds_attachment **src,
        _In_opt_  dds_attachment  *prev,
        _In_      dds_attachment  *idx)
{
    /* Remove from source. */
    if (prev == NULL) {
        *src = idx->next;
    } else {
        prev->next = idx->next;
    }

    /* Add to destination. */
    idx->next = *dst;
    *dst = idx;
}

static void
dds_waitset_signal_entity(
        _In_ dds_waitset *ws)
{
    dds_entity *e = (dds_entity*)ws;
    /* When signaling any observers of us through the entity,
     * we need to be unlocked. We still have claimed the related
     * handle, so possible deletions will be delayed until we
     * release it. */
    os_mutexUnlock(&(e->m_mutex));
    dds_entity_status_signal(e);
    os_mutexLock(&(e->m_mutex));
}

static dds_return_t
dds_waitset_entity_triggered(
        _In_ dds_waitset *ws,
        _In_ dds_entity_t observed)
{
    dds_return_t ret;
    dds_entity *w = (dds_entity*)ws;
    /* If we observer ourselves, then we are already locked at this point. Just
     * get the trigger value iso going the official way, which would deadlock. */
    if (w->m_hdl == observed) {
        ret = (w->m_trigger != 0);
    } else {
        ret = dds_triggered(observed);
    }
    return ret;
}

static dds_return_t
dds_waitset_wait_impl(
        _In_ dds_entity_t waitset,
        _Out_writes_to_(nxs, return < 0 ? 0 : return) dds_attach_t *xs,
        _In_ size_t nxs,
        _In_ dds_time_t abstimeout,
        _In_ dds_time_t tnow)
{
    dds_waitset *ws;
    dds_return_t ret;
    dds_attachment *idx;
    dds_attachment *next;
    dds_attachment *prev;

    /* Locking the waitset here will delay a possible deletion until it is
     * unlocked. Even when the related mutex is unlocked by a conditioned wait. */
    ret = dds_waitset_lock(waitset, &ws);
    if (ret == DDS_RETCODE_OK) {
        /* Check if any of the entities have been triggered. */
        idx = ws->observed;
        prev = NULL;
        while (idx != NULL) {
            next = idx->next;
            if (dds_waitset_entity_triggered(ws, idx->entity) > 0) {
                /* Move observed entity to triggered list. */
                dds_waitset_swap(&(ws->triggered), &(ws->observed), prev, idx);
            } else {
                prev = idx;
            }
            idx = next;
        }

        /* Only wait/keep waiting when whe have something to observer and there aren't any triggers yet. */
        ret = DDS_RETCODE_OK;
        while ((ws->observed != NULL) && (ws->triggered == NULL) && (ret == DDS_RETCODE_OK)) {
            if (abstimeout == DDS_NEVER) {
                os_condWait(&ws->m_entity.m_cond, &ws->m_entity.m_mutex);
            } else if (abstimeout <= tnow) {
                ret = DDS_RETCODE_TIMEOUT;
            } else {
                dds_duration_t dt = abstimeout - tnow;
                os_time to;
                if ((dt / (dds_duration_t)DDS_NSECS_IN_SEC) >= (dds_duration_t)OS_TIME_INFINITE_SEC) {
                  to.tv_sec = OS_TIME_INFINITE_SEC;
                  to.tv_nsec = DDS_NSECS_IN_SEC - 1;
                } else {
                  to.tv_sec = (os_timeSec) (dt / DDS_NSECS_IN_SEC);
                  to.tv_nsec = (uint32_t) (dt % DDS_NSECS_IN_SEC);
                }
                os_condTimedWait(&ws->m_entity.m_cond, &ws->m_entity.m_mutex, &to);
                tnow = dds_time();
            }
        }

        /* Get number of triggered entities
         *   - set attach array when needed
         *   - swap them back to observed */
        if (ret == DDS_RETCODE_OK) {
            ret = 0;
            idx = ws->triggered;
            while (idx != NULL) {
                if ((uint32_t)ret < (uint32_t)nxs) {
                    xs[ret] = idx->arg;
                }
                ret++;

                next = idx->next;
                /* The idx is always the first in triggered, so no prev. */
                dds_waitset_swap(&(ws->observed), &(ws->triggered), NULL, idx);
                idx = next;
            }
        } else {
            ret = DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M1);
        }

        dds_waitset_unlock(ws);
    } else {
        ret = DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M2);
    }

    return ret;
}

static void
dds_waitset_close_list(
        _In_ dds_attachment **list,
        _In_ dds_entity_t waitset)
{
    dds_attachment *idx = *list;
    dds_attachment *next;
    while (idx != NULL) {
        next = idx->next;
        (void)dds_entity_observer_unregister(idx->entity, waitset);
         os_free(idx);
         idx = next;
    }
    *list = NULL;
}

static bool
dds_waitset_remove_from_list(
        _In_ dds_attachment **list,
        _In_ dds_entity_t observed)
{
    dds_attachment *idx = *list;
    dds_attachment *prev = NULL;

    while (idx != NULL) {
        if (idx->entity == observed) {
            if (prev == NULL) {
                *list = idx->next;
            } else {
                prev->next = idx->next;
            }
            os_free(idx);

            /* We're done. */
            return true;
        }
    }
    return false;
}

dds_return_t dds_waitset_close(struct dds_entity *e)
{
    dds_waitset *ws = (dds_waitset*)e;

    dds_waitset_close_list(&(ws->observed),  e->m_hdl);
    dds_waitset_close_list(&(ws->triggered), e->m_hdl);

    /* Trigger waitset to wake up. */
    os_condBroadcast(&e->m_cond);

    return DDS_RETCODE_OK;
}

_Pre_satisfies_((participant & DDS_ENTITY_KIND_MASK) == DDS_KIND_PARTICIPANT)
DDS_EXPORT _Must_inspect_result_ dds_entity_t
dds_create_waitset(
        _In_ dds_entity_t participant)
{
    dds_entity_t hdl;
    dds_entity *par;
    dds_return_t ret;
    ret = dds_entity_lock(participant, DDS_KIND_PARTICIPANT, &par);
    if (ret == DDS_RETCODE_OK) {
        dds_waitset *waitset = dds_alloc(sizeof(*waitset));
        hdl = dds_entity_init(&waitset->m_entity, par, DDS_KIND_WAITSET, NULL, NULL, 0);
        waitset->m_entity.m_deriver.close = dds_waitset_close;
        waitset->observed = NULL;
        waitset->triggered = NULL;
        dds_entity_unlock(par);
    } else {
        hdl = DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M5);
    }
    return hdl;
}


_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
DDS_EXPORT dds_return_t
dds_waitset_get_conditions(
        _In_ dds_entity_t waitset,
        _Out_writes_to_(size, return < 0 ? 0 : return) dds_entity_t *conds,
        _In_ size_t size)
{
    /* TODO: Implement dds_waitset_get_conditions (rename it to dds_waitset_get_entities???). */
    return DDS_ERRNO(DDS_RETCODE_UNSUPPORTED, DDS_MOD_WAITSET, DDS_ERR_M5);
}


static void
dds_waitset_move(
        _In_    dds_attachment **src,
        _Inout_ dds_attachment **dst,
        _In_    dds_entity_t entity)
{
    dds_attachment *idx = *src;
    dds_attachment *prev = NULL;
    while (idx != NULL) {
        if (idx->entity == entity) {
            /* Swap idx from src to dst. */
            dds_waitset_swap(dst, src, prev, idx);

            /* We're done. */
            return;
        }
        prev = idx;
    }
}

static void
dds_waitset_remove(
        dds_waitset *ws,
        dds_entity_t observed)
{
    if (!dds_waitset_remove_from_list(&(ws->observed), observed)) {
        dds_waitset_remove_from_list(&(ws->triggered), observed);
    }
}

/* This is called when the observed entity signals a status change. */
void dds_waitset_observer(dds_entity_t observer, dds_entity_t observed, uint32_t status)
{
    dds_waitset *ws;
    if (dds_waitset_lock(observer, &ws) == DDS_RETCODE_OK) {
        if (status & DDS_DELETING_STATUS) {
            /* Remove this observed entity, which is being deleted, from the waitset. */
            dds_waitset_remove(ws, observed);
            /* Our registration to this observed entity will be removed automatically. */
        } else if (status != 0) {
            /* Move observed entity to triggered list. */
            dds_waitset_move(&(ws->observed), &(ws->triggered), observed);
        } else {
            /* Remove observed entity from triggered list (which it possibly resides in). */
            dds_waitset_move(&(ws->triggered), &(ws->observed), observed);
        }
        /* Trigger waitset to wake up. */
        os_condBroadcast(&ws->m_entity.m_cond);
        dds_waitset_unlock(ws);
    }
}

_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
DDS_EXPORT dds_return_t
dds_waitset_attach(
        _In_ dds_entity_t waitset,
        _In_ dds_entity_t entity,
        _In_ dds_attach_t x)
{
    dds_waitset *ws;
    dds_return_t ret;
    ret = dds_waitset_lock(waitset, &ws);
    if (ret == DDS_RETCODE_OK) {
        /* This will fail if given entity is already attached (or deleted). */
        if (waitset == entity) {
            ret = dds_entity_observer_register_nl((dds_entity*)ws, waitset, dds_waitset_observer);
        } else {
            ret = dds_entity_observer_register(entity, waitset, dds_waitset_observer);
        }
        if (ret == DDS_RETCODE_OK) {
            dds_attachment *a = os_malloc(sizeof(dds_attachment));
            a->arg = x;
            a->entity = entity;
            ret = dds_waitset_entity_triggered(ws, entity);
            if (ret > 0) {
                a->next = ws->triggered;
                ws->triggered = a;
                ret = DDS_RETCODE_OK;
            } else if (ret == 0) {
                a->next = ws->observed;
                ws->observed = a;
                ret = DDS_RETCODE_OK;
            } else {
                os_free(a);
            }
        }
        dds_waitset_unlock(ws);
    }
    return DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M2);
}

_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
DDS_EXPORT dds_return_t
dds_waitset_detach(
        _In_ dds_entity_t waitset,
        _In_ dds_entity_t entity)
{
    dds_waitset *ws;
    dds_return_t ret;
    ret = dds_waitset_lock(waitset, &ws);
    if (ret == DDS_RETCODE_OK) {
        /* Possibly fails when entity was not attached. */
        if (waitset == entity) {
            ret = dds_entity_observer_unregister_nl((dds_entity*)ws, waitset);
        } else {
            ret = dds_entity_observer_unregister(entity, waitset);
        }
        dds_waitset_remove(ws, entity);
        dds_waitset_unlock(ws);
    }
    return DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M2);
}

_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
dds_return_t
dds_waitset_wait_until(
        _In_ dds_entity_t waitset,
        _Out_writes_to_(nxs, return < 0 ? 0 : return) dds_attach_t *xs,
        _In_ size_t nxs,
        _In_ dds_time_t abstimeout)
{
  return dds_waitset_wait_impl(waitset, xs, nxs, abstimeout, dds_time());
}

_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
dds_return_t
dds_waitset_wait(
        _In_ dds_entity_t waitset,
        _Out_writes_to_(nxs, return < 0 ? 0 : return) dds_attach_t *xs,
        _In_ size_t nxs,
        _In_ dds_duration_t reltimeout)
{
  dds_time_t tnow = dds_time();
  dds_time_t abstimeout = (DDS_INFINITY - reltimeout <= tnow) ? DDS_NEVER : (tnow + reltimeout);
  return dds_waitset_wait_impl(waitset, xs, nxs, abstimeout, tnow);
}

_Pre_satisfies_((waitset & DDS_ENTITY_KIND_MASK) == DDS_KIND_WAITSET)
dds_return_t
dds_waitset_set_trigger(
        _In_ dds_entity_t waitset,
        _In_ bool trigger)
{
    dds_waitset *ws;
    dds_return_t ret;

    /* Locking the waitset here will delay a possible deletion until it is
     * unlocked. Even when the related mutex is unlocked when we want to send
     * a signal. */
    ret = dds_waitset_lock(waitset, &ws);
    if (ret == DDS_RETCODE_OK) {
        if (trigger) {
            dds_entity_status_set(ws, DDS_WAITSET_TRIGGER_STATUS);
        } else {
            dds_entity_status_reset(ws, DDS_WAITSET_TRIGGER_STATUS);
        }
        dds_waitset_signal_entity(ws);
        dds_waitset_unlock(ws);
    }
    return DDS_ERRNO(ret, DDS_MOD_WAITSET, DDS_ERR_M2);
}

