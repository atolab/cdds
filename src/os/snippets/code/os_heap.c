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

#include <assert.h>
#include "os/os.h"

/** \brief Allocate memory from heap
 *
 */
_Check_return_
_Ret_opt_bytecap_(size)
void *
os_malloc_s(
    _In_ size_t size)
{
    return malloc(size ? size : 1); /* Allocate memory even if size == 0 */
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
    _In_range_(>, 0) size_t count,
    _In_range_(>, 0) size_t size)
{
    char *ptr;

    assert(size > 0);
    assert(count > 0);

    ptr = os_calloc_s(count, size);

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
    _In_ size_t count,
    _In_ size_t size)
{
    if(count == 0 || size == 0) {
        count = size = 1;
    }
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

    ptr = os_realloc_s(memblk, size ? size : 1); /* Allocate even if size == NULL */

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
    /* Even though newmem = realloc(mem, 0) is equivalent to calling free(mem), not all platforms
     * will return newmem == NULL. We consistently do, so the result of a non-failing os_realloc_s
     * always needs to be free'd, like os_malloc_s(0). */
    return realloc(memblk, size ? size : 1);
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
