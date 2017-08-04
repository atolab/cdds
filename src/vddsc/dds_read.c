#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"



static _Check_return_ dds_retcode_t
dds_read_lock(
        _In_ dds_entity_t hdl,
        _Out_ dds_reader   **reader,
        _Out_ dds_readcond **condition)
{
    dds_retcode_t rc = hdl;
    assert(reader);
    assert(condition);
    *reader = NULL;
    *condition = NULL;
    if (hdl >= 0) {
        switch (dds_entity_kind(hdl) ) {
            case DDS_KIND_READER: {
                rc = dds_entity_lock(hdl, DDS_KIND_READER, (dds_entity**)reader);
                break;
            }
            case DDS_KIND_COND_READ:
                /* FALLTHROUGH */
            case DDS_KIND_COND_QUERY: {
                rc = dds_entity_lock(hdl, DDS_KIND_DONTCARE, (dds_entity**)condition);
                if (rc == DDS_RETCODE_OK) {
                    dds_entity *parent = ((dds_entity*)*condition)->m_parent;
                    assert(parent);
                    rc = dds_entity_lock(parent->m_hdl, DDS_KIND_READER, (dds_entity**)reader);
                    if (rc != DDS_RETCODE_OK) {
                        dds_entity_unlock((dds_entity*)*condition);
                    }
                }
                break;
            }
            default: {
                rc = dds_valid_hdl(hdl, DDS_KIND_DONTCARE);
                if (rc == DDS_RETCODE_OK) {
                    rc = DDS_RETCODE_ILLEGAL_OPERATION;
                }
                break;
            }
        }
    }
    return rc;
}

static void
dds_read_unlock(
        _In_ dds_reader   *reader,
        _In_ dds_readcond *condition)
{
    assert(reader);
    dds_entity_unlock((dds_entity*)reader);
    if (condition) {
        dds_entity_unlock((dds_entity*)condition);
    }
}
/*
  dds_read_impl: Core read/take function. Usually maxs is size of buf and si
  into which samples/status are written, when set to zero is special case
  indicating that size set from number of samples in cache and also that cache
  has been locked. This is used to support C++ API reading length unlimited
  which is interpreted as "all relevant samples in cache".
*/
static dds_return_t
dds_read_impl(
        _In_  bool take,
        _In_  dds_entity_t reader_or_condition,
        _Inout_ void **buf,
        _In_ size_t bufsz,
        _In_  uint32_t maxs,
        _Out_ dds_sample_info_t *si,
        _In_  uint32_t mask,
        _In_  dds_instance_handle_t hand,
        _In_  bool lock)
{
  uint32_t i;
  dds_return_t ret = DDS_RETCODE_OK;
  dds_retcode_t rc;
  struct dds_reader * rd;
  struct dds_readcond * cond;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);

  if (asleep)
  {
    thread_state_awake (thr);
  }

  rc = dds_read_lock(reader_or_condition, &rd, &cond);
  if (rc == DDS_RETCODE_OK) {
      if ((buf == NULL) || (si == NULL) || (maxs == 0) || (bufsz == 0) || (bufsz < maxs)) {
          rc = DDS_RETCODE_BAD_PARAMETER;
          dds_read_unlock(rd, cond);
      } else if (hand != DDS_HANDLE_NIL) {
          if (dds_tkmap_find_by_id(gv.m_tkmap, hand) == NULL) {
              rc = DDS_RETCODE_PRECONDITION_NOT_MET;
              dds_read_unlock(rd, cond);
          }
      }
  }

  if (rc == DDS_RETCODE_OK) {
      /* Allocate samples if not provided (assuming all or none provided) */

      if (buf[0] == NULL)
      {
        char * loan;
        const size_t sz = rd->m_topic->m_descriptor->m_size;
        const uint32_t loan_size = (uint32_t) (sz * maxs);

        /* Allocate, use or reallocate loan cached on reader */

        if (rd->m_loan_out)
        {
          loan = dds_alloc (loan_size);
        }
        else
        {
          if (rd->m_loan)
          {
            if (rd->m_loan_size < loan_size)
            {
              rd->m_loan = dds_realloc_zero (rd->m_loan, loan_size);
              rd->m_loan_size = loan_size;
            }
          }
          else
          {
            rd->m_loan = dds_alloc (loan_size);
            rd->m_loan_size = loan_size;
          }
          loan = rd->m_loan;
          rd->m_loan_out = true;
        }
        for (i = 0; i < maxs; i++)
        {
          buf[i] = loan;
          loan += sz;
        }
      }

      if (take) {
        ret = (dds_return_t)dds_rhc_take(rd->m_rd->rhc, lock, buf, si, maxs, mask, hand, cond);
      } else {
        ret = (dds_return_t)dds_rhc_read(rd->m_rd->rhc, lock, buf, si, maxs, mask, hand, cond);
      }

      /* read/take resets data available status */
      dds_entity_status_reset(rd, DDS_DATA_AVAILABLE_STATUS);

      /* reset DATA_ON_READERS status on subscriber after successful read/take */

      if (dds_entity_kind(((dds_entity*)rd)->m_parent->m_hdl) == DDS_KIND_SUBSCRIBER)
      {
        dds_entity_status_reset(((dds_entity*)rd)->m_parent, DDS_DATA_ON_READERS_STATUS);
      }
      dds_read_unlock(rd, cond);
  } else {
      ret = DDS_ERRNO(rc);
  }

  if (asleep)
  {
    thread_state_asleep (thr);
  }

  return ret;
}

