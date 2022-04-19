/* sensor_frame_control.c
 *
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "sensor_frame_control.h"
#include "sensor_util.h"
#include <cutils/properties.h>

#define APPLY_NOW(a, b) ((a) == (b) ? TRUE:FALSE)

static boolean post_to_bus(mct_module_t* module,
  uint32_t session_id, mct_bus_msg_type_t type, uint32_t size, void *msg)
{
  mct_bus_msg_t bus_msg;
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = session_id;
  bus_msg.type      = type;
  bus_msg.size      = size;
  bus_msg.msg       = msg;

  if (mct_module_post_bus_msg(module, &bus_msg) == FALSE) {
    SERR("failed");
    return FALSE;
  }

  return TRUE;
}

/** sensor_util_post_led_state_msg: post LED state message on
 *  bus
 *
 *  @s_module: mct module handle
 *  @flash_params: flash parameters
 *  @session_id: session id
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates LED bus message and posts on bus
 **/
static boolean post_meta_flash(mct_module_t *s_module,
  sensor_flash_params_t *flash_params, uint32_t session_id)
{
  boolean                ret = TRUE;
  mct_bus_msg_t          bus_msg;
  uint8_t                flash_mode = 0;
  int32_t                flash_state = 0;

  RETURN_ON_NULL(s_module);
  RETURN_ON_NULL(flash_params);

  SLOW("flash_mode %d flash_state %d",
    flash_params->flash_mode, flash_params->flash_state);

  /* Fill bus msg params for flash mode */
  flash_mode = flash_params->flash_mode;
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = session_id;
  bus_msg.type = MCT_BUS_MSG_SENSOR_FLASH_MODE;
  bus_msg.msg = &flash_mode;
  bus_msg.size = sizeof(flash_mode);
  ret = mct_module_post_bus_msg(s_module, &bus_msg);
  if (ret == FALSE) {
    SERR("failed");
  }

  /* Fill bus msg params for flash state */
  flash_state = flash_params->flash_state;
  bus_msg.sessionid = session_id;
  bus_msg.type = MCT_BUS_MSG_SENSOR_FLASH_STATE;
  bus_msg.msg = &flash_state;
  bus_msg.size = sizeof(flash_state);
  ret = mct_module_post_bus_msg(s_module, &bus_msg);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

static boolean calculate_exposure_start_timestamp(
  mct_module_t* module, module_sensor_bundle_info_t* s_bundle,
  mct_event_t* event, int64_t exposure_time)
{
  int32_t                     rc = 0;
  sensor_exposure_starttime_t exposure_timestamp;
  uint32_t                    report_delay;
  mct_bus_msg_isp_sof_t      *mct_event_data;
  struct timeval             *sof_timestamp;

  if (event->u.ctrl_event.type != MCT_EVENT_CONTROL_SOF) {
    SERR("wrong event %d :SHOULD BE MCT_EVENT_CONTROL_SOF",
      event->u.ctrl_event.type);
    return FALSE;
  }

  memset(&exposure_timestamp, 0x00, sizeof(sensor_exposure_starttime_t));

  report_delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_meta_reporting_delay;

  mct_event_data =
    (mct_bus_msg_isp_sof_t *)(event->u.ctrl_event.control_event_data);
  sof_timestamp = &mct_event_data->mono_timestamp;

  exposure_timestamp.sof_timestamp =
    (int64_t)sof_timestamp->tv_sec * 1000000000 +
    (int64_t)sof_timestamp->tv_usec * 1000;
  exposure_timestamp.exposure_time = exposure_time;

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_EXPOSURE_START_TIME, &exposure_timestamp, rc);
  if (rc < 0)
    return FALSE;

  SLOW("exposure start time %lld", exposure_timestamp.exposure_start_timestamp);
  SLOW("frame readout time %lld", exposure_timestamp.frame_readout_time);
  if (report_delay == 0) {
    post_to_bus(module, s_bundle->sensor_info->session_id,
      MCT_BUS_MSG_SENSOR_EXPOSURE_TIMESTAMP,
      sizeof(int64_t), &exposure_timestamp.exposure_start_timestamp);
    SLOW("[process %d] report exposure timestamp meta", s_bundle->last_idx);
    #ifdef CAMERA_FEATURE_THIRDPARTYEIS
      rc = sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SENSOR_ROLLING_SHUTTER_SKEW, &exposure_timestamp);
      if (rc <0)
          return FALSE;
    #endif
    post_to_bus(module, s_bundle->sensor_info->session_id,
      MCT_BUS_MSG_SENSOR_ROLLING_SHUTTER_SKEW,
      sizeof(int64_t), &exposure_timestamp.frame_readout_time);
    SLOW("[process %d] report rolling shutter skew meta", s_bundle->last_idx);

  } else {
    sensor_fc_store(s_bundle, s_bundle->last_idx + report_delay,
      SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP,
      &exposure_timestamp.exposure_start_timestamp);
    SLOW("[store %d] SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP",
      s_bundle->last_idx + report_delay);

    sensor_fc_store(s_bundle, s_bundle->last_idx + report_delay,
      SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW,
      &exposure_timestamp.frame_readout_time);
    SLOW("[store %d] SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW",
      s_bundle->last_idx + report_delay);
  }

  return TRUE;
}

