/* awb_biz.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "awb_biz.h"
#include "stats_chromatix_wrapper.h"

/** Array to map the Q3A decision to biz decision */
static const uint32 decision_map_q3a_to_biz[AWB_ILLUMINANT_LAST] =
{
  Q3A_AWB_AGW_D75,
  Q3A_AWB_AGW_D65,
  Q3A_AWB_AGW_NOON,
  Q3A_AWB_AGW_D50,

  Q3A_AWB_AGW_COLD_FLO,
  Q3A_AWB_AGW_WARM_FLO,
  Q3A_AWB_AGW_A,
  Q3A_AWB_AGW_HORIZON,

  Q3A_AWB_AGW_CUSTOM_FLO,
  Q3A_AWB_AGW_CUSTOM_DAYLIGHT,
  Q3A_AWB_AGW_CUSTOM_A,
  Q3A_AWB_AGW_U30,

  Q3A_AWB_AGW_CUSTOM_DAYLIGHT1,
  Q3A_AWB_AGW_CUSTOM_DAYLIGHT2,
  Q3A_AWB_AGW_CUSTOM_FLO1,
  Q3A_AWB_AGW_CUSTOM_FLO2
};

/* Supporting function prototypes */
static boolean awb_biz_estimate_ccm(void *obj, float cct,
  awb_ccm_type *ccm, uint8_t camera_id);
static void awb_biz_reset_flash_info(awb_biz_t *awb);
static boolean awb_biz_is_longshot_flash(awb_biz_t *awb);
static void awb_biz_handle_manual_and_lock(stats_t *stats, void *obj);
static void awb_biz_copy_dual_led_setting(LED_setting_type *src, awb_led_combination_type *dst)
{
  dst->LED1_setting = src->LED1_setting;
  dst->LED2_setting = src->LED2_setting;
  dst->bg_ratio = src->bg_ratio;
  dst->rg_ratio = src->rg_ratio;
  dst->flux = src->flux;
  dst->first_entry_ratio = src->first_entry_ratio;
  dst->last_entry_ratio = src->last_entry_ratio;
}

inline static chromatix_awb_all_light_adapter map_decision_from_q3a_to_biz(
  const awb_illuminant_type decision)
{
  chromatix_awb_all_light_adapter mapped_decision = Q3A_AWB_AGW_INVALID_LIGHT;

  if (decision < AWB_ILLUMINANT_LAST)
  {
    mapped_decision =
      (chromatix_awb_all_light_adapter)decision_map_q3a_to_biz[decision];
  }

  return mapped_decision;
}

/**
 * awb_biz_load_dual_led_calibration
 *
 * Loads the dual LED calibration data from file (if it exists), and reads
 * props indicating whether dual LED calibration feature is enabled or not.
 * Reference implementation only; the OEM should modify or rewrite this.
 *
 * @param pCalib
 *
 */
static void awb_biz_load_dual_led_calibration(awb_dual_led_calibaration* pCalib) {
  int i = 0;
  bool feature_enabled = (boolean)property_get_bool(DUAL_LED_CALIB_FEATURE_ENABLED_PROP, 0);
  AWB_HIGH("Dual LED calibration feature is %s", feature_enabled ? "enabled" : "disabled");

  if (!pCalib) return;
  /* Reset the  calibration data */
  STATS_MEMSET(pCalib->data, 0,
         sizeof(awb_dual_led_calibration_point) * AWB_DUAL_LED_CALIB_ON_STATE_COUNT);
  if (!feature_enabled) return;

  /* Try to load the calibration data from file */
  FILE* file = fopen(DUAL_LED_CALIB_FILE_PATH, "rb");
  if (file){
    AWB_HIGH("Found dual LED calibration data file. Loading...");
    /* Read calibration information for each possible LED-ON state */
    for (i=0; i < AWB_DUAL_LED_CALIB_ON_STATE_COUNT; i++){
      /* Read the r/g and b/g ratios from file */
      if (fread(&pCalib->data[i].rg_ratio,
                sizeof(float),
                1,
                file) &&
          fread(&pCalib->data[i].bg_ratio,
                sizeof(float),
                1,
                file))
      {
        pCalib->data[i].is_valid = true;
        AWB_LOW("Loaded LED calibration data[%d]: %f, %f", i, pCalib->data[i].rg_ratio, pCalib->data[i].bg_ratio);
      } else {
        AWB_ERR("Error reading from the dual LED calibration file!");
        break;
      }
    }

    fclose(file);
  }
  else
  {
    AWB_HIGH("Could not find the dual LED calibration data file.");
  }
}

/**
 * awb_biz_save_dual_led_calibration
 *
 * Saves the Dual LED calibration data to file - if calibration is performed
 * and valid calibration data is acquired. This is a reference implementation
 * only and OEMs should rewrite or modify this.
 *
 * @param pCalib
 *
 */
static void awb_biz_save_dual_led_calibration(awb_dual_led_calibaration* pCalib){
  int i = 0;
  static bool saved_tofile = false;

  /* Only save calibration data if calibration mode is enabled
   * and the data is not already saved to file */
  if (!pCalib || !pCalib->is_calibration_mode || saved_tofile) {
    return;
  }

  /* Only save the calibration data if it's valid */
  for (i = 0; i < AWB_DUAL_LED_CALIB_ON_STATE_COUNT; i++) {
    if (!pCalib->data[i].is_valid){
      return;
    }
  }

  FILE* file = fopen(DUAL_LED_CALIB_FILE_PATH, "w");
  if (file){
    AWB_HIGH("Saving the dual LED calibration data file...");
    saved_tofile = true;
    for (i = 0; i < AWB_DUAL_LED_CALIB_ON_STATE_COUNT; i++){
      /* Write the r/g and b/g ratios to file */
      if (!fwrite(&pCalib->data[i].rg_ratio,
                  sizeof(float),
                  1,
                  file) ||
          !fwrite(&pCalib->data[i].bg_ratio,
                  sizeof(float),
                  1,
                  file))
      {
        AWB_ERR("Error! Failed to write to the dual LED calibration file!");
        saved_tofile = false;
        break;
      }
    }

    fclose(file);
  } else {
    AWB_ERR("Could not open to the dual LED calibration data file to save the data.");
  }
}

/**
 * awb_biz_set_chromatix
 *
 * @param awb
 * @param init_param
 *
 * @return boolean
 */
static boolean awb_biz_set_init(awb_biz_t *awb,
  awb_set_parameter_init_t* init_param, uint8_t camera_id)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  chromatix_3a_parms_wrapper *chromatix_wrapper;
  awb_algo_tuning_adapter *chromatix;
  awb_reserved_tuning_params_type reserved_params;
  awb_set_param_info_type set_param;
  awb_dual_led_calibaration dual_led_calibration;
  STATS_MEMSET(&dual_led_calibration, 0, sizeof(dual_led_calibration));

  if (!awb || !init_param)
    return FALSE;

  chromatix_wrapper = (chromatix_3a_parms_wrapper *)init_param->chromatix;
  chromatix = &chromatix_wrapper->AWB;
  if (!chromatix)
    return FALSE;

  reserved_params.white_current_decision_weight = default_white_current_decision_weight;
  reserved_params.a_h_line_boundary = default_a_h_line_boundary;
  reserved_params.day_f_line_boundary = default_day_f_line_boundary;
  reserved_params.d75_d65_line_boundary = default_d75_d65_line_boundary;
  reserved_params.d65_d50_line_boundary = default_d65_d50_line_boundary;
  reserved_params.d65_d50_shifted_line_boundary = default_d65_d50_shifted_line_boundary;
  reserved_params.f_a_line_boundary = default_f_a_line_boundary;
  reserved_params.cw_tl84_line_boundary = default_cw_tl84_line_boundary;
  reserved_params.d2cw_enable = default_d2cw_enable;
  reserved_params.A_H_warmup_enable = default_A_H_warmup_enable;
  reserved_params.dual_led_intersect_slope = default_dual_led_intersect_slope;
  reserved_params.led1_r_adj = default_led1_r_adj;
  reserved_params.led1_b_adj = default_led1_b_adj;
  reserved_params.led2_r_adj = default_led2_r_adj;
  reserved_params.led2_b_adj = default_led2_b_adj;
  reserved_params.face_enable = default_face_supported;

  /* Dual LED calibration data is needed to adjust tuned LED parameters; load this now */
  if (!awb->dual_led_calib_mode_enabled)
    awb_biz_load_dual_led_calibration(&dual_led_calibration);
  dual_led_calibration.is_calibration_mode = awb->dual_led_calib_mode_enabled;
  dual_led_calibration.state = AWB_DUAL_LED_CALIB_OFF;

  set_param.param_type = AWB_SET_PARAM_TUNING_DATA;
  set_param.u.tuning_params.reload_needed = FALSE;
  if (awb->op_mode == Q3A_OPERATION_MODE_CAMCORDER ||
    (Q3A_OPERATION_MODE_PREVIEW == awb->op_mode && Q3A_OPERATION_MODE_CAMCORDER == awb->previous_op_mode)) {
    set_param.u.tuning_params.reload_needed = TRUE;
  }

  pthread_rwlock_rdlock(&chromatix_wrapper->lock);
  set_param.u.tuning_params.dual_led_calib = &dual_led_calibration;
  set_param.u.tuning_params.tuning_params = chromatix;
  set_param.u.tuning_params.reserved_tuning_param = &reserved_params;
  set_param.u.tuning_params.initial_white_balance = &chromatix->awb_MWB.MWB_d50;
  set_param.u.tuning_params.led_flash_white_balance = &chromatix->awb_MWB.led_flash_white_balance;
  set_param.u.tuning_params.stored_params = init_param->stored_params;

  awb->awb_full_camif_enable = chromatix->awb_flow_control.awb_full_camif_enable;

  awb->chromatix_tl84_white_balance = chromatix->awb_MWB.MWB_tl84;
  awb->chromatix_d50_white_balance = chromatix->awb_MWB.MWB_d50;
  awb->chromatix_d65_white_balance = chromatix->awb_MWB.MWB_d65;
  awb->chromatix_incandescent_white_balance = chromatix->awb_MWB.MWB_A;
  awb->gain_adj = chromatix->awb_basic_tuning.awb_adjust_gain_table.gain_adj_normallight;
  awb->snow_blue_gain_adj_ratio = chromatix->awb_blue_tuning.snow_blue_gain_adj_ratio;
  awb->beach_blue_gain_adj_ratio = chromatix->awb_blue_tuning.beach_blue_gain_adj_ratio;
  awb->awb_ccm_enable = chromatix->awb_CCM_control.awb_ccm_enable;
  awb->dual_LED_enable = chromatix->awb_led_tuning.mix_LED_table.dual_led_enable;

  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  pthread_rwlock_unlock(&chromatix_wrapper->lock);
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AWB_ERR("awb set param failed");
    return FALSE;
  }

  if (!awb->first_init)
    return TRUE;

  awb->first_init = FALSE;

  /* Get the initial awb settings from core */
  awb_get_param_info_type get_param_info;
  awb_get_param_output_type get_param_output;
  get_param_info.param_type = AWB_GET_SNAPSHOT_INFO;
  get_param_info.u.snap_est_info.flash_needed = FALSE;
  (void)awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
  const awb_output_params_type *output = &get_param_output.u.snapshot_info.output_params;
  awb->color_temp = output->color_temperature;
  awb->stored_awb_gain = awb->curr_gains = output->gains;
  for (int i = 0; i < AWB_NUMBER_OF_REFERENCE_POINT; i++) {
    awb->dualcam_sync.reference_points.rg_table[i] = chromatix->awb_basic_tuning.reference[i].RG_ratio;
    awb->dualcam_sync.reference_points.bg_table[i] = chromatix->awb_basic_tuning.reference[i].BG_ratio;
  }
  awb->current_wb_type = AWB_AUTO;
  awb->bestshot_info.curr_mode = AWB_BESTSHOT_OFF;
  awb->bst_blue_gain_adj = 1.0;

  /* Default value for lux index to calc CCM. It will be overwritten
   * with correct value once aec update is received.
   * e.g. When camera is directly launced in some MWB mode, CCM will
   * be calculated and sent out before AEC update is received.
   * Setting default lux-idx here to avoid this */
  awb->core_input.exposure_params.lux_index = 300;

  /* Initialize default values for flash off ccm, gains and cct */
  awb->awb_flash_off_settings.color_temperature = awb->color_temp;
  awb->awb_flash_off_settings.gains = awb->curr_gains;
  if (awb->awb_ccm_enable) {

    /* Get the default ccm based on initial lux index and cct */
    const q3a_core_result_type result =
      awb_biz_estimate_ccm((void*)awb, awb->color_temp,
        &(awb->awb_flash_off_settings.ccm), camera_id);
    if (Q3A_CORE_RESULT_SUCCESS == result) {
      /* Initially enable the override, later it will be updated in pack output */
      awb->awb_flash_off_settings.ccm.override_ccm = TRUE;
    } else {
      AWB_ERR("Error in retrieving the default ccm");
    }
  }
  return TRUE;
}