static dds_return_t
dds_readcdr_impl(
        _In_  bool take,
        _In_  dds_entity_t reader_or_condition,
        _Out_ struct serdata ** buf,
        _In_  uint32_t maxs,
        _Out_ dds_sample_info_t * si,
        _In_  uint32_t mask,
        _In_  dds_instance_handle_t hand,
        _In_  bool lock)
{
  dds_return_t ret = DDS_RETCODE_OK;
  dds_retcode_t rc;
  struct dds_reader * rd;
  struct dds_readcond * cond;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);

  assert (take);
  assert (buf);
  assert (si);
  assert (hand == DDS_HANDLE_NIL);
  assert (maxs > 0);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  rc = dds_read_lock(reader_or_condition, &rd, &cond);
  if (rc >= DDS_RETCODE_OK) {
      ret = dds_rhc_takecdr
        (
         rd->m_rd->rhc, lock, buf, si, maxs,
         mask & DDS_ANY_SAMPLE_STATE,
         mask & DDS_ANY_VIEW_STATE,
         mask & DDS_ANY_INSTANCE_STATE,
         hand
         );

      /* read/take resets data available status */
      dds_entity_status_reset(rd, DDS_DATA_AVAILABLE_STATUS);

      /* reset DATA_ON_READERS status on subscriber after successful read/take */

      if (dds_entity_kind(((dds_entity*)rd)->m_parent->m_hdl) == DDS_KIND_SUBSCRIBER)
      {
        dds_entity_status_reset(((dds_entity*)rd)->m_parent, DDS_DATA_ON_READERS_STATUS);
      }
      dds_read_unlock(rd, cond);
  } else {
      ret = DDS_ERRNO(rc);
  }

  if (asleep)
  {
    thread_state_asleep (thr);
  }

  return ret;
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs, so use bufsz instead.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = (uint32_t)bufsz;
    }
    return dds_read_impl (false, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs. Just an arbitrarily number.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = 100;
    }
    return dds_read_impl (false, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_mask(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs, so use bufsz instead.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = (uint32_t)bufsz;
    }
    return dds_read_impl (false, rd_or_cnd, buf, bufsz, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs. Just an arbitrarily number.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = 100;
    }
    return dds_read_impl (false, rd_or_cnd, buf, maxs, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_instance(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, handle, lock);
    }
    return ret;
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_instance_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, handle, lock);
    }
    return ret;
}


_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_instance_mask(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle,
        _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(false, rd_or_cnd, buf, bufsz, maxs, si, mask, handle, lock);
    }
    return ret;
}


_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_instance_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle,
        _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(false, rd_or_cnd, buf, maxs, maxs, si, mask, handle, lock);
    }
    return ret;
}

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER )
dds_return_t
dds_read_next(
        _In_ dds_entity_t reader,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si)
{
  dds_return_t ret = (dds_return_t) reader;
  if(reader >= 0){
    if(dds_entity_kind(reader) == DDS_KIND_READER){
      uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
      ret = dds_read_impl (false, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true);
    }
    else{
      dds_retcode_t rc;
      rc = dds_valid_hdl(reader, DDS_KIND_DONTCARE);
      if(rc == DDS_RETCODE_OK){
        ret = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION);
      } else{
        ret = DDS_ERRNO(rc);
      }
    }
  }
  return ret;
}

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER )
dds_return_t
dds_read_next_wl(
        _In_ dds_entity_t reader,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si)
{
  dds_return_t ret = (dds_return_t) reader;
  if(reader >= 0){
    if(dds_entity_kind(reader) == DDS_KIND_READER){
      uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
      ret = dds_read_impl (false, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true);
    }
    else{
      dds_retcode_t rc;
      rc = dds_valid_hdl(reader, DDS_KIND_DONTCARE);
      if(rc == DDS_RETCODE_OK){
        ret = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION);
      } else{
        ret = DDS_ERRNO(rc);
      }
    }
  }
  return ret;
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs, so use bufsz instead.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = (uint32_t)bufsz;
    }
    return dds_read_impl (true, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs. Just an arbitrarily number.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = 100;
    }
    return dds_read_impl (true, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_mask(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs, so use bufsz instead.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = (uint32_t)bufsz;
    }
    return dds_read_impl (true, rd_or_cnd, buf, bufsz, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs. Just an arbitrarily number.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = 100;
    }
    return dds_read_impl (true, rd_or_cnd, buf, maxs, maxs, si, mask, DDS_HANDLE_NIL, lock);
}

