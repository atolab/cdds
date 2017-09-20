#ifndef _DDS_SUBSCRIBER_H_
#define _DDS_SUBSCRIBER_H_

#include "dds.h"

#if defined (__cplusplus)
extern "C" {
#endif

dds_return_t dds_subscriber_begin_coherent (_In_ dds_entity_t e);
dds_return_t dds_subscriber_end_coherent (_In_ dds_entity_t e);


_Must_inspect_result_ dds_entity_t
dds__get_builtin_subscriber(
    _In_ dds_entity_t e);

_Pre_satisfies_((subscriber & DDS_ENTITY_KIND_MASK) == DDS_KIND_SUBSCRIBER)
_Must_inspect_result_ dds_entity_t
dds__get_builtin_topic(
    _In_ dds_entity_t subscriber,
    _In_ dds_entity_t topic);

#if defined (__cplusplus)
}
#endif
#endif /* _DDS_SUBSCRIBER_H_ */
