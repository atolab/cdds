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
#ifndef UT_THREAD_POOL_H
#define UT_THREAD_POOL_H

#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct ut_thread_pool_s *ut_thread_pool;

/* 
  ut_thread_pool_new: Creates a new thread pool. Returns NULL if
  cannot create initial set of threads. Threads are created with 
  the optional atribute argument. Additional threads may be created
  on demand up to max_threads.
*/

OS_API ut_thread_pool ut_thread_pool_new 
(
  uint32_t threads,     /* Initial number of threads in pool (can be 0) */
  uint32_t max_threads, /* Maximum number of threads in pool (0 == infinite) */
  uint32_t max_queue,   /* Maximum number of queued requests (0 == infinite) */
  os_threadAttr * attr   /* Attributes used to create pool threads (can be NULL) */
);

/* ut_thread_pool_free: Frees pool, destroying threads. */

OS_API void ut_thread_pool_free (ut_thread_pool pool);

/* ut_thread_pool_purge: Purge threads from pool back to initial set. */

OS_API void ut_thread_pool_purge (ut_thread_pool pool);

/* 
  ut_thread_pool_submit: Submit a thread function and associated argument
  to be invoked by a thread from the pool. If no threads are available a
  new thread will be created on demand to handle the function unless the
  pool thread maximum has been reached, in which case the function is queued.
  Note that if the pool queue has reached it's maximum os_resultBusy is returned.
*/

OS_API os_result ut_thread_pool_submit
(
  ut_thread_pool pool,  /* Thread pool instance */
  void (*fn) (void *arg),  /* Function to be invoked by thread from pool */
  void * arg            /* Argument passed to invoked function */
);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_THREAD_POOL_H */
