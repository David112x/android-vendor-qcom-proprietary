/* luma_adaptation_algo.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GAMMA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GAMMA, fmt, ##args)


/* isp headers */
#include "luma_adaptation40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_adrc_tune_def.h"

isp_adrc_knee_lut_type la_adrc_tuning_config = {
  #include "isp_adrc_tune_param.h"
};

/** la40_adrc_gamma_table_interpolate
 *
 *    @gamma_in: Interpolated Gamma Table
 *    @la_mod: LA module handle
 *    @la_ratio: la_ratio given by AEC
 * Interpolate the ADRC LUTs with Interpoltaed Gamma Values
 * and update the LA_Curve.
 **/
static void la_adrc_generate_la_curve(void *gamma_in_tbl,
  luma_adaptation40_t *la_mod, float la_ratio)
{
  int i = 0, x1 = 0, x2 = 0, gamma_idx = 0;
  float index1 = 0.0f, index2 = 0.0f;
  float x_a = 0.0f, val1 = 0.0f, val2 = 0.0f, cur_y = 0.0f, cur_x = 0.0f;
  float input = 0.0f, x_temp_gain = 0.0f, x_a_eqvlnt = 0.0f, x_b_eqvlnt = 0.0f;
  float x_a_eqvlnt_true = 0.0f, gain_eqvlnt = 0.0f, cur_y_true;
  uint16_t gamma_in[GAMMA_NUM_ENTRIES];
  float max_ms_gain = 1.0f, min_ms_gain = 5000.0f, norm_factor;
  unsigned int unity_reached = 0;
  float la_curve_x_step = (float)(1 << GAMMA_HW_PACK_BIT) /
      ISP_LA_CURVE_TABLE_LENGTH;

  /* Gain lut related */
  unsigned short num_knee = la_adrc_tuning_config.knee_points;
  float* knee_gain = la_mod->adrc_gain_lut_table.knee_gain;
  float* knee_index = la_mod->adrc_gain_lut_table.knee_index;
  float knee_lut[ISP_ADRC_GAIN_LUT_KNEE_LENGTH];
  unsigned int lut_max_gain_idx = num_knee - 1;
  unsigned int gain_idx = 1;

  if( num_knee > ISP_ADRC_GAIN_LUT_KNEE_LENGTH ) {
    ISP_ERR(" num_knee is greater than MAX LUT Knee length");
    return;
  }

  /* Updated gain lut curve (in LUT format) based on gain rate */
  isp_sub_module_util_gain_lut_with_drc_gain_weight(knee_gain, knee_index, num_knee,
    la_ratio, knee_lut);


  /* Gamma table copy [64] @ 256 as full BW output */
  for (i = 0; i < GAMMA_NUM_ENTRIES; i++) {
    if (GAMMA_HW_PACK_BIT == 8)
      gamma_in[i] = (((uint8_t *)gamma_in_tbl)[i] & 0xff);
    else
      gamma_in[i] = (((uint16_t *)gamma_in_tbl)[i] & 0xff);
  }

  /* Generate each entry of the LA_Curve */
  for (i = 1; i < ISP_LA_CURVE_TABLE_LENGTH; i++) {
    /* Get X_b */
    cur_x = la_curve_x_step * i;

    /* Get X_a by inversing Gamma*/
    while (gamma_idx + 1 <= (GAMMA_NUM_ENTRIES - 1) &&
           (cur_x > gamma_in[gamma_idx + 1]))
      gamma_idx++;

    input = cur_x;

    x1 = gamma_in[gamma_idx];
    x2 = (gamma_idx == (GAMMA_NUM_ENTRIES - 1) ?
          (1 << GAMMA_HW_PACK_BIT) : gamma_in[gamma_idx + 1]);
    val1 = (float)gamma_idx;
    val2 = (float)(gamma_idx + 1);
    x_a = isp_sub_module_util_linear_interpolate(input, x1, x2, val1, val2);
    x_a = x_a / ((float)GAMMA_NUM_ENTRIES / (float)ISP_ADRC_LUT_LENGTH);

    /* Get X_a' */
    /* Get GLUT(X_a) (pre-gamma) */
    input = x_a;
    /* Find appropriate index in gain lut. */
    if ((gain_idx <= lut_max_gain_idx) &&
         (input > knee_index[gain_idx]))
      gain_idx += 1;
    /* Get ADRC gain in linear domain */
    if ( gain_idx < ISP_ADRC_GAIN_LUT_KNEE_LENGTH ) {
      index1   = knee_index[gain_idx - 1];
      index2   = knee_index[gain_idx];
      val1    = knee_lut[gain_idx - 1];
      val2    = knee_lut[gain_idx];
    }
    x_a_eqvlnt  = isp_sub_module_util_linear_interpolate(input, index1, index2, val1, val2);

    /* Get X_b'  = Gamma(X_a')) (post-gamma) */
    input = x_a_eqvlnt;
    x1 = input >= GAMMA_NUM_ENTRIES ? (GAMMA_NUM_ENTRIES - 1) : (unsigned int)input;

    x2 = x1 + 1;
    val1 = (float)gamma_in[x1];
    val2 =
        (float)(x2 == GAMMA_NUM_ENTRIES ? (1 << GAMMA_HW_PACK_BIT) : gamma_in[x2]);

    x_b_eqvlnt = isp_sub_module_util_linear_interpolate(input, x1, x2, val1, val2);
    cur_y = x_b_eqvlnt;

    /* Calculate LA gain ratio curve (Need to cap to less than 4.0f)*/
    la_mod->la_curve[i] = cur_y;
    if (la_mod->la_curve[i] >= 3.99f)
      la_mod->la_curve[i] = 3.99f;
  }
  return;
}

