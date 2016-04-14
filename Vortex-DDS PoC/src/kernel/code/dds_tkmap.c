#include <assert.h>
#include <string.h>
#include "q_thread.h"
#include "q_unused.h"
#include "q_gc.h"
#include "q_globals.h"
#include "q_config.h"
#include "sysdeps.h"
#include "dds_tkmap.h"
#include "dds_iid.h"
#include "ut_hopscotch.h"
#include "dds_stream.h"
#include "os.h"
#include "q_osplser.h"

struct tkmap
{
  struct ut_hh * m_hh;
  os_mutex m_lock;
  os_atomic_uint32_t m_refc;
};

/* Fixed seed and length */

#define DDS_MH3_LEN 16
#define DDS_MH3_SEED 0

#define DDS_MH3_ROTL32(x,r) (((x) << (r)) | ((x) >> (32 - (r))))

/* Really
 http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp,
 MurmurHash3_x86_32
*/

DDS_INLINE static uint32_t dds_mh3 (const void * key)
{
  const uint8_t *data = (const uint8_t *) key;
  const intptr_t nblocks = (intptr_t) (DDS_MH3_LEN / 4);
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  uint32_t h1 = DDS_MH3_SEED;

  const uint32_t *blocks = (const uint32_t *) (data + nblocks * 4);
  register intptr_t i;

  for (i = -nblocks; i; i++)
  {
    uint32_t k1 = blocks[i];

    k1 *= c1;
    k1 = DDS_MH3_ROTL32 (k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = DDS_MH3_ROTL32 (h1, 13);
    h1 = h1 * 5+0xe6546b64;
  }

  /* finalization */

  h1 ^= DDS_MH3_LEN;
  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  return h1;
}

static uint32_t dds_tk_hash (const struct tkmap_instance * inst)
{
  volatile struct serdata * sd = (volatile struct serdata *) inst->m_sample;

  if (! sd->v.hash_valid)
  {
    const uint32_t * k = (const uint32_t *) sd->v.keyhash.m_hash;
    sd->v.hash = ((sd->v.keyhash.m_flags & DDS_KEY_IS_HASH) ? dds_mh3 (k) : (*k)) ^ sd->v.st->topic->hash;
    sd->v.hash_valid = 1;
  }

  return sd->v.hash;
}

static uint32_t dds_tk_hash_void (const void * inst)
{
  return dds_tk_hash (inst);
}

static int dds_tk_equals (const struct tkmap_instance *a, const struct tkmap_instance *b)
{
  return serdata_cmp (a->m_sample, b->m_sample) == 0;
}

static int dds_tk_equals_void (const void *a, const void *b)
{
  return dds_tk_equals (a, b);
}

struct tkmap * dds_tkmap_new (void)
{
  struct tkmap *tkmap = dds_alloc (sizeof (*tkmap));
  tkmap->m_hh = ut_hhNew (1, dds_tk_hash_void, dds_tk_equals_void);
  os_mutexInit (&tkmap->m_lock, NULL);
  os_atomic_st32 (&tkmap->m_refc, 1);
  return tkmap;
}

void dds_tkmap_free (struct tkmap * map)
{
  if (os_atomic_dec32_nv (&map->m_refc) == 0)
  {
    ut_hhFree (map->m_hh);
    os_mutexDestroy (&map->m_lock);
    dds_free (map);
  }
}

uint64_t dds_tkmap_lookup (struct tkmap * map, struct serdata * sd)
{
  struct tkmap_instance dummy;
  struct tkmap_instance * tk;

  os_mutexLock (&map->m_lock);
  dummy.m_sample = sd;
  tk = ut_hhLookup (map->m_hh, &dummy);
  os_mutexUnlock (&map->m_lock);
  return (tk) ? tk->m_iid : DDS_HANDLE_NIL;
}

typedef struct
{
  uint64_t m_iid;
  void * m_sample;
  bool m_ret;
}
tkmap_get_key_arg;

static void dds_tkmap_get_key_fn (void * vtk, void * varg)
{
  struct tkmap_instance * tk = vtk;
  tkmap_get_key_arg * arg = (tkmap_get_key_arg*) varg;
  if (tk->m_iid == arg->m_iid)
  {
    deserialize_into (arg->m_sample, tk->m_sample);
    arg->m_ret = true;
  }
}

bool dds_tkmap_get_key (struct tkmap * map, uint64_t iid, void * sample)
{
  tkmap_get_key_arg arg = { iid, sample, false };
  os_mutexLock (&map->m_lock);
  ut_hhEnum (map->m_hh, dds_tkmap_get_key_fn, &arg);
  os_mutexUnlock (&map->m_lock);
  return arg.m_ret;
}

typedef struct
{
  uint64_t m_iid;
  struct tkmap_instance * m_inst;
}
tkmap_get_inst_arg;

static void dds_tkmap_get_inst_fn (void * vtk, void * varg)
{
  struct tkmap_instance * tk = vtk;
  tkmap_get_inst_arg * arg = (tkmap_get_inst_arg*) varg;
  if (tk->m_iid == arg->m_iid)
  {
    arg->m_inst = tk;;
  }
}

struct tkmap_instance * dds_tkmap_find_by_id (struct tkmap * map, uint64_t iid)
{
  tkmap_get_inst_arg arg = { iid, NULL };
  os_mutexLock (&map->m_lock);
  ut_hhEnum (map->m_hh, dds_tkmap_get_inst_fn, &arg);
  os_mutexUnlock (&map->m_lock);
  return arg.m_inst;
}

/* Debug keyhash generation for debug and coverage builds */

#ifdef NDEBUG
#if VL_BUILD_LCOV
#define DDS_DEBUG_KEYHASH 1
#else
#define DDS_DEBUG_KEYHASH 0
#endif
#else
#define DDS_DEBUG_KEYHASH 1
#endif

struct tkmap_instance * dds_tkmap_find
(
  const struct dds_topic * topic,
  struct serdata * sd,
  const bool rd,
  const bool create
)
{
  struct tkmap_instance dummy;
  struct tkmap_instance * tk;
  struct tkmap * map = (rd) ? topic->m_entity.m_domain->m_rd_tkmap : topic->m_entity.m_domain->m_wr_tkmap;

  dummy.m_sample = sd;

  /* Generate key hash if required and not provided */

  if (topic->m_descriptor->m_nkeys)
  {
    if ((sd->v.keyhash.m_flags & DDS_KEY_HASH_SET) == 0)
    {
      dds_stream_t is;
      dds_stream_from_serstate (&is, sd->v.st);
      dds_stream_read_keyhash (&is, &sd->v.keyhash, topic->m_descriptor, sd->v.st->kind == STK_KEY);
    }
    else
    {
      if (topic->m_descriptor->m_flagset & DDS_TOPIC_FIXED_KEY)
      {
        sd->v.keyhash.m_flags |= DDS_KEY_IS_HASH;
      }

#if DDS_DEBUG_KEYHASH

      {
        dds_stream_t is;
        dds_key_hash_t kh;

        /* Check that we generate same keyhash as provided */

        memset (&kh, 0, sizeof (kh));
        dds_stream_from_serstate (&is, sd->v.st);
        dds_stream_read_keyhash (&is, &kh, topic->m_descriptor, sd->v.st->kind == STK_KEY);
        assert (memcmp (kh.m_hash, sd->v.keyhash.m_hash, 16) == 0);
        if (kh.m_key_buff_size)
        {
          dds_free (kh.m_key_buff);
        }
      }
#endif
    }
  }

  os_mutexLock (&map->m_lock);
  tk = ut_hhLookup (map->m_hh, &dummy);
  if (tk)
  {
    if (rd)
    {
      os_atomic_inc32 (&tk->m_refc);
    }
  }
  else
  {
    if (create)
    {
      tk = dds_alloc (sizeof (*tk));
      tk->m_sample = ddsi_serdata_ref (sd);
      tk->m_map = map;
      os_atomic_st32 (&tk->m_refc, 1);
      os_atomic_inc32 (&map->m_refc);
      tk->m_iid = dds_iid_gen ();
      ut_hhAdd (map->m_hh, tk);
    }
  }
  os_mutexUnlock (&map->m_lock);

  if (tk && rd)
  {
    TRACE (("tk=%p iid=%"PRIx64"", &tk, tk->m_iid));
  }
  return tk;
}

struct tkmap_instance * dds_tkmap_lookup_instance_ref (struct serdata * sd)
{
  dds_topic * topic = sd->v.st->topic->status_cb_entity;

  assert (vtime_awake_p (lookup_thread_state ()->vtime));

  /* Topic might have been deleted */

  if (topic == NULL)
  {
    return NULL;
  }
  return dds_tkmap_find (topic, sd, true, true);
}

void dds_tkmap_instance_ref (struct tkmap_instance *tk)
{
  os_atomic_inc32 (&tk->m_refc);
}

void dds_tkmap_instance_unref (struct tkmap_instance * tk)
{
  if (os_atomic_dec32_nv (&tk->m_refc) == 0)
  {
    /* Remove from hash table */

    os_mutexLock (&tk->m_map->m_lock);
    ut_hhRemove (tk->m_map->m_hh, tk);
    os_mutexUnlock (&tk->m_map->m_lock);
    ddsi_serdata_unref (tk->m_sample);
    dds_tkmap_free (tk->m_map);
    dds_free (tk);
  }
}
