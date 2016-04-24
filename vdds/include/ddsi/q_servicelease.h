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
#ifndef NN_SERVICELEASE_H
#define NN_SERVICELEASE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_servicelease;

struct nn_servicelease *nn_servicelease_new (void (*renew_cb) (void *arg), void *renew_arg);
int nn_servicelease_start_renewing (struct nn_servicelease *sl);
void nn_servicelease_free (struct nn_servicelease *sl);
void nn_servicelease_statechange_barrier (struct nn_servicelease *sl);

#if defined (__cplusplus)
}
#endif

#endif /* NN_SERVICELEASE_H */
