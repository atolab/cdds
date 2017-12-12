#ifndef _DDS_QOS_H_
#define _DDS_QOS_H_

#include "dds__entity.h"
#include "ddsi/q_xqos.h"
#include "ddsi/q_time.h"
#include "ddsi/q_plist.h"

#if defined (__cplusplus)
extern "C" {
#endif

bool validate_deadline_and_timebased_filter (const nn_duration_t deadline, const nn_duration_t minimum_separation);
bool validate_entityfactory_qospolicy (const nn_entity_factory_qospolicy_t * entityfactory);
bool validate_octetseq (const nn_octetseq_t* seq);
bool validate_partition_qospolicy (_In_ const nn_partition_qospolicy_t * partition);
bool validate_reliability_qospolicy (const nn_reliability_qospolicy_t * reliability);
bool validate_stringseq (const nn_stringseq_t* seq);

bool dds_qos_validate_common (const dds_qos_t *qos);
dds_return_t dds_qos_validate_mutable_common (_In_ const dds_qos_t *qos);

#if defined (__cplusplus)
}
#endif
#endif