/**
 * @file dds_listener.h
 * @brief DDS C99 Listener API
 *
 * @todo add copyright header?
 *
 * This file contains the listeners implementation of the VortexDDS C99 language binding.
 */
#include <assert.h>
#include "dds.h"
#include "kernel/dds_listener.h"



_Ret_notnull_
dds_listener_t*
dds_listener_create(_In_opt_ void* arg)
{
    c99_listener_t *l = dds_alloc(sizeof(*l));
    dds_listener_reset(l);
    l->arg = arg;
    return l;
}

void
dds_listener_delete(_In_ _Post_invalid_ dds_listener_t * __restrict listener)
{
    if (listener) {
        dds_free(listener);
    }
}


void
dds_listener_reset(_Inout_ dds_listener_t * __restrict listener)
{
    if (listener) {
        c99_listener_t *l = listener;
        l->on_data_available = DDS_LUNSET;
        l->on_data_on_readers = DDS_LUNSET;
        l->on_inconsistent_topic = DDS_LUNSET;
        l->on_liveliness_changed = DDS_LUNSET;
        l->on_liveliness_lost = DDS_LUNSET;
        l->on_offered_deadline_missed = DDS_LUNSET;
        l->on_offered_incompatible_qos = DDS_LUNSET;
        l->on_publication_matched = DDS_LUNSET;
        l->on_requested_deadline_missed = DDS_LUNSET;
        l->on_requested_incompatible_qos = DDS_LUNSET;
        l->on_sample_lost = DDS_LUNSET;
        l->on_sample_rejected = DDS_LUNSET;
        l->on_subscription_matched = DDS_LUNSET;
    }
}

void
dds_listener_copy(_Inout_ dds_listener_t * __restrict dst, _In_ const dds_listener_t * __restrict src)
{
    if (src && dst) {
        const c99_listener_t *srcl = src;
        c99_listener_t *dstl = dst;

        dstl->on_data_available = srcl->on_data_available;
        dstl->on_data_on_readers = srcl->on_data_on_readers;
        dstl->on_inconsistent_topic = srcl->on_inconsistent_topic;
        dstl->on_liveliness_changed = srcl->on_liveliness_changed;
        dstl->on_liveliness_lost = srcl->on_liveliness_lost;
        dstl->on_offered_deadline_missed = srcl->on_offered_deadline_missed;
        dstl->on_offered_incompatible_qos = srcl->on_offered_incompatible_qos;
        dstl->on_publication_matched = srcl->on_publication_matched;
        dstl->on_requested_deadline_missed = srcl->on_requested_deadline_missed;
        dstl->on_requested_incompatible_qos = srcl->on_requested_incompatible_qos;
        dstl->on_sample_lost = srcl->on_sample_lost;
        dstl->on_sample_rejected = srcl->on_sample_rejected;
        dstl->on_subscription_matched = srcl->on_subscription_matched;
    }
}

void
dds_listener_merge (_Inout_ dds_listener_t * __restrict dst, _In_ const dds_listener_t * __restrict src)
{
    if (src && dst) {
        const c99_listener_t *srcl = src;
        c99_listener_t *dstl = dst;

        if (dstl->on_data_available == DDS_LUNSET) { dstl->on_data_available = srcl->on_data_available; }
        if (dstl->on_data_on_readers == DDS_LUNSET) { dstl->on_data_on_readers = srcl->on_data_on_readers; }
        if (dstl->on_inconsistent_topic == DDS_LUNSET) { dstl->on_inconsistent_topic = srcl->on_inconsistent_topic; }
        if (dstl->on_liveliness_changed == DDS_LUNSET) { dstl->on_liveliness_changed = srcl->on_liveliness_changed; }
        if (dstl->on_liveliness_lost == DDS_LUNSET) { dstl->on_liveliness_lost = srcl->on_liveliness_lost; }
        if (dstl->on_offered_deadline_missed == DDS_LUNSET) { dstl->on_offered_deadline_missed = srcl->on_offered_deadline_missed; }
        if (dstl->on_offered_incompatible_qos == DDS_LUNSET) { dstl->on_offered_incompatible_qos = srcl->on_offered_incompatible_qos; }
        if (dstl->on_publication_matched == DDS_LUNSET) { dstl->on_publication_matched = srcl->on_publication_matched; }
        if (dstl->on_requested_deadline_missed == DDS_LUNSET) { dstl->on_requested_deadline_missed = srcl->on_requested_deadline_missed; }
        if (dstl->on_requested_incompatible_qos == DDS_LUNSET) { dstl->on_requested_incompatible_qos = srcl->on_requested_incompatible_qos; }
        if (dstl->on_sample_lost == DDS_LUNSET) { dstl->on_sample_lost = srcl->on_sample_lost; }
        if (dstl->on_sample_rejected == DDS_LUNSET) { dstl->on_sample_rejected = srcl->on_sample_rejected; }
        if (dstl->on_subscription_matched == DDS_LUNSET) { dstl->on_subscription_matched = srcl->on_subscription_matched; }
    }
}

