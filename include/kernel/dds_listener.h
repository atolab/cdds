#ifndef _DDS_LISTENER_H_
#define _DDS_LISTENER_H_

#include "dds/dds_public_impl.h"
#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

void dds_listener_merge (dds_listener_t dst, const dds_listener_t src, dds_entity_kind_t kind);
void dds_listener_delete (_In_ _Post_invalid_ dds_listener_t __restrict listener);
void dds_listener_lock (_In_ _Post_invalid_ dds_listener_t __restrict listener);
void dds_listener_unlock (_In_ _Post_invalid_ dds_listener_t __restrict listener);


/* TODO: Merge the CHAM-65 Listener stuff. */
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

typedef struct c99_listener_cham65 {
    dds_on_inconsistent_topic_fn on_inconsistent_topic;
    dds_on_liveliness_lost_fn on_liveliness_lost;
    dds_on_offered_deadline_missed_fn on_offered_deadline_missed;
    dds_on_offered_incompatible_qos_fn on_offered_incompatible_qos;
    dds_on_data_on_readers_fn on_data_on_readers;
    dds_on_sample_lost_fn on_sample_lost;
    dds_on_data_available_fn on_data_available;
    dds_on_sample_rejected_fn on_sample_rejected;
    dds_on_liveliness_changed_fn on_liveliness_changed;
    dds_on_requested_deadline_missed_fn on_requested_deadline_missed;
    dds_on_requested_incompatible_qos_fn on_requested_incompatible_qos;
    dds_on_publication_matched_fn on_publication_matched;
    dds_on_subscription_matched_fn on_subscription_matched;
    os_mutex m_mutex;
} c99_listener_cham65_t;


#if defined (__cplusplus)
}
#endif
#endif