static boolean awb_biz_estimate_ccm(void *obj, float cct,
  awb_ccm_type *ccm, uint8_t camera_id) {
  int rc = 0;
  awb_biz_t *awb = (awb_biz_t *)obj;
  awb_get_param_info_type   get_param_info;
  awb_get_param_output_type get_param_output;
  (void)camera_id;

  /* Using lux-idx from AEC to get CCM value */
  get_param_info.u.ccm_input_info.exp_idx =
    (int)awb->core_input.exposure_params.lux_index;
  get_param_info.u.ccm_input_info.color_temperature = (int)cct;
  get_param_info.param_type = AWB_GET_PARAM_CCM_FROM_CCT;
  if (awb->awb_algo_ops.get_parameters) {
    rc = awb->awb_algo_ops.get_parameters(
      awb->handle, &get_param_info, &get_param_output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
    AWB_ERR("get_parameter fail: function not available");
  }

  if (rc == Q3A_CORE_RESULT_SUCCESS) {
    AWB_LOW("CCT: %f EI: %d ccm_updated: %d", cct,
      (int)awb->core_input.exposure_params.lux_index, ccm->override_ccm);
    STATS_MEMCPY(ccm, sizeof(awb_ccm_type), &(get_param_output.u.ccm), sizeof(awb_ccm_type));
  }
  return rc;
}
static q3a_core_result_type awb_biz_get_dual_led_settings(void *obj,
  awb_rgb_gains_type* input_gains) {
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  awb_biz_t *awb = (awb_biz_t *)obj;

  if (TRUE == awb->dual_LED_enable) {
    awb_get_param_info_type   get_param_info;
    awb_get_param_output_type get_param_output;

    /* Use MWB gains to get the dual LED settings */
    get_param_info.u.gains = *input_gains;
    get_param_info.param_type = AWB_GET_DUAL_LED_INFO;
    if (awb->awb_algo_ops.get_parameters) {
      rc = awb->awb_algo_ops.get_parameters(
        awb->handle, &get_param_info, &get_param_output);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
      AWB_ERR("get_parameter fail: function not available");
    }

    if (rc == Q3A_CORE_RESULT_SUCCESS) {
      awb->dual_led_settings.led1_high_setting =
        get_param_output.u.dual_led_data.dual_led_settings.led1_high_setting;
      awb->dual_led_settings.led1_low_setting =
        get_param_output.u.dual_led_data.dual_led_settings.led1_low_setting;
      awb->dual_led_settings.led2_high_setting =
        get_param_output.u.dual_led_data.dual_led_settings.led2_high_setting;
      awb->dual_led_settings.led2_low_setting =
        get_param_output.u.dual_led_data.dual_led_settings.led2_low_setting;
      awb->dual_led_flux_gain = get_param_output.u.dual_led_data.dual_led_flux_gain;

      AWB_LOW("LED1 High: %d, Low: %d, LED2 High: %d, Low: %d, flux: %f",
        awb->dual_led_settings.led1_high_setting, awb->dual_led_settings.led1_low_setting,
        awb->dual_led_settings.led2_high_setting, awb->dual_led_settings.led2_low_setting,
        awb->dual_led_flux_gain);
    }
  } else {
    /*For single led use case, we should set invalid value for daul
     *flash setting, so flash driver can use default current to fire led. */
    awb->dual_led_settings.led1_high_setting = -1;
    awb->dual_led_settings.led1_low_setting  = -1;
    awb->dual_led_settings.led2_high_setting = -1;
    awb->dual_led_settings.led2_low_setting  = -1;
  }
#else
  (void *)obj;
  (void *)input_gains;
#endif
  return rc;
}

/*===========================================================================
* FUNCTION    - awb_set_current_wb -
*
* DESCRIPTION:
*==========================================================================*/
static int awb_biz_set_current_wb(awb_biz_t *awb, uint32_t new_wb)
{
  int rc = 0;
  boolean update_hw = TRUE;
  const awb_gain_adjust_type *gain_adj = awb->gain_adj;
  awb_rgb_gains_type rgb_gains = awb->curr_gains;
  awb_gain_adjust_type gain_adjust = {1.0, 1.0};

  AWB_HIGH("E current awb=%d (rbg)=(%f, %f, %f)", awb->current_wb_type,
    awb->curr_gains.r_gain,awb->curr_gains.g_gain,awb->curr_gains.b_gain);
  if (awb->current_wb_type == new_wb && new_wb == CAMERA_WB_AUTO)
    update_hw = FALSE;
  else
    awb->current_wb_type = new_wb;

  switch (awb->current_wb_type) {
  case CAMERA_WB_AUTO:
    awb->awb_update = TRUE;
    rgb_gains = awb->stored_awb_gain;
    break;

  case CAMERA_WB_OFF: /* Maintain previous wb gains, Turn off auto WB */
    awb->awb_update = FALSE;
    break;

  case CAMERA_WB_CLOUDY_DAYLIGHT:
    /* Note the use of "day light" color conversion for cloudy here */
    /* Turn off auto WB */
    awb->awb_update= TRUE;
    rgb_gains = awb->chromatix_d65_white_balance;
    gain_adjust = gain_adj[Q3A_AWB_AGW_D65];
    awb->decision = Q3A_AWB_AGW_D65;
    awb->color_temp = 6500;
    break;

  case CAMERA_WB_INCANDESCENT: /* a.k.a "A",  Turn off auto WB */
    awb->awb_update = TRUE;
    rgb_gains = awb->chromatix_incandescent_white_balance;
    gain_adjust = gain_adj[Q3A_AWB_AGW_A];
    awb->decision = Q3A_AWB_AGW_A;
    awb->color_temp = 2800;
    break;

  case CAMERA_WB_FLUORESCENT: /* a.k.a "TL84",  Turn off auto WB */
  case CAMERA_WB_WARM_FLUORESCENT:
    awb->awb_update = TRUE;
    rgb_gains = awb->chromatix_tl84_white_balance;
    gain_adjust = gain_adj[Q3A_AWB_AGW_WARM_FLO];
    awb->decision = Q3A_AWB_AGW_WARM_FLO;
    awb->color_temp = 4100;
    break;

  case CAMERA_WB_TWILIGHT:
  case CAMERA_WB_DAYLIGHT: /* a.k.a "D65", Turn off auto WB */
    awb->awb_update = TRUE;
    rgb_gains = awb->chromatix_d50_white_balance;
    gain_adjust = gain_adj[Q3A_AWB_AGW_D50];
    awb->decision = Q3A_AWB_AGW_D50;
    awb->color_temp = 5000;
    break;

  case CAMERA_WB_SHADE:
    awb->awb_update = TRUE;
    rgb_gains = awb->chromatix_d65_white_balance;
    gain_adjust = gain_adj[Q3A_AWB_AGW_D75];
    awb->decision = Q3A_AWB_AGW_D75;
    awb->color_temp = 7500;
    break;

  case CAMERA_WB_CUSTOM:
  default:
    AWB_ERR("DEFAULT no effect");
    rc = -1;
    break;
  }

  if (rc == 0 && update_hw) {
    awb->curr_gains.r_gain = rgb_gains.r_gain * gain_adjust.red_gain_adj;
    awb->curr_gains.g_gain = rgb_gains.g_gain;
    awb->curr_gains.b_gain = rgb_gains.b_gain * gain_adjust.blue_gain_adj;
    awb->curr_gains.b_gain *= awb->bst_blue_gain_adj;
    awb->snap_gains = awb->curr_gains;
    awb->snap_color_temp = awb->color_temp;
  }

  AWB_HIGH("X new awb=%d (rbg)=(%f, %f, %f) Decision=%d", awb->current_wb_type,
    awb->curr_gains.r_gain,awb->curr_gains.g_gain,awb->curr_gains.b_gain,awb->decision);
  return rc;
} /* awb_set_current_wb */

static int awb_biz_set_bestshot_mode(awb_biz_t *awb,
  awb_bestshot_mode_type_t new_mode)
{
  int rc = 0;
  if (new_mode >= AWB_BESTSHOT_MAX) {
    new_mode = AWB_BESTSHOT_OFF;
    AWB_ERR("Invalid bestshot mode, setting it to default");
  }

  if (awb->bestshot_info.curr_mode == new_mode)
    return 0; /* Do Nothing */

  AWB_LOW("mode %d", new_mode);
  /* Store current AWB vals */
  if (awb->bestshot_info.curr_mode == AWB_BESTSHOT_OFF) {
    awb->bestshot_info.stored_wb = awb->current_wb_type;
    awb->bestshot_info.stored_bst_blue_gain_adj = awb->bst_blue_gain_adj;
  }
  /* CONFIG AWB for BESTHOT mode */
  if (new_mode  != AWB_BESTSHOT_OFF) {
    switch (new_mode) {
    case AWB_BESTSHOT_SNOW:
      awb->bst_blue_gain_adj = awb->snow_blue_gain_adj_ratio;
      AWB_LOW("snow %f", awb->bst_blue_gain_adj);
      break;
    case AWB_BESTSHOT_BEACH:
      awb->bst_blue_gain_adj = awb->beach_blue_gain_adj_ratio;
      AWB_LOW("beach %f", awb->bst_blue_gain_adj);
      break;
    case AWB_BESTSHOT_OFF:
    case AWB_BESTSHOT_LANDSCAPE:
    case AWB_BESTSHOT_NIGHT:
    case AWB_BESTSHOT_PORTRAIT:
    case AWB_BESTSHOT_BACKLIGHT:
    case AWB_BESTSHOT_SPORTS:
    case AWB_BESTSHOT_ANTISHAKE:
    case AWB_BESTSHOT_FLOWERS:
    case AWB_BESTSHOT_PARTY:
    case AWB_BESTSHOT_NIGHT_PORTRAIT:
    case AWB_BESTSHOT_THEATRE:
    case AWB_BESTSHOT_ACTION:
    case AWB_BESTSHOT_AR:
    case AWB_BESTSHOT_SUNSET:
    case AWB_BESTSHOT_FIREWORKS:
    case AWB_BESTSHOT_CANDLELIGHT:
    default:
      awb->bst_blue_gain_adj = 1.0;
    }
    switch (new_mode) {
    case AWB_BESTSHOT_FIREWORKS:
      rc = awb_biz_set_current_wb(awb, CAMERA_WB_CLOUDY_DAYLIGHT);
      break;
    case AWB_BESTSHOT_CANDLELIGHT:
      rc = awb_biz_set_current_wb(awb, CAMERA_WB_INCANDESCENT);
      break;
    case AWB_BESTSHOT_SUNSET:
      rc = awb_biz_set_current_wb(awb, CAMERA_WB_DAYLIGHT);
      break;
    case AWB_BESTSHOT_OFF:
    case AWB_BESTSHOT_LANDSCAPE:
    case AWB_BESTSHOT_SNOW:
    case AWB_BESTSHOT_BEACH:
    case AWB_BESTSHOT_NIGHT:
    case AWB_BESTSHOT_PORTRAIT:
    case AWB_BESTSHOT_BACKLIGHT:
    case AWB_BESTSHOT_SPORTS:
    case AWB_BESTSHOT_ANTISHAKE:
    case AWB_BESTSHOT_FLOWERS:
    case AWB_BESTSHOT_PARTY:
    case AWB_BESTSHOT_NIGHT_PORTRAIT:
    case AWB_BESTSHOT_THEATRE:
    case AWB_BESTSHOT_ACTION:
    case AWB_BESTSHOT_AR:
    default:
      rc = awb_biz_set_current_wb(awb, CAMERA_WB_AUTO);
      break;
    }
  } else { /* Restore AWB vals */
    rc = awb_biz_set_current_wb(awb, awb->bestshot_info.stored_wb);
    awb->bst_blue_gain_adj =  awb->bestshot_info.stored_bst_blue_gain_adj;
  }
  awb->bestshot_info.curr_mode = new_mode;
  return rc;
}

static boolean awb_set_aec_param(awb_biz_t *awb,
  awb_set_aec_parms  *aec_parms)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;

  if (!awb || !aec_parms) {
    AWB_ERR("invalid input:%p,%p",awb,aec_parms);
    return FALSE;
  }
  awb_aec_input_params_type *exp_param = &awb->core_input.exposure_params;
  exp_param->frame_luma = aec_parms->cur_luma;
  exp_param->exp_tbl_val = aec_parms->exp_tbl_val;
  exp_param->target_luma = aec_parms->target_luma;
  exp_param->flash_on = (aec_parms->est_state != AEC_EST_OFF
    || aec_parms->use_led_estimation);
  exp_param->lux_index = aec_parms->lux_idx;
  exp_param->exp_index = aec_parms->exp_index;
  exp_param->total_drc_gain = aec_parms->total_drc_gain;

  awb->est_state = aec_parms->est_state;
  awb->use_led_estimation = aec_parms->use_led_estimation;
  awb->flash_on = exp_param->flash_on;
  awb->flash_si.off = aec_parms->flash_sensitivity.off;
  awb->flash_si.low = aec_parms->flash_sensitivity.low;
  awb->flash_si.high = aec_parms->flash_sensitivity.high;
  awb->led_state = aec_parms->led_state;

  awb->core_input.snap_est_info.flash_needed = awb->use_led_estimation;
  awb->core_input.snap_est_info.sensitivity= awb->flash_si;

  /* Pass some of the AEC parameters insideto the core algorithm */
  set_param_info.param_type = AWB_SET_PARAM_AEC_PARMS;
  set_param_info.u.aec_parms.aec_settled = aec_parms->aec_settled;
  set_param_info.u.aec_parms.use_led_estimation = aec_parms->use_led_estimation;
  set_param_info.u.aec_parms.flash_on = exp_param->flash_on;
  set_param_info.u.aec_parms.led_state = aec_parms->led_state;
  set_param_info.u.aec_parms.cur_real_gain = aec_parms->cur_real_gain;

  /*FaceAWB*/
  set_param_info.u.aec_parms.roi_count = aec_parms->roi_count;
  int k = 0;
  for (k = 0; k < Q3A_CORE_MAX_ROI_COUNT; k++) {
    set_param_info.u.aec_parms.awb_roi_x[k] = aec_parms->awb_roi_x[k];
    set_param_info.u.aec_parms.awb_roi_y[k] = aec_parms->awb_roi_y[k];
    set_param_info.u.aec_parms.awb_roi_dx[k] = aec_parms->awb_roi_dx[k];
    set_param_info.u.aec_parms.awb_roi_dy[k] = aec_parms->awb_roi_dy[k];
    set_param_info.u.aec_parms.awb_roi_weight[k] = aec_parms->awb_roi_weight[k];
  }

  set_param_info.u.aec_parms.frm_width = aec_parms->frm_width;
  set_param_info.u.aec_parms.frm_height = aec_parms->frm_height;
  set_param_info.u.aec_parms.faceawb = aec_parms->faceawb;

  set_param_info.u.aec_parms.Dual_LED1_influence_ratio = aec_parms->Dual_LED1_influence_ratio;
  awb_biz_copy_dual_led_setting(&aec_parms->dual_led_setting.low_setting,
    &set_param_info.u.aec_parms.dual_led.low_setting);
  awb_biz_copy_dual_led_setting(&aec_parms->dual_led_setting.high_setting,
    &set_param_info.u.aec_parms.dual_led.high_setting);
  awb_biz_copy_dual_led_setting(&aec_parms->dual_led_setting.full_setting,
    &set_param_info.u.aec_parms.dual_led.full_setting);

  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }

  if(rc != Q3A_CORE_RESULT_SUCCESS){
    AWB_ERR("Failed to set AEC parameters in the algorithm");
    return FALSE;
  }
  return TRUE;
}

