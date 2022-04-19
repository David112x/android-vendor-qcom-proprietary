/*===========================================================================
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#include <cutils/properties.h>
#include "media_controller.h"
#include "modules.h"
#include "mct_stream.h"
#include "port_sensor.h"
#include "sensor_util.h"
#include "sensor_frame_control.h"
#include "sensor_fast_aec.h"
#include "eztune.h"
#include "../flash/module/flash.h"

/** port_sensor_caps_reserve: check caps functiont of find
 *  compatible port
 *
 *  @port: port
 *  @peer_caps: peer port caps
 *  @stream_info: pointer to cam_stream_info_t struct
 *  @streamid: stream id for which this port is linked
 *  @sessionid: session id for which this port is linked
 *
 *  @Return: TRUE if this port can be used for linking, FALSE
 *           otherwise
 *
 *  This function validates port capablities against stream
 *  info and checks whether the format requested by
 *  application is compatible with the format suppport by
 *  given port and returns decision */

static boolean port_sensor_caps_reserve(mct_port_t *port,
  void __attribute__((unused))*peer_caps, void *info)
{
  boolean                           ret = FALSE;
  mct_module_t                     *s_module = NULL;
  module_sensor_ctrl_t             *module_ctrl = NULL;
  mct_list_t                       *port_parent = NULL;
  mct_stream_info_t                *stream_info = (mct_stream_info_t *)info;
  sensor_src_port_cap_t            *sensor_src_port_cap =
    (sensor_src_port_cap_t *)port->caps.u.data;
  module_sensor_port_stream_info_t *port_stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;
  sensor_bundle_info_t              bundle_info;

  SLOW("Enter port %p", port);
  if (!stream_info || !sensor_src_port_cap) {
    SERR("failed stream_info %p, sensor_src_port_cap %p",
      stream_info, sensor_src_port_cap);
    return FALSE;
  }

  port_parent = (mct_list_t *)MCT_PORT_PARENT(port);
  if (!port_parent) {
    SERR("failed");
    return FALSE;
  }

  s_module = (mct_module_t *)port_parent->data;
  if (!s_module) {
    SERR("failed");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)s_module->module_private;
  if (!module_ctrl) {
    SERR("failed");
    return FALSE;
  }

  memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
  ret = sensor_util_get_sbundle(s_module, stream_info->identity, &bundle_info);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }

  /* Check whether session id of incoming request matches with current
     port's session id */
  SLOW("current session id %d port session id %d",
    bundle_info.session_id, sensor_src_port_cap->session_id);
  if (bundle_info.session_id != sensor_src_port_cap->session_id) {
    SHIGH("session id doesn't match: %d : %d",
      bundle_info.session_id,sensor_src_port_cap->session_id);
    return FALSE;
  }

  /* Check whether this incoming format is compatible with current port's
     supported format */
  ret = sensor_util_check_format(sensor_src_port_cap, stream_info);
  if (ret == FALSE) {
    SHIGH("format is not compatible");
    return ret;
  }

  /* add some of stream info to port private
   this will be used for stream bundle */
  port_data = (module_sensor_port_data_t*)(port->port_private);
  port_stream_info = (module_sensor_port_stream_info_t *)
                        malloc(sizeof(module_sensor_port_stream_info_t));
  if (!port_stream_info) {
    SERR("failed");
    return FALSE;
  }
  memset(port_stream_info, 0, sizeof(module_sensor_port_stream_info_t));
  port_stream_info->stream_type = stream_info->stream_type;
  port_stream_info->width = (uint32_t)stream_info->dim.width;
  port_stream_info->height = (uint32_t)stream_info->dim.height;
  port_stream_info->identity = stream_info->identity;
  port_stream_info->stream_info = stream_info;
  port_stream_info->is_stream_on = FALSE;
  port_stream_info->bundle_id = -1;
  port_data->stream_list = mct_list_append(port_data->stream_list,
                                            port_stream_info, NULL, NULL);
  sensor_util_dump_bundle_and_stream_lists(port, __func__, __LINE__);
  bundle_info.s_bundle->stream_mask |=
    (uint32_t)(1 << stream_info->stream_type);
  bundle_info.s_bundle->sensor_common_info.is_secure =
    stream_info->is_secure;

  SLOW("port=%p, identity=0x%x", port, stream_info->identity);
  SLOW("ide %x stream type %d w*h %d*%d", stream_info->identity,
    stream_info->stream_type, stream_info->dim.width, stream_info->dim.height);
  return TRUE;
}

/** port_sensor_caps_unreserve: check caps functiont of find
 *  compatible port
 *
 *  @port: port
 *  @streamid: stream id for which this port is linked
 *  @sessionid: session id for which this port is linked
 *
 *  This function releases resources locked during
 *  caps_reserve */

static boolean port_sensor_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  boolean                      ret = FALSE;
  mct_module_t                *s_module = NULL;
  mct_list_t                  *port_parent = NULL;
  sensor_bundle_info_t         bundle_info;
  struct timespec              ts;

  SLOW("port=%p, identity=0x%x", port, identity);
  if (!port) {
    SERR("failed");
    return FALSE;
  }
  port_parent = (mct_list_t *)MCT_PORT_PARENT(port);
  if (!port_parent) {
    SERR("failed");
    return FALSE;
  }

  s_module = (mct_module_t *)port_parent->data;
  if (!s_module) {
    SERR("failed");
    return FALSE;
  }

  memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
  ret = sensor_util_get_sbundle(s_module, identity, &bundle_info);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }

  TIMED_WAIT_ON_EVENT(bundle_info.s_bundle->mutex, bundle_info.s_bundle->cond,
              ts, INIT_CONFIG_TIMEOUT, bundle_info.s_bundle->init_config_done,
              FALSE, "init_config");

  /* Remove entries in stream and bundle lists correspoding to identity */
  sensor_util_remove_list_entries_by_identity(port, identity);
  sensor_util_dump_bundle_and_stream_lists(port, __func__, __LINE__);
  /* Clear s_bundle max width, height and stream mask */
  bundle_info.s_bundle->max_width = 0;
  bundle_info.s_bundle->max_height = 0;
  bundle_info.s_bundle->stream_mask = 0;
  SLOW("ide %x", identity);
  return TRUE;
}

/** port_sensor_handle_stream_on_normal: handle stream on in port to
 *  figure out FAST AEC session
 *
 *  @module: sensor module
 *  @event: incoming event
 *  @bundle_info: bundle that has sensor information for this
 *              session
 *
 *  This function decides whether fast AEC is required. If
 *  so, it start Fast AEC session and book keeps this stream
 *  that will be used later during normal stream on
 *
 *  Return TRUE on success and FALSE on failure **/
static boolean port_sensor_handle_stream_on_normal(mct_module_t *module,
  mct_event_t *event,
  sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;

  /* Call normal module stream ON */
  ret = module_sensor_stream_on(module, event, bundle_info->s_bundle);
  RETURN_ON_FALSE(ret);

  return TRUE;
}

/** port_sensor_handle_stream_on: handle stream on in port to
 *  figure out FAST AEC session
 *
 *  @module: sensor module
 *  @event: incoming event
 *  @bundle_info: bundle that has sensor information for this
 *              session
 *
 *  This function decides whether fast AEC is required. If
 *  so, it start Fast AEC session and book keeps this stream
 *  that will be used later during normal stream on
 *
 *  Return TRUE on success and FALSE on failure **/
boolean port_sensor_handle_stream_on(mct_module_t *module, mct_event_t *event,
  sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;
  mct_port_t *port = NULL;

  if (bundle_info->s_bundle->sensor_num_fast_aec_frame_skip &&
      bundle_info->s_bundle->fast_aec_required)
    ret = port_sensor_handle_stream_on_fastaec(module, event, bundle_info);
  else
    ret = port_sensor_handle_stream_on_normal(module, event, bundle_info);

   RETURN_ON_FALSE(ret);
   /* Set stream ON flag */
   ret = sensor_util_set_stream_on(module, event->identity, TRUE);
   RETURN_ON_FALSE(ret);

  return ret;
}

static boolean port_sensor_ext_link_func(uint32_t identity,
 mct_port_t* port, mct_port_t *peer)
{
  boolean                      ret = TRUE;

  SLOW("ide %x", identity);
  RETURN_ON_NULL(port);

  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port) = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto ERROR;
  }
  SLOW("Exit");
  return ret;
ERROR:
  SERR("failed");
  return FALSE;
}

