/* eeprom_util.c
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <math.h>
#include <stdlib.h>
#include "eeprom_lib.h"
#include "eeprom.h"
#include "debug_lib.h"

#define MIN(x, y) (((x) > (y)) ? (y):(x))
#define MAX(x, y) (((x) > (y)) ? (x):(y))
#define CUBIC_F(fs, fc0, fc1, fc2, fc3) {\
  float fs3, fs2;\
  fs2 = fs * fs; \
  fs3 = fs * fs2; \
  fc0 = (float)(0.5 * (-fs3 + (2.0 * fs2) - fs)); \
  fc1 = (float)(0.5 * ((3.0 * fs3) - (5.0 * fs2) + 2.0)); \
  fc2 = (float)(0.5 * ((-3.0 * fs3) + (4.0 * fs2) + fs)); \
  fc3 = (float)(0.5 * (fs3 - fs2)); \
}

#define MESH_ROLLOFF_HORIZONTAL_GRIDS_B  12
#define MESH_ROLLOFF_VERTICAL_GRIDS_B    9

static void swap(pixel_t *m, pixel_t *n)
{
  pixel_t temp;
  temp.x = m->x;
  temp.y = m->y;
  *m = *n;
  n->x = temp.x;
  n->y = temp.y;
}

/** adjust_step_table:
 *    @e_ctrl: address of pointer to
 *                   step_size_t struct
 *             adjusting ratio
 *
 * This function adjust TAF_table and CAF_table
 *
 * Return:
 * void
 **/
void adjust_step_table(__attribute__((unused)) const char *str,
  step_size_t *light, float ratio) {
  SLOW("%s: before adjust %s scan step: %d %d %d %d %d", __func__, str,
  light->rgn_0, light->rgn_1, light->rgn_2, light->rgn_3, light->rgn_4);
  light->rgn_0 =(unsigned short)round(light->rgn_0 * ratio);
  light->rgn_1 =(unsigned short)round(light->rgn_1 * ratio);
  light->rgn_2 =(unsigned short)round(light->rgn_2 * ratio);
  light->rgn_3 =(unsigned short)round(light->rgn_3 * ratio);
  light->rgn_4 =(unsigned short)round(light->rgn_4 * ratio);
  SLOW("%s: after adjust %s scan step: %d %d %d %d %d", __func__, str,
  light->rgn_0, light->rgn_1, light->rgn_2, light->rgn_3, light->rgn_4);
}

/** eeprom_autofocus_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs autofocus calibration assuming 2 regions
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

int eeprom_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  actuator_tuned_params_t *af_driver_tune = NULL;
  uint32_t                total_steps = 0;
  short                   macro_dac, infinity_dac, code_per_step = 0;
  uint32_t                qvalue = 0;

  /* Validate params */
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count = 0");
    return FALSE;
  }

  /* Get the calibrated steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
      step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  macro_dac = ectrl->eeprom_data.afc.macro_dac;
  infinity_dac = ectrl->eeprom_data.afc.infinity_dac;
  qvalue = af_driver_tune->region_params[0].qvalue;

  if(qvalue >= 1 && qvalue <= 4096) {
    code_per_step =
     (macro_dac - infinity_dac) / (float)total_steps * qvalue;
    af_driver_tune->region_params[0].code_per_step = code_per_step;
  }

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = ectrl->eeprom_data.afc.infinity_dac;

  SLOW("With Margin: AF : macro %d, infinity %d, total steps: %d,"
    "code_per_step: %d, qvalue: %d", macro_dac, infinity_dac, total_steps,
    (short)af_driver_tune->region_params[0].code_per_step, qvalue);

  return TRUE;
}

/** wbgain_calibration:
 *    @gain: gain of the whitebalance
 *    @calib_factor: calibration factor for whhitebalance
 *
 * Helper function for whitebalace calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

float wbgain_calibration(float gain, float calib_factor)
{
  /* gain = 1 / [(1/gain) * calibration_r_over_g_factor] */
  return gain / calib_factor;
}

void eeprom_whitebalance_green_calibration(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl;
  wbcalib_data_t       *wbc;
  chromatix_parms_type *chromatix_isp;
  float                 r_gain, b_gain, g_gain, min_gain, gr_over_gb;;

  ectrl = (sensor_eeprom_data_t *)e_ctrl;
  RETURN_VOID_ON_NULL(ectrl);

  wbc = &(ectrl->eeprom_data.wbc);

  chromatix_isp =
    ectrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_ISP];
  if (!chromatix_isp) {
    SERR("NULL pointer : chromatix isp");
    return;
  }

  /* Gr/Gb channel balance */
  gr_over_gb = wbc->gr_over_gb;
  /* To compensate invert the measured gains */
  b_gain = gr_over_gb;
  r_gain = 1;
  SLOW("%s: gb_gain: %f, gr_gain: %f", __func__, b_gain, r_gain);
  min_gain = MIN(r_gain, b_gain);
  SLOW("%s: min_gain: %f", __func__, min_gain);

  chromatix_isp->chromatix_VFE.chromatix_channel_balance_gains.green_even
    = r_gain/min_gain;
  chromatix_isp->chromatix_VFE.chromatix_channel_balance_gains.green_odd
    = b_gain/min_gain;
}


