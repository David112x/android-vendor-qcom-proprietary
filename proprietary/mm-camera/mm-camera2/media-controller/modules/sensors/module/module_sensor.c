/*===========================================================================
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#include <cutils/properties.h>
#include <linux/media.h>
#include "math.h"
#include "mct_module.h"
#include "module_sensor.h"
#include "modules.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "media_controller.h"
#include "mct_event_stats.h"
#include "actuator_driver.h"
#include "port_sensor.h"
#include "sensor_util.h"
#include "sensor_frame_control.h"
#include <poll.h>
#include "sensor_init.h"
#include <../stats/q3a/q3a_stats_hw.h>
#include "mct_profiler.h"
#include "eebin_interface.h"
#include "ois.h"
#include "sensor_fast_aec.h"
#include "live_connect.h"
#include "module_sensor_offload.h"
#include "sensor.h"
#include "module_sensor_dualcamera.h"

#define WAIT_TIME_MS 5000
#define RETRY_COUNT 100
#define CHECK_STREAM(_s)  (stream_mask & (1 << _s))
#define MAX_LED_ON_DURATION_DEFAULT 1280

/** Initialization table **/
static int32_t (*sub_module_init[SUB_MODULE_MAX])(sensor_func_tbl_t *) = {
  [SUB_MODULE_SENSOR]       = sensor_sub_module_init,
  [SUB_MODULE_CHROMATIX]    = chromatix_sub_module_init,
  [SUB_MODULE_ACTUATOR]     = actuator_sub_module_init,
  [SUB_MODULE_EEPROM]       = eeprom_sub_module_init,
  [SUB_MODULE_LED_FLASH]    = led_flash_sub_module_init,
  [SUB_MODULE_CSIPHY]       = csiphy_sub_module_init,
  [SUB_MODULE_CSIPHY_3D]    = csiphy_sub_module_init,
  [SUB_MODULE_CSID]         = csid_sub_module_init,
  [SUB_MODULE_CSID_3D]      = csid_sub_module_init,
  [SUB_MODULE_OIS]          = ois_sub_module_init,
  [SUB_MODULE_EXT]          = external_sub_module_init,
  [SUB_MODULE_IR_LED]       = ir_led_sub_module_init,
  [SUB_MODULE_IR_CUT]       = ir_cut_sub_module_init,
  [SUB_MODULE_LASER_LED]    = laser_led_sub_module_init
};

static eebin_load_data_t eebin_load[] = {
  {SUB_MODULE_ACTUATOR,   (uint32_t)ACTUATOR_SET_EEBIN_DATA},
};

boolean module_sensor_event_control_set_parm(
  mct_module_t *module, uint32_t frame_id,
  module_sensor_bundle_info_t *s_bundle,
  mct_event_control_parm_t *event_control,
  uint32_t identity);
boolean module_sensor_capture_control(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle,
  boolean is_delayed);

static boolean module_sensor_load_chromatix_stream_on(
    sensor_output_format_t output_format, sensor_get_t* sensor_get,
    module_sensor_bundle_info_t *s_bundle, mct_event_t *event,
    mct_stream_info_t* stream_info, sensor_set_res_cfg_t *stream_on_cfg,
    mct_module_t *module);

static boolean module_sensor_config_pdaf(
    mct_module_t *module,
    mct_event_t *event,
    module_sensor_bundle_info_t *s_bundle);

static boolean module_sensor_set_start_stream_on(
    sensor_output_format_t output_format, mct_module_t *module,
    mct_event_t *event, module_sensor_bundle_info_t *s_bundle,
    mct_stream_info_t* stream_info, boolean stream_on_flag);

static boolean module_sensor_init_chromatix(
    void *data, void *eebin_hdl);

static void module_sensor_set_decode_format_based_on_pack_mode(
    module_sensor_bundle_info_t *s_bundle, sensor_get_t* sensor_get_params);

static void module_sensor_fill_pack_mode(
  module_sensor_bundle_info_t *s_bundle, sensor_out_info_t *sensor_out_info,
  iface_output_pack_mode pack_mode);

/** module_sensor_set_sensor_stream: set the stream mode in sensor
 *
 *  @s_bundle: bundle information
 *
 *  This function sets the modes based on hal_version and stream_mask
 *
 *  return TRUE if pointers are non-null else return FALSE
 **/
static boolean module_sensor_set_sensor_stream(
  module_sensor_bundle_info_t *s_bundle)
{

  SLOW("hal_version = %d stream_mask = 0x%X", s_bundle->hal_version,
    s_bundle->stream_mask);

  if (s_bundle->hal_version == CAM_HAL_V1) {
    if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))
      s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_VIDEO;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)) {
      if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_ZSL;
      else
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_PREVIEW;
    } else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_SNAPSHOT;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_CALLBACK))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_PREVIEW;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_RAW))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_SNAPSHOT;
    else {
      SERR("Invalid stream mask %d", s_bundle->stream_mask);
      return FALSE;
    }
  } else if (s_bundle->hal_version == CAM_HAL_V3) {
    if (s_bundle->capture_intent == CAM_INTENT_VIDEO_RECORD ||
      s_bundle->capture_intent == CAM_INTENT_VIDEO_SNAPSHOT)
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_VIDEO;
    else if (s_bundle->capture_intent == CAM_INTENT_ZERO_SHUTTER_LAG)
      s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_ZSL;
    else if (s_bundle->capture_intent == CAM_INTENT_PREVIEW)
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_PREVIEW;
    else if (s_bundle->capture_intent == CAM_INTENT_STILL_CAPTURE)
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_SNAPSHOT;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_CALLBACK))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_PREVIEW;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_RAW))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_SNAPSHOT;
    else if (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_DEPTH))
        s_bundle->sensor_common_info.sensor_mode = SENSOR_MODE_DEPTH;
    else {
      SERR("Invalid capture_intent %d", s_bundle->capture_intent);
      return FALSE;
    }
  } else {
    SERR("Invalid HAL version %d", s_bundle->hal_version);
    return FALSE;
  }

  SLOW("sensor_mode = %d", s_bundle->sensor_common_info.sensor_mode);
  return TRUE;
}

/** module_sensor_set_parm_led_mode:
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @mode: flash mode
 *  @frame_id: event frame id
 *  @identity: event identity
 *
 *  Handle set parm LED mode
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean module_sensor_set_parm_led_mode(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, boolean is_capture,
  cam_flash_mode_t mode, uint32_t frame_id, uint32_t identity)
{
  boolean                       ret = TRUE;
  uint32_t                      led_delay, stream_delay, pipeline_delay;
  uint32_t                      apply_frame_id, delay_frame_id;
  uint32_t                      cur_sof = 0;
  sensor_submodule_event_type_t flash_event;
  cam_flash_ctrl_t              flash_ctrl;
  cam_flash_state_t             next_flash_state;
  int32_t                       next_delay_state;
  module_sensor_params_t       *led_module_params = NULL;
  module_sensor_params_t       *module_sensor_params = NULL;
  mct_bus_msg_t                 bus_msg;
  mct_bus_msg_delay_dequeue_t   bus_delay_msg;
  int32_t                       delay_en = 0;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);

  pipeline_delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_frame_applying_delay;
  /* query led delay from driver */
  led_delay = 2;

  apply_frame_id = frame_id + pipeline_delay - led_delay;
  delay_frame_id = apply_frame_id - 1;
  SHIGH("SOF = %d, requested frame id = %d, apply_frame_id = %d, %d, %d",
    s_bundle->last_idx, frame_id, apply_frame_id,delay_frame_id,pipeline_delay);

  if (mode >= CAM_FLASH_MODE_MAX) {
    SERR("failed: invalid mode: %d", mode);
    return FALSE;
  }

  SHIGH("CAM_INTF_PARM_LED_MODE %d is_capture %d", mode, is_capture);
  led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(led_module_params);
  RETURN_ON_NULL(led_module_params->func_tbl.process);

  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  cur_sof = s_bundle->last_idx;
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);

  if (((CAM_FLASH_MODE_AUTO == mode) || (CAM_FLASH_MODE_ON == mode))
       && s_bundle->last_flash_request == CAM_FLASH_MODE_TORCH) {
    flash_event = LED_FLASH_SET_OFF;
    next_flash_state = CAM_FLASH_STATE_READY;
    ret = sensor_fc_store(s_bundle, apply_frame_id,
      SENSOR_FRAME_CTRL_LED_FLASH, &flash_event);
    if (!ret) {
      SERR("Failed to queue flash off info");
      return ret;
    }
    ret = sensor_fc_store(s_bundle, apply_frame_id + led_delay,
      SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE, &next_flash_state);
    s_bundle->last_flash_request = CAM_FLASH_MODE_OFF;
  }

  switch (mode) {
  case CAM_FLASH_MODE_TORCH:
    flash_event      = LED_FLASH_SET_TORCH;
    flash_ctrl       = CAM_FLASH_CTRL_TORCH;
    next_flash_state = CAM_FLASH_STATE_FIRED;
    break;
  case CAM_FLASH_MODE_SINGLE:
    flash_event      = LED_FLASH_SET_MAIN_FLASH;
    flash_ctrl       = CAM_FLASH_CTRL_SINGLE;
    next_flash_state = CAM_FLASH_STATE_FIRED;
    break;
  case CAM_FLASH_MODE_OFF:
    flash_event      = LED_FLASH_SET_OFF;
    flash_ctrl       = CAM_FLASH_CTRL_OFF;
    next_flash_state = CAM_FLASH_STATE_READY;
    break;
  default:
    /* CAM_FLASH_MODE_ON : We don't do anything for flash-always-on command */
    return TRUE;
  }

  /* 0. Check the last flash request.
     If the requsest is same with the previous one,
     then do nothing. */
  SLOW("last flash request %d", s_bundle->last_flash_request);
  if (is_capture == FALSE && mode == s_bundle->last_flash_request) {
    /* Same mode as current */
    SLOW("Same flash mode %d: exit", mode);
    return TRUE;
  }

  s_bundle->last_flash_request = mode;

  /* 1. Ask MCT to delay the next request */
  if (is_capture == FALSE) {
    if (delay_frame_id > cur_sof) {
      ret = sensor_fc_store(s_bundle, delay_frame_id,
        SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ, &flash_ctrl);
      SLOW("1. store SENSOR_FRAME_CTRL_POST_LED_DELAY_REQ delay_frame_id:%d",
        delay_frame_id);
    } else {
      bus_delay_msg.curr_frame_id = cur_sof;
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
      SLOW("1. notify to MCT %d", cur_sof);
    }
  }

  /* Turn off delay API and schedule an action to restore
     after capture is done */
  if (is_capture == TRUE && mode == CAM_FLASH_MODE_SINGLE) {
    ret = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_DELAY_CFG, (void*)&next_delay_state);
    if (ret < 0) {
      SERR("failed to get delay config");
    } else {
      ret = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_DELAY_CFG, (void*)&delay_en);
      if (ret < 0) {
        SERR("failed to set delay config");
      } else {
        ret = sensor_fc_store(s_bundle, apply_frame_id + led_delay + 2,
          SENSOR_FRAME_CTRL_DELAY_CONFIG_CHANGE, &next_delay_state);
        if (ret == FALSE) {
          SERR("Failed to queue flash off info");
        }
      }
    }
  }

  /* 2. Apply the current flash mode/state */
  if (apply_frame_id > cur_sof) {
    SLOW("2. store SENSOR_FRAME_CTRL_LED_FLASH:%d apply_id:%d",
      flash_event, apply_frame_id);
    sensor_util_send_led_mode_downstream(module, &mode,
      identity, s_bundle->last_idx);
    ret = sensor_fc_store(s_bundle, apply_frame_id,
      SENSOR_FRAME_CTRL_LED_FLASH, &flash_event);
    if (ret == FALSE) {
      SERR("Failed to queue flash on info");
      return ret;
    }
  } else if (apply_frame_id == cur_sof) {
    SLOW("2. apply flash");
    /* Fill information for the flash offload to thread */
    sensor_capture_control_t *ctrl =
      &s_bundle->cap_control;
    memset(ctrl, 0, sizeof(sensor_capture_control_t));
    sensor_batch_config_t *bctrl = &ctrl->bctrl[0];
    bctrl->burst_count = 1;
    bctrl->flash_mode = flash_event;
    bctrl->cam_flash_mode = mode;
    bctrl->skip_config_mode = 1;
    s_bundle->batch_idx = 0;

    ret = module_sensor_post_flash_mode_to_thread(module, &identity,
        s_bundle, NULL);

    /* only report PARTIAL with SINGLE/TORCH mode
       instead of OFF/PARTIAL : [SINGLE/TORCH]/PARTIAL */
    if (mode != CAM_FLASH_MODE_OFF)
      s_bundle->flash_params.flash_mode = mode;
    s_bundle->flash_params.flash_state = CAM_FLASH_STATE_PARTIAL;

    ret = sensor_util_send_led_mode_downstream(module, &mode,
      identity, cur_sof);
    if (ret == FALSE) {
      SERR("Failed: sensor_util_send_led_mode_downstream");
      /* Not fatal, continue */
    }
  } else {
    SERR("failed: apply_frame_id %d, cur_sof %d", apply_frame_id, cur_sof);
    return FALSE;
  }

  /* 3. Change state of LED in next of sof of apply_frame_id */
  ret = sensor_fc_store(s_bundle, apply_frame_id + led_delay,
    SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE, &next_flash_state);
  SLOW("3. store SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE:%d apply_id:%d",
    next_flash_state, apply_frame_id + led_delay);
  if (ret == FALSE) {
    SHIGH("Failed: SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE");
    /* Not fatal, continue */
  }

  /* 4. (ONLY capture + flash case) Turn off the flash */
  if (is_capture == TRUE) {
    /* apply_frame_id                 : turning on flash (partial frame)
       apply_frame_id + led_delay     : ok frame (flash is properly applied)
       apply_frame_id + led_delay + 1 : now we can turn off flash */
    apply_frame_id += (led_delay+1);

    flash_event = LED_FLASH_SET_OFF;
    ret = sensor_fc_store(s_bundle, apply_frame_id,
      SENSOR_FRAME_CTRL_LED_FLASH, &flash_event);
    if (!ret) {
      SERR("Failed to queue flash off info");
      return ret;
    }
    /* Change state of LED in next of sof of apply_frame_id */
    next_flash_state = CAM_FLASH_STATE_READY;
    ret = sensor_fc_store(s_bundle, apply_frame_id + 1,
      SENSOR_FRAME_CTRL_CHANGE_FLASH_STATE, &next_flash_state);
    if (!ret) {
      SHIGH("Failed to queue flash off info");
    }
  }

  return TRUE;
}

boolean module_sensor_reload_chromatix_for_ir_mode(
  module_sensor_bundle_info_t *s_bundle,
  mct_module_t *module, uint32_t identity, boolean ir_mode)
{
  modulesChromatix_t        module_chromatix;
  sensor_chromatix_params_t chromatix_params;
  int32_t                   rc = SENSOR_SUCCESS;
  module_sensor_params_t    *chromatix_module_params = NULL;

  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];

  if (ir_mode) {
    sensor_util_set_special_mode(&(s_bundle->sensor_common_info),
      SENSOR_SPECIAL_MODE_IR, 1);
  } else {
    sensor_util_set_special_mode(&(s_bundle->sensor_common_info),
      SENSOR_SPECIAL_MODE_IR, 0);
  }
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
    CHROMATIX_GET_PTR, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0)

  RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(module,
    s_bundle, identity, &chromatix_params, CAMERA_CHROMATIX_MODULE_ALL));
  return TRUE;
}

static boolean module_sensor_send_aec_update_to_slave(
  mct_module_t* module, mct_event_t* event,
  module_sensor_bundle_info_t* s_bundle, aec_update_t *aec_update)
{
  int32_t rc = 0;
  sensor_dual_exp_t dual_exp;
  uint64_t exp_time = 0;
  stats_update_t stats_update;

  dual_exp.stats_update = &stats_update;

  if (aec_update->sensor_gain == 0 || aec_update->linecount == 0) {
    SHIGH("Invalid master exposure parameters");
    return TRUE;
  }

  dual_exp.stats_update->aec_update = *aec_update;

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle,
    SUB_MODULE_SENSOR,
    SENSOR_GET_EXPOSURE_INFO,
    &dual_exp, rc);
  if (rc == SENSOR_FAILURE) {
    SERR("failed");
    return FALSE;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_EXPOSURE_TIME, &exp_time, rc);
  if (rc == SENSOR_FAILURE) {
    SERR("failed");
    return FALSE;
  } else {
    dual_exp.stats_update->aec_update.exp_time =
      ((float)exp_time)/NANO_SEC_PER_SEC;
  }

  /* 3. Send aec update to peer session */
  sensor_util_post_intramode_event(module, event->identity,
    s_bundle->peer_identity,
    MCT_EVENT_MODULE_STATS_AEC_UPDATE, &dual_exp);

  return TRUE;
}

static boolean module_sensor_handle_sof_notify(mct_module_t* module,
  mct_event_t* event, module_sensor_bundle_info_t* s_bundle)
{
  int32_t                      rc = 0, idx = 0;
  module_sensor_params_t      *module_sensor_params;
  module_sensor_params_t      *ois_module_params;
  mct_bus_msg_t                bus_msg;
  uint32_t                     apply_frame_id = 0;
  uint16_t                     skip_count;
  sensor_hdr_meta_t            hdr_meta;
  sensor_thread_msg_t          msg;
  int32_t                      nwrite = 0;
  module_sensor_ctrl_t         *sensor_ctrl;
  ois_read_data_t              ois_data;
  mct_custom_data_payload_t    custom_data;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);
  pthread_mutex_lock(&s_bundle->mutex);
  s_bundle->last_idx = event->u.ctrl_event.current_frame_id;
  pthread_mutex_unlock(&s_bundle->mutex);

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;

  SLOW("identity=0x%x, frame_id=%d", event->identity, s_bundle->last_idx);

  if (module_sensor_capture_control(module, event, s_bundle, FALSE)
    == FALSE) {
    SERR("fatal : sensor flash control");
  }

  if(module_sensor_dualcamera_phase_sync(module,event,s_bundle) < 0)
    SERR("non-fatal: dual phase sync failed");

  if (sensor_fc_post_static_meta(module, s_bundle, event) == FALSE) {
    SERR("failed : sensor_fc_post_static_meta");
    /* It's not critical issue, so go on */
  }

  if (sensor_fc_process(module, s_bundle, event) == FALSE)
    SERR("failed: port_sensor_process_frame_control");

  /* read ois data if ois is enabled */
  if ((s_bundle->ois_enabled == 1) &&
      (s_bundle->sensor_info->subdev_id[SUB_MODULE_OIS] != -1)){

    ois_module_params = s_bundle->module_sensor_params[SUB_MODULE_OIS];

    if (ois_module_params->func_tbl.process != NULL) {
      rc = ois_module_params->func_tbl.process(
        ois_module_params->sub_module_private, OIS_READ_DATA, &ois_data);
      if (rc < 0 || !ois_data.ois_data) {
        SLOW("OIS_READ not enabled");
      }
      else{

        custom_data.data = (void*)ois_data.ois_data;
        custom_data.size = ois_data.size;

        /* send the read data downstream */
        if((sensor_util_post_downstream_event(module, event->identity,
          MCT_EVENT_MODULE_SET_OIS_READ_DATA,(void*) &custom_data)) == FALSE){
          SERR("(non-fatal)Failed to send ois read data downstream");
        }

        /* Post bus message */
        memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
        bus_msg.sessionid = s_bundle->sensor_info->session_id;
        bus_msg.type = MCT_BUS_MSG_OIS_READ_DATA;
        bus_msg.msg = &custom_data;
        bus_msg.size = sizeof(mct_custom_data_payload_t);
        if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
          SERR("(non-fatal)Failed to post MCT_BUS_MSG_OIS_READ_DATA");
      }
    }
  }

  if (s_bundle->ir_mode == SENSOR_IR_MODE_ON) {
    if (s_bundle->ir_mode != s_bundle->previous_ir_mode) {
      sensor_submodule_event_type_t ir_cut_event = IR_CUT_SET_OFF;
      s_bundle->previous_ir_mode = s_bundle->ir_mode;
      if (module_sensor_reload_chromatix_for_ir_mode(s_bundle,
        module, event->identity, s_bundle->ir_mode) == FALSE) {
        SERR("failed: IR mode chromatix reload");
        return FALSE;
      }

      ir_cut_event = IR_CUT_SET_ON;
      if (sensor_fc_store(s_bundle,
        s_bundle->last_idx + 3,
        SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER, &ir_cut_event) == FALSE) {
        SERR("failed: fc_store of SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER");
        return FALSE;
      }

      s_bundle->previous_ir_led_brightness = s_bundle->ir_led_brightness;
      sensor_ir_led_params_t led_params;
      led_params.ir_led_event = IR_LED_SET_ON;
      led_params.ir_led_brightness = s_bundle->ir_led_brightness;
      if (sensor_fc_store(s_bundle,
        s_bundle->last_idx + 3,
        SENSOR_FRAME_CTRL_IR_LED_ON, &led_params) == FALSE) {
        SERR("failed: fc_store of SENSOR_FRAME_CTRL_IR_LED_ON");
        return FALSE;
      }
    } else if (s_bundle->ir_led_brightness !=
        s_bundle->previous_ir_led_brightness &&
        (s_bundle->ir_led_brightness >= 0)) {
        module_sensor_params_t *ir_module_params =
          s_bundle->module_sensor_params[SUB_MODULE_IR_LED];
        if (ir_module_params->func_tbl.process != NULL) {
          sensor_ir_led_cfg_t cfg;
          cfg.intensity = s_bundle->ir_led_brightness;
          rc = ir_module_params->func_tbl.process(
            ir_module_params->sub_module_private, IR_LED_SET_ON, &cfg);
          if (rc < 0) {
            SERR("failed: IR_LED_SET_ON");
            return FALSE;
          }
        }
      }
  }

  if (s_bundle->ir_mode != s_bundle->previous_ir_mode &&
        s_bundle->ir_mode == SENSOR_IR_MODE_OFF) {
    s_bundle->previous_ir_mode = s_bundle->ir_mode;
    s_bundle->previous_ir_led_brightness = -1;
    sensor_submodule_event_type_t led_event = IR_LED_SET_OFF;

    module_sensor_params_t *ir_module_params =
      s_bundle->module_sensor_params[SUB_MODULE_IR_CUT];
    if (ir_module_params->func_tbl.process != NULL) {
      rc = ir_module_params->func_tbl.process(
        ir_module_params->sub_module_private, IR_CUT_SET_OFF, NULL);
      if (rc < 0) {
        SERR("failed: IR_CUT_SET_OFF");
        return FALSE;
      }
    }

    led_event = IR_LED_SET_OFF;
    sensor_ir_led_params_t led_params;
    led_params.ir_led_event = led_event;
    led_params.ir_led_brightness = s_bundle->ir_led_brightness;

    if (sensor_fc_store(s_bundle,
      s_bundle->last_idx + 3,
      SENSOR_FRAME_CTRL_IR_LED_ON, &led_params) == FALSE) {
      SERR("failed: fc_store of SENSOR_FRAME_CTRL_IR_LED_ON");
      return FALSE;
    }

    if (sensor_fc_store(s_bundle,
      s_bundle->last_idx + 3,
      SENSOR_FRAME_CTRL_IR_CHROMATIX_RELOAD, &s_bundle->ir_mode) == FALSE) {
      SERR("failed: fc_store of SENSOR_FRAME_CTRL_IR_CHROMATIX_RELOAD");
      return FALSE;
    }
  }

  /* SENSOR_SET_VFE_SOF */
  memset(&hdr_meta, 0, sizeof(sensor_hdr_meta_t));
  hdr_meta.current_sof = s_bundle->last_idx;
  hdr_meta.isp_frame_skip = s_bundle->max_isp_frame_skip;
  hdr_meta.aec_index = -1;
  RETURN_ON_FALSE(sensor_util_post_submod_event(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_SET_VFE_SOF, &hdr_meta));

  /* update af bracket entry upon each SOF */
  if (s_bundle->af_bracket_params.ctrl.enable == TRUE) {
    if(!module_sensor_update_af_bracket_entry(module, s_bundle, event)) {
      SERR("Fail to update af bracket entry");
      return FALSE;
    }
  }
  /* update flash bracket entry upon each SOF */
  sensor_bracket_ctrl_t * flash_ctrl = &(s_bundle->flash_bracket_params.ctrl);
  if (flash_ctrl->enable == TRUE) {
    /* update flash brackets */
    if(!module_sensor_update_flash_bracket_entry(module, s_bundle, event)) {
      SERR("Fail to update Flash bracket entry");
      return FALSE;
     }
  }
  if (hdr_meta.aec_index != -1) {
    uint32_t sensor_delay;
    uint32_t report_delay;
    sensor_aec_update_t  aec_update;
    aec_update_t aec_update_s;
    sensor_ae_bracket_t *ae_bracket_params =
      &s_bundle->sensor_common_info.ae_bracket_params;
    memset(&aec_update_s, 0, sizeof(aec_update_t));

    sensor_delay = s_bundle->frame_ctrl.max_sensor_delay;
    report_delay = s_bundle->frame_ctrl.session_data.
      max_pipeline_meta_reporting_delay;
    SLOW("hdr_meta.aec_index = %d", hdr_meta.aec_index);
    aec_update.linecount = aec_update_s.linecount =
      ae_bracket_params->linecount[hdr_meta.aec_index];
    aec_update.real_gain = aec_update_s.sensor_gain =
      ae_bracket_params->real_gain[hdr_meta.aec_index];
    aec_update.lux_idx = ae_bracket_params->lux_idx;
    aec_update.est_state = 0;
    SLOW("[store meta] SENSOR_FRAME_CTRL_EXP_META future sof %d",
      s_bundle->last_idx + sensor_delay + report_delay);
    if (sensor_fc_store(s_bundle,
      s_bundle->last_idx + sensor_delay + report_delay,
      SENSOR_FRAME_CTRL_EXP_META, &aec_update) == FALSE)
      SERR("failed");

    if (sensor_ctrl->is_dual_cam == TRUE &&
      s_bundle->dualc_info.fps_match_enabled == TRUE &&
      s_bundle->dualc_info.streaming_master == CAM_MODE_PRIMARY) {
      module_sensor_send_aec_update_to_slave(module, event,
        s_bundle, &aec_update_s);
    }

  }

  if (hdr_meta.post_meta_bus) {
    cam_frame_idx_range_t range;
    uint32_t sensor_pipeline_delay = s_bundle->
      frame_ctrl.max_sensor_delay;

    memset(&range, 0, sizeof(cam_frame_idx_range_t));

    if (sensor_pipeline_delay)
      range.min_frame_idx = s_bundle->last_idx + MAXQ(sensor_pipeline_delay,
      s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_num_HDR_frame_skip);
    else
      range.min_frame_idx = s_bundle->last_idx+1;
    range.max_frame_idx = s_bundle->last_idx+10;
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = s_bundle->sensor_info->session_id;
    bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
    bus_msg.msg = &range;
    bus_msg.size = sizeof(cam_frame_idx_range_t);
    if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
      SERR("Failed to send ae bkt frame idx");
  }

  /* Send digital gain to ISP. */
  sensor_util_set_digital_gain_to_isp(module, s_bundle, event->identity, NULL);

  /* Logic for flash burst mode. */
  SLOW("led_off_count: %d", s_bundle->led_off_count);
  /* Switch off LED if not longshot mode */
  if((s_bundle->led_off_count >= 0) && !s_bundle->longshot) {
      /* Turn off LED and apply led off gain and linecount*/
      if(s_bundle->led_off_count == 0) {

          RETURN_ON_FALSE(sensor_util_post_submod_event(s_bundle, SUB_MODULE_LED_FLASH,
            LED_FLASH_SET_OFF, NULL));

          aec_update_t aec_update;
          aec_update.sensor_gain= s_bundle->led_off_gain;
          aec_update.linecount = s_bundle->led_off_linecount;

          SLOW("led_off stats gain %f lnct %d", aec_update.sensor_gain,
              aec_update.linecount);

          RETURN_ON_FALSE(sensor_util_post_submod_event(s_bundle, SUB_MODULE_SENSOR,
            SENSOR_SET_AEC_UPDATE, &aec_update));

          if (sensor_ctrl->is_dual_cam == TRUE &&
            s_bundle->dualc_info.fps_match_enabled == TRUE &&
            s_bundle->dualc_info.streaming_master == CAM_MODE_PRIMARY) {
            module_sensor_send_aec_update_to_slave(module, event,
              s_bundle, &aec_update);
          }
      }
      s_bundle->led_off_count--;
  }

  if((s_bundle->hal_version == CAM_HAL_V1) &&
    (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))) {
    s_bundle->sensor_params.flash_state = s_bundle->flash_params.flash_state;
    s_bundle->sensor_params.flash_mode = s_bundle->flash_params.flash_mode;
  }
  /* port sensor params to bus if actuator is present.*/
  sensor_util_post_bus_sensor_params(module, s_bundle, event->identity);

  /* dynamic META reporting */
  if (s_bundle->is_bayer)
    if (sensor_fc_report_meta(module, s_bundle, event) == FALSE)
      SERR("failed:sensor_fc_report_meta");

   /*Queue the OIS initialization if it is not done already */
   if ((s_bundle->ois_cmd_queue_mask & OIS_INIT_QUEUE_MASK) == 0) {

    s_bundle->ois_cmd_queue_mask |= OIS_INIT_QUEUE_MASK;

    /* offload ois_init and calibrate to sensor thread */
    msg.msgtype = OFFLOAD_FUNC;
    msg.offload_func = module_sensor_offload_ois_init_calibrate;
    msg.param1 = s_bundle;
    msg.param2 = NULL;
    msg.param3 = NULL;
    msg.param4 = NULL;
    msg.stop_thread = FALSE;
    nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
    if (nwrite < 0) {
      SERR("%s: OIS init_calibrate: Writing into fd failed:",__func__);
    }
  }

  return TRUE;
}

static boolean module_sensor_event_control_set_super_parm(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle) {
  boolean                         ret   = TRUE;
  mct_event_super_control_parm_t  *param = NULL;
  uint32_t                        index = 0;
  uint32_t                        frame_id = 0;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);

  param =
    (mct_event_super_control_parm_t *)event->u.module_event.module_event_data;
  RETURN_ON_NULL(param);

  frame_id = event->u.module_event.current_frame_id;

  pthread_mutex_lock(&s_bundle->mutex);
  s_bundle->last_idx = frame_id;
  pthread_mutex_unlock(&s_bundle->mutex);

  /* Handle all set params */
  for (index = 0; index < param->num_of_parm_events; index++) {
    module_sensor_event_control_set_parm(module,
      frame_id, s_bundle, &param->parm_events[index],event->identity);
  }

  return ret;
}

static boolean module_sensor_pass_op_clk_change(void *data1, void *data2)
{
  int32_t                     rc = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data1;

  if (!s_bundle || !data2) {
    SERR("failed: s_bundle %p data2 %p", s_bundle, data2);
    /* Return TRUE here, else mct_list_traverse will terminate */
    return TRUE;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_SET_OP_PIXEL_CLK_CHANGE, data2, rc);
  if (rc < 0) {
    SERR("failed");
  }

  return TRUE;
}

/** module_sensor_handle_pixel_clk_change: handle pixel clk
 *  change event sent by ISP
 *
 *  @module: sensor module
 *  @data: event control data
 *
 *  This function handles stores op pixel clk value in module
 *  private
 *
 *  Return: TRUE for success and FALSE for failure
 *  **/

boolean module_sensor_handle_pixel_clk_change(mct_module_t *module,
  uint32_t identity __attribute__((unused)), void *data)
{
  boolean                     ret = TRUE;
  module_sensor_ctrl_t        *module_ctrl = NULL;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(data);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  mct_list_traverse(module_ctrl->sensor_bundle,
    module_sensor_pass_op_clk_change, data);

  return ret;
}

/** module_sensor_update_settings_size: Update the setting size in sensor library
 *
 *  @sensor_lib_ptr: pointer to sensor library
 *
 *  return TRUE if success else FALSE
 **/
