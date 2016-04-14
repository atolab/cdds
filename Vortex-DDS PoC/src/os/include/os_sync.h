#ifndef OS_SYNC_H
#define OS_SYNC_H

#include "os_defs.h"
#include "os_platform_sync.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

    /** \brief Definition of the mutex attributes
     */
    typedef struct os_mutexAttr {
        /**
         * - OS_SCOPE_SHARED The scope of the mutex *   is system wide
         * - OS_SCOPE_PRIVATE The scope of the mutex *   is process wide
         */
        os_scopeAttr    scopeAttr;

        /* - OS_ERRORCHECKING_DISABLED The mutex operations aren't checked
         * - OS_ERRORCHECKING_ENABLED The mutex operations are checked */
        os_errorCheckingAttr errorCheckingAttr;
    } os_mutexAttr;

    /** \brief Sets the priority inheritance mode for mutexes
     *   that are created after this call. (only effective on
     *   platforms that support priority inheritance)
     *
     * Possible Results:
     * - returns os_resultSuccess if
     *     mutex is successfuly initialized
     * - returns os_resultSuccess
     */
    OS_API os_result
    os_mutexSetPriorityInheritanceMode(
            bool enabled);

    /** \brief Initialize the mutex taking the mutex attributes
     *         into account
     *
     * Possible Results:
     * - assertion failure: mutex = NULL || mutexAttr = NULL
     * - returns os_resultSuccess if
     *     mutex is successfuly initialized
     * - returns os_resultFail if
     *     mutex is not initialized because of a failure
     */
    _Check_return_
    OS_API os_result
    os_mutexInit(
            _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_mutex *mutex,
            _In_opt_ const os_mutexAttr *mutexAttr)
        __nonnull((1));

    /** \brief Destroy the mutex
     *
     * Never returns on failure
     */
    OS_API void
    os_mutexDestroy(
            _Inout_ _Post_invalid_ os_mutex *mutex)
        __nonnull_all__;

    /** \brief Acquire the mutex.
     *
     * If you need to detect an error, use os_mutexLock_s instead.
     *
     * @see os_mutexLock_s
     */
    _Acquires_nonreentrant_lock_(&mutex->lock)
    OS_API void
    os_mutexLock(
            _Inout_ os_mutex *mutex)
    __nonnull_all__;

    /**
     * \brief Acquire the mutex. Returns whether the call succeeeded or an error
     * occurred.
     *
     * Precondition:
     * - mutex is not yet acquired by the calling thread
     *
     * Possible Results:
     * - assertion failure: mutex = NULL
     * - returns os_resultSuccess if
     *     mutex is acquired
     * - returns os_resultFail if
     *     mutex is not acquired because of a failure
     */
    _Check_return_
    _When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
        OS_API os_result
        os_mutexLock_s(
                _Inout_ os_mutex *mutex)
        __nonnull_all__
        __attribute_warn_unused_result__;

    /** \brief Try to acquire the mutex, immediately return if the mutex
     *         is already acquired by another thread
     *
     * Precondition:
     * - mutex is not yet acquired by the calling thread
     *
     * Possible Results:
     * - assertion failure: mutex = NULL
     * - returns os_resultSuccess if
     *      mutex is acquired
     * - returns os_resultBusy if
     *      mutex is not acquired because it is already acquired
     *      by another thread
     * - returns os_resultFail if
     *      mutex is not acquired because of a failure
     */
    _Check_return_
    _When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
        OS_API os_result
        os_mutexTryLock (
                _Inout_ os_mutex *mutex)
        __nonnull_all__
        __attribute_warn_unused_result__;

    /** \brief Release the acquired mutex
     */
    _Releases_nonreentrant_lock_(&mutex->lock)
    OS_API void
    os_mutexUnlock (
            _Inout_ os_mutex *mutex)
    __nonnull_all__;

    /** \brief Set the default mutex attributes
     *
     * Postcondition:
     * - mutex scope attribute is OS_SCOPE_PRIVATE
     * - mutex errorChecking attribute is OS_ERRORCHECKING_DISABLED
     *
     * Precondition:
     * - mutexAttr is a valid object
     */
    _Post_satisfies_(mutexAttr->scopeAttr == OS_SCOPE_PRIVATE)
    _Post_satisfies_(mutexAttr->errorCheckingAttr == OS_ERRORCHECKING_DISABLED)
    OS_API void
    os_mutexAttrInit(
            _Out_ os_mutexAttr *mutexAttr)
    __nonnull_all__;

    /** \brief Definition of the condition variable attributes
     */
    typedef struct os_condAttr {
        /**
         * - OS_SCOPE_SHARED The scope of the condition variable
         *   is system wide
         * - OS_SCOPE_PRIVATE The scope of the condition variable
         *   is process wide
         */
        os_scopeAttr    scopeAttr;
    } os_condAttr;

    /** \brief Initialize the condition variable taking the conition
     *         attributes into account
     * If condAttr == NULL, result is as if os_condInit was invoked
     * with the default os_condAttr as after os_condAttrInit.
     *
     * Possible Results:
     * - returns os_resultSuccess if
     *     cond is successfuly initialized
     * - returns os_resultFail if
     *     cond is not initialized and can not be used
     */
    _Check_return_
    _When_(condAttr == NULL, _Pre_satisfies_(mutex->scope == OS_SCOPE_PRIVATE))
        _When_(condAttr != NULL, _Pre_satisfies_(mutex->scope == condAttr->scopeAttr))
        OS_API os_result
        os_condInit(
                _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_cond *cond,
                _In_ os_mutex *mutex,
                _In_opt_ const os_condAttr *condAttr)
        __nonnull((1,2));

    /** \brief Destroy the condition variable
     */
    OS_API void
    os_condDestroy(
            _Inout_ _Post_invalid_ os_cond *cond)
        __nonnull_all__;

    /** \brief Wait for the condition
     *
     * Precondition:
     * - mutex is acquired by the calling thread before calling
     *   os_condWait
     *
     * Postcondition:
     * - mutex is still acquired by the calling thread and should
     *   be released by it
     */
    OS_API void
    os_condWait(
            os_cond *cond,
            os_mutex *mutex)
        __nonnull_all__;

    /** \brief Wait for the condition but return when the specified
     *         time has expired before the condition is triggered
     *
     * Precondition:
     * - mutex is acquired by the calling thread before calling
     *   os_condTimedWait
     *
     * Postcondition:
     * - mutex is still acquired by the calling thread and should
     *   be released by it
     *
     * Possible Results:
     * - assertion failure: cond = NULL || mutex = NULL ||
     *     time = NULL
     * - returns os_resultSuccess if
     *     cond is triggered
     * - returns os_resultTimeout if
     *     cond is timed out
     * - returns os_resultFail if
     *     cond is not triggered nor is timed out but
     *     os_condTimedWait has returned because of a failure
     */
    OS_API os_result
    os_condTimedWait(
            os_cond *cond,
            os_mutex *mutex,
            const os_time *time)
        __nonnull_all__;

    /** \brief Signal the condition and wakeup one thread waiting
     *         for the condition
     *
     * Precondition:
     * - the mutex used with the condition in general should be
     *   acquired by the calling thread before setting the
     *   condition state and signalling
     */
    OS_API void
    os_condSignal(
            os_cond *cond)
        __nonnull_all__;

    /** \brief Signal the condition and wakeup all thread waiting
     *         for the condition
     *
     * Precondition:
     * - the mutex used with the condition in general should be
     *   acquired by the calling thread before setting the
     *   condition state and signalling
     */
    OS_API void
    os_condBroadcast(
            os_cond *cond)
        __nonnull_all__;

    /** \brief Set the default condition variable attributes
     *
     * Postcondition:
     * - condition scope attribute is OS_SCOPE_SHARED
     *
     * Precondition:
     * - condAttr is a valid object
     */
    _Post_satisfies_(condAttr->scopeAttr == OS_SCOPE_PRIVATE)
    OS_API void
    os_condAttrInit(
            _Out_ os_condAttr *condAttr)
    __nonnull_all__;

    typedef struct os_rwlockAttr {
        /**
         * - OS_SCOPE_SHARED The scope of the multiple reader writer lock
         *   is system wide
         * - OS_SCOPE_PRIVATE The scope of the multiple reader writer lock
         *   is process wide
         */
        os_scopeAttr    scopeAttr;
    } os_rwlockAttr;

    /** \brief Initialize the rwlock taking the rwlock attributes into account
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL || rwlockAttr = NULL
     * - returns os_resultSuccess if
     *     rwlock is successfuly initialized
     * - returns os_resultFail
     *     rwlock is not initialized and can not be used
     */
    OS_API os_result
    os_rwlockInit(
            os_rwlock *rwlock,
            const os_rwlockAttr *rwlockAttr);

    /** \brief Destroy the rwlock
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *     rwlock is successfuly destroyed
     * - returns os_resultBusy if
     *     rwlock is not destroyed because it is still claimed or referenced by a thread
     * - returns os_resultFail if
     *     rwlock is not destroyed
     */
    OS_API void
    os_rwlockDestroy(
            os_rwlock *rwlock);

    /** \brief Acquire the rwlock while intending to read only
     *
     * Precondition:
     * - rwlock is not yet acquired by the calling thread
     *
     * Postcondition:
     * - The data related to the critical section is not changed
     *   by the calling thread
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *      rwlock is acquired
     * - returns os_resultFail if
     *      rwlock is not acquired because of a failure
     */
    OS_API void
    os_rwlockRead(
            os_rwlock *rwlock);

    /** \brief Acquire the rwlock while intending to write
     *
     * Precondition:
     * - rwlock is not yet acquired by the calling thread
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *      rwlock is acquired
     * - returns os_resultFail if
     *      rwlock is not acquired because of a failure
     */
    OS_API void
    os_rwlockWrite(
            os_rwlock *rwlock);

    /** \brief Try to acquire the rwlock while intending to read only
     *
     * Try to acquire the rwlock while intending to read only,
     * immediately return if the mutex is acquired by
     * another thread with the intention to write
     *
     * Precondition:
     * - rwlock is not yet acquired by the calling thread
     *
     * Postcondition:
     * - The data related to the critical section is not changed
     *   by the calling thread
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *      rwlock is acquired
     * - returns os_resultBusy if
     *      rwlock is not acquired because it is already
     *      acquired by another thread with the intention to write
     * - returns os_resultFail if
     *      rwlock is not acquired because of a failure
     */
    OS_API os_result
    os_rwlockTryRead(
            os_rwlock *rwlock);

    /** \brief Try to acquire the rwlock while intending to write
     *
     * Try to acquire the rwlock while intending to write,
     * immediately return if the mutex is acquired by
     * another thread, either for read or for write
     *
     * Precondition:
     * - rwlock is not yet acquired by the calling thread
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *      rwlock is acquired
     * - returns os_resultBusy if
     *      rwlock is not acquired because it is already
     *      acquired by another thread
     * - returns os_resultFail if
     *      rwlock is not acquired because of a failure
     */
    OS_API os_result
    os_rwlockTryWrite(
            os_rwlock *rwlock);

    /** \brief Release the acquired rwlock
     *
     * Precondition:
     * - rwlock is already acquired by the calling thread
     *
     * Possible Results:
     * - assertion failure: rwlock = NULL
     * - returns os_resultSuccess if
     *     rwlock is released
     * - returns os_resultFail if
     *     rwlock is not released because of a failure
     */
    OS_API void
    os_rwlockUnlock(
            os_rwlock *rwlock);

    /** \brief Set the default rwlock attributes
     *
     * Postcondition:
     * - rwlock scope attribute is OS_SCOPE_PRIVATE
     */
    _Post_satisfies_(rwlockAttr->scopeAttr == OS_SCOPE_PRIVATE)
    OS_API void
    os_rwlockAttrInit(
            _Out_ os_rwlockAttr *rwlockAttr)
    __nonnull_all__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
