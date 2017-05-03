#ifndef _DDS_TYPES_H_
#define _DDS_TYPES_H_

/* Note: this file contains bare necessities to build dds_listener.c and should asap be replaced by a
 * modified, original dds_types.h
 */

/* This should, eventually, get included implicitely via i.e. os.h */
#include "stdlib.h"
#include "os_decl_attributes_sal.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Types stubs */
typedef int* dds_entity_t;
typedef int dds_inconsistent_topic_status_t;
typedef int dds_liveliness_lost_status_t;
typedef int dds_offered_deadline_missed_status_t;
typedef int dds_offered_incompatible_qos_status_t;
typedef int dds_sample_lost_status_t;
typedef int dds_sample_rejected_status_t;
typedef int dds_liveliness_changed_status_t;
typedef int dds_requested_deadline_missed_status_t;
typedef int dds_requested_incompatible_qos_status_t;
typedef int dds_publication_matched_status_t;
typedef int dds_subscription_matched_status_t;
/* End types stubs */

/* This is copied from dds.h */
#undef DDS_EXPORT
#ifdef _WIN32_DLL_
  #if defined VL_BUILD_DDS_DLL
    #define DDS_EXPORT extern __declspec (dllexport)
  #else
    #define DDS_EXPORT extern __declspec (dllimport)
  #endif
#else
  #define DDS_EXPORT extern
#endif

#if defined (__cplusplus)
}
#endif
#endif
