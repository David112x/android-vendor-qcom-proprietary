/* chroma_enhan40.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_CHROMA_ENHAN40_H__
#define __MODULE_CHROMA_ENHAN40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_chroma_enhan40.h"
#include "chroma_enhan_reg.h"
#include "chromatix.h"

typedef struct
{
    float a_m;
    float a_p;
    float b_m;
    float b_p;
    float c_m;
    float c_p;
    float d_m;
    float d_p;
    float k_cb;
    float k_cr;
} chroma_enhancement_type_t;

typedef struct
{
    float v0;
    float v1;
    float v2;
    float k;
} luma_calculation_type_t;

/* Color Conversion */
typedef struct
{
    chroma_enhancement_type_t chroma;
    luma_calculation_type_t   luma;
} color_conversion_type_t;

/** chroma_enhan40_t:
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
 *  @mix_reg_cmd: mix reg cmd
 *  @ratio: trigger ratio
 *  @hw_udpate_pending: hw update pending flag
 *  @trigger_enable: trigger enable
 *  @skip_trigger: skip trigger
 *  @enable: enable
 *  @classifier_cfg_done: classifier cfg done
 *  @set_bestshot: boolean flag to store whether bestshot is
 *               enabled
 *  @set_effect: boolean flag to store whether effect is enabled
 **/
typedef struct {
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;

  stats_update_t                 stats_update;
  trigger_ratio_t                aec_ratio;
  uint32_t                       color_temp;
  stats_dual_led_settings_t      dual_led_setting;
  cam_scene_mode_type            bestshot_mode;
  cam_effect_mode_type           effect_mode;
  cam_wb_mode_type               wb_mode;
  cam_flash_mode_t               cur_flash_mode;

  /* Module Params*/
  ISP_Chroma_Enhance_CfgCmdType  RegCmd;
  ISP_Chroma_Enhance_CfgCmdType  applied_RegCmd;

  color_conversion_type_t        cv_data;
  float                          effects_matrix[2][2];

  /* ISP Related*/
  int fd;
  boolean                        set_bestshot;
  boolean                        set_effect;
  int32_t                        saturation;
  float                          prev_color_drc_gain;
  boolean                        enable_adrc;
} chroma_enhan40_t;

#endif
