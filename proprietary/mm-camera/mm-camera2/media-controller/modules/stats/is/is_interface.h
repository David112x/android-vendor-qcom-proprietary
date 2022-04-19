/* is_interface.h
 *
 * Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IS_INTERFACE_H__
#define __IS_INTERFACE_H__


#include "modules.h"


/** is_set_parameter_type:
 * List of IS parameters that can be set by other components.
 **/
typedef enum {
  IS_SET_PARAM_STREAM_CONFIG = 1,
  IS_SET_PARAM_DIS_CONFIG,
  IS_SET_PARAM_OUTPUT_DIM,
  IS_SET_PARAM_IS_ENABLE,
  IS_SET_PARAM_CHROMATIX,
  IS_SET_PARAM_IS_FLUSH,
  IS_SET_PARAM_MULTI_CAM_SYNC_TYPE,   /* In dual cam, if algo doing sync. It set the role of the algo */
  IS_SET_PARAM_LOW_POWER_MODE,        /* In dual cam, algo is request to work in low-power mode */
  IS_SET_PARAM_ROLE_SWITCH,            /* Indicates the role of the algorithm*/
  IS_SET_PARAM_MAX
} is_set_parameter_type;


/** is_sensor_info:
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 **/
typedef struct _is_sensor_info {
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
} is_sensor_info_t;


/** is_dis_config_info:
 *    @stream_type: stream type (preview, video)
 *    @width: stream width
 *    @heigh: stream height
 **/
typedef struct _is_dis_config_info {
  cam_stream_type_t stream_type;
  uint32_t width;
  uint32_t height;
} is_dis_config_info_t;


/** is_output_dim_info:
 *    @is_type: Selected IS technology (DIS = 1, Gyro-assisted DIS = 2, etc.)
 *    @vfe_width: VFE width (image width + margin)
 *    @vfe_height: VFE height (image height + margin)
 **/
typedef struct _is_output_dim_info {
  cam_is_type_t is_type;
  int32_t vfe_width;
  int32_t vfe_height;
} is_output_dim_info_t;


/** is_chromatix_info:
 *    @focal_length: focal length in pixel units for 1080p.  This value is
 *       ccaled for other resolutions.
 *    @gyro_pixel_scale: scale factor to convert degree of rotation (yaw and
 *       pitch) to pixels
 *    @gyro_noise_floor: gyro noise floor
 *    @gyro_frequency: gyro sample frequency
 *    @virtual_margin: extra margin that allows the IS algorithm to operate on
 *       a magin that is larger than the actual physical margin provided by the
 *       sensor
 *    @rs_offset_1: offset to adjust the rolling shutter gyro interval start/
 *        end times to better align with the frame
 *    @rs_offset_2: offset to adjust the rolling shutter gyro interval start/
 *        end times to better align with the frame
 *    @rs_offset_3: offset to adjust the rolling shutter gyro interval start/
 *        end times to better align with the frame
 *    @rs_offset_4: offset to adjust the rolling shutter gyro interval start/
 *        end times to better align with the frame
 *    @s3d_offset_1: offset to adjust the 3D shake gyro interval start/end
 *        times to better align with the frame
 *    @s3d_offset_2: offset to adjust the 3D shake gyro interval start/end
 *        times to better align with the frame
 *    @s3d_offset_3: offset to adjust the 3D shake gyro interval start/end
 *        times to better align with the frame
 *    @s3d_offset_4: offset to adjust the 3D shake gyro interval start/end
 *        times to better align with the frame
 *    @rs_threshold_1: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       rolling shutter gyro time interval
 *    @rs_threshold_2: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       rolling shutter gyro time interval
 *    @rs_threshold_3: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       rolling shutter gyro time interval
 *    @sd3_threshold_1: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       3D shake gyro time interval
 *    @sd3_threshold_2: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       3D shake gyro time interval
 *    @sd3_threshold_3: threshold whereby exposure times above and below
 *       this threshold results in a different offset getting applied to the
 *       3D shake gyro time interval
 *    @rs_time_interval_1: Width of the rolling shutter gyro time interval
 *    @rs_time_interval_2: Width of the rolling shutter gyro time interval
 *    @rs_time_interval_3: Width of the rolling shutter gyro time interval
 *    @rs_time_interval_4: Width of the rolling shutter gyro time interval
 *    @reserve_1: reserved parameter
 *    @reserve_2: reserved parameter
 *    @reserve_3: reserved parameter
 *    @reserve_4: reserved parameter
 **/
