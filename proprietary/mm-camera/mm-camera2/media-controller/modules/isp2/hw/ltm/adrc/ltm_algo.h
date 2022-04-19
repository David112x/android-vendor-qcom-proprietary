/* ltm_algo.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __LTM_ALGO_H__
#define __LTM_ALGO_H__

#include "modules.h"
#include "isp_sub_module_common.h"
#include "ltm_reg.h"
#include "isp_adrc_tune_def.h"

#define CURVE_ENTRY_NUM 128
#define MASTER_SCALE_CURVE_UNITY 512
#define MASTER_CURVE_UNITY 4096
#define ADRC_LUT_SUB_SAMPLE CURVE_ENTRY_NUM/LTM_HW_LUT_SIZE


typedef struct {
  isp_adrc_knee_gain_lut    adrc_gain_lut_table;
  GAMMA_LUT_TYPE            *gamma_table_g;
  double                    adrc_master_curve[CURVE_ENTRY_NUM + 1];
  double                    adrc_master_scale[CURVE_ENTRY_NUM + 1];
  int                       adrc_unpacked_master_scale[LTM_HW_LUT_SIZE + 1];
  int                       adrc_unpacked_master_curve[LTM_HW_LUT_SIZE + 1];
  int                       adrc_unpacked_shift_scale[LTM_HW_LUT_SIZE + 1];
  float                     adrc_ltm_mix_rate;
}ltm_algo_t;


boolean ltm_update_adrc_ltm_curve(ltm_algo_t *adrc_algo, aec_update_t *aec_update,
  mct_module_t *module, mct_event_t *event);
#endif /* __LTM_ALGO_H__ */
