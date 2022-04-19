/* abf44.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC_ABF44_H__
#define __DEMOSAIC_ABF44_H__

#include "chromatix.h"

#include "abf_reg.h"
#include "isp_sub_module_common.h"
#include "../common/abf_common.h"


typedef uint32_t signal2_lut_t[32];
typedef uint32_t noise_std2_lut_t[128];

typedef struct {
  signal2_lut_t signal2_lut_lv0;
  signal2_lut_t signal2_lut_lv1;
  noise_std2_lut_t noise_std2_lut_lv0;
  noise_std2_lut_t noise_std2_lut_lv1;
} abf3_parms_t;

typedef struct {
  /* Module Params*/
  ISP_ABF3_CmdType         v3RegCmd;
  trigger_ratio_t aec_ratio;
  uint8_t trigger_index;
  abf3_parms_t abf3_parms;
  float real_gain;
} abf44_t;

boolean abf44_init(isp_sub_module_t *isp_sub_module);

void abf44_destroy(isp_sub_module_t *isp_sub_module);

boolean abf44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf44_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf44_streamoff(isp_sub_module_t *isp_sub_module,
  void *data);
#endif //__DEMOSAIC_ABF44_H__
