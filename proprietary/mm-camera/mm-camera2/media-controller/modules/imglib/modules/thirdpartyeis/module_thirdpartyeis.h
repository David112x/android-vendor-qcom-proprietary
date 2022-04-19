/***************************************************************************
* Copyright (c) 2019 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_THIRDPARTY_EIS_H__
#define __MODULE_THIRDPARTY_EIS_H__

#include "module_imgbase.h"

static long long ts[2] = {0};

/** img_thirdpartyeis_session_data_t:
 *
 *   @sensor_max_dim: sensor max dim
 *   @sensor_crop: sensor crop request
 *   @calib_data: sensor calibration data
 *   @input_size: input sizes
 *   @output_size: output sizes
 *   @output_size_changed: flag for output size changed
 *   @orignal_output_size: orignal output sizes
 *   @thirdpartyeis_cfg: thirdparty eis config
 *   @session_mutex: Mutex to syncronize session data
 *   @process_stream_type: Stream to process
 *   @zoom_level: session zoom level
 *   @p_sensor_cap: pointer to sensor capabilities
 *   @pix_pitch: pixel pitch
 *   @focal_length: focal length
 *   @user_zoom_level: User zoom level
 *   @blur_level_info: Blur intensity level data
 *
 *   Session based parameters for thirdpartyeis modules
 */
typedef struct {
  img_dim_t sensor_max_dim;
  sensor_request_crop_t sensor_crop;
  sensor_isp_stream_sizes_t input_size;
  sensor_isp_stream_sizes_t output_size;
  bool output_size_changed[MAX_NUM_STREAMS];
  sensor_isp_stream_sizes_t orignal_output_size;
  img_thirdparty_eis_cfg_t thirdpartyeis_cfg;
  pthread_mutex_t session_mutex;
  cam_stream_type_t process_stream_type;
  int32_t zoom_level;
  mct_pipeline_sensor_cap_t *p_sensor_cap;
  float pix_pitch;
  float focal_length;
  int32_t user_zoom_level;
  mct_event_gyro_data_t is_gyro_data;
  mct_event_accel_data_t is_accel_data;
} img_thirdpartyeis_session_data_t;

/** img_thirdpartyeis_module_t:
 *
 *   @session_data: Session data
 *
 *   ThirdPartyEIS module private structure
 */
typedef struct {
  img_thirdpartyeis_session_data_t session_data[MAX_IMGLIB_SESSIONS];
} img_thirdpartyeis_module_t;

/** img_thirdpartyeis_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @p_client: pointer to IMGLIB base client
 *   @thirdpartyeis_cfg: eis config
 *
 *   client private structure
 */
typedef struct {
  img_thirdpartyeis_session_data_t *p_session_data;
  imgbase_client_t *p_client;
  img_thirdparty_eis_cfg_t thirdpartyeis_cfg;
} img_thirdpartyeis_client_t;

#endif
