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
#ifndef Q_TRANSMIT_H
#define Q_TRANSMIT_H

#include "os/os_defs.h"
#include "ddsi/q_rtps.h" /* for nn_entityid_t */

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xpack;
struct nn_xmsg;
struct writer;
struct proxy_reader;
struct serdata;
struct tkmap_instance;

/* Writing new data; serdata_twrite (serdata) is assumed to be really
   recentish; serdata is unref'd.  If xp == NULL, data is queued, else
   packed. */
int write_sample (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata, struct tkmap_instance *tk);
int write_sample_notk (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata);

/* When calling the following functions, wr->lock must be held */
int create_fragment_message (struct writer *wr, seqno_t seq, const struct nn_plist *plist, struct serdata *serdata, unsigned fragnum, struct proxy_reader *prd,struct nn_xmsg **msg, int isnew);
int enqueue_sample_wrlock_held (struct writer *wr, seqno_t seq, const struct nn_plist *plist, struct serdata *serdata, struct proxy_reader *prd, int isnew);
void add_Heartbeat (struct nn_xmsg *msg, struct writer *wr, int hbansreq, nn_entityid_t dst, int issync);

#if defined (__cplusplus)
}
#endif

#endif /* Q_TRANSMIT_H */