static boolean module_sensor_update_settings_size(
  sensor_lib_t *sensor_lib_ptr)
{
  uint32_t i = 0;
  int32_t size = 0;
  int32_t max_size = I2C_REG_SET_MAX;

  /* Update size for start_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->start_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->start_settings.size = size;
  SLOW("start_settings.size = %d", sensor_lib_ptr->start_settings.size);

  /* Update size for stop_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->stop_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->stop_settings.size = size;
  SLOW("stop_settings.size = %d", sensor_lib_ptr->stop_settings.size);

  /* Update size for groupon_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->groupon_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->groupon_settings.size = size;
  SLOW("groupon_settings.size = %d", sensor_lib_ptr->groupon_settings.size);

  /* Update size for groupoff_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->groupoff_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->groupoff_settings.size = size;
  SLOW("groupoff_settings.size = %d", sensor_lib_ptr->groupoff_settings.size);

  /* Update size for embedded_data_enable_settings */
  size = sensor_util_get_setting_size(
  sensor_lib_ptr->embedded_data_enable_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->embedded_data_enable_settings.size = size;
  SLOW("embedded_data_enable_settings.size = %d",
    sensor_lib_ptr->embedded_data_enable_settings.size);

  /* Update size for embedded_data_disable_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->embedded_data_disable_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->embedded_data_disable_settings.size = size;
  SLOW("embedded_data_disable_settings.size = %d",
    sensor_lib_ptr->embedded_data_disable_settings.size);

  /* Update size for aec_enable_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->aec_enable_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->aec_enable_settings.size = size;
  SLOW("aec_enable_settings.size = %d",
    sensor_lib_ptr->aec_enable_settings.size);

  /* Update size for master_sync_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->dualcam_master_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->dualcam_master_settings.size = size;
  SLOW("dualcam_master_settings.size = %d",
    sensor_lib_ptr->dualcam_master_settings.size);

  /* Update size for slave_sync_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->dualcam_slave_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->dualcam_slave_settings.size = size;
  SLOW("dualcam_slave_settings.size = %d",
    sensor_lib_ptr->dualcam_slave_settings.size);

  /* Update size for aec_disable_settings */
  size = sensor_util_get_setting_size(
    sensor_lib_ptr->aec_disable_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  sensor_lib_ptr->aec_disable_settings.size = size;
  SLOW("aec_disable_settings.size = %d",
    sensor_lib_ptr->aec_disable_settings.size);

  /* Update size for test_pattern settings */
  for (i = 0; i < sensor_lib_ptr->test_pattern_info.size; i++) {
    size = sensor_util_get_setting_size(sensor_lib_ptr->test_pattern_info.
      test_pattern_settings[i].settings.reg_setting_a, max_size);
    RETURN_FALSE_IF(size < 0);
    sensor_lib_ptr->test_pattern_info.test_pattern_settings[i].settings.size =
      size;
    SLOW("test_pattern_settings[%d].size = %d", i,
      sensor_lib_ptr->test_pattern_info.test_pattern_settings[i].settings.size);
  }

  max_size = MAX_SENSOR_SETTING_I2C_REG;

  /* Update size for init_settings */
  for (i = 0; i < sensor_lib_ptr->init_settings_array.size; i++) {
    size = sensor_util_get_setting_size(
      sensor_lib_ptr->init_settings_array.reg_settings[i].reg_setting_a,
      max_size);
    RETURN_FALSE_IF(size < 0);
    sensor_lib_ptr->init_settings_array.reg_settings[i].size = size;
    SLOW("init_settings_array[%d].size = %d", i,
      sensor_lib_ptr->init_settings_array.reg_settings[i].size);
  }

  /* Update size for resolution_settings */
  for (i = 0; i < sensor_lib_ptr->res_settings_array.size; i++) {
    size = sensor_util_get_setting_size(
      sensor_lib_ptr->res_settings_array.reg_settings[i].reg_setting_a,
      max_size);
    RETURN_FALSE_IF(size < 0);
    sensor_lib_ptr->res_settings_array.reg_settings[i].size = size;
    SLOW("res_settings_array[%d].size = %d", i,
      sensor_lib_ptr->res_settings_array.reg_settings[i].size);
  }

  return TRUE;
}

/** module_sensors_subinit: sensor module init function
 *
 *  @data: sensor bundle data for first sensor
 *  @user_data: NULL
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function allocates memory to hold module_sensor_prams_t struct for each
 *  sub module and calls init to initialize function table **/

static boolean module_sensors_subinit(void *data,
  void *user_data __attribute__((unused)))
{
  int32_t rc = SENSOR_SUCCESS, i = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  sensor_submodule_intf_info_t *intf_info = NULL;
  RETURN_ON_NULL(s_bundle);

  for (i = 0; i < SUB_MODULE_MAX; i++) {
    s_bundle->module_sensor_params[i] = malloc(sizeof(module_sensor_params_t));
    JUMP_ON_NULL(s_bundle->module_sensor_params[i], ERROR);
    intf_info = &s_bundle->subdev_info[i].intf_info[SUBDEV_INTF_PRIMARY];
    memset(s_bundle->module_sensor_params[i], 0,
      sizeof(module_sensor_params_t));
    if (s_bundle->sensor_info->subdev_id[i] != -1) {
      SLOW("i %d subdev name %s strlen %d", i, intf_info->sensor_sd_name,
        strlen(intf_info->sensor_sd_name));
      if (!strlen(intf_info->sensor_sd_name) &&
         ((i == SUB_MODULE_ACTUATOR) || (i == SUB_MODULE_EEPROM) ||
          (i == SUB_MODULE_LED_FLASH) || (i == SUB_MODULE_STROBE_FLASH) ||
          (i == SUB_MODULE_OIS) || (i == SUB_MODULE_IR_LED) ||
          (i == SUB_MODULE_IR_CUT) || (i == SUB_MODULE_LASER_LED))) {
        SLOW("session %d subdev %d not present, reset to -1",
          s_bundle->sensor_info->session_id, i);
        s_bundle->sensor_info->subdev_id[i] = -1;
      } else {
        rc = sub_module_init[i](&s_bundle->module_sensor_params[i]->func_tbl);
        if (rc < 0 || !s_bundle->module_sensor_params[i]->func_tbl.open ||
            !s_bundle->module_sensor_params[i]->func_tbl.process ||
            !s_bundle->module_sensor_params[i]->func_tbl.close) {
          SERR("failed");
          goto ERROR;
        }
        /* sub_mod_open_flag is used to split the subdev open into subgroup &
         * parallelize. Default value is 0 for all subdev. For blocks internal to msm and
         * which can be powered up in parallel to external, value is 1 */

        switch(i) {
        case SUB_MODULE_SENSOR:
          s_bundle->subdev_info[i].data = &(s_bundle->sensor_common_info);
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_CHROMATIX:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_ACTUATOR:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_EEPROM:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_LED_FLASH:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_STROBE_FLASH:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_CSID:
          s_bundle->subdev_info[i].sub_mod_open_flag = 1;
          break;
        case SUB_MODULE_CSID_3D:
          s_bundle->subdev_info[i].sub_mod_open_flag = 1;
          break;
        case SUB_MODULE_CSIPHY:
          s_bundle->subdev_info[i].sub_mod_open_flag = 1;
          break;
        case SUB_MODULE_CSIPHY_3D:
          s_bundle->subdev_info[i].sub_mod_open_flag = 1;
          break;
        case SUB_MODULE_OIS:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_EXT:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_IR_LED:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_IR_CUT:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        case SUB_MODULE_LASER_LED:
          s_bundle->subdev_info[i].sub_mod_open_flag = 0;
          break;
        }
      }
    }
  }

  if (module_sensor_update_settings_size(
    s_bundle->sensor_common_info.sensor_lib_params->sensor_lib_ptr) == FALSE) {
    SERR("module_sensor_update_settings_size failed");
    goto ERROR;
  }

  return TRUE;

ERROR:
  for (i--; i >= 0; i--)
    free(s_bundle->module_sensor_params[i]);
  SERR("failed");
  return FALSE;
}

/** module_sensor_frame_ctrl_release: Release the queued
 *  frame control data
 *
 *  @data: frame control data pointer
 *  @user_data: NULL
 *
 *  Return: TRUE for success and FALSE for failure
 *
 *  This function is called during deinit. It called for each node in
 *  the queue and frees all the memory associated with frame control
 *  parameters**/
static boolean module_sensor_frame_ctrl_release(void *data,
  void *user_data __attribute__((unused)))
{
  sensor_frame_ctrl_data_t *sensor_frame_ctrl_data =
    (sensor_frame_ctrl_data_t *)data;
  if (sensor_frame_ctrl_data) {
    free(sensor_frame_ctrl_data);
    sensor_frame_ctrl_data = NULL;
  }
  return TRUE;
}

boolean module_sensor_actuator_init_calibrate(
    module_sensor_bundle_info_t *s_bundle)
{
    int32_t                  rc = 0;
    char                     *a_name = NULL;
    sensor_get_af_algo_ptr_t af_algo;
    eeprom_set_chroma_af_t   eeprom_set;
    af_algo_tune_parms_t     *af_algo_cam_ptr = NULL;
    actuator_driver_params_t *af_driver_ptr = NULL;

    /* Get the actuator name from camera config read during daemon init */
    a_name = s_bundle->sensor_common_info.camera_config.actuator_name;
    SLOW("Actuator init and calibrate for %s", a_name);

    /* Initialize the actuator */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_INIT, a_name, rc);
    if (rc < 0) {
        SERR("sensor_failure : ACTUATOR_INIT failed");
        return FALSE;
    }

    /* Get diver param from actuator */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
        ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr, rc);
    if (rc < 0 || af_driver_ptr == NULL) {
        SERR("sensor_failure : ACTUATOR_GET_AF_DRIVER_PARAM_PTR failed");
        return FALSE;
    }

    /* Set driver param to eeprom */
    eeprom_set.af_driver_ptr = af_driver_ptr;

    /* Perform calibration if eeprom is present */
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1)
    {
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
            EEPROM_CALIBRATE_FOCUS_DATA, &eeprom_set, rc);
        if (rc < 0) {
            SERR("sensor_failure : EEPROM_SET_CALIBRATE_FOCUS failed");
            return FALSE;
        }
        /* calcualte actuator sensitivity
           assuming total_steps is tuned to number of um */
        s_bundle->actuator_sensitivity =
          (float)af_driver_ptr->actuator_tuned_params.region_params[0].qvalue /
          af_driver_ptr->actuator_tuned_params.region_params[0].code_per_step;
    }
    /* protect the qvalue */
    else {
      af_driver_ptr->actuator_tuned_params.region_params[0].qvalue = 1;
    }

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
      ACTUATOR_SET_PARAMETERS, NULL, rc);
    if (rc < 0) {
      SERR("sensor_failure : ACTUATOR_SET_PARAMETERS failed");
      return FALSE;
    }

    return TRUE;
}

boolean module_sensor_flash_init(
  module_sensor_bundle_info_t *s_bundle)
{
  int32_t rc = 0;
  char    *a_name = NULL;

  /* Get flash name from camera config read from daemon init */
  a_name = s_bundle->sensor_common_info.camera_config.flash_name;

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
      LED_FLASH_INIT, a_name, rc);
  if (rc < 0) {
      SERR("sensor_failure : LED_FLASH_INIT failed");
      return FALSE;
  }

  return TRUE;
}
/** module_sensor_load_external_libs: Load External libraries
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *
 *  Return: TRUE for success and FALSE for failure
 *
 **/
boolean module_sensor_load_external_libs(
  module_sensor_bundle_info_t *s_bundle)
{
  /* Validate input parameters */
  RETURN_ON_NULL(s_bundle);

  return TRUE;
}

/** module_sensor_unload_external_libs: Unload external libraries
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *
 *  Return: TRUE for success and FALSE for failure
 *
 **/
static boolean module_sensor_unload_external_libs(
  module_sensor_bundle_info_t *s_bundle)
{
  /* Validate input parameters */
  RETURN_ON_NULL(s_bundle);

  return TRUE;
}

/** module_sensor_init_session: init session function for sensor
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *  @eebin_hdl: pointing to eeprom bin data
 *
 *  Return: 0 for success and negative error for failure
 *
 *  When called first time, this function
 *  1) opens all sub modules to open subdev node
 *  2) loads sensor library
 *  3) calls init on sensor, csiphy and csid. Has ref count to
 *  ensure that actual add stream sequence is executed only
 *  once **/

static boolean module_sensor_init_session(
  module_sensor_bundle_info_t *s_bundle, void *eebin_hdl)
{
  int32_t                 rc = SENSOR_SUCCESS, i = 0, j = 0, k = 0;
  module_sensor_params_t  *module_sensor_params = NULL;
  module_sensor_params_t  *actuator_module_params = NULL;
  module_sensor_params_t  *csiphy_module_params = NULL;
  module_sensor_params_t  *csid_module_params = NULL;
  module_sensor_params_t  *eeprom_module_params = NULL;
  module_sensor_params_t  *flash_module_params = NULL;
  sensor_get_t             sensor_get;
  sensor_property_t        sensor_property;
  eebin_ctl_t              bin_ctl;
  enum sensor_sub_module_t s_module;
  af_algo_tune_parms_t     *af_algo_cam_ptr = NULL;
  af_algo_tune_parms_t     *af_algo_camcorder_ptr = NULL;
  actuator_driver_params_t *af_driver_ptr = NULL;
  boolean                   status = TRUE;
  sensor_submodule_intf_info_t *intf_info = NULL;
  char                      prop[PROPERTY_VALUE_MAX];
  int32_t                   eztune_enable;
  sensor_thread_msg_t msg;
  int32_t nwrite = 0;
  struct timespec ts;

  if (s_bundle->ref_count++) {
    SLOW("ref_count %d", s_bundle->ref_count);
    return TRUE;
  }

  if (s_bundle->close_pending) {
    TIMED_WAIT_ON_EVENT(s_bundle->mutex, s_bundle->cond,
          ts, CLOSE_TIMEOUT, s_bundle->close_done,
          FALSE, "close_done");
  }

  /* Initialize max width, height and stream on count */
  s_bundle->peer_identity = 0;
  s_bundle->max_width = 0;
  s_bundle->max_height = 0;
  s_bundle->stream_on_count = 0;
  s_bundle->stream_mask = 0;
  s_bundle->last_idx = 0;
  s_bundle->delay_frame_idx = 0;
  s_bundle->num_skip = 4;
  s_bundle->state = 0;
  s_bundle->block_parm = 0;
  s_bundle->regular_led_trigger = 0;
  s_bundle->main_flash_on_frame_skip = 0;
  s_bundle->main_flash_off_frame_skip = 0;
  s_bundle->torch_on_frame_skip = 0;
  s_bundle->torch_off_frame_skip = 0;
  s_bundle->last_flash_request = CAM_FLASH_MODE_OFF;
  s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_OFF;
  s_bundle->flash_params.flash_state = CAM_FLASH_STATE_READY;
  s_bundle->torch_on = 0;
  s_bundle->longshot = 0;
  s_bundle->led_off_count = -1;
  s_bundle->flash_rer_enable = 0;
  s_bundle->ois_cmd_queue_mask = 0x0000;
  s_bundle->ois_enabled = 0;
  s_bundle->ois_mode = OIS_DISABLE;
  s_bundle->hal_params.test_pattern_mode = CAM_TEST_PATTERN_OFF;
  s_bundle->stream_thread_wait_time = 5;
  s_bundle->actuator_sensitivity = 0;
  s_bundle->ir_mode = SENSOR_IR_MODE_OFF;
  s_bundle->retry_frame_skip = 0;
  s_bundle->init_config_done = 0;
  s_bundle->open_done = 0;
  s_bundle->is_plain16 = 0;
  s_bundle->close_done = 0;
  s_bundle->delay_frame_cnt = 0;
  s_bundle->cur_scene_mode = -1;
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.partial.skip", prop, "1");
  s_bundle->partial_flash_frame_skip  = atoi(prop);
  s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
  s_bundle->is_first_stream_on = 1;
  s_bundle->is_camera_closing = 0;

  memset(s_bundle->isp_frameskip, 0, sizeof(s_bundle->isp_frameskip));
  memset(s_bundle->identity, 0, sizeof(s_bundle->identity));
  memset(&s_bundle->cap_control, 0, sizeof(sensor_capture_control_t));
  memset(&s_bundle->hal_frame_batch, 0, sizeof(cam_capture_frame_config_t));
  memset(&s_bundle->dualc_info, 0, sizeof(sensor_dualc_info));

  /* initialize condition variable */
  pthread_condattr_init(&s_bundle->fast_aec_condattr);
  pthread_condattr_setclock(&s_bundle->fast_aec_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&s_bundle->fast_aec_cond, &s_bundle->fast_aec_condattr);
  /* initialize mutex */
  pthread_mutex_init(&s_bundle->fast_aec_mutex, NULL);

  /* initialize flash control mutex */
  pthread_mutex_init(&s_bundle->capture_control_mutex, NULL);

  /* Initialize frame control queues*/
  for (j = 0; j < FRAME_CTRL_SIZE; j++) {
    s_bundle->frame_ctrl.frame_ctrl_q[j] =
      (mct_queue_t*)CAM_CALLOC(1, sizeof(mct_queue_t));
    JUMP_ON_NULL(s_bundle->frame_ctrl.frame_ctrl_q[j], ERROR0);
    mct_queue_init(s_bundle->frame_ctrl.frame_ctrl_q[j]);
    /* Initialize frame control mutex */
    pthread_mutex_init(&s_bundle->frame_ctrl.frame_ctrl_mutex[j], NULL);
  }

  for (i = 0; i < SUB_MODULE_MAX; i++) {
    intf_info = &s_bundle->subdev_info[i].intf_info[SUBDEV_INTF_PRIMARY];
    SLOW("primary sd index = %d, id = %d subdev name %s,",
        i, intf_info->subdev_id, intf_info->sensor_sd_name);
    intf_info = &s_bundle->subdev_info[i].intf_info[SUBDEV_INTF_SECONDARY];
    SLOW("secondary sd index %d subdev name %s, secondary id = %d",
        i, intf_info->sensor_sd_name, s_bundle->sensor_info->subdev_intf[i]);
  }

  /* eeprom control is already created at cam daemon init */
  s_bundle->module_sensor_params[SUB_MODULE_EEPROM]->sub_module_private =
    s_bundle->eeprom_data;

  /* offload open  to sensor thread */
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_open;
  msg.param1 = s_bundle;
  msg.param2 = NULL;
  msg.param3 = NULL;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if (nwrite < 0) {
    SERR("%s: offload open: Writing into fd failed:", __func__);
    module_sensor_offload_open(s_bundle, eebin_hdl, NULL, NULL);
    if (s_bundle->open_done < 0)
      goto ERROR1;
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SD_OPEN);
  for (i = 0; i < SUB_MODULE_MAX; i++) {
    if (s_bundle->module_sensor_params[i]->func_tbl.open
         && !s_bundle->subdev_info[i].sub_mod_open_flag) {
      rc = s_bundle->module_sensor_params[i]->func_tbl.open(
        &s_bundle->module_sensor_params[i]->sub_module_private,
        &s_bundle->subdev_info[i]);
      if (rc < 0) {
        SERR("failed rc %d", rc);
        break;
      }
    }
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SD_OPEN);

  TIMED_WAIT_ON_EVENT(s_bundle->mutex, s_bundle->cond,
              ts, OPEN_TIMEOUT, s_bundle->open_done,
              TRUE, "open_done");

  /* register chromatix manager to chromatix sub module */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
    CHROMATIX_SET_CM, &s_bundle->chromatix_manager, rc);

  for (k = 0; k < (int32_t)ARRAY_SIZE(eebin_load); k++) {
    s_module = eebin_load[k].s_module;
    if (s_bundle->module_sensor_params[s_module]->func_tbl.process) {
      rc = s_bundle->module_sensor_params[s_module]->func_tbl.process(
        s_bundle->module_sensor_params[s_module]->sub_module_private,
        eebin_load[k].p_event, eebin_hdl);
      if (rc < 0) {
        SERR("failed rc %d", rc);
        goto ERROR1;
      }
    }
  }

  /* reload all chromatix for each session only for eztune */
  property_get("persist.vendor.camera.eztune.enable", prop, "0");
  eztune_enable = atoi(prop);
  SLOW("eztune enabled %d", eztune_enable);
  if (eztune_enable) {
    /* clear all the present chromatix before */
    cm_destroy(&s_bundle->chromatix_manager);
    /* reload chromatix files */
    status = module_sensor_init_chromatix(s_bundle,eebin_hdl);
    if (status == FALSE) {
      SERR("module_sensor_init_chromatix failed for eztune.");
      goto ERROR1;
    }
  }

  /* Load external libraries */
  if (s_bundle->sensor_lib_params->sensor_lib_ptr->external_library) {
    status = module_sensor_load_external_libs(s_bundle);
    if (status == FALSE) {
      SERR("failed: module_sensor_load_external_libs");
      goto ERROR1;
    }
  }

  /* offload init config to sensor thread */
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_init_config;
  msg.param1 = s_bundle;
  msg.param2 = eebin_hdl;
  msg.param3 = NULL;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if (nwrite < 0) {
    SERR("%s: offload init config: Writing into fd failed:",__func__);
    module_sensor_offload_init_config(s_bundle, eebin_hdl, NULL, NULL);
    if (s_bundle->init_config_done < 0)
      goto ERROR1;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_SET_FORMATTED_CAL_DATA, s_bundle->formatted_data, rc);

  if (s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_output.is_depth_sensor) {
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LASER_LED,
      LASER_LED_INIT, NULL, rc);
  }

  SHIGH("Success : sensor_name:%s",
   s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_slave_info.sensor_name);
  return TRUE;

ERROR1:
  for (i = SUB_MODULE_MAX-1; i >= 0; i--) {
    if (s_bundle->module_sensor_params[i]->func_tbl.close &&
      s_bundle->module_sensor_params[i]->sub_module_private) {
        s_bundle->module_sensor_params[i]->func_tbl.close(
          s_bundle->module_sensor_params[i]->sub_module_private);
      s_bundle->module_sensor_params[i]->sub_module_private = NULL;
    }
  }
ERROR0:
  s_bundle->ref_count--;
  for (j--; j >= 0; j--) {
    /* Free the frame control queue*/
    mct_queue_free(s_bundle->frame_ctrl.frame_ctrl_q[j]);
    s_bundle->frame_ctrl.frame_ctrl_q[j] = NULL;
    /* Destroy frame control mutex */
    pthread_mutex_destroy(&s_bundle->frame_ctrl.frame_ctrl_mutex[j]);
  }
  SERR("failed");
  return FALSE;
}

/** module_sensor_deinit_session: deinit session function for
 *  sensor
 *
 *  @s_bundle: sensor bundle pointer pointing to the sensor
 *             for which stream is added
 *  @port: port
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function calls close on all sub modules to close
 *  subdev node. Also, unloads sensor library. Has ref count
 *  to ensure that actual close happens only when last stream
 *  is removed */

static boolean module_sensor_deinit_session(
  module_sensor_bundle_info_t *s_bundle)
{
  int16_t              i = 0;
  sensor_thread_msg_t  msg;
  int32_t              nwrite = 0;
  struct timespec      ts;

  s_bundle->ois_cmd_queue_mask = 0x0000;

  if (!s_bundle->ref_count) {
    SERR("ref count 0");
    return FALSE;
  }

  if (--s_bundle->ref_count) {
    SLOW("ref_count %d", s_bundle->ref_count);
    return TRUE;
  }

  TIMED_WAIT_ON_EVENT(s_bundle->mutex, s_bundle->cond,
            ts, INIT_CONFIG_TIMEOUT, s_bundle->init_config_done,
            FALSE, "init_config");

  s_bundle->close_pending = 1;

  /* offload close to sensor thread */
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_close;
  msg.param1 = s_bundle;
  msg.param2 = NULL;
  msg.param3 = NULL;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if (nwrite < 0) {
    SERR("%s: offload close: Writing into fd failed:", __func__);
    module_sensor_offload_close(s_bundle, NULL, NULL, NULL);
  }

  s_bundle->flash_params.flash_mode = CAM_FLASH_MODE_OFF;

  /* Destroy Fast AEC mutex and condition variable*/
  pthread_cond_destroy(&s_bundle->fast_aec_cond);
  pthread_condattr_destroy(&s_bundle->fast_aec_condattr);
  pthread_mutex_destroy(&s_bundle->fast_aec_mutex);

  /* Destroy flash control mutex */
  pthread_mutex_destroy(&s_bundle->capture_control_mutex);
  for (i = 0; i < FRAME_CTRL_SIZE; i++) {
    pthread_mutex_lock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
    /* Flush Frame Control queue*/
    mct_queue_flush(s_bundle->frame_ctrl.frame_ctrl_q[i],
      module_sensor_frame_ctrl_release);
    pthread_mutex_unlock(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
    /* Free frame control queue */
    mct_queue_free(s_bundle->frame_ctrl.frame_ctrl_q[i]);
    s_bundle->frame_ctrl.frame_ctrl_q[i] = NULL;
    /* Destroy frame control mutex */
    pthread_mutex_destroy(&s_bundle->frame_ctrl.frame_ctrl_mutex[i]);
  }
  s_bundle->is_plain16 = 0;
  return TRUE;
}

/** module_sensor_start_session:
 *
 *  @module: sensor module
 *  @sessionid: session id
 *
 *  Return: 0 for success and negative error on failure
 **/
static boolean module_sensor_start_session(
  mct_module_t *module, uint32_t sessionid)
{
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  boolean                     ret = TRUE;

  SHIGH("sessionid %d", sessionid);
  RETURN_ON_NULL(module);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_START_SESSION);

  /* get the s_bundle from session id */
  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  RETURN_ON_NULL(s_list);

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  RETURN_ON_NULL(s_bundle);

  /* initialize the "torch on" flag to 0 */
  s_bundle->torch_on = 0;
  s_bundle->longshot = 0;
  s_bundle->ir_mode = SENSOR_IR_MODE_OFF;

  /* create sensor thread */
  ret = sensor_thread_create(s_bundle);
  if (ret == FALSE) {
    SERR("failed to create sensor thread");
    goto ERROR;
  }

  PTHREAD_MUTEX_LOCK(&module_ctrl->dual_cam_mutex);
  module_ctrl->is_dual_cam = FALSE;
  module_ctrl->is_dual_stream_on = FALSE;
  module_ctrl->is_dual_streaming = FALSE;
  PTHREAD_MUTEX_UNLOCK(&module_ctrl->dual_cam_mutex);

  /* Reset phase correction parameters when session starts */
  module_sensor_dualcamera_reset_pc_params(s_bundle);


  /* this init session includes
     power up sensor, config init setting */
  ret = module_sensor_init_session(s_bundle, module_ctrl->eebin_hdl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR;
  }

  /* Power up peer sensor if in stereo mode. */
  if (TRUE == s_bundle->is_stereo_configuration &&
    s_bundle->stereo_peer_s_bundle != NULL) {
    SERR("Powering up peer stereo sensor. ");

    ret = module_sensor_init_session(s_bundle->stereo_peer_s_bundle,
        module_ctrl->eebin_hdl);
    if (ret == FALSE) {
      SERR("failed");
      goto ERROR;
    }
  }


  SHIGH("SUCCESS");
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_START_SESSION);
  return TRUE;
ERROR:
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_START_SESSION);
  SERR("failed");
  return FALSE;
}

/** module_sensor_stop_session:
 *
 *  @module: sensor module
 *  @sessionid: session id
 *
 *  Return: 0 for success and negative error on failure
 **/
static boolean module_sensor_stop_session(
  mct_module_t *module, uint32_t sessionid)
{
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  boolean                     ret = TRUE;
  camif_cap_t*                camif_data_fmt = NULL;

  SHIGH("sessionid %d", sessionid);
  RETURN_ON_NULL(module);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);
  ATRACE_BEGIN_SNPRINTF(35, "SENSOR_STOP_SESSION sesson id %d", sessionid);

  /* get the s_bundle from session id */
  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  RETURN_ON_NULL(s_list);

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  RETURN_ON_NULL(s_bundle);

  /*Get ISP CAMIF capabilities and accordingly request for   *
   *CAMIF output buffer format that is acceptable for PDlib. */
  if ((camif_cap_t*)s_bundle->isp_cap == NULL) {
    SHIGH("isp capabilities are NULL or Read already");
  } else {
    /*This code block will run only during boot-time. When HAL *
     *initiates camera open read caps and close.               */
    camif_data_fmt =
      &(((mct_pipeline_isp_cap_t*)\
      (s_bundle->isp_cap))->camif_supported_formats);

    memcpy((void *)&(s_bundle->camif_data_fmt),
      (void *)camif_data_fmt,
      sizeof(camif_cap_t));

    /*Reset the isp_cap buff address, if not this will over-write *
     *camif_data_fmt structure for every stop_session call and    *
     *thus causing the data to be over-written to zero's.         */
    s_bundle->isp_cap = NULL;
  }

  /* this deinit session includes
     power off sensor */
  ret = module_sensor_deinit_session(s_bundle);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR;
  }

  /* Deinit stereo peer sensor.
   * Power down  peer sensor if in stereo mode. */
  if (TRUE == s_bundle->is_stereo_configuration &&
    s_bundle->stereo_peer_s_bundle != NULL) {

    SINFO("Powering down peer stereo sensor.");
    ret = module_sensor_deinit_session(s_bundle->stereo_peer_s_bundle);
    if (ret == FALSE) {
      SERR("failed");
      goto ERROR;
    }
  }

  /* Terminate sensor thread */
  sensor_thread_msg_t msg;
  msg.stop_thread = TRUE;
  ssize_t nwrite = 0;
  nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if(nwrite < 0)
  {
    SERR("%s: Writing into fd failed",__func__);
  }

  if (s_bundle->live_connect_thread_started > 0) {
    /* Destroy the sensor live connect thread */
    sensor_live_connect_thread_destroy(s_bundle);
    s_bundle->live_connect_thread_started = FALSE;
  }

  ATRACE_END();

  return TRUE;
ERROR:
  ATRACE_END();
  SERR("failed");
  return FALSE;
}

int32_t module_sensor_get_stats_data(mct_module_t *module,
  uint32_t identity, stats_get_data_t* stats_get)
{
  boolean rc;
  memset(stats_get, 0x00, sizeof(stats_get_data_t));

  rc = sensor_util_post_downstream_event(module, identity,
    MCT_EVENT_MODULE_STATS_GET_DATA, stats_get);
  if (rc == FALSE){
    SERR("failed");
    return -EFAULT;
  }
  return 0;
}

int32_t module_sensor_fill_supported_camif_data(
  module_sensor_bundle_info_t *s_bundle,
  pdaf_sensor_native_info_t *pdaf_native_pattern)
{
  camif_cap_t* camif_data_fmt = NULL;
  int rc;
  int i;
#ifdef __ANDROID__
  char   prop[PROPERTY_VALUE_MAX];
#endif
  int  enable_qcom10 = 0;

  camif_data_fmt = &(s_bundle->camif_data_fmt);

  for (i = 0; i < (int)camif_data_fmt->format_count; i++) {
    if (camif_data_fmt->format[i] == CAMIF_PLAIN16_FORMAT) {
      if ((pdaf_native_pattern->buffer_data_type ==
        PDAF_DATA_TYPE_RAW10_LSB) ||
        (pdaf_native_pattern->buffer_data_type ==
          PDAF_DATA_TYPE_RAW16)) {
#ifdef __ANDROID__
        property_get("persist.vendor.camera.pdaf.qcom10", prop, "0.0");
        enable_qcom10 = atoi(prop);
#endif
        if (enable_qcom10) {
          SHIGH("Config CAMIF with QCOM10 format only");
        } else {
          SHIGH("Config CAMIF with PLAIN16 format");
          pdaf_native_pattern->buffer_data_type =
            PDAF_DATA_TYPE_RAW16;
        }
        break;
      }
    }
  }
  return 0;
}

static boolean module_sensor_is_ready_for_stream_on(mct_port_t *port,
  mct_event_t *event, sensor_set_res_cfg_t *res_cfg,
  module_sensor_bundle_info_t *s_bundle, int32_t bundle_id)
{
  boolean     is_bundle_started = TRUE;
  uint32_t    identity = event->identity;
  uint32_t    session_id, stream_id;

  sensor_util_unpack_identity(identity, &session_id, &stream_id);

  SHIGH("session_id=%d, stream_id=%d", session_id, stream_id);

  s_bundle->subsequent_bundle_stream = 0;

  /* find whether there is any bundle for this session that is already
     streamed ON  */
  is_bundle_started = sensor_util_find_is_any_bundle_started(port);
  SLOW("any bundle started %d", is_bundle_started);

  /* Find whether this stream belongs to bundle */
  if (bundle_id == -1) {
    /* This stream does not belong to any bundle */

    res_cfg->width       = s_bundle->max_width;
    res_cfg->height      = s_bundle->max_height;
    res_cfg->stream_mask = s_bundle->stream_mask;

    SLOW("s_bundle->stream_on_count %d", s_bundle->stream_on_count);
    SLOW("is_bundle_started %d", is_bundle_started);
    /* Start sensor streaming if this is the first stream ON and
       no other bundle has started */
    if ((s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
      SLOW("non-bundled stream, count %d dim=%dx%d mask=0x%x",
        s_bundle->stream_on_count, res_cfg->width, res_cfg->height,
        res_cfg->stream_mask);
      s_bundle->stream_on_count++;
      return TRUE;
    } else {
      s_bundle->stream_on_count++;
      return FALSE;
    }
  }
  module_sensor_port_bundle_info_t *bundle_info;
  bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
  if (bundle_info == NULL) {
    SERR("can't find bundle with id=%d", bundle_id);
    return FALSE;
  }
  bundle_info->stream_on_count++;

  /* update the res_cfg with bundle dim and mask */
  res_cfg->width       = s_bundle->max_width;
  res_cfg->height      = s_bundle->max_height;
  res_cfg->stream_mask = s_bundle->stream_mask;

  SLOW("bundle_info->stream_on_count %d", bundle_info->stream_on_count);
  SLOW("bundle_info->bundle_config.num_of_streams %d",
    bundle_info->bundle_config.num_of_streams);
  SLOW("s_bundle->stream_on_count %d", s_bundle->stream_on_count);
  SLOW("is_bundle_started %d", is_bundle_started);

  if (bundle_info->stream_on_count > 1) {
    s_bundle->subsequent_bundle_stream = 1;
    SLOW("This is post-liminary bundled stream");
  }
  /* If all streams in this bundle are on, we are ready to stream ON
     provided no other streams which is NOT part of bundle is STREAMED ON */
  if ((bundle_info->stream_on_count ==
       bundle_info->bundle_config.num_of_streams) &&
      (s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
    SLOW("stream_on_count=%d, w=%d, h=%d, stream_mask=%x",
      bundle_info->stream_on_count, res_cfg->width, res_cfg->height,
      res_cfg->stream_mask);
    return TRUE;
  }
  return FALSE;
}

static enum msm_vfe_frame_skip_pattern
module_sensor_convert_to_isp_skip(uint32_t skip)
{
  if (skip > EVERY_32FRAME)
    return EVERY_32FRAME;
  else if (skip > EVERY_16FRAME)
    return EVERY_16FRAME;
  else if (skip > EVERY_8FRAME)
    return EVERY_8FRAME;
  else
    return skip;
}

static boolean module_sensor_set_frame_skip_for_isp(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle, boolean en,
  cam_stream_type_t stream_type)
{
  int32_t  rc = SENSOR_SUCCESS;
  uint32_t stream_identity;
  uint16_t sensor_num_HDR_frame_skip;
  uint32_t isp_frameskip;


  if (stream_type >= CAM_STREAM_TYPE_MAX)
    return FALSE;

  RETURN_ON_FALSE(
    sensor_util_find_stream_identity_by_stream_type_and_session(
      module, stream_type,
      (uint16_t)SENSOR_SESSIONID(event->identity),
      &stream_identity));

  sensor_util_post_braketing_state_event(module,
    s_bundle, stream_identity, en, 0);

  if (en) {
    isp_frameskip = s_bundle->isp_frameskip[stream_type];
    sensor_num_HDR_frame_skip =
      s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_num_HDR_frame_skip;
    enum msm_vfe_frame_skip_pattern skip = module_sensor_convert_to_isp_skip(
      MAXQ(isp_frameskip, sensor_num_HDR_frame_skip));
    rc = sensor_util_set_frame_skip_to_isp(module, stream_identity, skip);
    if (FALSE == rc) {
      SERR("sensor_util_set_frame_skip_to_isp failed");
    } else {
      SHIGH("Set frame skip: %d", skip);
      if (s_bundle->max_isp_frame_skip < skip) {
          s_bundle->max_isp_frame_skip = skip;
      }
    }
  }

  return TRUE;
}

#define QFACTOR 100
static int32_t module_sensor_get_pick_data(
  module_sensor_bundle_info_t *s_bundle,
  cam_stream_size_info_t  *stream_size_info,
  sensor_set_res_cfg_t *res_cfg)
{
  uint32_t i = 0;
  int32_t sensor_ar = 0, curr_ar, last_ar = 1000, tmp_ar = 0;
  sensor_dimension_t      *active_array_size;
  cam_dimension_t         *stream_sizes = NULL;
  uint32_t                 mpix_t, mpix = 0;
  cam_format_t             fmt;

  RETURN_ON_NULL(s_bundle->sensor_lib_params);
  RETURN_ON_NULL(s_bundle->sensor_lib_params->sensor_lib_ptr);

  active_array_size = &s_bundle->sensor_lib_params->sensor_lib_ptr->
    pixel_array_size_info.active_array_size;

  SLOW("SENSOR DIM: W: %d, H: %d",
      active_array_size->width, active_array_size->height);

  if (active_array_size->width != 0 && active_array_size->height != 0) {

    sensor_ar = (active_array_size->width*QFACTOR)/active_array_size->height;
    /* Populate res_cfg struct based on requested dimensions */
    for (i = 0; i < stream_size_info->num_streams; i++) {

      fmt = stream_size_info->format[i];
      /* pick_data must not be performed for ANALYSIS stream */
      if (stream_size_info->type[i] == CAM_STREAM_TYPE_ANALYSIS)
        continue;

      stream_sizes = &stream_size_info->stream_sizes[i];
      if ((stream_sizes->width <= 0) || (stream_sizes->height <= 0)) {
        SERR("i %d invalid dim %d %d", i, stream_sizes->width,
          stream_sizes->height);
        continue;
      }

      /* Do not run if the format is raw meta */
      if ((fmt == CAM_FORMAT_META_RAW_10BIT) ||
        (fmt == CAM_FORMAT_META_RAW_8BIT))
        continue;

      tmp_ar = (stream_sizes->width*QFACTOR)/stream_sizes->height;
      curr_ar = abs(tmp_ar - sensor_ar);
      if (curr_ar < last_ar) {
        last_ar = curr_ar;
        res_cfg->aspect_r = tmp_ar;
      }
      mpix_t = (uint32_t)(stream_sizes->width * stream_sizes->height);
      if ((
        ((uint32_t)stream_sizes->width > res_cfg->width) ||
        ((uint32_t)stream_sizes->height > res_cfg->height))
        && (mpix_t > mpix)) {
        res_cfg->width  = (uint32_t)stream_sizes->width;
        res_cfg->height = (uint32_t)stream_sizes->height;
        mpix = mpix_t;
      }
      res_cfg->stream_mask |= (uint32_t)(1 << stream_size_info->type[i]);
    }
  } else {
    SERR("Failed. W: %d, H: %d",
      active_array_size->width, active_array_size->height);
  }

  /* Adjust width and AR for stereo configuration settings. */
  if (s_bundle->is_stereo_configuration == TRUE) {
    res_cfg->width/= 2L;
    res_cfg->aspect_r /= 2L;
  }

  SLOW("AR: %d, W: %d, H: %d, num streams: %d",res_cfg->aspect_r,
    res_cfg->width, res_cfg->height, stream_size_info->num_streams );
  return TRUE;
}

static boolean module_sensor_is_ready_for_stream_off(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean     is_bundle_started = TRUE;
  uint32_t    identity = event->identity;
  int32_t     bundle_id = -1;
  uint32_t    session_id, stream_id;
  sensor_util_unpack_identity(identity, &session_id, &stream_id);

  SHIGH("session_id=%d, stream_id=%d",
         session_id, stream_id);
  mct_port_t *port = sensor_util_find_src_port_with_identity(
                          module, identity);
  if (!port) {
    SERR("cannot find matching port with identity=0x%x", identity);
    return FALSE;
  }
  bundle_id = sensor_util_find_bundle_id_for_stream(port, identity);
  /* Find whether this stream is part of bundle */
  if (bundle_id == -1) {
    /* This stream does NOT belong to any bundle */
    s_bundle->stream_on_count--;

    /* find whether there is any bundle for this session that is already
       streamed ON  */
    is_bundle_started = sensor_util_find_is_any_bundle_started(port);
    SHIGH("no bundle id,any bundle started %d", is_bundle_started);

    /* Call sensor stream OFF only when all non bundle streams are streamed
       off AND no bundle is streaming */
    if ((s_bundle->stream_on_count == 0) && (is_bundle_started == FALSE)) {
      SLOW("non-bundled stream, stream count %d",
        s_bundle->stream_on_count);
      return TRUE;
    } else {
      SLOW("non-bundled stream, stream count %d",
        s_bundle->stream_on_count);
      return FALSE;
    }
  }
  module_sensor_port_bundle_info_t* bundle_info;
  bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
  if (bundle_info == NULL) {
    SERR("can't find bundle with id=%d",
                bundle_id);
    return FALSE;
  }
  /* decrement the counter */
  bundle_info->stream_on_count--;

  /* find whether there is any other bundle for this session that is already
     streamed ON  */
  is_bundle_started = sensor_util_find_is_any_bundle_started(port);
  SHIGH("any bundle started %d", is_bundle_started);

  /* If this stream is the last stream in the bundle to get stream_off,
     do sensor stream off provided no non bundle streams are streaming */
  if ((bundle_info->stream_on_count == 0) && (s_bundle->stream_on_count == 0) &&
      (is_bundle_started == FALSE)) {
    return TRUE;
  }
  SLOW("not needed, count=%d", bundle_info->stream_on_count);
  return FALSE;
}

boolean modules_sensor_set_new_resolution(mct_module_t *module,
  mct_event_t *event,
  module_sensor_bundle_info_t *s_bundle,
  module_sensor_params_t *module_sensor_params,
  sensor_set_res_cfg_t *stream_on_cfg,
  boolean *is_retry,
  mct_stream_info_t *stream_info)
{
  int32_t                rc = 0;
  boolean                ret = TRUE;
  sensor_out_info_t      sensor_out_info;
  uint8_t                i = 0;
  mct_port_t            *port = NULL;
  sensor_src_port_cap_t *port_cap = NULL;
  uint32_t               temp = 0;
  cam_rotation_t         rotation = stream_info->pp_config.rotation;
  sensor_set_res_t       set_res;
  sensor_thread_msg_t msg;
  int32_t nwrite = 0;
  module_sensor_params_t *stereo_peer_module_sensor_params = NULL;
  sensor_pack_mode_t pack_mode;

  SHIGH("SENSOR_SET_RESOLUTION:w*d:%d*%d, mask:%x,fast AEC:%d,FastAec mode:%d",
    stream_on_cfg->width, stream_on_cfg->height, stream_on_cfg->stream_mask,
    stream_on_cfg->is_fast_aec_mode_on, stream_on_cfg->fast_aec_sensor_mode);

  if (rotation == ROTATE_90 || rotation == ROTATE_270) {
    temp = stream_on_cfg->width;
    stream_on_cfg->width = stream_on_cfg->height;
    stream_on_cfg->height = temp;
    SHIGH("Width*Height after rotation: %d*%d", stream_on_cfg->width,
    stream_on_cfg->height);
  }

  rc = module_sensor_get_pick_data(s_bundle, &s_bundle->stream_size_info,
    stream_on_cfg);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  set_res.res_cfg = *stream_on_cfg;
  set_res.cfg_done_ptr = &s_bundle->res_cfg_done;

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SET_RESOLUTION);
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_RESOLUTION, &set_res);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  /* offload resolution config to sensor thread */
  if (!s_bundle->res_cfg_done) {
    msg.msgtype = OFFLOAD_FUNC;
    msg.offload_func = module_sensor_offload_cfg_resolution;
    msg.param1 = s_bundle;
    msg.param2 = module_sensor_params->sub_module_private;
    msg.param3 = NULL;
    msg.param4 = NULL;
    msg.stop_thread = FALSE;
    nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
    if (nwrite < 0) {
      SERR("%s: sensor_offload_resolution: Writing into fd failed:",__func__);
      module_sensor_offload_cfg_resolution(s_bundle,
        module_sensor_params->sub_module_private, NULL, NULL);
      RETURN_FALSE_IF(s_bundle->res_cfg_done < 0);
    }
  }

  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_RESOLUTION);
  memset(&sensor_out_info, 0, sizeof(sensor_out_info));
  sensor_out_info.actuator_sensitivity = s_bundle->actuator_sensitivity;
  sensor_out_info.is_secure = stream_info->is_secure;
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  s_bundle->frame_ctrl.max_sensor_delay =
    sensor_out_info.sensor_max_pipeline_frame_delay;

  /* Update lens informaiton */
  sensor_out_info.af_lens_info.f_number =
    s_bundle->sensor_common_info.camera_config.lens_info.f_number;
  sensor_out_info.af_lens_info.focal_length =
    s_bundle->sensor_common_info.camera_config.lens_info.focal_length;
  sensor_out_info.af_lens_info.hor_view_angle =
    s_bundle->sensor_common_info.camera_config.lens_info.hor_view_angle;
  sensor_out_info.af_lens_info.ver_view_angle =
    s_bundle->sensor_common_info.camera_config.lens_info.ver_view_angle;
  sensor_out_info.af_lens_info.total_f_dist =
    s_bundle->sensor_common_info.camera_config.lens_info.total_f_dist;
  sensor_out_info.af_lens_info.min_focus_distance =
    s_bundle->sensor_common_info.camera_config.lens_info.min_focus_distance;

  /* Fill sensor mount angle */
  sensor_out_info.sensor_mount_angle =
    s_bundle->sensor_common_info.camera_config.sensor_mount_angle;

  switch (s_bundle->sensor_common_info.camera_config.position){
  case CAM_POSITION_BACK:
    sensor_out_info.position = BACK_CAMERA_B;
    break;
  case CAM_POSITION_FRONT:
    sensor_out_info.position = FRONT_CAMERA_B;
    break;
  case CAM_POSITION_BACK_AUX:
    sensor_out_info.position = AUX_CAMERA_B;
    break;
  default:
    SERR("Invalid position = %d",
      s_bundle->sensor_common_info.camera_config.position);
    return FALSE;
  }

  sensor_out_info.mode =
    s_bundle->sensor_common_info.camera_config.modes_supported;

  sensor_out_info.af_lens_info.af_supported =
    (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) ?
                                                 TRUE : FALSE;

  if (sensor_out_info.meta_cfg.num_meta) {
    ret = sensor_util_post_downstream_event(module, event->identity,
      MCT_EVENT_MODULE_SENSOR_META_CONFIG, &sensor_out_info.meta_cfg);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
  }

  sensor_out_info.is_retry = FALSE;
  if(s_bundle->regular_led_trigger == 1)
    sensor_out_info.prep_flash_on = TRUE;
  else
    sensor_out_info.prep_flash_on = FALSE;

  /* Set stereo configuration information. */
  sensor_out_info.is_stereo_config = s_bundle->is_stereo_configuration;
  if(s_bundle->is_stereo_configuration) {
    sensor_out_info.dim_output.width *= 2;
    sensor_out_info.request_crop.last_pixel =
      sensor_out_info.request_crop.last_pixel * 2 + 1;
  }

  /* Set the pack mode accordingly in the stream config information.
   * There is no need to check here if the hardware supports it.
   * This check is already abstracted out on the ISP side. */
  pack_mode =
    sensor_util_get_pack_mode(stream_info->fmt, sensor_out_info.fmt);
  if (SENSOR_PACK_MODE_PLAIN8 == pack_mode ||
    SENSOR_PACK_MODE_PLAIN16 == pack_mode) {

    SLOW("Setting pack mode to plain pack.");
    module_sensor_fill_pack_mode(s_bundle, &sensor_out_info,
        PACK_MODE_PLAIN_PACK);
  }
  s_bundle->pack_mode = pack_mode;

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SET_STRM_CFG);
  ret = sensor_util_post_downstream_event(module, event->identity,
    MCT_EVENT_MODULE_SET_STREAM_CONFIG, &sensor_out_info);
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_STRM_CFG);
  *is_retry = sensor_out_info.is_retry;

  /* Output sensor/camif crop info to HAL (add to metadata).
   * This is needed to calculate the image FOV change caused by croping,
   * it allows aligning the image FOV's between the cameras for dual-camera.
   * Check for snapshot stream type is to handle any cases where the crop
   * information would be used in HAL1.
   * Check for HAL version is to ensure that we populate the crop
   * information to HAL3 for the default ON stream in case
   * of non-bundled streams.
   */
  if((s_bundle->hal_version == CAM_HAL_V3) ||
    (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT))
  {
    cam_stream_crop_info_t  crop_info;
    sensor_dimension_t     *active_array_size = NULL;
    int32_t                 crop_w, crop_h, base_w, base_h;

    active_array_size = &s_bundle->sensor_lib_params->sensor_lib_ptr->
      pixel_array_size_info.active_array_size;

    /* capture sensor native resolution */
    base_w = active_array_size->width;
    base_h = active_array_size->height;

    /* adjust initial crop size to account for binning */
    crop_w = sensor_out_info.dim_output.width * sensor_out_info.binning_factor;
    crop_h = sensor_out_info.dim_output.height * sensor_out_info.binning_factor;

    /* fix reported YUV cameras size - sensor width is 2x due to interleaved YUV */
    if(sensor_out_info.fmt == CAM_FORMAT_YUV_RAW_8BIT_YUYV ||
       sensor_out_info.fmt == CAM_FORMAT_YUV_RAW_8BIT_YVYU ||
       sensor_out_info.fmt == CAM_FORMAT_YUV_RAW_8BIT_UYVY ||
       sensor_out_info.fmt == CAM_FORMAT_YUV_RAW_8BIT_VYUY) {
      crop_w = crop_w / 2;
    }

    memset(&crop_info, 0, sizeof(crop_info));
    crop_info.stream_id       = (stream_info->identity & 0xffff);
    crop_info.roi_map.left    = 0;
    crop_info.roi_map.top     = 0;
    crop_info.roi_map.width   = base_w;
    crop_info.roi_map.height  = base_h;
    crop_info.crop.left       = sensor_out_info.offset_x;
    crop_info.crop.top        = sensor_out_info.offset_y;
    crop_info.crop.width      = crop_w;
    crop_info.crop.height     = crop_h;

    memcpy(&s_bundle->frame_ctrl.static_metadata.snap_crop_sensor, &crop_info, sizeof(crop_info));

    SLOW("CROP_INFO_SENSOR: str_id %d str_type %d crop (%d,%d,%d,%d) ==> (%d,%d,%d,%d)",
      crop_info.stream_id, stream_info->stream_type,
      crop_info.roi_map.left, crop_info.roi_map.top,
      crop_info.roi_map.width, crop_info.roi_map.height,
      crop_info.crop.left, crop_info.crop.top,
      crop_info.crop.width, crop_info.crop.height);

    memset(&crop_info, 0, sizeof(crop_info));
    crop_info.stream_id       = (stream_info->identity & 0xffff);
    crop_info.roi_map.left    = 0;
    crop_info.roi_map.top     = 0;
    crop_info.roi_map.width   = sensor_out_info.dim_output.width;
    crop_info.roi_map.height  = sensor_out_info.dim_output.height;
    crop_info.crop.left       = sensor_out_info.request_crop.first_pixel;
    crop_info.crop.top        = sensor_out_info.request_crop.first_line;
    crop_info.crop.width      = sensor_out_info.request_crop.last_pixel -
      sensor_out_info.request_crop.first_pixel + 1;
    crop_info.crop.height     = sensor_out_info.request_crop.last_line -
      sensor_out_info.request_crop.first_line + 1;

    memcpy(&s_bundle->frame_ctrl.static_metadata.snap_crop_camif, &crop_info, sizeof(crop_info));

    SLOW("CROP_INFO_CAMIF: str_id %d str_type %d crop (%d,%d,%d,%d) ==> (%d,%d,%d,%d)",
      crop_info.stream_id, stream_info->stream_type,
      crop_info.roi_map.left, crop_info.roi_map.top,
      crop_info.roi_map.width, crop_info.roi_map.height,
      crop_info.crop.left, crop_info.crop.top,
      crop_info.crop.width, crop_info.crop.height);
  }

  return ret;
}

/** module_sensor_start_af_bracketing: start focus bracketing
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: mct event
 *
 *  This function is used to start focus bracketing
 *
 *  Return: TRUE for success and FALSE on failure
 **/
boolean module_sensor_start_af_bracketing(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t *event)
{
  module_sensor_ctrl_t      *module_ctrl =
      (module_sensor_ctrl_t *)module->module_private;
  module_sensor_params_t    *actuator_module_params = NULL;
  sensor_af_bracket_t       *bracket = &(s_bundle->af_bracket_params);
  sensor_frame_order_ctrl_t *ctrl = &(s_bundle->af_bracket_params.ctrl);
  int32_t rc = 0;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("No Actuator Module!");
    return TRUE;
  }

  SHIGH("AF_bracketing set for zsl snapshot");

  /* Lock mutex to wait properly for reseting lens */
  pthread_mutex_lock(&bracket->lens_move_done_sig);
  bracket->lens_move_progress = TRUE;
  pthread_mutex_unlock(&bracket->lens_move_done_sig);

  sensor_util_post_braketing_state_event(module,
    s_bundle, event->identity, TRUE, 0);

  rc = sensor_util_set_frame_skip_to_isp(module,
         event->identity, SKIP_ALL);
  if (rc < 0) {
    SERR("sensor_util_set_frame_skip_to_isp failed");
  }

  bracket->lens_reset = FALSE;
  bracket->wait_frame = ACTUATOR_WAIT_FRAME;

  sensor_thread_msg_t msg;
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_lens_reset;
  msg.param1 = actuator_module_params;
  msg.param2 = NULL;
  msg.param3 = (void *)bracket;
  msg.param4 = NULL;
  msg.stop_thread = FALSE;
  int32_t nwrite = 0;
  nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
  if(nwrite < 0) {
    SERR("Writing into fd failed");
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  }

  ctrl->captured_count = 0;
  ctrl->enable = TRUE;

  return TRUE;
}


/** module_sensor_update_af_bracket_entry: update af bracket
 *  entry
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function is used to update selected af bracket entry to
 *  move lens in actuator upon sof under af bracketing mode.
 *  It Also updates the MCT bus msg to notify good frame id.
 *
 *  NOTE:
 *  For ZSL Snapshot, it will first reset lens at
 *  MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT.
 *  For Non-ZSL Snapshot, it will first reset lens at
 *  MCT_EVENT_CONTROL_STREAMON It will be always be the first
 *  lens position
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_update_af_bracket_entry(mct_module_t *module,
 void *s_bundle, mct_event_t *event)
{
  int32_t                        rc = 0;
  uint8_t                        num_of_burst;
  module_sensor_bundle_info_t   *bun = NULL;
  module_sensor_params_t        *actuator_module_params = NULL;
  sensor_af_bracket_t           *bracket = NULL;
  sensor_frame_order_ctrl_t     *ctrl = NULL;
  module_sensor_ctrl_t          *module_ctrl = NULL;

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  bun = (module_sensor_bundle_info_t *) s_bundle;

  if (bun->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("No Actuator Module Present!");
    return TRUE;
  }
  actuator_module_params = bun->module_sensor_params[SUB_MODULE_ACTUATOR];

  bracket = &(bun->af_bracket_params);
  ctrl = &(bun->af_bracket_params.ctrl);
  num_of_burst = bracket->burst_count;

  SLOW("AF_bracketing : Enter, sof LOGIC fid[%d]", bun->last_idx);

  RETURN_ON_NULL(bracket);
  RETURN_ON_NULL(ctrl);

  if (ctrl->captured_count < num_of_burst) {
    /* Check if lens movement is completed */
    pthread_mutex_lock(&bracket->lens_move_done_sig);
    if (!bracket->lens_move_progress) {
      if (!ctrl->captured_count) {
        sensor_util_post_braketing_state_event(module,
          s_bundle, event->identity, TRUE, 0);
      }

      if (bracket->wait_frame == ACTUATOR_WAIT_FRAME) {
        mct_bus_msg_t bus_msg;
        cam_frame_idx_range_t range;
        memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
        memset(&range, 0, sizeof(cam_frame_idx_range_t));
        bus_msg.sessionid = bun->sensor_info->session_id;
        bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
        range.min_frame_idx = bun->last_idx + ACTUATOR_WAIT_FRAME;
        range.max_frame_idx = range.min_frame_idx + ACTUATOR_MAX_WAIT_FRAME;
        bus_msg.msg = &range;
        bus_msg.size = sizeof(cam_frame_idx_range_t);
        if(mct_module_post_bus_msg(module,&bus_msg)!=TRUE)
          SERR("error posting af bracketing");
      }
      if (bracket->wait_frame-- > 1) {
        /*In case actuator need more than
        * one frame time to settle after reaching
        * its position*/
        pthread_mutex_unlock(&bracket->lens_move_done_sig);
        return TRUE;
      }
      rc = sensor_util_set_frame_skip_to_isp(module,
             event->identity, EVERY_32FRAME);
      if (!rc) {
        SERR("sensor_util_set_frame_skip_to_isp failed");
      }

      bracket->lens_reset = TRUE;
      bracket->wait_frame = ACTUATOR_WAIT_FRAME;
      bracket->lens_move_progress = TRUE;
    } else if (bracket->lens_reset &&
      sensor_util_is_previous_frame_sent(module, event,
        bun->identity[CAM_STREAM_TYPE_SNAPSHOT],
        bun->identity[CAM_STREAM_TYPE_PREVIEW])) {

     /* Move the lens only when previous frame is sent and lens is reset,
     * in order to avoid false potive check for the first frame
     */
      rc = sensor_util_set_frame_skip_to_isp(module,
             event->identity, SKIP_ALL);
      if (!rc) {
        SERR("sensor_util_set_frame_skip_to_isp failed");
      }

      ctrl->captured_count++;

        /* Don't do extra lens move after the last frame is captured */
      if (ctrl->captured_count < num_of_burst) {
        /* Move Lens in Actuator */
        sensor_thread_msg_t msg;
        msg.msgtype = OFFLOAD_FUNC;
        msg.offload_func = module_sensor_offload_lens_move;
        msg.param1 = actuator_module_params;
        msg.param2 = (void*)bracket->direction[ctrl->captured_count-1];
        msg.param3 = (void*)bracket->steps[ctrl->captured_count-1];
        msg.param4 = (void*)bracket;
        msg.stop_thread = FALSE;
        ssize_t nwrite = 0;
        nwrite = write(bun->pfd[1],
          &msg, sizeof(sensor_thread_msg_t));
        if(nwrite < 0) {
          SERR("Writing into fd failed");
          bracket->lens_move_progress = FALSE;
        }
      } else {
        bracket->lens_move_progress = FALSE;
      }
    }
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SLOW("AF_bracketing : captured passed busrt, disabling");
    ctrl->enable = FALSE;
    ctrl->captured_count = 0;

    /*Restore HAL skip pattern*/
    rc = sensor_util_set_frame_skip_to_isp(module,
           event->identity, bun->hal_params.hal_frame_skip_pattern);
    if (!rc) {
      SERR("sensor_util_set_frame_skip_to_isp failed");
    }

    sensor_util_post_braketing_state_event(module,
      s_bundle, event->identity, FALSE, 0);
  }
  SHIGH("AF_bracketing : sof fid[%d], captured_count %d, X",
    bun->last_idx, ctrl->captured_count);

  return TRUE;
}

/** module_sensor_start_flash_bracket_entry
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function configures and begin flash
 *  bracket control
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_start_flash_bracket_entry(mct_module_t *module,
 sensor_bundle_info_t *bundle_info, mct_event_t *event)
{
  int32_t rc = 0;
  sensor_bracket_params_t   *flash_bracket_params= NULL;
  sensor_bracket_ctrl_t     *flash_ctrl = NULL;
  cam_frame_idx_range_t      range;
  const uint8_t num_skip_led_up = 4;
  const uint8_t num_skip_exp_linecount = 3;

  RETURN_ON_NULL(bundle_info);
  RETURN_ON_NULL(bundle_info->s_bundle);

  flash_bracket_params = &(bundle_info->s_bundle->flash_bracket_params);
  flash_ctrl = &(flash_bracket_params->ctrl);

  RETURN_ON_FALSE(
    sensor_util_find_stream_identity_by_stream_type_and_session(
      module, CAM_STREAM_TYPE_SNAPSHOT,
      SENSOR_SESSIONID(event->identity),
      &flash_bracket_params->flash_bracket.snapshot_identity));

  flash_bracket_params->flash_bracket.preview_identity =
    event->identity;

  rc = sensor_util_set_frame_skip_to_isp(module,
         flash_bracket_params->flash_bracket.preview_identity, SKIP_ALL);
  if (!rc) {
    SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
    return FALSE;
  }
  rc = sensor_util_set_frame_skip_to_isp(module,
         flash_bracket_params->flash_bracket.snapshot_identity,
         SKIP_ALL);
  if (!rc) {
    SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
    return FALSE;
  }

  memset(&range, 0, sizeof(cam_frame_idx_range_t));
  /* Fill metadata msg */
  range.min_frame_idx = bundle_info->s_bundle->last_idx + num_skip_led_up;
  range.max_frame_idx = range.min_frame_idx + num_skip_exp_linecount;

  /* save frame info to local structure */
  flash_ctrl->min_frame_idx = range.min_frame_idx;
  flash_ctrl->max_frame_idx = range.max_frame_idx;

  SLOW(" Posting flash_bracket msg to bus, sof %d, min:%d max:%d",
    bundle_info->s_bundle->last_idx, range.min_frame_idx,
    range.max_frame_idx);

  if (sensor_util_post_to_bus(module, bundle_info->s_bundle,
        MCT_BUS_MSG_ZSL_TAKE_PICT_DONE, sizeof(cam_frame_idx_range_t),
        &range) != TRUE) {
    SERR("error posting pict done range msg");
    return FALSE;
  }

  /* enable internally */
  SHIGH("Flash Bracket Enable!!");
  flash_bracket_params->ctrl.enable = TRUE;
  return TRUE;

}

/** module_sensor_update_flash_bracket_entry:
 *
 *  @module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @event: received event
 *
 *  This function is used to update Flash bracket entry.
 *  Currently only two frame is needed, so it only turns off LED
 *  when LED on frame is reached.
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_update_flash_bracket_entry(mct_module_t *module,
 void *s_bundle, mct_event_t *event)
{
  int32_t rc = 0;
  uint8_t num_of_burst;
  module_sensor_params_t *led_module_params = NULL;
  sensor_flash_bracket_t *bracket = NULL;
  sensor_bracket_ctrl_t  *ctrl = NULL;
  module_sensor_params_t *module_sensor_params = NULL;
  module_sensor_ctrl_t   *sensor_ctrl;

  if (!s_bundle) {
     SERR("Invalid s_bundle_ptr");
     return FALSE;
  }

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
  module_sensor_bundle_info_t * bun = (module_sensor_bundle_info_t *) s_bundle;
  module_sensor_params = bun->module_sensor_params[SUB_MODULE_SENSOR];
  led_module_params = bun->module_sensor_params[SUB_MODULE_LED_FLASH];
  bracket = &(bun->flash_bracket_params.flash_bracket);
  ctrl = &(bun->flash_bracket_params.ctrl);
  num_of_burst = bracket->burst_count;

  SLOW("Flash_bracketing : Enter");

  /* Check internal flag */
  if (ctrl->enable == TRUE) {
    if (bun->last_idx < ctrl->max_frame_idx) {
      SLOW("Flash_bracketing : sof LOGIC fid[%d], captured_count %d",
        bun->last_idx, ctrl->captured_count);

      /* Request one isp frame for led on */
      if ((bun->last_idx == ctrl->min_frame_idx - 1) ||
          (bun->last_idx == ctrl->max_frame_idx - 1)) {
        if (bun->last_idx == ctrl->min_frame_idx - 1)
          sensor_util_post_braketing_state_event(module,
            s_bundle, bracket->preview_identity, TRUE, 0);

        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->preview_identity, EVERY_32FRAME);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->snapshot_identity, EVERY_32FRAME);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
      }
      /* check for matched LED frame */
      if(bun->last_idx < ctrl->min_frame_idx) {
        SLOW("waiting led frame, cur sof [%d), expect frame [%d], skipping",
          bun->last_idx, ctrl->min_frame_idx);
      } else if (bun->last_idx == ctrl->min_frame_idx) {

        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->preview_identity, SKIP_ALL);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }
        rc = sensor_util_set_frame_skip_to_isp(module,
          bracket->snapshot_identity, SKIP_ALL);
        if (!rc) {
          SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
        }

      } else if (bun->last_idx == ctrl->min_frame_idx + 1) {
        /* disable LED for max frame */
        SHIGH("LED frame match, turn off LED");
        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF , NULL);

        if ( rc < 0 ) {
          SERR("failed: LED_FLASH_SET_OFF");
          return FALSE;
        } else {
          /*recover gain and linecount from LED off*/
          aec_update_t aec_update;
          aec_update.sensor_gain = bun->led_off_gain;
          aec_update.linecount = bun->led_off_linecount;
          SHIGH("led_off stats gain %f lnct %d", aec_update.sensor_gain,
            aec_update.linecount);
          rc = module_sensor_params->func_tbl.process(
            module_sensor_params->sub_module_private,
            SENSOR_SET_AEC_UPDATE, &aec_update);
          if (rc < 0) {
            SERR("failed");
            return FALSE;
          }
          if (sensor_ctrl->is_dual_cam == TRUE &&
            bun->dualc_info.fps_match_enabled == TRUE &&
            bun->dualc_info.streaming_master == CAM_MODE_PRIMARY) {
            module_sensor_send_aec_update_to_slave(module, event,
              s_bundle, &aec_update);
          }
          bun->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
          sensor_util_post_led_state_msg(module, s_bundle,
            event->identity);
        }
      } else {
        SHIGH("waiting for LED off frame");
      }
    } else {
      SHIGH("max_frame_idx reached, disabling, cur sof : [%d]", bun->last_idx);
      ctrl->enable = FALSE;
      bracket->enable = FALSE;

      /*Restore HAL skip pattern*/
      rc = sensor_util_set_frame_skip_to_isp(module,
        bracket->preview_identity, bun->hal_params.hal_frame_skip_pattern);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }
      rc = sensor_util_set_frame_skip_to_isp(module,
        bracket->snapshot_identity, bun->hal_params.hal_frame_skip_pattern);
      if (!rc) {
        SERR("%s: sensor_util_set_frame_skip_to_isp failed",__func__);
      }

      sensor_util_post_braketing_state_event(module,
        s_bundle, event->identity, FALSE, 0);
    }
  } else
  SHIGH("Flash_Bracketing not enabled");
  return TRUE;
}

/** module_sensor_set_param_af_bracket:
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is passes HAL AF bracketing params to sensor
 *  and setup bracketing info. for UbiFocus the af is evenly
 *  spaced between the total steps.
 *
 *  Return: TRUE for success and FALSE on failure**/
boolean module_sensor_set_param_af_bracket(
  module_sensor_bundle_info_t * s_bundle, void *parm_data)
{
  boolean                      ret = TRUE;
  uint32_t                     total_steps;
  int32_t                      step;
  int32_t                      i;
  sensor_get_lens_lens_range_t lens_range;
  module_sensor_params_t       *actuator_module_params = NULL;
  cam_af_bracketing_t          *cam_af_bracket = NULL;
  sensor_af_bracket_t          *bracket = NULL;
  sensor_frame_order_ctrl_t    *ctrl = NULL;

  SLOW("AF_bracketing : E");
  RETURN_ON_NULL(parm_data);
  RETURN_ON_NULL(s_bundle);

  bracket = &(s_bundle->af_bracket_params);
  ctrl = &(s_bundle->af_bracket_params.ctrl);

  /* Copy structure from HAL */
  cam_af_bracket = (cam_af_bracketing_t*) parm_data;
  bracket->enable = cam_af_bracket->enable;
  bracket->burst_count = cam_af_bracket->burst_count;

  actuator_module_params = s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

  if (!bracket->enable) {
    af_update_t af_update;
    SHIGH("Disable focus bracketing");
    af_update.reset_lens = TRUE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    return TRUE;
  }

  if (bracket->burst_count > ACTUATOR_BRACKET_MAX ||
    bracket->burst_count < ACTUATOR_BRACKET_MIN) {
    SERR("Invalid AF Bracket burst num : %d", bracket->burst_count);
    return FALSE;
  }

  /* Get AF Pointer */
  ret = actuator_module_params->func_tbl.process(
    actuator_module_params->sub_module_private,
    ACTUATOR_GET_LENS_MOVING_RANGE, &lens_range);
  if (ret < 0) {
    SERR("failed to get lens data rc %d", ret);
    return FALSE;
  }

  /* Calculate bracket */
  total_steps = lens_range.position_far_end - lens_range.position_near_end + 1;

  if (total_steps < bracket->burst_count) {
    SERR("Invalid params: total steps %d, burst %d",
      total_steps, bracket->burst_count);
    return FALSE;
  }
  step = (int32_t)(total_steps / (bracket->burst_count - 1));
  for (i=0; i<bracket->burst_count-1; i++) {
    bracket->steps[i] = step;
    bracket->direction[i] = MOVE_NEAR;
  }
  SHIGH("total_steps = %d, burst %d, first step = %d, next step = %d",
    total_steps, bracket->burst_count, bracket->steps[0], step);

  /* Initialize control */
  ctrl->captured_count  = 0;
  ctrl->is_post_msg     = FALSE;
  ctrl->enable = FALSE;

  return TRUE;
}

/** module_sensor_set_hdr_zsl_mode: sensor hdr mode
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to set the hdr zsl mode
 *  and fetch the required information from stats
 *
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_set_hdr_zsl_mode(mct_module_t *module,
  uint32_t identity, module_sensor_bundle_info_t *s_bundle)
{
  module_sensor_params_t  *module_sensor_params;
  int32_t rc = 0;
  boolean ret = TRUE;
  sensor_set_hdr_ae_t hdr_info;
  uint16_t sensor_num_HDR_frame_skip;
  uint32_t isp_frameskip;
  memset(&hdr_info, 0, sizeof(sensor_set_hdr_ae_t));

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module);

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

  rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_SENSOR_FORMAT, &hdr_info.output_format);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  sensor_num_HDR_frame_skip =
    s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_num_HDR_frame_skip;

  if ((s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
    (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW))) {
    SLOW("HDR Zsl Mode");
    hdr_info.hdr_zsl_mode = 1;
    isp_frameskip = s_bundle->isp_frameskip[CAM_STREAM_TYPE_SNAPSHOT];

    /* The skip come from ISP */
    hdr_info.isp_frame_skip =
      module_sensor_convert_to_isp_skip(
        MAXQ(isp_frameskip, sensor_num_HDR_frame_skip));
  } else {
    SLOW("HDR Non Zsl Mode");
    hdr_info.hdr_zsl_mode = 0;
    isp_frameskip = s_bundle->isp_frameskip[CAM_STREAM_TYPE_PREVIEW];

    /* The skip come from ISP */
    hdr_info.isp_frame_skip =
      module_sensor_convert_to_isp_skip(
        MAXQ(isp_frameskip, sensor_num_HDR_frame_skip));
  }
  if (hdr_info.hdr_zsl_mode  && (hdr_info.output_format == SENSOR_BAYER)) {
    /* get initial gain/linecount from AEC */
    rc = module_sensor_get_stats_data(module, identity, &hdr_info.stats_get);
    if (rc < 0) {
      SERR("Failed to get stats info");
      return FALSE;
    }
  }
  rc = module_sensor_params->func_tbl.process(
         module_sensor_params->sub_module_private,
         SENSOR_SET_HDR_ZSL_MODE, &hdr_info);
  if (rc < 0) {
    SERR("Set HDR Zsl Mode failed");
    return FALSE;
  }
  s_bundle->sensor_common_info.ae_bracket_params.enable = TRUE;
  return ret;
}

/** module_sensor_post_flash_mode_to_thread: Post message to thread
 *
 *  @param1: led_module_params
 *  @param2: flash mode
 *
 *  This function is used to process the led events to sensor thread
 *
 *  Return: TRUE or FALSE
 **/
boolean module_sensor_post_flash_mode_to_thread(mct_module_t* module,
  uint32_t *identity,  module_sensor_bundle_info_t* s_bundle, void *data)
{
  uint8_t ret = TRUE;
  sensor_thread_msg_t msg;
  msg.msgtype = OFFLOAD_FUNC;
  msg.offload_func = module_sensor_offload_proc_led;
  msg.param1 = (void *)module;
  msg.param2 = (void*)identity;
  msg.param3 = (void*)s_bundle;
  msg.param4 = (void*)data;
  msg.stop_thread = FALSE;
  ssize_t nwrite = 0;
  nwrite = write(s_bundle->pfd[1],
    &msg, sizeof(sensor_thread_msg_t));
  if(nwrite < 0) {
    SERR("%s: Writing into fd failed",__func__);
    return FALSE;
  }
  return TRUE;
}


/** module_sensor_preflash_control: pre flash control
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to control the pre flash
 *  sequence and correct order of frames picked
 *
 *  Return: TRUE for success and FALSE on failure**/

static boolean module_sensor_preflash_control(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle)
{
  module_sensor_params_t *led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  uint8_t flash_mode;
  sensor_capture_control_t  *ctrl = &(s_bundle->cap_control);
  sensor_batch_config_t     *bctrl = &ctrl->bctrl[0];
  int32_t rc = 0;
  uint8_t ret = 0;
  void *data = NULL;
  uint32_t max_frame_skip = 0;

  if (s_bundle->last_idx < bctrl->ctrl_end_frame) {
    if (ctrl->retry_frame_skip >= 0) {
      if (!ctrl->retry_frame_skip--) {
        SHIGH("preflash on/off %d. Sending flash mode downstream - %d",
          bctrl->flash_mode, bctrl->cam_flash_mode);
        /* Post desired flash mode for each batch in advance */
        sensor_util_post_downstream_event(module, event->identity,
          MCT_EVENT_MODULE_SET_FLASH_MODE, &bctrl->cam_flash_mode);

        if (bctrl->flash_mode == LED_FLASH_SET_OFF &&
          s_bundle->regular_led_af == 0 &&
          s_bundle->flash_rer_enable) {
          data = (void *)led_module_params;
          bctrl->flash_mode = LED_FLASH_SET_RER_PROCESS;
        }

        s_bundle->batch_idx = 0;
        bctrl->skip_config_mode = 0;
        ret = module_sensor_post_flash_mode_to_thread(module, &event->identity, s_bundle, data);
        if (!ret) {
          SERR("failed to post flash mode=%d to thread",bctrl->flash_mode);
          bctrl->led_toggle_pending = FALSE;
        }
      } else {
        SHIGH("request isp sw skip range");
        /*send skip request to isp to enable skip*/
        max_frame_skip = bctrl->interval;
        rc = sensor_util_sw_frame_skip_to_isp(module, event->identity, s_bundle,
               IFACE_SKIP_RANGE, TRUE, FALSE, max_frame_skip);
        if (!rc) {
          SERR("sensor_util_sw_frame_skip_to_isp failed. Retrial counter %d",
            ctrl->retry_frame_skip+1);
        } else {
          ctrl->retry_frame_skip = 0;
        }
        bctrl->ctrl_end_frame++;
      }
    } else {
      /*Future frame info*/
      if (bctrl->flash_mode == LED_FLASH_SET_RER_PROCESS ||
        bctrl->flash_mode == LED_FLASH_SET_OFF ) {
        PTHREAD_MUTEX_LOCK(&s_bundle->capture_control_mutex);
        if (s_bundle->regular_led_af == 0) {
          if (!bctrl->led_toggle_pending) {
            SHIGH("Need future frame");
            s_bundle->regular_led_trigger = 1;
            cam_prep_snapshot_state_t state;
            ctrl->enable = 0;
            state = NEED_FUTURE_FRAME;
            if (sensor_util_post_to_bus(module,
              s_bundle, MCT_BUS_MSG_PREPARE_HW_DONE,
              sizeof(cam_prep_snapshot_state_t), &state) != TRUE) {
              SERR("error posting led frame range msg");
            }
          } else if (s_bundle->last_idx == bctrl->ctrl_end_frame - 1) {
            SHIGH("Led config ioctl in progress. flash mode %d",
              bctrl->flash_mode);
            /*This is to account for led kernel ioctl taking too much time.
             *Retry unless led ioctl fail or mode is set. Partial skip will
             *not take effect if ioctl take too long*/
            bctrl->ctrl_end_frame++;
          }
        } else {
          /* Sensor takes responsibility of resetting this flag in
            LDF AF usecase in order to avoid race condition in HAL1
            where sending MCT_BUS_MSG_PREPARE_HW_DONE happens in
            different thread */
          s_bundle->regular_led_af = 0;
        }
        PTHREAD_MUTEX_UNLOCK(&s_bundle->capture_control_mutex);
      } else if (bctrl->flash_mode == LED_FLASH_SET_PRE_FLASH){
        ctrl->enable = 0;
        SLOW("preflash on control exit");
      }
    }
  } else {
    SLOW("preflash control exit");
    ctrl->enable = 0;
  }

  return TRUE;
}

/** module_sensor_flash_exp_bkt_control: flash control
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to control the main flash
 *  sequence and correct order of frames picked
 *
 *  Return: TRUE for success and FALSE on failure**/

static boolean module_sensor_flash_exp_bkt_control(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle)
{
  module_sensor_params_t *led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  uint8_t flash_mode;
  sensor_capture_control_t   *ctrl = &(s_bundle->cap_control);
  sensor_batch_config_t      *bctrl = NULL;
  int32_t rc = TRUE;
  uint8_t ret = TRUE;
  uint32_t max_frame_skip = 0;

  if (ctrl->idx >= ctrl->num_batches) {
    ctrl->enable = 0;
    return TRUE;
  }
  /* Index for the batch*/
  bctrl =  &ctrl->bctrl[ctrl->idx];
  ctrl->delay_batch = 0;

  SHIGH("sof %d",s_bundle->last_idx);
  if (ctrl->trigger) {
    SLOW("Start batch %d",ctrl->idx);
    /* Fill start and end for this batch */
    bctrl->ctrl_start_frame = s_bundle->last_idx;
    bctrl->ctrl_end_frame = bctrl->ctrl_start_frame +
      bctrl->interval + bctrl->burst_count - 1;
    /* Post desired flash mode for each batch in advance */
    SHIGH("Sending flash mode downstream - %d", bctrl->cam_flash_mode);
    sensor_util_post_downstream_event(module, event->identity,
      MCT_EVENT_MODULE_SET_FLASH_MODE, &bctrl->cam_flash_mode);
    ctrl->trigger = 0;
  }

  if (s_bundle->delay_frame_cnt) {
    --s_bundle->delay_frame_cnt;
    SHIGH("Requested for Delay frame from ISP : %d",
      s_bundle->delay_frame_cnt);
    bctrl->ctrl_start_frame++;
    bctrl->ctrl_end_frame++;
    return TRUE;
  }

  if (s_bundle->last_idx < bctrl->ctrl_end_frame) {
    if (!(s_bundle->delay_frame_idx < s_bundle->last_idx ||
      (s_bundle->capture_pipeline_delay + s_bundle->last_idx <
      bctrl->interval + bctrl->ctrl_start_frame))) {
      SHIGH("delay batch");
      ctrl->delay_batch = 1; /* Delay frame setting */
      if (bctrl->type != CAM_CAPTURE_FLASH)
        goto DONE;
      else
        ctrl->retry_frame_skip++; /* Retry skip again for flash */
    }
    s_bundle->delay_frame_idx = 0;

    /* If we get a reset batch but there is no need to reset the state
       of flash(already off) then do not proceed with the Main flash
       on/off request. */
    if(bctrl->type == CAM_CAPTURE_RESET && bctrl->led_toggle_pending == FALSE)
    {
      ctrl->enable = 0;
      goto DONE;
    }

    if (ctrl->retry_frame_skip >= 0) {
      if (bctrl->flash_mode == LED_FLASH_SET_OFF)
        max_frame_skip = s_bundle->main_flash_off_frame_skip;
      else
        max_frame_skip = s_bundle->main_flash_on_frame_skip;
      if (!ctrl->retry_frame_skip--) {
        /* Dont delay flash off */
        if (bctrl->flash_mode != LED_FLASH_SET_OFF &&
          ctrl->max_pipeline_delay > max_frame_skip) {
          SHIGH("Delay main flash on if pipeline delay %d> led on time %d",
            ctrl->max_pipeline_delay, s_bundle->main_flash_on_frame_skip);
          ctrl->retry_frame_skip++;
          ctrl->max_pipeline_delay--;
          return TRUE;
        }
        /*Initial mode before the flash is fired*/
        SHIGH("Main flash on/off request");
        s_bundle->batch_idx = ctrl->idx;
        bctrl->skip_config_mode = 0;
        ret = module_sensor_post_flash_mode_to_thread(module, &event->identity, s_bundle, NULL);
        if (!ret) {
          SERR("Set flash mode %d failed",bctrl->flash_mode);
          bctrl->led_toggle_pending = FALSE;
        }
      } else {
        if (!ctrl->delay_batch && ctrl->max_pipeline_delay)
          ctrl->max_pipeline_delay--;
        max_frame_skip =
            MAXQ(max_frame_skip, ctrl->max_pipeline_delay);
        if (max_frame_skip > ctrl->max_pipeline_delay)
          ctrl->delay_batch = 1;
        SHIGH("isp max sw frame skip %d",max_frame_skip);
        rc = sensor_util_sw_frame_skip_to_isp(module, event->identity, s_bundle,
               IFACE_SKIP_RANGE, TRUE, FALSE, max_frame_skip);
        if (!rc) {
          SERR("sensor_util_sw_frame_skip_to_isp attempt failed Retrial counter %d",
            ctrl->retry_frame_skip+1);
        } else {
          ctrl->retry_frame_skip = 0;
        }
      }
    }
    if (s_bundle->last_idx ==
      (unsigned int)(bctrl->ctrl_start_frame + bctrl->interval - 1)
      && (ctrl->evt[ctrl->idx] == SENSOR_FLASH_EXP_BKT_EVT)) {
      /*Post zsl pict done event to hal layer*/
      PTHREAD_MUTEX_LOCK(&s_bundle->capture_control_mutex);
      cam_frame_idx_range_t range;

      memset(&range, 0, sizeof(cam_frame_idx_range_t));
      if (!bctrl->led_toggle_pending) {
        PTHREAD_MUTEX_UNLOCK(&s_bundle->capture_control_mutex);
        range.min_frame_idx = s_bundle->last_idx + 1;
        range.frame_skip_count = 0;
        range.config_batch_idx = ctrl->idx;
        SHIGH("Send zsl take picture done : %d for batch %d",range.min_frame_idx,
          range.config_batch_idx);
        if (sensor_util_post_to_bus(module,
            s_bundle, MCT_BUS_MSG_ZSL_TAKE_PICT_DONE,
            sizeof(cam_frame_idx_range_t), &range) != TRUE) {
              SERR("error posting led frame range msg");
        }
      } else {
        /* This is to account for led kernel ioctl taking too much time.
               * Retry unless led ioctl fail or mode is set. Partial skip will
               * not take effect if ioctl take too long*/
        PTHREAD_MUTEX_UNLOCK(&s_bundle->capture_control_mutex);
        SHIGH("Led config ioctl in progress. flash mode %d",bctrl->flash_mode);
        ctrl->delay_batch = 1;
      }
    }
  } else {
    /* Reset the controls for next batch */
    SLOW("Control end");
    ctrl->max_pipeline_delay = s_bundle->capture_pipeline_delay;
    ctrl->retry_frame_skip = 0;
    if (++ctrl->idx >= ctrl->num_batches) {
      ctrl->enable = 0;
      return TRUE;
    }
    bctrl =  &ctrl->bctrl[ctrl->idx];
    /* reset trigger for next transition */
    ctrl->trigger = 1;
    /* send skip request to isp to enable skip for next batch */
    rc = sensor_util_sw_frame_skip_to_isp(module, event->identity, s_bundle,
               IFACE_SKIP_RANGE, TRUE, FALSE, bctrl->interval);
    if (!rc) {
      ctrl->retry_frame_skip = s_bundle->retry_frame_skip;
      SERR("sensor_util_sw_frame_skip_to_isp attempt failed Retrial counter %d",
        ctrl->retry_frame_skip+1);
    }
  }
DONE:
  if (ctrl->delay_batch) {
    bctrl->ctrl_start_frame++;
    bctrl->ctrl_end_frame++;
    s_bundle->delay_frame_idx = 0;
  }
  return TRUE;
}

static boolean module_sensor_flash_led_cal_control(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle)
{
  module_sensor_params_t *led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  uint8_t flash_mode;
  sensor_capture_control_t   *ctrl = &(s_bundle->cap_control);
  sensor_batch_config_t      *bctrl = NULL;
  int32_t rc = TRUE;
  uint8_t ret = TRUE;
  uint32_t max_frame_skip = 0;

  if (ctrl->idx >= ctrl->num_batches) {
    ctrl->enable = 0;
    return TRUE;
  }
  /* Index for the batch*/
  bctrl =  &ctrl->bctrl[ctrl->idx];
  ctrl->delay_batch = 0;

  SHIGH("sof %d",s_bundle->last_idx);
  if (ctrl->trigger) {
    /* Fill start and end for this batch */
    bctrl->ctrl_start_frame = s_bundle->last_idx;
    bctrl->ctrl_end_frame = bctrl->ctrl_start_frame +
      bctrl->interval + bctrl->burst_count - 1;
    SLOW("Start batch %d, start_frame: %d, end_frame: %d",
      ctrl->idx, bctrl->ctrl_start_frame, bctrl->ctrl_end_frame);
    /* Post desired flash mode for each batch in advance */
    sensor_util_post_downstream_event(module, event->identity,
      MCT_EVENT_MODULE_SET_FLASH_MODE, &bctrl->cam_flash_mode);
    ctrl->trigger = 0;
  }

  if (s_bundle->last_idx < bctrl->ctrl_end_frame) {
    if (ctrl->retry_frame_skip >= 0) {
      if (bctrl->flash_mode == LED_FLASH_SET_OFF)
        max_frame_skip = s_bundle->main_flash_off_frame_skip;
      else
        max_frame_skip = s_bundle->main_flash_on_frame_skip;
      if (!ctrl->retry_frame_skip--) {
        /*Initial mode before the flash is fired*/
        SHIGH("Main flash on/off request");
        s_bundle->batch_idx = ctrl->idx;
        bctrl->skip_config_mode = 0;
        ret = module_sensor_post_flash_mode_to_thread(module, &event->identity, s_bundle, NULL);
        if (!ret) {
          SERR("Set flash mode %d failed",bctrl->flash_mode);
          bctrl->led_toggle_pending = FALSE;
        }
      }
    }
  } else {
    /* Reset the controls for next batch */
    SLOW("Control end");
    ctrl->retry_frame_skip = 0;
    if (++ctrl->idx >= ctrl->num_batches) {
      ctrl->enable = 0;
      return TRUE;
    }
    bctrl =  &ctrl->bctrl[ctrl->idx];
    /* reset trigger for next transition */
    ctrl->trigger = 1;
  }

  return TRUE;
}

/** module_sensor_adjust_lowlight_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @identity: stream identity
 *  @s_bundle: sensor bundle handle
 *  @flag: Low Light Mode flag
 *
 *  This function is used to reload chromatix
 *  for OIS/Low light Mode
 *
 *  Return: TRUE for success and FALSE on failure**/
 static boolean module_sensor_adjust_lowlight_chromatix_ptr(
  module_sensor_bundle_info_t *s_bundle,
  mct_module_t *module, uint32_t identity, boolean low_light_flag)
{
  boolean                   ret = TRUE;
  modulesChromatix_t        module_chromatix;
  sensor_chromatix_params_t chromatix_params;
  int32_t                   rc = SENSOR_SUCCESS;
  module_sensor_params_t    *chromatix_module_params = NULL;

  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];

  if (low_light_flag) {
    sensor_util_set_special_mode(&(s_bundle->sensor_common_info),
      SENSOR_SPECIAL_MODE_OIS_CAPTURE, 1);
  } else {
    sensor_util_set_special_mode(&(s_bundle->sensor_common_info),
      SENSOR_SPECIAL_MODE_OIS_CAPTURE, 0);
  }
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
    CHROMATIX_GET_PTR, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0)

  RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(module,
    s_bundle, identity, &chromatix_params, 0));

  return TRUE;
}

/** module_sensor_exp_bkt_control:
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to control the exposure
 *  sequence and correct order of frames picked
 *
 *  Return: TRUE for success and FALSE on failure**/
static boolean module_sensor_exp_bkt_control(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle)
{
  module_sensor_params_t *led_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  uint8_t flash_mode;
  sensor_capture_control_t   *ctrl  = &(s_bundle->cap_control);
  sensor_batch_config_t      *bctrl = NULL;
  int32_t                     rc    = TRUE;
  uint8_t                     ret   = TRUE;
  uint32_t           max_frame_skip = 0;

  /* Index for the batch*/
  bctrl =  &ctrl->bctrl[ctrl->idx];
  if (bctrl->type == CAM_CAPTURE_RESET) {
    memset(ctrl, 0, sizeof(sensor_capture_control_t));
    return TRUE;
  }

  if (ctrl->trigger) {
    bctrl->ctrl_start_frame = s_bundle->last_idx;
    bctrl->ctrl_end_frame = bctrl->ctrl_start_frame +
      bctrl->interval + bctrl->burst_count - 1;
    ctrl->trigger = 0;
  }

  if (s_bundle->delay_frame_cnt) {
    --s_bundle->delay_frame_cnt;
    SHIGH("Requested for Delay frame from ISP : %d", s_bundle->delay_frame_cnt);
    bctrl->ctrl_start_frame++;
    bctrl->ctrl_end_frame++;
    return TRUE;
  }

  if (s_bundle->last_idx < bctrl->ctrl_end_frame) {
    s_bundle->delay_frame_idx = 0;

    if (s_bundle->last_idx ==
      (unsigned int)(bctrl->ctrl_start_frame + bctrl->interval - 1)) {
      /*Post zsl pict done event to hal layer*/
      PTHREAD_MUTEX_LOCK(&s_bundle->capture_control_mutex);
      cam_frame_idx_range_t range;
      memset(&range, 0, sizeof(range));
      PTHREAD_MUTEX_UNLOCK(&s_bundle->capture_control_mutex);
      range.frame_skip_count = 0;
      range.config_batch_idx = ctrl->idx;
      /*Take batch interval into account before posting pic done event
        instead of just relying on pipeline delay. Useful when OIS and flash
        capture are sent in same batch. */
      range.min_frame_idx = s_bundle->last_idx +
        s_bundle->capture_pipeline_delay - (bctrl->interval - 1);
      SHIGH("Send zsl low light take picture done : %d for batch %d",
        range.min_frame_idx, range.config_batch_idx);
      if (sensor_util_post_to_bus(module,
        s_bundle, MCT_BUS_MSG_ZSL_TAKE_PICT_DONE,
        sizeof(cam_frame_idx_range_t), &range) != TRUE) {
        SERR("error posting led frame range msg");
      }
    }

    if (s_bundle->last_idx ==
      (unsigned int)(bctrl->ctrl_end_frame - 1)){
      /* Reset the controls for next batch */
      SLOW("Control end");
      ctrl->max_pipeline_delay = s_bundle->capture_pipeline_delay;
      if (++ctrl->idx >= ctrl->num_batches) {
        ctrl->enable = 0;
        return TRUE;
      }
      bctrl =  &ctrl->bctrl[ctrl->idx];
      /* reset trigger for next transition */
      ctrl->trigger = 1;
    }
  }
  return TRUE;
}

/** module_sensor_restore_unified_evnt
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to restore the params
 *  which are effected in the unified event
 *
 *  Return: TRUE for success and FALSE on failure**/
static int32_t module_sensor_restore_unified_evnt(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle,
  boolean is_delayed)
{
  sensor_capture_control_t *ctrl = &(s_bundle->cap_control);
  int32_t rc = 0;

  if (ctrl->idx){
    if ((ctrl->bctrl[ctrl->idx - 1].low_light_mode == 1) &&
      (s_bundle->last_idx == ctrl->bctrl[ctrl->idx - 1].ctrl_end_frame)){
        rc = module_sensor_adjust_lowlight_chromatix_ptr(s_bundle, module,
        event->identity, FALSE);
      if (rc < 0)
        SERR("OIS Chromatix restore failed");
    }
    ctrl->bctrl[ctrl->idx - 1].low_light_mode = 0;
    if (ctrl->bctrl[ctrl->idx].type == CAM_CAPTURE_RESET &&
      !ctrl->bctrl[ctrl->idx].ctrl_start_frame){
      sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_TRIGGER_CAPTURE_FRAME, NULL);
    }
  }

  if (ctrl->trigger && ctrl->delay_evt[ctrl->idx] == 1 &&
    !is_delayed) {
    if (ctrl->bctrl[ctrl->idx].low_light_mode == 1) {
      rc = module_sensor_adjust_lowlight_chromatix_ptr(s_bundle, module,
        event->identity, TRUE);
      if (rc < 0) {
        SERR("OIS Chromatix update failed");
      }
    }
    /* Send Trigger to Stats for next setting */
    sensor_util_post_downstream_event(module, event->identity,
      MCT_EVENT_MODULE_TRIGGER_CAPTURE_FRAME, NULL);
  }

  return rc;
}

/** module_sensor_capture_control
 *
 *  @module: mct module handle
 *  @event: event associated with stream
 *  @s_bundle: sensor bundle handle
 *
 *  This function is used to control the flash
 *  sequence and correct order of frames picked
 *
 *  Return: TRUE for success and FALSE on failure**/

boolean module_sensor_capture_control(mct_module_t* module,
  mct_event_t* event,  module_sensor_bundle_info_t* s_bundle,
  boolean is_delayed)
{
  module_sensor_params_t *led_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  uint8_t flash_mode;
  sensor_capture_control_t *ctrl = &(s_bundle->cap_control);
  uint8_t ret = TRUE;
  int32_t rc = 0;

  RETURN_ON_NULL(ctrl);

  rc = module_sensor_restore_unified_evnt(module, event,
    s_bundle, is_delayed);

  /* Allow to process delay event based batches only
  at the end of SOF or non delay event based batches
  before SOF*/
  if (ctrl->delay_evt[ctrl->idx] ^ is_delayed)
    return TRUE;

  if (ctrl->enable) {
    switch(ctrl->evt[ctrl->idx]) {
    case SENSOR_AEC_EST_EVT:
      ret = module_sensor_preflash_control(module, event, s_bundle);
      break;
    case SENSOR_ZSL_STOP_CAPTURE_EVT:
    case SENSOR_FLASH_EXP_BKT_EVT:
      ret = module_sensor_flash_exp_bkt_control(module, event, s_bundle);
      break;
    case SENSOR_EXP_BKT_EVT:
      ret = module_sensor_exp_bkt_control(module, s_bundle);
      break;
    case SENSOR_FLASH_LED_CAL_EVT:
      ret = module_sensor_flash_led_cal_control(module, event, s_bundle);
      break;
    case SENSOR_DEFAULT_EVT:
    default:
      break;
    }
  }
  if (!ret)
    ctrl->enable = FALSE;
  return ret;
}

/** module_sensor_delay_en: sensor delay
 *
 *  @s_bundle: sensor bundle handle
 *  @delay_en: delay enable val
 *
 *  Return: 0 for success and negative error on failure
 **/
static int32_t module_sensor_delay_en(module_sensor_bundle_info_t *s_bundle,
  int32_t *delay_en)
{
  uint32_t stream_mask;

  stream_mask = s_bundle->stream_mask;

  if ((CHECK_STREAM(CAM_STREAM_TYPE_SNAPSHOT) &&
    !(CHECK_STREAM(CAM_STREAM_TYPE_VIDEO)) &&
    !(CHECK_STREAM(CAM_STREAM_TYPE_PREVIEW))) ||
    (CHECK_STREAM(CAM_STREAM_TYPE_DEPTH))) {
    /*Exposure settings for depth stream don't
      follow standard mechanism*/
    *delay_en = 0;
  } else {
    *delay_en = 1;
  }
  return 0;
}

/** module_sensor_stream_on: sensor stream on
 *
 *  @module: mct module handle
 *  @event: event associated with stream on
 *  @s_bundle: sensor bundle handle
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function executes stream on sequence based on the
 *  following order:
 *  SENSOR_SET_NEW_RESOLUTION
 *  SENSOR_SET_CSIPHY_CFG
 *  SENSOR_SET_CSID_CFG
 *  SENSOR_LOAD_CHROMATIX --> Chromatix will be loaded only for Bayer cameras
 *  SENSOR_SEND_EVENT
 *  SENSOR_SET_START_STREAM
 **/

boolean module_sensor_stream_on(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean                       ret = TRUE;
  int32_t                       rc = SENSOR_SUCCESS;
  int32_t                       i = 0;
  module_sensor_params_t        *module_sensor_params = NULL;
  sensor_get_t                  sensor_get;
  sensor_output_format_t        output_format;
  int32_t                       stream_on_count;
  mct_stream_info_t* stream_info =
    (mct_stream_info_t*) event->u.ctrl_event.control_event_data;
  sensor_set_res_cfg_t stream_on_cfg, stream_on_cfg_stereo_peer;
  int32_t bundle_id = -1;
  boolean stream_on_flag = FALSE;
  module_sensor_port_bundle_info_t *bundle_info = NULL;
  int32_t delay_en = 1;
  sensor_thread_msg_t msg;
  int32_t nwrite = 0;
  module_sensor_params_t *stereo_peer_module_sensor_params = NULL;

  ATRACE_BEGIN_SNPRINTF(30, "Streamon type %d", stream_info->stream_type);

  memset(&stream_on_cfg, 0, sizeof(sensor_set_res_cfg_t));

  SHIGH("ide %x SENSOR_START_STREAM", event->identity);
  mct_port_t *port = sensor_util_find_src_port_with_identity(
                          module, event->identity);
  if (!port) {
    SERR("cannot find matching port with identity=0x%x",
      event->identity);
    return FALSE;
  }

  /* Store identity by stream type*/
  s_bundle->identity[stream_info->stream_type] = event->identity;

  sensor_util_dump_bundle_and_stream_lists(port, __func__, __LINE__);
  bundle_id = sensor_util_find_bundle_id_for_stream(port, event->identity);

  stream_on_cfg.is_fast_aec_mode_on = FALSE;
  stream_on_cfg.fast_aec_sensor_mode = CAM_HFR_MODE_OFF;
  stream_on_flag = module_sensor_is_ready_for_stream_on(port, event,
    &stream_on_cfg, s_bundle, bundle_id);
  if (!stream_on_flag) {
    SLOW("NO STREAM_ON, dummy excersice");
  } else {
    SLOW("REAL STREAM_ON");
  }
  rc = module_sensor_delay_en(s_bundle, &delay_en);
  if (rc < 0) {
    SHIGH("sensor delay enable failed. non fatal");
    /* Continue START STREAM since this is not FATAL error */
    ret = TRUE;
  }
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_DELAY_CFG, (void*)&delay_en);
  if (rc < 0) {
    SHIGH("SENSOR_SET_DELAY_CFG failed. non fatal");
    /* Continue START STREAM since this is not FATAL error */
    ret = TRUE;
  }

  SLOW("config: dim=%dx%d, mask=0x%x stream type: %d", stream_on_cfg.width,
    stream_on_cfg.height, stream_on_cfg.stream_mask, stream_info->stream_type);
  if (s_bundle->is_stereo_configuration == TRUE) {
    stream_on_cfg_stereo_peer = stream_on_cfg;
    SLOW("sterep peer config: dim=%dx%d, mask=0x%x stream type: %d",
      stream_on_cfg.width, stream_on_cfg.height, stream_on_cfg.stream_mask,
      stream_info->stream_type);
  }

  if (1 == s_bundle->torch_on) {
    module_sensor_params_t *led_module_params =
       s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    if (led_module_params->func_tbl.process != NULL) {
      rc = led_module_params->func_tbl.process(
        led_module_params->sub_module_private, LED_FLASH_SET_TORCH, NULL);
      if (rc < 0) {
        SHIGH("failed: LED_FLASH_SET_TORCH . non fatal");
      } else {
        s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_TORCH;
        s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_FIRED;
        s_bundle->torch_on = 1;
        sensor_util_post_led_state_msg(module, s_bundle, event->identity);
      }
    }
  }

  sensor_submodule_event_type_t ir_cut_event = IR_CUT_SET_ON;
  if (TRUE == stream_on_flag && SENSOR_IR_MODE_ON == s_bundle->ir_mode) {
    sensor_util_set_special_mode(&(s_bundle->sensor_common_info),
        SENSOR_SPECIAL_MODE_IR, 1);
    module_sensor_params_t *ir_module_params =
      s_bundle->module_sensor_params[SUB_MODULE_IR_LED];
    if (ir_module_params->func_tbl.process != NULL) {
      sensor_ir_led_cfg_t cfg;
      cfg.intensity = 0.0f;
      rc = ir_module_params->func_tbl.process(
        ir_module_params->sub_module_private, IR_LED_SET_ON, &cfg);
      if (rc < 0) {
        SERR("failed: led ir set");
        return FALSE;
      }
    }
    if (sensor_fc_store(s_bundle,
      s_bundle->last_idx + 3,
      SENSOR_FRAME_CTRL_REMOVE_IR_CUT_FILTER, &ir_cut_event) == FALSE)
      SERR("failed");
  } else if (SENSOR_IR_MODE_OFF == s_bundle->ir_mode) {
    module_sensor_params_t *ir_module_params =
      s_bundle->module_sensor_params[SUB_MODULE_IR_CUT];
    if (ir_module_params->func_tbl.process != NULL) {
      rc = ir_module_params->func_tbl.process(
        ir_module_params->sub_module_private, IR_CUT_SET_OFF, NULL);
      if (rc < 0) {
        SERR("failed: IR_CUT_SET_OFF");
        return FALSE;
      }
    }
  }

  if (bundle_id == -1 && stream_on_flag == FALSE) {
    SLOW("propagate stream on event");
    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    return TRUE;
  }

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);

  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_SENSOR_FORMAT, &output_format);
  RETURN_FALSE_IF(rc < 0);

  /* Request ISP HW skip before 1st ISP stream on for AE-bracketing */
  if ((output_format == SENSOR_BAYER) && (SECURE != stream_info->is_secure)) {
    if(bundle_id == -1) {
      stream_on_count = s_bundle->stream_on_count;
    } else {
      bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
      RETURN_ON_NULL(bundle_info);
      stream_on_count = bundle_info->stream_on_count;
    }
    if (s_bundle->sensor_common_info.ae_bracket_params.enable == 1 &&
      stream_on_count == 1) {
      RETURN_ON_FALSE(module_sensor_set_frame_skip_for_isp(module, event,
        s_bundle, TRUE, stream_info->stream_type));
    }
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SET_RESOLUTION);
  ret = module_sensor_set_new_resolution_stream_on(module, event, s_bundle,
        module_sensor_params, &stream_on_cfg, stream_info);
  if(ret == FALSE){
     SERR("set new resolution failed");
     ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_RESOLUTION);
     return FALSE;
  }

  if (TRUE == s_bundle->is_stereo_configuration &&
    s_bundle->stereo_peer_s_bundle != NULL) {

    stereo_peer_module_sensor_params = s_bundle->stereo_peer_s_bundle->
        module_sensor_params[SUB_MODULE_SENSOR];
    RETURN_ON_NULL(stereo_peer_module_sensor_params);

    ret = module_sensor_set_new_resolution_stream_on(module, event,
      s_bundle->stereo_peer_s_bundle,
      stereo_peer_module_sensor_params,
      &stream_on_cfg_stereo_peer, stream_info);
    if(ret == FALSE){
       SERR("set new resolution failed");
       ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_RESOLUTION);
       return FALSE;
    }
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_RESOLUTION);

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_CFG_PDAF);
  ret = module_sensor_config_pdaf(module, event, s_bundle);
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_CFG_PDAF);
  /* non fatel error */
  if (ret < SENSOR_SUCCESS) {
    SERR("config pdaf failed");
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SET_STRM_CFG);
  ret = module_sensor_set_cfg_stream_on(&sensor_get, s_bundle);
  if (ret == FALSE) {
    SERR("set stream config failed");
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_STRM_CFG);
    return FALSE;
  }

  /* Set stream config for peer stereo sensor. */
  if (TRUE == s_bundle->is_stereo_configuration &&
    s_bundle->stereo_peer_s_bundle != NULL) {
    ret = module_sensor_set_cfg_stream_on(&sensor_get,
        s_bundle->stereo_peer_s_bundle);
    if (ret == FALSE) {
      SERR("set stream config for peer stereo sensor failed");
      ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_STRM_CFG);
      return FALSE;
    }
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_STRM_CFG);

  if (!s_bundle->sensor_lib_params->sensor_lib_ptr->
    sensor_output.is_depth_sensor) {
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_LOAD_CHROMATIX);
    ret = module_sensor_load_chromatix_stream_on(output_format, &sensor_get,
        s_bundle, event, stream_info, &stream_on_cfg, module);
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_LOAD_CHROMATIX);
    if (ret == FALSE) {
      SERR("set load chromatix failed");
      return FALSE;
    }
  }

  if (TRUE == s_bundle->is_stereo_configuration &&
    s_bundle->stereo_peer_s_bundle != NULL) {
    /* Start sensor stream before stream_on event
       in case of stereo mode */
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_START_STREAM);
    ret = module_sensor_set_start_stream_on(output_format,
        module, event, s_bundle, stream_info, stream_on_flag);
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_START_STREAM);
    if (ret == FALSE) {
      SERR("start stream failed");
      return FALSE;
    }

    ret = module_sensor_send_event_stream_on(module, event);
    if (ret == FALSE) {
      SERR("send event failed");
      return FALSE;
    }
  } else {
    ret = module_sensor_send_event_stream_on(module, event);
    if (ret == FALSE) {
      SERR("send event failed");
      return FALSE;
    }

    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_START_STREAM);
    ret = module_sensor_set_start_stream_on(output_format,
        module, event, s_bundle, stream_info, stream_on_flag);
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_START_STREAM);
    if (ret == FALSE) {
      SERR("start stream failed");
      return FALSE;
    }
  }

  if (s_bundle->fps_info.max_fps) {
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_SET_FPS);
    rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_FPS, &s_bundle->fps_info);
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_SET_FPS);
    RETURN_FALSE_IF(rc < 0);
  }
  ATRACE_END();
  SHIGH("SUCCESS");
  return TRUE;
}

/** module_sensor_set_new_resolution_stream_on:
 *
 * This function sets new resolution during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
boolean module_sensor_set_new_resolution_stream_on(mct_module_t *module,
    mct_event_t *event, module_sensor_bundle_info_t *s_bundle,
    module_sensor_params_t* module_sensor_params,
    sensor_set_res_cfg_t *stream_on_cfg, mct_stream_info_t* stream_info)
{
  boolean retVal = FALSE;
  boolean is_retry;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module_sensor_params);
  RETURN_ON_NULL(stream_on_cfg);
  RETURN_ON_NULL(stream_info);

  s_bundle->res_cfg_done = 0;
  retVal = modules_sensor_set_new_resolution(module, event, s_bundle,
      module_sensor_params, stream_on_cfg, &is_retry, stream_info);
  RETURN_FALSE_IF(retVal < 0);

  SLOW("SENSOR_SET_NEW_RESOLUTION returning %d ", retVal);
  return retVal;
}

/** module_sensor_set_cfg_stream_on:
 *
 * This function sets csiphy and csid cfg during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
boolean module_sensor_set_cfg_stream_on( sensor_get_t* sensor_get,
    module_sensor_bundle_info_t *s_bundle)
{
  boolean retVal = TRUE;
  int16_t rc = SENSOR_SUCCESS;
  uint32_t csid_version = 0;

  RETURN_ON_NULL(sensor_get);
  RETURN_ON_NULL(s_bundle);

  /* check the csid test mode
   * if yes, send csid pattern generation info to csid
   */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_CSID_TEST_MODE, sensor_get, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CSID,
      CSID_SET_TEST_MODE, &sensor_get->csid_test_mode_params, rc);
  RETURN_FALSE_IF(rc < 0);

  sensor_get->csi_cfg.is_plain16 = s_bundle->is_plain16;
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_CUR_CSI_CFG, sensor_get, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CSID,
      CSID_GET_VERSION, &csid_version, rc);
  RETURN_FALSE_IF(rc < 0);

  /* Adjust decode format based on pack mode only on supported hardware. */
  if (sensor_util_is_plain_packing_supported(csid_version)) {
    module_sensor_set_decode_format_based_on_pack_mode(s_bundle, sensor_get);
  }

  sensor_get->csi_cfg.camera_csi_params =
    &s_bundle->sensor_common_info.camera_config.camera_csi_params;
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CSIPHY,
      CSIPHY_SET_CFG, &sensor_get->csi_cfg, rc);
  RETURN_FALSE_IF(rc < 0);

  sensor_get->csi_cfg.phy_sel =
      s_bundle->sensor_info->subdev_id[SUB_MODULE_CSIPHY];
  sensor_get->csi_cfg.csi_params->is_secure =
      s_bundle->stream_size_info.is_secure;

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CSID,
      CSID_SET_CFG, &sensor_get->csi_cfg, rc);
  RETURN_FALSE_IF(rc < 0);

  SLOW("SENSOR_SET_CSI_CFG returning %d ", retVal);
  return retVal;
}

/** module_sensor_send_event_stream_on:
 *
 * This function sends the event during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
boolean module_sensor_send_event_stream_on(mct_module_t *module,
    mct_event_t *event)
{
  boolean retVal = TRUE;

  RETURN_ON_NULL(event);
  RETURN_ON_NULL(module);

  /* Call send_event to propagate event to next module*/
  retVal = sensor_util_post_event_on_src_port(module, event);
  RETURN_FALSE_IF(retVal < 0);

  SLOW("SENSOR_SEND_EVENT returning %d", retVal);
  return retVal;
}

/** module_sensor_config_pdaf:
 *
 * This function gets pdaf init parameters: T3: dowstream config
 * paramters to isp to set up the camif channel T2: sensor call
 * the pdaf init directly with config paramters
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
static boolean module_sensor_config_pdaf(
    mct_module_t *module,
    mct_event_t  *event,
    module_sensor_bundle_info_t *s_bundle)
{
  int32_t                   rc = SENSOR_SUCCESS;
  pdaf_buffer_type_t        pd_type = PDAF_BUFFER_FLAG_INVALID;
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_PDAF_GET_TYPE, &pd_type, rc);
  RETURN_FALSE_IF(rc < 0);

  switch (pd_type)
  {
    case PDAF_BUFFER_FLAG_SPARSE:{
     pdaf_sensor_native_info_t pdaf_native_pattern;
     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
       SENSOR_PDAF_GET_NATIVE_PATTERN, &pdaf_native_pattern, rc);
     /*Browse through the supported camif formats and slect one
      * accepted by PDLIB*/
     module_sensor_fill_supported_camif_data(s_bundle, &pdaf_native_pattern);

     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
       SENSOR_PDAF_SET_BUF_DATA_TYPE,
       &pdaf_native_pattern.buffer_data_type, rc);

     rc = sensor_util_post_downstream_event(module, event->identity,
       MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG, &pdaf_native_pattern);
     if (rc == FALSE) {
       SERR("failed sending downstream event in pdaf T3");
     }
    }
    break;
    case PDAF_BUFFER_FLAG_SEQUENTIAL_LINE:
    case PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL: {
     pdaf_init_info_t                s_pdaf;
     if(s_bundle->formatted_data) {
       s_pdaf.pdaf_cal_data = (void*)&s_bundle->formatted_data->pdafc_2d;
       s_pdaf.macro_dac = s_bundle->formatted_data->afc.macro_dac;
       s_pdaf.infinity_dac = s_bundle->formatted_data->afc.infinity_dac;
       s_pdaf.actuator_sensitivity = s_bundle->actuator_sensitivity;
       s_pdaf.cur_logical_lens_pos = s_bundle->cur_logical_lens_pos;
       SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
       SENSOR_PDAF_INIT, &s_pdaf, rc);
     } else {
       SERR("Pdaf formatted data is unavailable");
     }
    }
    break;
   default:
     SHIGH("unsupported pd type");
     break;
  }
  return rc;
}

/** module_sensor_load_chromatix_stream_on:
 *
 * This function loads chromatix for bayer cameras during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
static boolean module_sensor_load_chromatix_stream_on(
    sensor_output_format_t output_format, sensor_get_t* sensor_get,
    module_sensor_bundle_info_t *s_bundle, mct_event_t *event,
    mct_stream_info_t* stream_info, sensor_set_res_cfg_t *stream_on_cfg,
    mct_module_t *module)
{
  int32_t                   rc = SENSOR_SUCCESS;
  sensor_chromatix_params_t chromatix_params;
  actuator_driver_params_t *af_driver_ptr = NULL;
  red_eye_reduction_type *rer_chromatix = NULL;
  eeprom_set_chroma_af_t eeprom_set;
  sensor_thread_msg_t msg;
  int32_t nwrite = 0;

  memset(&chromatix_params, 0, sizeof(chromatix_params));

  if (SENSOR_BAYER == output_format) {
   RETURN_ON_NULL(event);
   RETURN_ON_NULL(sensor_get);
   RETURN_ON_NULL(s_bundle);
   RETURN_ON_NULL(stream_info);
   RETURN_ON_NULL(stream_on_cfg);
   RETURN_ON_NULL(module);

   if (s_bundle->subsequent_bundle_stream) {
     return TRUE;
   }

   SHIGH("load chromatix for sensor %s",
     s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_slave_info.sensor_name);

   SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
       SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
   RETURN_FALSE_IF(rc < 0);

   SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
       CHROMATIX_GET_PTR, &chromatix_params, rc);
   RETURN_FALSE_IF(rc < 0);

   RETURN_ON_FALSE(sensor_util_validate_chromatix_params(&chromatix_params));

   if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
     int is_dpc;

     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
       EEPROM_DUMP_CALIB_DATA, &chromatix_params , rc);
     RETURN_FALSE_IF(rc < 0);

     /* Get awb grgb data */
     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_WB_GRGB,&s_bundle->frame_ctrl.static_metadata.opt_wb_grgb,rc);
     RETURN_FALSE_IF(rc < 0);

     /* Get dpc data and configure isp dpc */
     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_ISDPC_CALIB, &is_dpc, rc);
     RETURN_FALSE_IF(rc < 0);
     if (is_dpc) {
       uint16_t    sensor_mode;
       int         count;

       SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
           SENSOR_GET_SENSOR_MODE, &sensor_mode, rc);
       RETURN_FALSE_IF(rc < 0);
       count = s_bundle->formatted_data->dpc.dpc_calib[sensor_mode].count;
       if (count > 0) {
         rc = sensor_util_post_downstream_event(module, event->identity,
             MCT_EVENT_MODULE_SET_DEFECTIVE_PIXELS,
             &(s_bundle->formatted_data->dpc.dpc_calib[sensor_mode]));
         if (rc == FALSE) {
           SERR("dpc config failed");
         }
       }
     }
   } else {
     s_bundle->frame_ctrl.static_metadata.opt_wb_grgb = 1;
   }

   if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1) {
     rer_chromatix =
     &(((chromatix_parms_type *) chromatix_params.
         chromatix_ptr[SENSOR_CHROMATIX_ISP])->exposure_data.red_eye_reduction);

     /* Get (RER) data from chromatix */
     SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
         LED_FLASH_SET_RER_CHROMATIX, rer_chromatix, rc);
   }

   RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(module,
        s_bundle, event->identity, &chromatix_params,
        CAMERA_CHROMATIX_MODULE_ALL));
  }
  /* set the iso100 for all sensor type */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_SET_ISO100, chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_3A], rc);
  if (rc < 0) {
      SERR("set iso100 failed");
      return FALSE;
  }
  /* set the multiplier for all sensor type */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_SET_EXP_MULTIPLIER, NULL, rc);
  if (rc < 0) {
      SERR("set exp multiplier failed");
      return FALSE;
  }

  SLOW("Exit");
  return TRUE;
}

/** module_sensor_set_start_stream_on:
 *
 * This function starts the stream during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
static boolean module_sensor_set_start_stream_on(
    sensor_output_format_t output_format, mct_module_t *module,
    mct_event_t *event, module_sensor_bundle_info_t *s_bundle,
    mct_stream_info_t* stream_info, boolean stream_on_flag)
{
  int32_t rc = SENSOR_SUCCESS;
  boolean retVal = TRUE;
  uint32_t bus_index;
  mct_bus_msg_t bus_msg_aec_info;
  mct_bus_msg_t bus_msg_pict_done;
  mct_bus_msg_t bus_msg_sensor_start;
  module_sensor_ctrl_t  *sensor_ctrl;
  module_sensor_params_t *module_sensor_params = NULL;
  module_sensor_params_t *stereo_peer_module_sensor_params = NULL;
  struct timespec ts;
  uint64_t exp_time;
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(stream_info);
  RETURN_ON_NULL(module);

  TIMED_WAIT_ON_EVENT(s_bundle->mutex, s_bundle->cond, ts,
    RES_CONFIG_TIMEOUT, s_bundle->res_cfg_done, TRUE, "res_cfg");

  if (TRUE == s_bundle->is_stereo_configuration &&
     s_bundle->stereo_peer_s_bundle != NULL) {
    TIMED_WAIT_ON_EVENT(s_bundle->stereo_peer_s_bundle->mutex,
      s_bundle->stereo_peer_s_bundle->cond, ts, RES_CONFIG_TIMEOUT,
      s_bundle->stereo_peer_s_bundle->res_cfg_done, TRUE, "res_cfg");

    int32_t sync_mode = CAM_MODE_PRIMARY;
    /* Set sensor settings */
    module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_DUALCAM_SETTINGS, &sync_mode);
    RETURN_FALSE_IF(rc < 0);

    sync_mode = CAM_MODE_SECONDARY;
    stereo_peer_module_sensor_params = s_bundle->stereo_peer_s_bundle->
      module_sensor_params[SUB_MODULE_SENSOR];
    rc = stereo_peer_module_sensor_params->func_tbl.process(
    stereo_peer_module_sensor_params->sub_module_private,
    SENSOR_SET_DUALCAM_SETTINGS, &sync_mode);
    RETURN_FALSE_IF(rc < 0);
  }

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if ((output_format == SENSOR_BAYER) && (SECURE != stream_info->is_secure)) {
    stats_get_data_t stats_get;
    stats_get_data_t *dest_stats_get;
    memset(&stats_get, 0, sizeof(stats_get_data_t));

    /* get initial gain/linecount from AEC */
    rc = module_sensor_get_stats_data(module, event->identity, &stats_get);
    RETURN_FALSE_IF(rc < 0);

    if ((stats_get.flag & STATS_UPDATE_AEC) == 0x00) {
      /* non-fatal error */
      SHIGH("Invalid: No AEC update in stats_get");
    } else {
      /* Send bus msg for passing AEC trigger update */
      if (sizeof(stats_get_data_t)
          > sizeof(s_bundle->aec_metadata.private_data)) {
        SERR("failed");
      } else {
        memcpy(s_bundle->aec_metadata.private_data, &stats_get,
            sizeof(stats_get_data_t));
        memset(&bus_msg_aec_info, 0, sizeof(mct_bus_msg_t));
        bus_msg_aec_info.sessionid = s_bundle->sensor_info->session_id;
        bus_msg_aec_info.type = MCT_BUS_MSG_SET_STATS_AEC_INFO;
        bus_msg_aec_info.size = sizeof(stats_get_data_t);
        SLOW("set stats AEC info, bus msg size %d", bus_msg_aec_info.size);
        bus_msg_aec_info.msg = (void *) &s_bundle->aec_metadata;
        if (mct_module_post_bus_msg(module, &bus_msg_aec_info) == FALSE)
          SERR("failed");

        /* Print source stats get data */
        SDBG("source valid entries %d", stats_get.aec_get.valid_entries);
        for (bus_index = 0; bus_index < stats_get.aec_get.valid_entries;
            bus_index++) {
          SDBG("source g %f lux idx %f", stats_get.aec_get.sensor_gain[bus_index],
              stats_get.aec_get.lux_idx);
        }

        /* Print destination stats get data */
        dest_stats_get =
            (stats_get_data_t *) s_bundle->aec_metadata.private_data;
        SDBG("dest valid entries %d", dest_stats_get->aec_get.valid_entries);
        for (bus_index = 0; bus_index < dest_stats_get->aec_get.valid_entries;
            bus_index++) {
          SDBG("dest g %f lux idx %f",
              dest_stats_get->aec_get.sensor_gain[bus_index],
              dest_stats_get->aec_get.lux_idx);
        }
      }
    }

    sensor_bracket_params_t *flash_bracket_params = NULL;
    sensor_bracket_ctrl_t * flash_ctrl = NULL;
    flash_bracket_params = &(s_bundle->flash_bracket_params);
    flash_ctrl = &(flash_bracket_params->ctrl);

    /* save postview identity as preview one */
    if (CAM_STREAM_TYPE_POSTVIEW == stream_info->stream_type) {
      if (flash_bracket_params->flash_bracket.enable == TRUE) {
        flash_bracket_params->flash_bracket.preview_identity = event->identity;
      }
    }

    /* save snapshot identity */
    if (CAM_STREAM_TYPE_SNAPSHOT == stream_info->stream_type) {
      if (flash_bracket_params->flash_bracket.enable == TRUE) {
        flash_bracket_params->flash_bracket.snapshot_identity = event->identity;
      }
    }

    if (s_bundle->regular_led_trigger == 1
      && s_bundle->hal_version == CAM_HAL_V1) {
        uint8_t flash_mode;
        if (s_bundle->longshot)
          flash_mode = LED_FLASH_SET_PRE_FLASH;
        else
          flash_mode = LED_FLASH_SET_MAIN_FLASH;
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
            flash_mode, NULL, rc);
        if (rc < 0) {
          s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
          s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
          SERR("failed: LED_FLASH_SET_MAIN_FLASH");
        } else {
          s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_ON;
          s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_FIRED;
        }

      sensor_util_post_led_state_msg(module, s_bundle, event->identity);
      SHIGH("post-flash: ON");
      s_bundle->regular_led_trigger = 0;
    }

    /* enable Flash bracketing upon streamon for both streams */
    if (flash_bracket_params->flash_bracket.snapshot_identity
        && flash_bracket_params->flash_bracket.preview_identity) {
      SLOW("current frame %d", s_bundle->last_idx);
      /* @num_skip_exp_linecount :
       *   fixed delay for valid exp and linecount and led off
       * @num_skip_led_up :
       *   fixed delay for LED to take effect
       */
      const uint8_t num_skip_exp_linecount = 3;
      const uint8_t num_skip_led_up = 2;

      rc = sensor_util_set_frame_skip_to_isp(module,
          flash_bracket_params->flash_bracket.preview_identity, SKIP_ALL);
      if (!rc) {
        SERR("sensor_util_set_frame_skip_to_isp failed");
      }
      rc = sensor_util_set_frame_skip_to_isp(module,
          flash_bracket_params->flash_bracket.snapshot_identity, SKIP_ALL);
      if (!rc) {
        SERR("sensor_util_set_frame_skip_to_isp failed");
      }

      /* Fill metadata msg */
      cam_frame_idx_range_t range;
      memset(&range, 0, sizeof(cam_frame_idx_range_t));
      range.min_frame_idx = num_skip_led_up;
      range.max_frame_idx = range.min_frame_idx + num_skip_exp_linecount;
      memset(&bus_msg_pict_done, 0, sizeof(mct_bus_msg_t));
      bus_msg_pict_done.sessionid = s_bundle->sensor_info->session_id;
      bus_msg_pict_done.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
      bus_msg_pict_done.msg = &range;
      bus_msg_pict_done.size = sizeof(cam_frame_idx_range_t);

      /* save frame info to local structure */
      flash_ctrl->min_frame_idx = range.min_frame_idx;
      flash_ctrl->max_frame_idx = range.max_frame_idx;
      SLOW(" Posting flash_bracket msg to bus, sof %d, min:%d max:%d",
          s_bundle->last_idx, range.min_frame_idx, range.max_frame_idx);
      if (mct_module_post_bus_msg(module, &bus_msg_pict_done) != TRUE) {
        SERR("Failed to send flash_bracket msg - non fatal");
      }

      /* enable internally */
      SHIGH("Marking Flash Bracket Enable!!");
      flash_bracket_params->ctrl.enable = TRUE;
    }

    if (sensor_ctrl->is_dual_cam == TRUE &&
      s_bundle->dualc_info.fps_match_enabled == TRUE &&
      s_bundle->dualc_info.streaming_master != CAM_MODE_PRIMARY) {
      sensor_dual_exp_t dual_exp;
      stats_update_t stats_update;

      dual_exp.stats_update = &stats_update;

      sensor_util_post_intramode_event(module, event->identity,
        s_bundle->peer_identity,
        MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_INFO,
        &dual_exp);

      sensor_util_post_intramode_event(module, event->identity,
        s_bundle->peer_identity,
        MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_TIME,
        &exp_time);

      dual_exp.stats_update->aec_update.exp_time =
        ((float)exp_time)/NANO_SEC_PER_SEC;
      dual_exp.stats_update->aec_update.linecount =
        stats_get.aec_get.linecount[0];
      dual_exp.stats_update->aec_update.sensor_gain =
        stats_get.aec_get.sensor_gain[0];

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_CUR_FLL,
        &dual_exp, rc);

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle,
        SUB_MODULE_SENSOR,
        SENSOR_ADJUST_DUALC_SLAVE_LINECOUNT,
        &dual_exp, rc)

      stats_get.aec_get.linecount[0] =
        dual_exp.stats_update->aec_update.linecount;
      stats_get.aec_get.sensor_gain[0] =
        dual_exp.stats_update->aec_update.sensor_gain;
    }
    /* set initial exposure settings, before stream_on */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_AEC_INIT_SETTINGS, (void*) (&(stats_get.aec_get)), rc);
    if (rc < 0) {
      SERR("Sensor AEC INIT failed!!! - non fatal");
    }

    /* Set initial exposure for peer stereo sensor. */
    if (TRUE == s_bundle->is_stereo_configuration &&
        s_bundle->stereo_peer_s_bundle != NULL) {

        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle->stereo_peer_s_bundle, SUB_MODULE_SENSOR,
          SENSOR_SET_AEC_INIT_SETTINGS, (void*) (&(stats_get.aec_get)),
          rc);

        if (rc < 0) {
          SERR("Sensor AEC INIT failed for peer stereo camera!!! - non fatal");
        }
    }

    /* Enable focus bracketing for non zsl snapshot */
    sensor_af_bracket_t *af_bracket_params = &(s_bundle->af_bracket_params);
    if (af_bracket_params->enable == TRUE) {
      RETURN_ON_FALSE(
          module_sensor_start_af_bracketing(module, s_bundle, event));
    } else {
      SLOW("AF_bracketing not enabled");
    }

    if (FALSE == sensor_util_set_digital_gain_to_isp(module, s_bundle,
            event->identity, NULL)) {
      SERR("can't set digital gain - non fatal");
    }
  } /* if bayer */

  if (stream_on_flag == TRUE) {
    if (s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_output.is_depth_sensor) {
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LASER_LED,
        LASER_LED_SET_ON, NULL, rc);
    }

    if (sensor_ctrl->is_dual_cam == TRUE) {
        retVal = module_sensor_dualcamera_streamon(module,event,s_bundle);
    } else {
      SHIGH("ide %x SENSOR_START_STREAM", event->identity);

      /* Stereo sensor peer start stream. */
      if (TRUE == s_bundle->is_stereo_configuration &&
        s_bundle->stereo_peer_s_bundle != NULL) {

        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle->stereo_peer_s_bundle,
          SUB_MODULE_SENSOR, SENSOR_START_STREAM, NULL, rc);
        RETURN_FALSE_IF(rc < 0);
        SHIGH("ide %x SENSOR_START_STREAM on peer stereo bundle", event->identity);

        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
            SENSOR_DELAYED_START_STREAM, NULL, rc);
        RETURN_FALSE_IF(rc < 0);
      } else {
        if (s_bundle->stream_size_info.is_secure == 1 && s_bundle->is_first_stream_on) {
          sensor_csi_config csi_cfg;
          csi_cfg.is_stream_on = 1;
          csi_cfg.phy_sel = s_bundle->sensor_info->subdev_id[SUB_MODULE_CSIPHY];
          SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CSID,
            CSID_SECCAM_RESET_PIPELINE, &csi_cfg, rc);
          RETURN_FALSE_IF(rc < 0);
          s_bundle->is_first_stream_on = 0;
        }
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
            SENSOR_START_STREAM, NULL, rc);
        RETURN_FALSE_IF(rc < 0);
      }

      memset(&bus_msg_sensor_start, 0, sizeof(mct_bus_msg_t));
      bus_msg_sensor_start.sessionid = s_bundle->sensor_info->session_id;
      bus_msg_sensor_start.type = MCT_BUS_MSG_SENSOR_STARTING;
      bus_msg_sensor_start.msg = (void *) &s_bundle->stream_thread_wait_time;
      SLOW("%s: Sending start bus message\n", __func__);
      if (mct_module_post_bus_msg(module, &bus_msg_sensor_start) == FALSE)
        SERR("failed sending sensor start message: non fatal");
    }
  }

  SLOW("SENSOR_START_STREAM returning %d ", retVal);
  return retVal;
}

/** module_sensor_hal_set_parm: process event for
 *  sensor module
 *
 *  @module_sensor_params: pointer to sensor module params
 *  @event_control: pointer to control data that is sent with
 *                 S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles  events associated with S_PARM * */
boolean module_sensor_hal_set_parm(
   module_sensor_params_t *module_sensor_params,
   cam_intf_parm_type_t type, void *parm_data)
{
  int32_t rc = SENSOR_SUCCESS;

  switch(type){
    case CAM_INTF_PARM_SATURATION:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_SATURATION,
        parm_data);
      break;
    case CAM_INTF_PARM_CONTRAST:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_CONTRAST,
        parm_data);
      break;
    case CAM_INTF_PARM_SHARPNESS:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_SHARPNESS,
        parm_data);
      break;
    case CAM_INTF_PARM_EXPOSURE_COMPENSATION:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_EXPOSURE_COMPENSATION, parm_data);
      break;
    case CAM_INTF_PARM_ANTIBANDING:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_ANTIBANDING,
        parm_data);
      break;
    case CAM_INTF_PARM_EFFECT:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_EFFECT,
        parm_data);
      break;
    case CAM_INTF_PARM_WHITE_BALANCE:
      RETURN_ON_NULL(parm_data);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_WHITE_BALANCE,
        parm_data);
      break;
    default:
       SHIGH("Effect %d not implemented", type);
       return TRUE;
  }

  RETURN_FALSE_IF(rc < 0);
  return TRUE;
}

/** module_sensor_handle_meta_stream_info:
 *
 *  @module: sensor module handle
 *
 *  @s_bundle: sensor bundle info containing information of
 *           all sensors
 *
 *  @event_control: set param event
 *
 *  @identity: event identity
 *
 *  Pass requested dimension to sensor to pick sensor output
 *  and pass that information as module event downstream
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_sensor_handle_meta_stream_info(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle,
  mct_event_control_parm_t *event_control, uint32_t identity)
{
  boolean                    ret = TRUE;
  int32_t                    rc = 0;
  uint32_t                   i = 0;
  cam_stream_size_info_t    *stream_size_info = NULL;
  sensor_set_res_cfg_t      *res_cfg = NULL;
  module_sensor_params_t    *module_sensor_params = NULL;
  sensor_get_output_info_t   get_output_dim;
  cam_dimension_t           *stream_sizes = NULL;
  sensor_isp_stream_sizes_t  sensor_isp_info;
  cam_stream_size_info_t    *hal_stream_size_info = NULL;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event_control);

  stream_size_info = &s_bundle->stream_size_info;

  hal_stream_size_info = (cam_stream_size_info_t *)event_control->parm_data;
  *stream_size_info = *hal_stream_size_info;
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);
  RETURN_ON_NULL(module_sensor_params->func_tbl.process);
  RETURN_ON_NULL(module_sensor_params->sub_module_private);

  res_cfg = &get_output_dim.res_cfg;

  /* reset max_width and  max_height*/
  s_bundle->max_width = s_bundle->max_height = 0;

 /* reset is_plain16 mode */
  s_bundle->is_plain16 = 0;

  memset(&sensor_isp_info, 0, sizeof(sensor_isp_info));

  /* Copy the stream_info from hal stucture */
  sensor_isp_info.num_streams = stream_size_info->num_streams;
  for (i = 0; i < sensor_isp_info.num_streams; i++) {
    sensor_isp_info.stream_sizes[i] = stream_size_info->stream_sizes[i];
    sensor_isp_info.type[i] = stream_size_info->type[i];
    sensor_isp_info.is_type[i] = stream_size_info->is_type[i];
    sensor_isp_info.stream_sz_plus_margin[i] =
      stream_size_info->stream_sizes[i];
    sensor_isp_info.postprocess_mask[i] = stream_size_info->postprocess_mask[i];
  }

  ret = sensor_util_post_downstream_event(module, identity,
    MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE, &sensor_isp_info);
  if (rc < 0) {
    SERR("failed MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE rc %d", rc);
    ret = FALSE;
    goto ERROR;
  }

  for (i = 0; i < sensor_isp_info.num_streams; i++) {
    hal_stream_size_info->margins[i] = stream_size_info->margins[i] =
      sensor_isp_info.margins[i];
    hal_stream_size_info->stream_sz_plus_margin[i] =
      stream_size_info->stream_sz_plus_margin[i] =
      stream_size_info->stream_sizes[i] =
      sensor_isp_info.stream_sz_plus_margin[i];

    if (stream_size_info->format[i] == CAM_FORMAT_META_RAW_8BIT ||
      stream_size_info->format[i] == CAM_FORMAT_META_RAW_10BIT)
      continue;

    if (stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB)
       s_bundle->is_plain16 = 1;

    if (stream_size_info->rotation[i] == ROTATE_90 ||
        stream_size_info->rotation[i] == ROTATE_270) {
      if (s_bundle->max_width <
        (uint32_t)stream_size_info->stream_sizes[i].height)
        s_bundle->max_width = stream_size_info->stream_sizes[i].height;
      if (s_bundle->max_height <
        (uint32_t)stream_size_info->stream_sizes[i].width)
        s_bundle->max_height = stream_size_info->stream_sizes[i].width;
    } else {
      if (s_bundle->max_width <
        (uint32_t)stream_size_info->stream_sizes[i].width)
        s_bundle->max_width = stream_size_info->stream_sizes[i].width;
      if (s_bundle->max_height <
        (uint32_t)stream_size_info->stream_sizes[i].height)
        s_bundle->max_height = stream_size_info->stream_sizes[i].height;
    }
  }

  /* Clear res_cfg struct */
  memset(&get_output_dim, 0, sizeof(get_output_dim));
  res_cfg->height = s_bundle->max_height;
  res_cfg->width = s_bundle->max_width;
  rc = module_sensor_get_pick_data(s_bundle, stream_size_info, res_cfg);
  if (rc < 0) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }

  /* update stream_mask based on meta stream info */
  s_bundle->stream_mask = res_cfg->stream_mask;

  get_output_dim.is_secure = stream_size_info->is_secure;
  /* Get sensor output for requested dimension and stream mask */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private, SENSOR_GET_OUTPUT_INFO,
    &get_output_dim);
  if (rc < 0) {
    SERR("failed SENSOR_GET_OUTPUT_DIMENSION rc %d", rc);
    ret = FALSE;
    goto ERROR;
  }

  /* Width is double of the sensor width in stereo mode*/
  if(s_bundle->is_stereo_configuration) {
    get_output_dim.output_info.dim_output.width *= 2;
    get_output_dim.output_info.full_res.width *= 2;
  }

  ret = sensor_util_post_downstream_event(module, identity,
    MCT_EVENT_MODULE_SET_SENSOR_OUTPUT_INFO, &get_output_dim.output_info);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }

  return ret;

ERROR:
  return ret;
}

/** module_sensor_handle_meta_stream_info_for_meta:
 *
 *  @module: sensor module handle
 *
 *  @s_bundle: sensor bundle info containing information of
 *           all sensors
 *
 *  @event_control: set param event
 *
 *  @identity: event identity
 *
 *  Pass requested dimension to sensor to pick sensor output
 *  and pass that information as module event downstream
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_sensor_handle_meta_stream_info_for_meta(
  mct_module_t *module, module_sensor_bundle_info_t *s_bundle,
  mct_event_control_parm_t *event_control, uint32_t identity)
{
  boolean                              ret = TRUE;
  int32_t                              rc = 0;
  int32_t                              res_idx;
  uint32_t                             i = 0, j = 0;
  cam_stream_size_info_t              *stream_size_info = NULL;
  sensor_set_res_cfg_t                *res_cfg = NULL;
  module_sensor_params_t              *module_sensor_params = NULL;
  module_sensor_params_t              *module_external_params = NULL;
  sensor_get_output_info_t             get_output_dim;
  cam_dimension_t                     *stream_sizes = NULL;
  sensor_meta_info_t                   meta_info;
  struct sensor_csid_lut_params_array *lut_params = NULL;
    sensor_isp_stream_sizes_t          sensor_isp_info;
  cam_stream_size_info_t              *hal_stream_size_info = NULL;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event_control);

  stream_size_info = &s_bundle->stream_size_info;
  hal_stream_size_info = (cam_stream_size_info_t *)event_control->parm_data;
  *stream_size_info = *hal_stream_size_info;
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);
  RETURN_ON_NULL(module_sensor_params->func_tbl.process);
  RETURN_ON_NULL(module_sensor_params->sub_module_private);

  res_cfg = &get_output_dim.res_cfg;

  for (i = 0;i < stream_size_info->num_streams; i++) {
    SLOW("i = %d height = %d width = %d stream_type = %d format = %d subformat = %d dt = %d vc = %d", i,
      stream_size_info->stream_sizes[i].height,
      stream_size_info->stream_sizes[i].width,
      stream_size_info->type[i],
      stream_size_info->format[i],
      stream_size_info->sub_format_type[i],
      stream_size_info->dt[i],
      stream_size_info->vc[i]);
  }

  /* reset max_width and  max_height */
  s_bundle->max_width = s_bundle->max_height = 0;

  /* reset is_plain16 mode */
  s_bundle->is_plain16 = 0;

  memset(&sensor_isp_info, 0, sizeof(sensor_isp_info));

  /* Copy the stream_info from hal stucture */
  sensor_isp_info.num_streams = stream_size_info->num_streams;
  for (i = 0; i < sensor_isp_info.num_streams; i++) {
    sensor_isp_info.stream_sizes[i] = stream_size_info->stream_sizes[i];
    sensor_isp_info.type[i] = stream_size_info->type[i];
    sensor_isp_info.stream_sz_plus_margin[i] =
      stream_size_info->stream_sizes[i];
    sensor_isp_info.postprocess_mask[i] = stream_size_info->postprocess_mask[i];
  }

  ret = sensor_util_post_downstream_event(module, identity,
    MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE, &sensor_isp_info);
  if (rc < 0) {
    SERR("failed MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE rc %d", rc);
    return FALSE;
  }

  for (i = 0; i < sensor_isp_info.num_streams; i++) {
    hal_stream_size_info->margins[i] = stream_size_info->margins[i] =
      sensor_isp_info.margins[i];
    hal_stream_size_info->stream_sz_plus_margin[i] =
      stream_size_info->stream_sz_plus_margin[i] =
      stream_size_info->stream_sizes[i] =
      sensor_isp_info.stream_sz_plus_margin[i];

    if (stream_size_info->format[i] == CAM_FORMAT_META_RAW_8BIT ||
      stream_size_info->format[i] == CAM_FORMAT_META_RAW_10BIT)
      continue;

    if (stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG ||
        stream_size_info->format[i] == CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB)
       s_bundle->is_plain16 = 1;

    if (stream_size_info->rotation[i] == ROTATE_90 ||
        stream_size_info->rotation[i] == ROTATE_270) {
      if (s_bundle->max_width <
        (uint32_t)stream_size_info->stream_sizes[i].height)
        s_bundle->max_width = stream_size_info->stream_sizes[i].height;
      if (s_bundle->max_height <
        (uint32_t)stream_size_info->stream_sizes[i].width)
        s_bundle->max_height = stream_size_info->stream_sizes[i].width;
    } else {
      if (s_bundle->max_width <
        (uint32_t)stream_size_info->stream_sizes[i].width)
        s_bundle->max_width = stream_size_info->stream_sizes[i].width;
      if (s_bundle->max_height <
        (uint32_t)stream_size_info->stream_sizes[i].height)
        s_bundle->max_height = stream_size_info->stream_sizes[i].height;
    }
  }

  /* Clear res_cfg struct */
  memset(&get_output_dim, 0, sizeof(get_output_dim));
  res_cfg->height = s_bundle->max_height;
  res_cfg->width = s_bundle->max_width;
  rc = module_sensor_get_pick_data(s_bundle, stream_size_info, res_cfg);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  /* update stream_mask based on meta stream info */
  s_bundle->stream_mask = res_cfg->stream_mask;
  /* Get sensor output for requested dimension and stream mask */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private, SENSOR_GET_OUTPUT_INFO,
    &get_output_dim);
  if (rc < 0) {
    SERR("failed SENSOR_GET_OUTPUT_DIMENSION rc %d", rc);
    return FALSE;
  }

  res_idx = get_output_dim.output_info.res_idx;

  lut_params =
    &s_bundle->sensor_lib_params->sensor_lib_ptr->csid_lut_params_array;
  for (i = 0; i < stream_size_info->num_streams; i++) {
    if (stream_size_info->format[i] == CAM_FORMAT_META_RAW_8BIT ||
      stream_size_info->format[i] == CAM_FORMAT_META_RAW_10BIT) {

      for (j = 0; j < lut_params->lut_params[res_idx].num_cid; j++) {
        if ((stream_size_info->vc[i] ==
          lut_params->lut_params[res_idx].vc_cfg_a[j].cid / 4) &&
          (stream_size_info->dt[i] ==
          lut_params->lut_params[res_idx].vc_cfg_a[j].dt)) {

          meta_info.fmt = stream_size_info->format[i];
          meta_info.dt = stream_size_info->dt[i];
          meta_info.vc = stream_size_info->vc[i];
          SLOW("Index: %d, dt: %d, vc: %d",
            meta_info.fmt, meta_info.dt, meta_info.vc);
          rc = module_sensor_params->func_tbl.process(
            module_sensor_params->sub_module_private, SENSOR_SET_META_DIM,
            &meta_info);
          if (rc < 0) {
            SERR("failed to get dimension rc %d", rc);
            return FALSE;
          }
        }
      }
    }
  }

  return TRUE;
}

/** module_sensor_reload_chromatix: Reload chromatix pointers
 *
 * This function reloads the chromatix pointer by quering and send to down stream
 *  @s_bundle: pointer to sensor bundle
 *  @module: pointer to sensor module
 *  @identity: Identity on which event must be sent
 *
 *  Return: TRUE / FALSE
 *
 **/
static boolean module_sensor_reload_chromatix(
  module_sensor_bundle_info_t *s_bundle, mct_module_t *module,
  uint32_t identity)
{
  int32_t     rc = 0;
  mct_port_t *port = NULL;
  sensor_chromatix_params_t chromatix_params;

  SHIGH("re-load chromatix for sensor %s",
   s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_slave_info.sensor_name);

  memset(&chromatix_params, 0, sizeof(chromatix_params));

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
    CHROMATIX_GET_PTR, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  /* Find port that belongs to incoming identity */
  port = sensor_util_find_src_port_with_identity(module, identity);
  RETURN_ON_NULL(port);

  if (sensor_util_find_is_stream_on(port)) {
    SLOW("post chromatix event downstream");
    RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(
      module, s_bundle, identity, &chromatix_params, 0));
  }

  return TRUE;
}

/** module_sensor_set_scene_mode: Sets the scene mode
 *
 * This function will set the scene mode in sensor
 *  @s_bundle: pointer to sensor bundle
 *  @scene_mode: scene_mode sent from HAL
 *
 **/
static void module_sensor_set_scene_mode(
  module_sensor_bundle_info_t *s_bundle, cam_scene_mode_type scene_mode)
{
    boolean scene_mode_set = TRUE;
    sensor_special_mode special_mode = SENSOR_SPECIAL_GROUP_SCENE_BEGIN;

    SHIGH("requested scene_mode = %d", scene_mode);
    switch(scene_mode) {

    case CAM_SCENE_MODE_LANDSCAPE:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_LANDSCAPE;
      break;
    case CAM_SCENE_MODE_SNOW:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_SNOW;
      break;
    case CAM_SCENE_MODE_BEACH:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_BEACH;
      break;
    case CAM_SCENE_MODE_SUNSET:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_SUNSET;
      break;
    case CAM_SCENE_MODE_NIGHT:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_NIGHT;
      break;
    case CAM_SCENE_MODE_PORTRAIT:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_PORTRAIT;
      break;
    case CAM_SCENE_MODE_BACKLIGHT:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_BACKLIGHT;
      break;
    case CAM_SCENE_MODE_SPORTS:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_SPORTS;
      break;
    case CAM_SCENE_MODE_ANTISHAKE:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_ANTISHAKE;
      break;
    case CAM_SCENE_MODE_FLOWERS:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_FLOWERS;
      break;
    case CAM_SCENE_MODE_CANDLELIGHT:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_CANDLELIGHT;
      break;
    case CAM_SCENE_MODE_FIREWORKS:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_FIREWORKS;
      break;
    case CAM_SCENE_MODE_PARTY:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_PARTY;
      break;
    case CAM_SCENE_MODE_NIGHT_PORTRAIT:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_NIGHT_PORTRAIT;
      break;
    case CAM_SCENE_MODE_THEATRE:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_THEATRE;
      break;
    case CAM_SCENE_MODE_ACTION:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_ACTION;
      break;
    case CAM_SCENE_MODE_AR:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_AR;
      break;
    case CAM_SCENE_MODE_FACE_PRIORITY:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_FACE_PRIORITY;
      break;
    case CAM_SCENE_MODE_BARCODE:
      special_mode = SENSOR_SPECIAL_MODE_SCENE_BARCODE;
      break;
    default:
      scene_mode_set = FALSE;
      SHIGH("Unsupported scene_mode = %d", scene_mode);
      break;
  }
  sensor_util_set_special_mode(&(s_bundle->sensor_common_info), special_mode,
    scene_mode_set);
}

/** module_sensor_event_control_set_parm: process event for
 *  sensor module
 *
 *  @s_bundle: pointer to sensor bundle
 *  @control_data: pointer to control data that is sent with
 *               S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles all events associated with S_PARM * */

boolean module_sensor_event_control_set_parm(
  mct_module_t *module, uint32_t frame_id,
  module_sensor_bundle_info_t *s_bundle,
  mct_event_control_parm_t *event_control,
  uint32_t identity)
{
  boolean                     ret = TRUE;
  int32_t                     rc = SENSOR_SUCCESS;
  uint32_t                    future_frame_id = 0;
  uint32_t                    report_delay = 0;
  sensor_flash_params_t       *flash_params = NULL;
  cam_flash_state_t           flash_state = 0;
  module_sensor_params_t      *module_sensor_params = NULL;
  module_sensor_params_t      *led_module_params = NULL;
  module_sensor_params_t      *actuator_module_params = NULL;
  cam_dimension_t             *dim = NULL;

  RETURN_ON_NULL(event_control);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_SENSOR]);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH]);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR]);
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  led_module_params    = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  actuator_module_params = s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

  RETURN_ON_NULL(module_sensor_params);
  RETURN_ON_NULL(event_control);

  uint32_t delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_frame_applying_delay -
    s_bundle->frame_ctrl.max_sensor_delay;
  uint32_t pipeline_delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_frame_applying_delay;
  uint32_t sensor_delay = s_bundle->frame_ctrl.max_sensor_delay;
  report_delay = s_bundle->frame_ctrl.session_data.
    max_pipeline_meta_reporting_delay;
  flash_params = &s_bundle->flash_params;

  SLOW("event type =%d", event_control->type);
  switch (event_control->type) {
  case CAM_INTF_PARM_FPS_RANGE:
  {
    RETURN_ON_NULL(event_control->parm_data);
    s_bundle->fps_info = *(cam_fps_range_t *)event_control->parm_data;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_FPS, event_control->parm_data);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    break;
  }
  case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL:
    SLOW("UPDATE_DEBUG_LEVEL: Ignored due to centralized logging");
    break;
  case CAM_INTF_PARM_QUADRA_CFA :
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_QUADRA_MODE, event_control->parm_data);
    if (rc < 0) {
      SERR("CAM_INTF_PARM_QUADRA_CFA failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_HFR:
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_HFR_MODE, event_control->parm_data);
    if (rc < 0) {
      SERR("CAM_INTF_PARM_HFR failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_HDR:
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_HDR_AE_BRACKET, event_control->parm_data);
    if (rc < 0) {
      SERR("CAM_INTF_PARM_HDR failed");
      ret = FALSE;
      break;
    }

    cam_exp_bracketing_t *ae_bracket_config =
        (cam_exp_bracketing_t*)event_control->parm_data;
    if (ae_bracket_config->mode == CAM_EXP_BRACKETING_ON) {
      ret = module_sensor_set_hdr_zsl_mode(module, identity, s_bundle);
      if (ret == FALSE) {
        SERR("Failed at sensor but downstream propagation successful");
        ret = TRUE;
      }
      mct_bus_msg_t bus_msg;
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE;
      cam_prep_snapshot_state_t state;
      state = NEED_FUTURE_FRAME;
      bus_msg.msg = &state;
      bus_msg.size = sizeof(cam_prep_snapshot_state_t);
      if (mct_module_post_bus_msg(module, &bus_msg) != TRUE) {
        SERR("Failure posting to the bus!");
      }
    } else
      s_bundle->sensor_common_info.ae_bracket_params.enable = FALSE;
    break;

  case CAM_INTF_META_FLASH_POWER: {
    uint8_t power = *((uint8_t *)event_control->parm_data);
    SLOW("Setting CAM_INTF_META_FLASH_POWER %d \n", power);
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH, LED_FLASH_SET_FIRING_POWER,
      &power, rc);
    if (rc < 0) {
      SERR("CAM_INTF_META_FLASH_POWER failed");
      ret = FALSE;
    }
    break;
  }
  case CAM_INTF_META_FLASH_FIRING_TIME: {
    uint64_t firingTime = *((uint64_t *)event_control->parm_data);
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH, LED_FLASH_SET_FIRING_TIME,
      &firingTime, rc);
    if (rc < 0) {
      SERR("CAM_INTF_META_FLASH_FIRING_TIME failed");
      ret = FALSE;
    }
    break;
  }
  case CAM_INTF_PARM_FOCUS_BRACKETING:
    if (_check_event_pending(&s_bundle->mutex, s_bundle->init_config_done,
      &ret)) {
        sensor_fc_store(s_bundle, s_bundle->last_idx,
          SENSOR_FRAME_CTRL_INT_FOCUS_BKT, (void *)(event_control->parm_data));
        s_bundle->parm_pending = 1;
    } else {
      JUMP_ON_FALSE(ret,ERROR);
      ret = module_sensor_set_param_af_bracket(s_bundle,
        event_control->parm_data);
      if (ret == FALSE) {
        SERR("failed");
      }
    }
    break;
  case CAM_INTF_PARM_FLASH_BRACKETING:{
    sensor_bracket_params_t *flash_bracket_params = NULL;
    cam_flash_bracketing_t        *cam_flash_brkt = NULL;
    RETURN_ON_NULL(event_control);
    RETURN_ON_NULL(event_control->parm_data);

    cam_flash_brkt = (cam_flash_bracketing_t*) event_control->parm_data;
    flash_bracket_params = &(s_bundle->flash_bracket_params);
    /* Currently only expect burst of 2 */
    if (cam_flash_brkt->burst_count != 2) {
      SERR("Invalid flash brkt burst num : %d, abort",
        cam_flash_brkt->burst_count);
      return FALSE;
    }
    SHIGH("set flash_bracket");
    memset(flash_bracket_params, 0, sizeof(sensor_bracket_ctrl_t));
    /* Copy from HAL */
    flash_bracket_params->flash_bracket.burst_count =
        cam_flash_brkt->burst_count;
    flash_bracket_params->flash_bracket.enable = cam_flash_brkt->enable;
    break;
  }
  case CAM_INTF_PARM_VIDEO_HDR:
  case CAM_INTF_PARM_SENSOR_HDR:{
    boolean need_restart = TRUE;
    sensor_out_info_t sensor_out_info;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_HDR_MODE, event_control->parm_data);
    if (rc == SENSOR_FAILURE) {
      SERR("Setting HAL parameter HDR mode failed");
      ret = FALSE;
    } else if (rc == SENSOR_ERROR_INVAL) {
      /* This indicates that we are already in this mode
           so we do not need stream restart */
      need_restart = FALSE;
    }
    sensor_out_info.is_secure =
      s_bundle->sensor_common_info.is_secure;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }

    if((s_bundle->stream_on_count > 0) && need_restart) {
      mct_bus_msg_t bus_msg;
      mct_bus_msg_error_message_t cmd;

      cmd = MCT_ERROR_MSG_RSTART_VFE_STREAMING;
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = GET_SESSION_ID(identity);
      bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
      bus_msg.size = sizeof(cmd);
      bus_msg.msg = &cmd;
      SHIGH("Enable SENSOR_HDR: restart streaming");
      if(!mct_module_post_bus_msg(module, &bus_msg))
        SERR("Failed to send restart streaming message to bus");
    }
  }
  break;
  case CAM_INTF_PARM_DIS_ENABLE:
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_DIS_ENABLE, event_control->parm_data);
    if (rc < 0) {
      SERR("DIS_ENABLE failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_INSTANT_AEC:
    if (*((uint8_t *)event_control->parm_data) == CAM_AEC_FAST_CONVERGENCE)
      s_bundle->fast_aec_required = TRUE;
    else
      s_bundle->fast_aec_required = FALSE;
    break;
  case CAM_INTF_PARM_LONGSHOT_ENABLE : {
    int8_t mode = *((int8_t *)event_control->parm_data);
    s_bundle->longshot = mode;
    break;
  }
  case CAM_INTF_PARM_LED_MODE:
    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1) {
      if (event_control->parm_data) {
        if (_check_event_pending(&s_bundle->mutex, s_bundle->init_config_done,
          &ret)) {
          sensor_led_mode_info_t info;
          info.frame_id = frame_id;
          info.identity = identity;
          info.mode = *(int32_t *)event_control->parm_data;
          info.capture_intent = FALSE;

          sensor_fc_store(s_bundle, s_bundle->last_idx,
            SENSOR_FRAME_CTRL_INT_LED_MODE, (void *)&info);
          s_bundle->parm_pending = 1;
        } else {
          JUMP_ON_FALSE(ret,ERROR);
          if (module_sensor_set_parm_led_mode(module, s_bundle, FALSE,
            *(int32_t *)event_control->parm_data, frame_id, identity) == FALSE)
            SERR("failed: module_sensor_set_parm_led_mode");
        }
      } else {
        SERR("failed: event_control->parm_data is NULL");
      }
    }
    break;
  case CAM_INTF_PARM_BURST_NUM:
    s_bundle->burst_num = *((uint32_t *)event_control->parm_data);
    break;
  case CAM_INTF_PARM_BURST_LED_ON_PERIOD:
    s_bundle->led_on_period = *((uint32_t *)event_control->parm_data);
    break;
  case CAM_INTF_PARM_MAX_DIMENSION:
    dim = ((cam_dimension_t *)event_control->parm_data);
    if (s_bundle->is_stereo_configuration == TRUE) {
      dim->width /= 2L;
    }
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MAX_DIMENSION, event_control->parm_data);
    if (rc < 0) {
      SERR("setting max dimension failed");
      ret = FALSE;
    }
    break;
  case CAM_INTF_PARM_SET_AUTOFOCUSTUNING: {
    if (_check_event_pending(&s_bundle->mutex, s_bundle->init_config_done,
      &ret)) {
      sensor_fc_store(s_bundle, s_bundle->last_idx,
        SENSOR_FRAME_CTRL_INT_SET_AFTUNING, event_control->parm_data);
      s_bundle->parm_pending = 1;
    } else {
      JUMP_ON_FALSE(ret,ERROR);
      if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
        actuator_module_params->func_tbl.process(
          actuator_module_params->sub_module_private,
          ACTUATOR_FOCUS_TUNING, event_control->parm_data);
        if (rc < 0) {
          SERR("failed");
          ret = FALSE;
        }
      }
    }
  }
  break;
  case CAM_INTF_PARM_SET_RELOAD_CHROMATIX:
  case CAM_INTF_PARM_SET_RELOAD_AFTUNE:
    SHIGH("No longer supported as a Cam Interface param");
    break;
  case CAM_INTF_PARM_REDEYE_REDUCTION: {
    int32_t *mode = (int32_t *)event_control->parm_data;

    s_bundle->flash_rer_enable = *mode;

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
      LED_FLASH_SET_RER_PARAMS, mode, rc);
  }
  break;
  case CAM_INTF_PARM_HAL_VERSION: {
    int32_t *hal_version = (int32_t *)event_control->parm_data;
    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    /* update HAL version in s_bundle */
    s_bundle->hal_version = *hal_version;

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_HAL_VERSION, event_control->parm_data);
    if (rc < 0) {
      SERR("set HAL version failed");
      ret = FALSE;
      break;
    }
    break;
  }
  case CAM_INTF_META_CAPTURE_INTENT: {
    uint8_t                       *capture_intent = NULL;
    sensor_submodule_event_type_t flash_mode;
    cam_flash_ctrl_t              flash_ctrl;
    uint32_t                      apply_frame_id = 0;

    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    capture_intent = (uint8_t *)event_control->parm_data;
    s_bundle->capture_intent = *capture_intent;
    SLOW("capture intent = %d, stream on count: %d", *capture_intent,
      s_bundle->stream_on_count);

    if ((*capture_intent == CAM_INTENT_STILL_CAPTURE) &&
        (s_bundle->regular_led_trigger == 1) &&
        s_bundle->stream_on_count > 0) {
      SHIGH("Capture Intent is STILL_CAPTURE %d", frame_id);

      ret = module_sensor_set_parm_led_mode(module, s_bundle, TRUE,
        CAM_FLASH_MODE_SINGLE, frame_id, identity);
      if (ret == FALSE) {
        SERR("failed: module_sensor_set_parm_led_mode");
      }
      s_bundle->regular_led_trigger = 0;
    }

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_CAPTURE_INTENT, event_control->parm_data);
    if (rc < 0) {
      SERR("set capture intent failed");
      ret = FALSE;
      break;
    }
    break;
  }
  case CAM_INTF_META_SENSOR_FRAME_DURATION: {
    int64_t             *frame_duration = NULL;
    sensor_hal_params_t *hal_params = NULL;

    if (!event_control->parm_data) {
      SERR("failed: invalid params");
      ret = FALSE;
      break;
    }
    frame_duration = (int64_t *)event_control->parm_data;
    SLOW("req frame %d frame_duration time %jd", frame_id, *frame_duration);
    SLOW("SOF = %d, requested id = %d", s_bundle->last_idx, frame_id);
    hal_params = &s_bundle->hal_params;

    SLOW("control mode %d ae mode %d", hal_params->control_mode,
      hal_params->ae_mode);

    if ((hal_params->control_mode == CAM_CONTROL_USE_SCENE_MODE) ||
        ((hal_params->control_mode == CAM_CONTROL_AUTO) &&
            (hal_params->ae_mode == CAM_AE_MODE_ON))) {
      break;
    }

    pthread_mutex_lock(&s_bundle->mutex);
    if ((delay + frame_id) > s_bundle->last_idx) {
      pthread_mutex_unlock(&s_bundle->mutex);
      future_frame_id = delay + frame_id;
      SLOW("queue frame %d frame time %lld", future_frame_id,
        (long long int)*frame_duration);
      ret = sensor_fc_store(s_bundle,
        future_frame_id, SENSOR_FRAME_CTRL_FRAME_DURATION,
        event_control->parm_data);
      RETURN_ON_FALSE(ret);
    } else if ((delay + frame_id) < s_bundle->last_idx) {
      SERR("The frame window is missed");
      pthread_mutex_unlock(&s_bundle->mutex);
    } else {
      pthread_mutex_unlock(&s_bundle->mutex);
      SLOW("CAM_INTF_META_SENSOR_FRAME_DURATION");
      future_frame_id = delay + frame_id;
      SLOW("apply frame %d frame time %lld", future_frame_id,
        (long long int)*frame_duration);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_FRAME_DURATION,
        event_control->parm_data);
      if (rc < 0) {
        SERR("set frame duration failed");
        ret = FALSE;
      }
    }
    break;
  }
  case CAM_INTF_META_SENSOR_EXPOSURE_TIME: {
    int64_t *exposure_time = NULL;
      exposure_time = (int64_t *)event_control->parm_data;

    pthread_mutex_lock(&s_bundle->mutex);
    if ((delay + frame_id) > s_bundle->last_idx) {
      pthread_mutex_unlock(&s_bundle->mutex);
      future_frame_id = delay + frame_id;
      SLOW("queue frame %d frame time %lld", future_frame_id,
        (long long int)*exposure_time);
      ret = sensor_fc_store(s_bundle,
        future_frame_id, SENSOR_FRAME_CTRL_MANUAL_EXPOSURE_TIME,
        event_control->parm_data);
      RETURN_ON_FALSE(ret);
    } else if ((delay + frame_id) < s_bundle->last_idx) {
      pthread_mutex_unlock(&s_bundle->mutex);
      SERR("The frame window is missed");
    } else {
      pthread_mutex_unlock(&s_bundle->mutex);
      SLOW("CAM_INTF_META_SENSOR_EXPOSURE_TIME");
      future_frame_id = delay + frame_id;
      SLOW("apply frame %d frame time %lld", future_frame_id,
        (long long int)*exposure_time);
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_MANUAL_EXPOSURE_TIME,
        ((int64_t *)event_control->parm_data));
      if (rc < 0) {
        SERR("set exposure time failed");
        ret = FALSE;
      }
    }
  }
  break;
  case CAM_INTF_META_SENSOR_SENSITIVITY: {
    SLOW("CAM_INTF_META_SENSOR_SENSITIVITY frame %d sensitivity %d",
      frame_id, *((int32_t *)event_control->parm_data));
  }
  break;
  case CAM_INTF_META_MODE: {
    uint8_t *value = (uint8_t *)event_control->parm_data;
    if (!value) {
      SERR("failed: event_control->parm_data %p", event_control->parm_data);
      break;
    }
    s_bundle->hal_params.control_mode = *value;
  }
  break;
  case CAM_INTF_META_AEC_MODE: {
    uint8_t *value = (uint8_t *)event_control->parm_data;
    if (!value) {
      SERR("failed: event_control->parm_data %p", event_control->parm_data);
      break;
    }
    s_bundle->hal_params.ae_mode = *value;
  }
  break;
  case CAM_INTF_META_STREAM_INFO:
    ret = module_sensor_handle_meta_stream_info(module, s_bundle, event_control,
      identity);
    if (ret == FALSE) {
      SERR("failed: module_sensor_handle_meta_stream_info");
      break;
    }
    break;

  case CAM_INTF_PARM_FRAMESKIP:
    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    s_bundle->hal_params.hal_frame_skip_pattern =
        *((int32_t*)event_control->parm_data);
    break;

  case CAM_INTF_META_LENS_OPT_STAB_MODE: {
    uint8_t *value = NULL;
    value = (uint8_t *)CAM_CALLOC(1, sizeof(uint8_t));
    if (!value) {
      SERR("CAM_CALLOC failed");
      break;
    }
    *value = *(uint8_t *)event_control->parm_data;
    sensor_thread_msg_t msg;
    msg.msgtype = OFFLOAD_FUNC;
    msg.offload_func = module_sensor_offload_lens_opt_stab_mode;
    msg.param1 = (void *)value;
    msg.param2 = NULL;
    msg.param3 = NULL;
    msg.param4 = (void *)s_bundle;
    msg.stop_thread = FALSE;
    int32_t nwrite = 0;
    nwrite = write(s_bundle->pfd[1], &msg, sizeof(sensor_thread_msg_t));
    if(nwrite < 0) {
      SERR("Writing into fd failed");
    }
  }
    break;
  case CAM_INTF_META_TEST_PATTERN_DATA: {
    sensor_hal_params_t *hal_params = NULL;
    cam_test_pattern_data_t *test_pattern = NULL;

    if (!event_control->parm_data) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }

    test_pattern = (cam_test_pattern_data_t *)event_control->parm_data;

    if (s_bundle->hal_params.test_pattern_mode == test_pattern->mode) {
      SLOW("CAM_INTF_META_TEST_PATTERN_DATA %d : same with previous setting",
        test_pattern->mode);
      break;
    }
    s_bundle->hal_params.test_pattern_mode = test_pattern->mode;

    hal_params = &s_bundle->hal_params;

    PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
    if ((delay + frame_id) > s_bundle->last_idx) {
      PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
      future_frame_id = delay + frame_id;
      SLOW("queue frame %d", future_frame_id);
      ret = sensor_fc_store(s_bundle,
        future_frame_id, SENSOR_FRAME_CTRL_TEST_PATTERN_DATA,
        test_pattern);
      if (!ret) {
        SERR("Failed to queue frame control test pattern parm");
        return ret;
      }
    } else if ((delay + frame_id) < s_bundle->last_idx) {
      SERR("The frame window is missed");
      PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
    } else {
      PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
      future_frame_id = delay + frame_id;
      SLOW("apply frame %d", future_frame_id);
      if (_check_event_pending(&s_bundle->mutex, s_bundle->init_config_done,
        &ret)) {
        sensor_fc_store(s_bundle, s_bundle->last_idx,
          SENSOR_FRAME_CTRL_INT_TEST_PATTERN, (void *)test_pattern);
        s_bundle->parm_pending = 1;
      } else {
        JUMP_ON_FALSE(ret,ERROR);
        rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private, SENSOR_SET_TEST_PATTERN,
        test_pattern);
        if (rc < 0) {
          SERR("failed");
          ret = FALSE;
        }
      }

      /* Store frame time to be posted later */
      sensor_fc_store(s_bundle,
        future_frame_id, SENSOR_FRAME_CTRL_TEST_PATTERN_DATA, test_pattern);
    }
  }
  break;
  case CAM_INTF_PARM_EXPOSURE_TIME:{
    cam_intf_parm_manual_3a_t *manual_data =
      (cam_intf_parm_manual_3a_t *)event_control->parm_data;
    uint8_t manual_exposure_mode;
    uint64_t exposure_time;
    RETURN_ON_NULL(manual_data);
    exposure_time = manual_data->value;
    manual_exposure_mode = exposure_time > 0 ? 1 : 0;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MANUAL_EXPOSURE_MODE, &manual_exposure_mode);
    if (rc < 0) {
      SERR("Set Manual Exposure Mode failed");
      return FALSE;
    }
    /*Limit increase for minimum 8 sec exposure time*/
    s_bundle->stream_thread_wait_time =
      (exposure_time/1000000000 > 5) ?
        (exposure_time/1000000000 + 3) : 8;
  }
  break;
  case CAM_INTF_PARM_CAPTURE_FRAME_CONFIG:{
    cam_capture_frame_config_t *config =
      (cam_capture_frame_config_t *)event_control->parm_data;
    if (!config) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    memcpy(&s_bundle->hal_frame_batch, config,
      sizeof(cam_capture_frame_config_t));
  }
  break;
  case CAM_INTF_PARM_ISO:{
    cam_intf_parm_manual_3a_t *manual_data =
      (cam_intf_parm_manual_3a_t *)event_control->parm_data;
    int32_t iso_val;
    RETURN_ON_NULL(manual_data);
    iso_val = (int32_t)manual_data->value;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_SET_ISO, &iso_val, rc);
    RETURN_FALSE_IF(rc < 0);
    if(s_bundle->sensor_common_info.output_format == SENSOR_BAYER) {
      ret = module_sensor_reload_chromatix(s_bundle, module, identity);
      RETURN_ON_FALSE(ret);
    }
  }
  break;
  case CAM_INTF_PARM_BESTSHOT_MODE: {
    cam_scene_mode_type *scene_mode_ptr =
      (cam_scene_mode_type *)event_control->parm_data;
    RETURN_ON_NULL(scene_mode_ptr);
    module_sensor_set_scene_mode(s_bundle, *scene_mode_ptr);
    if(*scene_mode_ptr != s_bundle->cur_scene_mode) {
      s_bundle->cur_scene_mode = *scene_mode_ptr;
      RETURN_ON_FALSE(module_sensor_reload_chromatix(s_bundle, module, identity));
    }
  }
  break;
  case CAM_INTF_PARM_DUAL_LED_CALIBRATION:
    s_bundle->dual_led_calib_enabled =
      *((uint32_t *)event_control->parm_data);
    SLOW("dual led calibration: %d", s_bundle->dual_led_calib_enabled);
    break;
  case CAM_INTF_META_STREAM_INFO_FOR_PIC_RES:
    ret = module_sensor_handle_meta_stream_info_for_meta(module, s_bundle,
        event_control, identity);
    if (ret == FALSE) {
        SERR("failed: module_sensor_handle_meta_stream_info_for_meta");
    }
    break;
  case CAM_INTF_META_BINNING_CORRECTION_MODE:{
    boolean need_restart = TRUE;
    sensor_out_info_t sensor_out_info;
    sensor_out_info.is_secure =
      s_bundle->sensor_common_info.is_secure;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_RESOLUTION_INFO, &sensor_out_info);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }

    if((s_bundle->stream_on_count > 0) && need_restart &&
        (sensor_out_info.binning_factor > 1)) {
      mct_bus_msg_t bus_msg;
      mct_bus_msg_error_message_t cmd;

      cmd = MCT_ERROR_MSG_RSTART_VFE_STREAMING;
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = GET_SESSION_ID(identity);
      bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
      bus_msg.size = sizeof(cmd);
      bus_msg.msg = &cmd;
      SHIGH("Enable Binning correction: restart streaming");
      if(!mct_module_post_bus_msg(module, &bus_msg))
        SERR("Failed to send restart streaming message to bus");
    }
  }
  case CAM_INTF_PARM_EXPOSURE_COMPENSATION:{
    ret = module_sensor_hal_set_parm(module_sensor_params,
        event_control->type, event_control->parm_data);
  }
  break;
  case CAM_INTF_PARM_CLOSE_HINT:{
    if (*((uint8_t *)event_control->parm_data) == 1)
      s_bundle->is_camera_closing = 1;
  }
  break;
  default:{
    sensor_output_format_t output_format;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
    if(output_format == SENSOR_YCBCR) {
      /* msm_sensor.c will handle all hal special effect*/
      if (_check_event_pending(&s_bundle->mutex, s_bundle->init_config_done,
        &ret)) {
        sensor_fc_store(s_bundle, s_bundle->last_idx,
          SENSOR_FRAME_CTRL_INT_HAL_SET_PARM, (void *)event_control);
        s_bundle->parm_pending = 1;
      } else {
        JUMP_ON_FALSE(ret,ERROR);
        ret = module_sensor_hal_set_parm(module_sensor_params,
          event_control->type, event_control->parm_data);
      }
    }
  }
  break;
  }
ERROR:
  return ret;
}

/** module_sensor_handle_parm_raw_dimension: process event for
 *  sensor module
 *
 *  @module: pointer to sensor mct module
 *  @s_bundle: handle to sensor bundle
 *  @module_sensor_params: handle to sensor sub module
 *  @identity: identity of current stream
 *  @event_data: event data associated with this event
 *
 *  This function handles CAM_INTF_PARM_RAW_DIMENSION event
 *
 *  Return: TRUE for success
 *          FALSE for failure
 **/
static boolean module_sensor_handle_parm_raw_dimension(
  mct_module_t *module __attribute__((unused)),
  module_sensor_bundle_info_t *s_bundle,
  uint32_t identity __attribute__((unused)),
  void *event_data)
{
  int32_t                     rc = 0;
  boolean                     ret = FALSE;
  sensor_get_raw_dimension_t  sensor_get;
  sensor_set_res_cfg_t        res_cfg;
  module_sensor_params_t     *module_sensor_params = NULL;

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event_data);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_SENSOR]);

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params->func_tbl.process);

  memset(&res_cfg, 0, sizeof(res_cfg));
  res_cfg.height = s_bundle->max_height;
  res_cfg.width = s_bundle->max_width;
  RETURN_ON_FALSE(module_sensor_get_pick_data(s_bundle,
    &s_bundle->stream_size_info, &res_cfg));

  memset(&sensor_get, 0, sizeof(sensor_get));
  sensor_get.raw_dim = event_data;
  sensor_get.stream_mask = s_bundle->stream_mask;
  sensor_get.res_cfg = &res_cfg;

  SLOW("stream mask %x", sensor_get.stream_mask);
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_RAW_DIMENSION, &sensor_get, rc);
  if (rc < 0) {
    SERR("failed: SENSOR_GET_RAW_DIMENSION rc %d", rc);
    ret = FALSE;
  }

  if (s_bundle->is_stereo_configuration == TRUE) {
    cam_sensor_config_t* dimension = sensor_get.raw_dim;
    if (dimension) {
      dimension->width *= 2L;
    }
  }

  return TRUE;
}

/** module_sensor_event_control_get_parm: process event for
 *  sensor module
 *
 *  @module: pointert to sensor mct module
 *  @event: event to be handled
 *  @s_bundle: pointer to sensor bundle for this sensor
 *
 *  This function handles all events associated with G_PARM
 *
 *  Return: TRUE for success
 *          FALSE for failure
 **/
static boolean module_sensor_event_control_get_parm(
   mct_module_t *module, mct_event_t* event,
   module_sensor_bundle_info_t *s_bundle)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  mct_event_control_parm_t    *event_control = NULL;
  module_sensor_params_t      *module_sensor_params = NULL;
  module_sensor_params_t      *chromatix_module_params = NULL;
  module_sensor_params_t      *actuator_module_params = NULL;

  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_SENSOR]);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX]);
  RETURN_ON_NULL(s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR]);

  module_sensor_params =
    s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];
  actuator_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

  event_control =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);

  switch (event_control->type) {
  case CAM_INTF_PARM_GET_CHROMATIX:
  case CAM_INTF_PARM_GET_AFTUNE:
    SERR("No longer supported as a Cam Interface param");
    break;

  case CAM_INTF_PARM_RAW_DIMENSION:
    ret = module_sensor_handle_parm_raw_dimension(module, s_bundle,
      event->identity, event_control->parm_data);
    if (ret == FALSE) {
      SERR("failed: module_sensor_handle_parm_raw_dimension");
    }
    break;
  case CAM_INTF_META_RAW: {
    cam_dimension_t *meta_raw_dim =
      (cam_dimension_t *)event_control->parm_data;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private, SENSOR_GET_META_OUT_DIM,
      meta_raw_dim);
    SLOW("width = %d height = %d", meta_raw_dim->width, meta_raw_dim->height);
    if (rc < 0) {
      SERR("failed SENSOR_GET_META_OUTPUT_DIMENSION rc %d", rc);
      return FALSE;
    }
    break;
  }
  case CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION: {
    cam_related_system_calibration_data_t *dual_data =
      (cam_related_system_calibration_data_t *)event_control->parm_data;

    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
      // (1) Get the data from eeprom
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_SET_CALIBRATE_DUALCAM_PARAM, &s_bundle->dualcam_tune_data, rc);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
      SERR("CAM_INTF_PARM_RELATED_SENSORS_CALIBRATION size %d addr 0x%x",
            s_bundle->dualcam_tune_data.dc_otp_size,
            *(int*)&s_bundle->dualcam_tune_data.dc_otp_params);
      // (2) copy to HAL memory
      memcpy(dual_data, &s_bundle->dualcam_tune_data,
        sizeof(cam_related_system_calibration_data_t));
      // (3) Downstream to imglib
      sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SET_DUAL_OTP_PTR, &s_bundle->dualcam_tune_data);
    }
    break;
  }

  default:
    break;
  }

  return ret;
}

/** module_sensor_event_control_parm_stream_buf: process event for
 *  sensor module
 *
 *  @s_bundle: pointer to sensor bundle
 *  @control_data: pointer to control data that is sent with
 *               S_PARM
 *
 *  Return: TRUE / FALSE
 *
 *  This function handles all events associated with S_PARM * */

static boolean module_sensor_event_control_parm_stream_buf(
   mct_module_t *module,
   mct_event_t* event,
   module_sensor_bundle_info_t *s_bundle __attribute__((unused)))
{
  boolean                      ret = TRUE;

  if (!event) {
    SERR("failed");
    ret = FALSE;
    goto ERROR;
  }

  cam_stream_parm_buffer_t   *stream_parm =
    event->u.ctrl_event.control_event_data;

  switch (stream_parm->type) {
  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO: {
    SLOW("CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO");

    sensor_util_assign_bundle_id(module, event->identity,
       &stream_parm->bundleInfo);
    break;
  }
  default:
    break;
  }

ERROR:
  return ret;
}

/** module_sensor_fill_frame_timing: Fill frame timing info for entire batch
 *
 *  @module: mct module handle
 *  @event: stream off event
 *  @bundle_info: sensor bundle information
 *
 *  Return: true for success and false for failure
 *
 **/
boolean module_sensor_fill_frame_timing(mct_module_t *module __attribute__((unused)),
  mct_event_t *event __attribute__((unused)), sensor_bundle_info_t *bundle_info)
{
  int32_t rc = 0;
  uint32_t i = 0;
  module_sensor_bundle_info_t* s_bundle = bundle_info->s_bundle;
  sensor_capture_control_t *ctrl =
    &s_bundle->cap_control;
  sensor_batch_config_t *bctrl = NULL;
  cam_capture_frame_config_t *hal_batch =
    &s_bundle->hal_frame_batch; /*from HAL*/
  memset(ctrl, 0, sizeof(sensor_capture_control_t));
  /* Parse HAL and fill the mct frame batch timing structure */
  if (!hal_batch->num_batch) {
    SERR("Error: Num of batch is zero");
    return FALSE;
  }
  bctrl = &ctrl->bctrl[0];
  /* Fill sensor local structure */
  ctrl->num_batches = hal_batch->num_batch;
  ctrl->max_pipeline_delay =
    bundle_info->s_bundle->capture_pipeline_delay;
  /* Set control mode to 0 by default */
  ctrl->mode = 0;
  SHIGH("HAL batch info - num_batch = %d", hal_batch->num_batch);
  for (i = 0; i < hal_batch->num_batch; i++) {
    SHIGH("HAL batch info - num_frames = %d, type = %d",
      hal_batch->configs[i].num_frames, hal_batch->configs[i].type);
    bctrl[i].burst_count =
      hal_batch->configs[i].num_frames;
    switch(hal_batch->configs[i].type) {
    case CAM_CAPTURE_FLASH: {
      bctrl[i].type = CAM_CAPTURE_FLASH;
      /* Currently we are putting state as toggle pending for all the
       * batches of type flash. But this is not necessary */
      bctrl[i].led_toggle_pending = TRUE;
      bctrl[i].cam_flash_mode = hal_batch->configs[i].flash_mode;
      ctrl->evt[i] = SENSOR_FLASH_EXP_BKT_EVT;
      ctrl->delay_evt[i] = 1;

      if (bctrl[i].cam_flash_mode == CAM_FLASH_MODE_AUTO ||
        bctrl[i].cam_flash_mode == CAM_FLASH_MODE_ON) {
        bctrl[i].interval
          = MAXQ(s_bundle->capture_pipeline_delay,
              s_bundle->main_flash_on_frame_skip);
        bctrl[i].flash_mode = LED_FLASH_SET_MAIN_FLASH;
        bctrl[i].cam_flash_mode = CAM_FLASH_MODE_ON;
      } else if (bctrl[i].cam_flash_mode == CAM_FLASH_MODE_TORCH) {
        bctrl[i].interval
          = MAXQ(s_bundle->capture_pipeline_delay,
              s_bundle->main_flash_on_frame_skip);
        bctrl[i].flash_mode = LED_FLASH_SET_TORCH;
      } else if (bctrl[i].cam_flash_mode == CAM_FLASH_MODE_OFF) {
        bctrl[i].interval
          = MAXQ(s_bundle->capture_pipeline_delay,
              s_bundle->main_flash_off_frame_skip);
        bctrl[i].flash_mode = LED_FLASH_SET_OFF;
      } else {
        SERR("Failed");
        return FALSE;
      }
      /*If last batch has flash on, then run flash control for stop zsl*/
      if (i == (hal_batch->num_batch - 1) &&
        bctrl[i].cam_flash_mode != CAM_FLASH_MODE_OFF)
        ctrl->mode = 1;
      break;
    }
    case CAM_CAPTURE_BRACKETING:
      bctrl[i].type = CAM_CAPTURE_BRACKETING;
      bctrl[i].interval = 1;
      ctrl->evt[i] = SENSOR_EXP_BKT_EVT;
      bctrl[i].led_toggle_pending = FALSE;
      break;
    case CAM_CAPTURE_MANUAL_3A:
      bctrl[i].type = CAM_CAPTURE_BRACKETING;
      bctrl[i].interval = 1;
      ctrl->evt[i] = SENSOR_EXP_BKT_EVT;
      bctrl[i].led_toggle_pending = FALSE;
      ctrl->delay_evt[i] = 1;
      break;
    case CAM_CAPTURE_LOW_LIGHT:
      bctrl[i].type = CAM_CAPTURE_LOW_LIGHT;
      bctrl[i].interval = 1;
      ctrl->evt[i] = SENSOR_EXP_BKT_EVT;
      bctrl[i].low_light_mode = 1;
      ctrl->delay_evt[i] = 1;
      if (hal_batch->num_batch > 1 && (hal_batch->configs[i + 1].type ==
        CAM_CAPTURE_FLASH))
        bctrl[i].interval = s_bundle->capture_pipeline_delay;
      break;
    case CAM_CAPTURE_RESET:
      /* If HAL sends the Reset batch with num_frmaes = 0 at the end
         of flash sequence then we can initiate flash off immediately
         without waiting for Stop ZSL(therefore ctrl->mode = 0).
         If reset is received for a sequence in which the flash wasn't
         ON previously, then there is no need to do anything! */
      ctrl->mode = 0;
      bctrl[i].type = CAM_CAPTURE_RESET;
      bctrl[i].burst_count = 1;
      if (i > 0 && bctrl[i-1].type == CAM_CAPTURE_FLASH
        && bctrl[i-1].cam_flash_mode == CAM_FLASH_MODE_ON){
        bctrl[i].interval = MAXQ(s_bundle->capture_pipeline_delay,
            s_bundle->main_flash_off_frame_skip);
        bctrl[i].flash_mode = LED_FLASH_SET_OFF;
        bctrl[i].cam_flash_mode = CAM_FLASH_MODE_OFF;
        bctrl[i].led_toggle_pending = TRUE;
      }
      else{
        bctrl[i].interval = 1;
        bctrl[i].led_toggle_pending = FALSE;
      }
      /* In the case of restore the event should
         be same as the previous event. so, now
         Reset case should be handled in the every event.
         And condition to check Reset shouldnt be the
         first event*/
      if (i)
        ctrl->evt[i] = ctrl->evt[i - 1];
      ctrl->delay_evt[i] = 0;
      break;
    case CAM_CAPTURE_LED_CAL: {
      bctrl[i].type = CAM_CAPTURE_LED_CAL;
      bctrl[i].cam_flash_mode = hal_batch->configs[i].flash_mode;
      ctrl->evt[i] = SENSOR_FLASH_LED_CAL_EVT;
      ctrl->delay_evt[i] = 1;
      bctrl[i].interval = 1;
      if (bctrl[i].cam_flash_mode == CAM_FLASH_MODE_ON) {
        bctrl[i].flash_mode = LED_FLASH_SET_MAIN_FLASH;
      } else if (bctrl[i].cam_flash_mode == CAM_FLASH_MODE_OFF) {
        bctrl[i].flash_mode = LED_FLASH_SET_OFF;
      } else {
        SERR("Failed");
        return FALSE;
      }
    }
      break;
    case CAM_CAPTURE_NORMAL:
    default:
      break;
    }
  }

  ctrl->retry_frame_skip = bundle_info->s_bundle->retry_frame_skip;
  ctrl->trigger = 1;
  ctrl->enable = 1;
  return TRUE;
}

/** module_sensor_stream_off: stream off event for sensor module
 *
 *  @module: mct module handle
 *  @event: stream off event
 *  @bundle_info: sensor bundle information
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles stream off event by calling stop
 *  stream / flash off LED / pass stream off event downstream
 **/
static boolean module_sensor_stream_off(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
    boolean                          ret = TRUE;
    int32_t                          rc = 0;
    mct_list_t                       *port_stream_info_list = NULL;
    module_sensor_port_stream_info_t *port_stream_info = NULL;
    mct_port_t                       *port = NULL;
    module_sensor_port_data_t        *port_private = NULL;
    module_sensor_ctrl_t             *module_ctrl = NULL;

    if (!module || !event || !bundle_info) {
      SERR("failed: module %p event %p bundle_info %p", module, event,
        bundle_info);
      return FALSE;
    }
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_SENSOR_STREAMOFF);
    mct_stream_info_t* stream_info = (mct_stream_info_t*)
        event->u.ctrl_event.control_event_data;

    SHIGH("ide %x SENSOR_STOP_STREAM", event->identity);

    module_ctrl = (module_sensor_ctrl_t *)module->module_private;
    module_sensor_params_t *module_sensor_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    if (!module_sensor_params) {
      SERR("failed");
      return FALSE;
    }

    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    RETURN_ON_FALSE(ret);

    if (TRUE == module_sensor_is_ready_for_stream_off(module, event,
      bundle_info->s_bundle)) {
      sensor_af_bracket_t *af_bracket =
        &(bundle_info->s_bundle->af_bracket_params);
      sensor_bracket_params_t *flash_bracket_params =
        &(bundle_info->s_bundle->flash_bracket_params);

      if (af_bracket->enable || flash_bracket_params->flash_bracket.enable) {
        sensor_util_post_braketing_state_event(module,
          bundle_info->s_bundle, event->identity, FALSE, 0);
      }

      /* Check frame-control queue if there's any request that needs to process
         before stream-off */
      sensor_fc_process_now(module, bundle_info->s_bundle, event, FRAME_CTRL_SIZE);

      /* If streaming off preview, then turn off LED*/
      /* When stream off, HAL wipes out FLASH OFF command because
       * aec_mode AUTO is also set and there's a chance that APP doesn't
       * send TORCH_OFF command when mode change.
       * HAL1 does not reset the led request when changes res.
       * We check the stream_type instead of last_flash_request.
      */
      if(((stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
           stream_info->stream_type == CAM_STREAM_TYPE_POSTVIEW ||
           stream_info->stream_type == CAM_STREAM_TYPE_RAW) &&
           bundle_info->s_bundle->hal_version == CAM_HAL_V1)||
           ((bundle_info->s_bundle->last_flash_request == CAM_FLASH_MODE_TORCH ||
           bundle_info->s_bundle->last_flash_request == CAM_FLASH_MODE_SINGLE)
           && bundle_info->s_bundle->hal_version == CAM_HAL_V3) ||
           bundle_info->s_bundle->longshot) {
        SLOW ("stream off preview or snapshot, turn off LED");
        module_sensor_params_t        *led_module_params = NULL;
        led_module_params =
          bundle_info->s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
        if (led_module_params->func_tbl.process != NULL) {
          rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF, NULL);
          if (rc < 0) {
            SERR("failed: LED_FLASH_SET_OFF");
          } else {

            bundle_info->s_bundle->last_flash_request = CAM_FLASH_MODE_OFF;
            bundle_info->s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
            bundle_info->s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
            sensor_util_post_led_state_msg(module, bundle_info->s_bundle,
              event->identity);
          }
        }
      }

      module_sensor_params_t *ir_module_params =
        bundle_info->s_bundle->module_sensor_params[SUB_MODULE_IR_LED];
      if (ir_module_params->func_tbl.process != NULL) {
        rc = ir_module_params->func_tbl.process(
          ir_module_params->sub_module_private, IR_LED_SET_OFF, NULL);
        if (rc < 0) {
          SERR("failed: led ir set");
          return FALSE;
        }
      }

      mct_bus_msg_t bus_msg;
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = bundle_info->s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_SENSOR_STOPPING;
      bus_msg.msg = NULL;
      SLOW("%s: Sending stop bus message\n", __func__);
      if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
        SERR("failed");

      /* Reset chromatix names in chromatix submodule */
      SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle,
        SUB_MODULE_CHROMATIX, CHROMATIX_RESET_NAME, NULL, rc);

      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_PDAF_DEINIT, NULL);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_STOP_STREAM, NULL);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      if (bundle_info->s_bundle->stream_size_info.is_secure == 1 &&
        bundle_info->s_bundle->is_camera_closing) {
        sensor_csi_config csi_cfg;
        csi_cfg.phy_sel = bundle_info->s_bundle->sensor_info->
          subdev_id[SUB_MODULE_CSIPHY];
        csi_cfg.is_stream_on = 0;
        SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_CSID,
          CSID_SECCAM_RESET_PIPELINE, &csi_cfg, rc);
        RETURN_FALSE_IF(rc < 0);
        bundle_info->s_bundle->is_camera_closing = 0;
      }

      if (bundle_info->s_bundle->sensor_lib_params->
        sensor_lib_ptr->sensor_output.is_depth_sensor) {
        SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_LASER_LED,
          LASER_LED_SET_OFF, NULL, rc);
      }

      /* Stereo sensor peer stop stream. */
      if (TRUE == bundle_info->s_bundle->is_stereo_configuration &&
        bundle_info->s_bundle->stereo_peer_s_bundle != NULL) {

        module_sensor_params_t *stereo_peer_module_sensor_params =
          bundle_info->s_bundle->stereo_peer_s_bundle->
          module_sensor_params[SUB_MODULE_SENSOR];

        rc = stereo_peer_module_sensor_params->func_tbl.process(
          stereo_peer_module_sensor_params->sub_module_private,
          SENSOR_STOP_STREAM, NULL);
        if (rc < 0) {
          SERR("failed");
          return FALSE;
        }
      }

      bundle_info->s_bundle->last_idx = 0;
      bundle_info->s_bundle->delay_frame_idx = 0;

      if (stream_info->stream_type != CAM_STREAM_TYPE_PREVIEW) {
        bundle_info->s_bundle->
          sensor_common_info.ae_bracket_params.enable = FALSE;
      }
      bundle_info->s_bundle->max_isp_frame_skip = 0;

      PTHREAD_MUTEX_LOCK(&module_ctrl->dual_cam_mutex);
      module_ctrl->is_dual_stream_on = FALSE;
      module_ctrl->is_dual_streaming = FALSE;
      PTHREAD_MUTEX_UNLOCK(&module_ctrl->dual_cam_mutex);

      /* Reset phase correction parameters when sensor streams off */
      module_sensor_dualcamera_reset_pc_params(bundle_info->s_bundle);

    }else {
       SHIGH("is_ready_for_stream_off returned false");
    }

    if (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
      if (FALSE == module_sensor_set_frame_skip_for_isp(module, event,
        bundle_info->s_bundle, FALSE, stream_info->stream_type)) {
        SERR("failed");
      }
    }

    bundle_info->s_bundle->isp_frameskip[stream_info->stream_type] = 0;
    bundle_info->s_bundle->identity[stream_info->stream_type] = 0;
    bundle_info->s_bundle->cur_scene_mode = -1;
    memset(&(bundle_info->s_bundle->cap_control), 0,
      sizeof(sensor_capture_control_t));
    MCT_PROF_LOG_END();
    ret = sensor_util_set_stream_on(module, event->identity, FALSE);
    ATRACE_CAMSCOPE_END(CAMSCOPE_SENSOR_STREAMOFF);
    RETURN_ON_FALSE(ret);

    return ret;
}

