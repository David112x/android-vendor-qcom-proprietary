/***************************************************************************
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_SAT_H__
#define __MODULE_SAT_H__

/** g_sat_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_sat_params;

/** g_sat_caps:
 *
 *  Set the capabilities for SAT module
*/
static img_caps_t g_sat_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_slave_output = 1,
  .num_master_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .preload_mode = IMG_PRELOAD_COMMON_STREAM_CFG_MODE,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 8,
  .before_cpp = true,
  .dual_camfeature = 1,
  .will_dim_change = true,
};

/** img_qdc_sat_cfg_t:
 *
 *   @af_status: AF Status
 *   @frame_rate: Frame Rate
 *   @fov_cf : Spatial transform fov cfg data
 *
 *   Session based parameters for SAT module
 */
typedef struct {
  cam_af_state_t af_state;
  float frame_rate;
  img_fov_t fov_cf[IMG_MAX_FOV];
  img_aec_info_t aec_info;
  img_awb_info_t awb_info;
} img_qdc_sat_cfg_t;


/** img_qdc_sat_result_t:
 *
 *   @chosen_frame: Camera role chosen as output
 *   @is_master: in master flag
 *   @frameid: Frameid of the result
 *
 *   SAT client private structure
 */
typedef struct {
  img_camera_role_t chosen_frame;
  bool is_master;
  uint32_t frameid;
} img_qdc_sat_result_t;

#endif
