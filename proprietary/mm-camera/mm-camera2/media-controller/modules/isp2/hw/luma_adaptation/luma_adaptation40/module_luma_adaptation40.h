/* luma_adaptation40.h
 *
 * Copyright (c) 2013, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MODULE_LUMA_ADAPTATION40_H__
#define __MODULE_LUMA_ADAPTATION40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_luma_adaptation40.h"
#include "luma_adaptation40_reg.h"
#include "chromatix.h"
#include "isp_adrc_tune_def.h"

#define ISP_LA_TABLE_LENGTH           64
#define ISP_LA_CURVE_TABLE_LENGTH     256

typedef struct ISP_LA_TblEntry {
  /* Luma adaptation table entries. */
  uint32_t table[ISP_LA_TABLE_LENGTH];
} ISP_LA_TblEntry;

typedef struct ISP_LA_ConfigCmdType {
  /* LA Config */
  ISP_LABankSelCfg CfgCmd;
  ISP_LA_TblEntry  TblEntry;
} ISP_LA_ConfigCmdType;

/** luma_adaptation40_t:
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
 *  @la_8k_algo_parm: used for histogram calculation
 *  @applied_RegCmd: applied reg cmd
 *  @ratio: trigger ratio
 *  @hw_udpate_pending: hw update pending flag
 *  @trigger_enable: trigger enable
 *  @enable: enable
 **/
typedef struct {
  uint32_t isp_version;
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;

  float                          trigger_ratio;
  uint32_t                       backlight_severity;
  cam_scene_mode_type            bestshot_mode;
  cam_effect_mode_type           effect_mode;

  uint32_t                       applied_LUT_YRatio[ISP_LA_TABLE_LENGTH];
  uint32_t                       solarize_la_tbl[ISP_LA_TABLE_LENGTH];
  uint32_t                       posterize_la_tbl[ISP_LA_TABLE_LENGTH];

  isp_la_8k_type                 la_8k_algo_parm;
  uint8_t                        la_curve[ISP_LA_CURVE_TABLE_LENGTH];
  /* TRUE(1) initialized; FALSE(0) not initialized */
  uint8_t                        la_curve_is_valid;

  /* Module Params*/
  ISP_LA_ConfigCmdType           la_cmd;
  ISP_LA_ConfigCmdType           applied_la_cmd;
  boolean                        set_bestshot;
  boolean                        set_effect;
  stats_update_t                 stats_update;
  float                          prev_total_drc_gain;
  float                          prev_la_ratio;
  isp_adrc_knee_gain_lut         adrc_gain_lut_table;
  uint8_t                       *gamma_table_g;
  boolean                        enable_adrc;
} luma_adaptation40_t;
#endif
