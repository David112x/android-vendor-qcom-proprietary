/* Copyright (c) 2015-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "module_sensor.h"
#include "module_sensor_offload.h"
#include "eebin_interface.h"
#include "live_connect.h"

/** module_sensor_offload_open
 *
 *  @param1: bundle information
 *  @param2: eebin handle
 *  @param3: None
 *  @param4: None
 *
 *  This function is used to  offload subdev configuration.
 *
 *  Return: nothing
 **/
void module_sensor_offload_open(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  module_sensor_bundle_info_t *s_bundle =
    (module_sensor_bundle_info_t *)param1;
  module_sensor_params_t  *module_sensor_params = NULL;
  int32_t  rc = SENSOR_SUCCESS, i = 0;

  for (i = 0; i < SUB_MODULE_MAX; i++) {
    SLOW("sensor_sd_name=%s", s_bundle->sensor_sd_name[i]);
    if (s_bundle->module_sensor_params[i]->func_tbl.open
       && s_bundle->subdev_info[i].sub_mod_open_flag) {
      rc = s_bundle->module_sensor_params[i]->func_tbl.open(
        &s_bundle->module_sensor_params[i]->sub_module_private,
        &s_bundle->subdev_info[i]);
      if (rc < 0) {
        SERR("failed rc %d", rc);
        break;
      }
    }
  }

  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  if (rc < 0)
    s_bundle->open_done = -1;
  else
    s_bundle->open_done = 1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
}

/** module_sensor_offload_close
 *
 *  @param1: bundle information
 *  @param2: None
 *  @param3: None
 *  @param4: None
 *
 *  This function is used to  offload subdev close.
 *
 *  Return: nothing
 **/
void module_sensor_offload_close(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  module_sensor_bundle_info_t  *s_bundle =
                      (module_sensor_bundle_info_t *)param1;
  module_sensor_params_t       *module_sensor_params = NULL;
  int32_t                      rc = SENSOR_SUCCESS, i = 0;

  for (i = SUB_MODULE_MAX - 1; i >= 0; i--) {
    if (s_bundle->module_sensor_params[i]->func_tbl.close) {
      s_bundle->module_sensor_params[i]->func_tbl.close(
        s_bundle->module_sensor_params[i]->sub_module_private);
    }
    s_bundle->module_sensor_params[i]->sub_module_private = NULL;
  }

  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->close_done = 1;
  s_bundle->close_pending = 0;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
}

/** module_sensor_offload_init_config
 *
 *  @param1: bundle information
 *  @param2: eebin handle
 *  @param3: None
 *  @param4: None
 *
 *  This function is used to  offload subdev configuration.
 *
 *  Return: nothing
 **/
