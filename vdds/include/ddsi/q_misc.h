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
#ifndef NN_MISC_H
#define NN_MISC_H

#include "ddsi/q_protocol.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct v_gid_s;
struct nn_guid;

int vendor_is_opensplice (nn_vendorid_t vid);
int vendor_is_rti (nn_vendorid_t vendor);
int vendor_is_twinoaks (nn_vendorid_t vendor);
int vendor_is_prismtech (nn_vendorid_t vendor);
int vendor_is_cloud (nn_vendorid_t vendor);
int is_own_vendor (nn_vendorid_t vendor);
unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data);
#if !LITE
int version_info_is_6_4_1 (const char *internals);
#endif

int64_t fromSN (const nn_sequence_number_t sn);
nn_sequence_number_t toSN (int64_t);

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
int WildcardOverlap(char * p1, char * p2);
#endif

int ddsi2_patmatch (const char *pat, const char *str);

#if !LITE
void nn_guid_to_ospl_gid (struct v_gid_s *gid, const struct nn_guid *guid, int guid_has_systemid);
int gid_is_fake (const struct v_gid_s *gid);
#endif

#if LITE
uint32_t crc32_calc (const void *buf, uint32_t length);
#endif

#if defined (__cplusplus)
}
#endif

#endif /* NN_MISC_H */
