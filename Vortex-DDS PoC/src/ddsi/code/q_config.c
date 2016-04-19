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
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

#include "os_stdlib.h"
#include "os_socket.h"
#include "os_heap.h"

#if !LITE
#include "ut_crc.h"
#include "u_participant.h"
#include "u_cfElement.h"
#include "u_cfData.h"
#include "u_cfNode.h"
#endif

#include "q_config.h"
#include "q_log.h"
#include "ut_avl.h"
#include "q_unused.h"
#include "q_misc.h"
#include "q_addrset.h"
#include "q_nwif.h"
#include "q_error.h"
#include "sysdeps.h"

#if LITE
#include "ut_xmlparser.h"
#endif

#define WARN_DEPRECATED_ALIAS 1
#define WARN_DEPRECATED_UNIT 1
#define MAX_PATH_DEPTH 10 /* max nesting level of configuration elements */

struct cfgelem;
struct cfgst;

typedef int (*init_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef int (*update_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value);
typedef void (*free_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef void (*print_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default);

#ifdef DDSI_INCLUDE_SECURITY
struct q_security_plugins q_security_plugin = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
#endif

struct unit {
  const char *name;
  int64_t multiplier;
};

struct cfgelem {
  const char *name;
  const struct cfgelem *children;
  const struct cfgelem *attributes;
  int multiplicity;
  const char *defvalue; /* NULL -> no default */
  int relative_offset;
  int elem_offset;
  init_fun_t init;
  update_fun_t update;
  free_fun_t free;
  print_fun_t print;
};

struct cfgst_nodekey {
  const struct cfgelem *e;
};

struct cfgst_node {
  ut_avlNode_t avlnode;
  struct cfgst_nodekey key;
  int count;
  int failed;
  int is_default;
};

struct cfgst {
  ut_avlTree_t found;
  struct config *cfg;

#if !LITE
  /* Servicename is used by uf_service_name to use as a default value
     when the supplied string is empty, which happens when the service
     is started without a DDSI2EService configuration item, i.e. when
     everything is left at the default. */
  const char *servicename;
#endif

  /* path_depth, isattr and path together control the formatting of
     error messages by cfg_error() */
  int path_depth;
  int isattr[MAX_PATH_DEPTH];
  const struct cfgelem *path[MAX_PATH_DEPTH];
  void *parent[MAX_PATH_DEPTH];
};

/* "trace" is special: it enables (nearly) everything */
static const char *logcat_names[] = {
  "fatal", "error", "warning", "config", "info", "discovery", "data", "radmin", "timing", "traffic", "topic", "tcp", "plist", "whc", "trace", NULL
};
static const logcat_t logcat_codes[] = {
  LC_FATAL, LC_ERROR, LC_WARNING, LC_CONFIG, LC_INFO, LC_DISCOVERY, LC_DATA, LC_RADMIN, LC_TIMING, LC_TRAFFIC, LC_TOPIC, LC_TCP, LC_PLIST, LC_WHC, LC_ALLCATS
};

/* We want the tracing/verbosity settings to be fixed while parsing
   the configuration, so we update this variable instead. */
static unsigned enabled_logcats;

static int cfgst_node_cmp (const void *va, const void *vb);
static const ut_avlTreedef_t cfgst_found_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct cfgst_node, avlnode), offsetof (struct cfgst_node, key), cfgst_node_cmp, 0);

#define DU(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
DU (uf_networkAddress);
DU (uf_networkAddresses);
DU (uf_ipv4);
DU (uf_allow_multicast);
DU (uf_boolean);
DU (uf_negated_boolean);
DU (uf_string);
DU (uf_tracingOutputFileName);
DU (uf_verbosity);
DU (uf_logcat);
DU (uf_float);
DU (uf_int);
DU (uf_uint);
DU (uf_int32);
DU (uf_uint32);
DU (uf_natint);
DU (uf_natint_255);
DU (uf_participantIndex);
DU (uf_port);
DU (uf_dyn_port);
DU (uf_memsize);
DU (uf_duration_inf);
DU (uf_duration_ms_1hr);
DU (uf_duration_ms_1s);
DU (uf_duration_us_1s);
DU (uf_standards_conformance);
DU (uf_besmode);
DU (uf_retransmit_merging);
DU (uf_sched_prio_class);
DU (uf_sched_class);
DU (uf_maybe_memsize);
DU (uf_maybe_int32);
#ifdef DDSI_INCLUDE_ENCRYPTION
DU (uf_cipher);
#endif
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
DU (uf_bandwidth);
#endif
DU (uf_domainId);
#if !LITE
DU (uf_maybe_duration_inf);
DU (uf_service_name);
DU (uf_boolean_default);
#endif
#if LITE
DU (uf_durability_cdr);
#endif
#undef DU

#define DF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
DF (ff_free);
DF (ff_networkAddresses);
#undef DF

#define DI(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
DI (if_channel);
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */
#ifdef DDSI_INCLUDE_ENCRYPTION
DI (if_security_profile);
#endif
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
DI (if_network_partition);
DI (if_ignored_partition);
DI (if_partition_mapping);
#endif
DI (if_peer);
DI (if_thread_properties);
#undef DI

#define PF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
PF (pf_nop);
PF (pf_string);
PF (pf_networkAddress);
PF (pf_participantIndex);
PF (pf_networkAddresses);
PF (pf_memsize);
PF (pf_duration);
PF (pf_int);
PF (pf_uint);
PF (pf_int32);
PF (pf_uint32);
PF (pf_float);
PF (pf_allow_multicast);
PF (pf_boolean);
PF (pf_negated_boolean);
PF (pf_logcat);
PF (pf_standards_conformance);
PF (pf_besmode);
PF (pf_retransmit_merging);
PF (pf_sched_prio_class);
PF (pf_sched_class);
PF (pf_maybe_memsize);
PF (pf_maybe_int32);
#if ! LITE
PF (pf_maybe_duration);
PF (pf_boolean_default);
#endif
#if LITE
PF (pf_durability_cdr);
#endif
#ifdef DDSI_INCLUDE_ENCRYPTION
PF (pf_cipher);
PF (pf_key);
#endif
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
PF (pf_bandwidth);
#endif
#undef PF

#define CO(name) ((int) offsetof (struct config, name))
#define ABSOFF(name) 0, CO (name)
#define RELOFF(parent,name) 1, ((int) offsetof (struct parent, name))
#define NODATA 1, NULL, 0, 0, 0, 0, 0, 0
#define END_MARKER { NULL, NULL, NULL, NODATA }
#define WILDCARD { "*", NULL, NULL, NODATA }
#define LEAF(name) name, NULL, NULL
#define LEAF_W_ATTRS(name, attrs) name, NULL, attrs
#define GROUP(name, children) name, children, NULL, 1, NULL, 0, 0, 0, 0, 0, 0
#define MGROUP(name, children, attrs) name, children, attrs
#define ATTR(name) name, NULL, NULL
/* MOVED: whereto must be a path relative to DDSI2EService, may not be used in/for lists and only for elements, may not be chained */
#define MOVED(name, whereto) ">" name, NULL, NULL, 0, whereto, 0, 0, 0, 0, 0, 0
static const struct cfgelem timestamp_cfgattrs[] = {
  { ATTR ("absolute"), 1, "false", ABSOFF (tracingRelativeTimestamps), 0, uf_negated_boolean, 0, pf_negated_boolean },
  END_MARKER
};

