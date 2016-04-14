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
#ifndef Q_QOSMATCH_H
#define Q_QOSMATCH_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xqos;

int partition_match_based_on_wildcard_in_left_operand (const struct nn_xqos *a, const struct nn_xqos *b, const char **realname);
int partitions_match_p (const struct nn_xqos *a, const struct nn_xqos *b);
int is_wildcard_partition (const char *str);

/* Returns -1 on success, or QoS id of first mismatch (>=0) */

int32_t qos_match_p (const struct nn_xqos *rd, const struct nn_xqos *wr);

#if defined (__cplusplus)
}
#endif

#endif /* Q_QOSMATCH_H */