void module_sensor_offload_init_config(
  void* param1,
  void* param2,
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  module_sensor_bundle_info_t *s_bundle =
  (module_sensor_bundle_info_t *)param1;
  void *eebin_hdl = param2;
  int32_t  rc = SENSOR_SUCCESS, i = 0, j = 0, k = 0;
  module_sensor_params_t  *module_sensor_params = NULL;
  module_sensor_params_t  *eeprom_module_params = NULL;
  sensor_get_t             sensor_get;
  sensor_property_t        sensor_property;
  eebin_ctl_t              bin_ctl;
  enum sensor_sub_module_t s_module;
  af_algo_tune_parms_t    *af_algo_cam_ptr = NULL;
  af_algo_tune_parms_t    *af_algo_camcorder_ptr = NULL;
  actuator_driver_params_t *af_driver_ptr = NULL;
  boolean                  status = TRUE;
  boolean                  live_connect_enabled = FALSE;
  char                     prop[PROPERTY_VALUE_MAX];

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

  /* set mount angle, position and modes_supported */
  if (s_bundle->sensor_info->is_mount_angle_valid == 1)
    if (s_bundle->sensor_common_info.camera_config.sensor_mount_angle >=
      SENSOR_MOUNTANGLE_360)
      s_bundle->sensor_common_info.camera_config.sensor_mount_angle =
        s_bundle->sensor_info->sensor_mount_angle;

  switch(s_bundle->sensor_info->position) {
    case BACK_CAMERA_B:
      s_bundle->sensor_common_info.camera_config.position =
        CAM_POSITION_BACK;
      break;
    case FRONT_CAMERA_B:
      s_bundle->sensor_common_info.camera_config.position =
        CAM_POSITION_FRONT;
      break;
    case AUX_CAMERA_B:
      s_bundle->sensor_common_info.camera_config.position =
        CAM_POSITION_BACK_AUX;
      break;
    default:
      SERR("invalid position = %d", s_bundle->sensor_info->position);
      break;
  }

  if (s_bundle->sensor_info->modes_supported != CAMERA_MODE_INVALID)
    s_bundle->sensor_common_info.camera_config.modes_supported =
      s_bundle->sensor_info->modes_supported;

  s_bundle->sensor_params.aperture_value = 0;
  memset(&sensor_property, 0, sizeof(sensor_property));
  /* get lens info to sensor sub module */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_PROPERTY, &sensor_property);
  if (rc < 0) {
    SERR("failed:SENSOR_GET_PROPERTY rc: %d", rc);
  } else {
    /* Fill aperture */
    s_bundle->sensor_params.aperture_value =
      s_bundle->sensor_common_info.camera_config.lens_info.f_number;
    s_bundle->sensor_params.f_number =
      s_bundle->sensor_common_info.camera_config.lens_info.f_number;
    s_bundle->sensor_params.focal_length =
      s_bundle->sensor_common_info.camera_config.lens_info.focal_length;
    s_bundle->sensor_params.sensing_method =
      sensor_property.sensing_method;
    s_bundle->sensor_params.crop_factor =
      sensor_property.crop_factor;
    SLOW("aperture %f", s_bundle->sensor_params.aperture_value);
  }

  /* af_actuator_init */
  /* get actuator header */
  /* sensor init */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_INIT, NULL);
  if (rc < 0) {
    SERR("failed: SENSOR_INIT rc:%d", rc);
    goto ERROR0;
  }

  /* Set alternative slave address if needed */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_SET_ALTERNATIVE_SLAVE,
    &s_bundle->sensor_common_info.camera_config.sensor_slave_addr);
  if (rc < 0) {
    SERR("failed: SENSOR_SET_ALTERNATIVE_SLAVE rc:%d", rc);
    goto ERROR0;
  }

  /*Disabling fast aec by default. Will be enabled only based on
    HAL input */
  s_bundle->fast_aec_required = FALSE;

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
    struct msm_camera_i2c_reg_setting cal_setting;
    int32_t                           is_insensor = 0;

    /* Check if eeprom has cal data for sensor module. */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
      EEPROM_GET_ISINSENSOR_CALIB, &is_insensor, rc);
    if (rc < 0) {
      SERR("failed:EEPROM_GET_ISINSENSOR_CALIB rc %d", rc);
      goto ERROR0;
    }

    /* If yes, apply cal data to the sensor module. */
    if (is_insensor) {
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_RAW_DATA, &cal_setting, rc);
      if (rc < 0) {
        SERR("failed:EEPROM_GET_RAW_DATA rc %d", rc);
        goto ERROR0;
      }
      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_CALIBRATION_DATA, &cal_setting, rc);
      if (rc < 0) {
        SERR("failed:SENSOR_SET_CALIBRATION_DATA rc %d", rc);
        goto ERROR0;
      }
    }
  }

  SLOW("flash subdev id = %d",
  s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH]);
  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_LED_FLASH] != -1) {
    status = module_sensor_flash_init(s_bundle);
    if (status != TRUE) {
      SERR("sensor_failure : module_sensor_flash_init failed");
      goto ERROR0;
    }
  }

  /* Get led frame skip timing parameters */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_LED_FLASH,
    LED_FLASH_GET_FRAME_SKIP_TIME_PARAMS, s_bundle, rc);
  if (rc < 0) {
    SERR("failed to get led off frame skip time");
    s_bundle->main_flash_on_frame_skip = 2;
    s_bundle->main_flash_off_frame_skip = 2;
    s_bundle->torch_on_frame_skip = 2;
    s_bundle->torch_off_frame_skip = 2;
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_ACTUATOR] != -1) {
    status = module_sensor_actuator_init_calibrate(s_bundle);
    if (status != TRUE) {
      SERR("module_sensor_actuator_init_calibrate failed");
      goto ERROR0;
    }
  }

  /* Load external libraries */
  if (s_bundle->sensor_lib_params->sensor_lib_ptr->external_library) {
    status = module_sensor_load_external_libs(s_bundle);
    if (status == FALSE) {
      SERR("failed: module_sensor_load_external_libs");
      goto ERROR0;
    }
  }

  /* Get fast AEC capabilities */
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_FAST_AEC_WAIT_FRAMES, &s_bundle->sensor_num_fast_aec_frame_skip);
  if (rc < 0) {
    SERR("failed rc %d", rc);
    goto ERROR0;
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EXT] != -1) {
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EXT,
      EXT_GET_INFO, &s_bundle->ext_pipeline_delay, rc);
  }

  /* Start live connect thread if property is set */
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.liveconnect", prop, "0");
  live_connect_enabled = atoi(prop);
  if (live_connect_enabled > 0 ) {
    /* Create sensor live connect thread */
    status = sensor_live_connect_thread_create(s_bundle);
    if(status == FALSE) {
      SERR("sensor liveconnect thread creation failed");
      goto ERROR0;
    }
    s_bundle->live_connect_thread_started = TRUE;
  }

  SHIGH("Success");
  /*end*/
  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->init_config_done = 1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
  return;

