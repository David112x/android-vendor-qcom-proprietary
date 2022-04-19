/* linearization40_0310.c
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "linearization40.h"
#include "isp_pipeline_reg.h"

void linearization40_enab_sub_mod(isp_sub_module_t *isp_sub_module)
{
  chromatix_VFE_common_type *chromatix_common = NULL;

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;

  if (chromatix_common->chromatix_L.linearization_enable > 0 ||
      chromatix_common->chromatix_L_v2_data.linearization_v2_enable > 0) {
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->chromatix_module_enable = TRUE;
  }
}
/** linearization40_calc_clamp:
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_calc_clamp(linearization40_t *linearization,
  Linearization_high_res_Lut_t *pTbl,
  isp_sub_module_output_t *sub_module_output)
{
  boolean                  ret = TRUE;
  float                    clamping = 0.0f;
  float                    max_offset = (float)LINEAR_MAX_VAL;

  if (!linearization || !pTbl || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", linearization, pTbl, sub_module_output);
    return FALSE;
  }

  if (sub_module_output->algo_params) {
    ISP_DBG("unity_slope:%d r:%f gr:%f gb:%f b:%f",
            linearization->last_region_unity_slope_enable, pTbl->r_lut_p[0],
            pTbl->gr_lut_p[0], pTbl->gb_lut_p[0], pTbl->b_lut_p[0]);
    if (linearization->last_region_unity_slope_enable &&
        !linearization->pedestal_enable) {
      sub_module_output->algo_params->st_gain.r_gain = max_offset/(max_offset - pTbl->r_lut_p[0]);
      sub_module_output->algo_params->st_gain.gr_gain = max_offset/(max_offset - pTbl->gr_lut_p[0]);
      sub_module_output->algo_params->st_gain.gb_gain = max_offset/(max_offset - pTbl->gb_lut_p[0]);
      sub_module_output->algo_params->st_gain.b_gain = max_offset/(max_offset - pTbl->b_lut_p[0]);
    } else {
      sub_module_output->algo_params->st_gain.r_gain = 1.0f;
      sub_module_output->algo_params->st_gain.gr_gain = 1.0f;
      sub_module_output->algo_params->st_gain.gb_gain = 1.0f;
      sub_module_output->algo_params->st_gain.b_gain = 1.0f;
    }
    ISP_DBG("demux_dbg clamping: %f %f %f %f %f %f", clamping, (float)LINEAR_MAX_VAL,
            sub_module_output->algo_params->st_gain.r_gain,
            sub_module_output->algo_params->st_gain.gr_gain,
            sub_module_output->algo_params->st_gain.gb_gain,
            sub_module_output->algo_params->st_gain.b_gain);
  }
  return ret;
}

/** linearization40_v2_copy_table_to_float
 *
 *  @input: chromatix where values are unsigned int
 *  @output: converted input int to float
 *
 *  Covert chromatix uinsigned int to float
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_v2_copy_table_to_float(
  void *data1, void *data2)
{
  int i;
  chromatix_linearization_v2_type * input;
  Linearization_high_res_Lut_t *output;
  input = (chromatix_linearization_v2_type *)data1;
  output = (Linearization_high_res_Lut_t *)data2;
  for (i = 0; i < 8; i++) {
    output->r_lut_p[i] = (float)input->r_lut_p[i];
    output->gr_lut_p[i] = (float)input->gr_lut_p[i];
    output->gb_lut_p[i] = (float)input->gb_lut_p[i];
    output->b_lut_p[i] = (float)input->b_lut_p[i];
    output->r_lut_base[i] = (float)input->r_lut_base[i];
    output->gr_lut_base[i] = (float)input->gr_lut_base[i];
    output->gb_lut_base[i] = (float)input->gb_lut_base[i];
    output->b_lut_base[i] = (float)input->b_lut_base[i];
  }
  output->r_lut_base[8] = (float)input->r_lut_base[8];
  output->gr_lut_base[8] = (float)input->gr_lut_base[8];
  output->gb_lut_base[8] = (float)input->gb_lut_base[8];
  output->b_lut_base[8] = (float)input->b_lut_base[8];

  return TRUE;
}

/** linearization40_get_right_tables
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_get_right_tables (
  linearization40_t *linearization,
  chromatix_VFE_common_type    *pchromatix_common,
  Linearization_high_res_Lut_t *output1,
  Linearization_high_res_Lut_t *output2,
  cam_sensor_hdr_type_t        hdr_mode
)
{
  chromatix_L_v2_type          *pchromatix_L_v2 = NULL;
  chromatix_linearization_v2_type *ptable_v2 = NULL;
  int  multifactor = ISP_PIPELINE_WIDTH - HDR_OUTPUT_BIT_WIDTH;
  boolean is_lsb_alligned = FALSE;
  uint32_t ret = FALSE;

  pchromatix_L_v2 = &pchromatix_common->chromatix_L_v2_data;

  if (pchromatix_L_v2->linearization_v2_enable) {
    linearization40_update_base_tables(linearization, pchromatix_common);
    /* V2 chromatix input is 14 bit. No need for conversion except for
       HDR_IN_SENSOR use case */
    if (hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) {
      is_lsb_alligned = TRUE;
      linearization40_BitWidthUpdate(&linearization->linear_table_output1,
        multifactor, is_lsb_alligned);
      linearization40_BitWidthUpdate(&linearization->linear_table_output2,
        multifactor, is_lsb_alligned);
    }
    *output1 = linearization->linear_table_output1;
    *output2 = linearization->linear_table_output2;
    ret = TRUE;
  }

  return ret;
}

