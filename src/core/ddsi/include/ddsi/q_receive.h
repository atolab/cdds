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
#ifndef Q_RECEIVE_H
#define Q_RECEIVE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_rbufpool;
struct nn_rsample_info;
struct nn_rdata;
struct ddsi_tran_listener;

uint32_t recv_thread (struct nn_rbufpool *rbpool);
uint32_t listen_thread (struct ddsi_tran_listener * listener);
int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, const nn_guid_t *rdguid, void *qarg);

#if defined (__cplusplus)
}
#endif

#endif /* Q_RECEIVE_H */
