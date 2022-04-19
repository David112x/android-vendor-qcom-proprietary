/* module_cac47.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_CAC47_H__
#define __MODULE_CAC47_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_cac47.h"
#include "cac47_reg.h"
#include "chromatix.h"
#include "isp_sub_module_common.h"

/** cac47_t:
 *
 *  @stream_on_count: stream on count
 *  @streaming_mode_mask: streaming mode mask
 *  @cur_streaming_mode: current streaming mode
 *  @old_streaming_mode: old streaming mode
 *  @chromatix_ptrs: chromatix ptrs
 *  @stats_update: stats update
 *  @reg_cmd: reg cmd
 *  @applied_RegCmd: applied reg cmd
 *  @trigger_enable: trigger enable
 **/
typedef struct {
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;

  aec_update_t                   aec_update;
  uint32_t                       trigger_index;

  /*cac data*/
  uint32_t                       y_saturation_thr;
  uint32_t                       y_spot_thr;
  uint32_t                       c_saturation_thr;
  uint32_t                       c_spot_thr;
  uint32_t                       res_mode;
  trigger_ratio_t                aec_ratio;
  scaler_scale_factor_for_cac    cac_scaler_scale_factor[ISP_HW_STREAM_MAX];
  boolean                        cac_scaler_scale_factor_updated;

  boolean                        hyster_en;
  boolean                        scaling_en;

  /* Module Params*/
  ISP_CacCfgCmdType              RegCmd;
  ISP_CacCfgCmdType              applied_RegCmd;

} cac47_t;
#endif
