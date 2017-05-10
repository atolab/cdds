
#include <assert.h>
#include "kernel/dds_listener.h"




/* TODO: Merge the CHAM-65 Listener stuff. */
typedef void (*dds_on_any_fn) (); /**< Empty parameter list on purpose; should be assignable without cast to all of the above. @todo check with an actual compiler; I'm a sloppy compiler */
#define DDS_LUNSET ((dds_on_any_fn)1) /**< Callback indicating a callback isn't set */
dds_listener_t
dds_listener_create(void)
{
    dds_listener_cham65_t *l = dds_alloc(sizeof(dds_listener_cham65_t));
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
    os_mutexInit(&l->m_mutex);
    return (dds_listener_t*)l;
}

void
dds_listener_delete(_In_ _Post_invalid_ dds_listener_t __restrict l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    if (listener) {
        os_mutexDestroy(&listener->m_mutex);
        dds_free(listener);
    }
}

void dds_listener_lock (_In_ _Post_invalid_ dds_listener_t __restrict l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    os_mutexLock (&listener->m_mutex);
}

/* TODO: Lock/unlock listener when setting callback functions. */
void dds_listener_unlock (_In_ _Post_invalid_ dds_listener_t __restrict l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    os_mutexLock (&listener->m_mutex);
}

void
dds_listener_copy(_Inout_ dds_listener_t __restrict dst, _In_ const dds_listener_t __restrict src)
{
    if (src && dst) {
        const c99_listener_cham65_t *srcl = src;
        c99_listener_cham65_t *dstl = dst;

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