/** la_get_gamma_interpolated_table
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event_identity: identity of trigger update mct event
 *  @gamma_table_rgb: Gamma table
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean la_get_gamma_interpolated_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module,uint32_t event_identity,
  void *gamma_table_g)
{
  boolean ret = TRUE;
  mct_event_t                    event;
  isp_private_event_t            private_event;

  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));

  private_event.data = gamma_table_g;
  private_event.type = ISP_PRIVATE_GET_GAMMA_INTERPOLATED_TABLE;
  if (!private_event.data) {
    ISP_ERR("failed: private_event.data %p", private_event.data);
    return FALSE;
  }

  event.direction = MCT_EVENT_UPSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.module_event_data = &private_event;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.identity = event_identity;


  ret = isp_sub_module_util_forward_event_from_module(module, &event);
  if (ret == FALSE) {
    ISP_ERR(" failed to fetch rolloff table ");
    return ret;
  }
  return ret;
}

/** la_update_adrc_la_curve:
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Interpolate the ADRC LUTs with Interpoltaed Gamma Values
 * and update the LA_Curve.
 **/
boolean la_update_adrc_la_curve(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  luma_adaptation40_t     *la_mod = NULL;
  uint32_t                 lut_idx = 0, gamma_table_size = 0;
  float                    la_ratio = 0.0, drc_gain = 0.0;
  uint32_t                 i = 0;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /* get Luma adaptation mod private*/
  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: mod %p", la_mod);
    return FALSE;
  }

  /* get the ADRC LA gain values, updated by 3A*/
  la_ratio = la_mod->stats_update.aec_update.la_ratio;
  drc_gain = la_mod->stats_update.aec_update.total_drc_gain;

  /* get the LUT index values based the calculation*/
  lut_idx = isp_sub_module_util_get_lut_index(drc_gain);

  /*update Knee table entries from ADRC LA LUTs based on the
    knee Gain LUT index*/
  for (i = 0; i < ISP_ADRC_GAIN_LUT_KNEE_LENGTH; i++) {
    la_mod->adrc_gain_lut_table.knee_gain[i] =
     la_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_gain[i];
    la_mod->adrc_gain_lut_table.knee_index[i] =
      la_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_index[i];
  }

  /* Gamma table size depends on the Gamma HW version.*/
  if (GAMMA_HW_PACK_BIT == 8)
    gamma_table_size = sizeof(uint8_t) * GAMMA_NUM_ENTRIES;
  else
    gamma_table_size = sizeof(uint16_t) * GAMMA_NUM_ENTRIES;

  if (la_mod->gamma_table_g == NULL) {
    la_mod->gamma_table_g = (uint8_t *)CAM_MALLOC(gamma_table_size);
    if (!la_mod->gamma_table_g) {
      ISP_ERR("failed: gamma_table_rgb %p", la_mod->gamma_table_g);
      return FALSE;
    }
    memset(la_mod->gamma_table_g, 0, gamma_table_size);
  }
  /* get Gamma interpolated table */
  ret = la_get_gamma_interpolated_table(module,
    isp_sub_module, event->identity, la_mod->gamma_table_g);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_get_gamma_interpolated_table");
  }

  for (i = 0; i < ISP_LA_CURVE_TABLE_LENGTH; i++)
    la_mod->la_curve[i] = 0;

  /* get interpolated LTM Curve based on ADRC Knee gain
     and interpolated Gamma*/
  la_adrc_generate_la_curve(la_mod->gamma_table_g, la_mod, la_ratio);

  return ret;
}