#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
/** eeprom_whitebalance_extr_calibration:
*    @e_ctrl: address of pointer to
*                   sensor_eeprom_data_t struct
*
* performs extreme detection zone calibration
*
* This function executes in module sensor context
*
* Return:
* void
**/
void eeprom_whitebalance_extr_calibration(void *e_ctrl)
{
    uint32_t                       indx = 0;
    float                          r_over_g_calib_factor;
    float                          b_over_g_calib_factor;
    sensor_eeprom_data_t           *ectrl = (sensor_eeprom_data_t *)e_ctrl;
    wbcalib_data_t                 *wbc = &(ectrl->eeprom_data.wbc);
    awb_extreme_cct_type              *awb_extr = NULL;
    chromatix_3a_parms_type        *chromatix = ectrl->eeprom_afchroma.chromatix.
        chromatix_ptr[SENSOR_CHROMATIX_3A];
    awb_golden_module_info_type      awb_golden = chromatix->AWB_bayer_algo_data.
        awb_golden_module_info;
    chromatix_awb_all_light_type     light_type = AGW_AWB_D50;

    awb_extr = &(chromatix->AWB_bayer_algo_data.awb_extreme_cct_control);

    for (indx = 0; indx < EXTR_CCT_EXTRA_ZONE; indx++) {
        switch (awb_extr->awb_extr_high_cct_aux_zone_detect[indx].calibration_index) {
        case 1:/*TL84*/
            light_type = AGW_AWB_WARM_FLO;
            break;
        case 2:/*A*/
            light_type = AGW_AWB_A;
            break;
        default:/*for 0 or any other value use  - D50*/
            light_type = AGW_AWB_D50;
            break;
        }

        r_over_g_calib_factor = wbc->r_over_g[light_type] /
            awb_golden.AWB_golden_module_R_Gr_ratio[light_type];
        b_over_g_calib_factor = wbc->b_over_g[light_type] /
            awb_golden.AWB_golden_module_B_Gr_ratio[light_type];

        awb_extr->awb_extr_high_cct_aux_zone_detect[indx].rg_center
            *= r_over_g_calib_factor;
        awb_extr->awb_extr_high_cct_aux_zone_detect[indx].bg_center
            *= r_over_g_calib_factor;

    }

    return;
}


/** eeprom_whitebalance_pf_calibration:
*    @e_ctrl: address of pointer to
*                   sensor_eeprom_data_t struct
*
* performs whitebalace preflash calibration
*
* This function executes in module sensor context
*
* Return:
* void
**/
void eeprom_whitebalance_pf_calibration(void *e_ctrl)
{
    uint32_t                       i = 0, j = 0;
    float                          r_over_g_calib_factor;
    float                          b_over_g_calib_factor;
    sensor_eeprom_data_t           *ectrl = (sensor_eeprom_data_t *)e_ctrl;
    wbcalib_data_t                 *wbc = &(ectrl->eeprom_data.wbc);
    awb_preflash_type              *awb_pf = NULL;
    awb_preflash_enhance_type      *awb_pf_eh = NULL;
    chromatix_3a_parms_type        *chromatix = ectrl->eeprom_afchroma.chromatix.
        chromatix_ptr[SENSOR_CHROMATIX_3A];

    awb_pf = &(chromatix->AWB_bayer_algo_data.awb_preFlash_param);
    awb_pf_eh = &(chromatix->AWB_bayer_algo_data.awb_preflash_enhance_control);

    r_over_g_calib_factor = wbc->r_over_g[AGW_AWB_D50] /
        chromatix->AWB_bayer_algo_data.awb_golden_module_info.
        AWB_golden_module_R_Gr_ratio[AGW_AWB_D50];
    b_over_g_calib_factor = wbc->b_over_g[AGW_AWB_D50] /
        chromatix->AWB_bayer_algo_data.awb_golden_module_info.
        AWB_golden_module_B_Gr_ratio[AGW_AWB_D50];

    if (awb_pf->awb_pre_flash_enable) {
        /* preflash ref point */
        awb_pf->awb_mainflash_typ_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf->awb_mainflash_typ_ref.BG_ratio
            *= b_over_g_calib_factor;
        awb_pf->awb_mainflash_min_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf->awb_mainflash_min_ref.BG_ratio
            *= b_over_g_calib_factor;
        awb_pf->awb_mainflash_max_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf->awb_mainflash_max_ref.BG_ratio
            *= b_over_g_calib_factor;
        awb_pf->awb_preFlash_min_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf->awb_preFlash_min_ref.BG_ratio
            *= b_over_g_calib_factor;
    }

    if (awb_pf_eh->awb_preflash_enhancement_enable) {
        awb_pf_eh->awb_preFlash_min_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf_eh->awb_preFlash_min_ref.BG_ratio
            *= b_over_g_calib_factor;
        awb_pf_eh->awb_preFlash_max_ref.RG_ratio
            *= r_over_g_calib_factor;
        awb_pf_eh->awb_preFlash_max_ref.BG_ratio
            *= b_over_g_calib_factor;

        for (i = 0; i < PREFLASH_ENHANCE_HEURISTIC_NUM; i++) {
            for (j = 0; j < PREFLASH_DETECT_NUM; j++) {
                awb_pf_eh->preflash_heuristic[i].detection_zone[j].rg_center
                    *= r_over_g_calib_factor;
                awb_pf_eh->preflash_heuristic[i].detection_zone[j].bg_center
                    *= b_over_g_calib_factor;
            }
        }
    }

    return;
}
/** eeprom_whitebalance_store_calibration_factor:
*    @e_ctrl: address of pointer to
*                   sensor_eeprom_data_t struct
*
* Store the calibration factor in reserve params in chromatix header
* The stored calibration factor will be used in AWB
*
* This function executes in module sensor context
*
* Return:
* void
**/
void eeprom_whitebalance_store_calibration_factor(void *e_ctrl)
{
    uint32_t                       indx = 0;
    float                          r_over_g_calib_factor;
    float                          b_over_g_calib_factor;
    sensor_eeprom_data_t           *ectrl = (sensor_eeprom_data_t *)e_ctrl;
    wbcalib_data_t                 *wbc = &(ectrl->eeprom_data.wbc);
    awb_reserved_data_type         *awb_reserved = NULL;
    chromatix_3a_parms_type        *chromatix = ectrl->eeprom_afchroma.chromatix.
        chromatix_ptr[SENSOR_CHROMATIX_3A];
    awb_golden_module_info_type      awb_golden = chromatix->AWB_bayer_algo_data.
        awb_golden_module_info;
    chromatix_awb_all_light_type     light_type = AGW_AWB_D50;
    uint32_t                        start_offset = AWB_RESERVED_PARAM-(AGW_AWB_MAX_LIGHT*2);

    awb_reserved = &(chromatix->AWB_bayer_algo_data.awb_reserved_data);

    for(indx = 0 ; indx < AGW_AWB_MAX_LIGHT ; indx++)
    {
        r_over_g_calib_factor = wbc->r_over_g[indx] /
            awb_golden.AWB_golden_module_R_Gr_ratio[indx];
        b_over_g_calib_factor = wbc->b_over_g[indx] /
            awb_golden.AWB_golden_module_B_Gr_ratio[indx];

        awb_reserved->reserved_float[start_offset++] = r_over_g_calib_factor;
        awb_reserved->reserved_float[start_offset++] = b_over_g_calib_factor;
    }

    return;
}
#endif


