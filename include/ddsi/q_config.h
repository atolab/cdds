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
#ifndef NN_CONFIG_H
#define NN_CONFIG_H

#include "os/os.h"

#include "ddsi/q_log.h"
#include "ddsi/q_thread.h"
#ifdef DDSI_INCLUDE_ENCRYPTION
#include "ddsi/q_security.h"
#endif /* DDSI_INCLUDE_ENCRYPTION */
#include "ddsi/q_xqos.h"
#include "ddsi/ddsi_tran.h"
#include "ddsi/q_feature_check.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DDSI2
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* FIXME: should eventually move to abstraction layer */
typedef enum q__schedPrioClass {
  Q__SCHED_PRIO_RELATIVE,
  Q__SCHED_PRIO_ABSOLUTE
} q__schedPrioClass;

enum nn_standards_conformance {
  NN_SC_PEDANTIC,
  NN_SC_STRICT,
  NN_SC_LAX
};

#define NN_PEDANTIC_P (config.standards_conformance <= NN_SC_PEDANTIC)
#define NN_STRICT_P (config.standards_conformance <= NN_SC_STRICT)

enum besmode {
  BESMODE_FULL,
  BESMODE_WRITERS,
  BESMODE_MINIMAL
};

enum retransmit_merging {
  REXMIT_MERGE_NEVER,
  REXMIT_MERGE_ADAPTIVE,
  REXMIT_MERGE_ALWAYS
};

enum boolean_default {
  BOOLDEF_DEFAULT,
  BOOLDEF_FALSE,
  BOOLDEF_TRUE
};

enum durability_cdr
{
  DUR_CDR_LE,
  DUR_CDR_BE,
  DUR_CDR_SERVER,
  DUR_CDR_CLIENT
};

#define PARTICIPANT_INDEX_AUTO -1
#define PARTICIPANT_INDEX_NONE -2

/* config_listelem must be an overlay for all used listelem types */
struct config_listelem {
  struct config_listelem *next;
};

#ifdef DDSI_INCLUDE_ENCRYPTION
struct q_security_plugins
{
  c_bool (*encode) (q_securityEncoderSet, uint32_t, void *, uint32_t, uint32_t *);
  c_bool (*decode) (q_securityDecoderSet, void *, size_t, size_t *);
  q_securityEncoderSet (*new_encoder) (void);
  q_securityDecoderSet (*new_decoder) (void);
  c_bool (*free_encoder) (q_securityEncoderSet);
  c_bool (*free_decoder) (q_securityDecoderSet);
  ssize_t (*send_encoded) (ddsi_tran_conn_t, struct msghdr *, q_securityEncoderSet *, uint32_t, uint32_t);
  char * (*cipher_type) (q_cipherType);
  c_bool (*cipher_type_from_string) (const char *, q_cipherType *);
  uint32_t (*header_size) (q_securityEncoderSet, uint32_t);
  q_cipherType (*encoder_type) (q_securityEncoderSet, uint32_t);
  c_bool (*valid_uri) (q_cipherType, const char *);
};

struct q_security_plugins q_security_plugin;

struct config_securityprofile_listelem
{
  struct config_securityprofile_listelem *next;
  char *name;
  q_cipherType cipher;
  char *key;
};
#endif /* DDSI_INCLUDE_ENCRYPTION */

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
struct config_networkpartition_listelem {
  struct config_networkpartition_listelem *next;
  char *name;
  char *address_string;
  struct addrset *as;
  int connected;
#ifdef DDSI_INCLUDE_ENCRYPTION
  char *profileName;
  struct config_securityprofile_listelem *securityProfile;
#endif /* DDSI_INCLUDE_ENCRYPTION */
  uint32_t partitionHash;
  uint32_t partitionId;
};

struct config_ignoredpartition_listelem {
  struct config_ignoredpartition_listelem *next;
  os_char *DCPSPartitionTopic;
};

struct config_partitionmapping_listelem {
  struct config_partitionmapping_listelem *next;
  char *networkPartition;
  char *DCPSPartitionTopic;
  struct config_networkpartition_listelem *partition;
};
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
struct config_channel_listelem {
  struct config_channel_listelem *next;
  char   *name;
  int    priority;
  unsigned  queue_size;
  int64_t resolution;
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  uint32_t data_bandwidth_limit;
  uint32_t auxiliary_bandwidth_limit;
#endif
  int    diffserv_field;
  struct thread_state1 *channel_reader_ts;  /* keeping an handle to the running thread for this channel */
  struct nn_dqueue *dqueue; /* The handle of teh delivery queue servicing incoming data for this channel*/
  struct xeventq *evq; /* The handle of the event queue servicing this channel*/
  uint32_t queueId; /* the index of the networkqueue serviced by this channel*/
  struct ddsi_tran_conn * transmit_conn; /* the connection used for sending data out via this channel */
};
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */

struct config_maybe_int32 {
  int isdefault;
  int32_t value;
};

struct config_maybe_uint32 {
  int isdefault;
  uint32_t value;
};

struct config_maybe_int64 {
  int isdefault;
  int64_t value;
};

struct config_thread_properties_listelem {
  struct config_thread_properties_listelem *next;
  char *name;
  os_schedClass sched_class;
  struct config_maybe_int32 sched_priority;
  struct config_maybe_uint32 stack_size;
};

struct config_peer_listelem
{
  struct config_peer_listelem *next;
  char *peer;
};

/* allow multicast bits: */
#define AMC_FALSE 0u
#define AMC_SPDP 1u
#define AMC_ASM 2u
#ifdef DDSI_INCLUDE_SSM
#define AMC_SSM 4u
#define AMC_TRUE (AMC_SPDP | AMC_ASM | AMC_SSM)
#else
#define AMC_TRUE (AMC_SPDP | AMC_ASM)
#endif

struct config
{
  int valid;
  logcat_t enabled_logcats;
  char *servicename;
  char *pcap_file;

  char *networkAddressString;
  char **networkRecvAddressStrings;
  char *externalAddressString;
  char *externalMaskString;
  FILE *tracingOutputFile;
  char *tracingOutputFileName;
  int tracingTimestamps;
  int tracingRelativeTimestamps;
  int tracingAppendToFile;
  unsigned allowMulticast;
  int useIpv6;
  int dontRoute;
  int enableMulticastLoopback;
  int domainId;
  int participantIndex;
  int maxAutoParticipantIndex;
  int port_base;
  struct config_maybe_int32 discoveryDomainId;
  char *spdpMulticastAddressString;
  char *defaultMulticastAddressString;
  char *assumeMulticastCapable;
  int64_t spdp_interval;
  int64_t spdp_response_delay_max;
  int64_t startup_mode_duration;
  int64_t lease_duration;
  enum retransmit_merging retransmit_merging;
  int64_t retransmit_merging_period;
  int squash_participants;
  int startup_mode_full;
  int forward_all_messages;
  int noprogress_log_stacktraces;
  int prioritize_retransmit;


  unsigned primary_reorder_maxsamples;
  unsigned secondary_reorder_maxsamples;

  unsigned delivery_queue_maxsamples;

  float servicelease_expiry_time;
  float servicelease_update_factor;

  uint32_t guid_hash_softlimit;

  int enableLoopback;
  enum durability_cdr durability_cdr;

  unsigned nw_queue_size;

  int buggy_datafrag_flags_mode;

  uint32_t max_msg_size;
  uint32_t fragment_size;

  int publish_uc_locators; /* Publish discovery unicast locators */

  /* TCP transport configuration */

  int tcp_enable;
  int tcp_nodelay;
  int tcp_port;
  int64_t tcp_read_timeout;
  int64_t tcp_write_timeout;

#ifdef DDSI_INCLUDE_SSL

  /* SSL support for TCP */

  int ssl_enable;
  int ssl_verify;
  int ssl_verify_client;
  int ssl_self_signed;
  char * ssl_keystore;
  char * ssl_rand_file;
  char * ssl_key_pass;
  char * ssl_ciphers;

#endif

  /* Thread pool configuration */

  int tp_enable;
  uint32_t tp_threads;
  uint32_t tp_max_threads;

  int generate_builtin_topics;
  int advertise_builtin_topic_writers;

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  struct config_channel_listelem *channels;
  struct config_channel_listelem *max_channel; /* channel with highest prio; always computed */
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */
#ifdef DDSI_INCLUDE_ENCRYPTION
  struct config_securityprofile_listelem  *securityProfiles;
#endif /* DDSI_INCLUDE_ENCRYPTION */
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  struct config_networkpartition_listelem *networkPartitions;
  unsigned nof_networkPartitions;
  struct config_ignoredpartition_listelem *ignoredPartitions;
  struct config_partitionmapping_listelem *partitionMappings;
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */
  struct config_peer_listelem *peers;
  struct config_peer_listelem *peers_group;
  struct config_thread_properties_listelem *thread_properties;