/** module_sensor_process_event: process event for sensor
 *  module
 *
 *  @streamid: streamid associated with event
 *  @module: mct module handle
 *  @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream *   */

static boolean module_sensor_module_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_event_control_t         *event_ctrl = NULL;
  sensor_bundle_info_t         bundle_info;
  struct timespec              ts;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);

  if (event->type != MCT_EVENT_CONTROL_CMD) {
    SERR("failed invalid event type %d",
      event->type);
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  event_ctrl = &event->u.ctrl_event;

  memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
  RETURN_ON_FALSE(sensor_util_get_sbundle(module, event->identity, &bundle_info));
  SLOW("event type %d current_frame_id %d last_idx %d",
    event_ctrl->type, event_ctrl->current_frame_id, bundle_info.s_bundle->last_idx);

  if (bundle_info.s_bundle->block_parm || (event_ctrl->type !=
    MCT_EVENT_CONTROL_SET_PARM && event_ctrl->type !=
    MCT_EVENT_CONTROL_GET_PARM)) {
    TIMED_WAIT_ON_EVENT(bundle_info.s_bundle->mutex,
      bundle_info.s_bundle->cond, ts, INIT_CONFIG_TIMEOUT,
      bundle_info.s_bundle->init_config_done, TRUE, "init_config");
  }

  if (!_check_event_pending(&bundle_info.s_bundle->mutex,
      bundle_info.s_bundle->init_config_done, &ret) &&
      bundle_info.s_bundle->parm_pending) {
    sensor_fc_process_now(module, bundle_info.s_bundle, event, FRAME_CTRL_SIZE-1);
    bundle_info.s_bundle->parm_pending = 0;
  }

  if (event_ctrl->type == MCT_EVENT_CONTROL_PREPARE_SNAPSHOT) {
    sensor_output_format_t output_format;
    mct_bus_msg_t bus_msg;
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    module_sensor_params_t *module_sensor_params = NULL;

    bundle_info.s_bundle->state = 0;
    bundle_info.s_bundle->regular_led_trigger = 0;
    module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
    SLOW("in Prepare snapshot, sensor type is %d\n", output_format);
    if (output_format == SENSOR_YCBCR) {
      bus_msg.sessionid = bundle_info.s_bundle->sensor_info->session_id;
      bus_msg.type = MCT_BUS_MSG_PREPARE_HW_DONE;
      cam_prep_snapshot_state_t state;
      state = DO_NOT_NEED_FUTURE_FRAME;
      bus_msg.msg = &state;
      bus_msg.size = sizeof(cam_prep_snapshot_state_t);
      if (mct_module_post_bus_msg(module,&bus_msg)!=TRUE)
        SERR("Failure posting to the bus!");
      return TRUE;
    }
  }
  switch (event_ctrl->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    MCT_PROF_LOG_BEG(PROF_SENSOR_STREAM_ON, "stream type", module_ctrl->streaminfo.stream_type);
    SLOW("ide %x MCT_EVENT_CONTROL_STREAMON",event->identity);

    /* Set the sensor stream */
    ret = module_sensor_set_sensor_stream(bundle_info.s_bundle);
    BREAK_ON_FALSE(ret);

    memcpy(&module_ctrl->streaminfo, event->u.ctrl_event.control_event_data,
      sizeof(mct_stream_info_t));
    ret = port_sensor_handle_stream_on(module, event, &bundle_info);
    BREAK_ON_FALSE(ret);
    break;
  case MCT_EVENT_CONTROL_STREAMOFF: {
    MCT_PROF_LOG_BEG(PROF_SENSOR_STREAM_OFF, "stream type", module_ctrl->streaminfo.stream_type);
    SLOW("ide %x MCT_EVENT_CONTROL_STREAMOFF", event->identity);
    SLOW("CT_EVENT_CONTROL_STREAMOFF");
    memcpy(&module_ctrl->streaminfo, event->u.ctrl_event.control_event_data,
      sizeof(mct_stream_info_t));
    ret = module_sensor_stream_off(module, event, &bundle_info);
    RETURN_ON_FALSE(ret);
    break;
  }
  case MCT_EVENT_CONTROL_SET_PARM: {
      mct_event_control_parm_t    *event_control =
        (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
      ret = module_sensor_event_control_set_parm(
      module, event->u.ctrl_event.current_frame_id, bundle_info.s_bundle,
      (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data,
      event->identity);
    if (ret == FALSE) {
      SERR("control set parama failed");
    }

    module_sensor_params_t *module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

    sensor_output_format_t output_format;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_SENSOR_FORMAT, &output_format);
    if(output_format == SENSOR_BAYER ||
      (event_control->type == CAM_INTF_META_STREAM_INFO ) ||
      (event_control->type == CAM_INTF_PARM_USERZOOM ) ||
      (event_control->type == CAM_INTF_PARM_FD ) ||
      (event_control->type == CAM_INTF_PARM_VT) ||
      (event_control->type == CAM_INTF_META_USE_AV_TIMER) ||
      (event_control->type == CAM_INTF_PARM_FPS_RANGE) ||
      (event_control->type == CAM_INTF_PARM_VFE1_RESERVED_RDI)) {
      /* Call send_event to propogate event to next module*/
      ret = sensor_util_post_event_on_src_port(module, event);
      if (ret == FALSE) {
        SERR("post event on src port failed");
        return FALSE;
      }

      if (event_control->type == CAM_INTF_META_STREAM_INFO &&
        bundle_info.s_bundle->stream_size_info.is_secure == 1) {
        sensor_csi_config         csi_cfg;
        cam_stream_size_info_t    *hal_stream_size_info = NULL;
        mct_event_control_parm_t  *event_data =
          (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data;
        RETURN_ON_NULL(event_data);
        hal_stream_size_info = (cam_stream_size_info_t *)event_data->parm_data;
        if (hal_stream_size_info->num_streams > 0) {
          csi_cfg.topology = bundle_info.s_bundle->topology =
            hal_stream_size_info->topology;
          csi_cfg.phy_sel = bundle_info.s_bundle->
            sensor_info->subdev_id[SUB_MODULE_CSIPHY];
          csi_cfg.camera_csi_params = &bundle_info.s_bundle->
            sensor_common_info.camera_config.camera_csi_params;
          SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info.s_bundle, SUB_MODULE_CSID,
            CSID_SECCAM_SEND_TOPOLOGY, &csi_cfg, rc);
          RETURN_FALSE_IF(rc < 0);
        }
      }
    }
    break;
  }
  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
  SLOW("MCT_EVENT_CONTROL_SET_SUPER_PARM");
  ret = sensor_util_post_event_on_src_port(module, event);
  RETURN_ON_FALSE(ret);
  MCT_PROF_LOG_BEG(PROF_SENSOR_SP);
  ret = module_sensor_event_control_set_super_parm(module, event,
    bundle_info.s_bundle);
  MCT_PROF_LOG_END();
  if (ret == FALSE) {
    SERR("failed control supper param");
  }

  break;
  case MCT_EVENT_CONTROL_GET_PARM: {
    ret = module_sensor_event_control_get_parm(
       module, event, bundle_info.s_bundle);

    if (ret == FALSE) {
      SERR("get param failed");
    }
    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("post event on src port failed");;
      return FALSE;
    }
    break;
  }

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
    if(bundle_info.s_bundle->hal_version == CAM_HAL_V1) {
      ret = module_sensor_event_control_parm_stream_buf(
        module, event, bundle_info.s_bundle);

      if (ret == FALSE) {
        SERR("stream buf failed");
      }
    }

    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    break;
  }

  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
      uint32_t      max_led_on_period = 0;
      uint32_t      fps = 0;
      float         cur_fps = 0.0;
      uint32_t      sensor_pipeline_delay;
      uint8_t       delay_flash_on = 0;
      stats_get_data_t        stats_get;
      sensor_output_format_t  output_format;
      module_sensor_params_t *module_sensor_params = NULL;
      module_sensor_params_t *actuator_module_params = NULL;
      sensor_bracket_params_t  *flash_bracket_params =
        &(bundle_info.s_bundle->flash_bracket_params);
      int32_t delay_en = 0;
      module_sensor_bundle_info_t *s_bundle = bundle_info.s_bundle;

      MCT_PROF_LOG_BEG(PROF_SENSOR_PROCESS_EVENT_START_ZSL_SNAPSHOT);

      /* Get Max led on period */
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
          LED_FLASH_GET_MAX_DURATION, &max_led_on_period, rc);
      if (rc < 0) {
        SERR("failed to get flash max duration, falling back to default");
        max_led_on_period = MAX_LED_ON_DURATION_DEFAULT;
      }

      SLOW("max_led_on_period set to %d",max_led_on_period);

      memset(&stats_get, 0, sizeof(stats_get_data_t));
      module_sensor_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
      actuator_module_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];

      sensor_pipeline_delay = bundle_info.s_bundle->
        sensor_lib_params->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

      SHIGH("ZSL snapshot start ");
      ret = sensor_util_post_event_on_src_port(module, event);

      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_DELAY_CFG, (void*)&delay_en);
      if (rc < 0) {
        SERR("failed - non fatal");
        /* Continue START STREAM since this is not FATAL error */
        ret = TRUE;
      }

      rc = module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
      if (rc < 0) {
        SERR("get sensor format failed");
      } else {
        SLOW(" Get data from stats");
      }

      /* Currently enabled Only for chroma flash */
      ret = module_sensor_fill_frame_timing(module, event, &bundle_info);
      if (ret)
        goto DONE;
      else
        SHIGH("Warn in frame timing, Fallback to default");

      ret = sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_STATS_GET_DATA, &stats_get);
       if (ret == FALSE) {
          SERR("failed");
          /* Continue START STREAM since this is not FATAL error */
          ret = TRUE;
        } else if (stats_get.flag & STATS_UPDATE_AEC) {

          aec_update_t aec_update;
          aec_update.sensor_gain = stats_get.aec_get.sensor_gain[0];
          aec_update.linecount = stats_get.aec_get.linecount[0];
          aec_update.hdr_sensitivity_ratio = stats_get.aec_get.hdr_sensitivity_ratio;
          aec_update.hdr_exp_time_ratio = stats_get.aec_get.hdr_exp_time_ratio;
          aec_update.total_drc_gain = stats_get.aec_get.total_drc_gain;
          SLOW("get stats led trigger %d ",stats_get.aec_get.trigger_led);
          if (stats_get.aec_get.trigger_led) {
            module_sensor_params_t        *led_module_params = NULL;
            led_module_params =
                bundle_info.s_bundle->
                  module_sensor_params[SUB_MODULE_LED_FLASH];

            bundle_info.s_bundle->led_off_gain
                = stats_get.aec_get.led_off_sensor_gain;
            bundle_info.s_bundle->led_off_linecount
                = stats_get.aec_get.led_off_linecount;
            if (!(!bundle_info.s_bundle->partial_flash_frame_skip ||
                flash_bracket_params->flash_bracket.enable ||
                bundle_info.s_bundle->burst_num > 1 ||
                bundle_info.s_bundle->longshot))
              delay_flash_on = 1;

          if (led_module_params->func_tbl.process != NULL && !delay_flash_on) {
            uint8_t flash_mode;
            cam_flash_mode_t cam_flash_mode;
            if (bundle_info.s_bundle->longshot) {
              flash_mode = LED_FLASH_SET_TORCH;
              cam_flash_mode = CAM_FLASH_MODE_TORCH;
            } else {
               flash_mode  = LED_FLASH_SET_MAIN_FLASH;
               cam_flash_mode = CAM_FLASH_MODE_ON;
            }
            SHIGH("Sending flash mode downstream - %d", cam_flash_mode);
            sensor_util_post_downstream_event(module, event->identity,
              MCT_EVENT_MODULE_SET_FLASH_MODE, &cam_flash_mode);

            rc = led_module_params->func_tbl.process(
                   led_module_params->sub_module_private,
                   flash_mode , NULL);

            if (rc < 0) {
              bundle_info.s_bundle->sensor_params.flash_mode =
                CAM_FLASH_MODE_OFF;
              bundle_info.s_bundle->sensor_params.flash_state =
                CAM_FLASH_STATE_READY;
              SERR("failed: LED_FLASH_SET_MAIN_FLASH");
            } else {
              bundle_info.s_bundle->sensor_params.flash_mode =
                CAM_FLASH_MODE_ON;
              bundle_info.s_bundle->sensor_params.flash_state =
                CAM_FLASH_STATE_FIRED;
            }
            sensor_util_post_led_state_msg(module, bundle_info.s_bundle,
                event->identity);
          }
          bundle_info.s_bundle->regular_led_trigger = 0;
          SLOW(" led zsl capture start ");

              /* Handle Burst Mode */
              SLOW("burst mode: num_buf_requested:%u, burst_led_on_period:%u",
                bundle_info.s_bundle->burst_num,
                bundle_info.s_bundle->led_on_period);
              if(bundle_info.s_bundle->burst_num > 1) {
                SHIGH(" LED ZSL BURST ");
                if(bundle_info.s_bundle->led_on_period > max_led_on_period)
                  bundle_info.s_bundle->led_on_period = max_led_on_period;

                RETURN_ON_FALSE(sensor_util_post_submod_event(
                   bundle_info.s_bundle,
                   SUB_MODULE_SENSOR,SENSOR_GET_CUR_FPS, &fps));

                cur_fps = (float)(fps / 256.0);

                bundle_info.s_bundle->led_off_count =
                 (int16_t)((float)(bundle_info.s_bundle->led_on_period)
                 / 1000.0 * cur_fps);

                SLOW("cur_fps: %f", cur_fps);
              }

              /* Fill metadata msg */
              mct_bus_msg_t bus_msg;
              bus_msg.sessionid = bundle_info.session_id;
              bus_msg.type = MCT_BUS_MSG_ZSL_TAKE_PICT_DONE;
              cam_frame_idx_range_t      range;

              memset(&range, 0, sizeof(cam_frame_idx_range_t));
              SLOW("current frame %d", bundle_info.s_bundle->last_idx);

              if (flash_bracket_params->flash_bracket.enable == TRUE) {
                RETURN_ON_FALSE(module_sensor_start_flash_bracket_entry(module,
                        &bundle_info, event));
              } else {
                /* Below data will be fetched from sensor */
                range.min_frame_idx = bundle_info.s_bundle->last_idx +
                  bundle_info.s_bundle->main_flash_on_frame_skip + 1;

                if(bundle_info.s_bundle->led_off_count > 0) {
                  range.max_frame_idx = range.min_frame_idx +
                    (uint16_t)bundle_info.s_bundle->led_off_count;
                  range.frame_skip_count = sensor_pipeline_delay + 2;
                } else {
                  /* 13 is legacy value for maximum frame idx expected
                   * by hal layer*/
                  range.max_frame_idx = bundle_info.s_bundle->last_idx +
                    13;
                  range.frame_skip_count = 0;
                  /* Enable flash control for legacy flash */
                  if ((bundle_info.s_bundle->partial_flash_frame_skip) &&
                      !bundle_info.s_bundle->longshot) {
                    bundle_info.s_bundle->hal_frame_batch.num_batch = 1;
                    bundle_info.s_bundle->hal_frame_batch.configs[0].num_frames = 1;
                    bundle_info.s_bundle->hal_frame_batch.configs[0].type = CAM_CAPTURE_FLASH;
                    bundle_info.s_bundle->hal_frame_batch.configs[0].flash_mode = CAM_FLASH_MODE_ON;
                    module_sensor_fill_frame_timing(module, event, &bundle_info);
                  }
                }

                if(!delay_flash_on)
                  if (sensor_util_post_to_bus(module,
                    bundle_info.s_bundle, MCT_BUS_MSG_ZSL_TAKE_PICT_DONE,
                    sizeof(cam_frame_idx_range_t), &range) != TRUE) {
                    SERR("error posting led frame range msg");
                  }

            }
          } else {
            SHIGH("LED Trigger disabled.");
          }
          rc = module_sensor_params->func_tbl.process(
            module_sensor_params->sub_module_private,
            SENSOR_SET_AEC_UPDATE, &aec_update);

          ret = sensor_util_set_digital_gain_to_isp(module,
              bundle_info.s_bundle, event->identity, NULL);
              if (ret == FALSE) {
                SERR("failed");
                /* Continue START STREAM since this is not FATAL error */
                ret = TRUE;
              }

          if (module_ctrl->is_dual_cam == TRUE &&
            s_bundle->dualc_info.fps_match_enabled == TRUE &&
            s_bundle->dualc_info.streaming_master == CAM_MODE_PRIMARY) {
            module_sensor_send_aec_update_to_slave(module, event,
              s_bundle, &aec_update);
          }

          /* set exposure settings for zsl */
          rc = module_sensor_params->func_tbl.process(
                  module_sensor_params->sub_module_private,
                  SENSOR_SET_AEC_ZSL_SETTINGS, (void*)(&(stats_get.aec_get)));
          if (rc < 0) {
            SERR("set ZSL setting failed");
            return FALSE;
          }
       }

DONE:
          if (stats_get.aec_get.valid_entries > 1 &&
            bundle_info.s_bundle->sensor_common_info.ae_bracket_params.enable) {
            if (FALSE == module_sensor_set_frame_skip_for_isp(module, event,
              bundle_info.s_bundle, TRUE, CAM_STREAM_TYPE_SNAPSHOT)) {
              SERR("failed");
            }
          }

       MCT_PROF_LOG_END();
      /* Enable focus bracketing upon zsl snapshot */
      sensor_af_bracket_t *af_bracket_params =
        &(bundle_info.s_bundle->af_bracket_params);
      if (af_bracket_params->enable == TRUE) {
        RETURN_ON_FALSE(module_sensor_start_af_bracketing(module,
          bundle_info.s_bundle, event));
      }
    break;
  }

  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT:{
    uint32_t snapshot_identity;
    int32_t delay_en = 1;
    module_sensor_params_t *module_sensor_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    module_sensor_bundle_info_t *s_bundle = bundle_info.s_bundle;
    SHIGH("stop ZSL snapshot, led off\n");
    module_sensor_params_t        *led_module_params = NULL;
    sensor_capture_control_t *ctrl =
      &bundle_info.s_bundle->cap_control;
    sensor_batch_config_t *bctrl = &ctrl->bctrl[0];

    MCT_PROF_LOG_BEG(PROF_SENSOR_PROCESS_EVENT_STOP_ZSL_SNAPSHOT);
    led_module_params =
      bundle_info.s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];

    /*The mode flag is set if its not desired to wait for stop zsl
     * command to turn of led and not self trigger from control sof
     * Default value is 0 */
    if (ctrl->mode) {
      memset(ctrl, 0, sizeof(sensor_capture_control_t));
      ctrl->evt[ctrl->idx] = SENSOR_ZSL_STOP_CAPTURE_EVT;
      ctrl->num_batches = 1;
      ctrl->max_pipeline_delay = bundle_info.s_bundle->main_flash_off_frame_skip;

      bctrl->interval = bundle_info.s_bundle->main_flash_off_frame_skip;
      bctrl->ctrl_start_frame =
        bundle_info.s_bundle->last_idx + 1;
      bctrl->ctrl_end_frame =
        bctrl->ctrl_start_frame + bctrl->interval;
      bctrl->type = CAM_CAPTURE_FLASH;
      bctrl->flash_mode = LED_FLASH_SET_OFF;
      bctrl->led_toggle_pending = TRUE;

      ctrl->retry_frame_skip = bundle_info.s_bundle->retry_frame_skip;
      SHIGH("Sending flash mode downstream - %d", bctrl->cam_flash_mode);
      sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_SET_FLASH_MODE, &bctrl->cam_flash_mode);

      ctrl->enable = 1;
    } else if (led_module_params->func_tbl.process != NULL) {
      rc = led_module_params->func_tbl.process(
            led_module_params->sub_module_private,
            LED_FLASH_SET_OFF, NULL);
      if (rc < 0) {
        SERR("failed: LED_FLASH_SET_OFF");
      } else {
        PTHREAD_MUTEX_LOCK(&bundle_info.s_bundle->capture_control_mutex);
        bundle_info.s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
        bundle_info.s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
        PTHREAD_MUTEX_UNLOCK(&bundle_info.s_bundle->capture_control_mutex);
        sensor_util_post_led_state_msg(module, bundle_info.s_bundle,
          event->identity);
        SHIGH("Restoring LED gains in ZSL STOP SNAP");
        aec_update_t aec_update;
        aec_update.sensor_gain= s_bundle->led_off_gain;
        aec_update.linecount = s_bundle->led_off_linecount;
        SHIGH("restore: led_off stats gain %f lnct %d", aec_update.sensor_gain,
          aec_update.linecount);
        RETURN_ON_FALSE(sensor_util_post_submod_event(s_bundle, SUB_MODULE_SENSOR,
          SENSOR_SET_AEC_UPDATE, &aec_update));

        if (module_ctrl->is_dual_cam == TRUE &&
          s_bundle->dualc_info.fps_match_enabled == TRUE &&
          s_bundle->dualc_info.streaming_master == CAM_MODE_PRIMARY) {
          module_sensor_send_aec_update_to_slave(module, event,
            s_bundle, &aec_update);
        }
      }
    }

    SHIGH("post-flash: OFF");
    ret = module_sensor_set_frame_skip_for_isp(module, event,
      bundle_info.s_bundle, FALSE, CAM_STREAM_TYPE_SNAPSHOT);
    if (ret == FALSE)
      SERR("failed");

    memset(&bundle_info.s_bundle->hal_frame_batch, 0,
      sizeof(cam_capture_frame_config_t));
    bundle_info.s_bundle->sensor_common_info.ae_bracket_params.enable = FALSE;
    bundle_info.s_bundle->max_isp_frame_skip =
      bundle_info.s_bundle->isp_frameskip[CAM_STREAM_TYPE_PREVIEW];
    bundle_info.s_bundle->regular_led_trigger = 0;

      ret = sensor_util_post_event_on_src_port(module, event);

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_DELAY_CFG, (void*)&delay_en);
    if (rc < 0) {
      SERR("set delay config failed - non fatal");
      /* Continue START STREAM since this is not FATAL error */
      ret = TRUE;
    }
    MCT_PROF_LOG_END();
    break;
  }
  case MCT_EVENT_CONTROL_DO_AF:{
       sensor_output_format_t output_format;
        module_sensor_params_t *module_sensor_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
        module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
        if(output_format == SENSOR_YCBCR) {
          sensor_info_t sensor_info;
          sensor_info.module = module;
          sensor_info.session_id =
            bundle_info.s_bundle->sensor_info->session_id;
          sensor_info.write_fd = bundle_info.s_bundle->pfd[1];
          module_sensor_params->func_tbl.process(
                module_sensor_params->sub_module_private,
                SENSOR_SET_AUTOFOCUS, &sensor_info);
          SHIGH("Setting Auto Focus");
        }
        else {
          ret = sensor_util_post_event_on_src_port(module, event);
        }
    break;
  }
  case MCT_EVENT_CONTROL_CANCEL_AF:{
       sensor_output_format_t output_format;
        module_sensor_params_t *module_sensor_params =
        bundle_info.s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
        module_sensor_params->func_tbl.process(
              module_sensor_params->sub_module_private,
              SENSOR_GET_SENSOR_FORMAT, &output_format);
        if(output_format == SENSOR_YCBCR) {
          module_sensor_params->func_tbl.process(
                module_sensor_params->sub_module_private,
                SENSOR_CANCEL_AUTOFOCUS, module);
        SHIGH("Cancelling Auto Focus");
        sensor_cancel_autofocus_loop();
        }
        else {
          ret = sensor_util_post_event_on_src_port(module, event);
        }
    break;
  }
  case MCT_EVENT_CONTROL_SOF:
    ATRACE_BEGIN("sensor SOF");
    if (module_ctrl->is_dual_stream_on == TRUE &&
      module_ctrl->is_dual_streaming == FALSE) {
      module_sensor_bundle_info_t *peer_bundle;
      peer_bundle = sensor_util_get_peer_bundle(module, bundle_info.s_bundle);

      if (peer_bundle && peer_bundle->last_idx > 0) {
        SLOW("[dual] both sensors are streaming out");
        module_ctrl->is_dual_streaming = TRUE;
      } else {
        SLOW("[dual] peer is not streaming out yet");
      }
    }

    MCT_PROF_LOG_BEG(PROF_SENSOR_SOF);
    REPORT_ON_FALSE(module_sensor_handle_sof_notify(
        module, event, bundle_info.s_bundle), ret);
    MCT_PROF_LOG_END();
    REPORT_ON_FALSE(sensor_util_post_event_on_src_port(module, event), ret);
    if (module_sensor_capture_control(module,event,bundle_info.s_bundle,TRUE)
      == FALSE) {
      SERR(" Capture control failed in post SOF event");
    }
    bundle_info.s_bundle->delay_frame_cnt = 0;
    ATRACE_END();
    break;
  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
      ret = module_sensor_dualcamera_link_sessions
        (module, event, &bundle_info);
    }
    break;
  case MCT_EVENT_CONTROL_HW_WAKEUP:
  {
    ret = module_sensor_dualcamera_hw_wakeup
      (module, event, &bundle_info);
  }
  break;
  case MCT_EVENT_CONTROL_HW_SLEEP:
  {
    ret = module_sensor_dualcamera_hw_sleep
      (module, event, &bundle_info);
  }
  break;
  case MCT_EVENT_CONTROL_MASTER_INFO:
  {
      ret = module_sensor_dualcamera_handle_master_info
        (module, event, &bundle_info);
  }
  break;
  default:
    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    break;
  }
  return ret;
}

/** module_sensor_set_mod: set mod function for sensor module
 *
 *  This function handles set mod events sent by mct **/

static void module_sensor_set_mod(mct_module_t *module,
  uint32_t module_type, uint32_t identity)
{
  SLOW("Enter, module_type=%d", module_type);
  MCT_PROF_LOG_BEG(__func__);

  mct_module_add_type(module, module_type, identity);
  if (module_type == MCT_MODULE_FLAG_SOURCE) {
    mct_module_set_process_event_func(module,
      module_sensor_module_process_event);
  }
  MCT_PROF_LOG_END();
  return;
}

/** module_sensor_query_mod: query mod function for sensor
 *  module
 *
 *  @query_buf: pointer to module_sensor_query_caps_t struct
 *  @session: session id
 *  @s_module: mct module pointer for sensor
 *
 *  Return: 0 for success and negative error for failure
 *
 *  This function handles query module events to return
 *  information requested by mct any stream is created **/

static boolean module_sensor_query_mod(mct_module_t *module,
  void *buf, uint32_t sessionid)
{
  int32_t                      i, rc = SENSOR_SUCCESS;
  mct_pipeline_sensor_cap_t   *sensor_cap = NULL;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  module_sensor_params_t      *module_sensor_params = NULL;
  mct_list_t                  *s_list = NULL;
  sensor_static_medatadata_t  *static_metadata;
  sensor_output_format_t       output_format;
  sensor_chromatix_params_t    chromatix_params;
  wbcalib_data_t              *wbc_factor = NULL;
  chromatix_parms_type        *chromatix_ptr = NULL;
  chromatix_3a_parms_type     *chromatix_3a_ptr = NULL;
  sensor_ctrl_t               *sensor_ctrl;
  char                         prop[PROPERTY_VALUE_MAX];
  float                        iso100_override;
  float                        iso100_gain;
  sensor_property_t            sensor_property;
  struct timespec              ts;
  sensor_filter_arrangement    filter_arrangement;

  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;

  RETURN_ON_NULL(query_buf);
  RETURN_ON_NULL(module);
  MCT_PROF_LOG_BEG(__func__);

  sensor_cap = &query_buf->sensor_cap;

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  RETURN_ON_NULL(s_list);

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  RETURN_ON_NULL(s_bundle);

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);
  SLOW("sensor name %s",
    s_bundle->sensor_info->sensor_name);


  TIMED_WAIT_ON_EVENT(s_bundle->mutex, s_bundle->cond,
            ts, INIT_CONFIG_TIMEOUT, s_bundle->init_config_done,
            TRUE, "init_config");

  rc = module_sensor_params->func_tbl.process(s_bundle->sensor_lib_params,
     SENSOR_GET_CAPABILITIES, sensor_cap);
  if (rc < 0) {
    SERR("get capabilities failed rc %d", rc);
    return FALSE;
  }

  /* If in identical stereo pair mode adjust the resolution accordingly. */
  if (s_bundle->is_stereo_configuration) {
    for (i = 0; i < sensor_cap->dim_fps_table_count; i++) {
      sensor_cap->dim_fps_table[i].dim.width *= 2L;
    }
    sensor_cap->active_array_size.width *= 2L;
    sensor_cap->pixel_array_size.width *= 2L;
  }

  /*Store ISP cap buff address to access later for PDAF config*/
  s_bundle->isp_cap = &query_buf->isp_cap;

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_PROPERTY, &sensor_property, rc);
  RETURN_FALSE_IF(rc < 0);

  /*Copy Dual Camera Calibration data*/
  memcpy(&sensor_cap->dualcam_tune_data,
    &s_bundle->dualcam_tune_data,
    sizeof (cam_related_system_calibration_data_t));

  /* Update lens info into capabilities */
  sensor_cap->lens_type =
    s_bundle->sensor_common_info.camera_config.lens_info.lens_type;
  sensor_cap->focal_length =
    s_bundle->sensor_common_info.camera_config.lens_info.focal_length;
  sensor_cap->hor_view_angle =
    s_bundle->sensor_common_info.camera_config.lens_info.hor_view_angle;
  sensor_cap->ver_view_angle =
    s_bundle->sensor_common_info.camera_config.lens_info.ver_view_angle;
  sensor_cap->apertures_count = 1;
  sensor_cap->apertures[0] =
    s_bundle->sensor_common_info.camera_config.lens_info.f_number;
  sensor_cap->pix_size = sensor_property.pix_size;
  /* mininum focus distance in diapters */
  sensor_cap->min_focus_distance = 1.0 /
    s_bundle->sensor_common_info.camera_config.lens_info.min_focus_distance;
  sensor_cap->hyper_focal_distance =
    (s_bundle->sensor_common_info.camera_config.lens_info.f_number * 2 *
    sensor_property.pix_size) / (sensor_cap->focal_length *
    sensor_cap->focal_length);
  sensor_cap->max_roll_degree =
      s_bundle->sensor_common_info.camera_config.lens_info.max_roll_degree;
  sensor_cap->max_pitch_degree =
       s_bundle->sensor_common_info.camera_config.lens_info.max_pitch_degree;
  sensor_cap->max_yaw_degree =
       s_bundle->sensor_common_info.camera_config.lens_info.max_yaw_degree;
  /* Update optical stab modes */
  if (strlen(s_bundle->sensor_common_info.camera_config.ois_name) > 0) {
    sensor_cap->optical_stab_modes_count = 2;
    sensor_cap->optical_stab_modes[0] = CAM_OPT_STAB_OFF;
    sensor_cap->optical_stab_modes[1] = CAM_OPT_STAB_ON;
  } else {
    sensor_cap->optical_stab_modes_count = 1;
    sensor_cap->optical_stab_modes[0] = CAM_OPT_STAB_OFF;
  }

  /* Fill sensor init params */
  sensor_cap->modes_supported =
    s_bundle->sensor_common_info.camera_config.modes_supported;

  sensor_cap->position = s_bundle->sensor_common_info.camera_config.position;
  sensor_cap->sensor_mount_angle =
    s_bundle->sensor_common_info.camera_config.sensor_mount_angle;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1 ||
    s_bundle->sensor_info->subdev_id[SUB_MODULE_STROBE_FLASH] != -1) {
    sensor_cap->is_flash_supported = TRUE;
   if (strlen(s_bundle->sensor_sd_name[SUB_MODULE_LED_FLASH]))
     strlcpy(sensor_cap->flash_dev_name,
       s_bundle->sensor_sd_name[SUB_MODULE_LED_FLASH],
       sizeof(sensor_cap->flash_dev_name));
  } else {
    SHIGH("led flash is not supported for this sensor.");
    sensor_cap->is_flash_supported = FALSE;
  }

  sensor_cap->af_supported =
    (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) ?
                                                      TRUE : FALSE;
  if (sensor_cap->af_supported) {
    sensor_cap->focus_dist_calibrated = CAM_FOCUS_APPROXIMATE;
  } else {
    sensor_cap->focus_dist_calibrated = CAM_FOCUS_UNCALIBRATED;
  }
  /* Fill sensor init params */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_SENSOR_FORMAT, &output_format, rc);
  if (rc < 0) {
    SERR("get sensor format failed rc %d", rc);
    return FALSE;
  }
  if (output_format == SENSOR_YCBCR) {
    sensor_cap->sensor_format = FORMAT_YCBCR;
  }
  else if (output_format == SENSOR_BAYER) {
    sensor_cap->sensor_format = FORMAT_BAYER;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_FILTER_ARRANGEMENT, &filter_arrangement, rc);
    if (rc < 0) {
      SERR("SENSOR_GET_FILTER_ARRANGEMENT failed rc %d", rc);
      return FALSE;
    }
    if (filter_arrangement == SENSOR_Y)
      sensor_cap->sensor_format = FORMAT_GREY;
  }

  /* Fill static metadata */
  static_metadata = &s_bundle->frame_ctrl.static_metadata;
  memset(static_metadata, 0, sizeof(sensor_static_medatadata_t));

  static_metadata->aperture = sensor_cap->apertures[0];
  static_metadata->filter_density = sensor_cap->filter_densities[0];
  static_metadata->focal_length = sensor_cap->focal_length;
  for (i = 0; i < sensor_cap->optical_stab_modes_count; i++) {
    if (sensor_cap->optical_stab_modes[i] == CAM_OPT_STAB_ON) {
      static_metadata->ois_mode = 1;
      break;
    }
  }

  if (output_format == SENSOR_BAYER &&
    !s_bundle->sensor_lib_params->sensor_lib_ptr->
    sensor_output.is_depth_sensor) {
    /* DNG info : forward/color/calibration transform matrix */
    /* 1. Load chromatix */
    SHIGH("output format is SENSOR_BAYER, load chromatix for sensor %s",
      s_bundle->sensor_lib_params->sensor_lib_ptr->
      sensor_slave_info.sensor_name);
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
    RETURN_FALSE_IF(rc < 0);

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
      CHROMATIX_GET_PTR, &chromatix_params, rc);
    RETURN_FALSE_IF(rc < 0);

    /* 2. Generate the matrix */
    chromatix_ptr = (chromatix_parms_type *)(chromatix_params.
      chromatix_ptr[SENSOR_CHROMATIX_ISP]);
    chromatix_3a_ptr = (chromatix_3a_parms_type *)(chromatix_params.
      chromatix_ptr[SENSOR_CHROMATIX_3A]);

    sensor_generate_transfrom_matrix(
      sensor_cap->forward_transform_D65, sensor_cap->color_transform_D65,
      &chromatix_ptr->chromatix_VFE.chromatix_CC.D65_color_correction,
      &chromatix_3a_ptr->AWB_bayer_algo_data.awb_MWB.MWB_d65,
      TRUE);

    sensor_generate_transfrom_matrix(
      sensor_cap->forward_transform_A, sensor_cap->color_transform_A,
      &chromatix_ptr->chromatix_VFE.chromatix_CC.A_color_correction,
      &chromatix_3a_ptr->AWB_bayer_algo_data.awb_MWB.MWB_A,
      FALSE);

    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_WB_CAL, &wbc_factor, rc);
      RETURN_FALSE_IF(rc < 0);
    }

    if (wbc_factor) {
      sensor_generate_calib_trans(sensor_cap->calibration_transform_D65,
        wbc_factor->r_over_g[AGW_AWB_D65], wbc_factor->b_over_g[AGW_AWB_D65]);
      sensor_generate_calib_trans(sensor_cap->calibration_transform_A,
        wbc_factor->r_over_g[AGW_AWB_A], wbc_factor->b_over_g[AGW_AWB_A]);
    } else {
      sensor_generate_unit_matrix(sensor_cap->calibration_transform_D65);
      sensor_generate_unit_matrix(sensor_cap->calibration_transform_A);
    }
    sensor_cap->base_gain_factor.numerator = 0;
    sensor_cap->base_gain_factor.denominator = FLOAT_TO_Q(7, 1);

    /* Set reference illuminance used to generate matrix */
    sensor_cap->reference_illuminant1 = CAM_AWB_D65;
    sensor_cap->reference_illuminant2 = CAM_AWB_A;
  }

  /* Based on chromatix ISO100_gain
     present real gain to normalized ISO gain.
     Keep this info in sensor_ctrl data for later use.
     (in case of rounding) */
  if (chromatix_3a_ptr) {
    iso100_gain =
      chromatix_3a_ptr->AEC_algo_data.aec_generic.ISO100_gain;
    if (iso100_gain == 0)
      iso100_gain = 1;
  } else {
     iso100_gain = 0.7;
     property_get("persist.vendor.camera.iso100", prop, "0.0");
     iso100_override = atof(prop);
     if(iso100_override > 0.0) {
       SHIGH("setprop override iso100_gain %f ==> iso100_override %f",
             iso100_gain, iso100_override);
       iso100_gain = iso100_override;
     }
  }

  sensor_cap->max_gain /= iso100_gain;
  sensor_cap->max_analog_gain /= iso100_gain;
  sensor_cap->min_gain /= iso100_gain;

  SLOW("ISO 100_gain %f max %d min %d max analog %d",
    iso100_gain,
    sensor_cap->max_gain, sensor_cap->min_gain, sensor_cap->max_analog_gain);

  MCT_PROF_LOG_END();

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_IR_LED] != -1) {
    sensor_cap->supported_ir_modes[0] = TRUE;
    sensor_cap->supported_ir_modes[1] = TRUE;
  }

  return TRUE;
}