/** eeprom_whitebalance_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_whitebalance_calibration(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");

  RETURN_VOID_ON_NULL(ectrl);

  chromatix_3a_parms_type *chromatix =
    ectrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A];

  wbcalib_data_t *wbc = &(ectrl->eeprom_data.wbc);
  float r_gain, b_gain, g_gain, min_gain, gr_over_gb;
  uint32_t indx;
  float r_over_g_calib_factor[AGW_AWB_MAX_LIGHT];
  float b_over_g_calib_factor[AGW_AWB_MAX_LIGHT];


  /* Calibrate the AWB in chromatix based on color measurement read */
  for (indx = 0; indx < AGW_AWB_MAX_LIGHT; indx++) {

    r_over_g_calib_factor[indx] = wbc->r_over_g[indx] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.AWB_golden_module_R_Gr_ratio[indx];
    b_over_g_calib_factor[indx] = wbc->b_over_g[indx] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.AWB_golden_module_B_Gr_ratio[indx];

    chromatix->AWB_bayer_algo_data.awb_basic_tuning.reference[indx].RG_ratio *=
      r_over_g_calib_factor[indx];
    chromatix->AWB_bayer_algo_data.awb_basic_tuning.reference[indx].BG_ratio *=
      b_over_g_calib_factor[indx];

    ectrl->eeprom_wbc_factor.r_over_g[indx] = r_over_g_calib_factor[indx];
    ectrl->eeprom_wbc_factor.b_over_g[indx] = b_over_g_calib_factor[indx];
  }

  /* Calibrate the MWB in chromatix based on color measurement read */
  /* MWB TL84 */
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.r_gain,
    r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.b_gain,
    b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.r_gain = r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.g_gain = g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.b_gain = b_gain / min_gain;

  /* MWB D50 */
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.r_gain,
    r_over_g_calib_factor[AGW_AWB_D50]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.b_gain,
    b_over_g_calib_factor[AGW_AWB_D50]);
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.r_gain = r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.g_gain = g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.b_gain = b_gain / min_gain;

  /* MWB Incandescent */
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.r_gain,
    r_over_g_calib_factor[AGW_AWB_A]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.b_gain,
    b_over_g_calib_factor[AGW_AWB_A]);
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.r_gain =
    r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.g_gain =
    g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.b_gain =
    b_gain / min_gain;

  /* MWB D65 */
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.r_gain,
     r_over_g_calib_factor[AGW_AWB_D65]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.b_gain,
     b_over_g_calib_factor[AGW_AWB_D65]);
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.r_gain = r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.g_gain = g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.b_gain = b_gain / min_gain;

  /* MWB Strobe */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.r_gain,
     r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.b_gain,
     b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
#else
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.r_gain,
     r_over_g_calib_factor[AGW_AWB_D50]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.b_gain,
     b_over_g_calib_factor[AGW_AWB_D50]);
#endif
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.r_gain =
   r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.g_gain =
    g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.b_gain =
    b_gain / min_gain;

  /* MWB LED flash */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.r_gain,
    r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.b_gain,
   b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
#else
  r_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.r_gain,
    r_over_g_calib_factor[AGW_AWB_D50]);
  b_gain = wbgain_calibration(
    chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.b_gain,
   b_over_g_calib_factor[AGW_AWB_D50]);
#endif
  g_gain = chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.r_gain =
    r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.g_gain =
   g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.b_gain =
   b_gain / min_gain;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  /* min/max gain */
  if (chromatix->AWB_bayer_algo_data.awb_front_camera_control.awb_cap_gain_enable) {
    chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_min_gains.r_gain =
      wbgain_calibration(
        chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_min_gains.r_gain,
        r_over_g_calib_factor[AGW_AWB_D50]);
    chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_min_gains.b_gain =
      wbgain_calibration(
        chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_min_gains.b_gain,
        b_over_g_calib_factor[AGW_AWB_D50]);
    chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_max_gains.r_gain =
      wbgain_calibration(
        chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_max_gains.r_gain,
        r_over_g_calib_factor[AGW_AWB_D50]);
    chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_max_gains.b_gain =
      wbgain_calibration(
        chromatix->AWB_bayer_algo_data.awb_basic_tuning.awb_max_gains.b_gain,
        b_over_g_calib_factor[AGW_AWB_D50]);
  }

  /* initial awb gain */
  r_gain =
    wbgain_calibration(chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.r_gain,
      r_over_g_calib_factor[AGW_AWB_D50]);
  b_gain =
    wbgain_calibration(chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.b_gain,
      b_over_g_calib_factor[AGW_AWB_D50]);
  g_gain = chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.r_gain =
    r_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.g_gain =
    g_gain / min_gain;
  chromatix->AWB_bayer_algo_data.awb_flow_control.initial_gain.b_gain =
    b_gain / min_gain;
#endif

  /*WB misleading color zone(mlc) calibration */
  eeprom_whitebalance_mlczone_calibration(e_ctrl);

  /*WB extended outdoor heuristic(eoh) calibration*/
  eeprom_whitebalance_eoh_calibration(e_ctrl);

  /*WB single color tracking(sct) calibration*/
  eeprom_whitebalance_sct_calibration(e_ctrl);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  /*WB extreme heuristic calibration*/
  eeprom_whitebalance_extr_calibration(e_ctrl);

  /*WB preflash(pf) calibration*/
  eeprom_whitebalance_pf_calibration(e_ctrl);

  /*Store calibration factor*/
  eeprom_whitebalance_store_calibration_factor(e_ctrl);
#endif

  SDBG("Exit");

  return;
}