/** sensor_fc_store: Store frame control requests
 *
 *  @s_bundle: sensor bundle info
 *  @future_frame_id: future frame id to apply
 *  @type: Type of frame control parm
 *  @data: Data associated with frame control parm
 *
 *  This function is called to queue the frame control parms
 *  request to be processed later
 *
 *  Return: TRUE for success and FALSE for failure
**/
boolean sensor_fc_store(module_sensor_bundle_info_t* s_bundle,
    uint32_t future_frame_id, sensor_frame_ctrl_type_t type, void *data)
{
  sensor_frame_ctrl_data_t *frame_ctrl_data = NULL;
  /* Push the sensor internal events to dedicated queue */
  int32_t q_idx = (type >= SENSOR_FRAME_CTRL_INT_FPS_RANGE) ?
          (FRAME_CTRL_SIZE - 1):(future_frame_id%(FRAME_CTRL_SIZE-1));

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(data);

  frame_ctrl_data = CAM_CALLOC(1, sizeof(sensor_frame_ctrl_data_t));
  RETURN_ON_NULL(frame_ctrl_data);

  if ((int)type >=0 && type < SENSOR_FRAME_CTRL_MAX) {
    frame_ctrl_data->frame_ctrl_type = type;
    frame_ctrl_data->current_frame_id = future_frame_id;
  } else {
    free(frame_ctrl_data);
    frame_ctrl_data = NULL;
    SERR("Invalid frame control type %d", type);
    return FALSE;
  }

  SLOW("type %d future_frame_id %d", type, future_frame_id);

  switch(type) {
  case SENSOR_FRAME_CTRL_BET_META:
    memcpy(&frame_ctrl_data->u.bet_meta, data,
      sizeof(sensor_bet_metadata_t));
    break;
  case SENSOR_FRAME_CTRL_EXP_META: {
    sensor_aec_update_t *aec_update = (sensor_aec_update_t *)data;
    frame_ctrl_data->u.aec_update.real_gain =
      aec_update->real_gain;
    frame_ctrl_data->u.aec_update.sensor_gain =
      aec_update->sensor_gain;
    frame_ctrl_data->u.aec_update.lux_idx =
      aec_update->lux_idx;
    frame_ctrl_data->u.aec_update.linecount =
      aec_update->linecount;
    frame_ctrl_data->u.aec_update.hdr_sensitivity_ratio =
      aec_update->hdr_sensitivity_ratio;
    frame_ctrl_data->u.aec_update.hdr_exp_time_ratio =
      aec_update->hdr_exp_time_ratio;
    frame_ctrl_data->u.aec_update.total_drc_gain =
      aec_update->total_drc_gain;
    }
    break;
  case SENSOR_FRAME_CTRL_EXP: {
    sensor_aec_update_t *aec_update = (sensor_aec_update_t *)data;
    frame_ctrl_data->u.aec_update.real_gain =
      aec_update->real_gain;
    frame_ctrl_data->u.aec_update.linecount =
      aec_update->linecount;
    frame_ctrl_data->u.aec_update.est_state=
      aec_update->est_state;
    frame_ctrl_data->u.aec_update.lux_idx=
      aec_update->lux_idx;
    }
    break;
  case SENSOR_FRAME_CTRL_FRAME_DURATION:
    frame_ctrl_data->u.frame_duration = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_EXP_TIME:
    frame_ctrl_data->u.post_exp_time = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_FRAME_DURATION:
    frame_ctrl_data->u.post_frame_time = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_SENSITIVITY:
    frame_ctrl_data->u.post_sensitivity = *(int32_t *)data;
    break;
  case SENSOR_FRAME_CTRL_LED_FLASH:
    frame_ctrl_data->u.flash_event = *(sensor_submodule_event_type_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_LED_BUS_MSG:
    frame_ctrl_data->u.flash_params = *(sensor_flash_params_t *)data;
    break;
  case SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE:
    frame_ctrl_data->u.flash_params.flash_state = *(cam_flash_state_t *)data;
    break;
  case SENSOR_FRAME_CTRL_DELAY_CONFIG_CHANGE:
    frame_ctrl_data->u.delay_state = *(int32_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ:
    frame_ctrl_data->u.flash_ctrl = *(cam_flash_ctrl_t *)data;
    break;
  case SENSOR_FRAME_CTRL_CALCULATE_EXP_START_TIMESTAMP:
    frame_ctrl_data->u.exp_time = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_MANUAL_EXPOSURE_TIME:
    frame_ctrl_data->u.manual_exp_time = *(int64_t *)data;
    break;
  case SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP:
    frame_ctrl_data->u.post_exp_start_timestamp = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW:
    frame_ctrl_data->u.frame_readout_time = *(int64_t *)data;
    break;
  case SENSOR_FRAME_CTRL_TEST_PATTERN_DATA:
  case SENSOR_FRAME_CTRL_POST_TEST_PATTERN_DATA: {
    cam_test_pattern_data_t *test_pattern = (cam_test_pattern_data_t *)data;
    frame_ctrl_data->u.test_pattern = *test_pattern;
  }
    break;
  case SENSOR_FRAME_CTRL_INT_FPS_RANGE:
    frame_ctrl_data->u.fps_info = *(cam_fps_range_t *)data;
    break;
  case SENSOR_FRAME_CTRL_INT_SET_AFTUNING:
    frame_ctrl_data->u.tdata = *(tune_actuator_t *)data;
    break;
  case SENSOR_FRAME_CTRL_INT_TEST_PATTERN:
    frame_ctrl_data->u.tpattern = *(cam_test_pattern_data_t *)data;
    break;
  case SENSOR_FRAME_CTRL_INT_LED_MODE:
    frame_ctrl_data->u.led_info = *(sensor_led_mode_info_t *)data;
    break;
  case SENSOR_FRAME_CTRL_IR_LED_ON:
    frame_ctrl_data->u.led_params = *(sensor_ir_led_params_t *)data;
    break;
  case SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER:
    frame_ctrl_data->u.ir_cut_event = *(sensor_submodule_event_type_t *)data;
    break;
  case SENSOR_FRAME_CTRL_INT_HAL_SET_PARM: {
    mct_event_control_parm_t *parm = (mct_event_control_parm_t *)data;
    frame_ctrl_data->u.yuv_parm.type = parm->type;
    frame_ctrl_data->u.yuv_parm.u.data = *(int32_t *)parm->parm_data;
    }
    break;
  case SENSOR_FRAME_CTRL_INT_FOCUS_BKT:
    frame_ctrl_data->u.af_bkt = *(cam_af_bracketing_t *)data;
    break;
  case SENSOR_FRAME_CTRL_IR_CHROMATIX_RELOAD:
    frame_ctrl_data->u.ir_mode = *(boolean *)data;
    break;
  default:
    break;
  }

  pthread_mutex_lock(&s_bundle->frame_ctrl.frame_ctrl_mutex[q_idx]);
  mct_queue_push_tail(s_bundle->frame_ctrl.frame_ctrl_q[q_idx],frame_ctrl_data);
  pthread_mutex_unlock(&s_bundle->frame_ctrl.frame_ctrl_mutex[q_idx]);

  return TRUE;
}

/** sensor_fc_process: Process frame control requests
 *
 *  @module:  mct module handle
 *  @s_bundle: sensor bundle info
 *  @event: mct_event_t
 *
 *  This function is called for every sof when frame control is enabled.
 *  It handles the frame control requests scheduled at this SOF
 *
 *  Return: TRUE for success and FALSE for failure **/
boolean sensor_fc_process(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, mct_event_t* event)
{
  boolean                      ret = TRUE;
  int32_t                      rc  = 0;
  int32_t                      idx = 0;
  sensor_frame_ctrl_data_t    *sensor_frame_ctrl_data = NULL;
  module_sensor_params_t      *led_module_params;
  module_sensor_params_t      *ir_module_params;
  mct_bus_msg_t                bus_msg;
  mct_bus_msg_delay_dequeue_t  bus_delay_msg;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event);

  led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];

  idx = s_bundle->last_idx % (FRAME_CTRL_SIZE-1);

  do {
    pthread_mutex_lock(&s_bundle->frame_ctrl.frame_ctrl_mutex[idx]);
    sensor_frame_ctrl_data =
      mct_queue_pop_head(s_bundle->frame_ctrl.frame_ctrl_q[idx]);
    pthread_mutex_unlock(&s_bundle->frame_ctrl.frame_ctrl_mutex[idx]);
    if (!sensor_frame_ctrl_data)
      break;

    if (sensor_frame_ctrl_data->current_frame_id != s_bundle->last_idx) {
      SHIGH("Frame control info in wrong queue for type = %d frame %d exp %d",
        (int)sensor_frame_ctrl_data->frame_ctrl_type,
        sensor_frame_ctrl_data->current_frame_id, s_bundle->last_idx);
      free(sensor_frame_ctrl_data);
      sensor_frame_ctrl_data = NULL;
      continue;
    }

    switch(sensor_frame_ctrl_data->frame_ctrl_type) {
    case SENSOR_FRAME_CTRL_BET_META: {
      sensor_bet_metadata_t *bet_meta = &sensor_frame_ctrl_data->u.bet_meta;

      SLOW("[process %d] SENSOR_FRAME_CTRL_BET_META", s_bundle->last_idx);

      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SENSOR_BET_META,
        sizeof(sensor_bet_metadata_t), &sensor_frame_ctrl_data->u.bet_meta);

      SLOW("post BET frame %d w %d h %d bp %d srg %f sdg %f et %d dv %d",
        s_bundle->last_idx, bet_meta->width, bet_meta->height,
        bet_meta->bayer_pattern,
        bet_meta->real_gain, bet_meta->isp_digital_gain,
        bet_meta->exposure_time, bet_meta->dac_value);
    }
      break;
    case SENSOR_FRAME_CTRL_EXP_META: {
      stats_get_data_t *stats_get;

      SLOW("[process %d] SENSOR_FRAME_CTRL_EXP_META", s_bundle->last_idx);

      stats_get = (stats_get_data_t *)s_bundle->aec_metadata.private_data;
      if (stats_get) {
        stats_get->aec_get.real_gain[0] =
          sensor_frame_ctrl_data->u.aec_update.real_gain;
        stats_get->aec_get.sensor_gain[0] =
          sensor_frame_ctrl_data->u.aec_update.sensor_gain;
        stats_get->aec_get.lux_idx =
          sensor_frame_ctrl_data->u.aec_update.lux_idx;
        stats_get->aec_get.valid_entries = 1;
        stats_get->aec_get.linecount[0] =
          sensor_frame_ctrl_data->u.aec_update.linecount;
        stats_get->aec_get.hdr_sensitivity_ratio =
          sensor_frame_ctrl_data->u.aec_update.hdr_sensitivity_ratio;
        stats_get->aec_get.hdr_exp_time_ratio =
          sensor_frame_ctrl_data->u.aec_update.hdr_exp_time_ratio;
        stats_get->aec_get.total_drc_gain =
          sensor_frame_ctrl_data->u.aec_update.total_drc_gain;

        post_to_bus(module, s_bundle->sensor_info->session_id,
          MCT_BUS_MSG_SET_STATS_AEC_INFO,
          sizeof(stats_get_data_t), &s_bundle->aec_metadata);
        SLOW("post g %f lc %d lux %f cur sof %d",
          sensor_frame_ctrl_data->u.aec_update.real_gain,
          sensor_frame_ctrl_data->u.aec_update.linecount,
          sensor_frame_ctrl_data->u.aec_update.lux_idx, s_bundle->last_idx);
      }
    }
      break;
    case SENSOR_FRAME_CTRL_FRAME_DURATION:
      SLOW("[process %d] SENSOR_FRAME_CTRL_FRAME_DURATION %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.frame_duration);

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_FRAME_DURATION,
        &sensor_frame_ctrl_data->u.frame_duration, rc);
      if (rc < 0) {
        SERR("Failed to set frame duration");
        ret = FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_MANUAL_EXPOSURE_TIME:
      SLOW("[process %d] SENSOR_FRAME_CTRL_MANUAL_EXPOSURE_TIME %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.manual_exp_time);

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_MANUAL_EXPOSURE_TIME,
        &sensor_frame_ctrl_data->u.manual_exp_time, rc);
      if (rc < 0) {
        SERR("Failed to set manual exposure time");
        ret = FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_EXT_AEC_UPDATE:
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EXT,
        EXT_AEC_UPDATE, NULL, rc);
      if (rc < 0) {
        SERR("Failed to external aec update");
        ret = FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_EXP: {
      sensor_aec_update_t *aec_update = &sensor_frame_ctrl_data->u.aec_update;

      SLOW("[process %d] SENSOR_FRAME_CTRL_EXP g %f lc %d",
        s_bundle->last_idx, aec_update->real_gain, aec_update->linecount);

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_AEC_MANUAL_UPDATE, aec_update, rc);
      if (rc < 0) {
        SERR("unable to process aec update request");
        ret = FALSE;
      } else {
        if (sensor_util_set_digital_gain_to_isp(module,
          s_bundle, event->identity, aec_update) == FALSE)
          SERR("failed");
      }
    }
      break;
    case SENSOR_FRAME_CTRL_POST_EXP_TIME:
      SLOW("[process %d] SENSOR_FRAME_CTRL_POST_EXP_TIME %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.post_exp_time);

      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_EXPOSURE_TIME,
        sizeof(int64_t),
        &sensor_frame_ctrl_data->u.post_exp_time);
      break;
    case SENSOR_FRAME_CTRL_POST_FRAME_DURATION:
      SLOW("[process %d] SENSOR_FRAME_CTRL_POST_FRAME_DURATION %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.post_frame_time);

      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_FRAME_DURATION,
        sizeof(int64_t),
        &sensor_frame_ctrl_data->u.post_frame_time);
      break;
    case SENSOR_FRAME_CTRL_POST_SENSITIVITY:
      SLOW("[process %d] SENSOR_FRAME_CTRL_POST_SENSITIVITY %d",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.post_sensitivity);

      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_SENSITIVITY,
        sizeof(int32_t),
        &sensor_frame_ctrl_data->u.post_sensitivity);
       break;
    case SENSOR_FRAME_CTRL_LED_FLASH: {
      sensor_submodule_event_type_t flash_event =
        sensor_frame_ctrl_data->u.flash_event;
      cam_flash_ctrl_t flash_ctrl;
      cam_flash_mode_t flash_mode_to_isp;

      SLOW("processing CTRL_LED_FLASH flash_event:%d", flash_event);
      if (led_module_params->func_tbl.process != NULL) {

        if (flash_event == LED_FLASH_SET_MAIN_FLASH) {
          s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_SINGLE;
          flash_mode_to_isp = CAM_FLASH_MODE_SINGLE;
          flash_ctrl = CAM_FLASH_CTRL_SINGLE;
        } else if (flash_event == LED_FLASH_SET_TORCH) {
          s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_TORCH;
          flash_mode_to_isp = CAM_FLASH_MODE_TORCH;
          flash_ctrl = CAM_FLASH_CTRL_TORCH;
        } else {
          /* only report PARTIAL with SINGLE/TORCH mode
             instead of OFF/PARTIAL : [SINGLE/TORCH]/PARTIAL
             so CAM_FLASH_MODE_OFF case, don't change flash_mode
          */
          flash_mode_to_isp = CAM_FLASH_MODE_OFF;
          flash_ctrl = CAM_FLASH_CTRL_OFF;
        }

        /* Fill information for the flash offload to thread */
        sensor_capture_control_t *ctrl = &s_bundle->cap_control;
        memset(ctrl, 0, sizeof(sensor_capture_control_t));
        sensor_batch_config_t *bctrl = &ctrl->bctrl[0];
        bctrl->burst_count = 1;
        bctrl->flash_mode = flash_event;
        bctrl->cam_flash_mode = s_bundle->flash_params.flash_mode;
        bctrl->skip_config_mode = 1;
        s_bundle->batch_idx = 0;
        ret = module_sensor_post_flash_mode_to_thread(module, &event->identity,
            s_bundle, NULL);

        s_bundle->flash_params.flash_state = CAM_FLASH_STATE_PARTIAL;

        sensor_util_send_led_mode_downstream(module,
          &flash_mode_to_isp,
          event->identity, s_bundle->last_idx);
        SLOW("downstream to ISP flash ctrl:%d", flash_ctrl);
      }
    }
      break;
    case SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE:
      s_bundle->flash_params.flash_state =
        sensor_frame_ctrl_data->u.flash_params.flash_state;

      /* only report PARTIAL with SINGLE/TORCH mode
         instead of OFF/PARTIAL : [SINGLE/TORCH]/PARTIAL */
      if (s_bundle->flash_params.flash_state == CAM_FLASH_STATE_READY)
        s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_OFF;

      SLOW("SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE %d",
        s_bundle->flash_params.flash_state);
      break;
    case SENSOR_FRAME_CTRL_DELAY_CONFIG_CHANGE:
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_DELAY_CFG, &sensor_frame_ctrl_data->u.delay_state, rc);
      if (rc < 0) {
        SERR("unable to set sensor delay config");
        ret = FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_IR_LED_ON:
      ir_module_params = s_bundle->module_sensor_params[SUB_MODULE_IR_LED];
      if (ir_module_params && ir_module_params->func_tbl.process != NULL) {
        sensor_ir_led_cfg_t cfg;
        cfg.intensity = sensor_frame_ctrl_data->u.led_params.ir_led_brightness;
        rc = ir_module_params->func_tbl.process(
          ir_module_params->sub_module_private,
          sensor_frame_ctrl_data->u.led_params.ir_led_event, &cfg);
        if (rc < 0) {
          SERR("failed: IR_LED_SET_ON");
          return FALSE;
        }
      }
      else {
        SERR("invalid IR LED module params");
        return FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER:
      ir_module_params = s_bundle->module_sensor_params[SUB_MODULE_IR_CUT];
      if (ir_module_params && ir_module_params->func_tbl.process != NULL) {
        rc = ir_module_params->func_tbl.process(
          ir_module_params->sub_module_private,
          sensor_frame_ctrl_data->u.ir_cut_event, NULL);
        if (rc < 0) {
          SERR("failed: IR_CUT_SET_ON");
          return FALSE;
        }
      }
      else {
        SERR("invalid IR CUT module params");
        return FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_IR_CHROMATIX_RELOAD:
      if (module_sensor_reload_chromatix_for_ir_mode(s_bundle,
        module, event->identity, sensor_frame_ctrl_data->u.ir_mode) == FALSE) {
        SERR("IR mode chromatix reload failed");
        return FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ:
      SLOW("SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ index %d",
        s_bundle->last_idx);
      /* query led delay from driver*/
      uint32_t  led_delay = 2;
      bus_delay_msg.curr_frame_id = s_bundle->last_idx;
      bus_delay_msg.delay_numframes = led_delay;
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_DELAY_SUPER_PARAM;
      bus_msg.size = sizeof(mct_bus_msg_delay_dequeue_t);
      bus_msg.msg = &bus_delay_msg;
      if(mct_module_post_bus_msg(module, &bus_msg) == FALSE) {
        SERR("failed");
        return FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_CALCULATE_EXP_START_TIMESTAMP:
      SLOW("[process %d] SENSOR_FRAME_CTRL_CALCULATE_EXP_START_TIMESTAMP",
        s_bundle->last_idx);
      calculate_exposure_start_timestamp(module, s_bundle, event,
        sensor_frame_ctrl_data->u.exp_time);
      ret |= sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_TIME, &sensor_frame_ctrl_data->u.exp_time);
      if(!ret){
        SWARN("Exposure time post event failed");
      }
      break;
    case SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP:
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SENSOR_EXPOSURE_TIMESTAMP,
        sizeof(int64_t), &sensor_frame_ctrl_data->u.post_exp_start_timestamp);
      SLOW("[process %d] SENSRO_FRAME_CTRL_POST_EXP_START_TIMESTAMP %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.post_exp_start_timestamp);
      break;
    case SENSOR_FRAME_CTRL_POST_LED_BUS_MSG:
      SLOW("[process %d] SENSOR_FRAME_CTRL_POST_LED_BUS_MSG",
        s_bundle->last_idx);
      if (post_meta_flash(module, &sensor_frame_ctrl_data->u.flash_params,
        s_bundle->sensor_info->session_id) == FALSE)
         SERR("failed");
      break;
    case SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW:
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
      rc = sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SENSOR_ROLLING_SHUTTER_SKEW,
        &sensor_frame_ctrl_data->u.frame_readout_time );
      if (rc <0)
        return FALSE;
#endif
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SENSOR_ROLLING_SHUTTER_SKEW,
        sizeof(int64_t), &sensor_frame_ctrl_data->u.frame_readout_time);
      SLOW("[process %d] SENSOR_FRAME_CTRL_POST_ROLLINGSHUTTERSKEW %lld",
        s_bundle->last_idx, sensor_frame_ctrl_data->u.frame_readout_time);
      break;
    case SENSOR_FRAME_CTRL_TEST_PATTERN_DATA:
      SLOW("processing CTRL_TEST_PATTERN_DATA at idx: %d", idx);
      cam_test_pattern_data_t *testpattern_data =
        &sensor_frame_ctrl_data->u.test_pattern;
      SLOW("apply test pattern mode %d cur sof %d",
        testpattern_data->mode, s_bundle->last_idx);
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_TEST_PATTERN, testpattern_data, rc);
      if (rc < 0) {
        SERR("Failed to set frame duration");
        ret = FALSE;
      }
      break;
    case SENSOR_FRAME_CTRL_POST_TEST_PATTERN_DATA:
      SLOW("post frame %d test_pattern mode %d", s_bundle->last_idx,
      sensor_frame_ctrl_data->u.test_pattern.mode);
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_TEST_PATTERN_DATA,
        sizeof(cam_test_pattern_data_t),
        &sensor_frame_ctrl_data->u.test_pattern);
      break;

    default:
      break;
    }
    free(sensor_frame_ctrl_data);
    sensor_frame_ctrl_data = NULL;
  } while (1);

  return ret;
}

