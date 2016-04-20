#ifndef _DDS_QOS_H_
#define _DDS_QOS_H_

#include "dds_types.h"
#include "dds_entity.h"
#include "q_xqos.h"
#include "q_time.h"
#include "q_plist.h"

#if defined (__cplusplus)
extern "C" {
#endif

int dds_qos_validate (dds_entity_kind_t kind, const dds_qos_t * qos);

/* required by CPP to set defaults */

DDS_EXPORT bool dds_reader_qos_validate (const dds_qos_t * qos);
DDS_EXPORT bool dds_writer_qos_validate (const dds_qos_t * qos);
DDS_EXPORT bool dds_pubsub_qos_validate (const dds_qos_t * qos);
DDS_EXPORT bool dds_topic_qos_validate (const dds_qos_t * qos);

#if defined (__cplusplus)
}
#endif
#endif