static const struct cfgelem general_cfgelems[] = {
  { LEAF ("NetworkInterfaceAddress"), 1, "auto", ABSOFF (networkAddressString), 0, uf_networkAddress, ff_free, pf_networkAddress },
  { LEAF ("MulticastRecvNetworkInterfaceAddresses"), 1, "preferred", ABSOFF (networkRecvAddressStrings), 0, uf_networkAddresses, ff_networkAddresses, pf_networkAddresses },
  { LEAF ("ExternalNetworkAddress"), 1, "auto", ABSOFF (externalAddressString), 0, uf_networkAddress, ff_free, pf_networkAddress },
  { LEAF ("ExternalNetworkMask"), 1, "0.0.0.0", ABSOFF (externalMaskString), 0, uf_string, ff_free, pf_string },
  { LEAF ("AllowMulticast"), 1, "true", ABSOFF (allowMulticast), 0, uf_allow_multicast, 0, pf_allow_multicast },
  { LEAF ("MulticastTimeToLive"), 1, "32", ABSOFF (multicast_ttl), 0, uf_natint_255, 0, pf_int },
  { LEAF ("DontRoute"), 1, "false", ABSOFF (dontRoute), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("UseIPv6"), 1, "false", ABSOFF (useIpv6), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("EnableMulticastLoopback"), 1, "true", ABSOFF (enableMulticastLoopback), 0, uf_boolean, 0, pf_boolean },
#if LITE
  { LEAF ("EnableLoopback"), 1, "false", ABSOFF (enableLoopback), 0, uf_boolean, 0, pf_boolean },
#else
  { LEAF ("CoexistWithNativeNetworking"), 1, "false", ABSOFF (coexistWithNativeNetworking), 0, uf_boolean, 0, pf_boolean },
#endif
  { LEAF ("StartupModeDuration"), 1, "2 s", ABSOFF (startup_mode_duration), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("StartupModeCoversTransient"), 1, "true", ABSOFF (startup_mode_full), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("MaxMessageSize"), 1, "4096 B", ABSOFF (max_msg_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("FragmentSize"), 1, "1280 B", ABSOFF (fragment_size), 0, uf_memsize, 0, pf_memsize },
  END_MARKER
};

#ifdef DDSI_INCLUDE_ENCRYPTION
static const struct cfgelem securityprofile_cfgattrs[] = {
  { ATTR ("Name"), 1, NULL, RELOFF (config_securityprofile_listelem, name), 0, uf_string, ff_free, pf_string },
  { ATTR ("Cipher"), 1, "null", RELOFF (config_securityprofile_listelem, cipher), 0, uf_cipher, 0, pf_cipher },
  { ATTR ("CipherKey"), 1, "", RELOFF (config_securityprofile_listelem, key), 0, uf_string, ff_free, pf_key },
  END_MARKER
};

static const struct cfgelem security_cfgelems[] = {
  { LEAF_W_ATTRS ("SecurityProfile", securityprofile_cfgattrs), 0, 0, ABSOFF (securityProfiles), if_security_profile, 0, 0, 0 },
  END_MARKER
};
#endif /* DDSI_INCLUDE_ENCRYPTION */

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
static const struct cfgelem networkpartition_cfgattrs[] = {
  { ATTR ("Name"), 1, NULL, RELOFF (config_networkpartition_listelem, name), 0, uf_string, ff_free, pf_string },
  { ATTR ("Address"), 1, NULL, RELOFF (config_networkpartition_listelem, address_string), 0, uf_string, ff_free, pf_string },
  { ATTR ("Connected"), 1, "true", RELOFF (config_networkpartition_listelem, connected), 0, uf_boolean, 0, pf_boolean },
#ifdef DDSI_INCLUDE_ENCRYPTION
  { ATTR ("SecurityProfile"), 1, "null", RELOFF (config_networkpartition_listelem, profileName), 0, uf_string, ff_free, pf_string },
#endif /* DDSI_INCLUDE_ENCRYPTION */
  END_MARKER
};

static const struct cfgelem networkpartitions_cfgelems[] = {
  { LEAF_W_ATTRS ("NetworkPartition", networkpartition_cfgattrs), 0, 0, ABSOFF (networkPartitions), if_network_partition, 0, 0, 0 },
  END_MARKER
};

static const struct cfgelem ignoredpartitions_cfgattrs[] = {
  { ATTR ("DCPSPartitionTopic"), 1, NULL, RELOFF (config_ignoredpartition_listelem, DCPSPartitionTopic), 0, uf_string, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem ignoredpartitions_cfgelems[] = {
  { LEAF_W_ATTRS ("IgnoredPartition", ignoredpartitions_cfgattrs), 0, 0, ABSOFF (ignoredPartitions), if_ignored_partition, 0, 0, 0 },
  END_MARKER
};

static const struct cfgelem partitionmappings_cfgattrs[] = {
  { ATTR ("NetworkPartition"), 1, NULL, RELOFF (config_partitionmapping_listelem, networkPartition), 0, uf_string, ff_free, pf_string },
  { ATTR ("DCPSPartitionTopic"), 1, NULL, RELOFF (config_partitionmapping_listelem, DCPSPartitionTopic), 0, uf_string, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem partitionmappings_cfgelems[] = {
  { LEAF_W_ATTRS ("PartitionMapping", partitionmappings_cfgattrs), 0, 0, ABSOFF (partitionMappings), if_partition_mapping, 0, 0, 0 },
  END_MARKER
};

static const struct cfgelem partitioning_cfgelems[] = {
  { GROUP ("NetworkPartitions", networkpartitions_cfgelems) },
  { GROUP ("IgnoredPartitions", ignoredpartitions_cfgelems) },
  { GROUP ("PartitionMappings", partitionmappings_cfgelems) },
  END_MARKER
};
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
static const struct cfgelem channel_cfgelems[] = {
  { LEAF ("QueueSize"), 1, "0", RELOFF (config_channel_listelem, queue_size), 0, uf_natint, 0, pf_int },
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  { LEAF ("DataBandwidthLimit"), 1, "inf", RELOFF (config_channel_listelem, data_bandwidth_limit), 0, uf_bandwidth, 0, pf_bandwidth },
  { LEAF ("AuxiliaryBandwidthLimit"), 1, "inf", RELOFF (config_channel_listelem, auxiliary_bandwidth_limit), 0, uf_bandwidth, 0, pf_bandwidth },
#endif
  { LEAF ("DiffServField"), 1, "0", RELOFF (config_channel_listelem, diffserv_field), 0, uf_natint, 0, pf_int },
#if ! LITE
  { ATTR ("Resolution"), 1, "1ms", RELOFF (config_channel_listelem, resolution), 0, uf_duration_ms_1s, 0, pf_duration },
#endif
  END_MARKER
};

static const struct cfgelem channel_cfgattrs[] = {
  { ATTR ("Name"), 1, NULL, RELOFF (config_channel_listelem, name), 0, uf_string, ff_free, pf_string },
  { ATTR ("TransportPriority"), 1, "0", RELOFF (config_channel_listelem, priority), 0, uf_natint, 0, pf_int },
  END_MARKER
};

static const struct cfgelem channels_cfgelems[] = {
  { MGROUP ("Channel", channel_cfgelems, channel_cfgattrs), 42, 0, ABSOFF (channels), if_channel, 0, 0, 0 },
  END_MARKER
};
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */

static const struct cfgelem thread_properties_sched_cfgelems[] = {
  { LEAF ("Class"), 1, "default", RELOFF (config_thread_properties_listelem, sched_class), 0, uf_sched_class, 0, pf_sched_class },
  { LEAF ("Priority"), 1, "default", RELOFF (config_thread_properties_listelem, sched_priority), 0, uf_maybe_int32, 0, pf_maybe_int32 },
  END_MARKER
};

static const struct cfgelem thread_properties_cfgattrs[] = {
  { ATTR ("Name"), 1, NULL, RELOFF (config_thread_properties_listelem, name), 0, uf_string, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem thread_properties_cfgelems[] = {
  { GROUP ("Scheduling", thread_properties_sched_cfgelems) },
  { LEAF ("StackSize"), 1, "default", RELOFF (config_thread_properties_listelem, stack_size), 0, uf_maybe_memsize, 0, pf_maybe_memsize },
  END_MARKER
};

static const struct cfgelem threads_cfgelems[] = {
  { MGROUP ("Thread", thread_properties_cfgelems, thread_properties_cfgattrs), 1000, 0, ABSOFF (thread_properties), if_thread_properties, 0, 0, 0 },
  END_MARKER
};

static const struct cfgelem compatibility_cfgelems[] = {
  { LEAF ("StandardsConformance"), 1, "lax", ABSOFF (standards_conformance), 0, uf_standards_conformance, 0, pf_standards_conformance },
  { LEAF ("ExplicitlyPublishQosSetToDefault"), 1, "false", ABSOFF (explicitly_publish_qos_set_to_default), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("ManySocketsMode"), 1, "false", ABSOFF (many_sockets_mode), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("ArrivalOfDataAssertsPpAndEpLiveliness"), 1, "true", ABSOFF (arrival_of_data_asserts_pp_and_ep_liveliness), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("AckNackNumbitsEmptySet"), 1, "0", ABSOFF (acknack_numbits_emptyset), 0, uf_natint, 0, pf_int },
  { LEAF ("RespondToRtiInitZeroAckWithInvalidHeartbeat"), 1, "false", ABSOFF (respond_to_rti_init_zero_ack_with_invalid_heartbeat), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("AssumeRtiHasPmdEndpoints"), 1, "false", ABSOFF (assume_rti_has_pmd_endpoints), 0, uf_boolean, 0, pf_boolean },
  END_MARKER
};

static const struct cfgelem unsupp_test_cfgelems[] = {
  { LEAF ("XmitLossiness"), 1, "0", ABSOFF (xmit_lossiness), 0, uf_int, 0, pf_int },
  END_MARKER
};

static const struct cfgelem unsupp_watermarks_cfgelems[] = {
  { LEAF ("WhcLow"), 1, "1 kB", ABSOFF (whc_lowwater_mark), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("WhcHigh"), 1, "100 kB", ABSOFF (whc_highwater_mark), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("WhcHighInit"), 1, "30 kB", ABSOFF (whc_init_highwater_mark), 0, uf_maybe_memsize, 0, pf_maybe_memsize },
  { LEAF ("WhcAdaptative"), 1, "true", ABSOFF (whc_adaptive), 0, uf_boolean, 0, pf_boolean },
  END_MARKER
};

static const struct cfgelem control_topic_cfgattrs[] = {
  { ATTR ("Enable"), 1, "false", ABSOFF (enable_control_topic), 0, uf_boolean, 0, pf_boolean },
  END_MARKER
};

static const struct cfgelem control_topic_cfgelems[] = {
  { LEAF ("DeafMute"), 1, "false", ABSOFF (initial_deaf_mute), 0, uf_boolean, 0, pf_boolean },
  END_MARKER
};

static const struct cfgelem unsupp_cfgelems[] = {
  { MOVED ("MaxMessageSize", "General/MaxMessageSize") },
  { MOVED ("FragmentSize", "General/FragmentSize") },
  { LEAF ("DeliveryQueueMaxSamples"), 1, "256", ABSOFF (delivery_queue_maxsamples), 0, uf_uint, 0, pf_uint },
  { LEAF ("PrimaryReorderMaxSamples"), 1, "64", ABSOFF (primary_reorder_maxsamples), 0, uf_uint, 0, pf_uint },
  { LEAF ("SecondaryReorderMaxSamples"), 1, "16", ABSOFF (secondary_reorder_maxsamples), 0, uf_uint, 0, pf_uint },
  { LEAF ("DefragUnreliableMaxSamples"), 1, "4", ABSOFF (defrag_unreliable_maxsamples), 0, uf_uint, 0, pf_uint },
  { LEAF ("DefragReliableMaxSamples"), 1, "16", ABSOFF (defrag_reliable_maxsamples), 0, uf_uint, 0, pf_uint },
  { LEAF ("BuiltinEndpointSet"), 1, "writers", ABSOFF (besmode), 0, uf_besmode, 0, pf_besmode },
#if !LITE
  { LEAF ("AggressiveKeepLastWhc|AggressiveKeepLast1Whc"), 1, "false", ABSOFF (aggressive_keep_last_whc), 0, uf_boolean, 0, pf_boolean },
#else
  { LEAF ("AggressiveKeepLastWhc|AggressiveKeepLast1Whc"), 1, "true", ABSOFF (aggressive_keep_last_whc), 0, uf_boolean, 0, pf_boolean },
#endif
  { LEAF ("ConservativeBuiltinReaderStartup"), 1, "false", ABSOFF (conservative_builtin_reader_startup), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("MeasureHbToAckLatency"), 1, "false", ABSOFF (meas_hb_to_ack_latency), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("SuppressSPDPMulticast"), 1, "false", ABSOFF (suppress_spdp_multicast), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("UnicastResponseToSPDPMessages"), 1, "true", ABSOFF (unicast_response_to_spdp_messages), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("SynchronousDeliveryPriorityThreshold"), 1, "0", ABSOFF (synchronous_delivery_priority_threshold), 0, uf_int, 0, pf_int },
  { LEAF ("SynchronousDeliveryLatencyBound"), 1, "inf", ABSOFF (synchronous_delivery_latency_bound), 0, uf_duration_inf, 0, pf_duration },
  { LEAF ("MaxParticipants"), 1, "0", ABSOFF (max_participants), 0, uf_natint, 0, pf_int },
  { LEAF ("AccelerateRexmitBlockSize"), 1, "0", ABSOFF (accelerate_rexmit_block_size), 0, uf_uint, 0, pf_uint },
#if !LITE
  { LEAF ("ResponsivenessTimeout"), 1, "1 s", ABSOFF (responsiveness_timeout), 0, uf_duration_ms_1hr, 0, pf_duration },
#endif
  { LEAF ("RetransmitMerging"), 1, "adaptive", ABSOFF (retransmit_merging), 0, uf_retransmit_merging, 0, pf_retransmit_merging },
  { LEAF ("RetransmitMergingPeriod"), 1, "5 ms", ABSOFF (retransmit_merging_period), 0, uf_duration_us_1s, 0, pf_duration },
  { LEAF ("MaxQueuedRexmitBytes"), 1, "50 kB", ABSOFF (max_queued_rexmit_bytes), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("MaxQueuedRexmitMessages"), 1, "200", ABSOFF (max_queued_rexmit_msgs), 0, uf_uint, 0, pf_uint },
#if !LITE
  { LEAF ("MirrorRemoteEntities"), 1, "default", ABSOFF (mirror_remote_entities), 0, uf_boolean_default, 0, pf_boolean_default },
  { LEAF ("ForwardRemoteData"), 1, "default", ABSOFF (forward_remote_data), 0, uf_boolean_default, 0, pf_boolean },
#endif
#if LITE
  { LEAF ("LeaseDuration"), 1, "10 s", ABSOFF (lease_duration), 0, uf_duration_ms_1hr, 0, pf_duration },
#else
  { LEAF ("LeaseDuration"), 1, "0 s", ABSOFF (lease_duration), 0, uf_duration_ms_1hr, 0, pf_duration },
#endif
  { LEAF ("WriterLingerDuration"), 1, "1 s", ABSOFF (writer_linger_duration), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("MinimumSocketReceiveBufferSize"), 1, "default", ABSOFF (socket_min_rcvbuf_size), 0, uf_maybe_memsize, 0, pf_maybe_memsize },
  { LEAF ("MinimumSocketSendBufferSize"), 1, "64 KiB", ABSOFF (socket_min_sndbuf_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("NackDelay"), 1, "10 ms", ABSOFF (nack_delay), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("PreEmptiveAckDelay"), 1, "10 ms", ABSOFF (preemptive_ack_delay), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("ScheduleTimeRounding"), 1, "0 ms", ABSOFF (schedule_time_rounding), 0, uf_duration_ms_1hr, 0, pf_duration },
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  { LEAF ("AuxiliaryBandwidthLimit"), 1, "inf", ABSOFF (auxiliary_bandwidth_limit), 0, uf_bandwidth, 0, pf_bandwidth },
#endif
  { LEAF ("DDSI2DirectMaxThreads"), 1, "1", ABSOFF (ddsi2direct_max_threads), 0, uf_uint, 0, pf_uint },
  { LEAF ("SquashParticipants"), 1, "false", ABSOFF (squash_participants), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("LegacyFragmentation"), 1, "false", ABSOFF (buggy_datafrag_flags_mode), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("SPDPResponseMaxDelay"), 1, "0 ms", ABSOFF (spdp_response_delay_max), 0, uf_duration_ms_1s, 0, pf_duration },
  { LEAF ("LateAckMode"), 1, "false", ABSOFF (late_ack_mode), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("ForwardAllMessages"), 1, "false", ABSOFF (forward_all_messages), 0, uf_boolean, 0, pf_boolean },
#if !LITE
  { LEAF ("RetryOnRejectDuration"), 1, "default", ABSOFF (retry_on_reject_duration), 0, uf_maybe_duration_inf, 0, pf_maybe_duration },
#endif
  { LEAF ("RetryOnRejectBestEffort"), 1, "false", ABSOFF (retry_on_reject_besteffort), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("GenerateKeyhash"), 1, "true", ABSOFF (generate_keyhash), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("MaxSampleSize"), 1, "2147483647 B", ABSOFF (max_sample_size), 0, uf_memsize, 0, pf_memsize },
#if LITE
  { LEAF ("WriteBatch"), 1, "false", ABSOFF (whc_batch), 0, uf_boolean, 0, pf_boolean },
#endif
  { LEAF ("LogStackTraces"), 1, "true", ABSOFF (noprogress_log_stacktraces), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("MonitorPort"), 1, "-1", ABSOFF (monitor_port), 0, uf_int, 0, pf_int },
  { LEAF ("AssumeMulticastCapable"), 1, "", ABSOFF (assumeMulticastCapable), 0, uf_string, ff_free, pf_string },
  { LEAF ("PrioritizeRetransmit"), 1, "true", ABSOFF (prioritize_retransmit), 0, uf_boolean, 0, pf_boolean },
  { MGROUP ("ControlTopic", control_topic_cfgelems, control_topic_cfgattrs), 1, 0, 0, 0, 0, 0, 0, 0 },
  { GROUP ("Test", unsupp_test_cfgelems) },
  { GROUP ("Watermarks", unsupp_watermarks_cfgelems) },
  END_MARKER
};

static const struct cfgelem sizing_cfgelems[] =
{
#if LITE
  { LEAF ("ReceiveBufferSize"), 1, "128 KiB", ABSOFF (rbuf_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("ReceiveBufferChunkSize"), 1, "64 KiB", ABSOFF (rmsg_chunk_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("EndpointsInSystem"), 1, "200", ABSOFF (guid_hash_softlimit), 0, uf_uint32, 0, pf_uint32 },
  { LEAF ("NetworkQueueSize"), 1, "10", ABSOFF (nw_queue_size), 0, uf_uint, 0, pf_uint },
#else
  { LEAF ("ReceiveBufferSize"), 1, "1 MiB", ABSOFF (rbuf_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("ReceiveBufferChunkSize"), 1, "128 KiB", ABSOFF (rmsg_chunk_size), 0, uf_memsize, 0, pf_memsize },
  { LEAF ("LocalEndpoints"), 1, "1000", ABSOFF (gid_hash_softlimit), 0, uf_uint32, 0, pf_uint32 },
  { LEAF ("EndpointsInSystem"), 1, "20000", ABSOFF (guid_hash_softlimit), 0, uf_uint32, 0, pf_uint32 },
  { LEAF ("NetworkQueueSize"), 1, "1000", ABSOFF (nw_queue_size), 0, uf_uint, 0, pf_uint },
#endif
  END_MARKER
};

static const struct cfgelem discovery_ports_cfgelems[] = {
  { LEAF ("Base"), 1, "7400", ABSOFF (port_base), 0, uf_port, 0, pf_int },
  { LEAF ("DomainGain"), 1, "250", ABSOFF (port_dg), 0, uf_int, 0, pf_int },
  { LEAF ("ParticipantGain"), 1, "2", ABSOFF (port_pg), 0, uf_int, 0, pf_int },
  { LEAF ("MulticastMetaOffset"), 1, "0", ABSOFF (port_d0), 0, uf_int, 0, pf_int },
  { LEAF ("UnicastMetaOffset"), 1, "10", ABSOFF (port_d1), 0, uf_int, 0, pf_int },
  { LEAF ("MulticastDataOffset"), 1, "1", ABSOFF (port_d2), 0, uf_int, 0, pf_int },
  { LEAF ("UnicastDataOffset"), 1, "11", ABSOFF (port_d3), 0, uf_int, 0, pf_int },
  END_MARKER
};

static const struct cfgelem tcp_cfgelems[] = {
  { LEAF ("Enable"), 1, "false", ABSOFF (tcp_enable), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("NoDelay"), 1, "true", ABSOFF (tcp_nodelay), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("Port"), 1, "-1", ABSOFF (tcp_port), 0, uf_dyn_port, 0, pf_int },
  { LEAF ("ReadTimeout"), 1, "2 s", ABSOFF (tcp_read_timeout), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("WriteTimeout"), 1, "2 s", ABSOFF (tcp_write_timeout), 0, uf_duration_ms_1hr, 0, pf_duration },
  END_MARKER
};

#ifdef DDSI_INCLUDE_SSL
static const struct cfgelem ssl_cfgelems[] = {
  { LEAF ("Enable"), 1, "false", ABSOFF (ssl_enable), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("CertificateVerification"), 1, "true", ABSOFF (ssl_verify), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("VerifyClient"), 1, "false", ABSOFF (ssl_verify_client), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("SelfSignedCertificates"), 1, "false", ABSOFF (ssl_self_signed), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("KeystoreFile"), 1, "keystore", ABSOFF (ssl_keystore), 0, uf_string, ff_free, pf_string },
  { LEAF ("KeyPassphrase"), 1, "secret", ABSOFF (ssl_key_pass), 0, uf_string, ff_free, pf_string },
  { LEAF ("Ciphers"), 1, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH", ABSOFF (ssl_ciphers), 0, uf_string, ff_free, pf_string },
  { LEAF ("EntropyFile"), 1, "", ABSOFF (ssl_rand_file), 0, uf_string, ff_free, pf_string },
  END_MARKER
};
#endif

static const struct cfgelem tp_cfgelems[] = {
  { LEAF ("Enable"), 1, "false", ABSOFF (tp_enable), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("Threads"), 1, "4", ABSOFF (tp_threads), 0, uf_natint, 0, pf_int },
  { LEAF ("ThreadMax"), 1, "8", ABSOFF (tp_max_threads), 0, uf_natint, 0, pf_int },
  END_MARKER
};

static const struct cfgelem discovery_peer_cfgattrs[] = {
  { ATTR ("Address"), 1, NULL, RELOFF (config_peer_listelem, peer), 0, uf_ipv4, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem discovery_peers_group_cfgelems[] = {
  { MGROUP ("Peer", NULL, discovery_peer_cfgattrs), 0, NULL, ABSOFF (peers_group), if_peer, 0, 0, 0 },
  END_MARKER
};

static const struct cfgelem discovery_peers_cfgelems[] = {
  { MGROUP ("Peer", NULL, discovery_peer_cfgattrs), 0, NULL, ABSOFF (peers), if_peer, 0, 0, 0 },
  { GROUP ("Group", discovery_peers_group_cfgelems) },
  END_MARKER
};

static const struct cfgelem discovery_cfgelems[] = {
  { LEAF ("DomainId"), 1, "default", ABSOFF (discoveryDomainId), 0, uf_maybe_int32, 0, pf_maybe_int32 },
#if !LITE
  { LEAF ("LocalDiscoveryPartition"), 1, "__BUILT-IN PARTITION__", ABSOFF (local_discovery_partition), 0, uf_string, ff_free, pf_string },
#endif
  { LEAF ("GenerateBuiltinTopics"), 1, "true", ABSOFF (generate_builtin_topics), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("AdvertiseBuiltinTopicWriters"), 1, "true", ABSOFF (advertise_builtin_topic_writers), 0, uf_boolean, 0, pf_boolean },
  { GROUP ("Peers", discovery_peers_cfgelems) },
  { LEAF ("ParticipantIndex"), 1, "none", ABSOFF (participantIndex), 0, uf_participantIndex, 0, pf_participantIndex },
  { LEAF ("MaxAutoParticipantIndex"), 1, "9", ABSOFF (maxAutoParticipantIndex), 0, uf_natint, 0, pf_int },
  { LEAF ("SPDPMulticastAddress"), 1, "239.255.0.1", ABSOFF (spdpMulticastAddressString), 0, uf_ipv4, ff_free, pf_string },
  { LEAF ("SPDPInterval"), 1, "30 s", ABSOFF (spdp_interval), 0, uf_duration_ms_1hr, 0, pf_duration },
  { LEAF ("DefaultMulticastAddress"), 1, "auto", ABSOFF (defaultMulticastAddressString), 0, uf_networkAddress, 0, pf_networkAddress },
  { GROUP ("Ports", discovery_ports_cfgelems) },
  END_MARKER
};

static const struct cfgelem tracing_cfgelems[] = {
  { LEAF ("EnableCategory"), 1, "", 0, 0, 0, uf_logcat, 0, pf_logcat },
  { LEAF ("Verbosity"), 1, "none", 0, 0, 0, uf_verbosity, 0, pf_nop },
#if LITE
  { LEAF ("OutputFile"), 1, "lite.log", ABSOFF (tracingOutputFileName), 0, uf_tracingOutputFileName, ff_free, pf_string },
#else
  { LEAF ("OutputFile"), 1, "ddsi2.log", ABSOFF (tracingOutputFileName), 0, uf_tracingOutputFileName, ff_free, pf_string },
#endif
  { LEAF_W_ATTRS ("Timestamps", timestamp_cfgattrs), 1, "true", ABSOFF (tracingTimestamps), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("AppendToFile"), 1, "false", ABSOFF (tracingAppendToFile), 0, uf_boolean, 0, pf_boolean },
  { LEAF ("PacketCaptureFile"), 1, "", ABSOFF (pcap_file), 0, uf_string, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem sched_prio_cfgattrs[] = {
  { ATTR ("priority_kind"), 1, "relative", ABSOFF (watchdog_sched_priority_class), 0, uf_sched_prio_class, 0, pf_sched_prio_class },
  END_MARKER
};

static const struct cfgelem sched_cfgelems[] = {
  { LEAF ("Class"), 1, "default", ABSOFF (watchdog_sched_class), 0, uf_sched_class, 0, pf_sched_class },
  { LEAF_W_ATTRS ("Priority", sched_prio_cfgattrs), 1, "0", ABSOFF (watchdog_sched_priority), 0, uf_int32, 0, pf_int32 },
  END_MARKER
};

static const struct cfgelem watchdog_cfgelems[] = {
  { GROUP ("Scheduling", sched_cfgelems) },
  END_MARKER
};

/* Note: adding "Unsupported" with NULL for a description hides it
   from the configurator */

static const struct cfgelem ddsi2_cfgelems[] = {
  { GROUP ("General", general_cfgelems) },
#ifdef DDSI_INCLUDE_ENCRYPTION
  { GROUP ("Security", security_cfgelems) },
#endif
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  { GROUP ("Partitioning", partitioning_cfgelems) },
#endif
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  { GROUP ("Channels", channels_cfgelems) },
#endif
  { GROUP ("Threads", threads_cfgelems) },
  { GROUP ("Sizing", sizing_cfgelems) },
  { GROUP ("Compatibility", compatibility_cfgelems) },
  { GROUP ("Discovery", discovery_cfgelems) },
  { GROUP ("Tracing", tracing_cfgelems) },
  { GROUP ("Internal|Unsupported", unsupp_cfgelems) },
  { GROUP ("Watchdog", watchdog_cfgelems) },
  { GROUP ("TCP", tcp_cfgelems) },
  { GROUP ("ThreadPool", tp_cfgelems) },
#ifdef DDSI_INCLUDE_SSL
  { GROUP ("SSL", ssl_cfgelems) },
#endif
  END_MARKER
};

/* Note: using 2e-1 instead of 0.2 to avoid use of the decimal
   separator, which is locale dependent. */
static const struct cfgelem lease_expiry_time_cfgattrs[] = {
  { ATTR ("update_factor"), 1, "2e-1", ABSOFF (servicelease_update_factor), 0, uf_float, 0, pf_float },
  END_MARKER
};

static const struct cfgelem lease_cfgelems[] = {
  { LEAF_W_ATTRS ("ExpiryTime", lease_expiry_time_cfgattrs), 1, "10", ABSOFF (servicelease_expiry_time), 0, uf_float, 0, pf_float },
  END_MARKER
};

#if LITE

static const struct cfgelem domain_cfgelems[] = {
  { GROUP ("Lease", lease_cfgelems) },
  { LEAF ("Id"), 1, "0", ABSOFF (domainId), 0, uf_domainId, 0, pf_int },
  WILDCARD,
  END_MARKER
};

static const struct cfgelem durability_cfgelems[] = {
  { LEAF ("Encoding"), 1, "CDR_CLIENT", ABSOFF (durability_cdr), 0, uf_durability_cdr, 0, pf_durability_cdr },
  END_MARKER
};

static const struct cfgelem root_cfgelems[] = {
  { "Domain", domain_cfgelems, NULL, NODATA },
  { "DDSI2E", ddsi2_cfgelems, NULL, NODATA },
  { "Durability", durability_cfgelems, NULL, NODATA },
  { "Lease", lease_cfgelems, NULL, NODATA },
  END_MARKER
};

static const struct cfgelem lite_root_cfgelems[] =
{
  { "Lite", root_cfgelems, NULL, NODATA },
  END_MARKER
};

static const struct cfgelem root_cfgelem =
  { "root", lite_root_cfgelems, NULL, NODATA };

#else

static const struct cfgelem domain_cfgelems[] = {
  { GROUP ("Lease", lease_cfgelems) },
  { LEAF ("Id"), 1, "0", ABSOFF (domainId), 0, uf_domainId, 0, pf_int },
  WILDCARD,
  END_MARKER
};

static const struct cfgelem ddsi2_cfgattrs[] = {
  { ATTR ("name"), 1, NULL, ABSOFF (servicename), 0, uf_service_name, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem root_cfgelems[] = {
  { "DDSI2EService|DDSI2Service", ddsi2_cfgelems, ddsi2_cfgattrs, NODATA },
  { "Lease", lease_cfgelems, NULL, NODATA },
  { "Domain", domain_cfgelems, NULL, NODATA },
  END_MARKER
};

static const struct cfgelem root_cfgelem =
  { "root", root_cfgelems, NULL, NODATA };

#endif

#undef ATTR
#undef GROUP
#undef LEAF_W_ATTRS
#undef LEAF
#undef WILDCARD
#undef END_MARKER
#undef NODATA
#undef RELOFF
#undef ABSOFF
#undef CO

struct config config;
struct ddsi_plugin ddsi_plugin;

static const struct unit unittab_duration[] = {
  { "ns", 1 },
  { "us", 1000 },
  { "ms", T_MILLISECOND },
  { "s", T_SECOND },
  { "min", 60 * T_SECOND },
  { "hr", 3600 * T_SECOND },
  { "day", 24 * 3600 * T_SECOND },
  { NULL, 0 }
};

/* note: order affects whether printed as KiB or kB, for consistency
   with bandwidths and pedanticness, favour KiB. */
static const struct unit unittab_memsize[] = {
  { "B", 1 },
  { "KiB", 1024 },
  { "kB", 1024 },
  { "MiB", 1048576 },
  { "MB", 1048576 },
  { "GiB", 1073741824 },
  { "GB", 1073741824 },
  { NULL, 0 }
};

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
static const struct unit unittab_bandwidth_bps[] = {
  { "b/s", 1 }, { "bps", 1 },
  { "Kib/s", 1024 }, { "Kibps", 1024 },
  { "kb/s", 1000 }, { "kbps", 1000 },
  { "Mib/s", 1048576 }, { "Mibps", 1000 },
  { "Mb/s", 1000000 }, { "Mbps", 1000000 },
  { "Gib/s", 1073741824 }, { "Gibps", 1073741824 },
  { "Gb/s", 1000000000 }, { "Gbps", 1000000000 },
  { "B/s", 8 }, { "Bps", 8 },
  { "KiB/s", 8 * 1024 }, { "KiBps", 8 * 1024 },
  { "kB/s", 8 * 1000 }, { "kBps", 8 * 1000 },
  { "MiB/s", 8 * 1048576 }, { "MiBps", 8 * 1048576 },
  { "MB/s", 8 * 1000000 }, { "MBps", 8 * 1000000 },
  { "GiB/s", 8 * (int64_t) 1073741824 }, { "GiBps", 8 * (int64_t) 1073741824 },
  { "GB/s", 8 * (int64_t) 1000000000 }, { "GBps", 8 * (int64_t) 1000000000 },
  { NULL, 0 }
};

static const struct unit unittab_bandwidth_Bps[] = {
  { "B/s", 1 }, { "Bps", 1 },
  { "KiB/s", 1024 }, { "KiBps", 1024 },
  { "kB/s", 1000 }, { "kBps", 1000 },
  { "MiB/s", 1048576 }, { "MiBps", 1048576 },
  { "MB/s", 1000000 }, { "MBps", 1000000 },
  { "GiB/s", 1073741824 }, { "GiBps", 1073741824 },
  { "GB/s", 1000000000 }, { "GBps", 1000000000 },
  { NULL, 0 }
};
#endif

static void cfgst_push (struct cfgst *cfgst, int isattr, const struct cfgelem *elem, void *parent)
{
  assert (cfgst->path_depth < MAX_PATH_DEPTH);
  assert (isattr == 0 || isattr == 1);
  cfgst->isattr[cfgst->path_depth] = isattr;
  cfgst->path[cfgst->path_depth] = elem;
  cfgst->parent[cfgst->path_depth] = parent;
  cfgst->path_depth++;
}

static void cfgst_pop (struct cfgst *cfgst)
{
  assert (cfgst->path_depth > 0);
  cfgst->path_depth--;
}

static const struct cfgelem *cfgst_tos (const struct cfgst *cfgst)
{
  assert (cfgst->path_depth > 0);
  return cfgst->path[cfgst->path_depth-1];
}

static void *cfgst_parent (const struct cfgst *cfgst)
{
  assert (cfgst->path_depth > 0);
  return cfgst->parent[cfgst->path_depth-1];
}

struct cfg_note_buf {
  size_t bufpos;
  size_t bufsize;
  char *buf;
};

static size_t cfg_note_vsnprintf (struct cfg_note_buf *bb, const char *fmt, va_list ap)
{
  int x;
  x = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
  if (x >= 0 && (size_t) x >= bb->bufsize - bb->bufpos)
  {
    size_t nbufsize = ((bb->bufsize + (size_t)x+1) + 1023) & (size_t)(-1024);
    char *nbuf = os_realloc (bb->buf, nbufsize);
    bb->buf = nbuf;
    bb->bufsize = nbufsize;
    return nbufsize;
  }
  if (x < 0)
    NN_FATAL0 ("cfg_note_vsnprintf: os_vsnprintf failed\n");
  else
    bb->bufpos += (size_t)x;
  return 0;
}

static void cfg_note_snprintf (struct cfg_note_buf *bb, const char *fmt, ...)
{
  /* The reason the 2nd call to os_vsnprintf is here and not inside
     cfg_note_vsnprintf is because I somehow doubt that all platforms
     implement va_copy() */
  va_list ap;
  size_t r;
  va_start (ap, fmt);
  r = cfg_note_vsnprintf (bb, fmt, ap);
  va_end (ap);
  if (r > 0)
  {
    int s;
    va_start (ap, fmt);
    s = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
    if (s < 0 || (size_t)s >= bb->bufsize - bb->bufpos)
      NN_FATAL0 ("cfg_note_snprintf: os_vsnprintf failed\n");
    va_end (ap);
    bb->bufpos += (size_t)s;
  }
}

static size_t cfg_note (struct cfgst *cfgst, logcat_t cat, size_t bsz, const char *fmt, va_list ap)
{
  /* Have to snprintf our way to a single string so we can OS_REPORT
     as well as nn_log.  Otherwise configuration errors will be lost
     completely on platforms where stderr doesn't actually work for
     outputting error messages (this includes Windows because of the
     way "ospl start" does its thing). */
  struct cfg_note_buf bb;
  int i, sidx;
  size_t r;

  bb.bufpos = 0;
  bb.bufsize = (bsz == 0) ? 1024 : bsz;
  if ((bb.buf = os_malloc (bb.bufsize)) == NULL)
    NN_FATAL0 ("cfg_note: out of memory\n");

  cfg_note_snprintf (&bb, "config: ");

  /* Path to element/attribute causing the error. Have to stop once an
     attribute is reached: a NULL marker may have been pushed onto the
     stack afterward in the default handling. */
  sidx = 0;
  while (sidx < cfgst->path_depth && cfgst->path[sidx]->name == NULL)
    sidx++;
  for (i = sidx; i < cfgst->path_depth && (i == sidx || !cfgst->isattr[i-1]); i++)
  {
    if (cfgst->path[i] == NULL)
    {
      assert (i > sidx);
      cfg_note_snprintf (&bb, "/#text");
    }
    else if (cfgst->isattr[i])
    {
      cfg_note_snprintf (&bb, "[@%s]", cfgst->path[i]->name);
    }
    else
    {
      const char *p = strchr (cfgst->path[i]->name, '|');
      int n = p ? (int) (p - cfgst->path[i]->name) : (int) strlen (cfgst->path[i]->name);
      cfg_note_snprintf (&bb, "%s%*.*s", (i == sidx) ? "" : "/", n, n, cfgst->path[i]->name);
    }
  }

  cfg_note_snprintf (&bb, ": ");
  if ((r = cfg_note_vsnprintf (&bb, fmt, ap)) > 0)
  {
    /* Can't reset ap ... and va_copy isn't widely available - so
       instead abort and hope the caller tries again with a larger
       initial buffer */
    os_free (bb.buf);
    return r;
  }

  switch (cat)
  {
    case LC_CONFIG:
      nn_log (cat, "%s\n", bb.buf);
      break;
    case LC_WARNING:
      NN_WARNING1 ("%s\n", bb.buf);
      break;
    case LC_ERROR:
      NN_ERROR1 ("%s\n", bb.buf);
      break;
    default:
      NN_FATAL2 ("cfg_note unhandled category %u for message %s\n", (unsigned) cat, bb.buf);
      break;
  }

  os_free (bb.buf);
  return 0;
}

#if WARN_DEPRECATED_ALIAS || WARN_DEPRECATED_UNIT
static void cfg_warning (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  size_t bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_WARNING, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
}
#endif

static int cfg_error (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  size_t bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_ERROR, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
  return 0;
}

static int cfg_log (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  size_t bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_CONFIG, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
  return 0;
}

static int list_index (const char *list[], const char *elem)
{
  int i;
  for (i = 0; list[i] != NULL; i++)
  {
    if (os_strcasecmp (list[i], elem) == 0)
      return i;
  }
  return -1;
}

static int64_t lookup_multiplier (struct cfgst *cfgst, const struct unit *unittab, const char *value, int unit_pos, int value_is_zero, int64_t def_mult, int err_on_unrecognised)
{
  assert (0 <= unit_pos && (size_t) unit_pos <= strlen (value));
  while (value[unit_pos] == ' ')
    unit_pos++;
  if (value[unit_pos] == 0)
  {
    if (value_is_zero)
    {
      /* No matter what unit, 0 remains just that.  For convenience,
         always allow 0 to be specified without a unit */
      return 1;
    }
    else if (def_mult == 0 && err_on_unrecognised)
    {
      cfg_error (cfgst, "%s: unit is required", value);
      return 0;
    }
    else
    {
#if WARN_DEPRECATED_UNIT
      cfg_warning (cfgst, "%s: use of default unit is deprecated", value);
#endif
      return def_mult;
    }
  }
  else
  {
    int i;
    for (i = 0; unittab[i].name != NULL; i++)
    {
      if (strcmp (unittab[i].name, value + unit_pos) == 0)
        return unittab[i].multiplier;
    }
    if (err_on_unrecognised)
      cfg_error (cfgst, "%s: unrecognised unit", value + unit_pos);
    return 0;
  }
}

static void *cfg_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
  assert (cfgelem->multiplicity == 1);
  return (char *) parent + cfgelem->elem_offset;
}

static void *cfg_deref_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
  assert (cfgelem->multiplicity != 1);
  return *((void **) ((char *) parent + cfgelem->elem_offset));
}

static void *if_common (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem, unsigned size)
{
  struct config_listelem **current = (struct config_listelem **) ((char *) parent + cfgelem->elem_offset);
  struct config_listelem *new = os_malloc (size);
  new->next = *current;
  *current = new;
  return new;
}

static int if_thread_properties (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_thread_properties_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (*new));
  if (new == NULL)
    return -1;
  new->name = NULL;
  return 0;
}

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
static int if_channel (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_channel_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (*new));
  if (new == NULL)
    return -1;
  new->name = NULL;
  new->channel_reader_ts = NULL;
  new->dqueue = NULL;
  new->queueId = 0;
  new->evq = NULL;
  new->transmit_conn = NULL;
  return 0;
}
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */

#ifdef DDSI_INCLUDE_ENCRYPTION
static int if_security_profile (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (if_common (cfgst, parent, cfgelem, sizeof (struct config_securityprofile_listelem)) == NULL)
    return -1;
  return 0;
}
#endif /* DDSI_INCLUDE_ENCRYPTION */

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
static int if_network_partition (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_networkpartition_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (*new));
  if (new == NULL)
    return -1;
  new->address_string = NULL;
#ifdef DDSI_INCLUDE_ENCRYPTION
  new->profileName = NULL;
  new->securityProfile = NULL;
#endif /* DDSI_INCLUDE_ENCRYPTION */
  return 0;
}

static int if_ignored_partition (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (if_common (cfgst, parent, cfgelem, sizeof (struct config_ignoredpartition_listelem)) == NULL)
    return -1;
  return 0;
}

static int if_partition_mapping (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (if_common (cfgst, parent, cfgelem, sizeof (struct config_partitionmapping_listelem)) == NULL)
    return -1;
  return 0;
}
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

static int if_peer (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_peer_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (struct config_peer_listelem));
  if (new == NULL)
    return -1;
  new->peer = NULL;
  return 0;
}

static void ff_free (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  void **elem = cfg_address (cfgst, parent, cfgelem);
  os_free (*elem);
}

static int uf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "false", "true", NULL };
  int *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  else
  {
    *elem = idx;
    return 1;
  }
}

static int uf_negated_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  if (!uf_boolean (cfgst, parent, cfgelem, first, value))
    return 0;
  else
  {
    int *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = ! *elem;
    return 1;
  }
}

#if ! LITE
static int uf_boolean_default (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "default", "false", "true", NULL };
  static const enum boolean_default ms[] = {
    BOOLDEF_DEFAULT, BOOLDEF_FALSE, BOOLDEF_TRUE, 0,
  };
  int *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_boolean_default (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum besmode *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case BOOLDEF_DEFAULT: str = "default"; break;
    case BOOLDEF_FALSE: str = "false"; break;
    case BOOLDEF_TRUE: str = "true"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}
#endif

static int uf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char **vs = logcat_names;
  static const logcat_t *lc = logcat_codes;
  char *copy = os_strdup (value), *cursor = copy, *tok;
  while ((tok = os_strsep (&cursor, ",")) != NULL)
  {
    int idx = list_index (vs, tok);
    if (idx < 0)
    {
      int ret = cfg_error (cfgst, "'%s' in '%s' undefined", tok, value);
      os_free (copy);
      return ret;
    }
    enabled_logcats |= lc[idx];
  }
  os_free (copy);
  return 1;
}

static int uf_verbosity (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "finest", "finer", "fine", "config", "info", "warning", "severe", "none", NULL
  };
  static const logcat_t lc[] = {
    LC_ALLCATS, LC_TRAFFIC | LC_TIMING, LC_DISCOVERY, LC_CONFIG, LC_INFO, LC_WARNING, LC_ERROR | LC_FATAL, 0, 0
  };
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  else
  {
    int i;
    for (i = (int) (sizeof (vs) / sizeof (*vs)) - 1; i >= idx; i--)
      enabled_logcats |= lc[i];
    return 1;
  }
}

static int uf_besmode (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "full", "writers", "minimal", NULL
  };
  static const enum besmode ms[] = {
    BESMODE_FULL, BESMODE_WRITERS, BESMODE_MINIMAL, 0,
  };
  int idx = list_index (vs, value);
  enum besmode *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_besmode (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum besmode *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case BESMODE_FULL: str = "full"; break;
    case BESMODE_WRITERS: str = "writers"; break;
    case BESMODE_MINIMAL: str = "minimal"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

#if LITE

static int uf_durability_cdr (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "cdr_le", "cdr_be", "cdr_server", "cdr_client", NULL };
  static const enum durability_cdr ms[] = { DUR_CDR_LE, DUR_CDR_BE, DUR_CDR_SERVER, DUR_CDR_CLIENT, 0 };
  int idx = list_index (vs, value);
  enum durability_cdr * elem = cfg_address (cfgst, parent, cfgelem);
  if (idx < 0)
  {
    return cfg_error (cfgst, "'%s': undefined value", value);
  }
  *elem = ms[idx];
  return 1;
}

static void pf_durability_cdr (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum durability_cdr *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case DUR_CDR_LE: str = "cdr_le"; break;
    case DUR_CDR_BE: str = "cdr_be"; break;
    case DUR_CDR_SERVER: str = "cdr_server"; break;
    case DUR_CDR_CLIENT: str = "cdr_client"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

#endif

static int uf_retransmit_merging (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "never", "adaptive", "always", NULL
  };
  static const enum retransmit_merging ms[] = {
    REXMIT_MERGE_NEVER, REXMIT_MERGE_ADAPTIVE, REXMIT_MERGE_ALWAYS, 0,
  };
  int idx = list_index (vs, value);
  enum retransmit_merging *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_retransmit_merging (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum retransmit_merging *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case REXMIT_MERGE_NEVER: str = "never"; break;
    case REXMIT_MERGE_ADAPTIVE: str = "adaptive"; break;
    case REXMIT_MERGE_ALWAYS: str = "always"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char **elem = cfg_address (cfgst, parent, cfgelem);
  *elem = os_strdup (value);
  return 1;
}

static int uf_natint64_unit (struct cfgst *cfgst, int64_t *elem, const char *value, const struct unit *unittab, int64_t def_mult, int64_t max)
{
  int pos;
  double v_dbl;
  int64_t v_int;
  int64_t mult;
  /* try convert as integer + optional unit; if that fails, try
     floating point + optional unit (round, not truncate, to integer) */
  if (*value == 0)
  {
    *elem = 0; /* some static analyzers don't "get it" */
    return cfg_error (cfgst, "%s: empty string is not a valid value", value);
  }
  else if (sscanf (value, "%lld%n", (long long int *) &v_int, &pos) == 1 && (mult = lookup_multiplier (cfgst, unittab, value, pos, v_int == 0, def_mult, 0)) != 0)
  {
    assert (mult > 0);
    if (v_int < 0 || v_int > max / mult)
      return cfg_error (cfgst, "%s: value out of range", value);
    *elem = mult * v_int;
    return 1;
  }
  else if (sscanf (value, "%lf%n", &v_dbl, &pos) == 1 && (mult = lookup_multiplier (cfgst, unittab, value, pos, v_dbl == 0, def_mult, 1)) != 0)
  {
    double dmult = (double) mult;
    assert (dmult > 0);
    if (v_dbl < 0 || (int64_t) (v_dbl * dmult + 0.5) > max)
      return cfg_error (cfgst, "%s: value out of range", value);
    *elem = (int64_t) (v_dbl * dmult + 0.5);
    return 1;
  }
  else
  {
    *elem = 0; /* some static analyzers don't "get it" */
    return cfg_error (cfgst, "%s: invalid value", value);
  }
}

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
static int uf_bandwidth (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  int64_t bandwidth_bps = 0;
  if (strncmp (value, "inf", 3) == 0) {
    /* special case: inf needs no unit */
    int *elem = cfg_address (cfgst, parent, cfgelem);
    if (strspn (value + 3, " ") != strlen (value + 3) &&
        lookup_multiplier (cfgst, unittab_bandwidth_bps, value, 3, 1, 8, 1) == 0)
      return 0;
    *elem = 0;
    return 1;
  } else if (!uf_natint64_unit (cfgst, &bandwidth_bps, value, unittab_bandwidth_bps, 8, INT64_MAX)) {
    return 0;
  } else if (bandwidth_bps / 8 > INT_MAX) {
    return cfg_error (cfgst, "%s: value out of range", value);
  } else {
    uint32_t *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = (uint32_t) (bandwidth_bps / 8);
    return 1;
  }
}
#endif

static int uf_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  int64_t size = 0;
  if (!uf_natint64_unit (cfgst, &size, value, unittab_memsize, 1, INT32_MAX))
    return 0;
  else
  {
    uint32_t *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = (uint32_t) size;
    return 1;
  }
}

#ifdef DDSI_INCLUDE_ENCRYPTION
static int uf_cipher (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  if (q_security_plugin.cipher_type_from_string)
  {
    q_cipherType *elem = cfg_address (cfgst, parent, cfgelem);
    if ((q_security_plugin.cipher_type_from_string) (value, elem))
    {
      return 1;
    }
    else
    {
      return cfg_error (cfgst, "%s: undefined value", value);
    }
  }
  return 1;
}
#endif /* DDSI_INCLUDE_ENCRYPTION */

#if !LITE
static int uf_service_name (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char **elem = cfg_address (cfgst, parent, cfgelem);
  if (*value == 0)
    *elem = os_strdup (cfgst->servicename);
  else
    *elem = os_strdup (value);
  return 1;
}
#endif

static int uf_tracingOutputFileName (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  struct config *cfg = cfgst->cfg;
#if !LITE
  if (os_strcasecmp (value, "stdout") != 0 && os_strcasecmp (value, "stderr") != 0)
    cfg->tracingOutputFileName = os_fileNormalize (value);
  else
#endif
  {
    cfg->tracingOutputFileName = os_strdup (value);
  }
  return 1;
}

static int uf_ipv4 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* Not actually doing any checking yet */
  return uf_string (cfgst, parent, cfgelem, first, value);
}

static int uf_networkAddress (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  if (os_strcasecmp (value, "auto") != 0)
    return uf_ipv4 (cfgst, parent, cfgelem, first, value);
  else
  {
    char **elem = cfg_address (cfgst, parent, cfgelem);
    *elem = NULL;
    return 1;
  }
}

static void ff_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  char ***elem = cfg_address (cfgst, parent, cfgelem);
  int i;
  for (i = 0; (*elem)[i]; i++)
    os_free ((*elem)[i]);
  os_free (*elem);
}

static int uf_networkAddresses_simple (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char ***elem = cfg_address (cfgst, parent, cfgelem);
  if ((*elem = os_malloc (2 * sizeof(char *))) == NULL)
    return cfg_error (cfgst, "out of memory");
  if (((*elem)[0] = os_strdup (value)) == NULL)
  {
    os_free (*elem);
    *elem = NULL;
    return cfg_error (cfgst, "out of memory");
  }
  (*elem)[1] = NULL;
  return 1;
}

static int uf_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* Check for keywords first */
  {
    static const char *keywords[] = { "all", "any", "none" };
    int i;
    for (i = 0; i < (int) (sizeof (keywords) / sizeof (*keywords)); i++)
    {
      if (os_strcasecmp (value, keywords[i]) == 0)
        return uf_networkAddresses_simple (cfgst, parent, cfgelem, first, keywords[i]);
    }
  }

  /* If not keyword, then comma-separated list of addresses */
  {
    char ***elem = cfg_address (cfgst, parent, cfgelem);
    char *copy;
    unsigned count;

    /* First count how many addresses we have - but do it stupidly by
       counting commas and adding one (so two commas in a row are both
       counted) */
    {
      const char *scan = value;
      count = 1;
      while (*scan)
        count += (*scan++ == ',');
    }

    copy = os_strdup (value);

    /* Allocate an array of address strings (which may be oversized a
       bit because of the counting of the commas) */
    *elem = os_malloc ((count+1) * sizeof (char *));

    {
      char *cursor = copy, *tok;
      unsigned idx = 0;
      while ((tok = os_strsep (&cursor, ",")) != NULL)
      {
        assert (idx < count);
        (*elem)[idx] = os_strdup (tok);
        idx++;
      }
      (*elem)[idx] = NULL;
    }
    os_free (copy);
  }
  return 1;
}

static int uf_allow_multicast (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
#ifdef DDSI_INCLUDE_SSM
  static const char *vs[] = { "false", "spdp", "asm", "ssm", "true", NULL };
  static const unsigned bs[] = { AMC_FALSE, AMC_SPDP, AMC_ASM, AMC_SSM, AMC_TRUE };
#else
  static const char *vs[] = { "false", "spdp", "asm", "true", NULL };
  static const unsigned bs[] = { AMC_FALSE, AMC_SPDP, AMC_ASM, AMC_TRUE };
#endif
  char *copy = os_strdup (value), *cursor = copy, *tok;
  unsigned *elem = cfg_address (cfgst, parent, cfgelem);
  if (copy == NULL)
    return cfg_error (cfgst, "out of memory");
  *elem = 0;
  while ((tok = os_strsep (&cursor, ",")) != NULL)
  {
    int idx = list_index (vs, tok);
    if (idx < 0)
    {
      int ret = cfg_error (cfgst, "'%s' in '%s' undefined", tok, value);
      os_free (copy);
      return ret;
    }
    *elem |= bs[idx];
  }
  os_free (copy);
  return 1;
}

static void pf_allow_multicast (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  unsigned *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case AMC_FALSE: str = "false"; break;
    case AMC_SPDP: str = "spdp"; break;
    case AMC_ASM: str = "asm"; break;
#ifdef DDSI_INCLUDE_SSM
    case AMC_SSM: str = "ssm"; break;
    case (AMC_SPDP | AMC_ASM): str = "spdp,asm"; break;
    case (AMC_SPDP | AMC_SSM): str = "spdp,ssm"; break;
#endif
    case AMC_TRUE: str = "true"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem,UNUSED_ARG (int first), const char *value)
{
  int ret;
  q__schedPrioClass *prio;

  assert (value != NULL);

  prio = cfg_address (cfgst, parent, cfgelem);

  if (os_strcasecmp (value, "relative") == 0) {
    *prio = Q__SCHED_PRIO_RELATIVE;
    ret = 1;
  } else if (os_strcasecmp (value, "absolute") == 0) {
    *prio = Q__SCHED_PRIO_ABSOLUTE;
    ret = 1;
  } else {
    ret = cfg_error (cfgst, "'%s': undefined value", value);
  }

  return ret;
}

static void pf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  char *str;
  q__schedPrioClass *prio = cfg_address (cfgst, parent, cfgelem);

  if (*prio == Q__SCHED_PRIO_RELATIVE) {
    str = "relative";
  } else if (*prio == Q__SCHED_PRIO_ABSOLUTE) {
    str = "absolute";
  } else {
    str = "INVALID";
  }

  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "realtime", "timeshare", "default" };
  static const os_schedClass ms[] = { OS_SCHED_REALTIME, OS_SCHED_TIMESHARE, OS_SCHED_DEFAULT };
  int idx = list_index (vs, value);
  os_schedClass *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  os_schedClass *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case OS_SCHED_DEFAULT: str = "default"; break;
    case OS_SCHED_TIMESHARE: str = "timeshare"; break;
    case OS_SCHED_REALTIME: str = "realtime"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_maybe_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_int32 *elem = cfg_address (cfgst, parent, cfgelem);
  int pos;
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (sscanf (value, "%d%n", &elem->value, &pos) == 1 && value[pos] == 0) {
    elem->isdefault = 0;
    return 1;
  } else {
    return cfg_error (cfgst, "'%s': neither 'default' nor a decimal integer\n", value);
  }
}

static int uf_maybe_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_uint32 *elem = cfg_address (cfgst, parent, cfgelem);
  int64_t size = 0;
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (!uf_natint64_unit (cfgst, &size, value, unittab_memsize, 1, INT32_MAX)) {
    return 0;
  } else {
    elem->isdefault = 0;
    elem->value = (uint32_t) size;
    return 1;
  }
}

#if !LITE
static int uf_maybe_duration_inf (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_int64 *elem = cfg_address (cfgst, parent, cfgelem);
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (os_strcasecmp (value, "inf") == 0) {
    elem->isdefault = 0;
    elem->value = T_NEVER;
    return 1;
  } else if (uf_natint64_unit (cfgst, &elem->value, value, unittab_duration, 0, T_NEVER - 1)) {
    elem->isdefault = 0;
    return 1;
  } else {
    return 0;
  }
}
#endif

static int uf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  float *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  float f = os_strtof (value, &endptr);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a floating point number", value);
  *elem = f;
  return 1;
}