int
dds_takecdr(
        dds_entity_t rd_or_cnd,
        struct serdata **buf,
        uint32_t maxs,
        dds_sample_info_t *si,
        uint32_t mask)
{
    bool lock = true;
    if (maxs == DDS_READ_WITHOUT_LOCK) {
        lock = false;
        /* Use a more sensible maxs. Just an arbitrarily number.
         * CHAM-306 will remove this ugly piece of code. */
        maxs = 100;
    }
    return dds_readcdr_impl (true, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL, lock);
}


_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_instance(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, NO_STATE_MASK_SET, handle, lock);
    }
    return ret;
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_instance_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, NO_STATE_MASK_SET, handle, lock);
    }
    return ret;
}


_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_instance_mask(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle,
        _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(true, rd_or_cnd, buf, bufsz, maxs, si, mask, handle, lock);
    }
    return ret;
}


_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_instance_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si,
        _In_ uint32_t maxs,
        _In_ dds_instance_handle_t handle,
        _In_ uint32_t mask)
{
    dds_return_t ret = DDS_ERRNO(DDS_RETCODE_PRECONDITION_NOT_MET);
    if (handle != DDS_HANDLE_NIL) {
        bool lock = true;
        if (maxs == DDS_READ_WITHOUT_LOCK) {
            lock = false;
            /* Use a more sensible maxs. Just an arbitrarily number.
             * CHAM-306 will remove this ugly piece of code. */
            maxs = 100;
        }
        ret = dds_read_impl(true, rd_or_cnd, buf, maxs, maxs, si, mask, handle, lock);
    }
    return ret;
}

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER )
dds_return_t
dds_take_next(
        _In_ dds_entity_t reader,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si)
{
  dds_return_t ret = (dds_return_t) reader;
  if(reader >= 0){
    if(dds_entity_kind(reader) == DDS_KIND_READER){
      uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
      ret = dds_read_impl (true, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true);
    }
    else{
      dds_retcode_t rc;
      rc = dds_valid_hdl(reader, DDS_KIND_DONTCARE);
      if(rc == DDS_RETCODE_OK){
        ret = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION);
      } else{
        ret = DDS_ERRNO(rc);
      }
    }
  }
  return ret;
}

_Pre_satisfies_((reader & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER )
dds_return_t
dds_take_next_wl(
        _In_ dds_entity_t reader,
        _Inout_ void **buf,
        _Out_ dds_sample_info_t *si)
{
  dds_return_t ret = (dds_return_t) reader;
  if(reader >= 0){
    if(dds_entity_kind(reader) == DDS_KIND_READER){
      uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
      ret = dds_read_impl (true, reader, buf, 1u, 1u, si, mask, DDS_HANDLE_NIL, true);
    }
    else{
      dds_retcode_t rc;
      rc = dds_valid_hdl(reader, DDS_KIND_DONTCARE);
      if(rc == DDS_RETCODE_OK){
        ret = DDS_ERRNO(DDS_RETCODE_ILLEGAL_OPERATION);
      } else{
        ret = DDS_ERRNO(rc);
      }
    }
  }
  return ret;
}

_Pre_satisfies_(((reader_or_condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((reader_or_condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((reader_or_condition & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
_Must_inspect_result_ dds_return_t
dds_return_loan(
        _In_ dds_entity_t reader_or_condition,
        _Inout_updates_(bufsz) void **buf,
        _In_ size_t bufsz)
{
    dds_retcode_t rc;
    const dds_topic_descriptor_t * desc;
    dds_reader *rd;
    dds_readcond *cond;

    if (!buf || (*buf == NULL && bufsz > 0)) {
        return DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER);
    }

    rc = dds_read_lock(reader_or_condition, &rd, &cond);
    if (rc == DDS_RETCODE_OK) {
        desc = rd->m_topic->m_descriptor;

        /* Only free sample contents if they have been allocated */
        if (desc->m_flagset & DDS_TOPIC_NO_OPTIMIZE) {
            size_t i = 0;
            for (i = 0; i < bufsz; i++) {
                dds_sample_free(buf[i], desc, DDS_FREE_CONTENTS);
            }
        }

        /* If possible return loan buffer to reader */
        if (rd->m_loan != 0 && (buf[0] == rd->m_loan)) {
            rd->m_loan_out = false;
            memset (rd->m_loan, 0, rd->m_loan_size);
            buf[0] = NULL;
        }

        dds_read_unlock(rd, cond);
    }

    return DDS_ERRNO(rc);
}