static boolean module_sensor_delete_port(void *data, void *user_data)
{
  mct_port_t *s_port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  if (!s_port || !module) {
    SERR("failed s_port %p module %p",
      s_port, module);
    return TRUE;
  }
  free(s_port->caps.u.data);
  return TRUE;
}

/** module_sensor_free_bundle: free bundle function for
 *  sensor module
 *
 *  @data: sensor bundle pointer
 *  @user_data: NULL
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function is called for each sensor when module close is
 *  called. It releases all resources and memory held by each
 *  sensor */

static boolean module_sensor_free_bundle(void *data, void *user_data)
{
  uint32_t i = 0;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;
  pthread_mutex_t *lens_move_done_sig;

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module);

  if(MCT_MODULE_SRCPORTS(module)) {
   mct_list_free_all(MCT_MODULE_SRCPORTS(module), module_sensor_delete_port);
   MCT_MODULE_SRCPORTS(module) = NULL;
  }

  cm_destroy(&s_bundle->chromatix_manager);

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
    eeprom_unload_library(s_bundle->eeprom_data);
    free(s_bundle->eeprom_data);
  }

  sensor_unload_library(s_bundle->sensor_lib_params);
  if (s_bundle->sensor_lib_params) {
    free(s_bundle->sensor_lib_params);
  }
  for (i = 0; i < SUB_MODULE_MAX; i++) {
    free(s_bundle->module_sensor_params[i]);
  }

  lens_move_done_sig =
    &s_bundle->af_bracket_params.lens_move_done_sig;
  pthread_mutex_destroy(lens_move_done_sig);

  /* Destroy mutex and cond */
  pthread_cond_destroy(&s_bundle->cond);
  pthread_condattr_destroy(&s_bundle->condattr);
  pthread_mutex_destroy(&s_bundle->mutex);

  free(s_bundle->sensor_info);
  free(s_bundle);
  return TRUE;
}

/** module_sensor_free_mod: free module function for sensor
 *  module
 *
 *  @module: mct module pointer for sensor
 *
 *  This function releases all resources held by sensor mct
 *  module */

void module_sensor_deinit(mct_module_t *module)
{
  module_sensor_ctrl_t        *module_ctrl = NULL;

  RETURN_VOID_ON_NULL(module);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;

  eebin_interface_deinit(module_ctrl->eebin_hdl);

  mct_list_traverse(module_ctrl->sensor_bundle, module_sensor_free_bundle,
    module);

  /* free dual cam streaming mutex */
  pthread_mutex_destroy(&module_ctrl->dual_cam_mutex);
  free(module);

  return;
}


boolean module_sensor_get_session_data(mct_module_t *module, void *get_buf,
  unsigned int sessionid)
{
  mct_pipeline_session_data_t    *session_data =
    (mct_pipeline_session_data_t *)get_buf;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  module_sensor_bundle_info_t *s_bundle = NULL;
  struct sensor_lib_out_info_t *out_info = NULL;
  sensor_output_format_t       output_format;
  sensor_filter_arrangement    filter_arrangement;

  uint32_t i = 0, size = 0;
  int32_t rc = SENSOR_SUCCESS;

  SHIGH("sessionid %d", sessionid);

  /* Validate input parameters */
  RETURN_ON_NULL(module);
  RETURN_ON_NULL(get_buf);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  /* get the s_bundle from session id */
  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &sessionid,
    sensor_util_find_bundle);
  RETURN_ON_NULL(s_list);

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  RETURN_ON_NULL(s_bundle);

  RETURN_ON_NULL(s_bundle->sensor_lib_params);
  RETURN_ON_NULL(s_bundle->sensor_lib_params->sensor_lib_ptr);

  /*Updating pipeline and meta reporting delay */
  session_data->max_pipeline_frame_applying_delay =
    MTYPE_MAX(session_data->max_pipeline_frame_applying_delay,
    s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_max_pipeline_frame_delay);

  session_data->max_pipeline_meta_reporting_delay =
   MTYPE_MAX(session_data->max_pipeline_meta_reporting_delay,0);

  session_data->max_sensor_delay =
    s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  out_info = s_bundle->sensor_lib_params->sensor_lib_ptr->out_info_array.out_info;

  if (s_bundle->sensor_lib_params->sensor_lib_ptr->out_info_array.size >
    SENSOR_MAX_RESOLUTION) {
    size = SENSOR_MAX_RESOLUTION;
  } else {
    size = s_bundle->sensor_lib_params->sensor_lib_ptr->out_info_array.size;
  }

  for (i = 0; i < size; i++) {
    session_data->sensor_pix_clk= MAXQ(session_data->sensor_pix_clk,
    out_info[i].op_pixel_clk);
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_SENSOR_FORMAT, &output_format, rc);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    return FALSE;
  }
  if (output_format == SENSOR_YCBCR) {
    s_bundle->is_bayer = FALSE;
    session_data->sensor_format = FORMAT_YCBCR;
  } else if (output_format == SENSOR_BAYER) {
    s_bundle->is_bayer = TRUE;
    session_data->sensor_format = FORMAT_BAYER;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_FILTER_ARRANGEMENT, &filter_arrangement, rc);
    if (rc < 0) {
      SERR("SENSOR_GET_FILTER_ARRANGEMENT failed rc %d", rc);
      return FALSE;
    }
    if (filter_arrangement == SENSOR_Y)
      session_data->sensor_format = FORMAT_GREY;
  }

  session_data->lens_type =
    s_bundle->sensor_common_info.camera_config.lens_info.lens_type;
  session_data->position = s_bundle->sensor_common_info.camera_config.position;
  if (session_data->position == CAM_POSITION_BACK_AUX ||
    session_data->position == CAM_POSITION_FRONT_AUX) {
    session_data->is_aux_camera = 1;
  } else {
    session_data->is_aux_camera = 0;
  }
  if (s_bundle->sensor_lib_params->sensor_lib_ptr->external_library) {
    session_data->q3a_lib_info.aec_libptr =
      s_bundle->external_library[SENSOR_EXTERNAL_LIB_AEC];
    session_data->q3a_lib_info.awb_libptr =
      s_bundle->external_library[SENSOR_EXTERNAL_LIB_AWB];
    session_data->q3a_lib_info.af_libptr =
      s_bundle->external_library[SENSOR_EXTERNAL_LIB_AF];
  } else {
    session_data->q3a_lib_info.aec_libptr = NULL;
    session_data->q3a_lib_info.awb_libptr = NULL;
    session_data->q3a_lib_info.af_libptr = NULL;
  }
  session_data->is_valid_dualcalib = s_bundle->is_valid_dualcalib;
  /* copy to session_data struct. This will be passed to all other
   * pipeline modules*/
  memcpy(&session_data->dual_data, &s_bundle->dualcam_tune_data,
    sizeof(cam_related_system_calibration_data_t));

  return TRUE;
}

/** module_sensor_set_session_data: set session data
 *
 *  @module: sensor module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  contorl parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_sensor_set_session_data(mct_module_t *module,
  void *set_buf, uint32_t sessionid)
{
  mct_pipeline_session_data_t          *frame_ctrl_data = NULL;
  module_sensor_ctrl_t                 *module_ctrl = NULL;
  sensor_bundle_info_t                  bundle_info;
  module_sensor_bundle_info_t           *s_bundle = NULL;
  uint32_t                              identity = 0;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(set_buf);

  frame_ctrl_data = (mct_pipeline_session_data_t *)set_buf;

  /* Extract module private */
  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
  identity = pack_identity(sessionid, 0);

  /* Find sensor bundle */
  RETURN_ON_FALSE(sensor_util_get_sbundle(module, identity, &bundle_info));

  s_bundle = bundle_info.s_bundle;
  RETURN_ON_NULL(s_bundle);

  s_bundle->frame_ctrl.session_data = *frame_ctrl_data;

  s_bundle->capture_pipeline_delay =
    s_bundle->frame_ctrl.session_data.max_pipeline_frame_applying_delay;
  s_bundle->retry_frame_skip =
    s_bundle->frame_ctrl.session_data.min_isp_sw_frame_skip_retry;

  SHIGH("max delay %d report delay %d retry skip %d",
    frame_ctrl_data->max_pipeline_frame_applying_delay,
    frame_ctrl_data->max_pipeline_meta_reporting_delay,
    s_bundle->retry_frame_skip);

  return TRUE;
}

/** module_sensor_fill_sub_module_info: set subdev info for sensor sub
 *  modules
 *
 *  @data: module_sensor_bundle_info_t pointer
 *  @user_data: module_sensor_match_id_params_t pointer
 *  @intf_type: intf type exposed by sub dev to sub module
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function fills the multiple sub dev nodes exposed for
 *  the sub module. If the primary subdev node is missing , user
 *  module discards the remaining subdev nodes as well. If successfully
 *  filled, these nodes are later used in open to communicate to
 * kernel driver **/

static boolean module_sensor_fill_sub_module_info(void *data, void *user_data,
  sensor_subdev_intf_type_t intf_type)
{
  module_sensor_bundle_info_t *sensor_bundle =
    (module_sensor_bundle_info_t *)data;
  module_sensor_match_id_params_t *match_id_params =
    (module_sensor_match_id_params_t *)user_data;
  if (!match_id_params) {
    SERR("failed match_id_params = %p", match_id_params);
    return FALSE;
  }
  sensor_submodule_intf_info_t *intf_info =
    &sensor_bundle->subdev_info[match_id_params->sub_module].intf_info[intf_type];
  if (!sensor_bundle || !intf_info || (intf_type >= SUBDEV_INTF_MAX)) {
    SERR("failed data1 %p data2 %p intf info %p intf type %d", data,
      user_data, intf_info, intf_type);
    return FALSE;
  }
  if (match_id_params->sub_module >= SUB_MODULE_MAX) {
    SERR("Invalid subdev module %d",match_id_params->sub_module);
    return FALSE;
  }
  intf_info->subdev_id = match_id_params->subdev_id;
  sensor_bundle->subdev_info[match_id_params->sub_module].intf_mask |= (intf_type << 1);
  SLOW("intf_type : %d intf_mask : %d, sub_module : %d", intf_type,
      sensor_bundle->subdev_info[match_id_params->sub_module].intf_mask,
      match_id_params->sub_module);

  switch(intf_type) {
  case SUBDEV_INTF_PRIMARY:
    memcpy(intf_info->sensor_sd_name,
      match_id_params->subdev_name, MAX_SUBDEV_SIZE);
    break;
  case SUBDEV_INTF_SECONDARY:
    memcpy(intf_info->sensor_sd_name,
      match_id_params->subdev_name, MAX_SUBDEV_SIZE);
    break;
  default:
    break;
  }

  return TRUE;
}

/** module_sensor_set_sub_module_id: set subdev id for sensor sub
 *  modules
 *
 *  @data: module_sensor_bundle_info_t pointer
 *  @user_data: module_sensor_match_id_params_t pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function matches subdev id against subdev id present in
 *  sensor info, if both matches, copy the subdev name for this
 *  subdev which will be used later to open and communicate with
 *  kernel driver **/

static boolean module_sensor_set_sub_module_id(void *data, void *user_data)
{
  module_sensor_bundle_info_t *sensor_bundle =
    (module_sensor_bundle_info_t *)data;
  module_sensor_match_id_params_t *match_id_params =
    (module_sensor_match_id_params_t *)user_data;
  if (!sensor_bundle || !match_id_params) {
    SERR("failed data1 %p data2 %p", data,
      user_data);
    return FALSE;
  }
  SLOW("sub module %d id %d subdev name %s",
    match_id_params->sub_module, match_id_params->subdev_id,
    match_id_params->subdev_name);
  SLOW("sensor_info->subdev_id %d ?= match_id_params->subdev_id %d",
    sensor_bundle->sensor_info->subdev_id[match_id_params->sub_module],
    match_id_params->subdev_id);

  if (sensor_bundle->sensor_info->subdev_id[match_id_params->sub_module] ==
    match_id_params->subdev_id) {
    memcpy(sensor_bundle->sensor_sd_name[match_id_params->sub_module],
      match_id_params->subdev_name, MAX_SUBDEV_SIZE);
    SLOW("match found sub module %d session id %d subdev name %s",
      match_id_params->sub_module,
      sensor_bundle->sensor_info->session_id, match_id_params->subdev_name);
      if (!module_sensor_fill_sub_module_info(data, user_data, SUBDEV_INTF_PRIMARY))
        return FALSE;
  } else if(sensor_bundle->sensor_info->subdev_intf[match_id_params->sub_module] ==
    match_id_params->subdev_id) {
    SLOW("Secondary Subdev node found for sub module %d but not listed"
      "in the sensor info queried from kernel",match_id_params->sub_module);
    if(!module_sensor_fill_sub_module_info(data, user_data, SUBDEV_INTF_SECONDARY))
      return FALSE;
  }
  return TRUE;
}

/*
 * EEPROM control
 * : create "sensor_eeprom_data_t" in bundle
 *   alloc buffer for data from kernel
 *   load eeprom driver library */
static boolean module_sensor_init_eeprom(void *data, void *eebin_hdl)
{
  int32_t                          rc = 0;
  module_sensor_bundle_info_t     *s_bundle = NULL;
  sensor_func_tbl_t                func_tbl;
  eebin_ctl_t bin_ctl;

  s_bundle = (module_sensor_bundle_info_t *)data;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] == -1) {
    SERR("Exit subdev_id[SUB_MODULE_EEPROM] == -1");
    return TRUE;
  }

  eeprom_sub_module_init(&func_tbl);

  s_bundle->eeprom_data = (sensor_eeprom_data_t *)
    malloc(sizeof(sensor_eeprom_data_t));
  if (!s_bundle->eeprom_data) {
    SERR("failed to allocate memory");
    goto malloc_failed;
  }

  /* (1) Open the eeprom sub module */
  rc = func_tbl.open((void **)&s_bundle->eeprom_data,
         &s_bundle->subdev_info[SUB_MODULE_EEPROM]);
  if (rc < 0) {
    SERR("Failed EEPROM_OPEN");
    goto init_eeprom_Exit;
  }

  memset(&s_bundle->eeprom_data->eeprom_params, 0, sizeof(eeprom_params_t));
  memcpy(s_bundle->eeprom_data->eeprom_params.eeprom_name,
    s_bundle->sensor_common_info.camera_config.eeprom_name,
    sizeof(s_bundle->eeprom_data->eeprom_params.eeprom_name));

  /* (2) Load eeprom library */
  bin_ctl.cmd = EEPROM_BIN_GET_NAME_DATA;
  bin_ctl.ctl.q_num.type = EEPROM_BIN_LIB_EEPROM;
  bin_ctl.ctl.name_data.name = s_bundle->sensor_info->sensor_name;
  bin_ctl.ctl.name_data.path = NULL;
  rc = eebin_interface_control(eebin_hdl, &bin_ctl);
  if (rc < 0) {
    SERR("No eeprom data, name : %s",
      s_bundle->eeprom_data->eeprom_params.eeprom_name);
  }
  SHIGH("EEPROM path: %s name: %s", bin_ctl.ctl.name_data.path,
    s_bundle->eeprom_data->eeprom_params.eeprom_name);

  rc = eeprom_load_library(s_bundle->eeprom_data, bin_ctl.ctl.name_data.path);
  if (rc < 0) {
    SERR("Failed eeprom_load_library");
    goto init_eeprom_Exit;
  }
  /* (3) Powerup and parse the eeprom */
  rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_INIT, NULL);
  if (rc < 0) {
    SERR("Failed in EEPROM_INIT");
    goto init_eeprom_Exit;
  }

  /* (4) Read the eeprom data from kernel */
  rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_READ_DATA, NULL);
  if (rc < 0) {
    SERR("Failed EEPROM_READ_DATA");
    goto init_eeprom_Exit;
  }

  /* (5) Format the cal data */
  rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_SET_FORMAT_DATA, NULL);
  if (rc < 0) {
      SERR("Failed EEPROM_READ_DATA");
      goto init_eeprom_Exit;
  }
  rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_GET_FORMATTED_DATA,
    &s_bundle->formatted_data);
  if (rc < 0) {
      SERR("Failed EEPROM_READ_DATA");
      goto init_eeprom_Exit;
  }
  rc = func_tbl.process(s_bundle->eeprom_data, EEPROM_SET_CALIBRATE_DUALCAM_PARAM,
    &s_bundle->dualcam_tune_data);
  if (rc < 0) {
    SERR("<non-Fatal> Failed EEPROM_SET_CALIBRATE_DUALCAM_PARAM");
    s_bundle->is_valid_dualcalib = FALSE;
  } else {
    s_bundle->is_valid_dualcalib = TRUE;
  }

  /* (6) close the eeprom sub module */
  rc = func_tbl.close(s_bundle->eeprom_data);
  if (rc < 0) {
      SERR("Failed EEPROM_CLOSE");
      goto init_eeprom_Exit;
  }

  SLOW("Exit");
  return TRUE;

init_eeprom_Exit:
  free(s_bundle->eeprom_data);
  s_bundle->eeprom_data = NULL;
malloc_failed:
  s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] = -1;
  memset(&s_bundle->module_sensor_params[SUB_MODULE_EEPROM]->func_tbl, 0,
    sizeof(sensor_func_tbl_t));
  return TRUE;
}

static boolean module_sensor_init_chromatix(void *data, void *eebin_hdl)
{
  module_sensor_bundle_info_t *s_bundle;
  sensor_lib_t                *sensor_lib_ptr;
  sensor_func_tbl_t            func_tbl;
  boolean                      rc = TRUE;

  SLOW("Enter");

  RETURN_ON_NULL(data);

  s_bundle = (module_sensor_bundle_info_t *)data;
  sensor_lib_ptr = s_bundle->sensor_lib_params->sensor_lib_ptr;

  /* No EEPROM */
  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] == -1) {
    rc = cm_create(&s_bundle->chromatix_manager,
      s_bundle->sensor_info->sensor_name,
      &s_bundle->sensor_common_info.camera_config.chromatix_info,
      NULL, eebin_hdl);
    if (rc == FALSE) {
      SERR("failed: create cm");
      return FALSE;
    }
  /* EEPROM : pass the eeprom handle for calibration */
  } else {
    eeprom_sub_module_init(&func_tbl);

    if (func_tbl.open((void **)&s_bundle->eeprom_data,
           &s_bundle->subdev_info[SUB_MODULE_EEPROM]) < 0) {
      SERR("Failed EEPROM_OPEN");
      return FALSE;
    }

    rc = cm_create(&s_bundle->chromatix_manager,
      s_bundle->sensor_info->sensor_name,
      &s_bundle->sensor_common_info.camera_config.chromatix_info,
      s_bundle->eeprom_data, eebin_hdl);
    if (rc == FALSE) {
      SERR("failed: create cm");
      if (func_tbl.close(s_bundle->eeprom_data) < 0)
        SERR("Failed EEPROM_CLOSE");
      return FALSE;
    }

    if (func_tbl.close(s_bundle->eeprom_data) < 0) {
      cm_destroy(&s_bundle->chromatix_manager);
      SERR("Failed EEPROM_CLOSE");
      return FALSE;
    }
  }

  SLOW("Exit");
  return TRUE;
}

/** module_sensor_set_decode_format_based_on_pack_mode: Sets the decode format
 *  based on the pack mode.
 *
 *  @s_bundle:                  sensor bundle pointer
 *  @sensor_get_params: Pointer to the CSID parameters
 *
 *  This function finds the corresponding cid for the bayer/grey data format
 *  in the stream info array. It then fills in the corresponding decode format
 *  for this cid in the sensor_get_params structure to support packing if
 *  needed.**/
static void module_sensor_set_decode_format_based_on_pack_mode(
    module_sensor_bundle_info_t *s_bundle, sensor_get_t* sensor_get_params)
{
  struct sensor_csid_lut_params        *lut_params = NULL;
  uint32_t                             i = 0, j = 0;
  sensor_stream_info_array_t           *stream_info_array = NULL;
  sensor_stream_info_t                 *stream_info = NULL;
  unsigned char                        cid = 0;
  boolean                              cid_found = FALSE;
  int32_t                              pack_mode_mask = -1;

  RETURN_VOID_ON_NULL(s_bundle);
  RETURN_VOID_ON_NULL(sensor_get_params);

  SERR("Pack mode is %d", s_bundle->pack_mode);
  if (s_bundle->pack_mode == SENSOR_PACK_MODE_NONE) {
    return;
  }

  pack_mode_mask = (s_bundle->pack_mode == SENSOR_PACK_MODE_PLAIN8) ?
    CSI_RDI_PLAIN8 : CSI_RDI_PLAIN16;
  lut_params = sensor_get_params->csi_cfg.lut_params;
  stream_info_array =
    &s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_stream_info_array;

  for (i = 0; i < stream_info_array->size; i++) {
    stream_info = &stream_info_array->sensor_stream_info[i];
    for (j = 0; j < stream_info->vc_cfg_size; j++) {
      if (SENSOR_BAYER == stream_info->pix_data_fmt[j] ||
          SENSOR_GREY == stream_info->pix_data_fmt[j]) {
        cid = stream_info->vc_cfg[j].cid;
        cid_found = TRUE;
        SLOW("Found CID %d with format %d", cid, stream_info->pix_data_fmt[j]);
      }
    }
  }

  if (!cid_found) {
    SERR("No corresponding CID found for bayer/grey sensor.");
    return;
  }

  /* Loop through the sensor get parameters and set the decode
     * format based on pack mode. */
  for (i = 0; i < lut_params->num_cid; i++) {
    if (cid == lut_params->vc_cfg_a[i].cid) {
      lut_params->vc_cfg_a[i].decode_format |=
        (pack_mode_mask | CSI_RDI_PACKED | CSI_RDI_LSB_ALIGNED);
      SLOW("Setting pack mode 0x%x for CID %d", pack_mode_mask, cid);
    }
  }
}

/** module_sensor_fill_pack_mode: Fills sensor out info with
 *  appropriate pack mode information
 *
 *  @s_bundle: sensor bundle pointer
 *  @sensor_out_info: Pointer to the sensor_out_info structure
 *  @pack_mode: Pack mode that is to be used
 *
 *  This function finds the corresponding cid for the bayer/grey data format
 *  in the stream info array. It then fills in the override pack mode info
 *  inside the sensor_out_info structure with this cid and the pack
 *  mode. **/
static void module_sensor_fill_pack_mode(
  module_sensor_bundle_info_t *s_bundle, sensor_out_info_t *sensor_out_info,
  iface_output_pack_mode pack_mode)
{
  uint32_t                             i = 0, j = 0;
  sensor_stream_info_array_t           *stream_info_array = NULL;
  sensor_stream_info_t                 *stream_info = NULL;
  unsigned char                        cid = 0;
  uint32_t                             num_entries = 0;

  RETURN_VOID_ON_NULL(s_bundle);
  RETURN_VOID_ON_NULL(sensor_out_info)

  memset(&sensor_out_info->pack_mode_override, 0,
    sizeof(sensor_pack_mode_info_t));
  stream_info_array =
    &s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_stream_info_array;

  for (i = 0; i < stream_info_array->size; i++) {
    stream_info = &stream_info_array->sensor_stream_info[i];
    for (j = 0; j < stream_info->vc_cfg_size; j++) {
      if (SENSOR_BAYER == stream_info->pix_data_fmt[j] ||
          SENSOR_GREY == stream_info->pix_data_fmt[j]) {
        cid = stream_info->vc_cfg[j].cid;
        SLOW("Found CID %d with format %d", cid, stream_info->pix_data_fmt[j]);

        num_entries = sensor_out_info->pack_mode_override.num_entries;
        sensor_out_info->pack_mode_override.pack_mode_info[num_entries].cid =
          cid;
        sensor_out_info->pack_mode_override.pack_mode_info[num_entries].
          pack_mode = pack_mode;
        sensor_out_info->pack_mode_override.num_entries++;
      }
    }
  }
}

/** module_sensor_find_other_subdev: find subdevs other than
 *  sensor
 *
 *  @module_ctrl: module_sensor_ctrl_t pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function finds all subdevs other than sensor and fills
 *  the subdev name in sensor bundle for those sensor whose
 *  subdev id matches with current subdev **/

static boolean module_sensor_find_other_subdev(
  module_sensor_ctrl_t *module_ctrl)
{
  struct    media_device_info mdev_info;
  int32_t   num_media_devices = 0;
  int32_t   rc = 0, dev_fd = 0, sd_fd = 0;
  int32_t   subdev_id;
  char      dev_name[32];
  char      subdev_name[32];
  module_sensor_match_id_params_t match_id_params;

  RETURN_ON_NULL(module_ctrl);

  while (1) {
    uint32_t num_entities = 0, subdev_type = 0;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    SLOW("Opened Device %s",dev_name);
    if (dev_fd < 0) {
      SLOW("Done enumerating media devices");
      break;
    }
    num_media_devices++;
    rc = LOG_IOCTL(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info, "dev_info");
    if (rc < 0) {
      SERR("Error: ioctl media_dev failed: %s", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      SLOW("entity id %d", entity.id);
      rc = LOG_IOCTL(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity, "enum_entities");
      if (rc < 0) {
        SLOW("Done enumerating media entities");
        rc = 0;
        break;
      }
      num_entities = entity.id;
      SLOW("entity name %s type %x group id %d",
        entity.name, entity.type, entity.group_id);

      #ifndef USE_4_9_DEFS
        subdev_type = entity.group_id;
      #else
        subdev_type = entity.type;
      #endif

      if (subdev_type == MSM_CAMERA_SUBDEV_ACTUATOR ||
        subdev_type == MSM_CAMERA_SUBDEV_EEPROM ||
        subdev_type == MSM_CAMERA_SUBDEV_FLASH ||
        subdev_type == MSM_CAMERA_SUBDEV_STROBE_FLASH ||
        subdev_type == MSM_CAMERA_SUBDEV_CSIPHY ||
        subdev_type == MSM_CAMERA_SUBDEV_CSID ||
        subdev_type == MSM_CAMERA_SUBDEV_OIS ||
        subdev_type == MSM_CAMERA_SUBDEV_EXT ||
        subdev_type == MSM_CAMERA_SUBDEV_IR_LED ||
        subdev_type == MSM_CAMERA_SUBDEV_IR_CUT ||
        subdev_type == MSM_CAMERA_SUBDEV_LASER_LED) {
        snprintf(subdev_name, sizeof(subdev_name), "/dev/%s", entity.name);
        SLOW("sensor subdev %s", subdev_name);
        sd_fd = open(subdev_name, O_RDWR);
        if (sd_fd < 0) {
          SLOW("Open subdev failed: %s", subdev_name);
          continue;
        }
        /* Read subdev index */
        rc = LOG_IOCTL(sd_fd, VIDIOC_MSM_SENSOR_GET_SUBDEV_ID, &subdev_id, "subdev_id");
        if (rc < 0) {
          SERR("failed rc %d", rc);
          close(sd_fd);
          continue;
        }
        SLOW("subdev_name %s subdev id %d", subdev_name, subdev_id);
        /* TODO: read id and fill in sensor_bundle.entity.actuator_name */
        switch (subdev_type) {
           case MSM_CAMERA_SUBDEV_ACTUATOR:
            match_id_params.sub_module = SUB_MODULE_ACTUATOR;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_ACTUATOR subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_EEPROM:
            match_id_params.sub_module = SUB_MODULE_EEPROM;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_EEPROM subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_FLASH:
            match_id_params.sub_module = SUB_MODULE_LED_FLASH;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_LED_FLASH subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_STROBE_FLASH:
            match_id_params.sub_module = SUB_MODULE_STROBE_FLASH;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_STROBE_FLASH subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_CSIPHY:
            match_id_params.sub_module = SUB_MODULE_CSIPHY;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_CSIPHY subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_CSID:
            match_id_params.sub_module = SUB_MODULE_CSID;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_CSID subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_OIS:
            match_id_params.sub_module = SUB_MODULE_OIS;
            match_id_params.subdev_id = subdev_id;
            match_id_params.subdev_name = entity.name;
            SLOW("SUB_MODULE_OIS subdev_name %s subdev_id %d",
              subdev_name, match_id_params.subdev_id);
            mct_list_traverse(module_ctrl->sensor_bundle,
              module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_IR_LED:
              match_id_params.sub_module = SUB_MODULE_IR_LED;
              match_id_params.subdev_id = subdev_id;
              match_id_params.subdev_name = entity.name;
              SLOW("SUB_MODULE_IR_LED subdev_name %s subdev_id %d",
                subdev_name, match_id_params.subdev_id);
              mct_list_traverse(module_ctrl->sensor_bundle,
                module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_IR_CUT:
              match_id_params.sub_module = SUB_MODULE_IR_CUT;
              match_id_params.subdev_id = subdev_id;
              match_id_params.subdev_name = entity.name;
              SLOW("SUB_MODULE_IR_CUT subdev_name %s subdev_id %d",
                subdev_name, match_id_params.subdev_id);
              mct_list_traverse(module_ctrl->sensor_bundle,
                module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_LASER_LED:
              match_id_params.sub_module = SUB_MODULE_LASER_LED;
              match_id_params.subdev_id = subdev_id;
              match_id_params.subdev_name = entity.name;
              SLOW("SUB_MODULE_LASER_LED subdev_name %s subdev_id %d",
                subdev_name, match_id_params.subdev_id);
              mct_list_traverse(module_ctrl->sensor_bundle,
                module_sensor_set_sub_module_id, &match_id_params);
            break;
          case MSM_CAMERA_SUBDEV_EXT:
              match_id_params.sub_module = SUB_MODULE_EXT;
              match_id_params.subdev_id = subdev_id;
              match_id_params.subdev_name = entity.name;
              SLOW("SUB_MODULE_EXT subdev_name %s subdev_id %d",
                subdev_name, match_id_params.subdev_id);
              mct_list_traverse(module_ctrl->sensor_bundle,
                module_sensor_set_sub_module_id, &match_id_params);
            break;
          default:
              SLOW("ERROR Default subdev_type %d subdev_name %s subdev_id %d",
                subdev_type, entity.name, subdev_id);
            break;
        }
        close(sd_fd);
      }
    }
    close(dev_fd);
  }
  return TRUE;
};

/** module_sensor_init: sensor module init
 *
 *  Return: mct_module_t pointer corresponding to sensor
 *
 *  This function creates mct_module_t for sensor module,
 *  creates port, fills capabilities and add it to the sensor
 *  module **/

mct_module_t *module_sensor_init(const char *name)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  mct_module_t                *s_module = NULL;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  eebin_ctl_t bin_ctl;

  SHIGH("Sensor driver Version: %s", SENSOR_DRIVER_VERSION);
  SHIGH("Sensor SDK capabilities: %s", SENSOR_SDK_CAPABILITIES);

  SHIGH("Actuator driver Version: %s", ACTUATOR_DRIVER_VERSION);
  SHIGH("Actuator SDK capabilities: %s", ACTUATOR_SDK_CAPABILITIES);

  SHIGH("EEPROM driver Version: %s", EEPROM_DRIVER_VERSION);
  SHIGH("EEPROM SDK capabilities: %s", EEPROM_SDK_CAPABILITIES);

  SHIGH("Flash driver Version: %s", FLASH_DRIVER_VERSION);
  SHIGH("Flash SDK capabilities: %s", FLASH_SDK_CAPABILITIES);

  SHIGH("OIS driver Version: %s", OIS_DRIVER_VERSION);
  SHIGH("OIS SDK capabilities: %s", OIS_SDK_CAPABILITIES);

  SHIGH("PDAF driver Version: %s", PDAF_DRIVER_VERSION);
  SHIGH("PDAF SDK capabilities: %s", PDAF_SDK_CAPABILITIES);


  /* Create MCT module for sensor */
  s_module = mct_module_create(name);
  if (!s_module) {
    SERR("failed");
    return NULL;
  }

  /* Fill function table in MCT module */
  s_module->set_mod = module_sensor_set_mod;
  s_module->query_mod = module_sensor_query_mod;
  s_module->start_session = module_sensor_start_session;
  s_module->stop_session = module_sensor_stop_session;
  s_module->set_session_data = module_sensor_set_session_data;
  s_module->get_session_data = module_sensor_get_session_data;
  /* Create sensor module control structure that consists of bundle
     information */
  module_ctrl = malloc(sizeof(module_sensor_ctrl_t));
  if (!module_ctrl) {
    SERR("failed");
    goto ERROR1;
  }
  memset(module_ctrl, 0, sizeof(module_sensor_ctrl_t));

  s_module->module_private = (void *)module_ctrl;

  /* sensor module doesn't have sink port */
  s_module->numsinkports = 0;

  rc = eebin_interface_init(&module_ctrl->eebin_hdl);
  if (rc < 0) {
    SERR("failed");
  }

  bin_ctl.cmd = EEPROM_BIN_GET_BIN_DATA;
  rc = eebin_interface_control(module_ctrl->eebin_hdl, &bin_ctl);
  if (rc < 0) {
    SERR("failed");
  }

  /* module_sensor_probe_sensors */
  ret = sensor_init_probe(module_ctrl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* find all the actuator, etc with sensor */
  ret = module_sensor_find_other_subdev(module_ctrl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Init sensor modules */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, module_sensors_subinit,
    NULL);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Create ports based on CID info */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, port_sensor_create,
    s_module);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* intiialize the eeprom */
  ret = mct_list_traverse(module_ctrl->sensor_bundle, module_sensor_init_eeprom,
    module_ctrl->eebin_hdl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Create chromatix manager */
  ret = mct_list_traverse(module_ctrl->sensor_bundle,
    module_sensor_init_chromatix, module_ctrl->eebin_hdl);
  if (ret == FALSE) {
    SERR("failed");
    goto ERROR1;
  }

  /* Initialize dual cam stream mutex */
  pthread_mutex_init(&module_ctrl->dual_cam_mutex, NULL);

  SLOW("SUCCESS");
  return s_module;

  SERR("FAILED");
ERROR1:
  mct_module_destroy(s_module);
  return NULL;
}
