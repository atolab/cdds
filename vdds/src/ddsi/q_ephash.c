/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <stddef.h>
#include <assert.h>

#include "os/os.h"
#include "ddsi/sysdeps.h"

#include "util/ut_avl.h"
#include "ddsi/q_ephash.h"
#include "ddsi/q_config.h"
#include "ddsi/q_globals.h"
#include "ddsi/q_entity.h"
#include "ddsi/q_rtps.h" /* guid_t */
#include "ddsi/q_thread.h" /* for assert(thread is awake) */


#define CONTAINER_OF(ptr, type, member) ((type *) ((char *) (ptr) - offsetof (type, member)))

struct ephash {
  os_mutex lock;
  int nbitskey;
  struct ephash_chain_entry **heads;

  /* Separate lists for enumerating are supposed to be here just
     temporarily, to be replaced when all this gets changed into a
     proper lock-free, wait-free, auto-resizing hash table. */
  struct ephash_chain_entry *enum_lists[EK_NKINDS];
  /* We track live enumerations, so we can avoid restarting on
     deletes */
  struct ephash_enum *live_enums;
};

static const uint64_t unihashconsts[] = {
  UINT64_C (16292676669999574021),
  UINT64_C (10242350189706880077),
  UINT64_C (12844332200329132887),
  UINT64_C (16728792139623414127)
};

static void ephash_update_enums_on_delete (struct ephash *h, struct ephash_chain_entry *ce);


static int hash_guid (const struct nn_guid *guid, int nbitskey)
{
  return
    (int) (((((uint32_t) guid->prefix.u[0] + unihashconsts[0]) *
             ((uint32_t) guid->prefix.u[1] + unihashconsts[1])) +
            (((uint32_t) guid->prefix.u[2] + unihashconsts[2]) *
             ((uint32_t) guid->entityid.u  + unihashconsts[3])))
           >> (64 - nbitskey));
}


static int guid_eq (const struct nn_guid *a, const struct nn_guid *b)
{
  return
    a->prefix.u[0] == b->prefix.u[0] && a->prefix.u[1] == b->prefix.u[1] &&
    a->prefix.u[2] == b->prefix.u[2] && a->entityid.u == b->entityid.u;
}

struct ephash *ephash_new (uint32_t soft_limit)
{
  struct ephash *ephash;
  uint32_t limit;
  int nbitskey;
  unsigned i, init_size;

  /* We use soft_limit to compute hash table size; 70% occupancy
     supposedly is ok so (3/2) * soft_limit should be okay-ish. We
     want a power of two, and hence compute:
     floor(log2(2*(3*soft_limit/2))) */
  assert (sizeof (nbitskey) >= sizeof (soft_limit));
  assert (soft_limit < (1 << 28));
  limit = 3 * soft_limit / 2;
  nbitskey = 0;
  while (limit)
  {
    limit >>= 1;
    nbitskey++;
  }
  init_size = 1u << nbitskey;
  TRACE (("ephash_new: soft_limit %u nbitskey %d init_size %d l.f. %f\n", soft_limit, nbitskey, init_size, (double) soft_limit / init_size));
  ephash = os_malloc (sizeof (*ephash));
  if (os_mutexInit (&ephash->lock, NULL) != os_resultSuccess)
    goto fail_mutex;
  ephash->nbitskey = nbitskey;
  if ((ephash->heads = os_malloc (init_size * sizeof (*ephash->heads))) == NULL)
    goto fail_heads;
  for (i = 0; i < init_size; i++)
    ephash->heads[i] = NULL;
  for (i = 0; i < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])); i++)
    ephash->enum_lists[i] = NULL;
  ephash->live_enums = NULL;
  return ephash;
 fail_heads:
  os_mutexDestroy (&ephash->lock);
 fail_mutex:
  os_free (ephash);
  return NULL;
}

void ephash_free (struct ephash *ephash)
{
  assert (ephash->live_enums == NULL);
  os_free (ephash->heads);
  os_mutexDestroy (&ephash->lock);
  os_free (ephash);
}

static void ephash_insert (struct ephash *ephash, int idx, struct ephash_chain_entry *ce, int listidx)
{
  assert (0 <= idx && idx < (1 << ephash->nbitskey));
  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  ce->prev = NULL;
  os_mutexLock (&ephash->lock);
  ce->next = ephash->heads[idx];
  if (ce->next)
    ce->next->prev = ce;
  /* pa_membar ensures that the transmit and receive threads will see
     a properly linked hash chain */
  os_atomic_fence_rel ();
  ephash->heads[idx] = ce;

  /* enumerate support (temporary ... I hope) */
  ce->enum_next = ephash->enum_lists[listidx];
  if (ce->enum_next)
    ce->enum_next->enum_prev = ce;
  ce->enum_prev = NULL;
  ephash->enum_lists[listidx] = ce;
  os_mutexUnlock (&ephash->lock);
}