static void port_sensor_unlink_func(uint32_t identity,
 mct_port_t *port, mct_port_t *peer)
{
  SLOW("ide %x", identity);
  RETURN_VOID_ON_NULL(port);

  if (MCT_PORT_PEER(port) != peer) {
    SERR("failed");
    return;
  }

  SLOW("Exit");
  return;
}

#if SENSOR_MEASURE_FPS
static void __attribute__((unused)) port_sensor_measure_fps()
{
  static struct timeval old_tv;
  struct timeval new_tv;
  static int32_t count=0;
  static double fps_sum=0.0;
  gettimeofday(&new_tv, NULL);
  long timediff =
    (new_tv.tv_sec - old_tv.tv_sec)*1000000L +
    (new_tv.tv_usec - old_tv.tv_usec);
  fps_sum += 1000000.0/(timediff);
  old_tv = new_tv;
  count++;
  if (count == 10) {
    SHIGH("sensor_fps = %.2f", fps_sum/10);
    fps_sum = 0.0;
    count = 0;
  }
 return;
}
#else
static void __attribute__((unused)) port_sensor_measure_fps()
{
  return;
}
#endif

/** port_sensor_aec_est_frame_skip: avoid partial lit frame
 *  during aec estimation
 *  @module: mct module handle
 *  @port: mct port handle
 *  @event: event to be processed
 *  @bundle_info: sensor bundle info
 *  @event_module: event module handle
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles aec frame skip **/

static void port_sensor_aec_est_frame_skip(
  mct_module_t *module, mct_event_t *event,
  sensor_bundle_info_t *bundle_info)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_capture_control_t *ctrl = &bundle_info->s_bundle->cap_control;
  sensor_batch_config_t *bctrl = &ctrl->bctrl[0];
  uint32_t max_frame_skip = 0;
  memset(&(bundle_info->s_bundle->cap_control), 0,
    sizeof(sensor_capture_control_t));
  ctrl->num_batches = 1;
  if (bundle_info->s_bundle->state == SENSOR_AEC_EST_START)
    bctrl->interval = bundle_info->s_bundle->torch_on_frame_skip;
  else
    bctrl->interval = bundle_info->s_bundle->torch_off_frame_skip;

  rc = sensor_util_sw_frame_skip_to_isp(module, event->identity, bundle_info->s_bundle,
               IFACE_SKIP_RANGE, TRUE, FALSE, bctrl->interval);
  if (rc) {
    SHIGH("sensor_util_sw_frame_skip_to_isp succeed");
  } else {
    SHIGH("setting torch on/off frameskip to ISP failed. retry later");
    ctrl->retry_frame_skip =
      bundle_info->s_bundle->retry_frame_skip;
  }
  ctrl->max_pipeline_delay =
    bundle_info->s_bundle->capture_pipeline_delay;
  ctrl->evt[0] = SENSOR_AEC_EST_EVT;

  bctrl->ctrl_start_frame =
    bundle_info->s_bundle->last_idx + 1;
  bctrl->ctrl_end_frame =
    bctrl->ctrl_start_frame + bctrl->interval;
  bctrl->type = CAM_CAPTURE_FLASH;
  bctrl->led_toggle_pending = TRUE;
  if (bundle_info->s_bundle->state == SENSOR_AEC_EST_START) {
    bctrl->flash_mode = LED_FLASH_SET_PRE_FLASH;
    bctrl->cam_flash_mode = CAM_FLASH_MODE_TORCH;
  } else {
    bctrl->flash_mode = LED_FLASH_SET_OFF;
    bctrl->cam_flash_mode = CAM_FLASH_MODE_OFF;
  }
  /* Enable capture frame control */
  ctrl->enable = 1;
}

/** port_sensor_handle_aec_update: process event for sensor
 *  module
 *
 *  @module: mct module handle
 *  @port: mct port handle
 *  @event: event to be processed
 *  @bundle_info: sensor bundle info
 *  @event_module: event module handle
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles aec update event **/