/************************************************************************************************
 *  Setters
 ************************************************************************************************/

void
dds_lset_data_available (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_data_available_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_data_available = callback;
    }
}

void
dds_lset_data_on_readers (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_data_on_readers_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_data_on_readers = callback;
    }
}

void
dds_lset_inconsistent_topic (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_inconsistent_topic_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_inconsistent_topic = callback;
    }
}

void
dds_lset_liveliness_changed (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_liveliness_changed_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_liveliness_changed = callback;
    }
}

void
dds_lset_liveliness_lost (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_liveliness_lost_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_liveliness_lost = callback;
    }
}

void
dds_lset_offered_deadline_missed (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_offered_deadline_missed_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_offered_deadline_missed = callback;
    }
}

void
dds_lset_offered_incompatible_qos (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_offered_incompatible_qos_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_offered_incompatible_qos = callback;
    }
}

void
dds_lset_publication_matched (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_publication_matched_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_publication_matched = callback;
    }
}

void
dds_lset_requested_deadline_missed (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_requested_deadline_missed_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_requested_deadline_missed = callback;
    }
}

void
dds_lset_requested_incompatible_qos (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_requested_incompatible_qos_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_requested_incompatible_qos = callback;
    }
}

void
dds_lset_sample_lost (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_sample_lost_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_sample_lost = callback;
    }
}

void
dds_lset_sample_rejected (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_sample_rejected_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_sample_rejected = callback;
    }
}

void
dds_lset_subscription_matched (_Inout_ dds_listener_t * __restrict listener, _In_opt_ dds_on_subscription_matched_fn callback)
{
    if (listener) {
        ((c99_listener_t*)listener)->on_subscription_matched = callback;
    }
}

/************************************************************************************************
 *  Getters
 ************************************************************************************************/

void
dds_lget_data_available (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_data_available_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_data_available;
    }
}

void
dds_lget_data_on_readers (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_data_on_readers_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_data_on_readers;
    }
}

void dds_lget_inconsistent_topic (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_inconsistent_topic_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_inconsistent_topic;
    }
}

void
dds_lget_liveliness_changed (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_liveliness_changed_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_liveliness_changed;
    }
}

void
dds_lget_liveliness_lost (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_liveliness_lost_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_liveliness_lost;
    }
}

void
dds_lget_offered_deadline_missed (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_offered_deadline_missed_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_offered_deadline_missed;
    }
}

void
dds_lget_offered_incompatible_qos (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_offered_incompatible_qos_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_offered_incompatible_qos;
    }
}

void
dds_lget_publication_matched (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_publication_matched_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_publication_matched;
    }
}

void
dds_lget_requested_deadline_missed (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_requested_deadline_missed_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_requested_deadline_missed;
    }
}

void
dds_lget_requested_incompatible_qos (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_requested_incompatible_qos_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_requested_incompatible_qos;
    }
}

void
dds_lget_sample_lost (_In_ const dds_listener_t *__restrict listener, _Outptr_result_maybenull_ dds_on_sample_lost_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_sample_lost;
    }
}

void
dds_lget_sample_rejected (_In_ const dds_listener_t  *__restrict listener, _Outptr_result_maybenull_ dds_on_sample_rejected_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_sample_rejected;
    }
}

void
dds_lget_subscription_matched (_In_ const dds_listener_t * __restrict listener, _Outptr_result_maybenull_ dds_on_subscription_matched_fn *callback)
{
    if (callback && listener) {
        *callback = ((c99_listener_t*)listener)->on_subscription_matched;
    }
}