static void ephash_remove (struct ephash *ephash, int idx, struct ephash_chain_entry *ce, int listidx)
{
  /* removing a local object from the hash chain must:
     (1) prevent any subsequent lookups from finding the lobj
     (2) allow any parallel lookups to keep walking the chain
     therefore, obj->hash_next of the removed obj must not be
     changed until no further parallel lookups may need to touch the
     obj */
  assert (0 <= idx && idx < (1 << ephash->nbitskey));
  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  os_mutexLock (&ephash->lock);
  if (ce->next)
    ce->next->prev = ce->prev;
  if (ce->prev)
    ce->prev->next = ce->next;
  else
    ephash->heads[idx] = ce->next;

  /* enumerate support (temporary ... I hope) */
  if (ce->enum_next)
    ce->enum_next->enum_prev = ce->enum_prev;
  if (ce->enum_prev)
    ce->enum_prev->enum_next = ce->enum_next;
  else
    ephash->enum_lists[listidx] = ce->enum_next;
  ephash_update_enums_on_delete (ephash, ce);
  os_mutexUnlock (&ephash->lock);
}

/* GUID-based */

static void ephash_guid_insert (struct entity_common *e)
{
  ephash_insert (gv.guid_hash, hash_guid (&e->guid, gv.guid_hash->nbitskey), &e->guid_hash_chain, (int) e->kind);
}

static void ephash_guid_remove (struct entity_common *e)
{
  ephash_remove (gv.guid_hash, hash_guid (&e->guid, gv.guid_hash->nbitskey), &e->guid_hash_chain, (int) e->kind);
}

static void *ephash_lookup_guid (const struct ephash *ephash, const struct nn_guid *guid, enum entity_kind kind)
{
  struct ephash_chain_entry *ce;
  int idx = hash_guid (guid, ephash->nbitskey);

  assert (idx >= 0 && idx < (1 << ephash->nbitskey));
  for (ce = ephash->heads[idx]; ce; ce = ce->next)
  {
    struct entity_common *e = CONTAINER_OF (ce, struct entity_common, guid_hash_chain);
    if (guid_eq (guid, &e->guid) && (e->kind == kind))
    {
      return e;
    }
  }
  return NULL;
}

void ephash_insert_participant_guid (struct participant *pp)
{
  ephash_guid_insert (&pp->e);
}

void ephash_insert_proxy_participant_guid (struct proxy_participant *proxypp)
{
  ephash_guid_insert (&proxypp->e);
}

void ephash_insert_writer_guid (struct writer *wr)
{
  ephash_guid_insert (&wr->e);
}

void ephash_insert_reader_guid (struct reader *rd)
{
  ephash_guid_insert (&rd->e);
}

void ephash_insert_proxy_writer_guid (struct proxy_writer *pwr)
{
  ephash_guid_insert (&pwr->e);
}

void ephash_insert_proxy_reader_guid (struct proxy_reader *prd)
{
  ephash_guid_insert (&prd->e);
}

void ephash_remove_participant_guid (struct participant *pp)
{
  ephash_guid_remove (&pp->e);
}

void ephash_remove_proxy_participant_guid (struct proxy_participant *proxypp)
{
  ephash_guid_remove (&proxypp->e);
}

void ephash_remove_writer_guid (struct writer *wr)
{
  ephash_guid_remove (&wr->e);
}

void ephash_remove_reader_guid (struct reader *rd)
{
  ephash_guid_remove (&rd->e);
}

void ephash_remove_proxy_writer_guid (struct proxy_writer *pwr)
{
  ephash_guid_remove (&pwr->e);
}

void ephash_remove_proxy_reader_guid (struct proxy_reader *prd)
{
  ephash_guid_remove (&prd->e);
}