static int uf_int (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  int *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  long v = strtol (value, &endptr, 10);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a decimal integer", value);
  if (v != (int) v)
    return cfg_error (cfgst, "%s: value out of range", value);
  *elem = (int) v;
  return 1;
}

static int uf_duration_gen (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, const char *value, int64_t def_mult, int64_t max_ns)
{
  return uf_natint64_unit (cfgst, cfg_address (cfgst, parent, cfgelem), value, unittab_duration, def_mult, max_ns);
}

static int uf_duration_inf (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  if (os_strcasecmp (value, "inf") == 0)
  {
    int64_t *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = T_NEVER;
    return 1;
  }
  else
  {
    return uf_duration_gen (cfgst, parent, cfgelem, value, 0, T_NEVER - 1);
  }
}

static int uf_duration_ms_1hr (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, T_MILLISECOND, 3600 * T_SECOND);
}

static int uf_duration_ms_1s (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, T_MILLISECOND, T_SECOND);
}

static int uf_duration_us_1s (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, 1000, T_SECOND);
}

static int uf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  int32_t *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  long v = strtol (value, &endptr, 10);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a decimal integer", value);
  if (v != (int32_t) v)
    return cfg_error (cfgst, "%s: value out of range", value);
  *elem = (int32_t) v;
  return 1;
}