/** eeprom_whitebalance_mlczone_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace misleading color zone calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void eeprom_whitebalance_mlczone_calibration(void *e_ctrl)
{
  sensor_eeprom_data_t             *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  chromatix_3a_parms_type          *chromatix =
           ectrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A];
  wbcalib_data_t                   *wbc = &(ectrl->eeprom_data.wbc);
  awb_misleading_zone_process_type *awb_mlc =
                 &(chromatix->AWB_bayer_algo_data.awb_misleading_color_zone);
  awb_reserved_data_type           *awb_reserv =
                 &(chromatix->AWB_bayer_algo_data.awb_reserved_data);
  awb_golden_module_info_type      *awb_golden =
                 &(chromatix->AWB_bayer_algo_data.awb_golden_module_info);
  uint32_t                         indx, indx2;
  float                            r_over_g_calib_factor;
  float                            b_over_g_calib_factor;
  chromatix_awb_all_light_type     light_type = AGW_AWB_D50;

  /* Calibrate the AWB mlczone in chromatix based on reserved data values */
  for (indx = 0; indx < MISLEADING_COLOR_ZONE_NUM; indx++) {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
    switch(awb_reserv->reserved_int[indx + 20]) {
#else
    switch (awb_mlc->awb_misleading_color_zones[indx].detect_zone.calibration_index) {
#endif
      case 1:/*TL84*/
        light_type = AGW_AWB_WARM_FLO;
        break;
      case 2:/*A*/
       light_type = AGW_AWB_A;
       break;
      default:/*for 0 or any other value use  - D50*/
       light_type = AGW_AWB_D50;
       break;
    }

    r_over_g_calib_factor = wbc->r_over_g[light_type] /
      awb_golden->AWB_golden_module_R_Gr_ratio[light_type];
    b_over_g_calib_factor = wbc->b_over_g[light_type] /
      awb_golden->AWB_golden_module_B_Gr_ratio[light_type];

    awb_mlc->awb_misleading_color_zones[indx].detect_zone.
      rg_center *= r_over_g_calib_factor;
    awb_mlc->awb_misleading_color_zones[indx].detect_zone.
      bg_center *= b_over_g_calib_factor;
  }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  for (indx = 0; indx < ML_SUM_CTRL_NUM; indx++)  {
    for (indx2 = 0; indx2 < ML_SUM_AUXZONE_NUM; indx2++) {
      switch (awb_mlc->awb_misleading_sum_control_zones[indx].
        awb_misleading_sum_aux_zones[indx2].calibration_index) {
        case 1:
          light_type =AGW_AWB_WARM_FLO;
          break;
        case 2:
          light_type = AGW_AWB_A;
          break;
        default:
          light_type = AGW_AWB_D50;
          break;
      }

      r_over_g_calib_factor = wbc->r_over_g[light_type] /
      awb_golden->AWB_golden_module_R_Gr_ratio[light_type];
      b_over_g_calib_factor = wbc->b_over_g[light_type] /
      awb_golden->AWB_golden_module_B_Gr_ratio[light_type];

      awb_mlc->awb_misleading_sum_control_zones[indx].
        awb_misleading_sum_aux_zones[indx2].rg_center
        *=  r_over_g_calib_factor;
      awb_mlc->awb_misleading_sum_control_zones[indx].
        awb_misleading_sum_aux_zones[indx2].bg_center
        *=  b_over_g_calib_factor;
    }
  }
#endif
  return;
}

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
/** eeprom_eoh_light_type_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace extended outdoor heuristic calibration based on light type
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
 void eeprom_eoh_light_type_calibration(void *e_ctrl,
  awb_outdoor_special_type select_function)
{
  uint32_t indx;
  awb_extended_outdoor_heuristic_type   *awb_eoh = NULL;
  sensor_eeprom_data_t                  *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  wbcalib_data_t                        *wbc = &(ectrl->eeprom_data.wbc);
  chromatix_3a_parms_type               *chromatix = ectrl->eeprom_afchroma.
                                            chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A];
  awb_golden_module_info_type           awb_golden = chromatix->AWB_bayer_algo_data.
                                                awb_golden_module_info;
  float                                 r_over_g_calib_factor;
  float                                 b_over_g_calib_factor;
  chromatix_awb_all_light_type          light_type = AGW_AWB_D50;

  awb_eoh = &(chromatix->AWB_bayer_algo_data.awb_extended_outdoor_heuristic);

  for (indx =0; indx < DETECT_ZONE_NUM; indx ++) {
    switch (select_function.detection_zone[indx].calibration_index) {
      case 1:/*TL84*/
        light_type = AGW_AWB_WARM_FLO;
        break;
      case 2:/*A*/
       light_type = AGW_AWB_A;
       break;
      default:/*for 0 or any other value use  - D50*/
       light_type = AGW_AWB_D50;
       break;
    }

    r_over_g_calib_factor = wbc->r_over_g[light_type] /
      awb_golden.AWB_golden_module_R_Gr_ratio[light_type];
      b_over_g_calib_factor = wbc->b_over_g[light_type] /
      awb_golden.AWB_golden_module_B_Gr_ratio[light_type];

    select_function.detection_zone[indx].rg_center
      *= r_over_g_calib_factor;
    select_function.detection_zone[indx].bg_center
      *= b_over_g_calib_factor;
  }

  return;
}
#endif

