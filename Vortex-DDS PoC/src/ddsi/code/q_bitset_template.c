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
/* -*- c -*- */

#include "q_unused.h"

#if defined SUPPRESS_BITSET_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE int nn_bitset_isset (unsigned numbits, const unsigned *bits, unsigned idx)
{
  return idx < numbits && (bits[idx/32] & (1u << (31 - (idx%32))));
}

NN_C99_INLINE void nn_bitset_set (UNUSED_ARG_NDEBUG (unsigned numbits), unsigned *bits, unsigned idx)
{
  assert (idx < numbits);
  bits[idx/32] |= 1u << (31 - (idx%32));
}

NN_C99_INLINE void nn_bitset_clear (UNUSED_ARG_NDEBUG (unsigned numbits), unsigned *bits, unsigned idx)
{
  assert (idx < numbits);
  bits[idx/32] &= ~(1u << (31 - (idx%32)));
}

NN_C99_INLINE void nn_bitset_zero (unsigned numbits, unsigned *bits)
{
  memset (bits, 0, 4 * ((numbits + 31) / 32));
}

NN_C99_INLINE void nn_bitset_one (unsigned numbits, unsigned *bits)
{
  memset (bits, 0xff, 4 * ((numbits + 31) / 32));

  /* clear bits "accidentally" set */
  {
    const unsigned k = numbits / 32;
    const unsigned n = numbits % 32;
    bits[k] &= ~(~0u >> n);
  }
}
