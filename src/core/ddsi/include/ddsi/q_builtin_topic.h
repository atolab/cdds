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
#ifndef Q_BUILTIN_TOPIC_H
#define Q_BUILTIN_TOPIC_H

#include "ddsi/q_time.h"

#include "dds_builtinTopics.h"

struct entity_common;
struct nn_plist;

/* Functions called at proxy entity creation/deletion time, so they
   can do whatever is necessary to get the builtin topics function
   correctly.

   These probably should return an error code, but I don't quite know
   how to handle it yet and this way we have Coverity on our side.
   Implementation is outside the common core.

   These may assume the proxy entities are stable, without parallel QoS
   changes. */

void
propagate_builtin_topic_participant(
        _In_ const struct entity_common *proxypp,
        _In_ const nn_plist_t *plist,
        _In_ nn_wctime_t timestamp,
        _In_ int alive);

void
propagate_builtin_topic_cmparticipant(
        _In_ const struct entity_common *proxypp,
        _In_ const nn_plist_t *plist,
        _In_ nn_wctime_t timestamp,
        _In_ int alive);
#if 0
void dispose_builtin_topic_proxy_participant (const struct proxy_participant *proxypp, nn_wctime_t timestamp, int isimplicit);
void write_builtin_topic_proxy_writer (const struct proxy_writer *pwr, nn_wctime_t timestamp);
void dispose_builtin_topic_proxy_writer (const struct proxy_writer *pwr, nn_wctime_t timestamp, int isimplicit);
void write_builtin_topic_proxy_reader (const struct proxy_reader *prd, nn_wctime_t timestamp);
void dispose_builtin_topic_proxy_reader (const struct proxy_reader *prd, nn_wctime_t timestamp, int isimplicit);
void write_builtin_topic_proxy_group (const struct proxy_group *pgroup, nn_wctime_t timestamp);
void dispose_builtin_topic_proxy_group (const struct proxy_group *pgroup, nn_wctime_t timestamp, int isimplicit);

void write_builtin_topic_proxy_topic (const struct nn_plist *datap, nn_wctime_t timestamp);
#endif


/*
 * Let the layer on top of DDSI handle the received builtin data when it wants to.
 */
extern void
forward_builtin_participant(
        _In_ DDS_ParticipantBuiltinTopicData *data,
        _In_ nn_wctime_t timestamp,
        _In_ int alive);

extern void
forward_builtin_cmparticipant(
        _In_ DDS_CMParticipantBuiltinTopicData *data,
        _In_ nn_wctime_t timestamp,
        _In_ int alive);


#endif
