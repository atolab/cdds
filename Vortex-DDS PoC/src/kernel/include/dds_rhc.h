#ifndef _DDS_RHC_H_
#define _DDS_RHC_H_

#include "os_defs.h"
#include "dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct rhc;
struct nn_xqos;
struct serdata;
struct tkmap_instance;
struct tkmap;
struct nn_rsample_info;
struct proxy_writer_info;

struct rhc * dds_rhc_new (dds_reader * reader, const struct sertopic * topic);
void dds_rhc_free (struct rhc * rhc);
void dds_rhc_fini (struct rhc * rhc);

uint32_t dds_rhc_lock_samples (struct rhc * rhc);
uint32_t dds_rhc_samples (struct rhc * rhc);

DDS_EXPORT bool dds_rhc_store
(
  struct rhc * restrict rhc, const struct nn_rsample_info * restrict sampleinfo,
  struct serdata * restrict sample, struct tkmap_instance * restrict tk
);
void dds_rhc_unregister_wr (struct rhc * restrict rhc, const struct proxy_writer_info * restrict pwr_info);
void dds_rhc_relinquish_ownership (struct rhc * restrict rhc, const uint64_t wr_iid);

int dds_rhc_read 
(
  struct rhc *rhc, bool lock, void ** values, dds_sample_info_t *info_seq,
  uint32_t max_samples, unsigned sample_states,
  unsigned view_states, unsigned instance_states,
  dds_instance_handle_t handle
);
int dds_rhc_read_w_condition
(
  struct rhc *rhc, bool lock, void ** values, dds_sample_info_t *info_seq,
  uint32_t max_samples, const dds_condition *cond, dds_instance_handle_t handle
);
int dds_rhc_take
(
  struct rhc *rhc, bool lock, void ** values, dds_sample_info_t *info_seq,
  uint32_t max_samples, unsigned sample_states,
  unsigned view_states, unsigned instance_states,
  dds_instance_handle_t handle
);
int dds_rhc_take_w_condition
(
  struct rhc *rhc, bool lock, void ** values, dds_sample_info_t *info_seq,
  uint32_t max_samples, const dds_condition *cond, dds_instance_handle_t handle
);

void dds_rhc_set_qos (struct rhc * rhc, const struct nn_xqos * qos);

void dds_rhc_add_readcondition (dds_readcond * cond);
void dds_rhc_remove_readcondition (dds_readcond * cond);

bool dds_rhc_add_waitset (dds_readcond * cond, dds_waitset * waitset, dds_attach_t x);
int dds_rhc_remove_waitset (dds_readcond * cond, dds_waitset * waitset);

int dds_rhc_takecdr
(
  struct rhc *rhc, bool lock, struct serdata **values, dds_sample_info_t *info_seq,
  uint32_t max_samples, unsigned sample_states,
  unsigned view_states, unsigned instance_states,
  dds_instance_handle_t handle
);

#if defined (__cplusplus)
}
#endif
#endif
