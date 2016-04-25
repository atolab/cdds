/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef Q_PCAP_H
#define Q_PCAP_H

#include <stdio.h>
#include "ddsi/q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct msghdr;

FILE * new_pcap_file (const char *name);

void write_pcap_received
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const os_sockaddr_storage * dst,
  unsigned char * buf,
  size_t sz
);

void write_pcap_sent
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const struct msghdr * hdr,
  size_t sz
);

#if defined (__cplusplus)
}
#endif

#endif /* Q_PCAP_H */