/** awb_process_translate_dim_fov2camif
 *
 **/
boolean awb_biz_translate_dim_fov2camif(awb_biz_t *awb,
  uint16_t *dx, uint16_t *dy)
{
  int  roi_dx, roi_dy;
  if (awb->stream_crop_info.vfe_out_width == 0 ||
      awb->stream_crop_info.vfe_out_height == 0) {
    return FALSE;
  }
  if (! awb->stream_crop_info.pp_crop_out_x ||
      !awb->stream_crop_info.pp_crop_out_y) {
    awb->stream_crop_info.pp_x = 0;
    awb->stream_crop_info.pp_y = 0;
    awb->stream_crop_info.pp_crop_out_x = awb->preview_width;
    awb->stream_crop_info.pp_crop_out_y = awb->preview_height;
  }

  /* Reverse calculation to cpp output */
  roi_dx = *dx * awb->stream_crop_info.pp_crop_out_x / awb->preview_width ;
  roi_dy = *dy * awb->stream_crop_info.pp_crop_out_y / awb->preview_height;
  /* Reverse calculation for vfe output */
  roi_dx = roi_dx * awb->stream_crop_info.vfe_map_width / awb->stream_crop_info.vfe_out_width ;
  roi_dy = roi_dy * awb->stream_crop_info.vfe_map_height / awb->stream_crop_info.vfe_out_height;
  AWB_HIGH("orig (%d,%d), vfe (%d, %d,%d %d ) vfe out(%d %d) pp(%d %d %d %d) after (%d, %d)",
    *dx, *dy, awb->stream_crop_info.vfe_map_x, awb->stream_crop_info.vfe_map_y,
    awb->stream_crop_info.vfe_map_width, awb->stream_crop_info.vfe_map_height,
    awb->stream_crop_info.vfe_out_width, awb->stream_crop_info.vfe_out_height,
    awb->stream_crop_info.pp_x, awb->stream_crop_info.pp_y,
    awb->stream_crop_info.pp_crop_out_x, awb->stream_crop_info.pp_crop_out_y,
    roi_dx, roi_dy);
  *dx = roi_dx;
  *dy = roi_dy;
  return TRUE;
}

boolean awb_biz_translate_coord_fov2camif(awb_biz_t *awb,
  uint16_t *x, uint16_t *y)
{
  int  roi_x, roi_y;
  if (awb->stream_crop_info.vfe_out_width == 0 ||
      awb->stream_crop_info.vfe_out_height == 0) {
    return FALSE;
  }
  if (awb->preview_width == 0 || awb->preview_height == 0) {
    awb->preview_width = awb->stream_crop_info.pp_crop_out_x + 1;
    awb->preview_height = awb->stream_crop_info.pp_crop_out_y + 1;
  }
  AWB_LOW("awb->preview_width=%d, awb->preview_height=%d,"
    " awb->stream_crop_info.vfe_out_width=%d, awb->stream_crop_info.vfe_out_height=%d",
    awb->preview_width, awb->preview_height,
    awb->stream_crop_info.vfe_out_width, awb->stream_crop_info.vfe_out_height);
  if (!awb->stream_crop_info.pp_crop_out_x ||
      !awb->stream_crop_info.pp_crop_out_y) {
    awb->stream_crop_info.pp_x = 0;
    awb->stream_crop_info.pp_y = 0;
    awb->stream_crop_info.pp_crop_out_x = awb->preview_width;
    awb->stream_crop_info.pp_crop_out_y = awb->preview_height;
  }

  /* Reverse calculation to cpp output */
  roi_x = *x * awb->stream_crop_info.pp_crop_out_x /awb->preview_width ;
  roi_y = *y * awb->stream_crop_info.pp_crop_out_y / awb->preview_height;
  roi_x += awb->stream_crop_info.pp_x;
  roi_y += awb->stream_crop_info.pp_y;
  /* Reverse calculation for vfe output */
  roi_x = roi_x * awb->stream_crop_info.vfe_map_width / awb->stream_crop_info.vfe_out_width ;
  roi_y = roi_y *  awb->stream_crop_info.vfe_map_height / awb->stream_crop_info.vfe_out_height;
  roi_x += awb->stream_crop_info.vfe_map_x;
  roi_y += awb->stream_crop_info.vfe_map_y;

  AWB_LOW("coor orig (%d,%d), vfe (%d, %d,%d %d ) pp(%d %d %d %d) after (%d, %d) prev(%d %d)",
    *x, *y, awb->stream_crop_info.vfe_map_x, awb->stream_crop_info.vfe_map_y,
    awb->stream_crop_info.vfe_map_width, awb->stream_crop_info.vfe_map_height,
    awb->stream_crop_info.pp_x, awb->stream_crop_info.pp_y,
    awb->stream_crop_info.pp_crop_out_x, awb->stream_crop_info.pp_crop_out_y,
    roi_x, roi_y, awb->preview_width, awb->preview_height);

  *x = roi_x;
  *y = roi_y;
  return TRUE;
}