/** eeprom_whitebalance_eoh_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace extended outdoor heuristic calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void eeprom_whitebalance_eoh_calibration(void *e_ctrl)
{
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  uint32_t                             index;
  float                                r_over_g_calib_factor;
  float                                b_over_g_calib_factor;
  sensor_eeprom_data_t                 *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  wbcalib_data_t                       *wbc = &(ectrl->eeprom_data.wbc);
  awb_extended_outdoor_heuristic_type  *awb_eoh = NULL;
  chromatix_3a_parms_type              *chromatix = ectrl->eeprom_afchroma.
                                chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A];

  awb_eoh = &(chromatix->AWB_bayer_algo_data.awb_extended_outdoor_heuristic);
  r_over_g_calib_factor = wbc->r_over_g[AGW_AWB_D50] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.
      AWB_golden_module_R_Gr_ratio[AGW_AWB_D50];
  b_over_g_calib_factor = wbc->b_over_g[AGW_AWB_D50] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.
      AWB_golden_module_B_Gr_ratio[AGW_AWB_D50];

  for (index =0; index < DETECT_ZONE_NUM; index ++) {
    awb_eoh->awb_outdoor_green_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_green_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_bright_blue_sky_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_bright_blue_sky_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_not_enough_stat_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_not_enough_stat_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_blue_ground_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_blue_ground_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_cloud_sky_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_cloud_sky_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_blue_sky_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_blue_sky_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_fog_sky_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_fog_sky_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
    awb_eoh->awb_outdoor_green_detect_adjust.detection_zone[index].
        rg_center *= r_over_g_calib_factor;
    awb_eoh->awb_outdoor_green_detect_adjust.detection_zone[index].
        bg_center *= b_over_g_calib_factor;
  }
#else
  sensor_eeprom_data_t                 *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  awb_extended_outdoor_heuristic_type  *awb_eoh = NULL;
  chromatix_3a_parms_type              *chromatix = ectrl->eeprom_afchroma.
                                chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A];

  awb_eoh = &(chromatix->AWB_bayer_algo_data.awb_extended_outdoor_heuristic);

  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_green_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_bright_blue_sky_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_not_enough_stat_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_blue_ground_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_cloud_sky_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_blue_sky_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_fog_sky_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_green_detect_adjust);
  eeprom_eoh_light_type_calibration(e_ctrl, awb_eoh->awb_outdoor_artificial_grass_adjust);
#endif
  return;
}

/** eeprom_whitebalance_sct_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace single color tracking calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void eeprom_whitebalance_sct_calibration(void *e_ctrl)
{
  uint32_t                       i = 0, j = 0;
  float                          r_over_g_calib_factor;
  float                          b_over_g_calib_factor;
  sensor_eeprom_data_t           *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  wbcalib_data_t                 *wbc = &(ectrl->eeprom_data.wbc);
  awb_single_color_tracking_type *awb_sct = NULL;
  chromatix_3a_parms_type        *chromatix = ectrl->eeprom_afchroma.chromatix.
                                     chromatix_ptr[SENSOR_CHROMATIX_3A];
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)

  awb_sct = &(chromatix->AWB_bayer_algo_data.awb_single_color_tracking);
  r_over_g_calib_factor = wbc->r_over_g[AGW_AWB_D50] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.
      AWB_golden_module_R_Gr_ratio[AGW_AWB_D50];
  b_over_g_calib_factor = wbc->b_over_g[AGW_AWB_D50] /
      chromatix->AWB_bayer_algo_data.awb_golden_module_info.
      AWB_golden_module_B_Gr_ratio[AGW_AWB_D50];

  for (i = 0; i < NUM_SPECIAL_COLOR_DETECT; i++ ) {
    for (j = 0; j < DETECT_ZONE_NUM; j++) {
      awb_sct->awb_single_color_detect[i].detection_zone[j].
        rg_center *= r_over_g_calib_factor;
      awb_sct->awb_single_color_detect[i].detection_zone[j].
        bg_center *= b_over_g_calib_factor;
    }
  }
#else //310
  chromatix_awb_all_light_type     light_type = AGW_AWB_D50;
  awb_golden_module_info_type      awb_golden = chromatix->AWB_bayer_algo_data.
                                                awb_golden_module_info;

  awb_sct = &(chromatix->AWB_bayer_algo_data.awb_single_color_tracking);

  for (i = 0; i < NUM_SPECIAL_COLOR_DETECT; i++ ) {
    for (j = 0; j < DETECT_ZONE_NUM; j++) {
      switch (awb_sct->awb_single_color_detect[i].
        detection_zone[j].calibration_index) {
        case 1:/*TL84*/
          light_type = AGW_AWB_WARM_FLO;
          break;
        case 2:/*A*/
          light_type = AGW_AWB_A;
          break;
        default:/*for 0 or any other value use  - D50*/
          light_type = AGW_AWB_D50;
          break;
      }

      r_over_g_calib_factor = wbc->r_over_g[light_type] /
      awb_golden.AWB_golden_module_R_Gr_ratio[light_type];
      b_over_g_calib_factor = wbc->b_over_g[light_type] /
      awb_golden.AWB_golden_module_B_Gr_ratio[light_type];

      awb_sct->awb_single_color_detect[i].detection_zone[j].
        rg_center *= r_over_g_calib_factor;
      awb_sct->awb_single_color_detect[i].detection_zone[j].
        bg_center *= b_over_g_calib_factor;
    }
  }
#endif
  return;
}



/** eeprom_print_matrix:
 *    @paramlist: address of pointer to
 *                   chromatix struct
 *
 * Prints out debug logs
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void eeprom_print_matrix(float *paramlist __attribute__((unused)))
{
  int32_t j =0;

  for(j=0; j < MESH_ROLLOFF_SIZE; j = j+17) {
    SLOW("%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, "
      "%.1f, %.1f, %.1f, %.1f, %.1f, %.1f",
      paramlist[j],    paramlist[j+1],  paramlist[j+2],  paramlist[j+3],
      paramlist[j+4],  paramlist[j+5],  paramlist[j+6],  paramlist[j+7],
      paramlist[j+8],  paramlist[j+9],  paramlist[j+10], paramlist[j+11],
      paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
      paramlist[j+16]);
  }
}

/** eeprom_lensshading_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Performs lenshading calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_lensshading_calibration(void *e_ctrl)
{
  int32_t i, j;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  chromatix_VFE_common_type * common_chromatix =
    (chromatix_VFE_common_type *) (ectrl->eeprom_afchroma.
    chromatix.chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON]);
  chromatix_rolloff_type *chromatix = &(common_chromatix->chromatix_rolloff);
  lsccalib_data_t *lsc = &ectrl->eeprom_data.lsc;
  mesh_rolloff_array_type *mesh_rolloff_table;

  SDBG( ": Enter" );

  for (j = 0; j < ROLLOFF_MAX_LIGHT; j++) {
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
      chromatix->chromatix_mesh_rolloff_table[j].r_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
        lsc->lsc_calib[j].r_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].b_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
        lsc->lsc_calib[j].b_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
        lsc->lsc_calib[j].gr_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
        lsc->lsc_calib[j].gb_gain[i]);


      chromatix->chromatix_mesh_rolloff_table_lowlight[j].r_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
        lsc->lsc_calib[j].r_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].b_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
        lsc->lsc_calib[j].b_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].gr_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
        lsc->lsc_calib[j].gr_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].gb_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
        lsc->lsc_calib[j].gb_gain[i]);
    }
  }

  j= ROLLOFF_D65_LIGHT;
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    chromatix->chromatix_mesh_rolloff_table_LED.r_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
      lsc->lsc_calib[j].r_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.b_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
      lsc->lsc_calib[j].b_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.gr_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
      lsc->lsc_calib[j].gr_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.gb_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
      lsc->lsc_calib[j].gb_gain[i]);
  }

#if (CHROMATIX_VERSION >= 0x0310)
  mesh_rolloff_table = &(chromatix->chromatix_mesh_rolloff_table_LED2);
#else
  mesh_rolloff_table = &(chromatix->chromatix_mesh_rolloff_table_Strobe);
#endif

  /* Second led roll off table calibration based on A light */
  j= ROLLOFF_A_LIGHT;
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    mesh_rolloff_table->r_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
      lsc->lsc_calib[j].r_gain[i]);
    mesh_rolloff_table->b_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
      lsc->lsc_calib[j].b_gain[i]);
    mesh_rolloff_table->gr_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
      lsc->lsc_calib[j].gr_gain[i]);
    mesh_rolloff_table->gb_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
      lsc->lsc_calib[j].gb_gain[i]);
  }

  SLOW("CHROMATIX LSC MATRICES");
  for (i = 0; i < ROLLOFF_MAX_LIGHT; i++) {
    SLOW("chromatix_mesh_rolloff_table[%d] FINAL R MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].r_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL GR MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].gr_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL GB MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].gb_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL B MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].b_gain);
  }
  SDBG( "Exit" );
}

