#ifndef _DDS_SUBSCRIBER_H_
#define _DDS_SUBSCRIBER_H_

#include "vddsc/dds.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct dds_entity;

_Requires_exclusive_lock_held_(participant)
_Check_return_ dds_entity_t
dds__create_subscriber_l(
        _Inout_  struct dds_entity *participant, /* entity-lock must be held */
        _In_opt_ const dds_qos_t *qos,
        _In_opt_ const dds_listener_t *listener);

dds_return_t
dds_subscriber_begin_coherent(
        _In_ dds_entity_t e);

dds_return_t
dds_subscriber_end_coherent (
        _In_ dds_entity_t e);

#if defined (__cplusplus)
}
#endif
#endif /* _DDS_SUBSCRIBER_H_ */