ERROR0:
  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->init_config_done = -1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);

  SERR("failed");
  return;
}

/** module_sensor_offload_ois_init_calibrate
 *
 *  @param1: bundle information
 *  @param2: None
 *  @param3: None
 *  @param4: None
 *
 *  This function is used to  offload OIS initialization and calibration to sensor thread.
 *
 *  Return: nothing
 **/
void module_sensor_offload_ois_init_calibrate(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  int32_t rc = SENSOR_SUCCESS;
  module_sensor_bundle_info_t* s_bundle = NULL;
  uint8_t *value = NULL;

  SLOW("Enter");
  if (param1) {
    s_bundle = (module_sensor_bundle_info_t*)param1;
  } else {
    SERR("Null pointer detected");
    return;
  }

  if (s_bundle->sensor_info->subdev_id[SUB_MODULE_OIS] != -1) {
    /*Get OIS name from camera config */
    char *ois_name = s_bundle->sensor_common_info.camera_config.ois_name;
    /* Init OIS */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
      OIS_INIT,ois_name, rc);
    if (rc < 0) {
      SERR("ois_init failed rc %d", rc);
      return;
    }
    /*enable OIS init done flag*/
    s_bundle->ois_cmd_queue_mask |= OIS_INIT_DONE_MASK;

    if (s_bundle->sensor_info->subdev_id[SUB_MODULE_EEPROM] != -1) {
      struct msm_camera_i2c_seq_reg_setting ois_cal_setting;
      int32_t is_ois = 0;

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
        EEPROM_GET_ISOIS_CALIB, &is_ois, rc);
      if (rc < 0) {
        SERR("Get Is OIS calibration failed");
        return;
      }
      if (is_ois) {
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_EEPROM,
          EEPROM_GET_OIS_RAW_DATA, &ois_cal_setting, rc);
        if (rc < 0) {
          SERR("Get OIS raw data failed");
          return;
        }
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
          OIS_CALIBRATION, &ois_cal_setting, rc);
        if (rc < 0) {
          SERR("Set OIS calibration data failed");
          return;
        }
      } else {
        SHIGH("OIS Not available/enabled");
      }
    }

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
      OIS_CENTERING_ON, NULL, rc);
    if (rc < 0) {
      SERR("OIS centering failed");
      return;
    }

    /*verify if ois enable is cached and needes to be executed or not.*/
    if ((s_bundle->ois_cmd_queue_mask &
          OIS_ENABLE_CALLED_MASK) == OIS_ENABLE_CALLED_MASK) {
      SLOW("OIS enable cached so scheduling enable");
      value = (uint8_t *)CAM_CALLOC(1, sizeof(uint8_t));
      if (!value) {
        SERR("failed");
        return;
      }
      if(s_bundle->ois_cmd_queue_mask & OIS_ENABLE_VALUE_MASK) {
        *value = 1;
      }
      else {
        *value = 0;
      }
      module_sensor_offload_lens_opt_stab_mode((void*)value, NULL, NULL,
                                               (void*)s_bundle);
    }
  }

  SLOW("Exit");
}

