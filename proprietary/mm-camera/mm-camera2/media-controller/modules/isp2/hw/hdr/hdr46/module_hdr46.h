/* module_hdr46.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_HDR46_H__
#define __MODULE_HDR46_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_hdr46.h"
#include "hdr_reg.h"
#include "chromatix.h"


typedef struct {
  ISP_HdrCfgCmdType            hdrCfgCmd;
  ISP_HdrReconCfgCmdType       ReconCfgCmd;
  ISP_HdrMacCfgCmdType         MacCfgCmd;
} hdr_reg_cmd_t;

/** hdr46_t:
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
  float                          wb_rg_ratio;
  float                          wb_bg_ratio;
  float                          wb_gr_ratio;
  float                          wb_gb_ratio;
  float                          exp_ratio;
  float                          analog_gain;

  aec_update_t                   aec_update;
  awb_update_t                   awb_update;

  /* Module Params*/
  hdr_reg_cmd_t                  RegCmd;
  hdr_reg_cmd_t                  applied_RegCmd;

  /* Module Control */
  uint8_t                        trigger_enable;
} hdr46_t;
#endif
