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

/* x86 has supported 64-bit CAS for a long time, so Windows ought to
   provide all the interlocked operations for 64-bit operands on x86
   platforms, but it doesn't. */

#undef OS_API
#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined OS_64BIT
#define OS_ATOMIC64_SUPPORT 1
#else
#define OS_ATOMIC64_SUPPORT 0
#endif

#if defined OS_64BIT
#define OS_ATOMIC_PTROP(name) name##64
#else
#define OS_ATOMIC_PTROP(name) name
#endif

#if ! OS_ATOMICS_OMIT_FUNCTIONS

/* Experience is that WinCE doesn't provide these, and that neither does VS8 */
#if ! defined OS_WINCE_DEFS_H && _MSC_VER > 1400
#if defined _M_IX86 || defined _M_ARM
#define OS_ATOMIC_INTERLOCKED_AND _InterlockedAnd
#define OS_ATOMIC_INTERLOCKED_OR _InterlockedOr
#define OS_ATOMIC_INTERLOCKED_AND64 _InterlockedAnd64
#define OS_ATOMIC_INTERLOCKED_OR64 _InterlockedOr64
#else
#define OS_ATOMIC_INTERLOCKED_AND InterlockedAnd
#define OS_ATOMIC_INTERLOCKED_OR InterlockedOr
#define OS_ATOMIC_INTERLOCKED_AND64 InterlockedAnd64
#define OS_ATOMIC_INTERLOCKED_OR64 InterlockedOr64
#endif
#endif

#if OS_ATOMIC_HAVE_INLINE
#define OS_ATOMIC_API_INLINE OS_ATOMIC_INLINE
#else
#define OS_ATOMIC_API_INLINE OS_API
#endif

/* LD, ST */

OS_ATOMIC_API_INLINE uint32_t os_atomic_ld32 (const volatile os_atomic_uint32_t *x) { return x->v; }
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_ld64 (const volatile os_atomic_uint64_t *x) { return x->v; }
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_ldptr (const volatile os_atomic_uintptr_t *x) { return x->v; }
OS_ATOMIC_API_INLINE void *os_atomic_ldvoidp (const volatile os_atomic_voidp_t *x) { return (void *) os_atomic_ldptr (x); }

OS_ATOMIC_API_INLINE void os_atomic_st32 (volatile os_atomic_uint32_t *x, uint32_t v) { x->v = v; }
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_st64 (volatile os_atomic_uint64_t *x, uint64_t v) { x->v = v; }
#endif
OS_ATOMIC_API_INLINE void os_atomic_stptr (volatile os_atomic_uintptr_t *x, uintptr_t v) { x->v = v; }
OS_ATOMIC_API_INLINE void os_atomic_stvoidp (volatile os_atomic_voidp_t *x, void *v) { os_atomic_stptr (x, (uintptr_t) v); }

/* CAS */

