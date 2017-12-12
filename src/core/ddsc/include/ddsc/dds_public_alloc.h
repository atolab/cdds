/* TODO: add copyright header?
   TODO: do we really need to expose this as an API? */

/** @file
 *
 * @brief DDS C Allocation API
 *
 * This header file defines the public API of allocation convenience functions
 * in the VortexDDS C language binding.
 */
#ifndef DDS_ALLOC_H
#define DDS_ALLOC_H

#include "os/os_public.h"
#include "ddsc/dds_export.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct dds_topic_descriptor;
struct dds_sequence;

#define DDS_FREE_KEY_BIT 0x01
#define DDS_FREE_CONTENTS_BIT 0x02
#define DDS_FREE_ALL_BIT 0x04

typedef enum
{
  DDS_FREE_ALL = DDS_FREE_KEY_BIT | DDS_FREE_CONTENTS_BIT | DDS_FREE_ALL_BIT,
  DDS_FREE_CONTENTS = DDS_FREE_KEY_BIT | DDS_FREE_CONTENTS_BIT,
  DDS_FREE_KEY = DDS_FREE_KEY_BIT
}
dds_free_op_t;

typedef struct dds_allocator
{
  /* Behaviour as C library malloc, realloc and free */

  void * (*malloc) (size_t size);
  void * (*realloc) (void *ptr, size_t size); /* if needed */
  void (*free) (void *ptr);
}
dds_allocator_t;

DDS_EXPORT void dds_set_allocator (const dds_allocator_t * __restrict n, dds_allocator_t * __restrict o);

typedef struct dds_aligned_allocator
{
  /* size is a multiple of align, align is a power of 2 no less than
     the machine's page size, returned pointer MUST be aligned to at
     least align. */
  void * (*alloc) (size_t size, size_t align);
  void (*free) (size_t size, void *ptr);
}
dds_aligned_allocator_t;

DDS_EXPORT void dds_set_aligned_allocator (const dds_aligned_allocator_t * __restrict n, dds_aligned_allocator_t * __restrict o);

DDS_EXPORT void * dds_alloc (size_t size);
DDS_EXPORT void * dds_realloc (void * ptr, size_t size);
DDS_EXPORT void * dds_realloc_zero (void * ptr, size_t size);
DDS_EXPORT void dds_free (void * ptr);

typedef void * (*dds_alloc_fn_t) (size_t);
typedef void * (*dds_realloc_fn_t) (void *, size_t);
typedef void (*dds_free_fn_t) (void *);

DDS_EXPORT char * dds_string_alloc (size_t size);
DDS_EXPORT char * dds_string_dup (const char * str);
DDS_EXPORT void dds_string_free (char * str);
DDS_EXPORT void dds_sample_free (void * sample, const struct dds_topic_descriptor * desc, dds_free_op_t op);

#if defined (__cplusplus)
}
#endif
#endif