#include <assert.h>
#include <string.h>
#include "os/os.h"
#include "kernel/dds_key.h"
#include "kernel/dds_tkmap.h"
#include "kernel/dds_stream.h"
#include "ddsi/q_bswap.h"
#include "kernel/q_osplser.h"

serdata_t serialize (serstatepool_t pool, const struct sertopic * tp, const void * sample)
{
  dds_stream_t os;
  serstate_t st = ddsi_serstate_new (pool, tp);

  if (tp->nkeys)
  {
    dds_key_gen ((const dds_topic_descriptor_t*) tp->type, &st->data->v.keyhash, (char*) sample);
  }
  dds_stream_from_serstate (&os, st);
  dds_stream_write_sample (&os, sample, tp);
  dds_stream_add_to_serstate (&os, st);
  return st->data;
}

int serdata_cmp (const struct serdata *a, const struct serdata *b)
{
  /* First compare on topic */

  if (a->v.st->topic != b->v.st->topic)
  {
    return a->v.st->topic < b->v.st->topic ? -1 : 1;
  }

  /* Samples with a keyless topic map to the default instance */

  if 
  (
    (a->v.st->topic) &&
    (((dds_topic_descriptor_t*) a->v.st->topic->type)->m_keys == 0)
  )
  {
    return 0;
  }

  /* Check key has been hashed */

  assert (a->v.keyhash.m_flags & DDS_KEY_HASH_SET);

  /* Compare by hash */

  return memcmp (a->v.keyhash.m_hash, b->v.keyhash.m_hash, 16);
}

uint32_t serdata_hash (const struct serdata *a)
{
  static const uint64_t c[] = {
    UINT64_C (16292676669999574021),
    UINT64_C (10242350189706880077),
    UINT64_C (12844332200329132887),
    UINT64_C (16728792139623414127)
  };
  const uint32_t *kh;

  /* Samples with a keyless topic map to the default instance */

  if
    (
     (a->v.st->topic) &&
     (((dds_topic_descriptor_t*) a->v.st->topic->type)->m_keys == 0)
     )
  {
    return 0;
  }

  /* Check key has been hashed */

  assert (a->v.keyhash.m_flags & DDS_KEY_HASH_SET);

  /* Compare by hash */
  kh = (const uint32_t *) a->v.keyhash.m_hash;
  return (uint32_t) (((((uint32_t) kh[0] + c[0]) * ((uint32_t) kh[1] + c[1])) +
                      (((uint32_t) kh[2] + c[2]) * ((uint32_t) kh[3] + c[3])))
                     >> 32);
}

serdata_t serialize_key (serstatepool_t pool, const struct sertopic * tp, const void * sample)
{
  serdata_t sd;
  if (tp->nkeys)
  {
    dds_stream_t os;
    dds_topic_descriptor_t * desc = (dds_topic_descriptor_t*) tp->type;
    serstate_t st = ddsi_serstate_new (pool, tp);
    dds_key_gen (desc, &st->data->v.keyhash, (char*) sample);
    dds_stream_from_serstate (&os, st);
    dds_stream_write_key (&os, sample, desc);
    dds_stream_add_to_serstate (&os, st);
    sd = st->data;
  }
  else
  {
    sd = serialize (pool, tp, sample);
  }
  sd->v.st->kind = STK_KEY;
  return sd;
}

void deserialize_into (void * sample, const struct serdata * serdata)
{
  const serstate_t st = serdata->v.st;
  dds_stream_t is;

  dds_stream_from_serstate (&is, st);
  if (st->kind == STK_KEY)
  {
    dds_stream_read_key (&is, sample, (const dds_topic_descriptor_t*) st->topic->type);
  }
  else
  {
    dds_stream_read_sample (&is, sample, st->topic);
  }
}

void serstate_set_key (serstate_t st, int justkey, const void *key)
{
  st->kind = justkey ? STK_KEY : STK_DATA;
  memcpy (&st->data->v.keyhash.m_hash, key, 16);
  st->data->v.keyhash.m_flags = DDS_KEY_SET | DDS_KEY_HASH_SET | DDS_KEY_IS_HASH;
  st->data->v.keyhash.m_key_len = 16;
}

void serstate_init (serstate_t st, const struct sertopic * topic)
{
  st->pos = 0;
  st->topic = topic;
  st->kind = STK_DATA;
  st->twrite.v = -1;
  os_atomic_st32 (&st->refcount, 1);

  if (topic)
  {
    os_atomic_inc32 (&(((struct sertopic *) topic)->refcount));
  }

  st->data->hdr.identifier = topic ? 
    (PLATFORM_IS_LITTLE_ENDIAN ? CDR_LE : CDR_BE) : 
    (PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE);

  st->data->v.hash_valid = (topic == NULL || topic->nkeys) ? 0 : 1;
  st->data->v.hash = 0;
  st->data->v.bswap = false;
  memset (st->data->v.keyhash.m_hash, 0, sizeof (st->data->v.keyhash.m_hash));
  st->data->v.keyhash.m_key_len = 0;
  st->data->v.keyhash.m_flags = 0;
}

void serstate_free (serstate_t st)
{
#if ! USE_ATOMIC_LIFO
#ifndef NDEBUG
  {
    serstate_t b;
    for (b = st->pool->freelist; b && b != st; b = b->next);
    assert (b == NULL);
  }
#endif
#endif
  dds_free (st->data->v.keyhash.m_key_buff);
  dds_free (st->data);
  dds_free (st);
}

void sertopic_free (struct sertopic * tp)
{
  if (tp && (os_atomic_dec32_nv (&tp->refcount) == 0))
  {
    dds_free (tp->name);
    dds_free (tp->typename);
    dds_free (tp->name_typename);
    dds_sample_free (tp->filter_sample, (const struct dds_topic_descriptor *) tp->type, DDS_FREE_ALL);
    dds_free (tp);
  }
}