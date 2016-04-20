#ifndef _DDS_DOMAIN_H_
#define _DDS_DOMAIN_H_

#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern const ut_avlTreedef_t dds_domaintree_def;

extern dds_domain * dds_domain_create (dds_domainid_t id);
extern void dds_domain_free (dds_domain * domain);
extern dds_domain * dds_domain_find_locked (dds_domainid_t id);

#if defined (__cplusplus)
}
#endif
#endif