static int uf_uint32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  uint32_t *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  unsigned long v = strtoul (value, &endptr, 10);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a decimal integer", value);
  if (v != (uint32_t) v)
    return cfg_error (cfgst, "%s: value out of range", value);
  *elem = (uint32_t) v;
  return 1;
}

static int uf_uint (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  unsigned *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  unsigned long v = strtoul (value, &endptr, 10);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a decimal integer", value);
  if (v != (unsigned) v)
    return cfg_error (cfgst, "%s: value out of range", value);
  *elem = (unsigned) v;
  return 1;
}

static int uf_int_min_max (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value, int min, int max)
{
  int *elem = cfg_address (cfgst, parent, cfgelem);
  if (!uf_int (cfgst, parent, cfgelem, first, value))
    return 0;
  else if (*elem < min || *elem > max)
    return cfg_error (cfgst, "%s: out of range", value);
  else
    return 1;
}

static int uf_domainId (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 230);
}

static int uf_participantIndex (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  int *elem = cfg_address (cfgst, parent, cfgelem);
  if (os_strcasecmp (value, "auto") == 0) {
    *elem = PARTICIPANT_INDEX_AUTO;
    return 1;
  } else if (os_strcasecmp (value, "none") == 0) {
    *elem = PARTICIPANT_INDEX_NONE;
    return 1;
  } else {
    return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 120);
  }
}

