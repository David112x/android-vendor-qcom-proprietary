/* chroma_enhan_algo.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)

/* isp headers */
#include "module_chroma_enhan40.h"
#include "chroma_enhan40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

/** chroma_enhan_algo_adjust_cv_by_adrc:
 *
 *  @cv: chroma_enhancement_type
 *  @color_drc_gain: DRC gain
 *  This function adjust CV configuration
 *
 *  Return: None
 **/
void chroma_enhan_algo_adjust_cv_by_adrc(
  Chromatix_ADRC_ACE_adj_type *adrc_adj_data,
  chroma_enhancement_type_t *cv, aec_update_t aec_update)
{
  float trigger = aec_update.lux_idx;
  float trigger_start[MAX_SETS_FOR_TONE_NOISE_ADJ];
  float Interp_ratio_lux, Interp_ratio;
  int RegionIdxStrt_lux = 0, RegionIdxEnd_lux = 0, i = 0;
  int RegionIdxStrt_0,RegionIdxStrt_1, RegionIdxEnd_0,RegionIdxEnd_1;
  float trigger_end[MAX_SETS_FOR_TONE_NOISE_ADJ];
  float drc_sat_adj_factor = 1.0f;
  float drc_sat_adj_factor1, drc_sat_adj_factor2;
  ADRC_ACE_adj_type  *adrc_ace_adj_data_start, *adrc_ace_adj_data_end;

  for (i = 0; i < MAX_SETS_FOR_ADRC_LUX_ADJ; i++) {
    trigger_start[i] =
        (float)adrc_adj_data->adrc_core_adj_data[i].adrc_lux_idx_trigger.start;
    trigger_end[i] =
        (float)adrc_adj_data->adrc_core_adj_data[i].adrc_lux_idx_trigger.end;
  }

  isp_sub_module_util_find_region_index_spatial(trigger, trigger_start,
    trigger_end, &Interp_ratio_lux, &RegionIdxStrt_lux, &RegionIdxEnd_lux,
    MAX_SETS_FOR_ADRC_LUX_ADJ);

  adrc_ace_adj_data_start =
    adrc_adj_data->adrc_core_adj_data[RegionIdxStrt_lux].adrc_ace_adj_data;
  adrc_ace_adj_data_end =
    adrc_adj_data->adrc_core_adj_data[RegionIdxEnd_lux].adrc_ace_adj_data;

  if (RegionIdxStrt_lux != RegionIdxEnd_lux) {
    for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
        trigger = aec_update.color_drc_gain;
        trigger_start[i] =
          (float)adrc_ace_adj_data_start[i].drc_gain_trigger.start;
        trigger_end[i] =
          (float)adrc_ace_adj_data_start[i].drc_gain_trigger.end;
    }
    isp_sub_module_util_find_region_index_spatial(trigger, trigger_start,
      trigger_end, &Interp_ratio, &RegionIdxStrt_0, &RegionIdxEnd_0,
      MAX_SETS_FOR_TONE_NOISE_ADJ);

    drc_sat_adj_factor1 = LINEAR_INTERPOLATION(
       adrc_ace_adj_data_start[RegionIdxEnd_0].drc_sat_adj_factor,
       adrc_ace_adj_data_start[RegionIdxStrt_0].drc_sat_adj_factor,
       Interp_ratio);

    for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
        trigger = aec_update.color_drc_gain;
        trigger_start[i] =
          (float)adrc_ace_adj_data_end[i].drc_gain_trigger.start;
        trigger_end[i] =
          (float)adrc_ace_adj_data_end[i].drc_gain_trigger.end;
    }
    isp_sub_module_util_find_region_index_spatial(trigger, trigger_start,
       trigger_end, &Interp_ratio, &RegionIdxStrt_1, &RegionIdxEnd_1,
       MAX_SETS_FOR_TONE_NOISE_ADJ);
    drc_sat_adj_factor2 = LINEAR_INTERPOLATION(
       adrc_ace_adj_data_end[RegionIdxEnd_1].drc_sat_adj_factor,
       adrc_ace_adj_data_end[RegionIdxStrt_1].drc_sat_adj_factor,
       Interp_ratio);
    drc_sat_adj_factor = LINEAR_INTERPOLATION(
       drc_sat_adj_factor2, drc_sat_adj_factor1, Interp_ratio_lux);
  } else {
    for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
        trigger = aec_update.color_drc_gain;
        trigger_start[i] =
          (float)adrc_ace_adj_data_start[i].drc_gain_trigger.start;
        trigger_end[i] =
          (float)adrc_ace_adj_data_start[i].drc_gain_trigger.end;
    }
    isp_sub_module_util_find_region_index_spatial(trigger, trigger_start,
       trigger_end, &Interp_ratio, &RegionIdxStrt_0, &RegionIdxEnd_0,
       MAX_SETS_FOR_TONE_NOISE_ADJ);
    drc_sat_adj_factor = LINEAR_INTERPOLATION(
       adrc_ace_adj_data_start[RegionIdxEnd_0].drc_sat_adj_factor,
       adrc_ace_adj_data_start[RegionIdxStrt_0].drc_sat_adj_factor,
       Interp_ratio);
  }

  cv->a_m *= drc_sat_adj_factor;
  cv->a_p *= drc_sat_adj_factor;
  cv->c_m *= drc_sat_adj_factor;
  cv->c_p *= drc_sat_adj_factor;
}




