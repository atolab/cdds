#include <assert.h>
#include "kernel/dds_listener.h"

void dds_listener_get (dds_entity_t e, dds_listener_t l)
{
  os_mutexLock (&e->m_mutex);
  dds_listener_get_unl (e, l);
  os_mutexUnlock (&e->m_mutex);
}

void dds_listener_get_unl (dds_entity_t e, dds_listener_t l)
{
  assert (e);
  assert (l);

  switch (e->m_kind)
  {
    case DDS_TYPE_TOPIC: *((dds_topiclistener_t*) l) = ((dds_topic*) e)->m_listener; break;
    case DDS_TYPE_PARTICIPANT: *((dds_participantlistener_t*) l) = ((dds_participant*) e)->m_listener; break;
    case DDS_TYPE_READER: *((dds_readerlistener_t*) l) = ((dds_reader*) e)->m_listener; break;
    case DDS_TYPE_WRITER: *((dds_writerlistener_t*) l) = ((dds_writer*) e)->m_listener; break;
    case DDS_TYPE_SUBSCRIBER: *((dds_subscriberlistener_t*) l) = ((dds_subscriber*) e)->m_listener; break;
    case DDS_TYPE_PUBLISHER: *((dds_publisherlistener_t*) l) = ((dds_publisher*) e)->m_listener; break;
  }
}

void dds_listener_set (dds_entity_t e, const dds_listener_t l)
{
  os_mutexLock (&e->m_mutex);
  dds_listener_set_unl (e, l);
  os_mutexUnlock (&e->m_mutex);
}

void dds_listener_set_unl (dds_entity_t e, const dds_listener_t l)
{
  assert (e);
  assert (l);

  switch (e->m_kind)
  {
    case DDS_TYPE_TOPIC: ((dds_topic*) e)->m_listener = *(dds_topiclistener_t*) l; break;
    case DDS_TYPE_PARTICIPANT: ((dds_participant*) e)->m_listener = *(dds_participantlistener_t*) l; break;
    case DDS_TYPE_READER: ((dds_reader*) e)->m_listener = *(dds_readerlistener_t*) l; break;
    case DDS_TYPE_WRITER: ((dds_writer*) e)->m_listener = *(dds_writerlistener_t*) l; break;
    case DDS_TYPE_SUBSCRIBER: ((dds_subscriber*) e)->m_listener = *(dds_subscriberlistener_t*) l; break;
    case DDS_TYPE_PUBLISHER: ((dds_publisher*) e)->m_listener = *(dds_publisherlistener_t*) l; break;
  }
}

/*
  dds_listener_merge: Merge "dst" listener functions with those of "src". Only merge function
  pointers that are not already set in "dst".
*/

void dds_listener_merge (dds_listener_t dst, const dds_listener_t src, dds_entity_kind_t kind)
{
  assert (dst);
  assert (src);

#define DDS_LMERGE(n) if (d->n == NULL) d->n = s->n

  switch (kind)
  {
    case DDS_TYPE_TOPIC:
    {
      dds_topiclistener_t * d = (dds_topiclistener_t*) dst;
      dds_topiclistener_t * s = (dds_topiclistener_t*) src;
      DDS_LMERGE (on_inconsistent_topic);
      break;
    }
    case DDS_TYPE_PARTICIPANT:
    {
      dds_participantlistener_t * d = (dds_participantlistener_t*) dst;
      dds_participantlistener_t * s = (dds_participantlistener_t*) src;
      dds_listener_merge (&d->topiclistener, &s->topiclistener, DDS_TYPE_TOPIC);
      dds_listener_merge (&d->publisherlistener, &s->publisherlistener, DDS_TYPE_PUBLISHER);
      dds_listener_merge (&d->subscriberlistener, &s->subscriberlistener, DDS_TYPE_SUBSCRIBER);
      break;
    }
    case DDS_TYPE_READER:
    {
      dds_readerlistener_t * d = (dds_readerlistener_t*) dst;
      dds_readerlistener_t * s = (dds_readerlistener_t*) src;
      DDS_LMERGE (on_requested_deadline_missed);
      DDS_LMERGE (on_requested_incompatible_qos);
      DDS_LMERGE (on_sample_rejected);
      DDS_LMERGE (on_liveliness_changed);
      DDS_LMERGE (on_data_available);
      DDS_LMERGE (on_subscription_matched);
      DDS_LMERGE (on_sample_lost);
      break;
    }
    case DDS_TYPE_WRITER:
    {
      dds_writerlistener_t * d = (dds_writerlistener_t*) dst;
      dds_writerlistener_t * s = (dds_writerlistener_t*) src;
      DDS_LMERGE (on_offered_deadline_missed);
      DDS_LMERGE (on_offered_incompatible_qos);
      DDS_LMERGE (on_liveliness_lost);
      DDS_LMERGE (on_publication_matched);
      break;
    }
    case DDS_TYPE_SUBSCRIBER:
    {
      dds_subscriberlistener_t * d = (dds_subscriberlistener_t*) dst;
      dds_subscriberlistener_t * s = (dds_subscriberlistener_t*) src;
      DDS_LMERGE (on_data_readers);
      dds_listener_merge (&d->readerlistener, &s->readerlistener, DDS_TYPE_READER);
      break;
    }
    case DDS_TYPE_PUBLISHER:
    {
      dds_publisherlistener_t * d = (dds_publisherlistener_t*) dst;
      dds_publisherlistener_t * s = (dds_publisherlistener_t*) src;
      dds_listener_merge (&d->writerlistener, &s->writerlistener, DDS_TYPE_WRITER);
      break;
    }
  }

#undef DDS_LMERGE
}



/* TODO: Merge the CHAM-65 Listener stuff. */

#define DDS_LUNSET ((dds_on_any_fn)1) /**< Callback indicating a callback isn't set */
dds_listener_cham65_t*
dds_listener_create(void)
{
    dds_listener_cham65_t *l = dds_alloc(sizeof(*l));
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
    //os_mutexInit (&l->m_mutex);
    return l;
}

void dds_listener_lock (_In_ _Post_invalid_ dds_listener_t * restrict l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    //os_mutexLock (&listener->m_mutex);
}

void dds_listener_unlock (_In_ _Post_invalid_ dds_listener_t * restrict l)
{
    c99_listener_cham65_t *listener = (c99_listener_cham65_t*)l;
    assert(listener);
    //os_mutexLock (&listener->m_mutex);
}

void
dds_listener_copy(_Inout_ dds_listener_cham65_t * restrict dst, _In_ const dds_listener_cham65_t * restrict src)
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

