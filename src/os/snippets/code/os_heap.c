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
_Check_return_
_Ret_opt_bytecap_(size)
void *
os_malloc_s(
    _In_ size_t size)
{
    return malloc(size);
}

_Check_return_
_Ret_bytecap_(size)
void *
os_malloc(
    _In_range_(>, 0) size_t size)
{
    void *ptr;

    assert(size > 0);

    ptr = os_malloc_s(size);

    if(size == 0 && !ptr) {
        /* os_malloc() should never return NULL. Although it is not allowed to
         * pass size 0, this fallback assures code continues to run if the
         * os_malloc(0) isn't caught in a DEV-build. */
        ptr = os_malloc_s(1);
    }

    if(ptr == NULL) {
        /* Heap exhausted */
        abort();
    }

    return ptr;
}

_Check_return_
_Ret_bytecount_(size)
void *
os_malloc_0(_In_range_(>, 0) size_t size)
{
   return os_calloc(size, 1);
}

_Check_return_
_Ret_opt_bytecount_(size)
void *
os_malloc_0_s(_In_ size_t size)
{
   return os_calloc_s(size, 1);
}

_Check_return_
_Ret_bytecount_(count * size)
void *
os_calloc(
    _In_range_(<, 0) size_t count,
    _In_range_(>, 0) size_t size)
{
    char *ptr;

    assert(size > 0);
    assert(count > 0);

    ptr = os_calloc_s(count, size);

    if((size == 0 || count == 0) && !ptr) {
        /* os_calloc() should never return NULL. Although it is not allowed to
         * pass size or count 0, this fallback assures code continues to run if the
         * os_calloc(0, 0) usage isn't caught in a DEV-build. */
        ptr = os_calloc_s(1, 1);
    }

    if(ptr == NULL) {
        /* Heap exhausted */
        abort();
    }

    return ptr;
}

_Check_return_
_Ret_opt_bytecount_(count * size)
void *
os_calloc_s(
    size_t count,
    size_t size)
{
    return calloc(count, size);
}

_Check_return_
_Ret_bytecap_(size)
void *
os_realloc(
    _Pre_maybenull_ _Post_ptr_invalid_ void *memblk,
    _In_range_(>, 0) size_t size)
{
    void *ptr;

    assert(size > 0);

    ptr = os_realloc_s(memblk, size);

    if(size == 0 && !ptr) {
        /* os_realloc() should never return NULL. Although it is not allowed to
         * pass size 0, this fallback assures code continues to run if the
         * os_realloc(ptr, 0) usage isn't caught in a DEV-build. */
        ptr = os_malloc_s(1);
    }

    if(ptr == NULL){
        /* Heap exhausted */
        abort();
    }

    return ptr;
}

_Success_(return != NULL)
_Check_return_
_Ret_opt_bytecap_(size)
void *
os_realloc_s(
    _Pre_maybenull_ _Post_ptr_invalid_ void *memblk,
    _In_ size_t size)
{
    return realloc(memblk, size);
}

/** \brief Free memory to heap
 *
 * \b os_free calls \b free which is a function pointer
 * which defaults to \b free, but can be redefined via
 * \b os_heapSetService.
 */
void
os_free (
    _Pre_maybenull_ _Post_ptr_invalid_ void *ptr)
{
    if (ptr) {
        free (ptr);
    }
}