void awb_biz_pack_stats_config(awb_biz_t *awb,
  awb_output_data_t *output)
{
  uint16_t top, left, width = 0, height = 0, config_width, config_height;
  awb_config_t *config = NULL;
  aec_bg_config_t *bg_config = NULL;
  config = &awb->stats_config;
  top = left = 0;

  /* Verify if configuration is required */
  if (FALSE == awb->bg_config_stats_required) {
    AWB_LOW("BG configuration not required");
    return;
  }

  if (!awb_biz_translate_coord_fov2camif(awb, &left, &top)) {
    AWB_LOW("invalid inputs to translate fov 2 camif");
  }
  /* add check to to make widht as vfe width */
  if(!awb->stream_crop_info.pp_x && !awb->stream_crop_info.pp_y &&
    !awb->stream_crop_info.pp_crop_out_x && !awb->stream_crop_info.pp_crop_out_y){
    width = awb->stream_crop_info.vfe_out_width;
    height = awb->stream_crop_info.vfe_out_height;
  } else {
    width = awb->preview_width;
    height = awb->preview_height;
  }
  if (!awb_biz_translate_dim_fov2camif(awb, &width, &height)) {
    AWB_HIGH("invalid inputs to translate dimensions for  fov 2 camif");
    width = awb->sensor_info.sensor_res_width;
    height = awb->sensor_info.sensor_res_height;
  }

  int32 bit_shift = 0;
  if (awb->stats_depth > AWB_BG_STATS_CONSUMP_BIT_WIDTH) {
    bit_shift = awb->stats_depth - AWB_BG_STATS_CONSUMP_BIT_WIDTH;
  }

  /* BG stats to be configure */
  bg_config = &config->bg_config;

  bg_config->is_valid = TRUE;
  bg_config->b_Max  = (255 - 16) << bit_shift;
  bg_config->gb_Max = (255 - 16) << bit_shift;
  bg_config->gr_Max = (255 - 16) << bit_shift;
  bg_config->r_Max  = (255 - 16) << bit_shift;
  bg_config->grid_info.h_num = 64;
  bg_config->grid_info.v_num = 48;
  config_width =
    (left + width > awb->sensor_info.sensor_res_width) ?
    (awb->sensor_info.sensor_res_width - left) : width;
  config_height =
    (top + height > awb->sensor_info.sensor_res_height) ?
    (awb->sensor_info.sensor_res_height - top) : height;

  if (awb->awb_full_camif_enable == TRUE) {
    /* config AWB ROI to full camif to overwrite FOV config*/
    left = awb->sensor_info.sensor_left;
    top = awb->sensor_info.sensor_top;
    config_width = awb->sensor_info.sensor_res_width;
    config_height = awb->sensor_info.sensor_res_height;
  }

  AWB_LOW("ROI L=%d, T=%d, W=%d, H=%d:: L=%d, T=%d, W=%d, H=%d, BGConfigRqd = %d, isFullCAMIF=%d",
    bg_config->roi.left, bg_config->roi.top, bg_config->roi.width, bg_config->roi.height,
    left, top, config_width, config_height,
    awb->bg_config_stats_required, awb->awb_full_camif_enable);

  if (bg_config->roi.left != left ||
      bg_config->roi.top != top ||
      bg_config->roi.width != config_width ||
      bg_config->roi.height != config_height) {
    bg_config->roi.left   = left;
    bg_config->roi.top    = top;
    bg_config->roi.width  = config_width;
    bg_config->roi.height = config_height;

    /* Request config */
    output->need_config = 1;
    STATS_MEMCPY(&output->config, sizeof(awb_config_t), config, sizeof(awb_config_t));
  } else {
    output->need_config = 0;
  }

}

/** awb_biz_pack_output:
*
**/
static void  awb_biz_pack_output(awb_biz_t *awb, awb_output_data_t *output)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_get_param_info_type get_param_info;
  awb_get_param_output_type get_param_output;

  AWB_LOW("Enter");

  STATS_MEMSET(output, 0, sizeof(awb_output_data_t));

  if (NULL == awb->awb_algo_ops.get_parameters) {
    AWB_ERR("error: get_parameters func not set");
    return;
  }

  /* Store the ccm, gains and cct in flash off */
  if (FALSE == awb->flash_on) {
    awb->awb_flash_off_settings.color_temperature = awb->color_temp;
    awb->awb_flash_off_settings.gains = awb->curr_gains;
    awb->awb_flash_off_settings.unadjusted_awb_gain = awb->unadjusted_awb_gain;
    if (awb->awb_ccm_enable) {
      awb->awb_flash_off_settings.ccm = awb->core_output.ccm;
    }
  }

  output->type = AWB_UPDATE;
  output->r_gain = awb->curr_gains.r_gain;
  output->g_gain = awb->curr_gains.g_gain;
  output->b_gain = awb->curr_gains.b_gain;

  output->unadjusted_r_gain = awb->unadjusted_awb_gain.r_gain;
  output->unadjusted_g_gain = awb->unadjusted_awb_gain.g_gain;
  output->unadjusted_b_gain = awb->unadjusted_awb_gain.b_gain;
  output->is_awb_converge = awb->is_awb_converge;
  output->color_temp = awb->color_temp;
  output->decision = awb->decision;
  STATS_MEMCPY(output->samp_decision, sizeof(output->samp_decision),
    awb->sample_decision, sizeof(awb->sample_decision));
  output->awb_update = awb->awb_update;
  output->wb_mode = awb->current_wb_type;
  output->best_mode = awb->bestshot_info.curr_mode;
  output->frame_id = awb->stats.frame_id;
  output->awb_lock = awb->awb_locked;
  output->sof_id = awb->sof_id;
  output->awb_ccm_enable = awb->awb_ccm_enable;
  output->ccm = awb->core_output.ccm;

  memcpy(&output->dualcam_sync, &awb->dualcam_sync, sizeof(awb_dualcam_sync_info_t));

  if (awb->current_wb_type == AWB_AUTO)
    awb->stored_awb_gain = awb->curr_gains;

  output->snap_r_gain = awb->snap_gains.r_gain;
  output->snap_g_gain = awb->snap_gains.g_gain;
  output->snap_b_gain = awb->snap_gains.b_gain;
  output->snap_color_temp = awb->snap_color_temp;
  output->snap_ccm = awb->core_output.snapshot_ccm;
  if (awb->op_mode == Q3A_OPERATION_MODE_SNAPSHOT) {
    /* Update the WB gains and ccm for snapshot */
    output->r_gain = output->snap_r_gain;
    output->g_gain = output->snap_g_gain;
    output->b_gain = output->snap_b_gain;
    output->ccm = awb->core_output.snapshot_ccm;
    output->color_temp = awb->snap_color_temp;
  }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  output->dual_led_settings.is_valid = true;
  output->dual_led_settings.led1_high_setting =
    awb->dual_led_settings.led1_high_setting;
  output->dual_led_settings.led1_low_setting =
    awb->dual_led_settings.led1_low_setting;
  output->dual_led_settings.led2_high_setting =
    awb->dual_led_settings.led2_high_setting;
  output->dual_led_settings.led2_low_setting =
    awb->dual_led_settings.led2_low_setting;
  output->dual_led_flux_gain = awb->dual_led_flux_gain;
#endif

 /* Get eztune information from core if eztune enabled */
  output->eztune_data.ez_running = awb->ez_running;
  if (awb->ez_running) {
    get_param_info.param_type = AWB_GET_EZTUNE_INFO;
    get_param_output.u.eztune_info = (void*)(&output->eztune_data);
    rc = awb->awb_algo_ops.get_parameters(
      awb->handle,&get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("awb get param failed");
      return;
    }
    /* in case it is mwb mode, need update the eztune data. */
    output->eztune_data.awb_enable = !awb->awb_enable;
    output->eztune_data.awb_mode = output->wb_mode;
    output->eztune_data.lock = awb->awb_locked;
    output->eztune_data.snapshot_r_gain = output->snap_r_gain;
    output->eztune_data.snapshot_g_gain = output->snap_g_gain;
    output->eztune_data.snapshot_b_gain = output->snap_b_gain;
    output->eztune_data.preview_r_gain = output->r_gain;
    output->eztune_data.preview_g_gain = output->g_gain;
    output->eztune_data.preview_b_gain = output->b_gain;
    output->eztune_data.decision = output->decision;
    output->eztune_data.color_temp = output->color_temp;
    STATS_MEMCPY(output->eztune_data.samp_decision, sizeof(output->eztune_data.samp_decision),
      awb->sample_decision, sizeof(output->eztune_data.samp_decision));
  }

  /* Stats configuration */
  awb_biz_pack_stats_config(awb, output);

  /* Get debug data information from core if exif debug data enabled */
  if (awb->exif_dbg_enable) {
    get_param_info.param_type = AWB_GET_METADATA_INFO;
    get_param_info.u.metadata_size = AWB_DEBUG_DATA_SIZE;
    get_param_output.u.metadata_info.metadata = (void*)output->awb_debug_data_array;
    rc = awb->awb_algo_ops.get_parameters(
      awb->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("awb get param failed");
      return;
    }
    output->awb_debug_data_size = get_param_output.u.metadata_info.metadata_size;
    AWB_LOW("debug data size: %d",output->awb_debug_data_size);
  } else {
    output->awb_debug_data_size = 0;
    AWB_LOW("debug data not enabled");
  }
}

static boolean awb_set_force_dual_led_idx(awb_biz_t *awb, uint8_t force_dual_led_idx)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;

  set_param_info.param_type = AWB_SET_PARAM_DUAL_LED_FORCE_IDX;
  set_param_info.u.ez_force_dual_led_idx = force_dual_led_idx;
  AWB_LOW("set forced led idx: %d",force_dual_led_idx);
  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
      AWB_ERR("awb set param failed");
      return FALSE;
  }
  return TRUE;
}

static boolean awb_biz_set_hdr_mode(awb_biz_t *awb, awb_snapshot_hdr_type snapshot_hdr)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;
  set_param_info.param_type = AWB_SET_PARAM_HDR_MODE;
  set_param_info.u.snapshot_hdr = snapshot_hdr;
  AWB_LOW("hdr mode: %d", snapshot_hdr);
  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
      AWB_ERR("awb set param failed");
      return FALSE;
  }
  return TRUE;
}

