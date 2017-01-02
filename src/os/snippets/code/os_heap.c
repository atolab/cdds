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
/** \file os/common/code/os_heap.c
 *  \brief Heap memory management service
 *
 * Implements functions for allocation and freeing
 * memory from and to heap respectively.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#ifdef VXWORKS_RTP
#include <string.h>
#endif
#include "os/os.h"

#if defined LINUX && defined OSPL_STRICT_MEM
#include <stdint.h>
#endif

#if defined _WRS_KERNEL && defined OSPL_STRICT_MEM
#include <stdio.h>
#endif

#ifdef OSPL_STRICT_MEM
static pa_uint32_t alloccnt = { 0ULL };
#endif

/** \brief Allocate memory from heap
 *
 */
void *
os_malloc (
    size_t size)
{
    char *ptr;

    assert(size > 0);

#ifdef OSPL_STRICT_MEM
    /* Allow 24 bytes so we can store the allocation size, magic number and malloc count, ( and keep alignement ) */
    ptr = malloc((size_t)size+24);
    if ( ptr != NULL )
    {
       *((size_t *)ptr) = size;
       ptr += 24;
       memset(ptr, 0, size);
       *(((uint64_t*)ptr)-1) = OS_MALLOC_MAGIC_SIG;
       *(((uint64_t*)ptr)-2) = pa_inc32_nv(&alloccnt);
    }
#else
    ptr = malloc(size);
#endif

    if(size == 0 && !ptr) {
        /* os_malloc() should never return NULL. Although it is not allowed to
         * pass size 0, this fallback assures code continues to run if the
         * os_malloc(0) isn't caught in a DEV-build. */
        ptr = malloc(1);
    }

    if(ptr == NULL) {
        /* Heap exhausted */
        abort();
    }

    return ptr;
}

void *
os_realloc(
    void *memblk,
    size_t size)
{
    unsigned char *ptr = (unsigned char *)memblk;

    assert(size > 0);

#ifdef OSPL_STRICT_MEM
    size_t origsize = 0;
    if ( ptr != NULL )
    {
       size_t i;
       origsize = *((size_t *)(ptr - 24));

       assert (*(((uint64_t*)ptr)-1) != OS_FREE_MAGIC_SIG);
       assert (*(((uint64_t*)ptr)-1) == OS_MALLOC_MAGIC_SIG);
       *(((uint64_t*)ptr)-1) = OS_FREE_MAGIC_SIG;

       for ( i = 0; i+7 < origsize; i++ )
       {
          assert( OS_MAGIC_SIG_CHECK( &ptr[i] ) && "Realloc of memory containing mutex or Condition variable" );
       }
       ptr -= 24;
    }

    if ( size > 0 )
    {
       size += 24;
    }
#endif

    ptr = realloc(ptr, size);

#ifdef OSPL_STRICT_MEM
    if ( size > 0 && ptr != NULL )
    {
       size -= 24;
       if ( size > origsize )
       {
          memset( ptr + 24 + origsize, 0, size - origsize );
       }
       *((size_t *)ptr) = size;
       ptr += 24;
       *(((uint64_t*)ptr)-1) = OS_MALLOC_MAGIC_SIG;
       *(((uint64_t*)ptr)-2) = pa_inc32_nv(&alloccnt);
    }
#endif

    if(ptr == NULL){
        abort();
    }

    return ptr;
}

/** \brief Free memory to heap
 *
 * \b os_free calls \b free which is a function pointer
 * which defaults to \b free, but can be redefined via
 * \b os_heapSetService.
 */
void
os_free (
    void *ptr)
{
    if (ptr != NULL)
    {
#ifdef OSPL_STRICT_MEM
        {
          size_t i;
          unsigned char *cptr = (unsigned char *)ptr;
          size_t memsize = *((size_t *)(cptr - 24));
          assert (*(((uint64_t*)ptr)-1) != OS_FREE_MAGIC_SIG);
          if (*(((uint64_t*)ptr)-1) != OS_MALLOC_MAGIC_SIG)
          {
             fprintf (stderr, "%s (%d): os_free error\n", __FILE__, __LINE__);
          }
          assert (*(((uint64_t*)ptr)-1) == OS_MALLOC_MAGIC_SIG);
          *(((uint64_t*)ptr)-1) = OS_FREE_MAGIC_SIG;
          for ( i = 0; i+7 < memsize; i++ )
          {
            assert( OS_MAGIC_SIG_CHECK( &cptr[i] ) && "Free of memory containing Mutex or Condition variable");
          }
          ptr = cptr - 24;
        }
#endif
        free (((char *)ptr));
    }
    return;
}
