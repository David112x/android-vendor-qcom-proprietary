/* eis2_interface.c
 *
 * Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "is_interface.h"
#include "eis2_interface.h"
#include "stats_debug.h"
#include <time.h>

#define USEC_PER_SEC     1000000.0
#define MSEC_TO_USEC(x)  ((x) * 1000)
#define Q16              (1 << 16)
#ifndef MAX
#define MAX(a,b)         (((a) > (b)) ? (a) : (b))
#endif


/** eis2_process:
 *    @p_eis: EIS 2.0 context
 *    @frame_times: times associated with current frame (SOF, exposure, etc.)
 *    @is_output: place to put EIS 2.0 algorithm result
 *
 * Returns 0 on success.
 **/
int eis2_process(eis2_context_type *p_eis, frame_times_t *frame_times,
  is_output_type *is_output)
{
  uint8_t i;
  int rc = 0;
  eis2_position_type gyro_data;
  eis_input_type eis_input;
  uint64_t sof, eof;
  struct timespec t_now;
  gyro_data_s eis2_gyro_data[2];
  gyro_interval_tuning_parameters_t *gyro_int_parms;
  int idx;

  if (p_eis == NULL) {
    return -1;
  }

  eis2_gyro_data[0].gyro_data = p_eis->gyro_samples_rs;
  eis2_gyro_data[1].gyro_data = p_eis->gyro_samples_3ds;
  gyro_int_parms = &p_eis->gyro_interval_tuning_params;

  clock_gettime( CLOCK_REALTIME, &t_now );

  IS_LOW("%s, time %llu, frame_id %d", __FUNCTION__,
    (((int64_t)t_now.tv_sec * 1000) + t_now.tv_nsec/1000000),
    is_output->frame_id);

  /* Fill in EIS input structure */
  eis_input.dis_offset.x = is_output->prev_dis_output_x;
  eis_input.dis_offset.y = is_output->prev_dis_output_y;
  sof = frame_times->sof;
  eof = sof + frame_times->frame_time;
  eis_input.ts = ((sof + eof) / 2 -
    frame_times->exposure_time * USEC_PER_SEC / 2) /
    USEC_PER_SEC * Q16;
  eis_input.crop_factor = 4096.0;
  eis_input.exposure = frame_times->exposure_time;

  if (eis_input.exposure < gyro_int_parms->rs_exposure_threshold_1) {
    eis_input.time_interval[0].t_start = (sof + eof) / 2 -
      gyro_int_parms->rs_time_interval_1 / 2 +
      gyro_int_parms->rs_interval_offset_1;
    eis_input.time_interval[0].t_end = (sof + eof) / 2 +
      gyro_int_parms->rs_time_interval_1 / 2 +
      gyro_int_parms->rs_interval_offset_1;
  } else if (eis_input.exposure < gyro_int_parms->rs_exposure_threshold_2) {
    eis_input.time_interval[0].t_start = (sof + eof) / 2 -
      gyro_int_parms->rs_time_interval_2 / 2 +
      gyro_int_parms->rs_interval_offset_2;
    eis_input.time_interval[0].t_end = (sof + eof) / 2 +
      gyro_int_parms->rs_time_interval_2 / 2 +
      gyro_int_parms->rs_interval_offset_2;
  } else if (eis_input.exposure < gyro_int_parms->rs_exposure_threshold_3) {
    eis_input.time_interval[0].t_start = (sof + eof) / 2 -
      gyro_int_parms->rs_time_interval_3 / 2 +
      gyro_int_parms->rs_interval_offset_3;
    eis_input.time_interval[0].t_end = (sof + eof) / 2 +
      gyro_int_parms->rs_time_interval_3 / 2 +
      gyro_int_parms->rs_interval_offset_3;
  } else {
    eis_input.time_interval[0].t_start = (sof + eof) / 2 -
      gyro_int_parms->rs_time_interval_4 / 2 +
      gyro_int_parms->rs_interval_offset_4;
    eis_input.time_interval[0].t_end = (sof + eof) / 2 +
      gyro_int_parms->rs_time_interval_4 / 2 +
      gyro_int_parms->rs_interval_offset_4;
  }

  if (eis_input.exposure < gyro_int_parms->s3d_exposure_threshold_1) {
    eis_input.time_interval[1].t_end = (sof + eof) / 2 +
      gyro_int_parms->s3d_interval_offset_1;
  } else if (eis_input.exposure < gyro_int_parms->s3d_exposure_threshold_2) {
    eis_input.time_interval[1].t_end = (sof + eof) / 2 +
      gyro_int_parms->s3d_interval_offset_2;
  } else if (eis_input.exposure < gyro_int_parms->s3d_exposure_threshold_3) {
    eis_input.time_interval[1].t_end = (sof + eof) / 2 +
      gyro_int_parms->s3d_interval_offset_3;
  } else {
    eis_input.time_interval[1].t_end = (sof + eof) / 2 +
      gyro_int_parms->s3d_interval_offset_4;
  }

  /* In dual camera LPM case, camera may get suspended and then resumed at a much later time.  In such a case, the
   * time delta between current SOF and prev_frame_time can be huge.  Ensure that we don't use gyro data from way past
   * by capping the starting interval time to be in the vicinity of the current SOF. */
  if (sof > p_eis->prev_frame_time + 4 * frame_times->frame_time) {
    IS_HIGH("Detect %llu ms frame arrival gap, reset prev_frame_time to 0", (sof - p_eis->prev_frame_time) / 1000);
    p_eis->prev_frame_time = 0;
  }

  eis_input.time_interval[1].t_start = p_eis->prev_frame_time ?
    p_eis->prev_frame_time : eis_input.time_interval[1].t_end - 15000;

  p_eis->prev_frame_time = eis_input.time_interval[1].t_end;

  IS_LOW("frame_id = %d, sof = %.6f, frame time = %.6f, exposure time, %f, "
    "t0 = %.6f, t1 = %.6f, t2 = %.6f, t3 = %.6f", is_output->frame_id,
    (double)sof/USEC_PER_SEC, (double)frame_times->frame_time,
    frame_times->exposure_time,
    eis_input.time_interval[0].t_start / USEC_PER_SEC,
    eis_input.time_interval[0].t_end / USEC_PER_SEC,
    eis_input.time_interval[1].t_start / USEC_PER_SEC,
    eis_input.time_interval[1].t_end / USEC_PER_SEC);

  eis2_gyro_data[0].num_elements = 0;
  eis2_gyro_data[1].num_elements = 0;
  get_gyro_samples(p_eis->sns_handle, &eis_input.time_interval[0],
    &eis2_gyro_data[0]);
  get_gyro_samples(p_eis->sns_handle, &eis_input.time_interval[1],
    &eis2_gyro_data[1]);

  eis_input.gyro_samples_rs = &eis2_gyro_data[0];
  eis_input.gyro_samples_3dr = &eis2_gyro_data[1];

  /* Adjust time interval for rolling shutter if samples are not complete */
  if ((eis2_gyro_data[0].num_elements != 0) &&
      (eis_input.time_interval[0].t_end >
       eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts)) {
    IS_LOW("Shorten interval, change t_end from %llu to %llu",
      eis_input.time_interval[0].t_end,
      eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts);
    eis_input.time_interval[0].t_end =
      eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts;
  }

  eis_align_gyro_to_camera(eis2_gyro_data[0], p_eis->sensor_mount_angle,
    p_eis->camera_position);
  eis_align_gyro_to_camera(eis2_gyro_data[1], p_eis->sensor_mount_angle,
    p_eis->camera_position);

  IS_LOW("ggs t0-t1: # elements %d", eis2_gyro_data[0].num_elements);
  for (i = 0; i < eis2_gyro_data[0].num_elements; i++) {
    IS_LOW("ggs_rs: (x, y, z, ts) = %d, %d, %d, %llu",
      eis2_gyro_data[0].gyro_data[i].data[0],
      eis2_gyro_data[0].gyro_data[i].data[1],
      eis2_gyro_data[0].gyro_data[i].data[2],
      eis2_gyro_data[0].gyro_data[i].ts);
  }
  IS_LOW("ggs t2-t3: # elements %d", eis2_gyro_data[1].num_elements);
  for (i = 0; i < eis2_gyro_data[1].num_elements; i++) {
    IS_LOW("ggs_3d: (x, y, z, ts) = %d, %d, %d, %llu",
      eis2_gyro_data[1].gyro_data[i].data[0],
      eis2_gyro_data[1].gyro_data[i].data[1],
      eis2_gyro_data[1].gyro_data[i].data[2],
      eis2_gyro_data[1].gyro_data[i].ts);
  }

  /* Stabilize only when both RS and 3D shake gyro intervals are not empty */
  if (eis2_gyro_data[0].num_elements > 0 &&
    eis2_gyro_data[1].num_elements > 0) {
    rc = eis2_stabilize_frame(p_eis, &eis_input, is_output->transform_matrix);
    is_output->has_output = TRUE;
    if (rc != EIS_SUCCESS) {
      is_output->has_output = FALSE;
    }
  } else {
    IS_HIGH("Gyro sample interval empty");
    is_output->has_output = FALSE;
  }

  IS_LOW("tform mat: %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
    is_output->transform_matrix[0], is_output->transform_matrix[1],
    is_output->transform_matrix[2], is_output->transform_matrix[3],
    is_output->transform_matrix[4], is_output->transform_matrix[5],
    is_output->transform_matrix[6], is_output->transform_matrix[7],
    is_output->transform_matrix[8]);

  return rc;
}


