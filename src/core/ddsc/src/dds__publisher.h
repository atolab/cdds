#ifndef _DDS_PUBLISHER_H_
#define _DDS_PUBLISHER_H_

#include "ddsc/dds.h"

#if defined (__cplusplus)
extern "C" {
#endif

dds_return_t dds_publisher_begin_coherent (dds_entity_t e);
dds_return_t dds_publisher_end_coherent (dds_entity_t e);

#if defined (__cplusplus)
}
#endif
#endif /* _DDS_PUBLISHER_H_ */
