/* module_scaler46.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_SCALER46_H__
#define __MODULE_SCALER46_H__

/* std headers */
#include "pthread.h"

/* mctl headers */
#include "chromatix.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "isp_defs.h"
#include "module_scaler46.h"
#include "scaler_reg.h"
#include "isp_sub_module_zoom.h"

/* Scaler specific macros */
#define SCALE_RATIO_LIMIT 105

/** isp_scaler46_entry_t
 *
 *  @reg_cmd : current state of module
 *  @scaling_factor : session id that is currently using this
 *  @hw_update_pending :
 *  @is_right_stripe_config :
 *  @is_used :
 **/
typedef struct {
  ISP_ScaleCfgCmdType            reg_cmd;
  float                          scaling_factor;
  uint8_t                        hw_update_pending;
  uint8_t                        is_used;
} isp_scaler46_entry_t;

/** scaler46_t
 *
 *  @state: current state of module
 *  @session_id: session id that is currently using this module
 *  @streaming_mode_mask: streaming mode mask
 *  @cur_streaming_mode: Continuous or Burst
 *  @old_streaming_mode: Continuous or Burst
 *  @mutex: Mutex lock for scaler46 private data structure
 *  @entry_idx: ENCODER or VIEWFINDER path index
 *  @is_bayer_sensor: sensor type information
 *  @crop_factor: crop factor
 *  @chromatix_ptrs: chromatix pointer
 *  @isp_out_info: isp output info
 *  @l_stream_info: list of info of streams related
 *  @sensor_out_info: sensor streaming output info
 *  @hw_stream_info: hw stream info for ENCODER/VIEWFINDEr
 *  @scalers: scaler path specific data structre
 *  @applied_crop_factor:  Q12 = 1x
 *  @hw_update_pending: flag to indicate hw update to perform or not
 *  @trigger_enable: enable/disable trigger updates
 *  @skip_trigger: flag to indicate skip this trigger
 *  @enable: scaler46 module enable/disable flag
 *  @zoom: handle to isp_zoom_t
 **/
typedef struct {
  isp_state_t                    state;
  uint32_t                       session_id;
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;
  pthread_mutex_t                mutex;

  uint8_t                        entry_idx;
  boolean                        is_bayer_sensor;
  uint32_t                       crop_factor_applied;
  uint32_t                       crop_factor_calculate_crop;
  modulesChromatix_t             chromatix_ptrs;
  ispif_out_info_t               ispif_out_info;
  isp_out_info_t                 isp_out_info;
  mct_list_t                    *l_stream_info;
  sensor_out_info_t              sensor_out_info;
  isp_hw_stream_info_t           hw_stream_info;
  isp_scaler46_entry_t           scalers;
  cam_crop_region_t              modified_crop_window;
  cam_crop_region_t              applied_crop_window;
  uint32_t                       applied_crop_factor;
  uint32_t                       dis_enable;
  boolean                        is_cds_update;
  boolean                        scaling_factor_update;
  uint32_t                       stream_cnt;

  /* Module Control */
  uint8_t                        hw_update_pending;
  uint8_t                        trigger_enable;
  uint8_t                        skip_trigger;
  uint8_t                        enable;
  uint32_t                       hw_module_id;
  uint32_t                       reg_off;
  uint32_t                       reg_len;
  isp_zoom_t                     zoom;
  cam_zoom_info_t                zoom_value;
  float                          fovc_zoom_val;
  boolean                        update_enable_bit;
  uint32_t                       session_based_identity;
  uint32_t                       identity[ISP_MAX_STREAMS];
  uint32_t                       stream_is_type[ISP_MAX_STREAMS];
  cam_stream_type_t              stream_type;
  isp_dim_t                      camif_dim;
  boolean                        cds_cap[ISP_MAX_STREAMS];
  isp_scaler_hw_limit_desc_t     hw_limits;
  float                          max_scale_factor;
  boolean                        request_crop;
  boolean                        is_zzhdr_mode;
  int32_t                        orig_width;
  int32_t                        orig_height;
  boolean                        is_fast_aec_mode;
  uint32_t                       rtb_mode;
} scaler46_t;

boolean module_scaler46_pass_scale_factor(mct_module_t *module,
  scaler46_t *scaler, boolean is_stream_on, uint32_t identity);

#endif
