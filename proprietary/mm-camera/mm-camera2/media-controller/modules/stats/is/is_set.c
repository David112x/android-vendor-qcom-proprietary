/* is_set.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "is.h"
#include "stats_chromatix_wrapper.h"

static boolean is_set_init_chromatix(is_info_t *is_info,
  is_set_parameter_init_t *init_param)
{
  if (is_info == NULL || init_param == NULL
      || init_param->chromatix == NULL) {
    return FALSE;
  }

  chromatix_3a_parms_wrapper* chromatix_wrapper = (chromatix_3a_parms_wrapper*)init_param->chromatix;

  pthread_rwlock_rdlock(&chromatix_wrapper->lock);
  is_info->is_chromatix_info.focal_length = chromatix_wrapper->IS.focal_length;
  is_info->is_chromatix_info.gyro_pixel_scale = chromatix_wrapper->IS.gyro_pixel_scale;
  is_info->is_chromatix_info.virtual_margin = chromatix_wrapper->IS.virtual_margin;
  is_info->is_chromatix_info.gyro_noise_floor = chromatix_wrapper->IS.gyro_noise_floor;
  is_info->is_chromatix_info.gyro_frequency = chromatix_wrapper->IS.gyro_frequency;
  is_info->is_chromatix_info.rs_offset_1 = chromatix_wrapper->IS.rs_offset_1;
  is_info->is_chromatix_info.rs_offset_2 = chromatix_wrapper->IS.rs_offset_2;
  is_info->is_chromatix_info.rs_offset_3 = chromatix_wrapper->IS.rs_offset_3;
  is_info->is_chromatix_info.rs_offset_4 = chromatix_wrapper->IS.rs_offset_4;
  is_info->is_chromatix_info.s3d_offset_1 = chromatix_wrapper->IS.s3d_offset_1;
  is_info->is_chromatix_info.s3d_offset_2 = chromatix_wrapper->IS.s3d_offset_2;
  is_info->is_chromatix_info.s3d_offset_3 = chromatix_wrapper->IS.s3d_offset_3;
  is_info->is_chromatix_info.s3d_offset_4 = chromatix_wrapper->IS.s3d_offset_4;
  is_info->is_chromatix_info.rs_threshold_1 = chromatix_wrapper->IS.rs_threshold_1;
  is_info->is_chromatix_info.rs_threshold_2 = chromatix_wrapper->IS.rs_threshold_2;
  is_info->is_chromatix_info.rs_threshold_3 = chromatix_wrapper->IS.rs_threshold_3;
  is_info->is_chromatix_info.s3d_threshold_1 = chromatix_wrapper->IS.s3d_threshold_1;
  is_info->is_chromatix_info.s3d_threshold_2 = chromatix_wrapper->IS.s3d_threshold_2;
  is_info->is_chromatix_info.s3d_threshold_3 = chromatix_wrapper->IS.s3d_threshold_3;
  is_info->is_chromatix_info.rs_time_interval_1 = chromatix_wrapper->IS.rs_time_interval_1;
  is_info->is_chromatix_info.rs_time_interval_2 = chromatix_wrapper->IS.rs_time_interval_2;
  is_info->is_chromatix_info.rs_time_interval_3 = chromatix_wrapper->IS.rs_time_interval_3;
  is_info->is_chromatix_info.rs_time_interval_4 = chromatix_wrapper->IS.rs_time_interval_4;
  is_info->is_chromatix_info.reserve_1 = chromatix_wrapper->IS.reserve_1;
  is_info->is_chromatix_info.reserve_2 = chromatix_wrapper->IS.reserve_2;
  is_info->is_chromatix_info.reserve_3 = chromatix_wrapper->IS.reserve_3;
  is_info->is_chromatix_info.reserve_4 = chromatix_wrapper->IS.reserve_4;
  pthread_rwlock_unlock(&chromatix_wrapper->lock);

  return TRUE;
}

/** is_set_parameters:
 *    @param: information about parameter to be set
 *    @is_info: IS information
 *    @output: output of the set processing
 *
 * Returns TRUE ons success
 **/