/** sensor_fc_process_now: Process frame control requests in queue immediately
 *
 *  @module:  mct module handle
 *  @s_bundle: sensor bundle info
 *  @event: mct_event_t
 *
 *  This function is called when streaming off all streams.
 *  Some requests need to be processed before sreaming off.
 *  Check all queue and process immediately the "must process" requests.
 *
 **/
boolean sensor_fc_process_now(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, mct_event_t* event, uint32_t qidx)
{
  int32_t                      rc  = 0;
  uint32_t                     i;
  boolean                      ret = TRUE;
  sensor_frame_ctrl_data_t    *sensor_frame_ctrl_data = NULL;
  module_sensor_params_t      *led_module_params;
  module_sensor_params_t      *module_sensor_params;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event);

  led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

  for (i = 0; i < FRAME_CTRL_SIZE; i++) {
    /* If specific qidx is passed, process that queue instead of all queues */
    if (qidx < FRAME_CTRL_SIZE && i != qidx)
      continue;
    do {
      pthread_mutex_lock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
      sensor_frame_ctrl_data =
        mct_queue_pop_head(s_bundle->frame_ctrl.frame_ctrl_q[i]);
      pthread_mutex_unlock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
      if (!sensor_frame_ctrl_data)
        break;

      switch(sensor_frame_ctrl_data->frame_ctrl_type) {
      case SENSOR_FRAME_CTRL_LED_FLASH: {
        sensor_submodule_event_type_t flash_event =
          sensor_frame_ctrl_data->u.flash_event;
        cam_flash_ctrl_t flash_ctrl;

        SLOW("processing CTRL_LED_FLASH flash_event:%d", flash_event);
        if (led_module_params->func_tbl.process != NULL) {
          led_module_params->func_tbl.process(
            led_module_params->sub_module_private, flash_event, NULL);

          if (flash_event == LED_FLASH_SET_MAIN_FLASH) {
            s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_SINGLE;
            flash_ctrl = CAM_FLASH_CTRL_SINGLE;
          } else if (flash_event == LED_FLASH_SET_TORCH) {
            s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_TORCH;
            flash_ctrl = CAM_FLASH_CTRL_TORCH;
          } else {
            s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_OFF;
            flash_ctrl = CAM_FLASH_CTRL_OFF;
          }
          s_bundle->flash_params.flash_state = CAM_FLASH_STATE_PARTIAL;

          sensor_util_send_led_mode_downstream(module,
            &s_bundle->flash_params.flash_mode,
            event->identity, s_bundle->last_idx);
          SLOW("downstream to ISP flash ctrl:%d", flash_ctrl);
        }
      }
        break;
      case SENSOR_FRAME_CTRL_INT_FPS_RANGE:
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
          SENSOR_SET_FPS,
          &sensor_frame_ctrl_data->u.fps_info, rc);
        if (rc < 0) {
          SERR("Failed to set fps");
          ret = FALSE;
        }
        break;
      case SENSOR_FRAME_CTRL_INT_SET_AFTUNING:
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
          ACTUATOR_FOCUS_TUNING,
          &sensor_frame_ctrl_data->u.tdata, rc);
        if (rc < 0) {
          SERR("Failed to set af tuning");
          ret = FALSE;
        }
        break;
      case SENSOR_FRAME_CTRL_INT_TEST_PATTERN:
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
          SENSOR_SET_TEST_PATTERN,
          &sensor_frame_ctrl_data->u.tpattern, rc);
        if (rc < 0) {
          SERR("Failed to set test pattern");
          ret = FALSE;
        }
        break;
      case SENSOR_FRAME_CTRL_INT_LED_MODE: {
        sensor_led_mode_info_t *info = &sensor_frame_ctrl_data->u.led_info;
        ret = module_sensor_set_parm_led_mode(module, s_bundle, info->capture_intent,
          info->mode, info->frame_id, info->identity);
        }
        break;
      case SENSOR_FRAME_CTRL_INT_HAL_SET_PARM: {
        sensor_yuv_parm_t *parm = &sensor_frame_ctrl_data->u.yuv_parm;
        ret = module_sensor_hal_set_parm(
          module_sensor_params, parm->type, &parm->u.data);
        }
        break;
      case SENSOR_FRAME_CTRL_INT_FOCUS_BKT:
        ret = module_sensor_set_param_af_bracket(s_bundle,
          &sensor_frame_ctrl_data->u.af_bkt);
        break;
      default:
        break;
      }
      free(sensor_frame_ctrl_data);
      sensor_frame_ctrl_data = NULL;
    } while (1);
  }

  return ret;
}

