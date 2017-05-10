#ifndef _DDS_LISTENER_H_
#define _DDS_LISTENER_H_

#include "dds/dds_public_impl.h"
#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define dds_listener_get_unl(a, b); /* Deprecated */
#define dds_listener_merge(a, b,c); /* Deprecated */

void dds_listener_delete (_In_ _Post_invalid_ dds_listener_t __restrict listener);
void dds_listener_lock (_In_ _Post_invalid_ dds_listener_t __restrict listener);
void dds_listener_unlock (_In_ _Post_invalid_ dds_listener_t __restrict listener);




#if defined (__cplusplus)
}
#endif
#endif
