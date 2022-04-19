/* gamma_algo.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GAMMA_ALGO_H__
#define __GAMMA_ALGO_H__

#include "modules.h"
#include "isp_sub_module_common.h"
#include "isp_adrc_tune_def.h"
#include "module_gamma.h"

#define ISP_GAMMA_NUM_CHANNEL           3

typedef struct {
  isp_adrc_knee_gain_lut         adrc_gain_lut_table;
  void                           *gamma_tbl;
  void                           *stats_update;
}gamma_algo_t;

boolean gamma_update_adrc_gamma_curve(gamma_algo_t *algo);
#endif /* __GAMMA_ALGO_H__ */
