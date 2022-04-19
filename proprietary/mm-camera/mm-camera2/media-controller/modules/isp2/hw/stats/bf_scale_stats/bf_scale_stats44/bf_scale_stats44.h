/* bf_scale_stats44.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_SCALE44_H__
#define __BF_SCALE44_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "bf_scale_stats_reg44.h"
#include "isp_sub_module_common.h"

/** scaled_roi_t:
 *  Scaled ROI from the BF scale stats ROI
 *  @bf_scaled_left:    Scaled ROI left offset
 *  @bf_scaled_top:     Scaled ROI top offset
 *  @bf_scaled_width:   Scaled ROI Width
 *  @bf_scaled_height:  Scaled ROI height
 *   */
typedef struct {
  uint32_t              bf_scaled_left;
  uint32_t              bf_scaled_top;
  uint32_t              bf_scaled_width;
  uint32_t              bf_scaled_height;
}scaled_roi_t;

/** bf_stats44_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @af_config: af config params  It contains both bf & bf scale config
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/
typedef struct {
  ISP_StatsBf_Scale_MixCfgCmdType pcmd;
  ispif_out_info_t                ispif_out_info;
  isp_out_info_t                  isp_out_info;
  af_config_t                     af_config;
  sensor_out_info_t               sensor_out_info;
  uint32_t                        num_left_rgns;
  uint32_t                        num_right_rgns;
  int32_t                         buf_offset;
  cam_dimension_t                 camif;
  scaled_roi_t                    scale_roi;
} bf_scale_stats44_t;

boolean bf_scale_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_scale_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_scale_stats44_streamoff(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_scale_stats44_init(isp_sub_module_t *isp_sub_module);

void bf_scale_stats44_destroy(isp_sub_module_t *isp_sub_module);

boolean bf_scale_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_scale_stats44_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data);

boolean bf_scale_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_scale_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /*__BF_SCALE44_H__*/
