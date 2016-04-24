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
#ifndef Q_LEASE_H
#define Q_LEASE_H

#include "ddsi/q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct receiver_state;
struct participant;
struct lease;
struct entity_common;
struct thread_state1;

void lease_management_init (void);
void lease_management_term (void);
struct lease *lease_new (int64_t tdur, struct entity_common *e);
void lease_register (struct lease *l);
void lease_free (struct lease *l);
void lease_renew (struct lease *l, nn_wctime_t tnow);
void check_and_handle_lease_expiration (struct thread_state1 *self, nn_wctime_t tnow);

void handle_PMD (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, unsigned len);

#if defined (__cplusplus)
}
#endif

#endif /* Q_LEASE_H */