static uint8_t crc_reverse_byte(uint32_t data)
{
    return ((data * 0x0802LU & 0x22110LU) |
            (data * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

/** eeprom_set_chromatix_af_pointer:
 *    @e_ctrl: EEEPROM control handle
 *    @e_params: chromatix and af tuning pointer
 *
 * Sets the chromatix and af tuning pointers for calibration
 * purpose
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/
static int32_t eeprom_set_chromatix_af_pointer(
   sensor_eeprom_data_t *e_ctrl, eeprom_set_chroma_af_t *e_params) {

  int32_t rc = SENSOR_SUCCESS;

  SDBG("Enter");
  RETURN_ERR_ON_NULL(e_params,SENSOR_FAILURE);

  e_ctrl->eeprom_afchroma = *e_params;
  SLOW("chromatix pointer = %p",
    e_ctrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_ISP]);
  SLOW("common chromatix pointer = %p", e_ctrl->eeprom_afchroma.chromatix.
    chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON]);

  SDBG("Exit");
  return rc;
}

/** eeprom_util_check_crc16: runs CRC 16 over byte stream
 *    @data: input byte stream
 *    @size: num of bytes
 *    @ref_crc: crc to be checked
 *
 * Return:
 *  1 - crc match
 *  0 - crc not match
 **/
int32_t eeprom_util_check_crc16(uint8_t *data, uint32_t size, uint32_t ref_crc)
{
    int32_t crc_match = 0;
    uint16_t crc = 0x0000;
    uint16_t crc_reverse = 0x0000;
    uint32_t i, j;

    uint32_t tmp;
    uint32_t tmp_reverse;

    /* Calculate both methods of CRC since integrators differ on
      * how CRC should be calculated. */
    for (i = 0; i < size; i++) {
        tmp_reverse = crc_reverse_byte(data[i]);
        tmp = data[i] & 0xff;
        for (j = 0; j < 8; j++) {
            if (((crc & 0x8000) >> 8) ^ (tmp & 0x80))
                crc = (crc << 1) ^ 0x8005;
            else
                crc = crc << 1;
            tmp <<= 1;

            if (((crc_reverse & 0x8000) >> 8) ^ (tmp_reverse & 0x80))
                crc_reverse = (crc_reverse << 1) ^ 0x8005;
            else
                crc_reverse = crc_reverse << 1;

            tmp_reverse <<= 1;
        }
    }

    crc_reverse = (crc_reverse_byte(crc_reverse) << 8) |
        crc_reverse_byte(crc_reverse >> 8);

    if (crc == ref_crc || crc_reverse == ref_crc)
        crc_match = 1;

    SHIGH("REF_CRC 0x%x CALC CRC 0x%x CALC Reverse CRC 0x%x matches? %d\n",
            ref_crc, crc, crc_reverse, crc_match);

    return crc_match;
}

/** mesh_rolloff_V4_UpScaleOTPMesh_to_13x17:
 *    @MeshIn: Input Mesh  num_x by num_y
 *    @MeshOut: Output Mesh   13 by 17
 *    @width: Full-resolution Sensor width
 *    @height: Full-resolution Sensor height
 *    @num_x, num_y: the number of OTP grid points
 *    @offset_x, offset_y: the offset (in per-channel) of first OTP block
 *      center to the image top left corner.
 *    @block_x, block_y: OTP block size
 *
 *    This function is used to resample the OTP data (num_x by
 *    num_y OTP) onto the ideal rolloff grid (13by17) on
 *    8084/8994. This function assumes that the OTP data is
 *    uniformly sampled
 *
 *    Common Example (QTI OTP): num_x by num_y OTP with its first block
 *    center at (0, 0):
 *    float block_x = width/2/(float)(num_x-1);
 *    float block_y = height/2/(float)(num_y-1);
 *    int32_t offset_x = 0;
 *    int32_t offset_y = 0;
 *
 *    Special Example: num_x by num_y OTP with its first block center
 *    at (block_x/2, block_y/2):
 *    float block_x = width/2/(float)num_x;
 *    float block_y = height/2/(float)num_y;
 *    int32_t offset_x = block_x/2;
 *    int32_t offset_y = block_y/2;
 *
 * Return:
 * void
 **/

void mesh_rolloff_V4_ScaleOTPMesh_to_13x17(float *MeshIn,  float *MeshOut,
  int32_t width, int32_t height, int32_t num_x, int32_t num_y, int32_t offset_x, int32_t offset_y,
  float block_x, float block_y)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float *Extend_Mesh;
  double tx, ty;
  int32_t MESH_H, MESH_V, Nx, Ny;
  int32_t ix, iy;
  int32_t i, j;
  float gh, gv;

  /* Initialize the roll-off mesh grid */
  MESH_H = 16;  /* ideal rolloff block number (horizontal). */
  MESH_V = 12;  /* ideal rolloff block number (vertical). */

  Nx = num_x;    /* the OTP grid number (horizontal) */
  Ny = num_y;    /* the OTP grid number (vertical) */

  float Gh = block_x;   /* the OTP block size (horizontal) */
  float Gv = block_y;   /* the OTP block size (vertical) */

  gh = (float)(width/2.0/(float)MESH_H);  /* ideal rolloff block size (horizontal) */
  gv = (float)(height/2.0/(float)MESH_V); /* ideal rolloff block size (vertical) */

  /* outer extend the mesh data 1 block by keeping the same slope */
  Extend_Mesh =
    (float*)malloc(sizeof(float) * (uint32_t)(Nx+2) * (uint32_t)(Ny+2));
  RETURN_VOID_ON_NULL(Extend_Mesh);

  for (i=1; i<Ny+1; i++)
    for (j=1; j<Nx+1; j++)
      Extend_Mesh[i*(Nx+2)+j] = MeshIn[(i-1)*Nx+j-1];

  Extend_Mesh[0*(Nx+2)+0] = Extend_Mesh[1*(Nx+2)+1]*2- Extend_Mesh[2*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+0] = Extend_Mesh[(Ny)*(Nx+2)+1]*2
      - Extend_Mesh[(Ny-1)*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+Nx+1] = Extend_Mesh[(Ny)*(Nx+2)+Nx]*2
      - Extend_Mesh[(Ny-1)*(Nx+2)+Nx-1];
  Extend_Mesh[0*(Nx+2)+Nx+1] = Extend_Mesh[1*(Nx+2)+Nx]*2
      - Extend_Mesh[2*(Nx+2)+Nx-1];

  for (i=1; i<Ny+1; i++)
  {
    Extend_Mesh[i*(Nx+2)+0] = Extend_Mesh[i*(Nx+2)+1]*2 -
      Extend_Mesh[i*(Nx+2)+2];
    Extend_Mesh[i*(Nx+2)+Nx+1] = Extend_Mesh[i*(Nx+2)+Nx]*2 -
      Extend_Mesh[i*(Nx+2)+Nx-1];
  }
  for (j=1; j<Nx+1; j++)
  {
    Extend_Mesh[0*(Nx+2)+j] = Extend_Mesh[1*(Nx+2)+j]*2 -
      Extend_Mesh[2*(Nx+2)+j];
    Extend_Mesh[(Ny+1)*(Nx+2)+j] = Extend_Mesh[(Ny)*(Nx+2)+j]*2 -
      Extend_Mesh[(Ny-1)*(Nx+2)+j];
  }

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {

      tx = (double)(j * (double)gh - offset_x + Gh) / (double)Gh;
      ix = (int32_t)MAX(MIN(floor(tx), Nx), 0);
      tx -= (double)ix;
      ty = (double)(i * (double)gv - offset_y + Gv) / (double)Gv;
      iy = (int32_t)MAX(MIN(floor(ty), Ny), 0);
      ty -= (double)iy;

      if (i == 0 || j == 0  || i == MESH_V|| j == MESH_H) {
        /* for boundary points, use bilinear interpolation */

        b1 = (float)((1 - tx) * Extend_Mesh[iy*(Nx+2) + ix] +
          (float)tx * Extend_Mesh[iy*(Nx+2) + ix+1]);
        b2 = (float)((1 - tx) * Extend_Mesh[(iy+1)*(Nx+2) + ix] +
          (float)tx * Extend_Mesh[(iy+1)*(Nx+2) + ix+1]);

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((1 - ty)*b1 + ty*b2);
      } else {
        /* for nonboundary points, use bicubic interpolation */

        /* get x direction coeff and y direction coeff */
        CUBIC_F((float)tx, cxm, cx0, cx1, cx2);
        CUBIC_F((float)ty, cym, cy0, cy1, cy2);

        am = Extend_Mesh[(iy-1) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+2)];
        bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy  ) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+2)];
        b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy+1) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy+2) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((cym * bm) + (cy0 * b0)
            + (cy1 * b1) + (cy2 * b2));
      }
    }
  }
  /* free memory */
  free(Extend_Mesh);
}

