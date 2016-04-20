/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                   $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef NN_LAT_ESTIM_H
#define NN_LAT_ESTIM_H

#include "os_defs.h"

#include "q_log.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define NN_LAT_ESTIM_MEDIAN_WINSZ 7

struct nn_lat_estim {
  /* median filtering with a small window in an attempt to remove the
     worst outliers */
  int index;
  float window[NN_LAT_ESTIM_MEDIAN_WINSZ];
  /* simple alpha filtering for smoothing */
  float smoothed;
};

void nn_lat_estim_init (struct nn_lat_estim *le);
void nn_lat_estim_fini (struct nn_lat_estim *le);
void nn_lat_estim_update (struct nn_lat_estim *le, int64_t est);
double nn_lat_estim_current (const struct nn_lat_estim *le);
int nn_lat_estim_log (logcat_t logcat, const char *tag, const struct nn_lat_estim *le);

#if defined (__cplusplus)
}
#endif

#endif /* NN_LAT_ESTIM_H */
