/* gamma_algo.c
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
#include "module_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "isp_adrc_tune_def.h"
#include "gamma_algo.h"


isp_adrc_knee_lut_type gamma_adrc_tuning_config = {
    #include "isp_adrc_tune_param.h"
};

/** gamma_adrc_interpolate_gamma_curve
 *
 *    @gamma_ratio: input Gamma ratio
 *    @mod: Gamma module handle
 *
 * Interpolate the ADRC LUTs with Interpoltaed Gamma Values
 * and update the GAMMA Curve.
 **/
static void gamma_adrc_gamma_curve(float gamma_ratio, gamma_algo_t *algo)
{
  int i = 0, j = 0, x1 = 0, x2 = 0;
  float index1 = 0.0f, index2 = 0.0f;
  float x_temp = 0.0f, val1 = 0.0f, val2 = 0.0f, cur_y = 0.0f;
  GAMMA_LUT_TYPE *gamma[ISP_GAMMA_NUM_CHANNEL];
  gamma_rgb_ds_t *gamma_tbl;

  /* Gain lut related */
  unsigned short num_knee = gamma_adrc_tuning_config.knee_points;
  float *knee_gain = algo->adrc_gain_lut_table.knee_gain;
  float *knee_index = algo->adrc_gain_lut_table.knee_index;
  float knee_lut[ISP_ADRC_GAIN_LUT_KNEE_LENGTH];
  unsigned int lut_max_gain_idx = num_knee - 1;
  unsigned int gain_idx = 1;

  gamma_tbl = (gamma_rgb_ds_t *)algo->gamma_tbl;

  if (lut_max_gain_idx >= ISP_ADRC_GAIN_LUT_KNEE_LENGTH) {
    lut_max_gain_idx = ISP_ADRC_GAIN_LUT_KNEE_LENGTH - 1;
    ISP_ERR("lut max value exeeds the limit");
  }

  /* Updated gain lut curve (in LUT format) based on gain rate */
  isp_sub_module_util_gain_lut_with_drc_gain_weight(knee_gain, knee_index,
    ISP_ADRC_GAIN_LUT_KNEE_LENGTH, gamma_ratio, knee_lut);

  gamma[0] = gamma_tbl->gamma_r;
  gamma[1] = gamma_tbl->gamma_g;
  gamma[2] = gamma_tbl->gamma_b;

  /* Generate each entry of the Gamma_Curve */
  for (i = 1; i < ISP_GAMMA_NUM_ENTRIES; i++) {
    /* Get input Pixel Value */
    x_temp = i * ISP_ADRC_LUT_LENGTH / (float)ISP_GAMMA_NUM_ENTRIES;

    /* Find appropriate index in gain lut. */
    if ((x_temp > knee_index[gain_idx]) &&
        (gain_idx < lut_max_gain_idx))
      gain_idx += 1;

    /* Get ADRC gain in linear domain */
    index1  = knee_index[gain_idx - 1];
    index2  = knee_index[gain_idx];
    val1    = knee_lut[gain_idx -1];
    val2    = knee_lut[gain_idx];
    x_temp  = isp_sub_module_util_linear_interpolate_float(x_temp,
      index1, index2, val1, val2);

    /* Align knee lut again to GAMMA width */
    x_temp *= ISP_GAMMA_NUM_ENTRIES / ISP_ADRC_LUT_LENGTH;
    /* Apply gamma */
    for (j = 0; j < ISP_GAMMA_NUM_CHANNEL; j++) {
      x1 = (unsigned int)x_temp;
      x1 = (x1 >= ISP_GAMMA_NUM_ENTRIES) ? ISP_GAMMA_NUM_ENTRIES - 1 : x1;

      x2 = x1 + 1;
      val1 = (float)gamma[j][x1];
      val2 = (float)(x2 == ISP_GAMMA_NUM_ENTRIES ?
                     (1 << GAMMA_HW_PACK_BIT) : gamma[j][x2]);

      cur_y =
        isp_sub_module_util_linear_interpolate(x_temp, x1, x2, val1, val2);

      /* Update gamma */
      gamma[j][i] =
        (GAMMA_LUT_TYPE)Clamp(((uint32_t)(cur_y + 0.5f)), 0.0f, (float)MAX_GAMMA_VALUE);
    }
  }
  return;
}

/** gamma_update_adrc_gamma_curve:
 *  @gamma: gamma module
 *
 * Update the Gamma curve based on the DRC gain.
 **/
boolean gamma_update_adrc_gamma_curve(gamma_algo_t *algo)
{
  boolean                  ret = TRUE;
  uint32_t                 lut_idx = 0;
  float                    drc_gain = 0.0, gamma_ratio = 0.0;
  uint32_t                 i = 0;
  aec_update_t             *aec_update;

  /* get the ADRC Gamma gain values, updated by 3A*/
  aec_update = (aec_update_t*)algo->stats_update;
  gamma_ratio = aec_update->gamma_ratio;
  drc_gain = aec_update->total_drc_gain;

  /* get the LUT index values based the calculation*/
  lut_idx = isp_sub_module_util_get_lut_index(drc_gain);

  /*update Knee table entries from ADRC LUTs based on the
   DRC Gain LUT index*/
  for (i = 0; i < ISP_ADRC_GAIN_LUT_KNEE_LENGTH; i++) {
    algo->adrc_gain_lut_table.knee_gain[i] =
      gamma_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_gain[i];
    algo->adrc_gain_lut_table.knee_index[i] =
      gamma_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_index[i];
  }

  /* Update gamma Curve based on ADRC gamma_ratio
     and interpolated Gamma*/
  gamma_adrc_gamma_curve(gamma_ratio, algo);

  return ret;
}