static boolean awb_biz_set_operational_mode(awb_biz_t *awb)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;
  q3a_core_operational_mode_type core_op_mode =
    Q3A_CORE_OPERATIONAL_MODE_PREVIEW;

  // Map business logic mode to core operational mode.
  // Set the operational mode only for valid values.
  switch (awb->op_mode) {
    case Q3A_OPERATION_MODE_PREVIEW:
      core_op_mode = Q3A_CORE_OPERATIONAL_MODE_PREVIEW;
      break;

    case Q3A_OPERATION_MODE_CAMCORDER:
      core_op_mode = Q3A_CORE_OPERATIONAL_MODE_CAMCORDER;
      break;

    case Q3A_OPERATION_MODE_SNAPSHOT:
      core_op_mode = Q3A_CORE_OPERATIONAL_MODE_SNAPSHOT;
      break;

    default:
      // Don't do anything.
      break;
  }

  // Set operational mode inside the core.
  set_param_info.param_type = AWB_SET_PARAM_OPERATIONAL_MODE;
  set_param_info.u.op_mode = core_op_mode;
  AWB_LOW("set operational mode: %d",core_op_mode);

  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
      AWB_ERR("awb set operational operational mode failed");
      return FALSE;
  }
  return TRUE;
}

/**
 * awb_biz_set_dual_led_calibration_mode
 *
 * Set the current LED calibration mode in the algorithm. This should
 * be called to enable or disable the calibration mode
 *
 * @param awb
 * @param state
 *
 */
static boolean awb_biz_set_dual_led_calibration_mode(
  awb_biz_t* awb, boolean enabled)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;
  if (awb->dual_led_calib_mode_enabled != enabled) {
    AWB_HIGH("Setting dual LED calibration mode to: %s", enabled ? "enabled" : "disabled");
    /* Set dual LED calibration mode inside core. */
    set_param_info.param_type = AWB_SET_PARAM_DUALLED_CALIB_MODE;
    set_param_info.u.dual_led_calib_enabled = enabled;

    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    /* Save the mode in the business logic */
    awb->dual_led_calib_mode_enabled = enabled;

    if(rc != Q3A_CORE_RESULT_SUCCESS){
      AWB_ERR("Failed to set dual LED calibration mode");
      return FALSE;
    }
  }
  return TRUE;
}

static boolean awb_biz_set_fast_aec_data(awb_biz_t *awb, q3a_fast_aec_data_t *fast_aec_data)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;

  set_param_info.param_type = AWB_SET_PARAM_BYPASS_TEMPORAL_FILTER;
  if(fast_aec_data->enable) {
    set_param_info.u.bypass_temporal_filter = TRUE;
  } else {
    set_param_info.u.bypass_temporal_filter = FALSE;
  }
  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
    AWB_ERR("awb set param failed");
    return FALSE;
  }
  return TRUE;
}

static void awb_biz_prepare_for_zsl(awb_biz_t *awb) {

  /* 1. In normal ZSL, set operational mode to snapshot.
   * 2. In longshot flash, set operational mode based on tuning param. */
  q3a_operation_mode_t new_mode = Q3A_OPERATION_MODE_SNAPSHOT;
  if (awb_biz_is_longshot_flash(awb) && !LONGSHOT_FLASH_AWB_LOCK) {
    new_mode = Q3A_OPERATION_MODE_PREVIEW;
  }

  if (new_mode != awb->op_mode) {
    awb->op_mode = new_mode;
    (void)awb_biz_set_operational_mode(awb);
  }
}

static void awb_biz_post_zsl(awb_biz_t *awb) {

  /* Change the operational mode back to preview */
  if (Q3A_OPERATION_MODE_PREVIEW != awb->op_mode) {
    awb->op_mode = Q3A_OPERATION_MODE_PREVIEW;
    (void)awb_biz_set_operational_mode(awb);
  }

  /* Reset flash information */
  awb_biz_reset_flash_info(awb);
}

static boolean awb_biz_set_custom_evt_hal(awb_biz_t *awb, q3a_custom_data_t *qcd)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_set_param_info_type set_param_info;
  custom_parm_buffer_t *cpb;

  set_param_info.param_type = AWB_SET_PARAM_CUSTOM_EVT_HAL;
  set_param_info.u.q3a_custom_data.data = qcd->data;
  set_param_info.u.q3a_custom_data.size = qcd->size;
  if (awb->awb_algo_ops.set_parameters) {
    rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS){
    AWB_ERR("awb set param failed");
    return FALSE;
  }

  return TRUE;
}

/** awb_set_parameters:
*
**/
static boolean awb_biz_set_params(awb_set_parameter_t *param,
  awb_output_data_t *output, uint8_t number_of_outputs, void *awb_obj)
{
  boolean rc            = TRUE;
  boolean need_callback = FALSE;
  awb_biz_t *awb = (awb_biz_t *)awb_obj;

  if (!param || !awb_obj || number_of_outputs > MAX_AWB_ALGO_OUTPUTS) {
    AWB_ERR("Invalid params, outputs: %u", number_of_outputs);
    rc = FALSE;
    goto done;
  }

  AWB_LOW("AWBSetParam type =%d", param->type);

  switch (param->type) {
  case AWB_SET_PARAM_INIT_CHROMATIX_SENSOR:
    rc = awb_biz_set_init(awb, &param->u.init_param, param->camera_id);
    break;

  case AWB_SET_PARAM_INIT_SENSOR_INFO:
    awb->sensor_info = param->u.init_param.sensor_info;
    STATS_MEMSET(&awb->stats_config, 0, sizeof(awb_config_t));
    break;

  case AWB_SET_PARAM_UI_FRAME_DIM:
    awb->preview_width =  param->u.init_param.frame_dim.width;
    awb->preview_height = param->u.init_param.frame_dim.height;
    break;

  case AWB_SET_PARAM_CROP_INFO:
    awb->stream_crop_info = param->u.stream_crop;
    break;

  case AWB_SET_PARAM_PACK_OUTPUT:
    if (NULL == output) {
      AWB_ERR("Error: param: AWB_SET_PARAM_PACK_OUTPUT, output provided is null");
      rc = FALSE;
      break;
    }
    awb->sof_id = param->u.current_sof_id;
    awb_biz_pack_output(awb, output);
    break;

  case AWB_SET_PARAM_AEC_PARM:
    rc = awb_set_aec_param(awb, &param->u.aec_parms);
    break;

  case AWB_SET_PARAM_WHITE_BALANCE:
    if (awb->bestshot_info.curr_mode == AWB_BESTSHOT_OFF) {
      rc = awb_biz_set_current_wb(awb, param->u.awb_current_wb);
    } else {
      awb->bestshot_info.stored_wb = param->u.awb_current_wb;
    }
    break;

  case AWB_SET_PARAM_BESTSHOT:
    rc = awb_biz_set_bestshot_mode(awb,param->u.awb_best_shot);
    break;

  case AWB_SET_PARAM_OP_MODE:
    awb->previous_op_mode = awb->op_mode;

    /* Longshot flash, set operational mode based on tuning params.
     * This is needed only for non zsl longshot flash snapshot */
    if (awb_biz_is_longshot_flash(awb)) {
      awb->op_mode = (LONGSHOT_FLASH_AWB_LOCK) ?
      Q3A_OPERATION_MODE_SNAPSHOT : Q3A_OPERATION_MODE_PREVIEW;
      AWB_LOW("Set mode based on longshot, mode=%d", awb->op_mode);
    } else {
      awb->op_mode = param->u.init_param.op_mode;
    }

    AWB_LOW("current op_mode: %d, previous op_mode: %d",
      awb->op_mode, awb->previous_op_mode);

    rc = awb_biz_set_operational_mode(awb);
    break;

  case AWB_SET_PARAM_VIDEO_HDR:
    awb->video_hdr = param->u.video_hdr;
    break;

  case AWB_SET_PARAM_SNAPSHOT_HDR:
    awb->snapshot_hdr = param->u.snapshot_hdr;
    rc = awb_biz_set_hdr_mode(awb, awb->snapshot_hdr);
    break;

  case AWB_SET_PARAM_LOCK:
    awb->awb_locked = param->u.awb_lock;
    break;

  case AWB_SET_PARAM_STATS_DEBUG_MASK:
    break;

  case AWB_SET_PARAM_ENABLE:
    awb->awb_enable = !param->u.awb_enable;
    break;

  case AWB_SET_PARAM_EZ_TUNE_RUNNING:
    awb->ez_running = param->u.ez_running;
    break;

  case AWB_SET_MANUAL_AUTO_SKIP:
    awb->manual_to_auto_skip_cnt  = AWB_MANUAL_TO_AUTO_SKIP_CNT;
    break;

  case AWB_SET_PARAM_CAPTURE_MODE:
    awb->is_still_capture = 1;
    break;

  case AWB_SET_PARAM_EZ_DUAL_LED_FORCE_IDX:
    rc = awb_set_force_dual_led_idx(awb, param->u.ez_force_dual_led_idx);
    break;
  case AWB_SET_PARAM_STATS_DEPTH: {
    awb->stats_depth = param->u.stats_depth;
  }
    break;
  case AWB_SET_PARM_FAST_AEC_DATA: {
    rc = awb_biz_set_fast_aec_data(awb, &param->u.fast_aec_data);
  }
    break;
  case AWB_SET_PARAM_DUAL_LED_CALIB_MODE: {
    rc = awb_biz_set_dual_led_calibration_mode(awb, param->u.dual_led_calib_mode);
  }
    break;
  case AWB_SET_PARAM_LONGSHOT_MODE: {
    awb->in_longshot_mode = param->u.longshot_mode;
  }
    break;
  case AWB_SET_PARAM_LED_RESET: {
    awb_biz_reset_flash_info(awb);
  }
    break;
  case AWB_SET_PARAM_ZSL_START: {
    awb_biz_prepare_for_zsl(awb);
  }
    break;
  case AWB_SET_PARAM_ZSL_STOP: {
    awb_biz_post_zsl(awb);
  }
    break;
  case AWB_SET_PARM_INSTANT_AEC_TYPE: {
    q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;

    awb_set_param_info_type set_param_info;
    set_param_info.param_type = AWB_SET_PARAM_INSTANT_AEC_TYPE;
    set_param_info.u.instant_aec_type = param->u.instant_aec_type;
    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    if(rc != Q3A_CORE_RESULT_SUCCESS){
      AWB_ERR("Failed to set AEC parameters in the algorithm");
      return FALSE;
    }
  }
    break;
  case AWB_SET_PARM_CONV_SPEED: {
    q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;

    awb_set_param_info_type set_param_info;
    set_param_info.param_type = AWB_SET_PARAM_CONV_SPEED;
    set_param_info.u.conv_speed = param->u.conv_speed;
    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("Failed to set awb convergence speed in the algorithm");
      return FALSE;
    }
  }
    break;

  case AWB_SET_PARM_ROLE_SWITCH: {
    q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;

    awb_set_param_info_type set_param_info;
    set_param_info.param_type = AWB_SET_PARAM_ROLE_SWITCH;
    memcpy(&set_param_info.u.role_switch,&param->u.init_param.role_switch_param,
      sizeof(awb_role_switch_params_t));

    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("Failed to set awb Role Switch in the algorithm");
      return FALSE;
    }
  }
    break;

  case AWB_SET_PARM_CUSTOM_EVT_HAL: {
    rc = awb_biz_set_custom_evt_hal(awb, &param->u.awb_custom_data);
  }
    break;

  default:
    rc = FALSE;
    break;
 }