boolean is_set_parameters(is_set_parameter_t *param, is_info_t *is_info, is_set_output_t *output)
{
  boolean rc = FALSE;

  switch (param->type) {
  case IS_SET_PARAM_STREAM_CONFIG:
     IS_LOW("IS_SET_PARAM_STREAM_CONFIG, ma = %u, p = %d",
      param->u.is_sensor_info.sensor_mount_angle,
      param->u.is_sensor_info.camera_position);
    is_info->sensor_mount_angle = param->u.is_sensor_info.sensor_mount_angle;
    is_info->camera_position = param->u.is_sensor_info.camera_position;
    break;

  case IS_SET_PARAM_DIS_CONFIG:
    if (param->u.is_config_info.stream_type == CAM_STREAM_TYPE_VIDEO) {
      is_info->width[IS_VIDEO] = param->u.is_config_info.width;
      is_info->height[IS_VIDEO] = param->u.is_config_info.height;
      IS_HIGH("IS_SET_PARAM_DIS_CONFIG, vwidth = %ld, vheight = %ld",
        is_info->width[IS_VIDEO], is_info->height[IS_VIDEO]);
    } else {
      /* Must be CAM_STREAM_TYPE_PREVIEW because other types are filtered out
         by poster of this event */
      is_info->width[IS_PREVIEW] = param->u.is_config_info.width;
      is_info->height[IS_PREVIEW] = param->u.is_config_info.height;
      IS_HIGH("IS_SET_PARAM_DIS_CONFIG, pwidth = %ld, pheight = %ld",
        is_info->width[IS_PREVIEW], is_info->height[IS_PREVIEW]);
    }
    break;

  case IS_SET_PARAM_OUTPUT_DIM:
    break;

  case IS_SET_PARAM_CHROMATIX: {
    mct_event_t sns_cfg_req_event;
    mct_event_sns_config_request_t sns_config_request;

    is_set_init_chromatix(is_info, &param->u.is_init_param);

    IS_LOW("IS_SET_PARAM_CHROMATIX\nvirtual margin = %f\n"
      "gyro noise floor = %f\ngyro pixel scale = %d\ngyro frequency = %u",
      is_info->is_chromatix_info.virtual_margin, is_info->is_chromatix_info.gyro_noise_floor,
      is_info->is_chromatix_info.gyro_pixel_scale, is_info->is_chromatix_info.gyro_frequency);

    sns_config_request.is_gyro_sampling_rate = is_info->is_chromatix_info.gyro_frequency;
    sns_cfg_req_event.type = MCT_EVENT_MODULE_EVENT;
    sns_cfg_req_event.identity = param->u.is_init_param.event_id;
    sns_cfg_req_event.direction = MCT_EVENT_UPSTREAM;
    sns_cfg_req_event.u.module_event.type = MCT_EVENT_MODULE_SNS_CONFIG_REQUEST;
    sns_cfg_req_event.u.module_event.module_event_data = &sns_config_request;
    mct_port_send_event_to_peer(param->u.is_init_param.port, &sns_cfg_req_event);
  }
    break;

  case IS_SET_PARAM_IS_ENABLE:
    is_info->is_enabled[IS_PREVIEW] = param->u.is_enable;
    is_info->is_enabled[IS_VIDEO] = param->u.is_enable;
    IS_LOW("IS_SET_PARAM_IS_ENABLE, IS enable = %u", param->u.is_enable);
    break;

  case IS_SET_PARAM_ROLE_SWITCH:
    if (param->u.is_role_switch_mode == CAM_MODE_SECONDARY) {
      if ((is_info->is_type[IS_PREVIEW] == IS_TYPE_EIS_3_0 && is_info->is_inited[IS_PREVIEW]) ||
          (is_info->is_type[IS_VIDEO] == IS_TYPE_EIS_3_0 && is_info->is_inited[IS_VIDEO])) {
        is_info->sof_countdown = is_info->buffer_delay;
        output->type = IS_SET_OUTPUT_ROLE_SWITCH;
        output->request_frame = TRUE;
        rc = TRUE;
      }
    }
    IS_LOW("IS_SET_PARAM_ROLE_SWITCH, dual cam mode = %d", param->u.is_role_switch_mode);
    break;

  case IS_SET_PARAM_LOW_POWER_MODE:
    IS_LOW("IS_SET_PARAM_LOW_POWER_MODE, enable = %d, priority = %d, perf_mode = %d",
      param->u.is_low_power_mode.enable,
      param->u.is_low_power_mode.priority,
      param->u.is_low_power_mode.perf_mode);
    break;

  default:
    break;
  }

  return rc;
} /* is_set_parameters */
