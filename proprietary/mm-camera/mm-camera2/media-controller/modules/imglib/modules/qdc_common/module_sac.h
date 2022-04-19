/***************************************************************************
* Copyright (c) 2016-2018 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_SAC_H__
#define __MODULE_SAC_H__

/** g_sac_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_sac_params;

/** g_sac_caps:
 *
 *  Set the capabilities for SAC module
*/
static img_caps_t g_sac_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_slave_output = 0,
  .num_master_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .num_release_buf = 0,
  .preload_mode = IMG_PRELOAD_COMMON_STREAM_CFG_MODE,
  .before_cpp = true,
  .dual_camfeature = 1,
  .will_dim_change = false,
};

/** img_sac_cfg_t:
 *
 *   @af_status: AF Status
 *   @frame_rate: Frame Rate
 *   @fov_cf : Spatial transform fov cfg data
 *
 *   Session based parameters for SAC module
 */
typedef struct {
  cam_af_state_t af_state;
  float frame_rate;
  img_fov_t fov_cf[IMG_MAX_FOV];
  img_aec_info_t aec_info;
  img_awb_info_t awb_info;
} img_sac_cfg_t;

/** img_sac_result_t:
 *
 *   @proj_trans_matrix: Includes 3x3 Projective transform + X-Y
 *     Shift to be applied on aux with main as reference
 *   @frameid: Frameid of the result
 *   @is_master: in master flag
 *   @chosen_frame: chosen frame
 *
 *   SAC client private structure
 */
typedef struct {
  float proj_trans_matrix[9];
  uint32_t frameid;
  bool is_master;
  img_camera_role_t chosen_frame;
} img_sac_result_t;

#endif