static int uf_port (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 1, 65535);
}

static int uf_dyn_port (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, -1, 65535);
}

static int uf_natint (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, INT32_MAX);
}

static int uf_natint_255 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 255);
}

static int do_update (struct cfgst *cfgst, update_fun_t upd, void *parent, struct cfgelem const * const cfgelem, const char *value, int is_default)
{
  struct cfgst_node *n;
  struct cfgst_nodekey key;
  ut_avlIPath_t np;
  int ok;
  key.e = cfgelem;
  if ((n = ut_avlLookupIPath (&cfgst_found_treedef, &cfgst->found, &key, &np)) == NULL)
  {
    if ((n = os_malloc (sizeof (*n))) == NULL)
      return cfg_error (cfgst, "out of memory");

    n->key = key;
    n->count = 0;
    n->failed = 0;
    n->is_default = is_default;
    ut_avlInsertIPath (&cfgst_found_treedef, &cfgst->found, n, &np);
  }
  if (cfgelem->multiplicity == 0 || n->count < cfgelem->multiplicity)
    ok = upd (cfgst, parent, cfgelem, (n->count == n->failed), value);
  else
    ok = cfg_error (cfgst, "only %d instance(s) allowed",cfgelem->multiplicity);
  n->count++;
  if (!ok)
  {
    n->failed++;
  }
  return ok;
}