void interp_grid_optimization(int32_t raw_width, int32_t raw_height, int32_t *scale_cubic,
  int32_t *deltah, int32_t *deltav, int32_t *subgridh, int32_t *subgridv)
{
  int32_t level, w, h, sgh, sgv, gh, gv, dh, dv, nx, ny;

  nx = MESH_ROLLOFF_HORIZONTAL_GRIDS_B;
  ny = MESH_ROLLOFF_VERTICAL_GRIDS_B;

  w = raw_width>>1;  // per-channel image width
  h = raw_height>>1;  // per-channel image height

  level= 3 + 1; // Initial bicubic level level as 1 more than maximum 3

  do {
    level--;
    sgh = (w + nx - 1) / nx;  // Ceil
    sgh = (sgh + (1 << level) - 1) >> level;  // Ceil
    gh = sgh << level;     // Bayer grid width
    dh = gh * nx - w; // two-side overhead

    sgv = (h + ny - 1) / ny;  // Ceil
    sgv = (sgv + (1 << level) - 1) >> level;   // Ceil
    gv = sgv << level;     // Bayer grid height
    dv = gv * ny - h; // two-side overhead

  } while ((level > 0) && ((sgh < 9) || (sgv <9) || (dh >= gh) ||
      (dv >= gv) ||(gh-(dh+1)/2<16) ||(gv-(dv+1)/2<9)));

  *scale_cubic = 1 << level;
  *deltah = (dh + 1)>>1;
  *deltav = (dv + 1)>>1;
  *subgridh = sgh;
  *subgridv = sgv;
}

