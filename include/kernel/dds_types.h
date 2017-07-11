#ifndef _DDS_TYPES_H_
#define _DDS_TYPES_H_

/* DDS internal type definitions */

#include "os/os.h"
#include "dds.h"
#include "ddsi/q_rtps.h"
#include "util/ut_avl.h"
#include "util/ut_handleserver.h"

#if defined (__cplusplus)
extern "C" {
#endif


struct dds_domain;
struct dds_entity;
struct dds_participant;
struct dds_reader;
struct dds_writer;
struct dds_publisher;
struct dds_subscriber;
struct dds_topic;
struct dds_readcond;
struct dds_guardcond;

struct sertopic;
struct rhc;

/* Internal entity status flags */

#define DDS_INTERNAL_STATUS_MASK     (0xFF000000)

#define DDS_WAITSET_TRIGGER_STATUS   (0x01000000)
#define DDS_DELETING_STATUS          (0x02000000)


/* Module identifiers for error codes */

#define DDS_MOD_QOS 0x0100
#define DDS_MOD_KERNEL 0x0200
#define DDS_MOD_DDSI 0x0300
#define DDS_MOD_STREAM 0x0400
#define DDS_MOD_ALLOC 0x0500
#define DDS_MOD_WAITSET 0x0600
#define DDS_MOD_READER 0x0700
#define DDS_MOD_WRITER 0x0800
#define DDS_MOD_COND 0x0900
#define DDS_MOD_RHC 0x0a00
#define DDS_MOD_STATUS 0x0b00
#define DDS_MOD_THREAD 0x0c00
#define DDS_MOD_INST 0x0d00
#define DDS_MOD_PPANT 0x0e00
#define DDS_MOD_ENTITY 0x0f00
#define DDS_MOD_TOPIC 0x1000

/* Minor numbers for error codes */

#define DDS_ERR_M1 0x010000
#define DDS_ERR_M2 0x020000
#define DDS_ERR_M3 0x030000
#define DDS_ERR_M4 0x040000
#define DDS_ERR_M5 0x050000
#define DDS_ERR_M6 0x060000
#define DDS_ERR_M7 0x070000
#define DDS_ERR_M8 0x080000
#define DDS_ERR_M9 0x090000
#define DDS_ERR_M10 0x0A0000
#define DDS_ERR_M11 0x0B0000
#define DDS_ERR_M12 0x0C0000
#define DDS_ERR_M13 0x0D0000
#define DDS_ERR_M14 0x0E0000
#define DDS_ERR_M15 0x0F0000
#define DDS_ERR_M16 0x100000
#define DDS_ERR_M17 0x110000
#define DDS_ERR_M18 0x120000
#define DDS_ERR_M19 0x130000
#define DDS_ERR_M20 0x140000

/* To construct return status
 * Use '+' instead of '|'. Otherwise, the SAL checking doesn't
 * understand when a return value is negative or positive and
 * complains a lot about "A successful path through the function
 * does not set the named _Out_ parameter." */
#define DDS_ERRNO(e,m,n) ((e <= 0) ? e : -((n) + (m) + (e)))


typedef bool (*dds_querycondition_filter_with_ctx_fn) (const void * sample, const void *ctx);


/* The listener struct. */

typedef struct c_listener {
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
    void *arg;
} c_listener_t;

/* Entity flag values */

#define DDS_ENTITY_ENABLED      0x0001

typedef struct dds_domain
{
  ut_avlNode_t m_node;
  dds_domainid_t m_id;
  ut_avlTree_t m_topics;
  uint32_t m_refc;
}
dds_domain;

struct dds_entity;
typedef struct dds_entity_deriver {
    /* Close can be used to terminate (blocking) actions on a entity before actually deleting it. */
    dds_return_t (*close)(struct dds_entity *e);
    /* Delete is used to actually free the entity. */
    dds_return_t (*delete)(struct dds_entity *e);
    dds_return_t (*set_qos)(struct dds_entity *e, const dds_qos_t *qos, bool enabled);
    dds_return_t (*validate_status)(uint32_t mask);
    dds_return_t (*propagate_status)(struct dds_entity *e, uint32_t mask, bool set);
    dds_return_t (*get_instance_hdl)(struct dds_entity *e, dds_instance_handle_t *i);
}
dds_entity_deriver;

typedef void (*dds_entity_callback)(dds_entity_t observer, dds_entity_t observed, uint32_t status);

typedef struct dds_entity_observer
{
    dds_entity_callback m_cb;
    dds_entity_t m_observer;
    struct dds_entity_observer *m_next;
}
dds_entity_observer;

typedef struct dds_entity
{
  ut_handle_t m_hdl;
  dds_entity_deriver m_deriver;
  uint32_t m_refc;
  struct dds_entity * m_next;
  struct dds_entity * m_parent;
  struct dds_entity * m_children;
  struct dds_entity * m_participant;
  struct dds_domain * m_domain;
  dds_qos_t * m_qos;
  dds_domainid_t m_domainid;
  nn_guid_t m_guid;
  uint32_t m_status_enable;
  uint32_t m_flags;
  uint32_t m_cb_count;
  os_mutex m_mutex;
  os_cond m_cond;
  c_listener_t m_listener;
  uint32_t m_trigger;
  dds_entity_observer *m_observers;
  struct ut_handlelink *m_hdllink;
}
dds_entity;

extern const ut_avlTreedef_t dds_topictree_def;

typedef struct dds_participant
{
  struct dds_entity m_entity;
  struct dds_entity * m_dur_reader;
  struct dds_entity * m_dur_writer;
}
dds_participant;

typedef struct dds_reader
{
  struct dds_entity m_entity;
  const struct dds_topic * m_topic;
  struct reader * m_rd;
  bool m_data_on_readers;
  bool m_loan_out;
  char * m_loan;
  uint32_t m_loan_size;

  /* Status metrics */

  dds_sample_rejected_status_t m_sample_rejected_status;
  dds_liveliness_changed_status_t m_liveliness_changed_status;
  dds_requested_deadline_missed_status_t m_requested_deadline_missed_status;
  dds_requested_incompatible_qos_status_t m_requested_incompatible_qos_status;
  dds_sample_lost_status_t m_sample_lost_status;
  dds_subscription_matched_status_t m_subscription_matched_status;
}
dds_reader;

typedef struct dds_writer
{
  struct dds_entity m_entity;
  const struct dds_topic * m_topic;
  struct nn_xpack * m_xp;
  struct writer * m_wr;
  os_mutex m_call_lock;

  /* Status metrics */

  dds_liveliness_lost_status_t m_liveliness_lost_status;
  dds_offered_deadline_missed_status_t m_offered_deadline_missed_status;
  dds_offered_incompatible_qos_status_t m_offered_incompatible_qos_status;
  dds_publication_matched_status_t m_publication_matched_status;
}
dds_writer;

typedef struct dds_subscriber
{
  struct dds_entity m_entity;
}
dds_subscriber;

typedef struct dds_publisher
{
  struct dds_entity m_entity;
}
dds_publisher;

typedef struct dds_topic
{
  struct dds_entity m_entity;
  struct sertopic * m_stopic;
  const dds_topic_descriptor_t * m_descriptor;

  /* Status metrics */

  dds_inconsistent_topic_status_t m_inconsistent_topic_status;
}
dds_topic;

typedef struct dds_readcond
{
  dds_entity m_entity;
  struct rhc * m_rhc;
  uint32_t m_qminv;
  uint32_t m_sample_states;
  uint32_t m_view_states;
  uint32_t m_instance_states;
  nn_guid_t m_rd_guid;
  struct dds_readcond * m_rhc_next;
  struct
  {
      dds_querycondition_filter_fn m_filter;
  } m_query;
}
dds_readcond;

typedef struct dds_attachment
{
    dds_entity  *entity;
    dds_attach_t arg;
    struct dds_attachment* next;
}
dds_attachment;

typedef struct dds_waitset
{
  dds_entity m_entity;
  dds_attachment *observed;
  dds_attachment *triggered;
}
dds_waitset;

typedef struct dds_iid
{
  uint64_t counter;
  uint32_t key[4];
}
dds_iid;

/* Globals */

typedef struct dds_globals
{
  dds_domainid_t m_default_domain;
  os_atomic_uint32_t m_init_count;
  void (*m_dur_reader) (struct dds_reader * reader, struct rhc * rhc);
  int (*m_dur_wait) (struct dds_reader * reader, dds_duration_t timeout);
  void (*m_dur_init) (void);
  void (*m_dur_fini) (void);
  ut_avlTree_t m_domains;
  os_mutex m_mutex;
}
dds_globals;

DDS_EXPORT dds_globals dds_global;

#if defined (__cplusplus)
}
#endif
#endif