done:
  return rc;
}

static void awb_biz_get_snapshot_info(awb_biz_t *awb,
  awb_get_parameter_t *param) {
  awb_get_param_info_type get_param_info;
  awb_get_param_output_type get_param_output;
  q3a_core_result_type ret = Q3A_CORE_RESULT_SUCCESS;

  get_param_info.param_type = AWB_GET_SNAPSHOT_INFO;
  get_param_info.u.snap_est_info.flash_needed = awb->use_led_estimation;
  get_param_info.u.snap_est_info.sensitivity = awb->flash_si;
  ret = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
  if (ret == Q3A_CORE_RESULT_SUCCESS) {
    awb_output_params_type *output =
      &get_param_output.u.snapshot_info.output_params;
    param->u.awb_gains.color_temp = output->color_temperature;
    param->u.awb_gains.curr_gains = output->gains;
    param->u.awb_gains.ccm = output->ccm;
  } else {
    AWB_ERR("awb get snapshot info failed: %d",ret);
  }
}

static boolean awb_biz_is_longshot_flash (awb_biz_t *awb) {
  const boolean longshot_flash =
    (awb->in_longshot_mode && awb->use_led_estimation) ? TRUE : FALSE;
  return longshot_flash;
}

static void awb_biz_get_longshot_flash_settings(awb_biz_t *awb,
  awb_get_parameter_t *param) {
  boolean rc = FALSE;

  /* In longshot flash snapshot. This case execute only for nonZSL*/
  if (awb->in_longshot_mode && awb->use_led_estimation && param) {
    AWB_HIGH("Longshot Lock=%d", LONGSHOT_FLASH_AWB_LOCK);
    if (LONGSHOT_FLASH_AWB_LOCK) {
      /* AWB is locked, use snapshot WB settings */
      awb_biz_get_snapshot_info(awb, param);
    } else {
      /* AWB is not locked, use preflash WB settings */
      param->u.awb_gains.curr_gains = awb->curr_gains;
      param->u.awb_gains.color_temp = awb->color_temp;
      if (awb->awb_ccm_enable) {
        param->u.awb_gains.ccm = awb->core_output.ccm;
        param->u.awb_gains.ccm.override_ccm = TRUE;
      }
    }
  }
}

static boolean awb_biz_get_params(awb_get_parameter_t *param,
  void *awb_obj)
{
  boolean rc = TRUE;
  awb_biz_t *awb = (awb_biz_t *)awb_obj;
  q3a_core_result_type ret = Q3A_CORE_RESULT_SUCCESS;
  awb_get_param_info_type get_param_info;
  awb_get_param_output_type get_param_output;

  Q3A_MEMSET(&get_param_info, 0, sizeof(awb_get_param_info_type));
  Q3A_MEMSET(&get_param_output, 0, sizeof(awb_get_param_output_type));

  if (!param || !awb || NULL == awb->awb_algo_ops.get_parameters) {
      AWB_ERR("error: invalid parameters");
    return FALSE;
  }

  switch (param->type){

  case AWB_INTERPOLATED_GAINS:{
    get_param_info.param_type = AWB_GET_INTERPLOATED_GAIN;
    Q3A_MEMCPY(&get_param_info.u.interploatedGains,&param->u.interpolated_gains,sizeof(awb_interpolation_gain_params_t));
    rc = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
    Q3A_MEMCPY(&param->u.interpolated_gains,&get_param_output.u.interploatedGains,sizeof(awb_interpolation_gain_params_t));
  }
    break;

  case AWB_GAINS:{
    /* Get the initial WB gains */
    if (TRUE == awb_biz_is_longshot_flash(awb) &&
      AWB_AUTO == awb->current_wb_type) {
      /* Get longshot WB settings */
      awb_biz_get_longshot_flash_settings(awb, param);
    } else {
      AWB_HIGH("Operational mode=%d, WB mode=%d",
        awb->op_mode, awb->current_wb_type);

      /* Snapshot mode */
      if (awb->op_mode == Q3A_OPERATION_MODE_SNAPSHOT) {
        if (awb->current_wb_type == AWB_AUTO) {
          awb_biz_get_snapshot_info(awb, param);
        } else {
          /* Manual mode, get ccm based on the CCT */
          param->u.awb_gains.color_temp = awb->color_temp;
          param->u.awb_gains.curr_gains = awb->curr_gains;
          if (awb->awb_ccm_enable) {
            (void)awb_biz_estimate_ccm((void*)awb, awb->color_temp,
              &param->u.awb_gains.ccm, param->camera_id);
            param->u.awb_gains.ccm.override_ccm = TRUE;
          }
        }
      } else {
        /* In preview start, always use the non flash WB settings */
        param->u.awb_gains.curr_gains = awb->awb_flash_off_settings.gains;
        param->u.awb_gains.unadjusted_awb_gain = awb->awb_flash_off_settings.unadjusted_awb_gain;
        param->u.awb_gains.color_temp = awb->awb_flash_off_settings.color_temperature;
        if (awb->awb_ccm_enable) {
          param->u.awb_gains.ccm = awb->awb_flash_off_settings.ccm;
          param->u.awb_gains.ccm.override_ccm = TRUE;
        }
      }
    }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
    /* Update dual LED currents */
    param->u.awb_gains.led1_low_setting = awb->dual_led_settings.led1_low_setting;
    param->u.awb_gains.led2_low_setting = awb->dual_led_settings.led2_low_setting;
    param->u.awb_gains.led1_high_setting = awb->dual_led_settings.led1_high_setting;
    param->u.awb_gains.led2_high_setting = awb->dual_led_settings.led2_high_setting;
#endif
    param->u.awb_gains.awb_ccm_enable = awb->awb_ccm_enable;
  }
    break;

  case AWB_PARMS:{
    param->u.awb_gains.color_temp = awb->color_temp;
    param->u.awb_gains.curr_gains = awb->curr_gains;
  }
    break;

  case AWB_UNIFIED_FLASH:
  {
    if (AWB_AUTO == awb->current_wb_type) {
      int32_t led_off_color = 0, led_on_color = 0;
      chromatix_manual_white_balance_adapter led_off_gains;
      chromatix_manual_white_balance_adapter led_on_gains;
      chromatix_manual_white_balance_adapter output_gains;
      uint32 led_off_color_temp;
      get_param_info.param_type = AWB_GET_UNIFIED_FLASH;
      get_param_info.u.snap_est_info.flash_needed = awb->use_led_estimation;
      get_param_info.u.snap_est_info.sensitivity = awb->flash_si;
      ret = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
      if (ret == Q3A_CORE_RESULT_SUCCESS) {
        awb_output_params_type *output =
          &get_param_output.u.unified_flash.output_params;
        awb_rgb_gains_type *off_gains =
          &get_param_output.u.unified_flash.led_off_final_gain;
        led_off_color_temp = get_param_output.u.unified_flash.led_off_color_temp;
        int i = 0;
        if (awb->current_wb_type == AWB_AUTO) {
          if (awb->use_led_estimation) {
            led_on_gains.r_gain = output->gains.r_gain;
            led_on_gains.b_gain = output->gains.b_gain;
            led_on_gains.g_gain = output->gains.g_gain;
            led_on_color = output->color_temperature;
            led_off_gains.r_gain = off_gains->r_gain;
            led_off_gains.b_gain = off_gains->b_gain;
            led_off_gains.g_gain = off_gains->g_gain;
            led_off_color = led_off_color_temp;
          } else {
            led_off_gains.r_gain = led_on_gains.r_gain = output->gains.r_gain;
            led_off_gains.b_gain = led_on_gains.b_gain = output->gains.b_gain;
            led_off_gains.g_gain = led_on_gains.g_gain = output->gains.g_gain;
            led_off_color = led_on_color = output->color_temperature;
          }
        } else {
          led_off_color = led_on_color = awb->color_temp;
          led_off_gains = led_on_gains = awb->curr_gains;
        }
        for (i = 0; i < param->u.frame_info.num_batch; i++) {
         if ((param->u.frame_info.frame_batch[i].flash_mode == TRUE) &&
           (awb->use_led_estimation == TRUE)) {
           param->u.frame_info.frame_batch[i].r_gain = led_on_gains.r_gain;
           param->u.frame_info.frame_batch[i].b_gain = led_on_gains.b_gain;
           param->u.frame_info.frame_batch[i].g_gain = led_on_gains.g_gain;
           param->u.frame_info.frame_batch[i].color_temp = led_on_color;
           param->u.frame_info.frame_batch[i].ccm = output->snapshot_ccm;
         } else {
           param->u.frame_info.frame_batch[i].r_gain = led_off_gains.r_gain;
           param->u.frame_info.frame_batch[i].b_gain = led_off_gains.b_gain;
           param->u.frame_info.frame_batch[i].g_gain = led_off_gains.g_gain;
           param->u.frame_info.frame_batch[i].color_temp = led_off_color;
           param->u.frame_info.frame_batch[i].ccm = output->ccm;
         }
        }
      } else {
        AWB_ERR("awb get snapshot info failed: %d", rc);
        return FALSE;
      }
    } else {
      int32 i = 0;

      /* MWB uses same gains and CCM for Preview and MWB flash snapshot */
      for (i = 0; i < param->u.frame_info.num_batch; i++) {
         param->u.frame_info.frame_batch[i].r_gain = awb->curr_gains.r_gain;
         param->u.frame_info.frame_batch[i].b_gain = awb->curr_gains.b_gain;
         param->u.frame_info.frame_batch[i].g_gain = awb->curr_gains.g_gain;
         param->u.frame_info.frame_batch[i].color_temp = awb->color_temp;
         param->u.frame_info.frame_batch[i].ccm = awb->core_output.ccm;
       }
    }
  }
  break;

  case AWB_REQUIRED_STATS: {
    /* Request stats for AWB */
    if (Q3A_STATS_STREAM_OFFLINE == param->u.request_stats.stats_stream_type) {
      /* If it's offline stream, handle the stats request accordingly.
         Currently we'll use the same as that of online stats.*/
      AWB_LOW("Offline stats mask request.");
    }

    if (param->u.request_stats.supported_stats_mask & (1 << MSM_ISP_STATS_BG)) {
      param->u.request_stats.enable_stats_mask = (1 << MSM_ISP_STATS_BG);
    }

    /* Check if stats will be required to be configure by AWB */
    if (param->u.request_stats.supported_stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) {
      /* if AEC_BG stats enable, AEC will not configure BG stats. AWB will configure BG stats */
#ifdef _ENABLE_AEC_BG_STATS_
      awb->bg_config_stats_required = TRUE;
#else
      awb->bg_config_stats_required = FALSE;
#endif
    } else {
      awb->bg_config_stats_required = FALSE;
    }

    AWB_HIGH("AWB_REQUIRED_STATS: 0x%x, is AWB doing stats config: %s",
      (1 << MSM_ISP_STATS_BG),
      awb->bg_config_stats_required ? "TRUE" : "FALSE");

    get_param_info.param_type = AWB_GET_RGN_SKIP_PATTERN;
    if((awb->bg_config_stats_required == TRUE) &&
      (param->u.request_stats.supported_rgn_skip_mask & (1 << MSM_ISP_STATS_BG))) {
      /* AWB only interested in BG stats region skip pattern */
      get_param_info.u.supported_rgn_skip_mask |= STATS_BG;
    } else {
      get_param_info.u.supported_rgn_skip_mask &= ~STATS_BG;
    }
    if (awb->awb_algo_ops.get_parameters) {
      ret = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
    } else {
      ret = Q3A_CORE_RESULT_FAILED;
    }
    if (ret == Q3A_CORE_RESULT_SUCCESS) {
      param->u.request_stats.enable_rgn_skip_pattern =
        get_param_output.u.enable_rgn_skip_pattern;
    } else {
      AWB_ERR("awb get region skip pattern failed");
      rc = FALSE;
    }
  }
  break;

  default:
    rc = FALSE;
    break;
  }

  return rc;
}