/** module_sensor_offload_proc_led: Post message to thread
 *
 *  @param1: led_module_params
 *  @param2: flash mode
 *
 *  This function is used to process the led events to sensor thread
 *
 *  Return: TRUE or FALSE
 **/

void module_sensor_offload_proc_led(void* param1, void* param2,
  void* param3, void* param4)
{
  module_sensor_params_t *led_module_params = NULL;
  int32_t ret = 0;
  uint8_t post_msg = 0;
  mct_module_t *module = (mct_module_t *)param1;
  uint32_t identity = *(uint32_t *)param2;
  module_sensor_bundle_info_t* s_bundle =
    (module_sensor_bundle_info_t *)param3;
  sensor_batch_config_t *bctrl = NULL;
  int8_t idx = 0;
  if (!s_bundle)
    return;
  idx = s_bundle->batch_idx;
  bctrl = (idx >= 0) ? &(s_bundle->cap_control.bctrl[idx]):NULL;

  if (module && bctrl) {
    /* Set led mode */;
    led_module_params = s_bundle->module_sensor_params[SUB_MODULE_LED_FLASH];
    SHIGH("falsh_mode: %d",bctrl->flash_mode);

    if (bctrl->flash_mode < LED_FLASH_ENUM_MIN ||
        bctrl->flash_mode > LED_FLASH_ENUM_MAX) {
      SERR("Invalid flash mode");
      bctrl->led_toggle_pending = FALSE;
      return;
    }

  if (led_module_params && led_module_params->func_tbl.process != NULL) {
       ret = led_module_params->func_tbl.process(
                led_module_params->sub_module_private,
                bctrl->flash_mode, param4);
       if (ret < 0) {
         SERR("failed: LED_FLASH_MODE_SET failed");
       }

       if (bctrl->skip_config_mode || s_bundle->close_done) {
         SLOW("off load completed");
         return;
       }

       PTHREAD_MUTEX_LOCK(&s_bundle->capture_control_mutex);
       if (bctrl->flash_mode == LED_FLASH_SET_PRE_FLASH && !ret) {
         s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_TORCH;
         s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_FIRED;
       } else if ((bctrl->flash_mode == LED_FLASH_SET_OFF ||
         bctrl->flash_mode == LED_FLASH_SET_RER_PROCESS) && !ret) {
         s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
         s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
       } else if (bctrl->flash_mode == LED_FLASH_SET_MAIN_FLASH) {
         if (!ret) {
           s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_ON;
           s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_FIRED;
         } else {
           s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
           s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
         }
       } else if (bctrl->flash_mode == LED_FLASH_SET_TORCH) {
         if (!ret) {
           s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_TORCH;
           s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_FIRED;
         } else {
           s_bundle->sensor_params.flash_mode = CAM_FLASH_MODE_OFF;
           s_bundle->sensor_params.flash_state = CAM_FLASH_STATE_READY;
         }
       }
       bctrl->led_toggle_pending = FALSE;
       PTHREAD_MUTEX_UNLOCK(&s_bundle->capture_control_mutex);
       SLOW("off load completed");
    }
  } else {
    SERR("Null arguments");
    if (bctrl)
      bctrl->led_toggle_pending = FALSE;
    return;
  }
  if (post_msg)
    sensor_util_post_led_state_msg(module, s_bundle, identity);
}

/** module_sensor_offload_lens_reset: Reset lens
 *
 *  @param1: actuator module params
 *  @param2: not used
 *  @param3: not used
 *  @param4: if not null is pointer to mutex for signaling end of lens movement
 *
 *  This function is used to update reset the lens
 *
 *  NOTE:
 *  It is supposed to be execured in sensor thread,
 *  not to block thread that sends SOF
 *
 *  Return: nothing
 **/
void module_sensor_offload_lens_reset(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  af_update_t            af_update;
  module_sensor_params_t *actuator_module_params = NULL;
  sensor_af_bracket_t    *bracket = NULL;
  int32_t ret = -1;

  if (param1 && param3) {
    actuator_module_params = param1;
    bracket = (sensor_af_bracket_t *)param3;
    /* Move Lens in Actuator */
    af_update.reset_lens = TRUE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    if (ret < 0) {
      SERR("Fail to move lens");
    }

    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);

  } else {
    SERR("Null pointer detected");
  }
}

