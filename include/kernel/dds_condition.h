#ifndef _DDS_CONDITION_H_
#define _DDS_CONDITION_H_

#include "kernel/dds_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

dds_guardcond * dds_guardcond_alloc (void);
int dds_guardcond_free (void);

struct dds_condition * dds_readcond_alloc
(
  struct rhc * rhc, unsigned sample_states, unsigned view_states,
  unsigned instance_states
);
int dds_readcond_free (struct rhc * rhc, dds_condition * cond);

bool dds_scond_exist (dds_entity_kind_t kind);

void dds_cond_signal_waitsets_locked (dds_condition * cond);
void dds_cond_callback_signal (dds_condition * cond);

bool dds_condition_add_waitset (dds_condition * cond, dds_waitset * ws, dds_attach_t x);
int dds_condition_remove_waitset (dds_condition * cond, dds_waitset * ws);

#if defined (__cplusplus)
}
#endif
#endif
