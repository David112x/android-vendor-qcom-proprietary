/* bf_gamma.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_GAMMA_H__
#define __BF_GAMMA_H__

/* mctl headers */
#include "mct_event_stats.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "bf_gamma_reg.h"

/** bf_gamma_cfg_t:
 *
 *  @bf_gamma_lut_cfg: handle to bf_gamma_cfg to store 3A config
 *  @trigger_update_pending: flag to indicate whether
 *                         trigger_update_pending
 *  @lut: lut table
 *  @num_lut: num of valid lut entries
 *  @bf_stats_enable_mask: mask to configure bf gamma LUT and en
 *  @bf_stats_enable_val: val to configure bf gamma LUT and en
 **/
typedef struct bf_gamma_cfg_t {
  bf_gamma_lut_cfg_t        bf_gamma_lut_cfg;
  boolean                   trigger_update_pending;
  boolean                   flip_g_channel;
  uint32_t                  lut[MAX_BF_GAMMA_ENTRIES];
  uint32_t                  num_lut;
  bf_gamma_stats_enable_t   bf_stats_enable_mask;
  bf_gamma_stats_enable_t   bf_stats_enable_val;
} bf_gamma_cfg_t;

boolean bf_gamma_trigger_update(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, isp_sub_module_output_t *sub_module_output, uint32_t downscale_factor);

boolean bf_gamma_stats_config_update(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, bf_fw_config_t *bf_fw);

boolean bf_gamma_scale_config(bf_gamma_cfg_t *bf_gamma_cfg,
  bf_fw_config_t *bf_fw_local, float scale_ratio);

boolean bf_gamma_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  mct_stats_info_t *stats_info);

boolean bf_gamma_set_stream_config(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, sensor_out_info_t *sensor_out_info);

#endif /* __BF_GAMMA_H__ */
