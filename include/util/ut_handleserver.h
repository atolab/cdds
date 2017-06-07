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
#ifndef UT_HANDLESERVER_H
#define UT_HANDLESERVER_H

#include "os/os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#if VDDS_BUILD
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/*
 * The handleserver type.
 */
typedef struct ut_handleserver* ut_handleserver_t;

/*
 * The handle claim type.
 */
typedef struct ut_handleinfo* ut_handleclaim_t;

/*
 * The 32 bit handle
 *      |  bits |       values | description                                                 |
 *      --------------------------------------------------------------------------------------
 *      |    31 |            2 | positive/negative (negative can be used to indicate errors) |
 *      | 26-30 |           32 | handle kind       (value determined by client)              |
 *      | 23-25 |            8 | sequence number   (maintained by the handleserver)          |
 *      |  0-22 |    8.388.608 | index             (maintained by the handleserver)          |
 *
 * To make sure that sequence is wrapped as little as possible, the create handle should search
 * for a not used entry with the lowest sequence number and use that.
 *
 * TODO: maybe go for 64 bits anyway (dds_instance_handles_t is 64 bits as well), which makes
 * it more easy to create unique handles with different information within them.
 */
typedef int32_t ut_handle_t;

/*
 * Handle bits
 *   +kkk kkss siii iiii iiii iiii iiii iiii
 * 31|   26| 23|                          0|
 */
#define UT_HANDLE_SIGN_MASK (0x80000000)
#define UT_HANDLE_KIND_MASK (0x7C000000)
#define UT_HANDLE_SEQ_MASK  (0x03800000)
#define UT_HANDLE_IDX_MASK  (0x007FFFFF)

/*
 * Some error return values.
 */
#define UT_HANDLE_ERROR                  (-1) /* Internal error. */
#define UT_HANDLE_ERROR_DELETED          (-2) /* Handle has been previously deleted. */
#define UT_HANDLE_ERROR_INVALID          (-3) /* Handle is not a valid handle. */
#define UT_HANDLE_ERROR_KIND_NOT_EQUAL   (-4) /* Handle does not contain expected kind. */
#define UT_HANDLE_ERROR_NOT_LOCKABLE     (-5) /* Handle was created without mutex. */

/*
 * Create new handleserver.
 */
_Check_return_ OS_API ut_handleserver_t
ut_handleserver_new();

/*
 * Destroy handleserver.
 */
OS_API void
ut_handleserver_free(_Inout_ _Post_invalid_ ut_handleserver_t srv);

/*
 * This creates a new handle that contains the given type and is linked to the
 * user data.
 *
 * The handleserver will keep increasing its internal administration with
 * blocks of a arbitrary number of handles when needed. Before doing that,
 * it'll scan the previous published handles and see if any of them has been
 * deleted (the 'in use' bit will be 0). If so, then that handle will be
 * reused with an increased sequence number.
 *
 * A mutex can be provided with the creation of the handle. When it is
 * provided, then the claim/release functions will use that to actually
 * lock and unlock the handle.
 * When the mutex is NULL, then the claim/release functions won't do anything.
 * It is allowed to pass the same mutex to multiple different handles. Then
 * claiming one handle will prevent other handles to be claimed.
 *
 * A kind value != 0 has to be provided, just to make sure that no 0 handles
 * will be created. It should also fit the UT_HANDLE_KIND_MASK.
 * In other words handle creation will fail if
 * ((kind & ~UT_HANDLE_KIND_MASK != 0) || (kind & UT_HANDLE_KIND_MASK == 0)).
 *
 * Does not lock handle.
 *
 * srv  - The handle server to create and store the handle in.
 * kind - The handle kind, provided by the client.
 * arg  - The user data linked to the handle (may be NULL).
 * mtx  - The mutex used when claiming the created handle (may be NULL).
 *
 * Valid handle when returned value is positive.
 * Otherwise negative handle is returned with error value.
 */
_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Post_satisfies_((return & UT_HANDLE_KIND_MASK) == kind)
_Check_return_ OS_API ut_handle_t
ut_handle_create(_In_ ut_handleserver_t srv, _In_ int32_t kind, _In_opt_ void *arg, _In_opt_ os_mutex *mtx);

/*
 * This will remove the handle related information from the server administration
 * so that the entry can be used for another handle.
 *
 * When using the given handle again, UT_HANDLESERVER_ERROR_HANDLE_DELETED will
 * be returned by any handleserver function.
 *
 * Does not lock nor unlock at handle creation provided mutex
 */
OS_API void
ut_handle_delete(_In_ ut_handleserver_t srv, _In_ _Post_invalid_ ut_handle_t hdl);

/*
 * This will remove the handle related information from the server administration
 * so that the entry can be used for another handle.
 *
 * When using the given handle again, UT_HANDLESERVER_ERROR_HANDLE_DELETED will
 * be returned by any handleserver function.
 *
 * Does unlock at handle creation provided mutex (after deletion finished).
 */
OS_API void
ut_handle_delete_by_claim(_In_ ut_handleserver_t srv, _Inout_ _Post_invalid_ ut_handleclaim_t claim);

/*
 * Checks if the given handle is valid.
 *
 * An handle is deleted when either the handle server finds the related handle
 * entry un-used or if the entry's sequence number does not equal to the sequence
 * within the handle.
 *
 * An handle can also be invalid when it contains a index that hasn't been
 * published yet.
 *
 * Does not lock handle.
 *
 * Valid when returned handle == given hdl.
 * Otherwise negative handle is returned with error value.
 */
_Check_return_ OS_API ut_handle_t
ut_handle_is_valid(_In_ ut_handleserver_t srv, _In_ ut_handle_t hdl);

/*
 * This will give the arg that is linked to the handle.
 *
 * Does not lock handle.
 *
 * Valid when returned handle == given hdl.
 * Otherwise negative handle is returned with error value.
 */
_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Check_return_ OS_API ut_handle_t
ut_handle_get_arg(_In_ ut_handleserver_t srv, _In_ ut_handle_t hdl, _In_ int32_t kind, _Out_ void **arg);

/*
 * At creation given mutex will be locked and the related arg will be given
 * if the handle is valid, matches the kind and is not deleted.
 *
 * Valid when returned handle == given hdl.
 * Otherwise negative handle is returned with error value.
 */
_Pre_satisfies_((kind & UT_HANDLE_KIND_MASK) && !(kind & ~UT_HANDLE_KIND_MASK))
_Check_return_ OS_API ut_handle_t
ut_handle_claim(_In_ ut_handleserver_t srv, _In_ ut_handle_t hdl, _In_ int32_t kind, _Out_opt_ void **arg, _Out_ ut_handleclaim_t *claim);

/*
 * At creation given mutex will be unlocked.
 */
OS_API void
ut_handle_release(_In_ ut_handleserver_t srv, _Inout_ _Post_invalid_ ut_handleclaim_t claim);



#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_HANDLESERVER_H */
