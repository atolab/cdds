#ifndef UT_HOPSCOTCH_H
#define UT_HOPSCOTCH_H

#include "os/os.h"
#include "util/ut_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

#if __STDC_VERSION__ >= 199901L
#define UT_HH_RESTRICT restrict
#else
#define UT_HH_RESTRICT
#endif

/* Concurrent version */
struct ut_chh;
struct ut_chhBucket;
struct ut_chhIter {
  struct ut_chhBucket *bs;
  uint32_t size;
  uint32_t cursor;
};

typedef int (*ut_hhEquals_fn) (const void *, const void *);

UTIL_EXPORT struct ut_chh *ut_chhNew (uint32_t init_size, uint32_t (*hash) (const void *a), ut_hhEquals_fn, void (*gc_buckets) (void *a));
UTIL_EXPORT void ut_chhFree (struct ut_chh * UT_HH_RESTRICT hh);
UTIL_EXPORT void *ut_chhLookup (struct ut_chh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT int ut_chhAdd (struct ut_chh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT data);
UTIL_EXPORT int ut_chhRemove (struct ut_chh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT void ut_chhEnumUnsafe (struct ut_chh * UT_HH_RESTRICT rt, void (*f) (void *a, void *f_arg), void *f_arg); /* may delete a */
void *ut_chhIterFirst (struct ut_chh * UT_HH_RESTRICT rt, struct ut_chhIter *it);
void *ut_chhIterNext (struct ut_chhIter *it);

/* Sequential version */
struct ut_hh;

struct ut_hhIter {
    struct ut_hh *hh;
    uint32_t cursor;
};

UTIL_EXPORT struct ut_hh *ut_hhNew (uint32_t init_size, uint32_t (*hash) (const void *a), ut_hhEquals_fn);
UTIL_EXPORT void ut_hhFree (struct ut_hh * UT_HH_RESTRICT hh);
UTIL_EXPORT void *ut_hhLookup (const struct ut_hh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT int ut_hhAdd (struct ut_hh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT data);
UTIL_EXPORT int ut_hhRemove (struct ut_hh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT void ut_hhEnum (struct ut_hh * UT_HH_RESTRICT rt, void (*f) (void *a, void *f_arg), void *f_arg); /* may delete a */
UTIL_EXPORT void *ut_hhIterFirst (struct ut_hh * UT_HH_RESTRICT rt, struct ut_hhIter * UT_HH_RESTRICT iter); /* may delete nodes */
UTIL_EXPORT void *ut_hhIterNext (struct ut_hhIter * UT_HH_RESTRICT iter);

/* Sequential version, embedded data */
struct ut_ehh;

struct ut_ehhIter {
    struct ut_ehh *hh;
    uint32_t cursor;
};

UTIL_EXPORT struct ut_ehh *ut_ehhNew (size_t elemsz, uint32_t init_size, uint32_t (*hash) (const void *a), ut_hhEquals_fn);
UTIL_EXPORT void ut_ehhFree (struct ut_ehh * UT_HH_RESTRICT hh);
UTIL_EXPORT void *ut_ehhLookup (const struct ut_ehh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT int ut_ehhAdd (struct ut_ehh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT data);
UTIL_EXPORT int ut_ehhRemove (struct ut_ehh * UT_HH_RESTRICT rt, const void * UT_HH_RESTRICT template);
UTIL_EXPORT void ut_ehhEnum (struct ut_ehh * UT_HH_RESTRICT rt, void (*f) (void *a, void *f_arg), void *f_arg); /* may delete a */
UTIL_EXPORT void *ut_ehhIterFirst (struct ut_ehh * UT_HH_RESTRICT rt, struct ut_ehhIter * UT_HH_RESTRICT iter); /* may delete nodes */
UTIL_EXPORT void *ut_ehhIterNext (struct ut_ehhIter * UT_HH_RESTRICT iter);

#if defined (__cplusplus)
}
#endif

#endif