struct participant *ephash_lookup_participant_guid (const struct nn_guid *guid)
{
  assert (guid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (offsetof (struct participant, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PARTICIPANT);
}

struct proxy_participant *ephash_lookup_proxy_participant_guid (const struct nn_guid *guid)
{
  assert (guid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (offsetof (struct proxy_participant, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_PARTICIPANT);
}

struct writer *ephash_lookup_writer_guid (const struct nn_guid *guid)
{
  assert (is_writer_entityid (guid->entityid));
  assert (offsetof (struct writer, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_WRITER);
}

struct reader *ephash_lookup_reader_guid (const struct nn_guid *guid)
{
  assert (is_reader_entityid (guid->entityid));
  assert (offsetof (struct reader, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_READER);
}

struct proxy_writer *ephash_lookup_proxy_writer_guid (const struct nn_guid *guid)
{
  assert (is_writer_entityid (guid->entityid));
  assert (offsetof (struct proxy_writer, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_WRITER);
}

struct proxy_reader *ephash_lookup_proxy_reader_guid (const struct nn_guid *guid)
{
  assert (is_reader_entityid (guid->entityid));
  assert (offsetof (struct proxy_reader, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_READER);
}


/* Enumeration */

static void ephash_update_enums_on_delete (struct ephash *ephash, struct ephash_chain_entry *ce)
{
  struct ephash_enum *st;
  ASSERT_MUTEX_HELD (h->lock);
  for (st = ephash->live_enums; st; st = st->next_live)
    if (st->cursor == ce)
      st->cursor = ce->enum_next;
}

static void ephash_enum_init (struct ephash_enum *st, struct ephash *ephash, enum entity_kind kind)
{
  const int listidx = (int) kind;

  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  os_mutexLock (&ephash->lock);
  st->ephash = ephash;
  st->next_live = ephash->live_enums;
  st->prev_live = NULL;
  if (st->next_live)
    st->next_live->prev_live = st;
  ephash->live_enums = st;
  st->cursor = ephash->enum_lists[listidx];
  os_mutexUnlock (&ephash->lock);
}

void ephash_enum_writer_init (struct ephash_enum_writer *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_WRITER);
}

void ephash_enum_reader_init (struct ephash_enum_reader *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_READER);
}

void ephash_enum_proxy_writer_init (struct ephash_enum_proxy_writer *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_WRITER);
}

void ephash_enum_proxy_reader_init (struct ephash_enum_proxy_reader *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_READER);
}

void ephash_enum_participant_init (struct ephash_enum_participant *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PARTICIPANT);
}

void ephash_enum_proxy_participant_init (struct ephash_enum_proxy_participant *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_PARTICIPANT);
}

static void *ephash_perform_enum (struct ephash_enum *st)
{
  void *x;
  /* should be lock-free -- but this takes less development time */
  os_mutexLock (&st->ephash->lock);
  if (st->cursor == NULL)
    x = NULL;
  else
  {
    x = CONTAINER_OF (st->cursor, struct entity_common, guid_hash_chain);
    st->cursor = st->cursor->enum_next;
  }
  os_mutexUnlock (&st->ephash->lock);
  return x;
}

struct writer *ephash_enum_writer_next (struct ephash_enum_writer *st)
{
  assert (offsetof (struct writer, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct reader *ephash_enum_reader_next (struct ephash_enum_reader *st)
{
  assert (offsetof (struct reader, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_writer *ephash_enum_proxy_writer_next (struct ephash_enum_proxy_writer *st)
{
  assert (offsetof (struct proxy_writer, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_reader *ephash_enum_proxy_reader_next (struct ephash_enum_proxy_reader *st)
{
  assert (offsetof (struct proxy_reader, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct participant *ephash_enum_participant_next (struct ephash_enum_participant *st)
{
  assert (offsetof (struct participant, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_participant *ephash_enum_proxy_participant_next (struct ephash_enum_proxy_participant *st)
{
  assert (offsetof (struct proxy_participant, e) == 0);
  return ephash_perform_enum (&st->st);
}

static void ephash_enum_fini (struct ephash_enum *st)
{
  struct ephash *ephash = st->ephash;

  os_mutexLock (&ephash->lock);
  if (st->next_live)
  {
    st->next_live->prev_live = st->prev_live;
  }
  if (st->prev_live)
  {
    st->prev_live->next_live = st->next_live;
  }
  else
  {
    ephash->live_enums = st->next_live;
  }
  os_mutexUnlock (&ephash->lock);
}

void ephash_enum_writer_fini (struct ephash_enum_writer *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_reader_fini (struct ephash_enum_reader *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_writer_fini (struct ephash_enum_proxy_writer *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_reader_fini (struct ephash_enum_proxy_reader *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_participant_fini (struct ephash_enum_participant *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_participant_fini (struct ephash_enum_proxy_participant *st)
{
  ephash_enum_fini (&st->st);
}