static void awb_biz_set_debug_data_enable(awb_biz_t *awb)
{
  boolean exif_dbg_enable = FALSE;
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;

  exif_dbg_enable = (stats_exif_debug_mask & EXIF_DEBUG_MASK_AWB) ? TRUE : FALSE;

  AWB_LOW("AWB debug data exif mask:0x%x, AWB enabled:%d",
    stats_exif_debug_mask, exif_dbg_enable);

  /* Check whether debug data levels are set. */
  awb_debug_data_level_type debug_level = AWB_DEBUG_DATA_LEVEL_VERBOSE;
  if ((stats_debug_data_log_level & Q3A_DEBUG_DATA_LEVEL_CONCISE) > 0) {
    debug_level = AWB_DEBUG_DATA_LEVEL_CONCISE;
  }
  else if ((stats_debug_data_log_level & Q3A_DEBUG_DATA_LEVEL_VERBOSE) > 0) {
    debug_level = AWB_DEBUG_DATA_LEVEL_VERBOSE;
  }

  if (awb->exif_dbg_enable != exif_dbg_enable) {
    awb_set_param_info_type set_param_info;
    set_param_info.param_type = AWB_SET_PARAM_DEBUG_DATA_ENABLE;
    set_param_info.u.debug_data_enable = exif_dbg_enable;
    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("awb set param AWB_SET_PARAM_DEBUG_DATA_ENABLE failed with result %d", rc);
      return;
    }
    awb->exif_dbg_enable = exif_dbg_enable;
  }

  /* Set the EXIF debug level if it has changed and EXIF debugging is enabled. */
  if (awb->exif_dbg_level != debug_level && TRUE == awb->exif_dbg_enable) {
    awb_set_param_info_type set_param_info;

    /* Use AWB_SET_PARAM_DEBUG_DATA_LEVEL set param to set debug data level. */
    set_param_info.param_type = AWB_SET_PARAM_DEBUG_DATA_LEVEL;
    set_param_info.u.debug_data_level = debug_level;

    if (awb->awb_algo_ops.set_parameters) {
      rc = awb->awb_algo_ops.set_parameters(awb->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("awb set param AWB_SET_PARAM_DEBUG_DATA_LEVEL failed with result %d", rc);
      return;
    }

    awb->exif_dbg_level = debug_level;
  }
}

/** awb_restore_gains_ccm_after_flash:
*
**/
static void awb_restore_gains_ccm_after_flash(awb_biz_t *awb)
{
  /* Restore awb only in auto mode and its not locked. */
  if (awb->current_wb_type == AWB_AUTO && !awb->awb_locked) {

    awb->curr_gains = awb->awb_flash_off_settings.gains;
    awb->color_temp = awb->awb_flash_off_settings.color_temperature;

    if (awb->awb_ccm_enable) {
      awb->core_output.ccm = awb->awb_flash_off_settings.ccm;

      /* Update the ccm override during restoration */
      awb->core_output.ccm.override_ccm = TRUE;

      awb->core_output.snapshot_ccm = awb->core_output.ccm;
    }
  }
}

/** awb_initialize_post_flash_skip_count:
*
**/
static void awb_biz_reset_flash_info(awb_biz_t *awb)
{
  if (awb->use_led_estimation) {
    awb->use_led_estimation = FALSE;
    awb->led_frame_skip_cnt = AWB_LED_TO_OFF_SKIP_CNT;
    AWB_LOW("Led off skip %d frame", awb->led_frame_skip_cnt);

    /* Restored the gains and ccm after flash */
    awb_restore_gains_ccm_after_flash(awb);
  }
}

/** awb_biz_check_skip_needed:
*
**/
boolean  awb_biz_check_skip_needed(awb_biz_t *awb)
{
  int rc = FALSE;
  if (awb->manual_to_auto_skip_cnt > 0) {
   awb->manual_to_auto_skip_cnt--;
   AWB_HIGH("Manual to Auto switch skip for this frame, cnt %d",
      awb->manual_to_auto_skip_cnt);
    rc = TRUE;
  }

  if (awb->led_frame_skip_cnt > 0) {
    awb->led_frame_skip_cnt--;
    AWB_LOW("led off for this frame, cnt %d",
      awb->led_frame_skip_cnt);
    rc = TRUE;
  }

  return rc;
}

/**
 * awb_biz_pack_input: convert linux specific stats structure to
 * unified stats structure.
 *
 * @param awb_biz
 * @param stats
 * @param input
 */
static void awb_biz_pack_process_input(awb_biz_t *awb, stats_t *stats)
{
  if (!awb || !stats) {
    AWB_ERR("invalid input: %p,%p",awb,stats);
    return;
  }

  if (stats->stats_type_mask & STATS_BG){
    q3a_core_bg_stats_type *stats_input = &awb->core_input.bg_stats;
    q3a_bg_stats_t* bg_stats = stats->bayer_stats.p_q3a_bg_stats;
    if(bg_stats == NULL){
      AWB_ERR("BG stats buffer error: %p",bg_stats);
      return;
    }

    stats_input->frame_id = stats->frame_id;
    stats_input->array_length = MAX_BG_STATS_NUM;
    stats_input->num_horizontal_regions = bg_stats->bg_region_h_num;
    stats_input->num_vertical_regions = bg_stats->bg_region_v_num;
    stats_input->region_height = bg_stats->bg_region_height;
    stats_input->region_width = bg_stats->bg_region_width;
    stats_input->region_pixel_cnt = bg_stats->region_pixel_cnt;
    stats_input->r_max = bg_stats->rMax;
    stats_input->gr_max = bg_stats->grMax;
    stats_input->gb_max = bg_stats->gbMax;
    stats_input->b_max = bg_stats->bMax;
    stats_input->bit_depth = awb->stats_depth;
    stats_input->r_info.channel_sums = bg_stats->bg_r_sum;
    stats_input->r_info.channel_counts = bg_stats->bg_r_num;
    stats_input->r_info.channel_sat_sums = bg_stats->bg_r_sat_sum;
    stats_input->r_info.channel_sat_counts = bg_stats->bg_r_sat_num;
    stats_input->gr_info.channel_sums = bg_stats->bg_gr_sum;
    stats_input->gr_info.channel_counts = bg_stats->bg_gr_num;
    stats_input->gr_info.channel_sat_sums = bg_stats->bg_gr_sat_sum;
    stats_input->gr_info.channel_sat_counts = bg_stats->bg_gr_sat_num;
    stats_input->b_info.channel_sums = bg_stats->bg_b_sum;
    stats_input->b_info.channel_counts = bg_stats->bg_b_num;
    stats_input->b_info.channel_sat_sums = bg_stats->bg_b_sat_sum;
    stats_input->b_info.channel_sat_counts = bg_stats->bg_b_sat_num;
    stats_input->gb_info.channel_sums = bg_stats->bg_gb_sum;
    stats_input->gb_info.channel_counts = bg_stats->bg_gb_num;
    stats_input->gb_info.channel_sat_sums = bg_stats->bg_gb_sat_sum;
    stats_input->gb_info.channel_sat_counts = bg_stats->bg_gb_sat_num;
  } else {
    AWB_ERR("stats type is not valid");
    return;
  }
}

static void awb_biz_pack_process_output(awb_biz_t *awb)
{
  uint32 i = 0;

  if (!awb) {
    AWB_ERR("invalid input: %p",awb);
    return;
  }
  awb_output_params_type* core_output = &awb->core_output;

  awb->curr_gains = core_output->gains;
  awb->unadjusted_awb_gain = core_output->unadjust_gains;
  awb->color_temp = core_output->color_temperature;
  awb->decision = (int)map_decision_from_q3a_to_biz(core_output->decision);
  awb->is_awb_converge = core_output->is_converged;
  AWB_LOW("(r,g,b) gain = (%f, %f, %f), cct = %d, decision = %d",
    core_output->gains.r_gain, core_output->gains.g_gain,
    core_output->gains.b_gain, core_output->color_temperature,
    awb->decision);

  //dualcam sync info
  memcpy(&awb->dualcam_sync, &core_output->dualcam_sync, sizeof(awb_dualcam_sync_info_t));

  awb->snap_gains = core_output->snap_gains;
  awb->snap_color_temp = core_output->snap_color_temp;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  awb->dual_led_settings = core_output->dual_led_data.dual_led_settings;
  awb->dual_led_flux_gain = core_output->dual_led_data.dual_led_flux_gain;
#endif

  /* Save the Dual LED Calibration data if calibration is enabled */
  if (core_output->dual_led_data.dual_led_calib.is_calibration_mode)
    awb_biz_save_dual_led_calibration(&core_output->dual_led_data.dual_led_calib);

  /* Map decision from q3a to biz */
  for (i = 0; i < AWB_DECISION_MAP_SIZE; i++) {
    awb->sample_decision[i] =
      (int)map_decision_from_q3a_to_biz(core_output->decision_map.map[i]);
  }
}

/** awb_biz_process_offline_stats:
 * Process offline AWB stats.
 *
 * @param stats input AWB stats
 * @param obj internal AWB data structure
 * @param output output of AWB algorithm
**/
static void awb_biz_process_offline_stats(
  stats_t *stats, void *obj, awb_output_data_t *output)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_biz_t *awb = (awb_biz_t *)obj;
  AWB_LOW("AWB process offline stats!");
  awb_biz_pack_process_input(awb, stats);
  if (awb->awb_algo_ops.process) {
    rc = awb->awb_algo_ops.process(awb->handle,
        &awb->core_input, &awb->core_output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AWB_ERR("awb process failed");
    return;
  }

  awb_biz_pack_process_output(awb);

  /* pack awb output */
  awb_biz_pack_output(awb, output);
}