#if !OVERRIDE_FUNC
/** linearization40_update_right_base_tables
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_update_right_base_tables (
  void *data1, void *data2)
{
  linearization40_t *linearization;
  chromatix_VFE_common_type *pchromatix_common;
  chromatix_L_type    *pchromatix_L = NULL;
  chromatix_L_v2_type *pchromatix_L_v2 = NULL;
  uint8_t              tgr_idx2;
  uint32_t ret = FALSE;

  linearization = (linearization40_t *)data1;
  pchromatix_common = (chromatix_VFE_common_type *)data2;
  pchromatix_L = &pchromatix_common->chromatix_L;
  pchromatix_L_v2 = &pchromatix_common->chromatix_L_v2_data;

  if (pchromatix_L_v2->linearization_v2_enable) {
    /* Load tables based on the region indices */
    linearization->last_region_unity_slope_enable =
      pchromatix_L_v2->last_region_unity_slope_enable;
    linearization40_v2_copy_table_to_float(
      &pchromatix_L_v2->linear_table_data[linearization->trigger_index],
      &linearization->linear_table_output1);
    /* If trigger_idx + 1 falls outside range use same table as trigger_idx */
    tgr_idx2 = ((linearization->trigger_index + 1) < NUM_BLK_REGIONS) ?
      (linearization->trigger_index + 1) : linearization->trigger_index;
    linearization40_v2_copy_table_to_float(
        &pchromatix_L_v2->linear_table_data[tgr_idx2],
        &linearization->linear_table_output2);
    ret = TRUE;
  } else {
    linearization->last_region_unity_slope_enable =
      pchromatix_L->last_region_unity_slope_enable;
  }

  return ret;
}

/** linearization40_get_trigger_idx
 *
 *  @pchromatix_common: pchromatix common
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_get_trigger_idx (
  isp_sub_module_t *isp_sub_module,
  void *data1, void *data2)
{
  linearization40_t *linearization;
  chromatix_VFE_common_type *pchromatix_common;
  chromatix_L_v2_type       *linearization_v2 = NULL;
  uint8_t trigger_index = NUM_BLK_REGIONS + 1;
  trigger_point_type        *linearization_trigger = NULL;
  int i = 0;
  uint32_t ret = FALSE;
  float aec_reference;
  float ratio = 0;
  float start = 0;
  float end = 0;

  linearization = (linearization40_t *)data1;
  pchromatix_common = (chromatix_VFE_common_type *)data2;
  linearization_v2 = &pchromatix_common->chromatix_L_v2_data;

  if (linearization_v2 && linearization_v2->linearization_v2_enable) {
    if (linearization_v2->control_linearization == 0) {
      /* Lux index based */
      aec_reference = linearization->aec_update.lux_idx;
    } else {
      /* Gain based */
      aec_reference = linearization->aec_update.sensor_gain;
    }
    ISP_HIGH("aec_reference :%f", aec_reference);

    for (i = 0; i < NUM_BLK_REGIONS; i++) {
      linearization_trigger = &(linearization_v2->linear_table_data[i].
        linearization_v2_trigger);
      trigger_index = i;

      if (i == NUM_BLK_REGIONS - 1) {
        /* falls within region 6 but we do not use trigger points in the region */
        ratio = 0;
        break;
      }

      if (linearization_v2->control_linearization == 0) {
        start = linearization_trigger->lux_index_start;
        end   = linearization_trigger->lux_index_end;
      } else {

        start = linearization_trigger->gain_start;
        end   = linearization_trigger->gain_end;
        ISP_HIGH("gain_start :%f", start);
        ISP_HIGH("gain_end :%f", end);
      }

      /* index is within interpolation range, find ratio */
      if (aec_reference > start && aec_reference < end) {
        ratio = (aec_reference - start)/(end - start);
        ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
          ratio);
      }

      /* already scanned past the lux index */
      if (aec_reference < end) {
        break;
      }
    }

    if (trigger_index >= NUM_BLK_REGIONS) {
      ISP_ERR("no trigger match for linearization trigger value: %f\n", aec_reference);
      ret = FALSE;
      goto ERROR;
    }

    if ((trigger_index != linearization->trigger_index) ||
     (!F_EQUAL(ratio, linearization->aec_ratio))) {
      linearization->trigger_index = trigger_index;
      linearization->aec_ratio = ratio;
      isp_sub_module->trigger_update_pending = TRUE;
    }
	ret = TRUE;
  }

ERROR:
  return ret;
}
#endif