typedef struct _is_chromatix_info {
  int32_t focal_length;
  int32_t gyro_pixel_scale;
  float gyro_noise_floor;
  uint32_t gyro_frequency;
  float virtual_margin;
  int32_t rs_offset_1;
  int32_t rs_offset_2;
  int32_t rs_offset_3;
  int32_t rs_offset_4;
  int32_t s3d_offset_1;
  int32_t s3d_offset_2;
  int32_t s3d_offset_3;
  int32_t s3d_offset_4;
  float rs_threshold_1;
  float rs_threshold_2;
  float rs_threshold_3;
  float s3d_threshold_1;
  float s3d_threshold_2;
  float s3d_threshold_3;
  uint32_t rs_time_interval_1;
  uint32_t rs_time_interval_2;
  uint32_t rs_time_interval_3;
  uint32_t rs_time_interval_4;
  uint64_t reserve_1;
  uint64_t reserve_2;
  uint64_t reserve_3;
  uint64_t reserve_4;
} is_chromatix_info_t;

/** is_set_parameter_init_t:
 *    @chromatix: the chromatix pointer which has the data to
 *              initialze the IS
 */
typedef struct _is_set_parameter_init {
  void *chromatix;
  mct_port_t *port;
  uint32_t event_id;
} is_set_parameter_init_t;

/**
 * For algorithms handling power savings control.
 */
typedef struct {
  uint8_t enable;
  cam_dual_camera_perf_mode_t perf_mode;
  uint8_t priority;
} is_low_power_mode_t;

/** _is_set_parameter:
 *    @type: parameter type
 *
 * Used for setting IS parameters
 **/
typedef struct _is_set_parameter {
  is_set_parameter_type type;

  union {
    is_sensor_info_t        is_sensor_info;
    is_dis_config_info_t    is_config_info;
    is_output_dim_info_t    is_output_dim_info;
    is_set_parameter_init_t is_init_param;
    int32_t                 is_enable;
    int32_t                 is_flush;
//  q3a_sync_mode_t         multi_cam_sync_type;
    is_low_power_mode_t     is_low_power_mode;
    cam_sync_mode_t         is_role_switch_mode;
  } u;
} is_set_parameter_t;


/** rs_cs_config_t
 *    @num_row_sum: number of row sums
 *    @num_col_sum: number of column sums
 */
typedef struct {
  uint32_t num_row_sum;
  uint32_t num_col_sum;
} rs_cs_config_t;

/** frame_cfg_t
 *    @dis_frame_width: frame width
 *    @dis_frame_height: frame height
 *    @vfe_output_width: frame width + margin
 *    @vfe_output_height: frame height + margin
 *    @frame_fps: frame rate
 *    @num_mesh_y: number of horizontal meshes
 */
typedef struct {
  int32_t dis_frame_width;
  int32_t dis_frame_height;
  int32_t vfe_output_width;
  int32_t vfe_output_height;
  uint16_t frame_fps;
  uint16_t num_mesh_y;
} frame_cfg_t;

typedef struct {
  frame_cfg_t frame_cfg;
  rs_cs_config_t rs_cs_config;
  cam_is_type_t is_type;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
  is_chromatix_info_t is_chromatix_info;
  uint32_t dis_bias_correction;
  uint32_t buffer_delay;
  void *sns_handle;
} is_init_data_t;

typedef struct {
  uint32_t frame_id;
  boolean has_output;
  int x;
  int y;
  int prev_dis_output_x;
  int prev_dis_output_y;
  int num_matrices;
  float transform_matrix[(MAX_IS_MESH_Y + 1) * NUM_MATRIX_ELEMENTS];
} is_output_type;

#endif /* _IS_INTERFACE_H_ */
