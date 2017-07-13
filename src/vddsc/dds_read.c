#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"



static _Check_return_ dds_return_t
dds_read_lock(
        _In_ dds_entity_t hdl,
        _Out_ dds_reader   **reader,
        _Out_ dds_readcond **condition)
{
    dds_return_t ret = hdl;
    assert(reader);
    assert(condition);
    *reader = NULL;
    *condition = NULL;
    if (hdl >= 0) {
        switch (dds_entity_kind(hdl) ) {
            case DDS_KIND_READER: {
                ret = dds_entity_lock(hdl, DDS_KIND_READER, (dds_entity**)reader);
                break;
            }
            case DDS_KIND_COND_READ:
                /* FALLTHROUGH */
            case DDS_KIND_COND_QUERY: {
                ret = dds_entity_lock(hdl, DDS_KIND_DONTCARE, (dds_entity**)condition);
                if (ret == DDS_RETCODE_OK) {
                    dds_entity *parent = ((dds_entity*)*condition)->m_parent;
                    assert(parent);
                    ret = dds_entity_lock(parent->m_hdl, DDS_KIND_READER, (dds_entity**)reader);
                    if (ret != DDS_RETCODE_OK) {
                        dds_entity_unlock((dds_entity*)*condition);
                    }
                }
                break;
            }
            default: {
                ret = dds_valid_hdl(hdl, DDS_KIND_DONTCARE);
                if (ret == DDS_RETCODE_OK) {
                    ret = DDS_RETCODE_ILLEGAL_OPERATION;
                }
                break;
            }
        }
    }
    return DDS_ERRNO(ret, DDS_MOD_READER, 0);
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
static int dds_read_impl
(
  bool take, dds_entity_t reader_or_condition, void ** buf,
  uint32_t maxs, dds_sample_info_t * si, uint32_t mask,
  dds_instance_handle_t hand
)
{
  uint32_t i;
  int32_t ret = DDS_RETCODE_OK;
  struct dds_reader * rd;
  struct dds_readcond * cond;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool lock = maxs != 0;

  assert (buf);
  assert (si);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  ret = dds_read_lock(reader_or_condition, &rd, &cond);
  if (ret == DDS_RETCODE_OK) {
      if (maxs == 0)
      {
        maxs = dds_rhc_samples (rd->m_rd->rhc);
      }

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
        ret = dds_rhc_take(rd->m_rd->rhc, lock, buf, si, maxs, mask, hand, cond);
      } else {
        ret = dds_rhc_read(rd->m_rd->rhc, lock, buf, si, maxs, mask, hand, cond);
      }

      /* read/take resets data available status */
      dds_entity_status_reset(rd, DDS_DATA_AVAILABLE_STATUS);

      /* reset DATA_ON_READERS status on subscriber after successful read/take */

      if (dds_entity_kind(((dds_entity*)rd)->m_parent->m_hdl) == DDS_KIND_SUBSCRIBER)
      {
        dds_entity_status_reset(((dds_entity*)rd)->m_parent, DDS_DATA_ON_READERS_STATUS);
      }
      dds_read_unlock(rd, cond);
  }

  if (asleep)
  {
    thread_state_asleep (thr);
  }

  return ret;
}

static int dds_readcdr_impl
(
 bool take, dds_entity_t reader_or_condition, struct serdata ** buf,
 uint32_t maxs, dds_sample_info_t * si, uint32_t mask,
 dds_instance_handle_t hand
 )
{
  int32_t ret = DDS_RETCODE_OK;
  struct dds_reader * rd;
  struct dds_readcond * cond;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool lock = maxs != 0;

  assert (take);
  assert (buf);
  assert (si);
  assert (hand == DDS_HANDLE_NIL);
  assert (maxs > 0);

  if (asleep)
  {
    thread_state_awake (thr);
  }
  ret = dds_read_lock(reader_or_condition, &rd, &cond);
  if (ret == DDS_RETCODE_OK) {
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
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs)
{
  return dds_read_impl (false, rd_or_cnd, buf, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs)
{
  return dds_read_impl (false, rd_or_cnd, buf, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_mask(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
  return dds_read_impl (false, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_read_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
  return dds_read_impl (false, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL);
}

int
dds_read_instance(
        dds_entity_t reader,
        void **buf,
        uint32_t maxs,
        dds_sample_info_t *si,
        dds_instance_handle_t handle,
        uint32_t mask)
{
  assert (handle != DDS_HANDLE_NIL);
  return dds_read_impl (false, reader, buf, maxs, si, mask, handle);
}


int
dds_read_next(
        dds_entity_t reader,
        void **buf,
        dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  return dds_read_impl (false, reader, buf, 1u, si, mask, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs)
{
  return dds_read_impl (true, rd_or_cnd, buf, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs)
{
  return dds_read_impl (true, rd_or_cnd, buf, maxs, si, NO_STATE_MASK_SET, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_mask(
_In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ size_t bufsz,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
  return dds_read_impl (true, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL);
}

_Pre_satisfies_(((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_READER ) ||\
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_READ ) || \
                ((rd_or_cnd & DDS_ENTITY_KIND_MASK) == DDS_KIND_COND_QUERY ))
dds_return_t
dds_take_mask_wl(
        _In_ dds_entity_t rd_or_cnd,
        _Out_ void ** buf,
        _Out_ dds_sample_info_t * si,
        _In_ uint32_t maxs,
        _In_ uint32_t mask)
{
  return dds_read_impl (true, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL);
}

int
dds_takecdr(
        dds_entity_t rd_or_cnd,
        struct serdata **buf,
        uint32_t maxs,
        dds_sample_info_t *si,
        uint32_t mask)
{
  return dds_readcdr_impl (true, rd_or_cnd, buf, maxs, si, mask, DDS_HANDLE_NIL);
}

int
dds_take_instance(
        dds_entity_t reader,
        void **buf,
        uint32_t maxs,
        dds_sample_info_t *si,
        dds_instance_handle_t handle,
        uint32_t mask)
{
  assert (handle != DDS_HANDLE_NIL);
  return dds_read_impl (true, reader, buf, maxs, si, mask, handle);
}

int
dds_take_next(
        dds_entity_t reader,
        void **buf,
        dds_sample_info_t *si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  return dds_read_impl (true, reader, buf, 1u, si, mask, DDS_HANDLE_NIL);
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
    uint32_t ret;
    const dds_topic_descriptor_t * desc;
    dds_reader *rd;
    dds_readcond *cond;

    if (!buf || (*buf == NULL && bufsz > 0)) {
        return DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_READER, DDS_ERR_M1);
    }

    ret = dds_read_lock(reader_or_condition, &rd, &cond);
    if (ret == DDS_RETCODE_OK) {
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

    return ret;
}
