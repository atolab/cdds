#include <assert.h>
#include <string.h>
#include "kernel/dds_entity.h"
#include "kernel/dds_reader.h"
#include "kernel/dds_rhc.h"
#include "ddsi/q_thread.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_entity.h"

typedef int (*dds_read_fn) (struct rhc *, bool, void **, dds_sample_info_t *, uint32_t, const dds_condition *, dds_instance_handle_t);
typedef int (*dds_readc_fc) (struct rhc *, bool, void **, dds_sample_info_t *, uint32_t, unsigned, unsigned, unsigned, dds_instance_handle_t);

/*
  dds_read_impl: Core read/take function. Usually maxs is size of buf and si
  into which samples/status are written, when set to zero is special case
  indicating that size set from number of samples in cache and also that cache
  has been locked. This is used to support C++ API reading length unlimited
  which is interpreted as "all relevant samples in cache".
*/
static int dds_read_impl
(
  bool take, dds_entity_t reader, void ** buf,
  uint32_t maxs, dds_sample_info_t * si, uint32_t mask,
  dds_condition_t cond, dds_instance_handle_t hand
)
{
  uint32_t i;
  int32_t ret = DDS_RETCODE_OK;
  struct dds_reader * rd;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool lock = maxs != 0;

  assert (buf);
  assert (si);

#ifndef NDEBUG
  if (cond)
  {
    assert (cond->m_kind & DDS_TYPE_COND_READ);
  }
#endif

  if (asleep)
  {
    thread_state_awake (thr);
  }
  ret = dds_reader_lock(reader, &rd);
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
      if (cond)
      {
        dds_read_fn fn = take ? dds_rhc_take_w_condition : dds_rhc_read_w_condition;
        ret = (fn) (rd->m_rd->rhc, lock, buf, si, maxs, cond, hand);
      }
      else
      {
        dds_readc_fc fn = take ? dds_rhc_take : dds_rhc_read;
        ret = (fn)
        (
          rd->m_rd->rhc, lock, buf, si, maxs,
          mask & DDS_ANY_SAMPLE_STATE,
          mask & DDS_ANY_VIEW_STATE,
          mask & DDS_ANY_INSTANCE_STATE,
          hand
        );
      }

      /* read/take resets data available status */
      dds_entity_status_reset(rd, DDS_DATA_AVAILABLE_STATUS);

      /* reset DATA_ON_READERS status on subscriber after successful read/take */

      if (dds_entity_kind(((dds_entity*)rd)->m_parent->m_hdl) == DDS_KIND_SUBSCRIBER)
      {
        dds_entity_status_reset(((dds_entity*)rd)->m_parent, DDS_DATA_ON_READERS_STATUS);
      }
      dds_reader_unlock(rd);
  } else {
      ret = DDS_ERRNO(ret, DDS_MOD_READER, 0);
  }

  if (asleep)
  {
    thread_state_asleep (thr);
  }

  return ret;
}

static int dds_readcdr_impl
(
 bool take, dds_entity_t reader, struct serdata ** buf,
 uint32_t maxs, dds_sample_info_t * si, uint32_t mask,
 dds_condition_t cond, dds_instance_handle_t hand
 )
{
  int32_t ret = DDS_RETCODE_OK;
  struct dds_reader * rd;
  struct thread_state1 * const thr = lookup_thread_state ();
  const bool asleep = !vtime_awake_p (thr->vtime);
  const bool lock = maxs != 0;

  assert (take);
  assert (buf);
  assert (si);
  assert (cond == NULL);
  assert (hand == DDS_HANDLE_NIL);
  assert (maxs > 0);

#ifndef NDEBUG
  if (cond)
  {
    assert (cond->m_kind & DDS_TYPE_COND_READ);
  }
#endif

  if (asleep)
  {
    thread_state_awake (thr);
  }
  ret = dds_reader_lock(reader, &rd);
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
      dds_reader_unlock(rd);
  }

  if (asleep)
  {
    thread_state_asleep (thr);
  }

  return ret;
}

int dds_read
(
  dds_entity_t rd, void ** buf, uint32_t maxs,
  dds_sample_info_t * si, uint32_t mask
)
{
  return dds_read_impl (false, rd, buf, maxs, si, mask, NULL, DDS_HANDLE_NIL);
}

int dds_read_instance
(
  dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si,
  dds_instance_handle_t hand, uint32_t mask
)
{
  assert (hand != DDS_HANDLE_NIL);
  return dds_read_impl (false, rd, buf, maxs, si, mask, NULL, hand);
}

int dds_read_cond
(
  dds_entity_t rd, void ** buf, uint32_t maxs,
  dds_sample_info_t * si, dds_condition_t cond
)
{
  assert (cond);
  return dds_read_impl (false, rd, buf, maxs, si, 0, cond, DDS_HANDLE_NIL);
}

int dds_read_next (dds_entity_t rd, void ** buf, dds_sample_info_t * si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  return dds_read_impl (false, rd, buf, 1u, si, mask, NULL, DDS_HANDLE_NIL);
}

int dds_take
(
  dds_entity_t rd, void ** buf, uint32_t maxs,
  dds_sample_info_t * si, uint32_t mask
)
{
  return dds_read_impl (true, rd, buf, maxs, si, mask, NULL, DDS_HANDLE_NIL);
}

int dds_takecdr
(
 dds_entity_t rd, struct serdata ** buf, uint32_t maxs,
 dds_sample_info_t * si, uint32_t mask
 )
{
  return dds_readcdr_impl (true, rd, buf, maxs, si, mask, NULL, DDS_HANDLE_NIL);
}

int dds_take_instance
(
  dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si,
  dds_instance_handle_t hand, uint32_t mask
)
{
  assert (hand != DDS_HANDLE_NIL);
  return dds_read_impl (true, rd, buf, maxs, si, mask, NULL, hand);
}

int dds_take_cond
(
  dds_entity_t rd, void ** buf, uint32_t maxs,
  dds_sample_info_t * si, dds_condition_t cond
)
{
  assert (cond);
  return dds_read_impl (true, rd, buf, maxs, si, 0, cond, DDS_HANDLE_NIL);
}

int dds_take_next (dds_entity_t rd, void ** buf, dds_sample_info_t * si)
{
  uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ANY_INSTANCE_STATE;
  return dds_read_impl (true, rd, buf, 1u, si, mask, NULL, DDS_HANDLE_NIL);
}

void dds_return_loan (dds_entity_t reader, void ** buf, uint32_t count)
{
  uint32_t i;
  uint32_t ret;
  const dds_topic_descriptor_t * desc;
  dds_reader * rd;

  assert (buf);

  ret = dds_reader_lock(reader, &rd);
  if (ret == DDS_RETCODE_OK) {
      desc = rd->m_topic->m_descriptor;

      /* Only free sample contents if may have been allocated */

      if (desc->m_flagset & DDS_TOPIC_NO_OPTIMIZE)
      {
        for (i = 0; i < count; i++)
        {
          dds_sample_free (buf[i], desc, DDS_FREE_CONTENTS);
        }
      }

      /* If possible return loan buffer to reader */

      if (buf[0] == rd->m_loan)
      {
        rd->m_loan_out = false;
        memset (rd->m_loan, 0, rd->m_loan_size);
        buf[0] = NULL;
      }
      dds_reader_unlock(rd);
  }
  dds_free (buf[0]);
}