/**  sensor_fc_report_meta:
 *    report all sensor dynamic meta
 *
 *  Arguments:
 *    @module  : sensor module
 *    @s_bundle: sensor bundle info
 *
 *  Return: TRUE on success
 *          FALSE on failure **/

boolean sensor_fc_report_meta(mct_module_t* module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t* event)
{
  int32_t               rc = 0;
  int32_t               enabled;
  uint32_t              apply_frame_id, led_apply_frame_id;
  uint32_t              exposure_applied_id;
  uint32_t              pipeline_delay;
  uint32_t              report_delay;
  int8_t                decode_format;
  char                  prop[PROPERTY_VALUE_MAX];
  sensor_bet_metadata_t bet_meta;
  /* meta data to report */
  int64_t                   frame_duration = -1;
  int64_t                   exposure_time = -1;
  uint32_t                  sensitivity = 0;
  cam_test_pattern_data_t   testpattern_data;
  sensor_output_format_t    output_format;
  sensor_filter_arrangement filter_arrangement;
  sensor_raw_output_t       raw_output;
  uint32_t                  pix_fmt_fourcc = 0;
  cam_format_t              fmt;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event);

  report_delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_meta_reporting_delay;

  PTHREAD_MUTEX_LOCK(&(s_bundle->mutex));
  /* lc/gain was applied at the previous SOF context */
  /* lc/gain applied at the previous SOF context
     will be valid at the frame after sensor delay - 1 */
  exposure_applied_id = s_bundle->last_idx +
    (s_bundle->frame_ctrl.max_sensor_delay - 1);
  apply_frame_id = exposure_applied_id + report_delay;
  pipeline_delay =
    s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_max_pipeline_frame_delay;
  led_apply_frame_id = s_bundle->last_idx + report_delay;
  PTHREAD_MUTEX_UNLOCK(&(s_bundle->mutex));

  SLOW("cur_sof %d apply_frame_id %d led_apply_frame_id %d",
    s_bundle->last_idx, apply_frame_id, led_apply_frame_id);

  if(s_bundle->hal_version == CAM_HAL_V1 &&
    !(s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))) {
    s_bundle->flash_params.flash_mode = s_bundle->sensor_params.flash_mode;
    s_bundle->flash_params.flash_state = s_bundle->sensor_params.flash_state;
  }

  if (led_apply_frame_id == s_bundle->last_idx) {
    if (post_meta_flash(module, &s_bundle->flash_params,
      s_bundle->sensor_info->session_id) == FALSE)
       SERR("failed");
  } else {
    sensor_fc_store(s_bundle,
      led_apply_frame_id, SENSOR_FRAME_CTRL_POST_LED_BUS_MSG,
      &s_bundle->flash_params);
    SLOW("store META flash store %d led %d %d",
      led_apply_frame_id, s_bundle->flash_params.flash_mode,
      s_bundle->flash_params.flash_state);
  }

  /* FRAME DURATION */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_FRAME_DURATION, &frame_duration, rc);
  if (rc < 0) {
    SERR("Failed to get frame duration");
  }
  else {
    if (APPLY_NOW(apply_frame_id, s_bundle->last_idx)) {
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_FRAME_DURATION,
        sizeof(int64_t), &frame_duration);
      SLOW("[report meta] frame_duration %lld", frame_duration);
    } else {
      sensor_fc_store(s_bundle,
        apply_frame_id, SENSOR_FRAME_CTRL_POST_FRAME_DURATION, &frame_duration);
      SLOW("[store meta] frame %d frame_duration %lld",
        apply_frame_id, frame_duration);
    }
  }

  /* EXPOSURE TIME */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_EXPOSURE_TIME, &exposure_time, rc);
  if (rc < 0) {
    SERR("Failed to get exposure time");
  }
  else {
    if (APPLY_NOW(apply_frame_id, s_bundle->last_idx)) {
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_EXPOSURE_TIME,
        sizeof(int64_t), &exposure_time);
      SLOW("[report meta] exp time %lld", exposure_time);
    } else {
      sensor_fc_store(s_bundle,
        apply_frame_id, SENSOR_FRAME_CTRL_POST_EXP_TIME, &exposure_time);
      SLOW("[store meta] frame %d exp time %lld",
        apply_frame_id, exposure_time);
    }
  }

  /* TIMESTAMP : exposure start time */
  /* we can calcuate this timestamp at future frame that
     the current expousre time is applied */
  if (APPLY_NOW(exposure_applied_id, s_bundle->last_idx)) {
    calculate_exposure_start_timestamp(module, s_bundle, event,
      exposure_time);
  } else {
    sensor_fc_store(s_bundle,
      exposure_applied_id, SENSOR_FRAME_CTRL_CALCULATE_EXP_START_TIMESTAMP,
      &exposure_time);
    SLOW("[store calcuate exposure timestamp] frame %d exp time %lld",
      exposure_applied_id, exposure_time);
  }

  /* SENSITIVITY */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_SENSITIVITY, &sensitivity, rc);
  if (rc < 0) {
    SERR("Failed to get sensitivity");
  }
  else {
    if (APPLY_NOW(apply_frame_id, s_bundle->last_idx)) {
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SET_SENSOR_SENSITIVITY,
        sizeof(int32_t), &sensitivity);
      SLOW("[report meta] sensitivity %d", sensitivity);
    } else {
      sensor_fc_store(s_bundle,
        apply_frame_id, SENSOR_FRAME_CTRL_POST_SENSITIVITY, &sensitivity);
      SLOW("[store meta] frame %d sensitivity %d",
        apply_frame_id, sensitivity);
    }
  }

  /* TEST PATTERN */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_TEST_PATTERN_DATA, &testpattern_data, rc);
  if (rc < 0) {
     SERR("Failed to get test pattern data");
  }
   else {
     if (APPLY_NOW(apply_frame_id, s_bundle->last_idx)) {
       post_to_bus(module, s_bundle->sensor_info->session_id,
         MCT_BUS_MSG_TEST_PATTERN_DATA,
         sizeof(testpattern_data), &testpattern_data);
       SLOW("[report meta] testpattern mode %d", testpattern_data.mode);
     } else {
       sensor_fc_store(s_bundle,
         apply_frame_id, SENSOR_FRAME_CTRL_POST_TEST_PATTERN_DATA,
         &testpattern_data);
       SLOW("[store meta] frame %d testpattern mode %d",
         apply_frame_id, testpattern_data.mode);
     }
   }

  /* BET */
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.dumpmetadata", prop, "0");
  enabled = atoi(prop);
  if (enabled > 0) {
    memset(&bet_meta, 0, sizeof(bet_meta));

    sensor_util_post_submod_event(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_BET_METADATA, &bet_meta);
    if (IS_SENSOR_SUBMOD_VALID(s_bundle, SUB_MODULE_ACTUATOR)) {
      sensor_util_post_submod_event(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_GET_DAC_VALUE, &bet_meta.dac_value);
    }

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_SENSOR_FORMAT, &output_format, rc);
    if (rc < 0) {
      SERR("get sensor format failed rc %d", rc);
      return FALSE;
    }

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_FILTER_ARRANGEMENT, &filter_arrangement, rc);
    if (rc < 0) {
      SERR("SENSOR_GET_FILTER_ARRANGEMENT failed rc %d", rc);
      return FALSE;
    }

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_RAW_OUTPUT_BIT_WIDTH, &raw_output, rc);
    if (rc < 0) {
      SERR("SENSOR_GET_RAW_OUTPUT_BIT_WIDTH failed rc %d", rc);
      return FALSE;
    }

    decode_format = sensor_util_get_decode_fmt(raw_output);
    pix_fmt_fourcc = sensor_util_get_fourcc_format(
      output_format, filter_arrangement, decode_format);
    fmt = sensor_util_get_hal_format(pix_fmt_fourcc);
    bet_meta.bayer_pattern = fmt;

    if (s_bundle->last_idx <= pipeline_delay) {
      /* For first N (=max_pp_delay) frames, sensor need to pass
         initial exposure values directly to bus */
      post_to_bus(module, s_bundle->sensor_info->session_id,
        MCT_BUS_MSG_SENSOR_BET_META,
        sizeof(sensor_bet_metadata_t), &bet_meta);
      SLOW("[report meta] BET_META (frame_idx = %d, real_gain = %f)",
        s_bundle->last_idx, bet_meta.real_gain);
    }
    sensor_fc_store(s_bundle,
      apply_frame_id, SENSOR_FRAME_CTRL_BET_META, &bet_meta);
    SLOW("[store meta] BET_META (apply_idx = %d, real_gain = %f)",
      apply_frame_id, bet_meta.real_gain);
  }

  return TRUE;
}

/** sensor_fc_post_static_meta:
 *
 *  @module: sensor module handle
 *  @s_bundle: sensor bundle structure
 *  @event: event to be handled
 *
 *  Post metadata for current frame
 *
 **/
boolean sensor_fc_post_static_meta(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t *event)
{
  mct_bus_msg_t               bus_msg;
  sensor_static_medatadata_t *static_metadata;
  module_sensor_params_t *led_module_params = NULL;
  int32_t red_eye_reduction;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event);

  static_metadata = &s_bundle->frame_ctrl.static_metadata;

  /* CHROMATIX */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SET_SENSOR_INFO,
    sizeof(s_bundle->chromatix_metadata), &s_bundle->chromatix_metadata);

  /* APETURE */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SENSOR_APERTURE,
    sizeof(static_metadata->aperture),
    &(s_bundle->sensor_common_info.camera_config.lens_info.f_number));

  /* FOCAL LENGTH */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SENSOR_FOCAL_LENGTH,
    sizeof(static_metadata->focal_length),
    &(s_bundle->sensor_common_info.camera_config.lens_info.focal_length));

  /* FILTER DENSITY */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SENSOR_FILTERDENSITY,
    sizeof(static_metadata->filter_density), &static_metadata->filter_density);

  /* OPTICAL STABILIZATION */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SENSOR_OPT_STAB_MODE,
    sizeof(static_metadata->ois_mode), &s_bundle->ois_enabled);

  /* OPT WB grgb */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUG_MSG_OTP_WB_GRGB,
    sizeof(static_metadata->opt_wb_grgb), &static_metadata->opt_wb_grgb);

  /* SENSOR CROP */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SNAP_CROP_INFO_SENSOR,
    sizeof(static_metadata->snap_crop_sensor), &static_metadata->snap_crop_sensor);

  /* CAMIF CROP */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SNAP_CROP_INFO_CAMIF,
    sizeof(static_metadata->snap_crop_camif), &static_metadata->snap_crop_camif);

  /* Red Eye Reduction on LED */
  led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  if (led_module_params &&
    led_module_params->func_tbl.process &&
    led_module_params->sub_module_private) {
    led_module_params->func_tbl.process(led_module_params->sub_module_private,
      LED_FLASH_GET_RER_PARAMS, &red_eye_reduction);
  } else{
    red_eye_reduction = 0;
  }

  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_LED_REDEYE_REDUCTION_MODE,
    sizeof(red_eye_reduction), &red_eye_reduction);

  /* IR Mode */
  post_to_bus(module, s_bundle->sensor_info->session_id,
    MCT_BUS_MSG_SET_IR_MODE,
    sizeof(sensor_ir_mode_t), &s_bundle->ir_mode);

  return TRUE;
}