/** awb_biz_handle_manual_and_lock
 * Handle awb stats in manual mode or awb locked
 *
 * @param stats input AWB stats
 * @param awb internal AWB data structure
**/
static void awb_biz_handle_manual_and_lock(
  stats_t *stats, void *obj)
{
  awb_biz_t *awb = (awb_biz_t *)obj;
  /* Get dual led setting during MWB */
  awb_biz_get_dual_led_settings((void*)awb, &awb->curr_gains);

  /* Get awb-ccm info for MWB mode for every frame as EI can change which
   * might require CCM to be recomputed */
  if (FALSE == awb->flash_on) {
    awb_biz_estimate_ccm((void*)awb, awb->color_temp,
      &(awb->core_output.ccm), stats->camera_id);
    awb->core_output.snapshot_ccm = awb->core_output.ccm;
  }
}

/** awb_biz_process:
*
**/
static void awb_biz_process(stats_t *stats, void *obj,
  awb_output_data_t *output, uint8_t number_of_outputs)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_biz_t *awb = (awb_biz_t *)obj;

  AWB_LOW("awb->op_mode=%d, type=%d, lock=%d, streamType=%d", awb->op_mode,
    awb->current_wb_type, awb->awb_locked, stats->isp_stream_type);

  if (number_of_outputs != MAX_AWB_ALGO_OUTPUTS) {
    AWB_ERR("Error: invalid number of outputs requested: %u",
      number_of_outputs);
    return;
  }

  /* We'll handle offline processing separately */
  if (Q3A_STATS_STREAM_OFFLINE == stats->isp_stream_type) {
    /* we just need to process offline stats in auto mode and
     * awb is not locked. */
    if (awb->current_wb_type == AWB_AUTO && !awb->awb_locked) {
      awb_biz_process_offline_stats(stats, obj, output);
    } else {
      awb_biz_handle_manual_and_lock(stats,obj);
    }
    return;
  } else {
    if (0 == awb->op_mode)
    {
      AWB_ERR("In- correct state to process stats");
      return;
    }

    awb_biz_set_debug_data_enable(awb);
    /*eztuing enable awb return*/
    if(awb->awb_enable == TRUE)
    {
      awb_biz_pack_output(awb, output);
      return;
    }
    if(awb_biz_check_skip_needed(awb))
      return;

    awb->stats = *stats;

    /* In Flash ON, bright scene, flash gains are not computed and mode
     * changes to snapshot mode. To retieve the snapshot gains using get
     * parameters, needs many changes on port side, so currently runing the
     * AWB during snapshot mode to ensure that flash gains are computed. */
    if (Q3A_OPERATION_MODE_PREVIEW == awb->op_mode ||
      Q3A_OPERATION_MODE_CAMCORDER == awb->op_mode ||
      Q3A_OPERATION_MODE_SNAPSHOT == awb->op_mode) {

      if (awb->current_wb_type == AWB_AUTO && !awb->awb_locked){
        awb_biz_pack_process_input(awb, stats);
        if (awb->awb_algo_ops.process) {
          // Invoke the Algorithm
          rc = awb->awb_algo_ops.process(awb->handle, &awb->core_input, &awb->core_output);
        } else {
          rc = Q3A_CORE_RESULT_FAILED;
        }
        if (rc != Q3A_CORE_RESULT_SUCCESS) {
          AWB_ERR("awb_process_failed");
          return;
        }
        awb_biz_pack_process_output(awb);
      } else {
        awb_biz_handle_manual_and_lock(stats,obj);
      }
    } else {
      AWB_ERR("invalid awb op_mode: %d", awb->op_mode);
    }
  }
  return;
}

/**
 * awb_biz_get_cct_by_gains
 *
 * @param obj
 * @param r_gain
 * @param g_gain
 * @param b_gain
 * @param cct
 *
 * Return: TRUE on success
 */
static boolean awb_biz_estimate_cct_by_gains (void *obj,
  float r_gain, float g_gain, float b_gain, float *cct, uint8_t camera_id)
{
  awb_biz_t *awb = (awb_biz_t *)obj;
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  (void)camera_id;

  awb_get_param_info_type get_param_info;
  get_param_info.param_type = AWB_GET_PARAM_CCT_FROM_RGB;
  get_param_info.u.gains.r_gain = r_gain;
  get_param_info.u.gains.g_gain = g_gain;
  get_param_info.u.gains.b_gain = b_gain;
  awb_get_param_output_type get_param_output;

  if (awb->awb_algo_ops.get_parameters) {
    rc = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AWB_ERR("awb get param failed");
    return FALSE;
  }
  *cct = get_param_output.u.color_temperature;
  return TRUE;
}

/* awb_biz_estimate_gains_by_cct
*
* Return: TRUE on success
*/
static boolean awb_biz_estimate_gains_by_cct(void *obj,
  float *r_gain, float* g_gain, float* b_gain, float cct, uint8_t camera_id)
{
  awb_biz_t *awb = (awb_biz_t *)obj;
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  (void)camera_id;

  awb_get_param_info_type get_param_info;
  get_param_info.param_type = AWB_GET_PARAM_RGB_FROM_CCT;
  get_param_info.u.color_temperature = (uint32)cct;
  awb_get_param_output_type get_param_output;
  if (awb->awb_algo_ops.get_parameters) {
    rc = awb->awb_algo_ops.get_parameters(awb->handle, &get_param_info, &get_param_output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
    AWB_ERR("awb get param failed");
    return FALSE;
  }

  *r_gain = get_param_output.u.gains.r_gain;
  *g_gain = get_param_output.u.gains.g_gain;
  *b_gain = get_param_output.u.gains.b_gain;
  return TRUE;
}

/**
 * awb_biz_clear_iface_ops
 *
 * @awb_object: structure with function pointers to be assign
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void awb_biz_clear_iface_ops(awb_ops_t *awb_ops)
{
  awb_ops->set_parameters = NULL;
  awb_ops->get_parameters = NULL;
  awb_ops->process = NULL;
  awb_ops->init = NULL;
  awb_ops->deinit = NULL;
  awb_ops->estimate_cct = NULL;
  awb_ops->estimate_gains = NULL;
  awb_ops->estimate_ccm = NULL;
  return;
}

/**
 * awb_biz_clear_algo_ops
 *
 * @awb_algo_ops: structure with function pointers to algo lib
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void awb_biz_clear_algo_ops(awb_biz_algo_ops_t *awb_algo_ops)
{
  awb_algo_ops->set_parameters = NULL;
  awb_algo_ops->get_parameters = NULL;
  awb_algo_ops->process = NULL;
  awb_algo_ops->init = NULL;
  awb_algo_ops->deinit = NULL;
  awb_algo_ops->set_log = NULL;
  return;
}

/** awb_biz_dlsym
 *
 *    @lib_handler: Handler to library
 *    @fn_ptr: Function to initialize
 *    @fn_name: Name of symbol to find in library
 *
 * Return: TRUE on success
 **/
static boolean awb_biz_dlsym(void *lib_handler, void *fn_ptr,
  const char *fn_name)
{
  char *error = NULL;

  if (NULL == lib_handler || NULL == fn_ptr) {
    AWB_ERR("Error Loading %s", fn_name);
    return FALSE;
  }

  *(void **)(fn_ptr) = dlsym(lib_handler, fn_name);
  if (!fn_ptr) {
    error = (char *)dlerror();
    AWB_ERR("Error: %s", error);
    return FALSE;
  }

  AWB_LOW("Loaded %s %p", fn_name, fn_ptr);
  return TRUE;
}

/**
 * awb_biz_init
 *
 * @param awb_ops
 *
 * @return void*
 */
void *awb_biz_init(void *libptr)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  awb_biz_t  *awb_biz = NULL;

  do {
    awb_biz = (awb_biz_t *)calloc(1, sizeof(awb_biz_t));
    if (NULL == awb_biz) {
      AWB_ERR("malloc failed");
      return NULL;
    }

    awb_biz->first_init = TRUE;

    dlerror(); /* Clear previous errors */
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.set_parameters,
      "awb_set_param")) {
      break;
    }
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.get_parameters,
      "awb_get_param")) {
      break;
    }
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.process,
      "awb_process")) {
      break;
    }
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.init,
      "awb_init")) {
      break;
    }
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.deinit,
      "awb_deinit")) {
      break;
    }
    if (!awb_biz_dlsym(libptr, &awb_biz->awb_algo_ops.set_log,
      "q3a_core_set_log_level")) {
      break;
    }
    /* set logging for core */
    awb_biz->awb_algo_ops.set_log();

    rc = awb_biz->awb_algo_ops.init(&awb_biz->handle);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AWB_ERR("awb core init failed");
      break;
    }

    return awb_biz;
  } while (0);

  awb_biz_clear_algo_ops(&awb_biz->awb_algo_ops);

  /* Handling error */
  if (awb_biz) {
    free(awb_biz);
    awb_biz = NULL;
  }
  return NULL;
}

/**
 * awb_biz_deinit
 *
 * @param awb_obj
 */
void awb_biz_deinit(void *awb_obj)
{
  awb_biz_t *awb_biz = (awb_biz_t*)awb_obj;

  if (awb_biz) {
    if (awb_biz->handle && awb_biz->awb_algo_ops.deinit) {
      awb_biz->awb_algo_ops.deinit(awb_biz->handle);
    }
    awb_biz_clear_algo_ops(&awb_biz->awb_algo_ops);
    free(awb_biz);
    awb_biz = NULL;
  }
}

/**
 * awb_biz_load_function
 *
 * @awb_ops: structure with function pointers to be assign
 *
 * Return: Handler to AWB interface library
 **/
void * awb_biz_load_function(awb_ops_t *awb_ops)
{
  void *q3a_handler = NULL;

  if (!awb_ops) {
    return NULL;
  }

  q3a_handler = dlopen("libmmcamera2_q3a_core.so", RTLD_NOW);
  if (!q3a_handler) {
    AWB_ERR("dlerror: %s", dlerror());
    return NULL;
  }

  if (awb_ops) {
    awb_ops->set_parameters = awb_biz_set_params;
    awb_ops->get_parameters = awb_biz_get_params;
    awb_ops->process = awb_biz_process;
    awb_ops->estimate_cct = awb_biz_estimate_cct_by_gains;
    awb_ops->estimate_gains = awb_biz_estimate_gains_by_cct;
    awb_ops->estimate_ccm = awb_biz_estimate_ccm;
    awb_ops->init = awb_biz_init;
    awb_ops->deinit = awb_biz_deinit;
  }

  return q3a_handler;
}

/**
 * awb_biz_unload_function
 *
 * @awb_object: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void awb_biz_unload_function(awb_ops_t *awb_ops, void *lib_handler)
{
  if (lib_handler) {
    dlclose(lib_handler);
  }
  awb_biz_clear_iface_ops(awb_ops);

  return;
}
