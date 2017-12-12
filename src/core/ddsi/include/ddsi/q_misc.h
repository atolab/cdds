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

seqno_t fromSN (const nn_sequence_number_t sn);
nn_sequence_number_t toSN (seqno_t);

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
int WildcardOverlap(char * p1, char * p2);
#endif

int ddsi2_patmatch (const char *pat, const char *str);


uint32_t crc32_calc (const void *buf, uint32_t length);

#if defined (__cplusplus)
}
#endif

#endif /* NN_MISC_H */