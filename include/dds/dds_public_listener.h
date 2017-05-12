/**
 * @file dds_listener.h
 * @brief DDS C99 Listener API
 *
 * @todo add copyright header?
 *
 * This header file defines the public API of listeners in the VortexDDS C99 language binding.
 */

#ifndef _DDS_PUBLIC_LISTENER_H_
#define _DDS_PUBLIC_LISTENER_H_

#include "dds.h"
#include "dds/dds_public_impl.h"
#include "dds/dds_public_status.h"

#if defined (__cplusplus)
extern "C" {
#endif

#undef DDS_EXPORT
#if VDDS_BUILD
#define DDS_EXPORT OS_API_EXPORT
#else
#define DDS_EXPORT OS_API_IMPORT
#endif


/* Listener callbacks */
typedef void (*dds_on_inconsistent_topic_fn) (dds_entity_t topic, const dds_inconsistent_topic_status_t status);
typedef void (*dds_on_liveliness_lost_fn) (dds_entity_t writer, const dds_liveliness_lost_status_t status);
typedef void (*dds_on_offered_deadline_missed_fn) (dds_entity_t writer, const dds_offered_deadline_missed_status_t status);
typedef void (*dds_on_offered_incompatible_qos_fn) (dds_entity_t writer, const dds_offered_incompatible_qos_status_t status);
typedef void (*dds_on_data_on_readers_fn) (dds_entity_t subscriber);
typedef void (*dds_on_sample_lost_fn) (dds_entity_t reader, const dds_sample_lost_status_t status);
typedef void (*dds_on_data_available_fn) (dds_entity_t reader);
typedef void (*dds_on_sample_rejected_fn) (dds_entity_t reader, const dds_sample_rejected_status_t status);
typedef void (*dds_on_liveliness_changed_fn) (dds_entity_t reader, const dds_liveliness_changed_status_t status);
typedef void (*dds_on_requested_deadline_missed_fn) (dds_entity_t reader, const dds_requested_deadline_missed_status_t status);
typedef void (*dds_on_requested_incompatible_qos_fn) (dds_entity_t reader, const dds_requested_incompatible_qos_status_t status);
typedef void (*dds_on_publication_matched_fn) (dds_entity_t writer, const dds_publication_matched_status_t  status);
typedef void (*dds_on_subscription_matched_fn) (dds_entity_t reader, const dds_subscription_matched_status_t  status);

#if 0
/* TODO: Why use (*dds_on_any_fn) (); and DDS_LUNSET? Why not just set the callbacks to NULL? */
typedef void (*dds_on_any_fn) (); /**< Empty parameter list on purpose; should be assignable without cast to all of the above. @todo check with an actual compiler; I'm a sloppy compiler */
#define DDS_LUNSET ((dds_on_any_fn)1) /**< Callback indicating a callback isn't set */
#else
#define DDS_LUNSET (NULL)
#endif

struct c99_listener;
typedef struct c99_listener dds_listener_t;

/**
 * @brief Allocate memory and initializes to default values (::DDS_LUNSET) of a listener
 *
 * @return Returns a pointer to the allocated memory for dds_listener_t structure.
 */
_Ret_notnull_
DDS_EXPORT dds_listener_t* dds_listener_create (void);

/**
 * @brief Delete the memory allocated to listener structure
 *
 * @param[in] listener pointer to the listener struct to delete
 */
DDS_EXPORT void dds_listener_delete (_In_ _Post_invalid_ dds_listener_t * restrict listener);

/**
 * @brief Reset the listener structure contents to ::DDS_LUNSET
 *
 * @param[in,out] listener pointer to the listener struct to reset
 */
DDS_EXPORT void dds_listener_reset (_Inout_ dds_listener_t * restrict listener);

/**
 * @brief Copy the listener callbacks from source to destination
 *
 * @param[in,out] dst The pointer to the destination listener structure, where the content is to copied
 * @param[in] src The pointer to the source listener structure to be copied
 */
DDS_EXPORT void dds_listener_copy (_Inout_ dds_listener_t * restrict dst, _In_ const dds_listener_t * restrict src);

/**
 * @brief Copy the listener callbacks from source to destination, unless already set
 *
 * Any listener callbacks already set in @p dst (including NULL) are skipped, only
 * those set to DDS_LUNSET are copied from @p src.
 *
 * @param[in,out] dst The pointer to the destination listener structure, where the content is merged
 * @param[in] src The pointer to the source listener structure to be copied
 */
DDS_EXPORT void dds_listener_merge (_Inout_ dds_listener_t * restrict dst, _In_ const dds_listener_t * restrict src);


/************************************************************************************************
 *  Setters
 ************************************************************************************************/

/**
 * @brief Set the inconsistent_topic callback in the listener structure.
 *
 * @param listener The pointer to the listener structure, where the callback will be set
 * @param callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_inconsistent_topic (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_inconsistent_topic_fn callback);

/**
 * @brief Set the liveliness_lost callback in the listener structure.
 *
 * @param[out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_liveliness_lost (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_liveliness_lost_fn callback);

/**
 * @brief Set the offered_deadline_missed callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_offered_deadline_missed (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_offered_deadline_missed_fn callback);

/**
 * @brief Set the offered_incompatible_qos callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_offered_incompatible_qos (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_offered_incompatible_qos_fn callback);

/**
 * @brief Set the data_on_readers callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_data_on_readers (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_data_on_readers_fn callback);

/**
 * @brief Set the sample_lost callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_sample_lost (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_sample_lost_fn callback);

/**
 * @brief Set the data_available callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_data_available (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_data_available_fn callback);

/**
 * @brief Set the sample_rejected callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_sample_rejected (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_sample_rejected_fn callback);

/**
 * @brief Set the liveliness_changed callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_liveliness_changed (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_liveliness_changed_fn callback);

/**
 * @brief Set the requested_deadline_missed callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_requested_deadline_missed (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_requested_deadline_missed_fn callback);

/**
 * @brief Set the requested_incompatible_qos callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_requested_incompatible_qos (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_requested_incompatible_qos_fn callback);

/**
 * @brief Set the publication_matched callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_publication_matched (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_publication_matched_fn callback);

/**
 * @brief Set the subscription_matched callback in the listener structure.
 *
 * @param[in,out] listener The pointer to the listener structure, where the callback will be set
 * @param[in] callback The callback to set in the listener, can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lset_subscription_matched (_Inout_ dds_listener_t * restrict listener, _In_opt_ dds_on_subscription_matched_fn callback);


/************************************************************************************************
 *  Getters
 ************************************************************************************************/

/**
 * @brief Get the inconsistent_topic callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_inconsistent_topic (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_inconsistent_topic_fn *callback);

/**
 * @brief Get the liveliness_lost callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_liveliness_lost (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_liveliness_lost_fn *callback);

/**
 * @brief Get the offered_deadline_missed callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_offered_deadline_missed (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_offered_deadline_missed_fn *callback);

/**
 * @brief Get the offered_incompatible_qos callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_offered_incompatible_qos (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_offered_incompatible_qos_fn *callback);

/**
 * @brief Get the data_on_readers callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_data_on_readers (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_data_on_readers_fn *callback);

/**
 * @brief Get the sample_lost callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_sample_lost (_In_ const dds_listener_t *restrict listener, _Outptr_result_maybenull_ dds_on_sample_lost_fn *callback);

/**
 * @brief Get the data_available callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_data_available (_In_ const dds_listener_t *restrict listener, _Outptr_result_maybenull_ dds_on_data_available_fn *callback);

/**
 * @brief Get the sample_rejected callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_sample_rejected (_In_ const dds_listener_t  *restrict listener, _Outptr_result_maybenull_ dds_on_sample_rejected_fn *callback);

/**
 * @brief Get the liveliness_changed callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_liveliness_changed (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_liveliness_changed_fn *callback);

/**
 * @brief Get the requested_deadline_missed callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_requested_deadline_missed (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_requested_deadline_missed_fn *callback);

/**
 * @brief Get the requested_incompatible_qos callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_requested_incompatible_qos (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_requested_incompatible_qos_fn *callback);

/**
 * @brief Get the publication_matched callback from the listener structure.
 *
 * @param[in] listener The pointer to the listener structure, where the callback will be retrieved from
 * @param[in,out] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 */
DDS_EXPORT void dds_lget_publication_matched (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_publication_matched_fn *callback);

/**
 * @brief Get the subscription_matched callback from the listener structure.
 *
 * @param[in] callback Pointer where the retrieved callback can be stored; can be NULL, ::DDS_LUNSET or a valid callback pointer
 * @param[in,out] listener The pointer to the listener structure, where the callback will be retrieved from
 */
DDS_EXPORT void dds_lget_subscription_matched (_In_ const dds_listener_t * restrict listener, _Outptr_result_maybenull_ dds_on_subscription_matched_fn *callback);

#undef DDS_EXPORT

#if defined (__cplusplus)
}
#endif

#endif /*_DDS_PUBLIC_LISTENER_H_*/
