/* color_correct46.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_COLOR_CORRECT46_H__
#define __MODULE_COLOR_CORRECT46_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_color_correct46.h"
#include "color_correct_reg.h"
#include "chromatix.h"


#define CC_COEFF(x, q) (FLOAT_TO_Q((q), (x)))

typedef struct {
  float  c0;
  float  c1;
  float  c2;

  /* G */
  float  c3;
  float  c4;
  float  c5;

  /* B */
  float  c6;
  float  c7;
  float  c8;

  float  k0;
  float  k1;
  float  k2;
  unsigned char  q_factor;
} color_correct46_param_t;

/** color_correct46_t:
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
  int                            awb_ccm_update_flag;
  int                            awb_ccm_flag;
  awb_ccm_update_t               awb_ccm_update;
  cam_scene_mode_type            bestshot_mode;
  cam_flash_mode_t               cur_flash_mode;
  camera_flash_type              flash_type;

  float                          effects_matrix[3][3];
  float                          dig_gain;
  color_correct46_param_t          final_table;
  color_correct46_param_t          h_table;
  color_correct46_param_t          a_table;
  color_correct46_param_t          d50_table;
  color_correct46_param_t          d65_table;
  color_correct46_param_t          tl84_table;
  color_correct46_param_t          outdoor_table;
  color_correct46_param_t          lowlight_table;
  color_correct46_param_t          led_table;
  color_correct46_param_t          strobe_table;

  /* Module Params*/
  ISP_ColorCorrectionCfgCmdType  RegCmd;
  ISP_ColorCorrectionCfgCmdType  applied_RegCmd;

  /* Module Control */
  uint8_t                        trigger_enable;
  uint8_t                        skip_trigger;

  /* ISP Related*/
  int fd;
  cam_flash_mode_t  cam_flash_mode;
} color_correct46_t;

#endif
