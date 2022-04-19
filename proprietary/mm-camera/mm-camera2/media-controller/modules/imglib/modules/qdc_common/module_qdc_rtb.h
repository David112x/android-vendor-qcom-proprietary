/***************************************************************************
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_RTB_H__
#define __MODULE_RTB_H__

/** g_rtb_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_rtb_params;

/** g_rtb_caps:
 *
 *  Set the capabilities for RTB module
*/
static img_caps_t g_rtb_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_slave_output = 0,
  .num_master_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .preload_mode = IMG_PRELOAD_COMMON_STREAM_CFG_MODE,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 4,
  .before_cpp = true,
  .will_dim_change = true,
  .dual_camfeature = 1,
};

/** img_rtb_cfg_t:
 *
 *   @af_status: AF Status
 *   @frame_rate: Frame Rate
 *   @fov_cf : Spatial transform fov cfg data
 *   @capture_requested: is capture in process
 *
 *   Session based parameters for RTB module
 */
typedef struct {
  cam_af_state_t af_state;
  float frame_rate;
  img_fov_t fov_cf[IMG_MAX_FOV];
  img_aec_info_t aec_info;
  img_awb_info_t awb_info;
  uint8_t capture_requested;
} img_rtb_cfg_t;

/** img_rtb_result_t:
 *
 *   @frameid: Frameid of the result
 *   @is_master: in master flag
 *   @chosen_frame: chosen frame
 *
 *   RTB client private structure
 */
typedef struct {
  uint32_t frameid;
  bool is_master;
  img_camera_role_t chosen_frame;
} img_rtb_result_t;

#endif