static int set_default (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (cfgelem->defvalue == NULL)
    return cfg_error (cfgst, "element missing in configuration");
  return do_update (cfgst, cfgelem->update, parent, cfgelem, cfgelem->defvalue, 1);
}

static int set_defaults (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int clear_found)
{
  const struct cfgelem *ce;
  int ok = 1;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_node *n;
    struct cfgst_nodekey key;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce, parent);
    if (ce->multiplicity == 1)
    {
      if (ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key) == NULL)
      {
        if (ce->update)
        {
          int ok1;
          cfgst_push (cfgst, 0, NULL, NULL);
          ok1 = set_default (cfgst, parent, ce);
          cfgst_pop (cfgst);
          ok = ok && ok1;
        }
      }
      if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
      {
        if (clear_found)
        {
          ut_avlDelete (&cfgst_found_treedef, &cfgst->found, n);
          os_free (n);
        }
      }
      if (ce->children)
      {
        int ok1 = set_defaults (cfgst, parent, 0, ce->children, clear_found);
        ok = ok && ok1;
      }
      if (ce->attributes)
      {
        int ok1 = set_defaults (cfgst, parent, 1, ce->attributes, clear_found);
        ok = ok && ok1;
      }
    }
    cfgst_pop (cfgst);
  }
  return ok;
}

static void pf_nop (UNUSED_ARG (struct cfgst *cfgst), UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
}

static void pf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "(null)", is_default ? " [def]" : "");
}

static void pf_int64_unit (struct cfgst *cfgst, int64_t value, int is_default, const struct unit *unittab, const char *zero_unit)
{
  if (value == 0)
  {
    /* 0s is a bit of a special case: we don't want to print 0hr (or
       whatever unit will have the greatest multiplier), so hard-code
       as 0s */
    cfg_log (cfgst, "0 %s%s", zero_unit, is_default ? " [def]" : "");
  }
  else
  {
    int64_t m = 0;
    const char *unit = NULL;
    int i;
    for (i = 0; unittab[i].name != NULL; i++)
    {
      if (unittab[i].multiplier > m && (value % unittab[i].multiplier) == 0)
      {
        m = unittab[i].multiplier;
        unit = unittab[i].name;
      }
    }
    assert (m > 0);
    assert (unit != NULL);
    cfg_log (cfgst, "%lld %s%s", value / m, unit, is_default ? " [def]" : "");
  }
}

#ifdef DDSI_INCLUDE_ENCRYPTION
static void pf_key (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
  cfg_log (cfgst, "<hidden, see configfile>");
}

static void pf_cipher (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  q_cipherType *p = cfg_address (cfgst, parent, cfgelem);
  if (q_security_plugin.cipher_type)
  {
    cfg_log (cfgst, "%s%s", (q_security_plugin.cipher_type) (*p), is_default ? " [def]" : "");
  }
}
#endif /* DDSI_INCLUDE_ENCRYPTION */

static void pf_networkAddress (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "auto", is_default ? " [def]" : "");
}

static void pf_participantIndex (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  switch (*p)
  {
    case PARTICIPANT_INDEX_NONE:
      cfg_log (cfgst, "none%s", is_default ? " [def]" : "");
      break;
    case PARTICIPANT_INDEX_AUTO:
      cfg_log (cfgst, "auto%s", is_default ? " [def]" : "");
      break;
    default:
      cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
      break;
  }
}

static void pf_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int i;
  char ***p = cfg_address (cfgst, parent, cfgelem);
  for (i = 0; (*p)[i] != NULL; i++)
    cfg_log (cfgst, "%s%s", (*p)[i], is_default ? " [def]" : "");
}

static void pf_int (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static void pf_uint (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  unsigned *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%u%s", *p, is_default ? " [def]" : "");
}

static void pf_duration (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  const int64_t *elem = cfg_address (cfgst, parent, cfgelem);
  if (*elem == T_NEVER)
    cfg_log (cfgst, "inf%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, *elem, is_default, unittab_duration, "s");
}

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
static void pf_bandwidth (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  const uint32_t *elem = cfg_address (cfgst, parent, cfgelem);
  if (*elem == 0)
    cfg_log (cfgst, "inf%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, *elem, is_default, unittab_bandwidth_Bps, "B/s");
}
#endif

static void pf_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  const int *elem = cfg_address (cfgst, parent, cfgelem);
  pf_int64_unit (cfgst, *elem, is_default, unittab_memsize, "B");
}

static void pf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int32_t *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static void pf_uint32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  uint32_t *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%u%s", *p, is_default ? " [def]" : "");
}

static void pf_maybe_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_int32 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else
    cfg_log (cfgst, "%d%s", p->value, is_default ? " [def]" : "");
}

static void pf_maybe_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_uint32 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, p->value, is_default, unittab_memsize, "B");
}

#if !LITE
static void pf_maybe_duration (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_int64 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else if (p->value == T_NEVER)
    cfg_log (cfgst, "inf%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, p->value, is_default, unittab_duration, "s");
}
#endif

static void pf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  float *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%f%s", *p, is_default ? " [def]" : "");
}

static void pf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? "true" : "false", is_default ? " [def]" : "");
}

static void pf_negated_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? "false" : "true", is_default ? " [def]" : "");
}

static int uf_standards_conformance (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "pedantic", "strict", "lax", NULL
  };
  static const logcat_t lc[] = {
    NN_SC_PEDANTIC, NN_SC_STRICT, NN_SC_LAX, 0
  };
  enum nn_standards_conformance *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  else
  {
    *elem = lc[idx];
    return 1;
  }
}

static void pf_standards_conformance (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum nn_standards_conformance *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case NN_SC_PEDANTIC: str = "pedantic"; break;
    case NN_SC_STRICT: str = "strict"; break;
    case NN_SC_LAX: str = "lax"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static void pf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
  logcat_t remaining = config.enabled_logcats;
  char res[256] = "", *resp = res;
  const char *prefix = "";
  size_t i;
#ifndef NDEBUG
  {
    size_t max;
    for (i = 0, max = 0; i < sizeof (logcat_codes) / sizeof (*logcat_codes); i++)
      max += 1 + strlen (logcat_names[i]);
    max += 11; /* ,0x%x */
    max += 1; /* \0 */
    assert (max <= sizeof (res));
  }
#endif
  /* TRACE enables ALLCATS, all the others just one */
  if ((remaining & LC_ALLCATS) == LC_ALLCATS)
  {
    resp += snprintf (resp, 256, "%strace", prefix);
    remaining &= ~LC_ALLCATS;
    prefix = ",";
  }
  for (i = 0; i < sizeof (logcat_codes) / sizeof (*logcat_codes); i++)
  {
    if (remaining & logcat_codes[i])
    {
      resp += snprintf (resp, 256, "%s%s", prefix, logcat_names[i]);
      remaining &= ~logcat_codes[i];
      prefix = ",";
    }
  }
  if (remaining)
  {
    resp += snprintf (resp, 256, "%s0x%x", prefix, (unsigned) remaining);
  }
  assert (resp <= res + sizeof (res));
  /* can't do default indicator: user may have specified Verbosity, in
     which case EnableCategory is at default, but for these two
     settings, I don't mind. */
  cfg_log (cfgst, "%s", res);
}


