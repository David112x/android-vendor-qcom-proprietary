/* bf_down_scaler.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_DOWN_SCALER_H__
#define __BF_DOWN_SCALER_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "bf_down_scaler_reg.h"
#include "isp_sub_module_common.h"

/** bf_stats47_t:
 *
 *  @pcmd: hardware configuration
 *  @af_config: af config params  It contains both bf & bf scale config
 *  @sensor_out_info: sensor out info
 *  @bf_scale_cfg: store current ROI configuration
 *  @trigger_update_pending: trigger update pending flag
 *  @camif_width: camif width
 *  @camif_height: camif height
 **/
typedef struct {
  bf_down_scaler_enable_t      enable_mask;
  bf_down_scaler_enable_t      enable_val;
  bf_down_scaler_reg_cfg_t     pcmd;
  af_config_t                  af_config;
  sensor_out_info_t            sensor_out_info;
  bf_scale_cfg_t               bf_scale_cfg;
  boolean                      trigger_update_pending;
  uint32_t                     camif_width;
  uint32_t                     camif_height;
} bf_down_scaler_t;

boolean bf_down_scaler_trigger_update(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, isp_sub_module_output_t *sub_module_output);

boolean bf_down_scaler_stats_config_update(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, bf_fw_config_t *bf_fw);

boolean bf_down_scaler_set_stream_config(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, sensor_out_info_t *sensor_out_info);

boolean bf_down_scaler_get_downscaler_params(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, bf_scale_cfg_t *bf_out_scale_cfg);

#endif /*__BF_DOWN_SCALER_H__*/
