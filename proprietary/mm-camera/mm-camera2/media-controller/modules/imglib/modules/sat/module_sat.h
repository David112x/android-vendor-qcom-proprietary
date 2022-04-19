/***************************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_SPATIAL_TRANSFORM_H__
#define __MODULE_SPATIAL_TRANSFORM_H__

#include "module_imgbase.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_SAT_SHIFT


/**img_sat_client_cmd_t
*
* @IMG_SAT_BYPASS: Set the client in bypass mode
* @IMG_SAT_ROLE_SWITCH: Current sesion role switch
* @IMG_SAT_LPM: Is Low Power mode enabled
*
* Data to be propagated to multiple clients in a given session
*/
typedef enum {
  IMG_SAT_BYPASS,
  IMG_SAT_ROLE_SWITCH,
  IMG_SAT_LPM,
} img_sat_client_cmd_t;

/**img_sat_multi_client_data_t
*
* @session_id: Current session id
* @sat_cmd: Commands to the client
* @data: Data associated with the cmd
*
* Data to be propagated to multiple clients in a given session
*/
 typedef struct {
  uint32_t session_id;
  img_sat_client_cmd_t sat_cmd;
  void *data;
 } img_sat_multi_client_data_t;

/** img_sat_session_data_t:
 *
 *   @sensor_max_dim: Max sensor dim
 *   @sensor_crop: Sensor crop settings
 *   @af_tuning_cfg: AF tuning data
 *   @output_size: ouput size
 *   @input_size: input size
 *   @output_size_changed: flag to indicate if output size
 *                       changed
 *   @orignal_output_size: orignal output size
 *   @session_mutex: Mutex to syncronize session data
 *   @process_stream_type: Stream to process
 *   @zoom_level: app specific user zoom level
 *   @user_zoom_level: wide and tele specific zoom level
 *   Session based parameters for SAT module
 */
typedef struct {
  img_dim_t sensor_max_dim;
  sensor_request_crop_t sensor_crop;
  mct_imglib_af_focus_config_t af_tuning_cfg;
  sensor_isp_stream_sizes_t output_size;
  sensor_isp_stream_sizes_t input_size;
  bool output_size_changed[MAX_NUM_STREAMS];
  sensor_isp_stream_sizes_t orignal_output_size;
  pthread_mutex_t session_mutex;
  cam_stream_type_t process_stream_type;
  int32_t zoom_level;
  int32_t user_zoom_level;
} img_sat_session_data_t;

/** img_sat_cfg_t:
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
} img_sat_cfg_t;


/** img_sat_result_t:
 *
 *   @chosen_frame: Camera role chosen as output
 *   @frameid: Frameid of the result
 *   @is_master : Book keeping to know which session was master
 *              when the frame was processed
 *
 *   SAT client private structure
 */
typedef struct {
  img_camera_role_t chosen_frame;
  uint32_t frameid;
  bool is_master;
} img_sat_result_t;

/** img_sat_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @p_client: pointer to IMGLIB base client
 *   @sat_cfg: SAt cfg
 *   @sat_result: SAT result
 *
 *   SAT client private structure
 */
typedef struct {
  img_sat_session_data_t *p_session_data;
  imgbase_client_t *p_client;
  img_sat_cfg_t sat_cfg;
  img_sat_result_t sat_result;
} img_sat_client_t;


/** img_sat_module_t:
 *
 *   @session_data: Session data
 *   @calib_data: sensor calibration data
 *
 *   SAT module private structure
 */
typedef struct {
  img_sat_session_data_t session_data[MAX_IMGLIB_SESSIONS];
  img_opaque_data_set_t sensor_calib_data;
} img_sat_module_t;



#endif
