#include <stddef.h>

#include "os/os.h"
#include "ddsi/sysdeps.h"
#include "ddsi/q_freelist.h"

#if FREELIST_TYPE == FREELIST_ATOMIC_LIFO

void nn_freelist_init (struct nn_freelist *fl, uint32_t max, off_t linkoff)
{
  os_atomic_lifo_init (&fl->x);
  os_atomic_st32(&fl->count, 0);
  fl->max = (max == UINT32_MAX) ? max-1 : max;
  fl->linkoff = linkoff;
}

void nn_freelist_fini (struct nn_freelist *fl, void (*free) (void *))
{
  void *e;
  while ((e = os_atomic_lifo_pop (&fl->x, fl->linkoff)) != NULL)
    free (e);
}

bool nn_freelist_push (struct nn_freelist *fl, void *elem)
{
  if (os_atomic_inc32_nv (&fl->count) <= fl->max)
  {
    os_atomic_lifo_push (&fl->x, elem, fl->linkoff);
    return true;
  }
  else
  {
    os_atomic_dec32 (&fl->count);
    return false;
  }
}

void *nn_freelist_pushmany (struct nn_freelist *fl, void *first, void *last, uint32_t n)
{
  os_atomic_add32 (&fl->count, n);
  os_atomic_lifo_pushmany (&fl->x, first, last, fl->linkoff);
  return NULL;
}

void *nn_freelist_pop (struct nn_freelist *fl)
{
  void *e;
  if ((e = os_atomic_lifo_pop (&fl->x, fl->linkoff)) != NULL)
  {
    os_atomic_dec32(&fl->count);
    return e;
  }
  else
  {
    return NULL;
  }
}

#elif FREELIST_TYPE == FREELIST_DOUBLE

#if OS_HAS_TSD_USING_THREAD_KEYWORD
static __thread int freelist_inner_idx = -1;
#elif defined _WIN32
static __declspec(thread) int freelist_inner_idx = -1;
#endif

void nn_freelist_init (struct nn_freelist *fl, uint32_t max, off_t linkoff)
{
  int i;
  os_mutexInit (&fl->lock, NULL);
  for (i = 0; i < NN_FREELIST_NPAR; i++)
  {
    os_mutexInit (&fl->inner[i].lock, NULL);
    fl->inner[i].count = 0;
    fl->inner[i].m = os_malloc (sizeof (*fl->inner[i].m));
  }
  os_atomic_st32 (&fl->cc, 0);
  fl->mlist = NULL;
  fl->emlist = NULL;
  fl->count = 0;
  fl->max = (max == UINT32_MAX) ? max-1 : max;
  fl->linkoff = linkoff;
}

static void *get_next (const struct nn_freelist *fl, const void *e)
{
  return *((void **) ((char *)e + fl->linkoff));
}

void nn_freelist_fini (struct nn_freelist *fl, void (*xfree) (void *))
{
  int i, j;
  struct nn_freelistM *m;
  os_mutexDestroy (&fl->lock);
  for (i = 0; i < NN_FREELIST_NPAR; i++)
  {
    os_mutexDestroy (&fl->inner[i].lock);
    for (j = 0; j < fl->inner[i].count; j++)
      xfree (fl->inner[i].m->x[j]);
  }
  while ((m = fl->mlist) != NULL)
  {
    fl->mlist = m->next;
    for (j = 0; j < NN_FREELIST_MAGSIZE; j++)
      xfree (m->x[j]);
    os_free (m);
  }
  while ((m = fl->emlist) != NULL)
  {
    fl->emlist = m->next;
    os_free (m);
  }
}

static os_atomic_uint32_t freelist_inner_idx_off = OS_ATOMIC_UINT32_INIT(0);

static int get_freelist_inner_idx (void)
{
  if (freelist_inner_idx == -1)
  {
    static const uint64_t unihashconsts[] = {
      UINT64_C (16292676669999574021),
      UINT64_C (10242350189706880077),
    };
    uintptr_t addr;
    uint64_t t = (uint64_t) ((uintptr_t) &addr) + os_atomic_ld32 (&freelist_inner_idx_off);
    freelist_inner_idx = (int) (((((uint32_t) t + unihashconsts[0]) * ((uint32_t) (t >> 32) + unihashconsts[1]))) >> (64 - NN_FREELIST_NPAR_LG2));
  }
  return freelist_inner_idx;
}

static int lock_inner (struct nn_freelist *fl)
{
  int k = get_freelist_inner_idx();
  if (os_mutexTryLock (&fl->inner[k].lock) != os_resultSuccess)
  {
    os_mutexLock (&fl->inner[k].lock);
    if (os_atomic_inc32_nv (&fl->cc) == 100)
    {
      os_atomic_st32(&fl->cc, 0);
      os_atomic_inc32(&freelist_inner_idx_off);
      freelist_inner_idx = -1;
    }
  }
  return k;
}

bool nn_freelist_push (struct nn_freelist *fl, void *elem)
{
  int k = lock_inner (fl);
  if (fl->inner[k].count < NN_FREELIST_MAGSIZE)
  {
    fl->inner[k].m->x[fl->inner[k].count++] = elem;
    os_mutexUnlock (&fl->inner[k].lock);
    return true;
  }
  else
  {
    struct nn_freelistM *m = fl->inner[k].m;
    os_mutexLock (&fl->lock);
    if (fl->count + NN_FREELIST_MAGSIZE >= fl->max)
    {
      os_mutexUnlock (&fl->lock);
      os_mutexUnlock (&fl->inner[k].lock);
      return false;
    }
    m->next = fl->mlist;
    fl->mlist = m;
    fl->count += NN_FREELIST_MAGSIZE;
    fl->inner[k].count = 0;
    if (fl->emlist == NULL)
      fl->inner[k].m = os_malloc (sizeof (*fl->inner[k].m));
    else
    {
      fl->inner[k].m = fl->emlist;
      fl->emlist = fl->emlist->next;
    }
    os_mutexUnlock (&fl->lock);
    fl->inner[k].m->x[fl->inner[k].count++] = elem;
    os_mutexUnlock (&fl->inner[k].lock);
    return true;
  }
}

void *nn_freelist_pushmany (struct nn_freelist *fl, void *first, void *last, uint32_t n)
{
  void *m = first;
  while (m)
  {
    void *mnext = get_next (fl, m);
    nn_freelist_push (fl, m);
    m = mnext;
  }
  return m;
}

void *nn_freelist_pop (struct nn_freelist *fl)
{
  int k = lock_inner (fl);
  if (fl->inner[k].count > 0)
  {
    void *e = fl->inner[k].m->x[--fl->inner[k].count];
    os_mutexUnlock (&fl->inner[k].lock);
    return e;
  }
  else
  {
    os_mutexLock (&fl->lock);
    if (fl->mlist == NULL)
    {
      os_mutexUnlock (&fl->lock);
      os_mutexUnlock (&fl->inner[k].lock);
      return NULL;
    }
    else
    {
      void *e;
      fl->inner[k].m->next = fl->emlist;
      fl->emlist = fl->inner[k].m;
      fl->inner[k].m = fl->mlist;
      fl->mlist = fl->mlist->next;
      fl->count -= NN_FREELIST_MAGSIZE;
      os_mutexUnlock (&fl->lock);
      fl->inner[k].count = NN_FREELIST_MAGSIZE;
      e = fl->inner[k].m->x[--fl->inner[k].count];
      os_mutexUnlock (&fl->inner[k].lock);
      return e;
    }
  }
}

#endif
