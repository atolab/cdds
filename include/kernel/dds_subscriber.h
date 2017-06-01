#ifndef _DDS_SUBSCRIBER_H_
#define _DDS_SUBSCRIBER_H_

#include "dds.h"

#if defined (__cplusplus)
extern "C" {
#endif

dds_return_t dds_subscriber_begin_coherent (dds_entity_t e);
dds_return_t dds_subscriber_end_coherent (dds_entity_t e);

#if defined (__cplusplus)
}
#endif
#endif /* _DDS_SUBSCRIBER_H_ */
