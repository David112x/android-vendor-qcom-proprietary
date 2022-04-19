
/* gamma40_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "gamma40_ext.h"


/** gamma40_ext_set_spl_effect:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_ext_set_spl_effect(void *data1,
  void *data2, void *data3)
{
  gamma40_t            *gamma = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;
  cam_effect_mode_type  effect;
  boolean               ret = TRUE;
  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
  isp_sub_module = (isp_sub_module_t *) data2;
  effect = *(cam_effect_mode_type *)data3;

  switch (effect) {
  case CAM_EFFECT_MODE_BEAUTY:
    ISP_DBG("Beauty effect in Gamma");
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_BEAUTYSHOT);
    isp_sub_module->submod_trigger_enable = FALSE;
    isp_sub_module->trigger_update_pending = TRUE;
    break;

  default:
    gamma->effects.spl_effect = CAM_EFFECT_MODE_OFF;
    ret = TRUE;
    ret = gamma40_set_table(isp_sub_module, gamma, GAMMA_TABLE_DEFAULT);
    isp_sub_module->submod_trigger_enable = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
    break;
  }
  return ret;
}

/** gamma40_ext_set_table:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_ext_set_table(void *data1,
  void *data2, void *data3)
{
  gamma40_t        *gamma = NULL;
  isp_sub_module_t *isp_sub_module = NULL;
  gamma_table_type  table_type = GAMMA_TABLE_DEFAULT;
  boolean           ret = TRUE;
  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
  isp_sub_module = (isp_sub_module_t *) data2;
  table_type = *(gamma_table_type *)data3;

  switch (table_type) {
  case GAMMA_TABLE_BEAUTYSHOT:
    gamma->gamma_lut_size = sizeof(VFE_Beautyshot_GammaTable)/sizeof(uint16_t);
    gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_r[0],
      (uint16_t *)VFE_Beautyshot_GammaTable, gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
    gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_g[0],
      (uint16_t *)VFE_Beautyshot_GammaTable, gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
    gamma40_get_downsampled_table(&gamma->gamma_table_rgb.gamma_b[0],
      (uint16_t *)VFE_Beautyshot_GammaTable, gamma->gamma_lut_size/ISP_GAMMA_NUM_ENTRIES);
    break;

  default:
    ret = FALSE;
    break;
  }
  return ret;
}

/** gamma40_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_ext_interpolate(void *data1,
  void *data2, void *data3)
{
  gamma40_t *gamma = NULL;
  isp_sub_module_t *isp_sub_module = NULL;
  gamma_rgb_t                  gamma_Chromatixtbl_start_idx;
  gamma_rgb_t                  gamma_Chromatixtbl_end_idx;
  gamma_rgb_ds_t               gamma_tbl_start_idx;
  gamma_rgb_ds_t               gamma_tbl_end_idx;
  chromatix_parms_type          *chromatix_ptr = NULL;
  int                          skipRatio;

  if (!data1|| !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
  isp_sub_module = (isp_sub_module_t *) data2;

  chromatix_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: %p", chromatix_ptr);
    return FALSE;
  }
  if ((!gamma->aec_update.low_light_capture_update_flag) ||
    (!gamma->aec_update.gamma_flag)) {
    skipRatio = GAMMA_CHROMATIX_LUT_SIZE / ISP_GAMMA_NUM_ENTRIES;
    chromatix_gamma_type          *pchromatix_gamma =
      &(chromatix_ptr->chromatix_VFE.chromatix_gamma);
    if (!F_EQUAL(gamma->gamma_ratio.ratio, 0.0f)&&
      !F_EQUAL(gamma->gamma_ratio.ratio, 1.0f)){
      gamma40_chromatix_to_isp_gamma(&gamma_Chromatixtbl_start_idx,
        &(pchromatix_gamma->gamma_table[gamma->gamma_startidx]));
      gamma40_chromatix_to_isp_gamma(&gamma_Chromatixtbl_end_idx,
        &(pchromatix_gamma->gamma_table[gamma->gamma_endidx]));

      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_r[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_r[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_g[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_g[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_b[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_b[0]), skipRatio);

      gamma40_get_downsampled_table((&gamma_tbl_end_idx.gamma_r[0]),
        &(gamma_Chromatixtbl_end_idx.gamma_r[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_end_idx.gamma_g[0]),
        &(gamma_Chromatixtbl_end_idx.gamma_g[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_end_idx.gamma_b[0]),
        &(gamma_Chromatixtbl_end_idx.gamma_b[0]), skipRatio);

      gamma40_interpolate(&gamma_tbl_start_idx, &gamma_tbl_end_idx,
        &(gamma->gamma_table_rgb), gamma->gamma_ratio.ratio);
    }
    else {
      gamma40_chromatix_to_isp_gamma(&gamma_Chromatixtbl_start_idx,
        &(pchromatix_gamma->gamma_table[gamma->gamma_startidx]));

      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_r[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_r[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_g[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_g[0]), skipRatio);
      gamma40_get_downsampled_table((&gamma_tbl_start_idx.gamma_b[0]),
        &(gamma_Chromatixtbl_start_idx.gamma_b[0]), skipRatio);

      gamma40_copy_gamma(&(gamma->gamma_table_rgb), &gamma_tbl_start_idx,
        ISP_GAMMA_NUM_ENTRIES);
    }
  } else {
    // add interpolation logic when OIS and gamma flags are set
  }
  return TRUE;
}

/** gamma40_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean gamma40_get_trigger_ratio(void *data1,
  void *data2, void *data3)
{
  boolean ret = TRUE;
  uint8_t startIdx = -1, endIdx = -1;
  float trigger_start;
  float trigger_end;
  float trigger = 0;
  int i;
  trigger_ratio_t          trigger_ratio;
  trigger_point_type      *lowlight = NULL;
  trigger_point_type      *outdoor = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  aec_update_t            *new_aec_update = NULL;
  aec_update_t            *cur_aec_update = NULL;
  gamma40_t *gamma = NULL;
  isp_sub_module_t *isp_sub_module = NULL;

  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  gamma = (gamma40_t *)data1;
  isp_sub_module = (isp_sub_module_t *) data2;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_gamma = &chromatix_ptr->chromatix_VFE.chromatix_gamma;
  ISP_DBG("control_gamma %d", chromatix_gamma->control_gamma);
  new_aec_update = (aec_update_t *)data3;
  cur_aec_update = &gamma->aec_update;

  for (i = 0; i < MAX_GAMMA_REGIONS; i++) {
    if (chromatix_gamma->control_gamma == 0) {
      trigger_start = chromatix_gamma->gamma_table[i].gamma_trigger.lux_index_start;
      trigger_end = chromatix_gamma->gamma_table[i].gamma_trigger.lux_index_end;
      trigger = new_aec_update->lux_idx;
    } else if (chromatix_gamma->control_gamma == 1) {
      trigger_start = chromatix_gamma->gamma_table[i].gamma_trigger.gain_start;
      trigger_end = chromatix_gamma->gamma_table[i].gamma_trigger.gain_end;
      trigger = new_aec_update->real_gain;
    } else {
      ISP_ERR("%s: Unexpected tunning control: %d\n", __func__,
        chromatix_gamma->control_gamma);
      return false;
    }

    if (i == (MAX_GAMMA_REGIONS-1)) {
      startIdx = (MAX_GAMMA_REGIONS-1);
      endIdx = (MAX_GAMMA_REGIONS-1);
      trigger_ratio.lighting = (MAX_GAMMA_REGIONS-1);
      trigger_ratio.ratio = 1.0;
      break;
    } else if (trigger <= trigger_start) {
      startIdx = i;
      endIdx = i;
      trigger_ratio.lighting = i;
      trigger_ratio.ratio = 0.0;
      break;
    } else if (trigger < trigger_end) {
      startIdx = i;
      endIdx = i+1;
      trigger_ratio.lighting = i;
      trigger_ratio.ratio = 1.0 - (float)(trigger-trigger_start)/
                              (float)(trigger_end-trigger_start);
      break;
    }
  }
  ISP_DBG("ratio %f, trigger %f, startidx %d, endidx %d",
    trigger_ratio.ratio, trigger, startIdx, endIdx);
  if (trigger_ratio.ratio > 1.0) {
    ISP_ERR("%s: Unexpected trigger ratio: %f\n", __func__,
      trigger_ratio.ratio);
    return false;
  }
  isp_sub_module->trigger_update_pending = TRUE;
  *cur_aec_update = *new_aec_update;
  gamma->gamma_startidx = startIdx;
  gamma->gamma_endidx = endIdx;
  gamma->gamma_ratio = trigger_ratio;
  return TRUE;
}

static ext_override_func gamma_override_func_ext = {
  .ext_interpolate       = gamma40_ext_interpolate,
  .get_trigger_ratio     = gamma40_get_trigger_ratio,
  .ext_set_table         = gamma40_ext_set_table,
  .ext_set_spl_effect    = gamma40_ext_set_spl_effect,
};

boolean gamma40_fill_func_table_ext(gamma40_t *gamma)
{
  gamma->ext_func_table = &gamma_override_func_ext;
  return TRUE;
} /* bpc44_fill_func_table */
