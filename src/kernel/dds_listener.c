
#include <assert.h>
#include "dds.h"
#include "kernel/dds_listener.h"




/* TODO: Merge the CHAM-65 Listener stuff. */
typedef void (*dds_on_any_fn) (); /**< Empty parameter list on purpose; should be assignable without cast to all of the above. @todo check with an actual compiler; I'm a sloppy compiler */
#define DDS_LUNSET ((dds_on_any_fn)1) /**< Callback indicating a callback isn't set */
dds_listener_t
dds_listener_create(void)
{
    c99_listener_cham65_t *l = dds_alloc(sizeof(c99_listener_cham65_t));
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
dds_listener_delete(_In_ _Post_invalid_ dds_listener_t l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    if (listener) {
        os_mutexDestroy(&listener->m_mutex);
        dds_free(listener);
    }
}

void dds_listener_lock (_In_ _Post_invalid_ dds_listener_t l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    os_mutexLock (&listener->m_mutex);
}

/* TODO: Lock/unlock listener when setting callback functions. */
void dds_listener_unlock (_In_ _Post_invalid_ dds_listener_t l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    os_mutexLock (&listener->m_mutex);
}