boolean port_sensor_handle_aec_update(
  mct_module_t *module, mct_port_t *port, mct_event_t *event,
  sensor_bundle_info_t *bundle_info, mct_event_module_t *event_module)
{
  int32_t rc = SENSOR_SUCCESS;
  boolean ret = TRUE;
  sensor_frame_ctrl_params_t *frame_ctrl = NULL;
  uint32_t    sensor_delay, report_delay;
  module_sensor_params_t *module_sensor_params =
    bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  module_sensor_params_t *stereo_peer_module_sensor_params;
  stats_update_t *stats_update =
    (stats_update_t *)event_module->module_event_data;
  module_sensor_bundle_info_t *s_bundle = bundle_info->s_bundle;
  module_sensor_params_t *led_module_params =
    bundle_info->s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
  dual_led_settings_t dual_led_settings;
  float digital_gain = 0;
  mct_event_t new_event;
  mct_bus_msg_t bus_msg;
  stats_get_data_t *stats_get = NULL;
  module_sensor_ctrl_t *sensor_ctrl;

  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  if (!stats_update || !module_sensor_params) {
    SERR("failed");
    return FALSE;
  }
  SLOW("stats sensor_gain %f real_gain=%f lnct %d exp time %f",
    stats_update->aec_update.sensor_gain, stats_update->aec_update.real_gain,
    stats_update->aec_update.linecount, stats_update->aec_update.exp_time);

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (sensor_ctrl->is_dual_cam == TRUE &&
    stats_update->aec_update.exp_time == 0)
    return FALSE;

  s_bundle->ir_mode = stats_update->aec_update.ir_config.ir_needed;
  s_bundle->ir_led_brightness = stats_update->aec_update.ir_config.ir_led_brightness;
  s_bundle->sensor_common_info.ir_mode = s_bundle->ir_mode;

  if (stats_update->flag & STATS_UPDATE_AEC) {
    SLOW("stats update aec");
    frame_ctrl = &bundle_info->s_bundle->frame_ctrl;
    report_delay = frame_ctrl->session_data.max_pipeline_meta_reporting_delay;
    sensor_delay = frame_ctrl->max_sensor_delay;
    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_AEC_UPDATE, &stats_update->aec_update);
    if (rc < 0) {
        SERR("failed");
        return FALSE;
    }

    /* Set exposure on peer stereo sensor if needed. */
    if (TRUE == bundle_info->s_bundle->is_stereo_configuration &&
      bundle_info->s_bundle->stereo_peer_s_bundle != NULL) {
      module_sensor_params_t *stereo_peer_module_sensor_params =
        bundle_info->s_bundle->stereo_peer_s_bundle->
        module_sensor_params[SUB_MODULE_SENSOR];
      if(NULL != stereo_peer_module_sensor_params) {
        rc = stereo_peer_module_sensor_params->func_tbl.process(
          stereo_peer_module_sensor_params->sub_module_private,
          SENSOR_SET_AEC_UPDATE, &stats_update->aec_update);
        if (rc < 0) {
            SERR("failed");
            return FALSE;
        }
      }
    }

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_DIGITAL_GAIN, &digital_gain);
    if (rc < 0) {
        SERR("failed");
        return FALSE;
    }

    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EXT] != -1) {
      if (s_bundle->cap_control.enable &&
        (s_bundle->capture_pipeline_delay >
        s_bundle->ext_pipeline_delay)) {
        int delay = s_bundle->capture_pipeline_delay -
          s_bundle->ext_pipeline_delay;
        sensor_fc_store(s_bundle, s_bundle->last_idx + delay,
          SENSOR_FRAME_CTRL_EXT_AEC_UPDATE, NULL);
      } else {
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EXT,
          EXT_AEC_UPDATE, NULL, rc);
      }
    }

    if (led_module_params->func_tbl.process != NULL &&
          stats_update->aec_update.dual_led_setting.is_valid) {
      dual_led_settings.low_setting[0] =
            stats_update->aec_update.dual_led_setting.led1_low_setting;
      dual_led_settings.low_setting[1] =
            stats_update->aec_update.dual_led_setting.led2_low_setting;
      dual_led_settings.high_setting[0] =
           stats_update->aec_update.dual_led_setting.led1_high_setting;
      dual_led_settings.high_setting[1] =
            stats_update->aec_update.dual_led_setting.led2_high_setting;
      rc = led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_CURRENT, &dual_led_settings);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }
    }

    if (stats_update->aec_update.est_state == AEC_EST_NO_LED_DONE &&
         bundle_info->s_bundle->state!=SENSOR_AEC_EST_NO_LED) {
       bundle_info->s_bundle->state = SENSOR_AEC_EST_NO_LED;
       SHIGH("AEC EST DONE, no led needed. Do not need future frame");
       bundle_info->s_bundle->regular_led_trigger = 0;
       bus_msg.sessionid = bundle_info->session_id;
       bus_msg.type = MCT_BUS_MSG_PREPARE_HW_DONE;
       cam_prep_snapshot_state_t state;
       state = DO_NOT_NEED_FUTURE_FRAME;
       bus_msg.msg = &state;
       bus_msg.size = sizeof(cam_prep_snapshot_state_t);
       if(mct_module_post_bus_msg(module,&bus_msg)!=TRUE) {
           SERR("Failure posting to the bus!");
           return FALSE;
        }
    } else if (stats_update->aec_update.est_state == AEC_EST_DONE &&
        bundle_info->s_bundle->state!=SENSOR_AEC_EST_DONE
        && bundle_info->s_bundle->torch_on == 0) {
      bundle_info->s_bundle->state = SENSOR_AEC_EST_DONE;
      SHIGH ("AEC EST DONE");
      if (bundle_info->s_bundle->partial_flash_frame_skip
        && bundle_info->s_bundle->hal_version == CAM_HAL_V1) {
          port_sensor_aec_est_frame_skip(module, event, bundle_info);
      } else {
        if (led_module_params->func_tbl.process != NULL) {
          rc = led_module_params->func_tbl.process(
            led_module_params->sub_module_private,
            LED_FLASH_SET_OFF, NULL);
          SLOW("%s, pre-flash: OFF", __func__);
          if (rc < 0) {
            SERR("failed: LED_FLASH_SET_OFF");
          } else {
            bundle_info->s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
            bundle_info->s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
            sensor_util_post_led_state_msg(module, bundle_info->s_bundle,
              event->identity);
          }
        }
        if (bundle_info->s_bundle->regular_led_af == 0) {
          if(bundle_info->s_bundle->flash_rer_enable) {
            SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle,
              SUB_MODULE_LED_FLASH, LED_FLASH_SET_RER_PROCESS,
              led_module_params, rc);
          }
          SHIGH("Prepare for capture with LED AEC DONE. Need future frame");

          bundle_info->s_bundle->regular_led_trigger = 1;
          bus_msg.sessionid = bundle_info->session_id;
          bus_msg.type = MCT_BUS_MSG_PREPARE_HW_DONE;
          cam_prep_snapshot_state_t state;
          state = NEED_FUTURE_FRAME;
          bus_msg.msg = &state;
          bus_msg.size = sizeof(cam_prep_snapshot_state_t);
          rc = mct_module_post_bus_msg(module,&bus_msg);
          if(rc != TRUE)
            SERR("Failure posting to the bus!");
        } else {
          /* Sensor takes responsibility of resetting this flag
             in LDF AF usecase in order to avoid race condition
             in HAL1 where sending MCT_BUS_MSG_PREPARE_HW_DONE
             happens in different thread */
          bundle_info->s_bundle->regular_led_af = 0;
        }
      }
    } else if (stats_update->aec_update.est_state == AEC_EST_DONE_SKIP) {
      SHIGH("AEC EST DONE SKIP");
      bundle_info->s_bundle->regular_led_af = 0;
      if (led_module_params->func_tbl.process != NULL) {
        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_OFF, NULL);
        if (rc < 0) {
          SERR("failed: LED_FLASH_SET_OFF");
        } else {
          bundle_info->s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
          bundle_info->s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
          sensor_util_post_led_state_msg(module, bundle_info->s_bundle,
            event->identity);
        }
      }

     if(bundle_info->s_bundle->flash_rer_enable) {
       SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle,
         SUB_MODULE_LED_FLASH, LED_FLASH_SET_RER_PROCESS,
         led_module_params, rc);
     }
      bundle_info->s_bundle->regular_led_trigger = 1;
      bus_msg.sessionid = bundle_info->session_id;
      bus_msg.type = MCT_BUS_MSG_PREPARE_HW_DONE;
      cam_prep_snapshot_state_t state;
      state = NEED_FUTURE_FRAME;
      bus_msg.msg = &state;
      bus_msg.size = sizeof(cam_prep_snapshot_state_t);
      rc = mct_module_post_bus_msg(module,&bus_msg);
      if(rc != TRUE)
        SERR("Failure posting to the bus!");
    } else if (stats_update->aec_update.est_state == AEC_EST_START
          && bundle_info->s_bundle->state != SENSOR_AEC_EST_START) {
        SHIGH("AEC EST START");
        bundle_info->s_bundle->state = SENSOR_AEC_EST_START;
        if (bundle_info->s_bundle->dual_led_calib_enabled) {
          if (led_module_params->func_tbl.process != NULL) {
            SLOW("In dual LED calibration mode");
            rc = led_module_params->func_tbl.process(
              led_module_params->sub_module_private,
              LED_FLASH_SET_MAIN_FLASH, NULL);
            if (rc < 0) {
              SERR("failed: LED_FLASH_SET_MAIN_FLASH");
            } else {
              bundle_info->s_bundle->sensor_params.flash_mode =
                CAM_FLASH_MODE_ON;
              bundle_info->s_bundle->sensor_params.flash_state =
                CAM_FLASH_STATE_FIRED;
              sensor_util_post_led_state_msg(module, bundle_info->s_bundle,
                event->identity);
            }
          }
        } else if (bundle_info->s_bundle->partial_flash_frame_skip
          && bundle_info->s_bundle->hal_version == CAM_HAL_V1) {
          port_sensor_aec_est_frame_skip(module, event, bundle_info);
        } else if (led_module_params->func_tbl.process != NULL) {
          rc = led_module_params->func_tbl.process(
            led_module_params->sub_module_private,
            LED_FLASH_SET_PRE_FLASH, NULL);
          if (rc < 0) {
            SERR("failed: LED_FLASH_SET_PRE_FLASH");
          } else {
            bundle_info->s_bundle->sensor_params.flash_mode =
              CAM_FLASH_MODE_TORCH;
            bundle_info->s_bundle->sensor_params.flash_state =
              CAM_FLASH_STATE_FIRED;
            sensor_util_post_led_state_msg(module, bundle_info->s_bundle,
              event->identity);
          }
        }
    }
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity = event->identity;
    new_event.direction = MCT_EVENT_DOWNSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_SET_DIGITAL_GAIN;
    new_event.u.module_event.module_event_data = (void *)&digital_gain;
    ret = mct_port_send_event_to_peer(port, &new_event);
    if (ret == FALSE) {
      SERR("failed");
    }
    if (sizeof(stats_get_data_t) >
        sizeof(bundle_info->s_bundle->aec_metadata.private_data)) {
      SERR("failed");
    } else {
      stats_get =
        (stats_get_data_t *)bundle_info->s_bundle->aec_metadata.private_data;
      if (stats_get && ((stats_get->aec_get.real_gain[0] !=
          stats_update->aec_update.sensor_gain) ||
         (stats_get->aec_get.lux_idx != stats_update->aec_update.lux_idx))) {
        sensor_aec_update_t  aec_update;
        aec_update.real_gain = stats_update->aec_update.real_gain;
        aec_update.linecount = stats_update->aec_update.linecount;
        aec_update.sensor_gain = stats_update->aec_update.sensor_gain;
        aec_update.est_state = 0;
        aec_update.lux_idx = stats_update->aec_update.lux_idx;
        aec_update.hdr_sensitivity_ratio =
          stats_update->aec_update.hdr_sensitivity_ratio;
        aec_update.hdr_exp_time_ratio =
          stats_update->aec_update.hdr_exp_time_ratio;
        aec_update.total_drc_gain =
          stats_update->aec_update.total_drc_gain;
        stats_get->aec_get.sensor_gain[0] = stats_update->aec_update.sensor_gain;
        stats_get->aec_get.real_gain[0] = stats_update->aec_update.real_gain;
        stats_get->aec_get.lux_idx = stats_update->aec_update.lux_idx;
        stats_get->aec_get.exif_iso = stats_update->aec_update.exif_iso;
        stats_get->aec_get.valid_entries = 1;
        SLOW("[store meta] SENSOR_FRAME_CTRL_EXP_META future sof %d",
          bundle_info->s_bundle->last_idx + sensor_delay + report_delay);
        SLOW("stored data = g %f lux idx %f", stats_update->aec_update.sensor_gain,
          stats_update->aec_update.lux_idx);
        if (sensor_fc_store(bundle_info->s_bundle,
          bundle_info->s_bundle->last_idx + sensor_delay + report_delay,
          SENSOR_FRAME_CTRL_EXP_META, &aec_update) == FALSE)
          SERR("failed");
      }
    }
  }
  return ret;
}