/** module_sensor_offload_lens_move: Move lens
 *
 *  @param1: actuator module params
 *  @param2: direction
 *  @param3: number of steps
 *  @param4: if not null is pointer to mutex for signaling end of lens movement
 *
 *  This function is used to update move the lens in specified direction
 *  for specified number of steps
 *
 *  NOTE:
 *  It is supposed to be execured in sensor thread,
 *  not to block thread that sends SOF
 *
 *  Return: nothing
 **/
void module_sensor_offload_lens_move(void* param1, void* param2,
  void* param3, void* param4)
{
  af_update_t            af_update;
  module_sensor_params_t *actuator_module_params = NULL;;
  sensor_af_bracket_t    *bracket = NULL;
  int32_t ret = -1;

  if (param1 && param4) {
    actuator_module_params = param1;
    bracket = (sensor_af_bracket_t *)param4;

    /* Move Lens in Actuator */
    af_update.move_lens    = TRUE;
    af_update.direction    = (int)param2;
    af_update.num_of_steps = (int32_t)param3;
    af_update.reset_lens   = FALSE;
    af_update.reset_pos    = FALSE;
    af_update.stop_af      = FALSE;

    if (actuator_module_params->func_tbl.process != NULL) {
      ret = actuator_module_params->func_tbl.process(
        actuator_module_params->sub_module_private,
        ACTUATOR_MOVE_FOCUS, &af_update);
    }

    if (ret < 0) {
      SERR("Fail to move lens");
    }

    pthread_mutex_lock(&bracket->lens_move_done_sig);
    bracket->lens_move_progress = FALSE;
    pthread_mutex_unlock(&bracket->lens_move_done_sig);
  } else {
    SERR("Null pointer detected");
  }
}

/** module_sensor_offload_lens_opt_stab_mode: enable/disable OIS
 *
 *  @param1: event_control->parm_data
 *  @param2: not used
 *  @param3: not used
 *  @param4: s_bundle
 *
 *  This function is used to enable or disable OIS
 *
 *  NOTE:
 *  It is supposed to be execured in sensor thread,
 *  not to block thread that sends SOF
 *
 *  Return: nothing
 **/

void module_sensor_offload_lens_opt_stab_mode(void* param1,
  void* param2 __attribute__((unused)), void* param3 __attribute__((unused)),
  void* param4)
{
  module_sensor_bundle_info_t *s_bundle = NULL;
  uint8_t *value = NULL;
  boolean ret = TRUE;
  int32_t rc = SENSOR_SUCCESS;

  SLOW("Enter");

  if (param1 && param4) {
    value = (uint8_t *)param1;
    s_bundle = (module_sensor_bundle_info_t *)param4;

    /*OIS enable/disable should be executed after ois init is done*/
    if ((s_bundle->ois_cmd_queue_mask & OIS_INIT_DONE_MASK) == 0) {
      if (*value == 1) {
        /*set enable OIS flag to 1*/
        s_bundle->ois_cmd_queue_mask |= ( OIS_ENABLE_CALLED_MASK
                                          | OIS_ENABLE_VALUE_MASK);
      }
      else{
        /*set enable OIS flag to 0*/
        s_bundle->ois_cmd_queue_mask |= OIS_ENABLE_CALLED_MASK;
        /*Clear the Enable value flag indicating OIS disable called */
        s_bundle->ois_cmd_queue_mask &= ~(OIS_ENABLE_VALUE_MASK);
      }
      SLOW("OIS init not done so caching OIS enable");
      free(value);
      return;
    }
    if (*value == 1) {
      if(s_bundle->ois_mode == OIS_DISABLE) {
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
          OIS_PANTILT_ON, NULL, rc);
        if (rc < 0) {
          SERR("OIS enable failed");
          ret = FALSE;
        }
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
          OIS_ENABLE, NULL, rc);
        if (rc < 0) {
          SERR("OIS enable failed");
          ret = FALSE;
        }
      }
      if ((s_bundle->hal_version == CAM_HAL_V1 &&
        (s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO))) ||
        ((s_bundle->hal_version == CAM_HAL_V3) &&
        (s_bundle->capture_intent == CAM_INTENT_VIDEO_RECORD ||
        s_bundle->capture_intent == CAM_INTENT_VIDEO_SNAPSHOT))) {
        if(s_bundle->ois_mode != OIS_MOVIE_MODE) {
          SLOW("turning on OIS movie mode");
          SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
            OIS_MOVIE_MODE, NULL, rc);
          s_bundle->ois_mode = OIS_MOVIE_MODE;
        }
      } else {
        if(s_bundle->ois_mode != OIS_STILL_MODE) {
          SLOW("turning on OIS still mode");
          SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
            OIS_STILL_MODE, NULL, rc);
          s_bundle->ois_mode = OIS_STILL_MODE;
        }
      }
      if (rc < 0) {
        SERR("OIS enable failed");
        s_bundle->ois_mode = OIS_DISABLE;
        ret = FALSE;
      }
      s_bundle->ois_enabled = 1;
      SHIGH("OIS enabled");
    } else {
      if(s_bundle->ois_mode != OIS_DISABLE) {
        SLOW("turning off OIS");
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_OIS,
          OIS_DISABLE, NULL, rc);
        if (rc < 0) {
          SERR("OIS disable failed");
          ret = FALSE;
        }
        s_bundle->ois_mode = OIS_DISABLE;
       SHIGH("OIS disabled");
      }
      s_bundle->ois_enabled = 0;
    }
  }
  free(value);
}

