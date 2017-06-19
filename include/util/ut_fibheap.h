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
#ifndef UT_FIBHEAP_H
#define UT_FIBHEAP_H

#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct ut_fibheapNode {
  struct ut_fibheapNode *parent, *children;
  struct ut_fibheapNode *prev, *next;
  unsigned mark: 1;
  unsigned degree: 31;
} ut_fibheapNode_t;

typedef struct ut_fibheapDef {
    uintptr_t offset;
    int (*cmp) (const void *va, const void *vb);
} ut_fibheapDef_t;

typedef struct ut_fibheap {
  ut_fibheapNode_t *roots; /* points to root with min key value */
} ut_fibheap_t;

#define UT_FIBHEAPDEF_INITIALIZER(offset, cmp) { (offset), (cmp) }

OSAPI_EXPORT void ut_fibheapDefInit (ut_fibheapDef_t *fhdef, uintptr_t offset, int (*cmp) (const void *va, const void *vb));
OSAPI_EXPORT void ut_fibheapInit (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh);
OSAPI_EXPORT void *ut_fibheapMin (const ut_fibheapDef_t *fhdef, const ut_fibheap_t *fh);
OSAPI_EXPORT void ut_fibheapMerge (const ut_fibheapDef_t *fhdef, ut_fibheap_t *a, ut_fibheap_t *b);
OSAPI_EXPORT void ut_fibheapInsert (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode);
OSAPI_EXPORT void ut_fibheapDelete (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode);
OSAPI_EXPORT void *ut_fibheapExtractMin (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh);
OSAPI_EXPORT void ut_fibheapDecreaseKey (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode); /* to be called AFTER decreasing the key */

#if defined (__cplusplus)
}
#endif

#endif /* UT_FIBHEAP_H */
