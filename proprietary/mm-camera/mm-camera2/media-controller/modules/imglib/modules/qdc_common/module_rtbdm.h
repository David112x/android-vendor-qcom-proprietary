/***************************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_RTBDM_H__
#define __MODULE_RTBDM_H__

/** g_rtbdm_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_rtbdm_params;

/** g_rtbdm_caps:
 *
 *  Set the capabilities for RTBDM module
*/
static img_caps_t g_rtbdm_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .num_release_buf = 0,
  .preload_mode = IMG_PRELOAD_COMMON_STREAM_CFG_MODE,
  .before_cpp = false,
  .will_dim_change = false,
};

/** img_rtbdm_cfg_t:
 *
 *   @af_state: AF state
 *   @frame_rate: Frame Rate
 *   @fov_cf : Spatial transform fov cfg data
 *   @capture_requested: is capture in process
 *
 *   Session based parameters for RTBDM module
 */
typedef struct {
  cam_af_state_t af_state;
  float frame_rate;
  img_fov_t fov_cf[IMG_MAX_FOV];
  img_aec_info_t aec_info;
  img_awb_info_t awb_info;
  uint8_t capture_requested;
} img_rtbdm_cfg_t;

/** img_sac_result_t:
 *
 *   @frameid: Frameid of the result
 *   @is_master: in master flag
 *   @chosen_frame: chosen frame
 *   @depth_map_ready: depth map ready flag
 *
 *   RTBDM client private structure
 */
typedef struct {
  uint32_t frameid;
  bool is_master;
  img_camera_role_t chosen_frame;
  bool depth_map_ready;
} img_rtbdm_result_t;

#endif