/** mesh_rolloff_V4_UpScaleOTPMesh_to_10x13:
 *    @MeshIn: Input Mesh  num_x by num_y
 *    @MeshOut: Output Mesh   10 by 13
 *    @width: Full-resolution Sensor width
 *    @height: Full-resolution Sensor height
 *    @num_x, num_y: the number of OTP grid points
 *    @offset_x, offset_y: the offset (in per-channel) of first OTP block
 *      center to the image top left corner.
 *    @block_x, block_y: OTP block size
 *
 *    This function is used to resample the OTP data (num_x by
 *    num_y OTP) onto the 10x13 rolloff grid on
 *    8974. This function assumes that the OTP data is
 *    uniformly sampled.
 *
 *    Common Example (QTI OTP): num_x by num_y OTP with its first block
 *    center at (0, 0):
 *    float block_x = width/2/(float)(num_x-1);
 *    float block_y = height/2/(float)(num_y-1);
 *    int32_t offset_x = 0;
 *    int32_t offset_y = 0;
 *
 *    Special Example: num_x by num_y OTP with its first block center
 *    at (block_x/2, block_y/2):
 *    float block_x = width/2/(float)num_x;
 *    float block_y = height/2/(float)num_y;
 *    int32_t offset_x = block_x/2;
 *    int32_t offset_y = block_y/2;
 *
 * Return:
 * void
 **/

void mesh_rolloff_V4_ScaleOTPMesh_to_10x13(float *MeshIn,  float *MeshOut,
  int32_t width, int32_t height, int32_t num_x, int32_t num_y, int32_t offset_x, int32_t offset_y,
  float block_x, float block_y)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float *Extend_Mesh;
  double tx, ty;
  int32_t MESH_H, MESH_V, Nx, Ny;
  int32_t ix, iy;
  int32_t i, j;
  int32_t gh, gv, scale, sgh, sgv, dh, dv;


  /* Initialize the roll-off mesh grid */
  MESH_H = 12; /* the rolloff block number (horizontal). */
  MESH_V = 9;  /* the rolloff block number (vertical). */

  Nx = num_x;    /* the OTP grid number (horizontal) */
  Ny = num_y;    /* the OTP grid number (vertical) */

  float Gh = block_x;   /* the OTP block size (horizontal) */
  float Gv = block_y;   /* the OTP block size (vertical) */

  interp_grid_optimization(width, height, &scale, &dh, &dv, &sgh, &sgv);
  gh = sgh * scale; /* the rolloff block size (horizontal). */
  gv = sgv * scale; /* the rolloff block size (vertical). */

  /* outer extend the mesh data 1 block by keeping the same slope */
  Extend_Mesh =
    (float*)malloc(sizeof(float) * (uint32_t)(Nx+2) * (uint32_t)(Ny+2));
  RETURN_VOID_ON_NULL(Extend_Mesh);

  for (i=1; i<Ny+1; i++)
    for (j=1; j<Nx+1; j++)
      Extend_Mesh[i*(Nx+2)+j] = MeshIn[(i-1)*Nx+j-1];

  Extend_Mesh[0*(Nx+2)+0] = Extend_Mesh[1*(Nx+2)+1]*2- Extend_Mesh[2*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+0] = Extend_Mesh[(Ny)*(Nx+2)+1]*2
      - Extend_Mesh[(Ny-1)*(Nx+2)+2];
  Extend_Mesh[(Ny+1)*(Nx+2)+Nx+1] = Extend_Mesh[(Ny)*(Nx+2)+Nx]*2
      - Extend_Mesh[(Ny-1)*(Nx+2)+Nx-1];
  Extend_Mesh[0*(Nx+2)+Nx+1] = Extend_Mesh[1*(Nx+2)+Nx]*2
      - Extend_Mesh[2*(Nx+2)+Nx-1];

  for (i=1; i<Ny+1; i++)
  {
    Extend_Mesh[i*(Nx+2)+0] = Extend_Mesh[i*(Nx+2)+1]*2 -
      Extend_Mesh[i*(Nx+2)+2];
    Extend_Mesh[i*(Nx+2)+Nx+1] = Extend_Mesh[i*(Nx+2)+Nx]*2 -
      Extend_Mesh[i*(Nx+2)+Nx-1];
  }
  for (j=1; j<Nx+1; j++)
  {
    Extend_Mesh[0*(Nx+2)+j] = Extend_Mesh[1*(Nx+2)+j]*2 -
      Extend_Mesh[2*(Nx+2)+j];
    Extend_Mesh[(Ny+1)*(Nx+2)+j] = Extend_Mesh[(Ny)*(Nx+2)+j]*2 -
      Extend_Mesh[(Ny-1)*(Nx+2)+j];
  }

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {

      tx = (double)(j * (double)gh - dh - offset_x + Gh) / (double)Gh;
      ix = (int32_t)MAX(MIN(floor(tx), Nx), 0);
      tx -= (double)ix;

      ty = (double)(i * (double)gv - dv - offset_y + Gv) / (double)Gv;
      iy = (int32_t)MAX(MIN(floor(ty), Ny), 0);
      ty -= (double)iy;

      if (i == 0 || j == 0  || i == MESH_V|| j == MESH_H) {
        /* for boundary points, use bilinear interpolation */

        b1 = (float)((1 - tx) * Extend_Mesh[iy*(Nx+2) + ix] +
          (float)tx * Extend_Mesh[iy*(Nx+2) + ix+1]);
        b2 = (float)((1 - tx) * Extend_Mesh[(iy+1)*(Nx+2) + ix] +
          (float)tx * Extend_Mesh[(iy+1)*(Nx+2) + ix+1]);

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((1 - ty)*b1 + ty*b2);
      } else {
        /* for nonboundary points, use bicubic interpolation */

        /* get x direction coeff and y direction coeff */
        CUBIC_F((float)tx, cxm, cx0, cx1, cx2);
        CUBIC_F((float)ty, cym, cy0, cy1, cy2);

        am = Extend_Mesh[(iy-1) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy-1) *(Nx+2)+ (ix+2)];
        bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy  ) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy  ) *(Nx+2)+ (ix+2)];
        b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy+1) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy+1) *(Nx+2)+ (ix+2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy+2) *(Nx+2)+ (ix-1)];
        a0 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix)];
        a1 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+1)];
        a2 = Extend_Mesh[(iy+2) *(Nx+2)+ (ix+2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((cym * bm) + (cy0 * b0)
            + (cy1 * b1) + (cy2 * b2));
      }
    }
  }
  /* free memory */
  free(Extend_Mesh);
}