static boolean port_sensor_handle_manual_aec_update(
  mct_module_t *module, mct_port_t *port, mct_event_t *event,
  sensor_bundle_info_t *bundle_info, mct_event_module_t *event_module)
{
  module_sensor_params_t     *module_sensor_params;
  sensor_frame_ctrl_params_t *frame_ctrl;
  aec_manual_update_t        *stats_update;
  sensor_aec_update_t         aec_update;
  uint32_t                    sensor_delay, report_delay;
  uint32_t                    apply_frame_id;
  uint32_t                    cur_sof;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(port);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(bundle_info);
  RETURN_ON_NULL(event_module);

  module_sensor_params =
    bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);
  stats_update = (aec_manual_update_t *)event_module->module_event_data;
  RETURN_ON_NULL(stats_update);

  PTHREAD_MUTEX_LOCK(&(bundle_info->s_bundle->mutex));
  cur_sof = bundle_info->s_bundle->last_idx;
  PTHREAD_MUTEX_UNLOCK(&(bundle_info->s_bundle->mutex));

  aec_update.real_gain = stats_update->sensor_gain;
  aec_update.linecount = stats_update->linecount;
  aec_update.est_state = 0;
  aec_update.lux_idx = 0;

  SLOW("g %f lc %d mctl id %d cur sof %d",
    aec_update.real_gain, aec_update.linecount,
    event_module->current_frame_id, cur_sof);

  frame_ctrl = &bundle_info->s_bundle->frame_ctrl;

  apply_frame_id = event_module->current_frame_id +
    frame_ctrl->session_data.max_pipeline_frame_applying_delay -
    frame_ctrl->max_sensor_delay;
  SLOW("apply frame id %d", apply_frame_id);
  report_delay = frame_ctrl->session_data.max_pipeline_meta_reporting_delay;
  sensor_delay = frame_ctrl->max_sensor_delay;

  if (apply_frame_id < cur_sof) {
    SERR("cannot apply exposure mctl id %d cur sof %d",
      event_module->current_frame_id, cur_sof);
    return FALSE;
  }

  /* APPLY or STORE event
     : Depends on the current frame id */
  if (apply_frame_id == cur_sof) {
    SLOW("[apply exposure] g %f lc %d",
      aec_update.real_gain, aec_update.linecount);
    if (module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_AEC_MANUAL_UPDATE, &aec_update) < 0) {
      SERR("failed from SENSOR_SET_AEC_MANUAL_UPDATE");
      return FALSE;
    }
    /* Set exposure on peer stereo sensor if needed. */
    if (TRUE == bundle_info->s_bundle->is_stereo_configuration &&
      bundle_info->s_bundle->stereo_peer_s_bundle != NULL) {
      module_sensor_params_t *stereo_peer_module_sensor_params =
        bundle_info->s_bundle->stereo_peer_s_bundle->
        module_sensor_params[SUB_MODULE_SENSOR];
      if(NULL != stereo_peer_module_sensor_params) {
        SLOW("[apply exposure stereo peer] g %f lc %d",
            aec_update.real_gain, aec_update.linecount);
        if (stereo_peer_module_sensor_params->func_tbl.process(
          stereo_peer_module_sensor_params->sub_module_private,
          SENSOR_SET_AEC_MANUAL_UPDATE, &aec_update) < 0) {
            SERR("failed from SENSOR_SET_AEC_MANUAL_UPDATE for stereo pair.");
            return FALSE;
        }
      }
    }
    if (sensor_util_set_digital_gain_to_isp(module,
      bundle_info->s_bundle, event->identity, &aec_update) == FALSE)
      SERR("failed");
  } else {
    SLOW("[store exposure] future sof %d g %f lc %d",
      apply_frame_id, aec_update.real_gain, aec_update.linecount);
    if (sensor_fc_store(bundle_info->s_bundle, apply_frame_id,
      SENSOR_FRAME_CTRL_EXP, &aec_update) == FALSE) {
      SERR("failed from sensor_fc_store");
      return FALSE;
    }
  }

  SLOW("[store meta] SENSOR_FRAME_CTRL_EXP_META future sof %d",
    apply_frame_id + sensor_delay + report_delay);
  if (sensor_fc_store(bundle_info->s_bundle,
    apply_frame_id + sensor_delay + report_delay,
    SENSOR_FRAME_CTRL_EXP_META, &aec_update) == FALSE)
    SERR("failed");

  return TRUE;
}

static boolean module_sensor_handle_isp_frame_skip(mct_port_t *port,
  sensor_bundle_info_t *bundle_info,
  uint32_t identity, void *data)
{
  boolean ret = TRUE;
  int32_t rc = 0, stream_type;

  if (!data)
    return FALSE;

  stream_type = sensor_util_find_stream_type_for_stream(port, identity);
  if (stream_type < 0 || stream_type >= CAM_STREAM_TYPE_MAX )
    return FALSE;

  bundle_info->s_bundle->isp_frameskip[stream_type] = *(uint32_t*)data;
  SLOW("stream_type:%d, skip:%d",stream_type,
    bundle_info->s_bundle->isp_frameskip[stream_type]);
  if (bundle_info->s_bundle->max_isp_frame_skip <
    bundle_info->s_bundle->isp_frameskip[stream_type]) {
      bundle_info->s_bundle->max_isp_frame_skip =
        bundle_info->s_bundle->isp_frameskip[stream_type];
  }
  return TRUE;
}

static boolean module_sensor_handle_eztune_chromatix_event(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t* s_bundle)
{
  boolean                   ret = TRUE;
  int32_t                   rc = SENSOR_SUCCESS;
  mct_event_module_t       *event_module = NULL;
  module_sensor_params_t   *chromatix_module_params = NULL;
  eztune_chromatix_t       *chromatix = NULL;
  sensor_chromatix_params_t chromatix_params;

  event_module = &event->u.module_event;
  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];

  RETURN_ON_NULL(chromatix_module_params);
  RETURN_ON_NULL(event_module->module_event_data);

  memset(&chromatix_params,0,sizeof(sensor_chromatix_params_t));

  chromatix = (eztune_chromatix_t *)event_module->module_event_data;

  switch (event_module->type) {
  case MCT_EVENT_MODULE_EZTUNE_GET_CHROMATIX:
  {
    rc = chromatix_module_params->func_tbl.process(
      chromatix_module_params->sub_module_private,
      CHROMATIX_GET_CACHED_PTR, &chromatix_params);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    memcpy(chromatix->chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP],
      sizeof(chromatix_parms_type));
    memcpy(chromatix->snap_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP_SNAPSHOT],
      sizeof(chromatix_parms_type));
    memcpy(chromatix->common_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON],
      sizeof(chromatix_VFE_common_type));
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x304
    memcpy(chromatix->cpp_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_CPP_PREVIEW],
      sizeof(chromatix_cpp_type));
    memcpy(chromatix->snap_cpp_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_CPP_SNAPSHOT],
      sizeof(chromatix_cpp_type));
    memcpy(chromatix->postproc_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_SW_PPROC],
      sizeof(chromatix_sw_postproc_type));
#endif
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x307
    memcpy(chromatix->aaa_chromatixData,
      chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_3A],
      sizeof(chromatix_3a_parms_type));
#endif

    break;
  }

  case MCT_EVENT_MODULE_EZTUNE_SET_CHROMATIX:
  {
    boolean is_non_zsl_snapshot = FALSE;
    rc = chromatix_module_params->func_tbl.process(
      chromatix_module_params->sub_module_private,
      CHROMATIX_GET_CACHED_PTR, &chromatix_params);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    if (s_bundle->hal_version == CAM_HAL_V1) {
      if (chromatix_params.stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) &&
      !(chromatix_params.stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)))
        is_non_zsl_snapshot = TRUE;
    } else if (s_bundle->hal_version == CAM_HAL_V3 &&
      s_bundle->capture_intent == CAM_INTENT_STILL_CAPTURE) {
        is_non_zsl_snapshot = TRUE;
    }

    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP],
      chromatix->chromatixData, sizeof(chromatix_parms_type));

    if(is_non_zsl_snapshot) {
      memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP_SNAPSHOT],
        chromatix->snap_chromatixData, sizeof(chromatix_parms_type));
    }

    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON],
      chromatix->common_chromatixData, sizeof(chromatix_VFE_common_type));
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x304
    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_CPP_PREVIEW],
      chromatix->cpp_chromatixData, sizeof(chromatix_cpp_type));
    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_CPP_SNAPSHOT],
      chromatix->snap_cpp_chromatixData, sizeof(chromatix_cpp_type));
    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_SW_PPROC],
      chromatix->postproc_chromatixData, sizeof(chromatix_sw_postproc_type));
#endif
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x307
    memcpy(chromatix_params.chromatix_ptr[SENSOR_CHROMATIX_3A],
      chromatix->aaa_chromatixData,
      sizeof(chromatix_3a_parms_type));
#endif

    RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(module,
      s_bundle, event->identity, &chromatix_params,
      CAMERA_CHROMATIX_MODULE_ALL));

    break;
  }

  default:
    SHIGH("error: unknown entry (default case)");
    break;
  }

  return ret;
}

static boolean module_sensor_handle_eztune_aftune_event(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t* s_bundle)
{
  boolean                   ret = TRUE;
  int32_t                   rc = SENSOR_SUCCESS;
  mct_event_module_t       *event_module = NULL;
  module_sensor_params_t   *actuator_module_params = NULL;
  actuator_driver_params_t *eztune_af_driver_ptr = NULL;
  actuator_driver_params_t *af_driver_ptr = NULL;

  event_module = &event->u.module_event;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    actuator_module_params =
        s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
  } else {
    SHIGH("No Actuator Module!");
    return FALSE;
  }

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(actuator_module_params);
  RETURN_ON_NULL(event_module->module_event_data);

  eztune_af_driver_ptr = (actuator_driver_params_t *)
    event_module->module_event_data;

  RETURN_ON_NULL(eztune_af_driver_ptr);

  switch (event_module->type) {
  case MCT_EVENT_MODULE_EZTUNE_GET_AFTUNE:
  {
    rc = actuator_module_params->func_tbl.process(
      actuator_module_params->sub_module_private,
      ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr);
    if (rc < 0 || !af_driver_ptr) {
      SERR("failed rc %d af_tune_ptr %p", rc, af_driver_ptr);
      ret = FALSE;
      break;
    }

    memcpy(eztune_af_driver_ptr, af_driver_ptr,
      sizeof(actuator_driver_params_t));
    break;
  }

  case MCT_EVENT_MODULE_EZTUNE_SET_AFTUNE:
  {
    rc = actuator_module_params->func_tbl.process(
      actuator_module_params->sub_module_private,
      ACTUATOR_GET_AF_DRIVER_PARAM_PTR, &af_driver_ptr);
    if (rc < 0 || !af_driver_ptr) {
      SERR("failed rc %d af_tune_ptr %p", rc, af_driver_ptr);
      ret = FALSE;
      break;
    }

    memcpy(af_driver_ptr, eztune_af_driver_ptr,
      sizeof(actuator_driver_params_t));
    break;
  }

  default:
    SHIGH("error: unknown entry (default case)");
    break;
  }

  return ret;
}

/** port_sensor_handle_upstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *  @bundle_info: sensor bundle info
 *
 *  Handle upstream module event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_sensor_handle_upstream_module_event(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  int32_t             rc = 0;
  boolean             ret = TRUE;
  mct_event_module_t *event_module = NULL;

  if (!module || !port || !event || !bundle_info) {
    SERR("failed: %p %p %p %p", module, port, event, bundle_info);
    return FALSE;
  }
  event_module = &event->u.module_event;
  SLOW("event id %d", event_module->type);
  switch (event_module->type) {
  case MCT_EVENT_MODULE_GET_CHROMATIX_PTR:
    if (bundle_info->s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX]) {
      event_module->module_event_data =
        bundle_info->s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX]->
        sub_module_private;
    } else {
      SERR("failed");
      ret = FALSE;
    }
    break;
  case MCT_EVENT_MODULE_DEPTH_LASER_CURRENT: {
    module_sensor_params_t *module_sensor_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_LASER_LED];
      sensor_ctrl_t *ctrl;

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      LASER_LED_SET_CURRENT,
      (uint32_t *)event_module->module_event_data);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }
    break;
  }
  case MCT_EVENT_MODULE_DEPTH_EXPOSURE_TIME: {
    module_sensor_params_t *module_sensor_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

    rc = module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_SET_MANUAL_EXPOSURE_TIME,
      (int64_t *)event_module->module_event_data);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }
    break;
  }
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {

    module_sensor_ctrl_t *sensor_ctrl =
      (module_sensor_ctrl_t *)module->module_private;

    /* normal case : call port_sensor_handle_aec_update() */
    if (sensor_ctrl->is_dual_cam == FALSE) {
      ret = port_sensor_handle_aec_update(module,
        port, event, bundle_info, event_module);
      if (ret == FALSE)
        SERR("failed");
    }
    else
    {
      /*Handle AEC update in case of dual camera system */
      module_sensor_dualcamera_handle_aec_update(module,
        port, event, bundle_info);
    }
    break;
  }
  case MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE: {
    ret = port_sensor_handle_manual_aec_update(module,
      port, event, bundle_info, event_module);
    if (ret == FALSE) {
      SERR("failed");
    }
    break;
  }

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    module_sensor_params_t *module_sensor_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    module_sensor_params_t *led_module_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    dual_led_settings_t dual_led_settings;

    stats_update_t *stats_update =
      (stats_update_t *)event_module->module_event_data;
    sensor_ctrl_t *ctrl;

    if (!stats_update || !module_sensor_params) {
      SERR("failed");
      return FALSE;
    }

    if (stats_update->flag & STATS_UPDATE_AWB) {
      SLOW("stats update awb hdr");
      ctrl = module_sensor_params->sub_module_private;
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_SET_AWB_UPDATE, &stats_update->awb_update);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

      if (led_module_params->func_tbl.process != NULL &&
            stats_update->awb_update.dual_led_setting.is_valid) {
      dual_led_settings.low_setting[0] =
            stats_update->awb_update.dual_led_setting.led1_low_setting;
      dual_led_settings.low_setting[1] =
            stats_update->awb_update.dual_led_setting.led2_low_setting;
      dual_led_settings.high_setting[0] =
           stats_update->awb_update.dual_led_setting.led1_high_setting;
      dual_led_settings.high_setting[1] =
            stats_update->awb_update.dual_led_setting.led2_high_setting;

        rc = led_module_params->func_tbl.process(
          led_module_params->sub_module_private,
          LED_FLASH_SET_CURRENT, &dual_led_settings);
        if (rc < 0) {
          SERR("failed");
          return FALSE;
        }
      }
    }
    break;
  }

  case MCT_EVENTS_MODULE_PDAF_ISP_INFO:{
    SLOW("MCT_EVENTS_MODULE_PDAF_ISP_INFO");
    pdaf_init_info_t    s_pdaf;

    if (event_module->module_event_data == NULL) {
     SERR("PDAF_ISP_INFO failed");
     ret = FALSE;
     break;
    }
    s_pdaf.isp_config =
     (pdaf_data_buffer_info_t *)event_module->module_event_data;

    if(bundle_info->s_bundle->formatted_data) {
      s_pdaf.pdaf_cal_data =
        (void*)&bundle_info->s_bundle->formatted_data->pdafc_2d;
      s_pdaf.macro_dac = bundle_info->s_bundle->formatted_data->afc.macro_dac;
      s_pdaf.infinity_dac =
        bundle_info->s_bundle->formatted_data->afc.infinity_dac;
      s_pdaf.actuator_sensitivity =
        bundle_info->s_bundle->actuator_sensitivity;
      s_pdaf.cur_logical_lens_pos =
        bundle_info->s_bundle->cur_logical_lens_pos;
      SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_SENSOR,
        SENSOR_PDAF_INIT, &s_pdaf, rc);
    } else {
      SERR("Pdaf formatted data is unavailable");
      rc = -1;
    }

    if (rc < 0) {
      SERR("PDAF init failed");
      ret = FALSE;
    }
    break;
  }

  case MCT_EVENT_MODULE_STATS_PDAF_UPDATE: {
    pdaf_update_t *pdaf_update =
      (pdaf_update_t *)event_module->module_event_data;
    pdaf_params_t s_pdaf;
    format_data_t *formatted_data;
    unsigned int i = 0;
    SLOW("MCT_EVENT_MODULE_STATS_PDAF_UPDATE");
    if (!pdaf_update) {
      SERR("PDAF_UPDATE failed");
      ret = FALSE;
      break;
    }

    memset(&s_pdaf, 0, sizeof(pdaf_params_t));

    /* populate sensor pdaf input param*/
    s_pdaf.status      = FALSE;
    s_pdaf.pd_stats    = pdaf_update->pd_stats;
    s_pdaf.eeprom_data = bundle_info->s_bundle->formatted_data;
    s_pdaf.cur_logical_lens_pos = bundle_info->s_bundle->cur_logical_lens_pos;
    s_pdaf.num_of_valid_data_wrt_camif = pdaf_update->num_of_valid_data_wrt_camif;
    SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_ACTUATOR,
      ACTUATOR_GET_DAC_VALUE, &s_pdaf.cur_lens_pos, rc);

    /* parse PD stats */
    SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_SENSOR,
      SENSOR_PDAF_PARSE_PD, &s_pdaf, rc);
    if (rc < 0) {
      SERR("PDAF parsing failed");
      ret = FALSE;
      break;
    }

    /* calculate defocus using API */
    SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_SENSOR,
      SENSOR_PDAF_CAL_DEFOCUS, &s_pdaf, rc);
    if (rc < 0) {
      SERR("PDAF calculate defocus failed");
      ret = FALSE;
      break;
    }

    /* populate sensor pdaf output */
    pdaf_update->x_offset = s_pdaf.x_offset;
    pdaf_update->y_offset = s_pdaf.y_offset;
    pdaf_update->width = s_pdaf.width;
    pdaf_update->height = s_pdaf.height;
    pdaf_update->x_win_num = s_pdaf.x_win_num;
    pdaf_update->y_win_num = s_pdaf.y_win_num;
    pdaf_update->status = s_pdaf.status;
    pdaf_update->stat_process_result = (pdaf_lib_processing_result_t)s_pdaf.stat_process_result;
    for(i = 0;i < pdaf_update->y_win_num * pdaf_update->x_win_num +1; i++) {
      pdaf_update->defocus[i].defocus = s_pdaf.defocus[i].defocus;
      pdaf_update->defocus[i].df_confidence= s_pdaf.defocus[i].df_confidence;
      pdaf_update->defocus[i].df_conf_level= s_pdaf.defocus[i].df_conf_level;
      pdaf_update->defocus[i].phase_diff = s_pdaf.defocus[i].phase_diff;
      SLOW("window %d defocus %d, df_confidence %d, df_conf_level %d, phase_diff %f",
           i,
           pdaf_update->defocus[i].defocus,
           pdaf_update->defocus[i].df_confidence,
           pdaf_update->defocus[i].df_conf_level,
           pdaf_update->defocus[i].phase_diff);
    }

    /* populate peripheral pdaf data if it's valid */
    if (TRUE == s_pdaf.is_peripheral_valid) {
      for (i = 0; i < MAX_PERIPHERAL_WINDOW; i++) {
        pdaf_update->peripheral[i].defocus = s_pdaf.peripheral[i].defocus;
        pdaf_update->peripheral[i].df_confidence= s_pdaf.peripheral[i].df_confidence;
        pdaf_update->peripheral[i].df_conf_level= s_pdaf.peripheral[i].df_conf_level;
        pdaf_update->peripheral[i].phase_diff = s_pdaf.peripheral[i].phase_diff;
        SLOW("window %d defocus %d, df_confidence %d, df_conf_level %d, phase_diff %f",
           i,
           pdaf_update->peripheral[i].defocus,
           pdaf_update->peripheral[i].df_confidence,
           pdaf_update->peripheral[i].df_conf_level,
           pdaf_update->peripheral[i].phase_diff);
      }
    } else {
      memset(pdaf_update->peripheral, 0 ,sizeof(pdaf_update->single));
    }
    pdaf_update->is_peripheral_valid = s_pdaf.is_peripheral_valid;

    /* populate single pdaf data if it's valid */
    if (TRUE == s_pdaf.is_single_valid) {
      for (i = 0; i < MAX_SINGLE_WINDOWS; i++) {
        pdaf_update->single[i].defocus = s_pdaf.single[i].defocus;
        pdaf_update->single[i].df_confidence= s_pdaf.single[i].df_confidence;
        pdaf_update->single[i].df_conf_level= s_pdaf.single[i].df_conf_level;
        pdaf_update->single[i].phase_diff = s_pdaf.single[i].phase_diff;
        SLOW("window %d defocus %d, df_confidence %d, df_conf_level %d, phase_diff %f",
           i,
           pdaf_update->single[i].defocus,
           pdaf_update->single[i].df_confidence,
           pdaf_update->single[i].df_conf_level,
           pdaf_update->single[i].phase_diff);
      }
    } else {
      memset(pdaf_update->single, 0 ,sizeof(pdaf_update->single));
    }
    pdaf_update->is_single_valid = s_pdaf.is_single_valid;
  }
  break;
  case MCT_EVENT_MODULE_STATS_PDAF_AF_WINDOW_UPDATE:
  {
    SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_SENSOR,
      SENSOR_PDAF_SET_AF_WINDOW, event_module->module_event_data, rc);
    if (rc < 0) {
      SERR("PDAF update AF window fails");
      ret = FALSE;
    }
    break;
  }
  case MCT_EVENT_MODULE_STATS_AF_UPDATE:
  case MCT_EVENT_MODULE_STATS_AF_MANUAL_UPDATE: {
    module_sensor_params_t *actuator_module_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_ACTUATOR];
    stats_update_t *stats_update =
      (stats_update_t *)event_module->module_event_data;
    if (!stats_update || !actuator_module_params->func_tbl.process) {
      SERR("failed");
      ret = FALSE;
      break;
    }

    if (stats_update->flag & STATS_UPDATE_AF) {
      sensor_af_bracket_t *af_bracket_params =
        &(bundle_info->s_bundle->af_bracket_params);
      if (af_bracket_params->enable) {
        SERR("failed to move lens, because focus bracketing is enabled");
        ret = FALSE;
        break;
      }

      if (stats_update->af_update.use_dac_value == TRUE) {
        rc = actuator_module_params->func_tbl.process(
          actuator_module_params->sub_module_private,
          ACTUATOR_SET_POSITION, &stats_update->af_update);
        if (rc < 0) {
          SERR("failed");
          ret = FALSE;
          break;
        }
      } else {
        rc = actuator_module_params->func_tbl.process(
          actuator_module_params->sub_module_private,
          ACTUATOR_MOVE_FOCUS, &stats_update->af_update);

        if (stats_update->af_update.move_lens) {
          bundle_info->s_bundle->cur_logical_lens_pos=
            stats_update->af_update.cur_logical_lens_pos;
        }

        if (rc < 0) {
          SERR("failed");
          ret = FALSE;
          break;
        }
      }

    }
    break;
  }
  case MCT_EVENT_MODULE_3A_GET_CUR_FPS: {
    module_sensor_params_t *module_sensor_params =
      bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
    if (module_sensor_params) {
      rc = module_sensor_params->func_tbl.process(
        module_sensor_params->sub_module_private,
        SENSOR_GET_CUR_FPS, event->u.module_event.module_event_data);
      if (rc < 0) {
          SERR("failed");
          ret = FALSE;
      }
    } else {
      SERR("failed");
      ret = FALSE;
    }
    break;
  }
  case MCT_EVENT_MODULE_QUERY_CURRENT: {
    /* Query current */
    SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_LED_FLASH,
      LED_FLASH_QUERY_CURRENT, event_module->module_event_data, rc);
    if (rc < 0) {
      SERR("failed");
      ret = FALSE;
    }
    break;
  }
  case MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK: {
    ret = module_sensor_handle_pixel_clk_change(module, event->identity,
      event->u.module_event.module_event_data);
    if (ret == FALSE) {
      SERR("MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK failed");
    }
    break;
  }
  case MCT_EVENT_MODULE_LED_STATE_TIMEOUT:
    SHIGH("Reset previously set LED state!");
    bundle_info->s_bundle->regular_led_trigger = 0;
    break;
  case  MCT_EVENT_MODULE_LED_AF_UPDATE:
    SHIGH("MCT_EVENT_MODULE_LED_AF_UPDATE");
    bundle_info->s_bundle->regular_led_af =
      *(int*)(event->u.module_event.module_event_data);
    break;
  case  MCT_EVENT_MODULE_ISP_FRAMESKIP:
    ret = module_sensor_handle_isp_frame_skip(port, bundle_info,
      event->identity, event->u.module_event.module_event_data);
      if (ret == FALSE) {
        SERR("MCT_EVENT_MODULE_ISP_FRAMESKIP failed");
      }
    break;
  case MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK:
    SHIGH("MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK");
    ret = module_sensor_handle_fast_aec_ack(module, event->identity);
    if (ret == FALSE) {
      SERR("MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK failed");
    }
    break;
  case MCT_EVENT_MODULE_SOF_NOTIFY:
    SLOW("MCT_EVENT_MODULE_SOF_NOTIFY");
    sensor_fast_aec_timeout_ack(module, event, bundle_info->s_bundle);
    break;
  case MCT_EVENT_MODULE_EZTUNE_GET_CHROMATIX:
  case MCT_EVENT_MODULE_EZTUNE_SET_CHROMATIX:
    ret = module_sensor_handle_eztune_chromatix_event(module, event,
      bundle_info->s_bundle);
    break;
  case MCT_EVENT_MODULE_EZTUNE_GET_AFTUNE:
  case MCT_EVENT_MODULE_EZTUNE_SET_AFTUNE:
    ret = module_sensor_handle_eztune_aftune_event(module, event,
      bundle_info->s_bundle);
    break;
  case MCT_EVENT_MODULE_DELAY_FRAME_SETTING:
    SLOW("MCT_EVENT_MODULE_DELAY_FRAME_SETTING");
    bundle_info->s_bundle->delay_frame_cnt =
      *((uint32_t *)event_module->module_event_data);
    break;
  case MCT_EVENT_MODULE_STATS_LED_CAL_CONFIG_DATA: {
    cam_capture_frame_config_t *config =
      (cam_capture_frame_config_t *)event_module->module_event_data;
    if (!config || !config->num_batch) {
      SERR("failed parm_data NULL");
      ret = FALSE;
      break;
    }
    memcpy(&bundle_info->s_bundle->hal_frame_batch, config,
      sizeof(cam_capture_frame_config_t));
    ret = module_sensor_fill_frame_timing(module, event, bundle_info);
    if (!ret)
      SERR("Failed to config dual led");

    bundle_info->s_bundle->cap_control.retry_frame_skip = 0;

    }
    break;
  default:
    break;
  }
  return ret;
}