OS_ATOMIC_API_INLINE int os_atomic_cas32 (volatile os_atomic_uint32_t *x, uint32_t exp, uint32_t des) {
  return InterlockedCompareExchange (&x->v, des, exp) == exp;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE int os_atomic_cas64 (volatile os_atomic_uint64_t *x, uint64_t exp, uint64_t des) {
  return InterlockedCompareExchange64 (&x->v, des, exp) == exp;
}
#endif
OS_ATOMIC_API_INLINE int os_atomic_casptr (volatile os_atomic_uintptr_t *x, uintptr_t exp, uintptr_t des) {
  return OS_ATOMIC_PTROP (InterlockedCompareExchange) (&x->v, des, exp) == exp;
}
OS_ATOMIC_API_INLINE int os_atomic_casvoidp (volatile os_atomic_voidp_t *x, void *exp, void *des) {
  return os_atomic_casptr ((volatile os_atomic_uintptr_t *) x, (uintptr_t) exp, (uintptr_t) des);
}

/* INC */

OS_ATOMIC_API_INLINE void os_atomic_inc32 (volatile os_atomic_uint32_t *x) {
  InterlockedIncrement (&x->v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_inc64 (volatile os_atomic_uint64_t *x) {
  InterlockedIncrement64 (&x->v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_incptr (volatile os_atomic_uintptr_t *x) {
  OS_ATOMIC_PTROP (InterlockedIncrement) (&x->v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_inc32_nv (volatile os_atomic_uint32_t *x) {
  return InterlockedIncrement (&x->v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_inc64_nv (volatile os_atomic_uint64_t *x) {
  return InterlockedIncrement64 (&x->v);
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_incptr_nv (volatile os_atomic_uintptr_t *x) {
  return OS_ATOMIC_PTROP (InterlockedIncrement) (&x->v);
}

/* DEC */

OS_ATOMIC_API_INLINE void os_atomic_dec32 (volatile os_atomic_uint32_t *x) {
  InterlockedDecrement (&x->v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_dec64 (volatile os_atomic_uint64_t *x) {
  InterlockedDecrement64 (&x->v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_decptr (volatile os_atomic_uintptr_t *x) {
  OS_ATOMIC_PTROP (InterlockedDecrement) (&x->v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_dec32_nv (volatile os_atomic_uint32_t *x) {
  return InterlockedDecrement (&x->v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_dec64_nv (volatile os_atomic_uint64_t *x) {
  return InterlockedDecrement64 (&x->v);
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_decptr_nv (volatile os_atomic_uintptr_t *x) {
  return OS_ATOMIC_PTROP (InterlockedDecrement) (&x->v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_dec32_ov (volatile os_atomic_uint32_t *x) {
  return InterlockedDecrement (&x->v) + 1;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_dec64_ov (volatile os_atomic_uint64_t *x) {
  return InterlockedDecrement64 (&x->v) + 1;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_decptr_ov (volatile os_atomic_uintptr_t *x) {
  return OS_ATOMIC_PTROP (InterlockedDecrement) (&x->v) + 1;
}

/* ADD */

OS_ATOMIC_API_INLINE void os_atomic_add32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  InterlockedExchangeAdd (&x->v, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_add64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  InterlockedExchangeAdd64 (&x->v, v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_addptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  OS_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, v);
}
OS_ATOMIC_API_INLINE void os_atomic_addvoidp (volatile os_atomic_voidp_t *x, ptrdiff_t v) {
  os_atomic_addptr ((volatile os_atomic_uintptr_t *) x, (uintptr_t) v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_add32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  return InterlockedExchangeAdd (&x->v, v) + v;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_add64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedExchangeAdd64 (&x->v, v) + v;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_addptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, v) + v;
}
OS_ATOMIC_API_INLINE void *os_atomic_addvoidp_nv (volatile os_atomic_voidp_t *x, ptrdiff_t v) {
  return (void *) os_atomic_addptr_nv ((volatile os_atomic_uintptr_t *) x, (uintptr_t) v);
}

/* SUB */

OS_ATOMIC_API_INLINE void os_atomic_sub32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  InterlockedExchangeAdd (&x->v, -v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_sub64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  InterlockedExchangeAdd64 (&x->v, -v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_subptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  OS_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, -v);
}
OS_ATOMIC_API_INLINE void os_atomic_subvoidp (volatile os_atomic_voidp_t *x, ptrdiff_t v) {
  os_atomic_subptr ((volatile os_atomic_uintptr_t *) x, (uintptr_t) v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_sub32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  return InterlockedExchangeAdd (&x->v, -v) - v;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_sub64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedExchangeAdd64 (&x->v, -v) - v;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_subptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (InterlockedExchangeAdd) (&x->v, -v) - v;
}
OS_ATOMIC_API_INLINE void *os_atomic_subvoidp_nv (volatile os_atomic_voidp_t *x, ptrdiff_t v) {
  return (void *) os_atomic_subptr_nv ((volatile os_atomic_uintptr_t *) x, (uintptr_t) v);
}

/* AND */

#if defined OS_ATOMIC_INTERLOCKED_AND

OS_ATOMIC_API_INLINE void os_atomic_and32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  OS_ATOMIC_INTERLOCKED_AND (&x->v, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_and64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  InterlockedAnd64 (&x->v, v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_andptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_AND) (&x->v, v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_and32_ov (volatile os_atomic_uint32_t *x, uint32_t v) {
  return OS_ATOMIC_INTERLOCKED_AND (&x->v, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_and64_ov (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedAnd64 (&x->v, v);
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_andptr_ov (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_AND) (&x->v, v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_and32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  return OS_ATOMIC_INTERLOCKED_AND (&x->v, v) & v;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_and64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedAnd64 (&x->v, v) & v;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_andptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_AND) (&x->v, v) & v;
}

#else /* synthesize via CAS */

OS_ATOMIC_API_INLINE uint32_t os_atomic_and32_ov (volatile os_atomic_uint32_t *x, uint32_t v) {
  uint64_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_cas32 (x, oldval, newval));
  return oldval;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_and64_ov (volatile os_atomic_uint64_t *x, uint64_t v) {
  uint64_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_cas64 (x, oldval, newval));
  return oldval;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_andptr_ov (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  uintptr_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_casptr (x, oldval, newval));
  return oldval;
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_and32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  uint32_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_cas32 (x, oldval, newval));
  return newval;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_and64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  uint64_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_cas64 (x, oldval, newval));
  return newval;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_andptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  uintptr_t oldval, newval;
  do { oldval = x->v; newval = oldval & v; } while (!os_atomic_casptr (x, oldval, newval));
  return newval;
}
OS_ATOMIC_API_INLINE void os_atomic_and32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  (void) os_atomic_and32_nv (x, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_and64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  (void) os_atomic_and64_nv (x, v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_andptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  (void) os_atomic_andptr_nv (x, v);
}

#endif

/* OR */

#if defined OS_ATOMIC_INTERLOCKED_OR

OS_ATOMIC_API_INLINE void os_atomic_or32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  OS_ATOMIC_INTERLOCKED_OR (&x->v, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_or64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  InterlockedOr64 (&x->v, v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_orptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_OR) (&x->v, v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_or32_ov (volatile os_atomic_uint32_t *x, uint32_t v) {
  return OS_ATOMIC_INTERLOCKED_OR (&x->v, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_or64_ov (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedOr64 (&x->v, v);
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_orptr_ov (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_OR) (&x->v, v);
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_or32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  return OS_ATOMIC_INTERLOCKED_OR (&x->v, v) | v;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_or64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  return InterlockedOr64 (&x->v, v) | v;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_orptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  return OS_ATOMIC_PTROP (OS_ATOMIC_INTERLOCKED_OR) (&x->v, v) | v;
}

#else /* synthesize via CAS */

OS_ATOMIC_API_INLINE uint32_t os_atomic_or32_ov (volatile os_atomic_uint32_t *x, uint32_t v) {
  uint32_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_cas32 (x, oldval, newval));
  return oldval;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_or64_ov (volatile os_atomic_uint64_t *x, uint64_t v) {
  uint64_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_cas64 (x, oldval, newval));
  return oldval;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_orptr_ov (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  uintptr_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_casptr (x, oldval, newval));
  return oldval;
}
OS_ATOMIC_API_INLINE uint32_t os_atomic_or32_nv (volatile os_atomic_uint32_t *x, uint32_t v) {
  uint32_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_cas32 (x, oldval, newval));
  return newval;
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE uint64_t os_atomic_or64_nv (volatile os_atomic_uint64_t *x, uint64_t v) {
  uint64_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_cas64 (x, oldval, newval));
  return newval;
}
#endif
OS_ATOMIC_API_INLINE uintptr_t os_atomic_orptr_nv (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  uintptr_t oldval, newval;
  do { oldval = x->v; newval = oldval | v; } while (!os_atomic_casptr (x, oldval, newval));
  return newval;
}
OS_ATOMIC_API_INLINE void os_atomic_or32 (volatile os_atomic_uint32_t *x, uint32_t v) {
  (void) os_atomic_or32_nv (x, v);
}
#if OS_ATOMIC64_SUPPORT
OS_ATOMIC_API_INLINE void os_atomic_or64 (volatile os_atomic_uint64_t *x, uint64_t v) {
  (void) os_atomic_or64_nv (x, v);
}
#endif
OS_ATOMIC_API_INLINE void os_atomic_orptr (volatile os_atomic_uintptr_t *x, uintptr_t v) {
  (void) os_atomic_orptr_nv (x, v);
}

#endif

/* FENCES */

OS_ATOMIC_API_INLINE void os_atomic_fence (void) {
  volatile LONG tmp = 0;
  InterlockedExchange (&tmp, 0);
}
OS_ATOMIC_API_INLINE void os_atomic_fence_acq (void) {
  os_atomic_fence ();
}
OS_ATOMIC_API_INLINE void os_atomic_fence_rel (void) {
  os_atomic_fence ();
}

#undef OS_ATOMIC_INTERLOCKED_AND
#undef OS_ATOMIC_INTERLOCKED_OR
#undef OS_ATOMIC_INTERLOCKED_AND64
#undef OS_ATOMIC_INTERLOCKED_OR64
#undef OS_ATOMIC_API_INLINE

#endif /* not omit functions */

#undef OS_ATOMIC_PTROP
#define OS_ATOMIC_SUPPORT 1