  /* debug/test/undoc features: */
  int xmit_lossiness;           /**<< fraction of packets to drop on xmit, in units of 1e-3 */
  uint32_t rmsg_chunk_size;          /**<< size of a chunk in the receive buffer */
  uint32_t rbuf_size;                /* << size of a single receiver buffer */
  enum besmode besmode;
  int aggressive_keep_last_whc;
  int conservative_builtin_reader_startup;
  int meas_hb_to_ack_latency;
  int suppress_spdp_multicast;
  int unicast_response_to_spdp_messages;
  int synchronous_delivery_priority_threshold;
  int64_t synchronous_delivery_latency_bound;

  /* Write cache */

  int whc_batch;
  uint32_t whc_lowwater_mark;
  uint32_t whc_highwater_mark;
  struct config_maybe_uint32 whc_init_highwater_mark;
  int whc_adaptive;

  unsigned defrag_unreliable_maxsamples;
  unsigned defrag_reliable_maxsamples;
  unsigned accelerate_rexmit_block_size;
  int64_t responsiveness_timeout;
  uint32_t max_participants;
  int64_t writer_linger_duration;
  int multicast_ttl;
  struct config_maybe_uint32 socket_min_rcvbuf_size;
  uint32_t socket_min_sndbuf_size;
  int64_t nack_delay;
  int64_t preemptive_ack_delay;
  int64_t schedule_time_rounding;
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  uint32_t auxiliary_bandwidth_limit; /* bytes/second */
#endif
  uint32_t max_queued_rexmit_bytes;
  unsigned max_queued_rexmit_msgs;
  unsigned ddsi2direct_max_threads;
  int late_ack_mode;
  int retry_on_reject_besteffort;
  int generate_keyhash;
  uint32_t max_sample_size;

  /* compability options */
  enum nn_standards_conformance standards_conformance;
  int explicitly_publish_qos_set_to_default;
  int many_sockets_mode;
  int arrival_of_data_asserts_pp_and_ep_liveliness;
  int acknack_numbits_emptyset;
  int respond_to_rti_init_zero_ack_with_invalid_heartbeat;
  int assume_rti_has_pmd_endpoints;

  int port_dg;
  int port_pg;
  int port_d0;
  int port_d1;
  int port_d2;
  int port_d3;

  int monitor_port;

  int enable_control_topic;
  int initial_deaf_mute;

  /* not used by ddsi2, only validated; user layer directly accesses
     the configuration tree */
  os_schedClass watchdog_sched_class;
  int32_t watchdog_sched_priority;
  q__schedPrioClass watchdog_sched_priority_class;
};

struct rhc;
struct nn_xqos;
struct tkmap_instance;
struct nn_rsample_info;
struct serdata;
struct sertopic;
struct proxy_writer;
struct proxy_writer_info;

struct ddsi_plugin
{
  int (*init_fn) (void);
  void (*fini_fn) (void);


  /* Read cache */

  void (*rhc_free_fn) (struct rhc *rhc);
  void (*rhc_fini_fn) (struct rhc *rhc);
  bool (*rhc_store_fn)
    (struct rhc * restrict rhc, const struct nn_rsample_info * restrict sampleinfo,
     struct serdata * restrict sample, struct tkmap_instance * restrict tk);
  void (*rhc_unregister_wr_fn)
    (struct rhc * restrict rhc, const struct proxy_writer_info * restrict pwr_info);
  void (*rhc_relinquish_ownership_fn)
    (struct rhc * restrict rhc, const uint64_t wr_iid);
  void (*rhc_set_qos_fn) (struct rhc * rhc, const struct nn_xqos * qos);
  struct tkmap_instance * (*rhc_lookup_fn) (struct serdata *serdata);
  void (*rhc_unref_fn) (struct tkmap_instance *tk);

  /* IID generator */

  uint64_t (*iidgen_fn) (void);

};

extern struct config OS_API config;
extern struct ddsi_plugin ddsi_plugin;

struct cfgst;

struct cfgst *config_init (const char *configfile);
void config_print_and_free_cfgst (struct cfgst *cfgst);
void config_fini (void);

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
struct config_partitionmapping_listelem *find_partitionmapping (const char *partition, const char *topic);
struct config_networkpartition_listelem *find_networkpartition_by_id (uint32_t id);
int is_ignored_partition (const char *partition, const char *topic);
#endif
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
struct config_channel_listelem *find_channel (nn_transport_priority_qospolicy_t transport_priority);
#endif
#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* NN_CONFIG_H */