/** port_sensor_handle_intramode_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *  @bundle_info: sensor bundle info
 *
 *  Handle intra session event
 *
 *  Return TRUE on success and FALSE on failure
 **/

static boolean port_sensor_handle_intramode_event(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;
  mct_event_module_t          *event_module;
  module_sensor_ctrl_t        *sensor_ctrl;
  module_sensor_bundle_info_t *s_bundle;
  int32_t                      rc = TRUE;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(port);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(bundle_info);

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
  s_bundle = bundle_info->s_bundle;

  event_module = &event->u.module_event;

  SLOW("[dual]current session %d", s_bundle->sensor_info->session_id);
  SLOW("[dual]event id %d", event_module->type);

  switch (event_module->type) {
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {

      ret = module_sensor_dualcamera_handle_peer_aec_update(module,
            event, bundle_info);
      if(!ret)
        SERR("module_sensor_dualcamera_handle_peer_aec_update failed");

      break;
  }
  case MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_TIME: {
    uint64_t* exp_time = (uint64_t *)event_module->module_event_data;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_EXPOSURE_TIME, exp_time, rc);
    break;
  }
  case MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_INFO: {
    sensor_dual_exp_t* dual_exp =
      (sensor_dual_exp_t*)event_module->module_event_data;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_EXPOSURE_INFO, dual_exp, rc);
    break;
  }
  case MCT_EVENT_MODULE_SENSOR_ADJUST_FPS: {
    float adjusted_frame_duration = *(float *)event_module->module_event_data;
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_SET_CUR_FLL, &adjusted_frame_duration, rc);
    break;
  }
  case MCT_EVENT_MODULE_SENSOR_GET_PEER_FORMAT:{
      sensor_output_format_t* output_format;
      output_format = (sensor_output_format_t *)event_module->module_event_data;

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_GET_SENSOR_FORMAT, output_format, rc);
      if (rc < 0) {
        SERR("[dual] get peer data format failed");
      }
      break;
  }
  case MCT_EVENT_MODULE_SENSOR_STREAM_ON: {
    mct_bus_msg_t bus_msg;
    module_sensor_params_t *module_sensor_params =
      s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

    SHIGH("[dual] stream_on for session %d", s_bundle->sensor_info->session_id);

    if (module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_START_STREAM, NULL) < 0) {

      SERR("[dual]failed");
      return FALSE;
    }

    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = s_bundle->sensor_info->session_id;
    bus_msg.type = MCT_BUS_MSG_SENSOR_STARTING;
    bus_msg.msg = &s_bundle->stream_thread_wait_time;
    mct_module_post_bus_msg(module, &bus_msg);
    break;
  }
  case MCT_EVENT_MODULE_SENSOR_GET_PEER_INFO: {
      peer_info_t* peer_info;

      peer_info = (peer_info_t *)event_module->module_event_data;
      peer_info->peer_timestamp = s_bundle->dualc_info.sof_timestamp;
      peer_info->peer_frameid = s_bundle->dualc_info.frame_idx;

      SLOW("MCT_EVENT_MODULE_SENSOR_GET_PEER_INFO for session %d, frame %d time %d",
        s_bundle->sensor_info->session_id,
        s_bundle->dualc_info.frame_idx,
        s_bundle->dualc_info.sof_timestamp);

      break;
  }
  case MCT_EVENT_MODULE_SENSOR_CORRECT_PHASE_DIFF: {
      sensor_dualc_info* peer_dual_info = NULL;
      peer_info_t peer_info;
      int32_t phase_diff = 0;

      /* Master session info */
      peer_dual_info = (sensor_dualc_info*)event_module->module_event_data;
      peer_info.peer_timestamp = peer_dual_info->sof_timestamp;
      peer_info.peer_frameid = peer_dual_info->frame_idx;

      SLOW("MCT_EVENT_MODULE_SENSOR_CORRECT_PHASE_DIFF session %d frame id %d ",
        s_bundle->sensor_info->session_id, s_bundle->dualc_info.frame_idx);

      module_sensor_dualcamera_calculate_phase_diff(module, event,
        s_bundle, peer_info);

      break;
  }
  default:
    SHIGH("[dual]invalid event");
    break;
  }

  return ret;
}

/** port_sensor_port_process_event: process event for sensor
 *  module
 *
 *  @port: mct port handle
 *  @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream **/

static boolean port_sensor_port_process_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  mct_module_t                *module = NULL;
  mct_event_module_t          *event_module = NULL;
  sensor_bundle_info_t         bundle_info;

  RETURN_ON_NULL(port);
  RETURN_ON_NULL(event);

  module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  RETURN_ON_NULL(module);

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  if (event->type == MCT_EVENT_MODULE_EVENT) {

    memset(&bundle_info, 0, sizeof(sensor_bundle_info_t));
    RETURN_ON_FALSE(sensor_util_get_sbundle(module, event->identity, &bundle_info));
    event_module = &event->u.module_event;

    if (event->direction == MCT_EVENT_DOWNSTREAM) {
      ret = mct_port_send_event_to_peer(port, event);
      if (ret == FALSE) {
        SERR("failed");
      }
    } else if (event->direction == MCT_EVENT_UPSTREAM) {
      ret = port_sensor_handle_upstream_module_event(module, port, event,
        &bundle_info);
      if (ret == FALSE) {
        SERR("failed: port_sensor_handle_upstream_module_event");
      }
    } else if (event->direction == MCT_EVENT_INTRA_MOD) {
      ret = port_sensor_handle_intramode_event(module, port, event,
        &bundle_info);
      if (ret == FALSE) {
        SERR("failed: port_sensor_handle_upstream_module_event");
      }
    } else {
      SERR("failed: invalid module event dir %d", event->direction);
      ret = FALSE;

    }
  } else if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = mct_port_send_event_to_peer(port, event);
    if (ret == FALSE) {
      SERR("failed peer_port %p", port);
    }
  }

  return ret;
}

/** port_sensor_create: create ports for sensor module
 *
 *  @data: module_sensor_bundle_info_t pointer
 *  @user_data: mct_module_t pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function creates one source port and fills capabilities
 *  for the created port, adds it the sensor module **/

boolean port_sensor_create(void *data, void *user_data)
{
  boolean                      ret = TRUE;
  int32_t                      rc = SENSOR_SUCCESS;
  mct_port_t                  *s_port = NULL;
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data;
  sensor_lib_params_t         *sensor_lib_params = NULL;
  mct_module_t                *s_module = (mct_module_t *)user_data;
  module_sensor_ctrl_t        *module_ctrl = NULL;
  int32_t                      i = 0, j = 0;
  char                         port_name[32];
  module_sensor_params_t      *module_sensor_params = NULL;
  module_sensor_params_t      *module_right_sensor_params = NULL;
  sensor_stream_info_array_t  *sensor_stream_info_array = NULL;
  sensor_stream_info_array_t  *right_sensor_stream_info_array = NULL;
  sensor_src_port_cap_t       *sensor_src_port_cap = NULL;
  module_sensor_params_t      *csid_module_params = NULL;
  uint32_t                     csid_version = 0;
  uint8_t                      num_meta_ch = 0;
  uint32_t                     pix_fmt_fourcc = 0;

  if (!s_bundle || !s_module) {
    SERR("failed data1 %p data2 %p", data, user_data);
    return FALSE;
  }
  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  csid_module_params = s_bundle->module_sensor_params[SUB_MODULE_CSID];
  if (!s_bundle->module_sensor_params[i]->func_tbl.open) {
    SERR("failed");
    return FALSE;
  }
  rc = csid_module_params->func_tbl.open(
    &csid_module_params->sub_module_private,
    &s_bundle->subdev_info[SUB_MODULE_CSID]);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  rc = csid_module_params->func_tbl.process(
    csid_module_params->sub_module_private,
    CSID_GET_VERSION, &csid_version);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  csid_module_params->func_tbl.close(
    csid_module_params->sub_module_private);

  module_ctrl = (module_sensor_ctrl_t *)s_module->module_private;
  sensor_lib_params = s_bundle->sensor_lib_params;
  rc = module_sensor_params->func_tbl.process(s_bundle->sensor_lib_params,
    SENSOR_GET_SENSOR_PORT_INFO, &sensor_stream_info_array);
  if (rc < 0) {
    SERR("failed %d", rc);
    goto ERROR1;
  }

  if (NULL != s_bundle->stereo_peer_s_bundle) {
    module_right_sensor_params =
      s_bundle->stereo_peer_s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

    rc = module_right_sensor_params->func_tbl.process(
      s_bundle->stereo_peer_s_bundle->sensor_lib_params,
      SENSOR_GET_SENSOR_PORT_INFO, &right_sensor_stream_info_array);
    if (rc < 0) {
      SERR("failed %d", rc);
      goto ERROR1;
    }
  }

  SLOW("sensor_stream_info_array->size %d", sensor_stream_info_array->size);
  for (j = 0; j < sensor_stream_info_array->size; j++) {
    if (SENSOR_CID_CH_MAX <
      sensor_stream_info_array->sensor_stream_info[j].vc_cfg_size) {
      SERR("vc_cfg_size out of range (%d) ",
        (int) sensor_stream_info_array->sensor_stream_info[j].vc_cfg_size);
      goto ERROR1;
    }
    snprintf(port_name, sizeof(port_name), "%s%d",
      s_bundle->sensor_info->sensor_name, j);
    s_port = mct_port_create(port_name);
    if (!s_port) {
      SERR("failed");
      goto ERROR1;
    }
    sensor_src_port_cap = malloc(sizeof(sensor_src_port_cap_t));
    if (!sensor_src_port_cap) {
      SERR("failed");
      goto ERROR2;
    }
    memset(sensor_src_port_cap, 0, sizeof(sensor_src_port_cap_t));

    sensor_src_port_cap->session_id = s_bundle->sensor_info->session_id;
    sensor_src_port_cap->num_cid_ch =
      sensor_stream_info_array->sensor_stream_info[j].vc_cfg_size;
    sensor_src_port_cap->is_stereo_config = s_bundle->is_stereo_configuration;
    for (i = 0; i < sensor_src_port_cap->num_cid_ch; i++) {
      sensor_src_port_cap->sensor_cid_ch[i].cid =
        sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i].cid;
      sensor_src_port_cap->sensor_cid_ch[i].csid =
          (uint32_t)s_bundle->sensor_info->subdev_id[SUB_MODULE_CSID];

      if (TRUE == sensor_src_port_cap->is_stereo_config &&
        NULL != s_bundle->stereo_peer_s_bundle) {
        sensor_src_port_cap->sensor_cid_ch[i].stereo_right_cid =
          right_sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i].cid;
        sensor_src_port_cap->sensor_cid_ch[i].stereo_right_csid =
          (uint32_t)s_bundle->stereo_peer_s_bundle->
            sensor_info->subdev_id[SUB_MODULE_CSID];
      } else {
        sensor_src_port_cap->sensor_cid_ch[i].stereo_right_cid =  MAX_CID;
        sensor_src_port_cap->sensor_cid_ch[i].stereo_right_csid = MAX_CSID;
      }

      sensor_src_port_cap->sensor_cid_ch[i].csid_version = csid_version;
      sensor_src_port_cap->sensor_cid_ch[i].dt =
        sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i].dt;

      sensor_src_port_cap->sensor_cid_ch[i].is_bayer_sensor =
        s_bundle->sensor_common_info.sensor_lib_params->sensor_lib_ptr->
        sensor_output.output_format == SENSOR_YCBCR ? 0 : 1;

      pix_fmt_fourcc = sensor_util_get_fourcc_format(
        sensor_stream_info_array->sensor_stream_info[j].pix_data_fmt[i],
        s_bundle->sensor_lib_params->sensor_lib_ptr->
          sensor_output.filter_arrangement,
        sensor_stream_info_array->
          sensor_stream_info[j].vc_cfg[i].decode_format);
      if (pix_fmt_fourcc == 0) {
        SERR("failed");
        goto ERROR2;
      }

      if ((pix_fmt_fourcc == MSM_V4L2_PIX_FMT_META) ||
        (pix_fmt_fourcc == MSM_V4L2_PIX_FMT_META10)){
        num_meta_ch = sensor_src_port_cap->num_meta_ch;

        sensor_src_port_cap->meta_ch[num_meta_ch].is_bayer_sensor = 0;
        sensor_src_port_cap->meta_ch[num_meta_ch].fmt =
          sensor_util_get_hal_format(pix_fmt_fourcc);
        sensor_src_port_cap->meta_ch[num_meta_ch].cid =
          sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i].cid;
        sensor_src_port_cap->meta_ch[num_meta_ch].csid =
          (uint32_t)s_bundle->sensor_info->subdev_id[SUB_MODULE_CSID];
        sensor_src_port_cap->meta_ch[num_meta_ch].dt =
          sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i].dt;
        if (sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i]
          .decode_format & CSI_RDI_PACKED) {
          sensor_src_port_cap->meta_ch[num_meta_ch].pack_mode
            = PACK_MODE_PLAIN_PACK;
        } else {
          sensor_src_port_cap->meta_ch[num_meta_ch].pack_mode
            = PACK_MODE_BYTE;
        }
        sensor_src_port_cap->num_meta_ch++;
      } else {
        sensor_src_port_cap->sensor_cid_ch[i].fmt =
          sensor_util_get_hal_format(pix_fmt_fourcc);
        if (sensor_stream_info_array->sensor_stream_info[j].vc_cfg[i]
              .decode_format == CSI_DECODE_10BIT_PLAIN16_LSB)
           sensor_src_port_cap->sensor_cid_ch[i].pack_mode =
               PACK_MODE_PLAIN_PACK;
        if (sensor_src_port_cap->sensor_cid_ch[i].fmt == CAM_FORMAT_MAX) {
          SERR("invalid format %d", sensor_src_port_cap->sensor_cid_ch[i].fmt);
        }
      }

    }
    s_port->direction = MCT_PORT_SRC;
    s_port->check_caps_reserve = port_sensor_caps_reserve;
    s_port->check_caps_unreserve = port_sensor_caps_unreserve;
    s_port->ext_link = port_sensor_ext_link_func;
    s_port->un_link = port_sensor_unlink_func;
    s_port->event_func = port_sensor_port_process_event;
    s_port->intra_event_func = port_sensor_port_process_event;

    SLOW("s_port=%p event_func=%p", s_port, s_port->event_func);
    s_port->caps.u.data = (void *)sensor_src_port_cap;
    s_port->port_private = (module_sensor_port_data_t *)
      malloc(sizeof(module_sensor_port_data_t));
    if (!s_port->port_private) {
      SERR("failed");
      goto ERROR2;
    }
    memset(s_port->port_private, 0, sizeof(module_sensor_port_data_t));
    ret = mct_module_add_port(s_module, s_port);
    if (ret == FALSE) {
      SERR("failed");
      goto ERROR2;
    }
  }
  SLOW("Exit");
  return TRUE;
ERROR2:
  mct_port_destroy(s_port);
ERROR1:
  SERR("failed");
  return FALSE;
}