static void print_configitems (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int unchecked)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    struct cfgst_node *n;
    if (ce->name[0] == '>') /* moved, so don't care */
      continue;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce, parent);
    if (ce->multiplicity == 1)
    {
      if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
      {
        cfgst_push (cfgst, 0, NULL, NULL);
        ce->print (cfgst, parent, ce, n->is_default);
        cfgst_pop (cfgst);
      }
      else
      {
        if (unchecked && ce->print)
        {
          cfgst_push (cfgst, 0, NULL, NULL);
          ce->print (cfgst, parent, ce, 0);
          cfgst_pop (cfgst);
        }
      }

      if (ce->children)
        print_configitems (cfgst, parent, 0, ce->children, unchecked);
      if (ce->attributes)
        print_configitems (cfgst, parent, 1, ce->attributes, unchecked);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      while (p)
      {
        cfgst_push (cfgst, 0, NULL, NULL);
        if (ce->print)
        {
          ce->print (cfgst, p, ce, 0);
        }
        cfgst_pop (cfgst);
        if (ce->attributes)
          print_configitems (cfgst, p, 1, ce->attributes, 1);
        if (ce->children)
          print_configitems (cfgst, p, 0, ce->children, 1);
        p = p->next;
      }
    }
    cfgst_pop (cfgst);
  }
}


static void free_all_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;

  for (ce = cfgelem; ce && ce->name; ce++)
  {
    if (ce->name[0] == '>') /* moved, so don't care */
      continue;

    if (ce->free)
      ce->free (cfgst, parent, ce);

    if (ce->multiplicity == 1)
    {
      if (ce->children)
        free_all_elements (cfgst, parent, ce->children);
      if (ce->attributes)
        free_all_elements (cfgst, parent, ce->attributes);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      struct config_listelem *r ;
      while (p) {
        if (ce->attributes)
          free_all_elements (cfgst, p, ce->attributes);
        if (ce->children)
          free_all_elements (cfgst, p, ce->children);
        r = p;
        p = p->next;
        os_free(r);
      }
    }
  }
}

static void free_configured_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    struct cfgst_node *n;
    if (ce->name[0] == '>') /* moved, so don't care */
      continue;
    key.e = ce;
    if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
    {
      if (ce->free && n->count > n->failed)
        ce->free (cfgst, parent, ce);
    }

    if (ce->multiplicity == 1)
    {
      if (ce->children)
        free_configured_elements (cfgst, parent, ce->children);
      if (ce->attributes)
        free_configured_elements (cfgst, parent, ce->attributes);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      struct config_listelem *r;
      while (p)
      {
        if (ce->attributes)
          free_all_elements (cfgst, p, ce->attributes);
        if (ce->children)
          free_all_elements (cfgst, p, ce->children);
        r = p;
        p = p->next;
        os_free(r);
      }
    }
  }
}

static int matching_name_index (const char *name_w_aliases, const char *name)
{
  const char *ns = name_w_aliases, *p = strchr (ns, '|');
  int idx = 0;
  while (p)
  {
    if (os_strncasecmp (ns, name, (size_t) (p-ns)) == 0 && name[p-ns] == 0)
    {
      /* ns upto the pipe symbol is a prefix of name, and name is terminated at that point */
      return idx;
    }
    /* If primary name followed by '||' instead of '|', aliases are non-warning */
    ns = p + 1 + (idx == 0 && p[1] == '|');
    p = strchr (ns, '|');
    idx++;
  }
  return (os_strcasecmp (ns, name) == 0) ? idx : -1;
}

static const struct cfgelem *lookup_redirect (const char *target)
{
  const struct cfgelem *cfgelem = ddsi2_cfgelems;
  char *target_copy = os_strdup(target), *p1;
  const char *p = target_copy;
  while (p) {
    p1 = strchr (p, '/');
    if (p1) *p1++ = 0;
    for (; cfgelem->name; cfgelem++)
    {
      /* not supporting multiple redirects */
      assert(cfgelem->name[0] != '>');
      if (matching_name_index (cfgelem->name, p) >= 0)
        break;
    }
    if (p1)
    {
      cfgelem = cfgelem->children;
    }
    p = p1;
  }
  os_free(target_copy);
  return cfgelem;
}

static int proc_elem_open (void *varg, UNUSED_ARG(uintptr_t parentinfo), UNUSED_ARG(uintptr_t *eleminfo), const char *name)
{
  struct cfgst * const cfgst = varg;
  const struct cfgelem *cfgelem = cfgst_tos (cfgst);
  const struct cfgelem *cfg_subelem;
  int moved = 0;
  if (cfgelem == NULL)
  {
    /* Ignoring, but do track the structure so we can know when to stop ignoring */
    cfgst_push (cfgst, 0, NULL, NULL);
    return 1;
  }
  for (cfg_subelem = cfgelem->children; cfg_subelem && cfg_subelem->name && strcmp (cfg_subelem->name, "*") != 0; cfg_subelem++)
  {
    const char *csename = cfg_subelem->name;
    int idx;
    moved = (csename[0] == '>');
    if (moved)
      csename++;
    idx = matching_name_index (csename, name);
#if WARN_DEPRECATED_ALIAS
    if (idx > 0)
    {
      int n = (int) (strchr (csename, '|') - csename);
      if (csename[n+1] != '|')
        cfg_warning (cfgst, "'%s': deprecated alias for '%*.*s'", name, n, n, csename);
    }
#endif
    if (idx >= 0)
    {
      break;
    }
  }
  if (cfg_subelem == NULL || cfg_subelem->name == NULL)
    return cfg_error (cfgst, "%s: unknown element", name);
  else if (strcmp (cfg_subelem->name, "*") == 0)
  {
    /* Push a marker that we are to ignore this part of the DOM tree */
    cfgst_push (cfgst, 0, NULL, NULL);
    return 1;
  }
  else
  {
    void *parent, *dynparent;

    if (moved)
    {
#if WARN_DEPRECATED_ALIAS
      cfg_warning (cfgst, "'%s': deprecated alias for '%s'", name, cfg_subelem->defvalue);
#endif
      cfg_subelem = lookup_redirect (cfg_subelem->defvalue);
    }

    parent = cfgst_parent (cfgst);
    assert (cfgelem->init || cfgelem->multiplicity == 1); /*multi-items must have an init-func */
    if (cfg_subelem->init)
    {
      if (cfg_subelem->init (cfgst, parent, cfg_subelem) < 0)
        return 0;
    }

    if (cfg_subelem->multiplicity != 1)
      dynparent = cfg_deref_address (cfgst, parent, cfg_subelem);
    else
      dynparent = parent;

    cfgst_push (cfgst, 0, cfg_subelem, dynparent);
    return 1;
  }
}

static int proc_attr (void *varg, UNUSED_ARG(uintptr_t eleminfo), const char *name, const char *value)
{
  /* All attributes are processed immediately after opening the element */
  struct cfgst * const cfgst = varg;
  const struct cfgelem *cfgelem = cfgst_tos (cfgst);
  const struct cfgelem *cfg_attr;
  if (cfgelem == NULL)
    return 1;
  for (cfg_attr = cfgelem->attributes; cfg_attr && cfg_attr->name; cfg_attr++)
  {
    if (os_strcasecmp (cfg_attr->name, name) == 0)
      break;
  }
  if (cfg_attr == NULL || cfg_attr->name == NULL)
    return cfg_error (cfgst, "%s: unknown attribute", name);
  else
  {
    void *parent = cfgst_parent (cfgst);
    int ok;
    cfgst_push (cfgst, 1, cfg_attr, parent);
    ok = do_update (cfgst, cfg_attr->update, parent, cfg_attr, value, 0);
    cfgst_pop (cfgst);
    return ok;
  }
}

static int proc_elem_data (void *varg, UNUSED_ARG(uintptr_t eleminfo), const char *value)
{
  struct cfgst * const cfgst = varg;
  const struct cfgelem *cfgelem = cfgst_tos (cfgst);
  if (cfgelem == NULL)
    return 1;
  if (cfgelem->update == 0)
    return cfg_error (cfgst, "%s: no data expected", value);
  else
  {
    void *parent = cfgst_parent (cfgst);
    int ok;
    cfgst_push (cfgst, 0, NULL, parent);
    ok = do_update (cfgst, cfgelem->update, parent, cfgelem, value, 0);
    cfgst_pop (cfgst);
    return ok;
  }
}

static int proc_elem_close (void *varg, UNUSED_ARG(uintptr_t eleminfo))
{
  struct cfgst * const cfgst = varg;
  const struct cfgelem * cfgelem = cfgst_tos (cfgst);
  int ok = 1;
  if (cfgelem && cfgelem->multiplicity != 1)
  {
    void *parent = cfgst_parent (cfgst);
    int ok1;
    ok1 = set_defaults (cfgst, parent, 1, cfgelem->attributes, 1);
    ok = ok && ok1;
    ok1 = set_defaults (cfgst, parent, 0, cfgelem->children, 1);
    ok = ok && ok1;
  }
  cfgst_pop (cfgst);
  return ok;
}

static void proc_error (void *varg, const char *msg, int line)
{
  struct cfgst * const cfgst = varg;
  cfg_error (cfgst, "parser error %s at line %d", msg, line);
}

#if ! LITE
static int walk_element (struct cfgst *cfgst, const char *name, u_cfElement elem);

static int walk_attributes (struct cfgst *cfgst, u_cfElement base)
{
  c_iter iter;
  u_cfNode child;
  int ok = 1;
  iter = u_cfElementGetAttributes (base);
  child = u_cfNode (c_iterTakeFirst (iter));
  while (child)
  {
    u_cfAttribute attr;
    c_char *name, *value;
    int ok1 = 0;
    name = u_cfNodeName (child);
    assert (name != NULL);
    assert (u_cfNodeKind (child) == V_CFATTRIBUTE);
    attr = u_cfAttribute (child);
    if (!u_cfAttributeStringValue (attr, &value))
      ok1 = cfg_error (cfgst, "failed to extract data");
    else
    {
      ok1 = proc_attr (cfgst, name, value);
      os_free (value);
    }
    ok = ok && ok1;
    os_free (name);
    u_cfNodeFree (child);
    child = u_cfNode (c_iterTakeFirst (iter));
  }
  c_iterFree (iter);
  return ok;
}

static int walk_children (struct cfgst *cfgst, u_cfElement base)
{
  c_iter iter;
  u_cfNode child;
  int ok = 1;
  iter = u_cfElementGetChildren (base);
  child = u_cfNode (c_iterTakeFirst (iter));
  while (child)
  {
    c_char *child_name;
    int ok1 = 0;
    child_name = u_cfNodeName (child);
    assert (child_name != NULL);
    switch (u_cfNodeKind (child))
    {
      case V_CFELEMENT:
      {
        u_cfElement elem = u_cfElement (child);
        ok1 = walk_element (cfgst, child_name, elem);
        break;
      }
      case V_CFDATA:
      {
        u_cfData data = u_cfData (child);
        c_char *value;
        if (!u_cfDataStringValue (data, &value))
          ok1 = cfg_error (cfgst, "failed to extract data");
        else
        {
          if (strspn (value, " \t\r\n") != strlen (value))
            ok1 = proc_elem_data (cfgst, value);
          else
            ok1 = 1;
          os_free (value);
        }
        break;
      }
      default:
        abort ();
    }
    ok = ok && ok1;
    os_free (child_name);
    u_cfNodeFree (child);
    child = u_cfNode (c_iterTakeFirst (iter));
  }
  c_iterFree (iter);
  return ok;
}

static int walk_element (struct cfgst *cfgst, const char *name, u_cfElement elem)
{
  if (!proc_elem_open (cfgst, name))
    return 0;
  else
  {
    int ok;
    ok = walk_attributes (cfgst, elem) && walk_children (cfgst, elem);
    if (!proc_elem_close (cfgst))
      ok = 0;
    return ok;
  }
}
#endif

static int cfgst_node_cmp (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (struct cfgst_nodekey));
}

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
static int set_default_channel (struct config *cfg)
{
  if (cfg->channels == NULL)
  {
    /* create one default channel if none configured */
    struct config_channel_listelem *c;
    if ((c = os_malloc (sizeof (*c))) == NULL)
      return ERR_OUT_OF_MEMORY;
    c->next = NULL;
    c->name = os_strdup ("user");
    c->priority = 0;
    c->resolution = 1 * T_MILLISECOND;
#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
    c->data_bandwidth_limit = 0;
    c->auxiliary_bandwidth_limit = 0;
#endif
    c->queue_size = 0;
    c->diffserv_field = 0;
    c->channel_reader_ts = NULL;
    c->queueId = 0;
    c->dqueue = NULL;
    c->evq = NULL;
    c->transmit_conn = NULL;
    cfg->channels = c;
  }
  return 0;
}