void module_sensor_offload_cfg_resolution(void* param1,
  void* param2,
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  int32_t rc = SENSOR_SUCCESS;
  module_sensor_bundle_info_t *s_bundle =
   (module_sensor_bundle_info_t *)param1;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)param2;
  int32_t res;
  sensor_lib_params_t *lib = NULL;
  struct camera_i2c_reg_setting *reg_settings = NULL;
  struct msm_camera_i2c_reg_array *regs = NULL;
  struct sensor_lib_out_info_t *dimension = NULL;
  struct sensorb_cfg_data cfg;
  JUMP_ON_NULL(s_bundle, ERROR);
  JUMP_ON_NULL(ctrl, ERROR);
  res = ctrl->s_data->cur_res;
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  JUMP_ON_NULL(lib, ERROR);

  dimension = &lib->sensor_lib_ptr->out_info_array.out_info[res];
  SLOW("res x %d y %d llpclk %d fl %d",
    dimension->x_output, dimension->y_output, dimension->line_length_pclk,
    dimension->frame_length_lines);

  /* Program the settings */
  rc = sensor_write_i2c_init_res_setting_array(ctrl,
    &lib->sensor_lib_ptr->res_settings_array.reg_settings[res]);
  if (rc < 0) {
    SERR("sensor_write_i2c_array failed");
    goto ERROR;
  }

  if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_YCBCR) {
    if (!sensor_write_aec_settings(ctrl, s_bundle->stream_mask)) {
      SERR("sensor_write_aec_settings failed");
      goto ERROR;
    }
  }
  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->res_cfg_done = 1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
  return;

ERROR:
  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->res_cfg_done = -1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
}

/** module_sensor_offload_actuator: offload actuator config
 *
 *  @param1: s_bundle
 *  @param2: not used
 *  @param3: not used
 *  @param4: not used
 *
 *  This function is used to offload actuator config
 *
 *  Return: nothing
 **/

void module_sensor_offload_actuator(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)))
{
  module_sensor_bundle_info_t *s_bundle =
    (module_sensor_bundle_info_t *)param1;
  int32_t rc = 0;

  JUMP_ON_NULL(s_bundle, ERROR);
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_ACTUATOR,
    ACTUATOR_SET_PARAMETERS, NULL, rc);
  if (rc < 0) {
    SERR("ACTUATOR_SET_PARAMETERS failed");
    goto ERROR;
  }

  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->actuator_cfg_done = 1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
  SHIGH("Exit: actuator_cfg_done: %d",s_bundle->actuator_cfg_done);
  return;
ERROR:
  PTHREAD_MUTEX_LOCK(&s_bundle->mutex);
  s_bundle->actuator_cfg_done = -1;
  pthread_cond_signal(&s_bundle->cond);
  PTHREAD_MUTEX_UNLOCK(&s_bundle->mutex);
}
