/* bf_stats47.h
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BF_STATS_47_H__
#define __BF_STATS_47_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "bf_stats_reg.h"
#include "isp_sub_module_common.h"
#include "bf_gamma.h"
#include "bf_down_scaler.h"

/* overlapping regions are not supported */
/* #define OVERLAPPING_REG_SUPPORT */


/** bf_stats47_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @af_config: af config params
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 *  @camif_width: camif width
 *  @camif_height: camif height
 *  @validate_horiz_limit: handle to store max # of regions
 *                       horizontally in a frame
 *  @in_cfg_trigger_update_pending: trigger update pending for
 *                                 input cfg
 *  @roi_trigger_update_pending: trigger update pending for ROI
 *                              DMI
 *  @filter_trigger_update_pending: trigger update pending for
 *                                 filters
 *  @bf_gamma_cfg: handle to bf_gamma_cfg
 **/
typedef struct {
  bf_stats_reg_cfg_t     pcmd;
  uint64_t               packed_roi_dmi_cfg[BF_FW_MAX_ROI_REGIONS+1];
  uint32_t               num_roi;
  ispif_out_info_t       ispif_out_info;
  isp_out_info_t         isp_out_info;
  af_config_t            af_config;
  sensor_out_info_t      sensor_out_info;
  uint32_t               num_left_rgns;
  uint32_t               num_right_rgns;
  uint32_t               sensor_width;
  uint32_t               sensor_height;
  uint32_t              *validate_horiz_limit;
  boolean                in_cfg_trigger_update_pending;
  boolean                roi_trigger_update_pending;
  boolean                filter_trigger_update_pending;
  boolean                gamma_trigger_update_pending;
  boolean                skip_gamma_cfg;
  bf_gamma_cfg_t         bf_gamma_cfg;
  bf_down_scaler_t       bf_down_scaler;
  bf_fw_roi_cfg_t        bf_hw_roi_cfg;
  bf_fw_roi_cfg_t        bf_roi_end_pix_order ;
  uint32_t               start_pix_order[BF_FW_MAX_ROI_REGIONS];
  uint32_t               end_pix_order[BF_FW_MAX_ROI_REGIONS];
  uint32_t               gamma_downscale_factor;
  uint32_t               x_min;
  uint32_t               x_max;
  uint32_t               y_min;
  uint32_t               y_max;
  void                  *ext_data;
  void                  *ext_func_table;
  aec_update_t           aec_update;
  float                  scale_ratio;
  int                    ref_exp_index;
} bf_stats47_t;

typedef struct bf_stats47_ext_func_table_t {
  boolean (*bf_stats47_ext_fill_filter_params)(bf_stats47_t *);
  boolean (*bf_stats47_ext_fill_roi_params)(bf_stats47_t *);
  boolean (*bf_stats47_ext_write_ext_params)(isp_sub_module_t *,
    bf_stats47_t *);
  void    (*bf_stats47_ext_destroy)(bf_stats47_t *);
} bf_stats47_ext_func_table_t;

boolean bf_stats47_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_af_exp_compensate(isp_sub_module_t *isp_sub_module,
      void *data);

boolean bf_stats47_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean bf_stats47_init(isp_sub_module_t *isp_sub_module);

void bf_stats47_destroy(isp_sub_module_t *isp_sub_module);

boolean bf_stats47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data);

boolean bf_stats47_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean bf_stats47_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats47_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats_ext47_init(bf_stats47_t *bf_stats);

int32_t bf_stats47_adjust_signbit(int32_t val, uint8_t num_bits);

#endif /* __BF_STATS_47_H__ */