static int sort_channels_cmp (const void *va, const void *vb)
{
  const struct config_channel_listelem * const *a = va;
  const struct config_channel_listelem * const *b = vb;
  return ((*a)->priority == (*b)->priority) ? 0 : ((*a)->priority < (*b)->priority) ? -1 : 1;
}

static int sort_channels_check_nodups (struct config *cfg)
{
  /* Selecting a channel is much easier & more elegant if the channels
     are sorted on descending priority.  While we do retain the list
     structure, sorting is much easier in an array, and hence we
     convert back and forth. */
  struct config_channel_listelem **ary, *c;
  unsigned i, n;
  int result;

  n = 0;
  for (c = cfg->channels; c; c = c->next)
    n++;
  assert (n > 0);

  ary = os_malloc (n * sizeof (*ary));

  i = 0;
  for (c = cfg->channels; c; c = c->next)
    ary[i++] = c;
  qsort (ary, n, sizeof (*ary), sort_channels_cmp);

  result = 0;
  for (i = 0; i < n-1; i++)
  {
    if (ary[i]->priority == ary[i+1]->priority)
    {
      NN_ERROR3 ("config: duplicate channel definition for priority %u: channels %s and %s\n",
                 ary[i]->priority, ary[i]->name, ary[i+1]->name);
      result = ERR_ENTITY_EXISTS;
    }
  }

  if (result == 0)
  {
    cfg->channels = ary[0];
    for (i = 0; i < n-1; i++)
      ary[i]->next = ary[i+1];
    ary[i]->next = NULL;
    cfg->max_channel = ary[i];
  }

  os_free (ary);
  return result;
}
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */

struct cfgst * config_init
(
#if LITE
  const char *configfile
#else
  /* C_STRUCT (u_participant) const * */u_participant participant, const char *servicename
#endif
)
{
  int ok = 1;
  struct cfgst *cfgst;
#if !LITE
  u_cfElement root, elem;
  c_iter iter;
  int rootidx;
  assert (participant != NULL);
#endif

  memset (&config, 0, sizeof (config));

#if ! LITE
  /* Enable logging of errors &c. to stderr until configuration is read */
  config.enabled_logcats = LC_FATAL | LC_ERROR | LC_WARNING;
#ifdef DDSI_INCLUDE_ENCRYPTION
  ddsi_security_plugin ();
#endif
#endif
  config.tracingOutputFile = stderr;

  cfgst = os_malloc (sizeof (*cfgst));
  memset (cfgst, 0, sizeof (*cfgst));

  ut_avlInit (&cfgst_found_treedef, &cfgst->found);
  cfgst->cfg = &config;
#if !LITE
  cfgst->servicename = servicename;
#endif

#if LITE
  /* configfile == NULL will get you the default configuration */
  if (configfile)
  {
    char *copy = os_strdup (configfile), *cursor = copy, *tok;
    while ((tok = os_strsep (&cursor, ",")) != NULL)
    {
      struct ut_xmlpCallbacks cb;
      struct ut_xmlpState *qx;
      FILE *fp;

      if ((fp = fopen (tok, "r")) == NULL)
      {
        if (strncmp (tok, "file://", 7) != 0 || (fp = fopen (tok + 7, "r")) == NULL)
        {
          os_free (copy);
          os_free (cfgst);
          NN_ERROR1 ("can't open configuration file %s\n", tok);
          return NULL;
        }
      }

      cb.attr = proc_attr;
      cb.elem_close = proc_elem_close;
      cb.elem_data = proc_elem_data;
      cb.elem_open = proc_elem_open;
      cb.error = proc_error;

      if ((qx = ut_xmlpNewFile (fp, cfgst, &cb)) == NULL)
      {
        fclose (fp);
        os_free (copy);
        os_free (cfgst);
        return NULL;
      }
      cfgst_push (cfgst, 0, &root_cfgelem, &config);

      ok = ut_xmlpParse (qx);
      /* Pop until stack empty: error handling is rather brutal */
      assert (!ok || cfgst->path_depth == 1);
      while (cfgst->path_depth > 0)
        cfgst_pop (cfgst);
      ut_xmlpFree (qx);
      fclose (fp);
    }
    os_free (copy);
  }
#else
  if ((root = u_participantGetConfiguration ((u_participant) participant)) == NULL)
  {
    NN_ERROR0 ("config_init: u_participantGetConfiguration failed");
    ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
    os_free (cfgst);
    return NULL;
  }

  /* Only suitable for Domain (without a attributes) and a service
   with a matching name attribute */
  cfgst_push (cfgst, 0, &root_cfgelem, &config);
  for (rootidx = 0; root_cfgelems[rootidx].name; rootidx++)
  {
    const struct cfgelem *root_cfgelem = &root_cfgelems[rootidx];
    char *copy = os_strdup (root_cfgelem->name), *cursor = copy, *tok;
    while ((tok = os_strsep (&cursor, "|")) != NULL)
    {
      if (*tok == 0)
      {
        /* The configuration tables are supposed to be reasonable and not contain empty tags. Then, "||" is returned as an empty token by os_strsep, but we can simply skip it */
        continue;
      }
      iter = u_cfElementXPath (root, tok);
      elem = u_cfElement (c_iterTakeFirst (iter));
      while (elem)
      {
        c_char *str;
        if (root_cfgelem->attributes == NULL)
        {
          /* Domain element */
          int ok1;
          char *name = u_cfNodeName (u_cfNode (elem));
          ok1 = walk_element (cfgst, name, elem);
          os_free (name);
          ok = ok && ok1;
        }
        else if (u_cfElementAttributeStringValue (elem, "name", &str))
        {
          int ok1;
          if (os_strcasecmp (servicename, str) != 0)
            ok1 = 1;
          else
          {
            char *name = u_cfNodeName (u_cfNode (elem));
            ok1 = walk_element (cfgst, name, elem);
            os_free (name);
          }
          ok = ok && ok1;
          os_free (str);
        }
        u_cfElementFree (elem);
        elem = u_cfElement (c_iterTakeFirst (iter));
      }
      c_iterFree (iter);
    }
    os_free (copy);
  }
  cfgst_pop (cfgst);
  u_cfElementFree (root);
#endif

  /* Set defaults for everything not set that we have a default value
     for, signal errors for things unset but without a default. */
  {
    int ok1 = set_defaults (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
    ok = ok && ok1;
  }

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  /* Default channel gets set outside set_defaults -- a bit too
     complicated for the poor framework */

  if (set_default_channel (cfgst->cfg) < 0)
  {
    ok = 0;
  }
  if (cfgst->cfg->channels && sort_channels_check_nodups (cfgst->cfg) < 0)
  {
    ok = 0;
  }
#endif

#ifdef DDSI_INCLUDE_ENCRYPTION
  /* Check security profiles */
  {
    struct config_securityprofile_listelem *s = config.securityProfiles;
    while (s)
    {
      switch (s->cipher)
      {
        case Q_CIPHER_UNDEFINED:
        case Q_CIPHER_NULL:
          /* nop */
          if (s->key && strlen (s->key) > 0) {
            NN_ERROR1 ("config: DDSI2EService/Security/SecurityProfile[@cipherkey]: %s: cipher key not required\n",s->key);
          }
          break;

        default:
          /* read the cipherkey if present */
          if (!s->key || strlen (s->key) == 0) {
            NN_ERROR0 ("config: DDSI2EService/Security/SecurityProfile[@cipherkey]: cipher key missing\n");
            ok = 0;
          } else if (q_security_plugin.valid_uri && ! (q_security_plugin.valid_uri) (s->cipher,s->key)) {
            NN_ERROR1 ("config: DDSI2EService/Security/SecurityProfile[@cipherkey]: %s : incorrect key\n", s->key);
            ok = 0;
          }
      }
      s = s->next;
    }
  }
#endif /* DDSI_INCLUDE_ENCRYPTION */

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  /* Assign network partition ids */
#ifdef DDSI_INCLUDE_ENCRYPTION
  /* also create links from the network partitions to the
     securityProfiles and signal errors if profiles do not exist */
#endif /* DDSI_INCLUDE_ENCRYPTION */
  {
    struct config_networkpartition_listelem *p = config.networkPartitions;
    config.nof_networkPartitions = 0;
    while (p)
    {
#ifdef DDSI_INCLUDE_ENCRYPTION
      if (os_strcasecmp (p->profileName, "null") == 0)
        p->securityProfile = NULL;
      else
      {
        struct config_securityprofile_listelem *s = config.securityProfiles;
        while (s && os_strcasecmp (p->profileName, s->name) != 0)
          s = s->next;
        if (s)
          p->securityProfile = s;
        else
        {
          NN_ERROR1 ("config: DDSI2EService/Partitioning/NetworkPartitions/NetworkPartition[@securityprofile]: %s: unknown securityprofile\n", p->profileName);
          ok = 0;
        }
      }
#endif /* DDSI_INCLUDE_ENCRYPTION */
      config.nof_networkPartitions++;
      /* also use crc32 just like native nw and ordinary ddsi2e, only
         for interoperability because it is asking for trouble &
         forces us to include a crc32 routine when we have md5
         available anyway */
      p->partitionId = config.nof_networkPartitions; /* starting at 1 */
#if LITE
      p->partitionHash = crc32_calc (p->name, (int) strlen (p->name));
#else
      p->partitionHash = ut_crcCalculate (p->name, strlen (p->name));
#endif
      p = p->next;
    }
  }

  /* Create links from the partitionmappings to the network partitions
     and signal errors if partitions do not exist */
  {
    struct config_partitionmapping_listelem * m = config.partitionMappings;
    while (m) {
      struct config_networkpartition_listelem * p = config.networkPartitions;
      while (p && os_strcasecmp(m->networkPartition, p->name) != 0) {
        p = p->next;
      }
      if (p) {
        m->partition = p;
      } else {
        NN_ERROR1 ("config: DDSI2EService/Partitioning/PartitionMappings/PartitionMapping[@networkpartition]: %s: unknown partition\n", m->networkPartition);
        ok = 0;
      }
      m = m->next;
    }
  }
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

  /* Now switch to configured tracing settings */
  config.enabled_logcats = enabled_logcats;

  if (!ok)
  {
    free_configured_elements (cfgst, cfgst->cfg, root_cfgelems);
  }

  if (ok)
  {
    config.valid = 1;
    return cfgst;
  }
  else
  {
    ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
    os_free (cfgst);
    return NULL;
  }
}

void config_print_and_free_cfgst (struct cfgst *cfgst)
{
  if (cfgst == NULL)
    return;
  print_configitems (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
  ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
  os_free (cfgst);
}

void config_fini (void)
{
  if (config.valid)
  {
    struct cfgst cfgst;
    cfgst.cfg = &config;
    free_all_elements (&cfgst, cfgst.cfg, root_cfgelems);
    if (config.tracingOutputFile)
    {
      fclose (config.tracingOutputFile);
    }
    memset (&config, 0, sizeof (config));
  }
}

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
static char *get_partition_search_pattern (const char *partition, const char *topic)
{
  char *pt = os_malloc (strlen (partition) + strlen (topic) + 2);
  os_sprintf (pt, "%s.%s", partition, topic);
  return pt;
}

struct config_partitionmapping_listelem *find_partitionmapping (const char *partition, const char *topic)
{
  char *pt = get_partition_search_pattern (partition, topic);
  struct config_partitionmapping_listelem *pm;
  for (pm = config.partitionMappings; pm; pm = pm->next)
    if (WildcardOverlap (pt, pm->DCPSPartitionTopic))
      break;
  os_free (pt);
  return pm;
}

struct config_networkpartition_listelem *find_networkpartition_by_id (uint32_t id)
{
  struct config_networkpartition_listelem *np;
  for (np = config.networkPartitions; np; np = np->next)
    if (np->partitionId == id)
      return np;
  return 0;
}

int is_ignored_partition (const char *partition, const char *topic)
{
  char *pt = get_partition_search_pattern (partition, topic);
  struct config_ignoredpartition_listelem *ip;
  for (ip = config.ignoredPartitions; ip; ip = ip->next)
    if (WildcardOverlap (pt, ip->DCPSPartitionTopic))
      break;
  os_free (pt);
  return ip != NULL;
}
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
struct config_channel_listelem *find_channel (nn_transport_priority_qospolicy_t transport_priority)
{
  struct config_channel_listelem *c;
  /* Channel selection is to use the channel with the lowest priority
     not less than transport_priority, or else the one with the
     highest priority. */
  assert (config.channels != NULL);
  assert (config.max_channel != NULL);
  for (c = config.channels; c; c = c->next)
  {
    assert (c->next == NULL || c->next->priority > c->priority);
    if (transport_priority.value <= c->priority)
      return c;
  }
  return config.max_channel;
}
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */
