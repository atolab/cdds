#ifndef _DDS_TYPES_H_
#define _DDS_TYPES_H_

/* DDS internal type definitions */

#include "os/os.h"
#include "dds.h"
#include "ddsi/q_rtps.h"
#include "util/ut_avl.h"

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

/* To construct return status */

#define DDS_ERRNO(e,m,n) (-((n) | (m) | (e)))

/* Bit flags for entity kind */

#define DDS_IS_PP_OR_SUB 0x00010000 /* Is a participant or subscriber */
#define DDS_IS_PP_OR_PUB 0x00020000 /* Is a participant or publisher */
#define DDS_IS_RD_OR_WR  0x00040000 /* Is a reader or writer */
#define DDS_IS_MAPPED    0x00080000 /* Is mapped via guid to DDSI type */
#define DDS_HAS_SCOND    0x00100000 /* Has an associated status condition */

/*
  Have separate kinds for entity and condition, matching DDS type
  hierarchy. Have distinct values so will detect if an entity is passed
  as a condition or vice versa.
*/

typedef enum dds_entity_kind
{
  DDS_TYPE_TOPIC       = 0x00000000 | DDS_HAS_SCOND,
  DDS_TYPE_PARTICIPANT = 0x00000001 | DDS_IS_MAPPED | DDS_IS_PP_OR_SUB | DDS_IS_PP_OR_PUB,
  DDS_TYPE_READER      = 0x00000002 | DDS_IS_MAPPED | DDS_HAS_SCOND | DDS_IS_RD_OR_WR,
  DDS_TYPE_WRITER      = 0x00000003 | DDS_IS_MAPPED | DDS_HAS_SCOND | DDS_IS_RD_OR_WR,
  DDS_TYPE_SUBSCRIBER  = 0x00000004 | DDS_IS_PP_OR_SUB | DDS_HAS_SCOND,
  DDS_TYPE_PUBLISHER   = 0x00000005 | DDS_IS_PP_OR_PUB
}
dds_entity_kind_t;

#define DDS_ENTITY_NUM 6
#define DDS_TYPE_INDEX_MASK 0x0000000f /* To use entity kind as an array index */
#define DDS_TYPE_INDEX_COUNT (DDS_TYPE_INDEX_MASK + 1)

typedef enum dds_cond_kind
{
  DDS_TYPE_COND_GUARD  = 0x00000040,
  DDS_TYPE_COND_READ   = 0x00000080,
  DDS_TYPE_COND_QUERY  = 0x00000100|DDS_TYPE_COND_READ,
  DDS_TYPE_COND_STATUS = 0x00000200
}
dds_cond_kind_t;

/* Link between waitset and condition */

typedef struct dds_ws_cond_link
{
  dds_attach_t m_attach;
  struct dds_waitset * m_waitset;
  struct dds_condition * m_cond;
  struct dds_ws_cond_link * m_ws_next;
  struct dds_ws_cond_link * m_cond_next;
}
dds_ws_cond_link;

typedef struct dds_waitset
{
  os_mutex conds_lock;
  os_cond cv;
  uint64_t triggered;
  uint32_t timeout_counter;
  uint32_t m_nconds;
  dds_ws_cond_link * m_conds;
  void (*block) (struct dds_waitset *ws, void *arg, dds_time_t abstimeout);
  void (*cont) (struct dds_waitset *ws, void *arg, int ret);
  size_t trp_nxs;
  dds_attach_t *trp_xs;
  dds_time_t trp_abstimeout;
}
dds_waitset;

typedef struct dds_condition
{
  dds_cond_kind_t m_kind;
  uint32_t m_trigger;
  dds_ws_cond_link * m_waitsets;
  os_mutex * m_lock;
  void * m_cpp;
}
dds_condition;

typedef bool (*dds_querycondition_filter_with_ctx_fn) (const void * sample, const void *ctx);

typedef struct dds_readcond
{
  dds_condition m_cond;
  struct rhc * m_rhc;
  uint32_t m_qminv;
  uint32_t m_sample_states;
  uint32_t m_view_states;
  uint32_t m_instance_states;
  nn_guid_t m_rd_guid;
  struct dds_readcond * m_rhc_next;
  struct
  {
    void * m_cxx_ctx;
    union
    {
      dds_querycondition_filter_fn m_filter;
      dds_querycondition_filter_with_ctx_fn m_filter_with_ctx;
    } u;
  } m_query;
}
dds_readcond;

typedef struct dds_condition dds_statuscond;

typedef struct dds_guardcond
{
  dds_condition m_cond;
  os_mutex m_lock;
}
dds_guardcond;

/* Entity flag values */

#define DDS_ENTITY_IN_USE 0x0001
#define DDS_ENTITY_DELETED 0x0002
#define DDS_ENTITY_WAITING 0x0004
#define DDS_ENTITY_FAILED 0x0008
#define DDS_ENTITY_DDSI_DELETED 0x0010

typedef struct dds_domain
{
  ut_avlNode_t m_node;
  dds_domainid_t m_id;
  ut_avlTree_t m_topics;
  uint32_t m_refc;
}
dds_domain;

typedef struct dds_entity
{
  dds_entity_kind_t m_kind;
  uint32_t m_refc;
  struct dds_entity * m_next;
  struct dds_entity * m_parent;
  struct dds_entity * m_children;
  struct dds_participant * m_pp;
  struct dds_domain * m_domain;
  dds_statuscond * m_scond;
  dds_qos_t * m_qos;
  void * m_cpp;
  dds_domainid_t m_domainid;
  nn_guid_t m_guid;
  uint32_t m_status_enable;
  uint32_t m_flags;
  os_mutex m_mutex;
  os_cond m_cond;
}
dds_entity;

extern const ut_avlTreedef_t dds_topictree_def;

typedef struct dds_participant
{
  struct dds_entity m_entity;
  dds_participantlistener_t m_listener;
  struct dds_entity * m_dur_reader;
  struct dds_entity * m_dur_writer;
}
dds_participant;

typedef struct dds_reader
{
  struct dds_entity m_entity;
  const struct dds_topic * m_topic;
  struct reader * m_rd;
  dds_readerlistener_t m_listener;
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
  dds_writerlistener_t m_listener;
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
  dds_subscriberlistener_t m_listener;
}
dds_subscriber;

typedef struct dds_publisher
{
  struct dds_entity m_entity;
  dds_publisherlistener_t m_listener;
}
dds_publisher;

typedef struct dds_topic
{
  struct dds_entity m_entity;
  struct sertopic * m_stopic;
  const dds_topic_descriptor_t * m_descriptor;
  dds_topiclistener_t m_listener;

  /* Status metrics */

  dds_inconsistent_topic_status_t m_inconsistent_topic_status;
}
dds_topic;

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
  os_atomic_uint32_t m_entity_count[DDS_ENTITY_NUM];
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
