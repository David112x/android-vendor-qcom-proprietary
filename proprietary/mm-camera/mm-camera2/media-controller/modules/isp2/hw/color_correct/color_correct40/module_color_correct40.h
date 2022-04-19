/* color_correct40.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_COLOR_CORRECT40_H__
#define __MODULE_COLOR_CORRECT40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_color_correct40.h"
#include "color_correct40_reg.h"
#include "chromatix.h"


#define CC_COEFF(x, q) (FLOAT_TO_Q((q), (x)))

/** color_correct40_t:
 *
 *  @stream_on_count: stream on count
 *  @streaming_mode_mask: streaming mode mask
 *  @cur_streaming_mode: current streaming mode
 *  @old_streaming_mode: old streaming mode
 *  @mutex: mutex
 *  @chromatix_ptrs: chromatix ptrs
 *  @l_stream_info: stream info list
 *  @stats_update: stats update
 *  @reg_cmd: reg cmd
 *  @applied_RegCmd: applied reg cmd
 *  @ratio: trigger ratio
 *  @hw_udpate_pending: hw update pending flag
 *  @trigger_enable: trigger enable
 *  @enable: enable
 **/
typedef struct {
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;

  stats_update_t                 stats_update;
  trigger_ratio_t                aec_ratio;
  uint32_t                       color_temp;
  int                            awb_ccm_flag;
  int                            awb_ccm_update_flag;
  awb_ccm_update_t               awb_ccm_update;
  cam_scene_mode_type            bestshot_mode;
  cam_flash_mode_t               cur_flash_mode;
  camera_flash_type              flash_type;

  float                          effects_matrix[3][3];
  float                          dig_gain;
  chromatix_color_correction_type            final_table;


  /* Module Params*/
  ISP_ColorCorrectionCfgCmdType  RegCmd;
  ISP_ColorCorrectionCfgCmdType  applied_RegCmd;

  /* Module Control */
  uint8_t                        trigger_enable;
  uint8_t                        skip_trigger;

  /* ISP Related*/
  int fd;
} color_correct40_t;

#endif
