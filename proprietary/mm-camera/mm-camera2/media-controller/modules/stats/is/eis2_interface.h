/* eis2_interface.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __EIS2_INTERFACE_H__
#define __EIS2_INTERFACE_H__
#include "is_interface.h"
#include "is_sns_lib.h"
#include "is_common_intf.h"


/* EIS error codes */
#define EIS_SUCCESS             0
#define EIS_ERR_BASE            0x00
#define EIS_ERR_FAILED          (EIS_ERR_BASE + 1)
#define EIS_ERR_NO_MEMORY       (EIS_ERR_BASE + 2)
#define EIS_ERR_BAD_HANDLE      (EIS_ERR_BASE + 3)
#define EIS_ERR_BAD_POINTER     (EIS_ERR_BASE + 4)
#define EIS_ERR_BAD_PARAM       (EIS_ERR_BASE + 5)

/* Number of samples for correlation calculation */
#define CORR2_WINDOW_LEN 16


typedef struct
{
  int32_t x;
  int32_t y;
  int32_t z;
} eis2_position_type;


/** _eis2_context_type:
 *    @frame_num: current frame number
 *    @past_dis_offsets: recent history of DIS offsets
 *    @past_eis_offsets: recent history of EIS offsets
 *    @timestamp: timestamps of recent offsets
 *    @rolling_shutter_tform: rolling shutter correction transform
 *    @Qmat:
 *    @prev_Qnet:
 *    @Qnet:
 *    @projective_tform:
 *    @composite_tform:
 *    @bias: gyro sensor bias
 *    @kalman_params_x: Kalman filter parameters for x axis
 *    @kalman_params_y: Kalman filter parameters for y axis
 *    @alpha: pan filter coefficient
 *    @avg_gyro_stdev: filtered mean (over x, y, z) gyro standard deviation
 *    @is_shaking: flag to indicate motion
 *    @crop_factor: zoom
 *    @margin_x:
 *    @margin_y:
 *    @width:
 *    @height:
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 *    @res_scale:
 *    @margin_scale: margin scale
 *    @crop_scale: crop scale
 *    @gyro_noise_floor: gyro noise floor
 *    @gyro_pixel_scale: gyro pixel scale
 *    @gyro_interval_tuning_params: gyro interval tuning parameters
 *    @dis_bias_correction: Indicates whether DIS bias correction is enabled
 *    @sns_handle: sensors handle
 *    @gyro_samples_rs: gyro samples for calculating rolling shutter correction
 *       matrix
 *    @gyro_samples_3ds: gyro samples for calculating 3D shake rotation
 *       correction matrix
 *    @Rmat: for debug logging
 *    @pan: for debug logging
 *    @del_offset: for debug logging (applicable only when DIS bias correction
 *       is enabled)
 *
 * This structure maintains the EIS 2.0 alogirthm context.
 **/
typedef struct
{
  uint32_t frame_num;
  eis2_position_type past_dis_offsets[CORR2_WINDOW_LEN];
  eis2_position_type past_eis_offsets[CORR2_WINDOW_LEN];
  int64_t timestamp[CORR2_WINDOW_LEN];
  mat3x3 rolling_shutter_tform;
  quaternion_type Qmat;
  quaternion_type prev_Qnet;
  quaternion_type Qnet;
  mat3x3 projective_tform;
  mat3x3 composite_tform;
  int bias[3];
  eis_kalman_type kalman_params_x;
  eis_kalman_type kalman_params_y;
  double alpha;
  double avg_gyro_stdev;
  boolean is_shaking;
  int32_t crop_factor;
  double margin_x;
  double margin_y;
  uint32_t width;
  uint32_t height;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
  double res_scale;
  uint64_t prev_frame_time;
  double margin_scale;
  double crop_scale;
  int32_t focal_length;
  double gyro_noise_floor;
  int32_t gyro_pixel_scale;
  uint32_t dis_bias_correction;
  void *sns_handle;
  gyro_interval_tuning_parameters_t gyro_interval_tuning_params;
  gyro_sample_s gyro_samples_rs[100];
  gyro_sample_s gyro_samples_3ds[100];
  mat3x3 Rmat;
  double pan[3];
  double del_offset[2];
} eis2_context_type;


/** _eis2_init_type:
 *    @margin_x: margin in x direction
 *    @margin_y: margin in y direction
 *    @width: IS frame width
 *    @height: IS frame width
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 *    @virtual_margin: extra margin in addition to the physical margin
 *    @gyro_noise_floor: gyro noise floor
 *    @gyro_pixel_scale: gyro pixel scale
 *    @dis_bias_correction: Indicates whether DIS bias correction is enabled
 **/
typedef struct {
  uint16_t margin_x;
  uint16_t margin_y;
  uint16_t width;
  uint16_t height;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
  int32_t focal_length;
  double virtual_margin;
  double gyro_noise_floor;
  int32_t gyro_pixel_scale;
  uint32_t dis_bias_correction;
} eis2_init_type;


/** _eis2_input_type:
 *    @gyro_sample_rs: gyro samples for calculating rolling shutter transform
 *    @gyro_sample_3dr: gyro samples for calculating 3D rotation transform
 *    @time_interval: time intervals for calculating integrated gyro data and
 *      quaternion product
 *    @dis_offset: DIS offset in pixels, integer format
 *    @ts: frame timestamp, Q16 format
 *    @exposure: exposure time
 *    @crop_factor: zoom
 **/
typedef struct {
  gyro_data_s *gyro_samples_rs;
  gyro_data_s *gyro_samples_3dr;
  time_interval_s time_interval[2];
  eis2_position_type dis_offset;
  int64_t ts;
  double exposure;
  int32_t crop_factor;
} eis_input_type;


int eis2_init(eis2_context_type *p_eis_context,
  const eis2_init_type *p_init_param);
int eis2_stabilize_frame (eis2_context_type *p_context, eis_input_type *p_input,
  float *p_matrix);
int eis2_exit (eis2_context_type *p_eis_context);
int eis2_initialize(eis2_context_type *eis, is_init_data_t *data);
int eis2_process(eis2_context_type *eis, frame_times_t *frame_times,
  is_output_type *is_output);
int eis2_deinitialize(eis2_context_type *eis);
int eis_align_gyro_to_camera(gyro_data_s gyro_data, unsigned int camera_mount_angle,
  enum camb_position_t camera_position);
#endif