/** eis2_initialize:
 *    @eis: EIS2 context
 *    @data: initialization parameters
 *
 * This function initializes the EIS2 algorithm.
 *
 * Returns 0 on success.
 **/
int eis2_initialize(eis2_context_type *eis, is_init_data_t *data)
{
  int i, rc = 0;
  eis2_init_type init_param;
  frame_cfg_t *frame_cfg = &data->frame_cfg;
  rs_cs_config_t *rs_cs_config = &data->rs_cs_config;
  gyro_interval_tuning_parameters_t *gyro_int_parms;

  init_param.sensor_mount_angle = data->sensor_mount_angle;
  init_param.camera_position = data->camera_position;
  init_param.focal_length = data->is_chromatix_info.focal_length;
  init_param.virtual_margin = data->is_chromatix_info.virtual_margin;
  init_param.gyro_noise_floor = data->is_chromatix_info.gyro_noise_floor;
  init_param.gyro_pixel_scale = data->is_chromatix_info.gyro_pixel_scale;
  init_param.dis_bias_correction = data->dis_bias_correction;

  init_param.width = frame_cfg->dis_frame_width;
  init_param.height = frame_cfg->dis_frame_height;

  init_param.margin_x = (frame_cfg->vfe_output_width -
      frame_cfg->dis_frame_width) / 2;
  init_param.margin_y = (frame_cfg->vfe_output_height -
      frame_cfg->dis_frame_height) / 2;

  IS_HIGH("init_param->margin_x = %u", init_param.margin_x);
  IS_HIGH("init_param->margin_y = %u", init_param.margin_y);
  IS_HIGH("virtual margin = %f", init_param.virtual_margin);
  IS_HIGH("gyro noise floor = %f", init_param.gyro_noise_floor);
  IS_HIGH("gyro pixel scale = %d", init_param.gyro_pixel_scale);
  IS_HIGH("DIS bias correction = %d", init_param.dis_bias_correction);

  gyro_int_parms = &eis->gyro_interval_tuning_params;
  gyro_int_parms->rs_interval_offset_1 = data->is_chromatix_info.rs_offset_1;
  gyro_int_parms->rs_interval_offset_2 = data->is_chromatix_info.rs_offset_2;
  gyro_int_parms->rs_interval_offset_3 = data->is_chromatix_info.rs_offset_3;
  gyro_int_parms->rs_interval_offset_4 = data->is_chromatix_info.rs_offset_4;
  gyro_int_parms->s3d_interval_offset_1 = data->is_chromatix_info.s3d_offset_1;
  gyro_int_parms->s3d_interval_offset_2 = data->is_chromatix_info.s3d_offset_2;
  gyro_int_parms->s3d_interval_offset_3 = data->is_chromatix_info.s3d_offset_3;
  gyro_int_parms->s3d_interval_offset_4 = data->is_chromatix_info.s3d_offset_4;
  gyro_int_parms->rs_exposure_threshold_1 =
    data->is_chromatix_info.rs_threshold_1;
  gyro_int_parms->rs_exposure_threshold_2 =
    data->is_chromatix_info.rs_threshold_2;
  gyro_int_parms->rs_exposure_threshold_3 =
    data->is_chromatix_info.rs_threshold_3;
  gyro_int_parms->s3d_exposure_threshold_1 =
    data->is_chromatix_info.s3d_threshold_1;
  gyro_int_parms->s3d_exposure_threshold_2 =
    data->is_chromatix_info.s3d_threshold_2;
  gyro_int_parms->s3d_exposure_threshold_3 =
    data->is_chromatix_info.s3d_threshold_3;
  gyro_int_parms->rs_time_interval_1 =
    MSEC_TO_USEC(data->is_chromatix_info.rs_time_interval_1);
  gyro_int_parms->rs_time_interval_2 =
    MSEC_TO_USEC(data->is_chromatix_info.rs_time_interval_2);
  gyro_int_parms->rs_time_interval_3 =
    MSEC_TO_USEC(data->is_chromatix_info.rs_time_interval_3);
  gyro_int_parms->rs_time_interval_4 =
    MSEC_TO_USEC(data->is_chromatix_info.rs_time_interval_4);

  IS_HIGH("gyro rs interval offsets: %lld, %lld, %lld, %lld",
    gyro_int_parms->rs_interval_offset_1, gyro_int_parms->rs_interval_offset_2,
    gyro_int_parms->rs_interval_offset_3, gyro_int_parms->rs_interval_offset_4);
  IS_HIGH("gyro s3d interval offsets: %lld, %lld, %lld, %lld",
    gyro_int_parms->s3d_interval_offset_1, gyro_int_parms->s3d_interval_offset_2,
    gyro_int_parms->s3d_interval_offset_3, gyro_int_parms->s3d_interval_offset_4);
  IS_HIGH("gyro rs exposure thresholds: %f, %f, %f",
    gyro_int_parms->rs_exposure_threshold_1,
    gyro_int_parms->rs_exposure_threshold_2,
    gyro_int_parms->rs_exposure_threshold_3);
  IS_HIGH("gyro s3d exposure thresholds: %f, %f, %f",
    gyro_int_parms->s3d_exposure_threshold_1,
    gyro_int_parms->s3d_exposure_threshold_2,
    gyro_int_parms->s3d_exposure_threshold_3);
  IS_HIGH("rs time intervals: %lld, %lld, %lld, %lld",
    gyro_int_parms->rs_time_interval_1, gyro_int_parms->rs_time_interval_2,
    gyro_int_parms->rs_time_interval_3, gyro_int_parms->rs_time_interval_4);

  eis->sns_handle = data->sns_handle;
  if (eis2_init(eis, &init_param) > 0) {
    IS_ERR("eis2_init failed");
    rc = -1;
  }
  return rc;
}


/** eis2_deinitialize:
 *    @eis: EIS context
 *
 * This function deinits the EIS algorithm.
 *
 * Returns 0 on success.
 **/
int eis2_deinitialize(eis2_context_type *eis)
{
  int rc;

  rc = eis2_exit(eis);
  return rc;
}
