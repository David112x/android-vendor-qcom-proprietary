/*
* Copyright (c) 2014-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gtm46.h"
#include "gtm_curve.h"
#include "isp_pipeline_reg.h"
#include "isp_defs.h"
#include "isp_adrc_tune_def.h"

isp_adrc_knee_lut_type gtm_adrc_tuning_config = {
    #include "isp_adrc_tune_param.h"
};

#ifdef ZZHDR_CHROMATIX_EXTN
static chromatix_hdr_ext_type zzHDR_chromatix = {
    #include "zzHDR_chromatix_data.h"
};
#endif

#ifndef MIN
  #define MIN(a,b)          (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b)          (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
  #define CLAMP(x,min,max)  MAX (MIN (x, max), min)
#endif

static uint32_t gtm_xin_tbl[] = {
  0,    2,    4,    7,    13,    21,    31,    45,
  62,   83,   107,  137,  169,   201,   233,   265,
  297,  329,  361,  428,  463,   499,   538,   578,
  621,  665,  712,  760,  811,   863,   919,   976,
  1037, 1098, 1164, 1230, 1300,  1371,  1447,  1524,
  1605, 1687, 1774, 1861, 1954,  2047,  2146,  2245,
  2456, 2679, 2915, 3429, 3999,  4630,  5323,  6083,
  6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
  16383};

/** gtm46_curve_debug:
 *
 *  @isp_sub_module: isp sub module handle
 *  @gtm_mod: gtm module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void gtm46_curve_debug(gtm46_t *gtm_mod)
{
  int i = 0;

  ISP_DBG("==== GTM CURVE ====");
  for (i = 0; i < (GTM_LUT_NUM_BIN-1); i++) {
    ISP_DBG("gtm tbl[%d] %llx", i, gtm_mod->dmi_tbl[i]);
  }
}

/** gtm46_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @gtm_mod: gtm module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_do_hw_update(isp_sub_module_t *isp_sub_module,
  gtm46_t *gtm_mod, isp_sub_module_output_t *output)
{
  boolean                      ret         = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd     = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update   = NULL;
  ISP_GTM_Cfg                  cfg_mask;

  cfg_mask.fields.lutBankSel |= ~0;

  if (GTM_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GTM_CGC_OVERRIDE_REGISTER, GTM_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)
    CAM_MALLOC(sizeof(struct msm_vfe_cfg_cmd_list));
  if (!hw_update) {
    ISP_ERR("failed allocating for hw_update");
    return FALSE;
  }
  memset(hw_update, 0, sizeof(struct msm_vfe_cfg_cmd_list));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd =
    (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed allocating for reg_cfg_cmd");
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, sizeof(struct msm_vfe_reg_cfg_cmd));

  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_GTM_OFF;
  reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.bytes.value;
  reg_cfg_cmd[0].u.mask_info.val  = gtm_mod->reg_cfg.bytes.value;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_FREE_REG_CFG_CMD;
  }

  gtm46_curve_debug(gtm_mod);

  ret = isp_sub_module_util_write_dmi(gtm_mod->dmi_tbl,
    sizeof(gtm_mod->dmi_tbl),
    GTM_LUT_RAM_BANK0 + gtm_mod->reg_cfg.fields.lutBankSel,
    VFE_WRITE_DMI_64BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi");
    goto ERROR_FREE_REG_CFG_CMD;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR_FREE_REG_CFG_CMD;
  }

  gtm_mod->reg_cfg.fields.lutBankSel ^= 1;

  if (GTM_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GTM_CGC_OVERRIDE_REGISTER, GTM_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;
  return TRUE;

ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
  return FALSE;
}

/** gtm46_pack_dmi:
 *
 *  Return TRUE on success and FALSE on failure
 **/
void gtm46_pack_dmi(double *yout_tbl, uint32_t *xin_tbl,
  uint64_t *dmi_tbl, uint32_t *base, int32_t *slope)
{
  uint32_t i;
  uint32_t gains[GTM_LUT_NUM_BIN];
  uint8_t max_gain = 0;
  uint64_t gtm_max = 0;
  char value[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.maxgain.enable", value, "0");
  max_gain = atoi(value);
  if(max_gain ==0) {
    max_gain = 18;
    gtm_max = (1 << max_gain);
  } else if(max_gain == 1) {
    max_gain = 17;
    gtm_max = (1 << max_gain);
  } else if(max_gain == 2) {
    max_gain = 16;
    gtm_max = 20 * (1 << 12);
  }
  ISP_DBG("\n BAND is %d", max_gain);

  for (i = 0; i < GTM_LUT_NUM_BIN; i++) {
    /* compute gain base: (Yout / Xin) */
    gains[i] = FLOAT_TO_Q(12, yout_tbl[i] / (xin_tbl[i] ? xin_tbl[i] : 1));
    /* Clamp to 0 and 2^18-1 */
    gains[i] = CLAMP(gains[i], 0, gtm_max - 1);
  }

  for (i = 0; i < GTM_LUT_NUM_BIN-1; i++) {
    int32_t slope_loc;
    int32_t x_delta = xin_tbl[i+1] - xin_tbl[i];
    int32_t y_delta = (int32_t)gains[i+1] - (int32_t)gains[i];

    y_delta *= (1 << 8);
    if (y_delta >= 0)
      y_delta += x_delta / 2;
    else
      y_delta -= x_delta / 2;
    slope_loc = y_delta / x_delta;
    /* Clamp to -2^25 and 2^25-1 */
    slope_loc = CLAMP(slope_loc, -(1 << 25), (1 << 25) - 1);
    base[i] = gains[i];
    slope[i] = slope_loc;
    dmi_tbl[i] = (uint64_t)gains[i] & 0x3FFFF;
    dmi_tbl[i] |= ((uint64_t)slope_loc & 0x3FFFFFF) << 32;
  }
}

/** gtm46_pack_dmi_fix_curve:
 *
 *  Return TRUE on success and FALSE on failure
 **/
void gtm46_pack_dmi_fix_curve(uint64_t *dmi_tbl,
  const uint32_t *base, const int32_t *slope)
{
  uint32_t i;

  for (i = 0; i < GTM_LUT_NUM_BIN-1; i++) {
    dmi_tbl[i] = (uint64_t)base[i] & 0x3FFFF;
    dmi_tbl[i] |= ((uint64_t)slope[i] & 0x3FFFFFF) << 32;
  }

}

/** gtm46_ez_isp_update
 *
 *  @mod: gtm module handle
 *  @gicDiag: gic Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void gtm46_ez_isp_update(gtm46_t *mod,
  chromatix_GTM *params, gtmdiag_t  *gtmDiag)
{
  ISP_GTM_Cfg *gtmCfg = &(mod->reg_cfg);

  gtmDiag->LUTBankSel = gtmCfg->fields.lutBankSel;
  gtmDiag->KeyHistBinWeight = params->gtm_reserve.GTM_key_hist_bin_weight;
  gtmDiag->KeyMaxThresh = params->gtm_reserve.GTM_key_max_th;
  gtmDiag->KeyMinThresh = params->gtm_reserve.GTM_key_min_th;
  gtmDiag->MaxValThresh = params->gtm_reserve.GTM_maxval_th;
  gtmDiag->YoutMaxVal = params->gtm_reserve.GTM_Yout_maxval;
  gtmDiag->AMiddleTone = params->gtm_core.GTM_a_middletone;
  gtmDiag->MiddleToneW = params->gtm_core.GTM_middletone_w;
  gtmDiag->TemporalW = params->gtm_core.GTM_temporal_w;

  memcpy(&gtmDiag->Xarr[0], &gtm_xin_tbl[0], (GTM_LUT_NUM_BIN-1));
  memcpy(&gtmDiag->YRatioBase[0], &mod->base[0], (GTM_LUT_NUM_BIN-1));
  memcpy(&gtmDiag->YRatioSlope[0], &mod->slope[0], (GTM_LUT_NUM_BIN-1));

}/* gtm46_ez_isp_update */

/** gtm46_fill_vfe_diag_data:
 *
 *  @gic: gic module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean gtm46_fill_vfe_diag_data(gtm46_t *mod, chromatix_GTM *params,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean  ret = TRUE;
  gtmdiag_t  *gtmDiag = NULL;
  vfe_diagnostics_t  *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    gtmDiag = &(vfe_diag->prev_gtmdiag);

    gtm46_ez_isp_update(mod, params, gtmDiag);
  }

  return ret;
}/* gtm46_fill_vfe_diag_data */


/** gtm46_derive_and_pack_lut_adrc_based:
 *
 *  @gtm: gtm module handle
 *
 *  Based on the drc_gain calculate the adrc_lut index
 *  Apply corresponding gtm_ratio
 *  Run ADRC algo and update the curve.
 *
 */
void gtm46_derive_and_pack_lut_adrc_based(gtm46_t *gtm)
{
  int32_t i;
  float cur_x, cur_y, val1, val2;
  int32_t  x_delta, y_delta;
  uint32_t Yratio_base[ISP_ADRC_LUT_LENGTH+1];
  int32_t  Yratio_slope;
  float    drc_gain = 0.0, gtm_ratio = 0.0;
  float *knee_gain;
  float *knee_index;
  uint32_t  lut_idx = 0;
  float knee_lut[ISP_ADRC_GAIN_LUT_KNEE_LENGTH];
  uint32_t gain_idx = 1;
  unsigned int lut_max_gain_idx;

  /* get the ADRC DRC gain values, updated by 3A*/
  drc_gain = gtm->aec_update.total_drc_gain;
  gtm_ratio = gtm->aec_update.gtm_ratio;

  /* get the LUT indec values based the calculation*/
  lut_idx = isp_sub_module_util_get_lut_index(drc_gain);

  knee_gain = gtm_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_gain;
  knee_index = gtm_adrc_tuning_config.adrc_knee_gain_lut[lut_idx].knee_index;
  lut_max_gain_idx = gtm_adrc_tuning_config.knee_points - 1;

  if (lut_max_gain_idx >= ISP_ADRC_GAIN_LUT_KNEE_LENGTH) {
    lut_max_gain_idx = ISP_ADRC_GAIN_LUT_KNEE_LENGTH - 1;
    ISP_ERR("lut max value exeeds the limit");
  }

    /* Updated gain lut curve (in LUT format) based on gain rate */
  isp_sub_module_util_gain_lut_with_drc_gain_weight(knee_gain, knee_index,
    ISP_ADRC_GAIN_LUT_KNEE_LENGTH, gtm_ratio, knee_lut);

  /* compute Y_ratio */
  for (i = 1; i < GTM_LUT_NUM_BIN; i++) {
    /* Map from 16384(2^14) to 64(adrc lut length */
    cur_x = (float)(gtm_xin_tbl[i] * ISP_ADRC_LUT_LENGTH)/16384.0f;

    if (cur_x > knee_index[gain_idx] && (gain_idx < lut_max_gain_idx)) {
      gain_idx += 1;
    }

    /* interpolate gain */
    val1 = knee_lut[gain_idx - 1];
    val2 = knee_lut[gain_idx];
    cur_y = isp_sub_module_util_linear_interpolate_float(cur_x,
                  knee_index[gain_idx - 1], knee_index[gain_idx], val1, val2);

    Yratio_base[i] = (unsigned int)(cur_y / cur_x * 4096.0f + 0.5f);

    /* Clamp to 0 and 2^18-1 */
    Yratio_base[i] = CLAMP(Yratio_base[i], 0, (1 << 18) - 1);
  }

  Yratio_base[0] = Yratio_base[1];

  /* compute Y_ratio_slope */
  for (i = 0; i < GTM_LUT_NUM_BIN-1; i++) {
    x_delta = gtm_xin_tbl[i+1] - gtm_xin_tbl[i];

    if (x_delta > 0) {
      y_delta = ((int32_t)Yratio_base[i+1] -
                 (int32_t)Yratio_base[i]) * (1 << 8);

      if (y_delta >= 0)
        y_delta += x_delta / 2;
      else
        y_delta -= x_delta / 2;
      Yratio_slope = y_delta / x_delta;
    } else {
      ISP_ERR("Error: x_delta = %d <= 0\n", x_delta);
      Yratio_slope = 0;
    }

    /* Clamp to -2^25 and 2^25-1 */
    Yratio_slope = CLAMP(Yratio_slope, -(1 << 25), (1 << 25) - 1);
    gtm->dmi_tbl[i] = (uint64_t)(Yratio_base[i] & 0x3FFFF);
    gtm->dmi_tbl[i] |= ((uint64_t)(Yratio_slope &  0x03FFFFFF )) << 32;
  }
  return;
}

/** gtm_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using aec_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret               = TRUE;
  gtm46_t                 *gtm               = NULL;
  isp_private_event_t     *private_event     = NULL;
  isp_sub_module_output_t *output            = NULL;
  chromatix_parms_type    *chromatix_ptr     = NULL;
  isp_gtm_algo_params_t   *algo_param        = NULL;
  isp_meta_entry_t        *gtm_dmi_info      = NULL;
  int8_t                  module_enable;


  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  gtm = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gtm);

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(output);

  algo_param = &(output->algo_params->gtm);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(chromatix_ptr);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
    isp_sub_module->chromatix_module_enable) {
    module_enable = (isp_sub_module->manual_ctrls.tonemap_mode ==
                     CAM_TONEMAP_MODE_CONTRAST_CURVE) ? FALSE : TRUE;
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;

      if (!isp_sub_module->submod_enable) {
        PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
        return ret;
      }
    }
  }

  if ((isp_sub_module->submod_enable == FALSE) ||
      (isp_sub_module->submod_trigger_enable == FALSE)) {
    ISP_DBG("enable = %d, trigger_enable = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->trigger_update_pending == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d trigger_update_pending %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable,
      isp_sub_module->trigger_update_pending);
    goto FILL_METADATA;
  }

  if (gtm46_fill_dmi_table(module,
       gtm, algo_param, isp_sub_module, chromatix_ptr, event) == FALSE)
  goto FILL_METADATA;

  isp_sub_module->trigger_update_pending = FALSE;

  ret = gtm46_do_hw_update(isp_sub_module, gtm, output);
  if (ret == FALSE) {
    ISP_ERR("failed: gtm_do_hw_update ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable &&
      (gtm->use_gtm_fix_curve == 0)) {
    ret = gtm46_fill_vfe_diag_data(gtm, &algo_param->params,
                                   isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: gtm46_fill_vfe_diag_data");
    }
  }

  if (output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    gtm_dmi_info = &output->
      meta_dump_params->meta_entry[ISP_META_GTM_TBL];
    gtm_dmi_info->len =
      sizeof(gtm->dmi_tbl);
    /*dmi type */
    gtm_dmi_info->dump_type  = ISP_META_GTM_TBL;
    gtm_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;
    memcpy(gtm_dmi_info->isp_meta_dump,
      gtm->dmi_tbl,
      sizeof(gtm->dmi_tbl));

    output->meta_dump_params->frame_meta.adrc_info.gtm_ratio =
     gtm->aec_update.gtm_ratio;
    output->meta_dump_params->frame_meta.adrc_info.reserved_data[0] =
      gtm->aec_update.total_drc_gain;

  }

  if (output->frame_meta)
    output->frame_meta->sensor_hdr =
      gtm->sensor_hdr_mode;
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG && output->hdr_data) {
    output->hdr_data->gtm_table.is_enable = TRUE;
    output->hdr_data->gtm_table.size = GTM_LUT_NUM_BIN-1;
    memcpy(&output->hdr_data->gtm_table.base, &gtm->base,
      sizeof(gtm->base));
    memcpy(&output->hdr_data->gtm_table.slope, &gtm->slope,
    sizeof(gtm->slope));
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* gtm_trigger_update */


/** gtm46_calculate_default_cuve:
 *
 *  @gtm: gtm module handle
 *  @algo_param: algo params
 *  Calculate the default curve, if the algo update is not yet
 *  available
 *
 *  Return TRUE on success and FALSE on failure
 **/

boolean gtm46_calculate_default_cuve(gtm46_t *gtm,
      isp_gtm_algo_params_t   *algo_param)
{
  float max_v, key;
  int32_t max_v_hist = 0;
  float middletone;
  int32_t yout_maxval;
  uint32_t i = 0;

  RETURN_IF_NULL(gtm);
  RETURN_IF_NULL(algo_param);

  max_v_hist  = algo_param->prev_max_v_hist;
  key         = algo_param->prev_key;
  middletone  = algo_param->params.gtm_core.GTM_a_middletone;
  yout_maxval = algo_param->params.gtm_reserve.GTM_Yout_maxval;

  max_v = (middletone * max_v_hist) / key;
  /* compute yout */
  for (i = 0; i < sizeof(gtm_xin_tbl) / sizeof(uint32_t); i++) {
    int X_tmp = MAX(1, gtm_xin_tbl[i]);
    float xin = (middletone * X_tmp) / key;
    gtm->algo_output.gtm_yout[i] = yout_maxval * (1.0f + xin / (max_v * max_v)) * xin / (1.0f + xin);
    ISP_DBG("yout %lf ",gtm->algo_output.gtm_yout[i] );
  }

  return TRUE;
}
/** gtm_reset:
 *
 *  @mod: Pointer to gtm module
 *
 *  Perform reset of gtm module
 *
 *  Return void
 **/
static void gtm46_reset(gtm46_t *mod)
{
  uint32_t i;
  memset(mod, 0, sizeof(gtm46_t));
  /* reset default algo look up table - unity gain */
  mod->use_gtm_fix_curve = USE_GTM46_FIXED_CURVE;
  for (i = 0; i < sizeof(gtm_xin_tbl) / sizeof(uint32_t); i++)
    mod->algo_output.gtm_yout[i] = (float)gtm_xin_tbl[i];
}

/** gtm46_stats_aec_update:
 *
 *  @module: demosaic module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean         ret = FALSE;
  gtm46_t        *gtm = NULL;
  stats_update_t *stats_update = NULL;
  boolean  module_enable;
  aec_manual_update_t *manual_stats_update  = NULL;
  stats_update_t       new_aec_stats;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  gtm = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gtm);

  if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE) {
    manual_stats_update = (aec_manual_update_t *)event->u.module_event.module_event_data;
    if (!manual_stats_update){
      ISP_ERR("failed: stats_update %p", stats_update);
      return FALSE;
    }
    memset(&new_aec_stats, 0, sizeof(stats_update_t));
    new_aec_stats.aec_update.exif_iso = manual_stats_update->exif_iso;
    new_aec_stats.aec_update.linecount = manual_stats_update->linecount;
    new_aec_stats.aec_update.lux_idx   = manual_stats_update->lux_idx;
    new_aec_stats.aec_update.sensor_gain = manual_stats_update->sensor_gain;
    new_aec_stats.aec_update.real_gain = manual_stats_update->sensor_gain;
    new_aec_stats.aec_update.total_drc_gain = 1.0;
    new_aec_stats.aec_update.gtm_ratio = -1;
    new_aec_stats.aec_update.ltm_ratio = -1;
    new_aec_stats.aec_update.la_ratio = -1;
    new_aec_stats.aec_update.gamma_ratio = -1;
    new_aec_stats.aec_update.color_drc_gain = 1.0;
  } else if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_UPDATE) {
    stats_update = (stats_update_t *)event->u.module_event.module_event_data;
    if (!stats_update){
      ISP_ERR("failed: stats_update %p", stats_update);
      return FALSE;
    }
    new_aec_stats = *stats_update;
  }

  ISP_DBG("STATS G %f LC %d EXP_TIME %f SGAIN %f TOTDRC-GAIN %f GTM %f"
          " LTM %f LA %f GA %f COLOR_DRC %f)",
    new_aec_stats.aec_update.real_gain,
    new_aec_stats.aec_update.linecount,
    new_aec_stats.aec_update.exp_time,
    new_aec_stats.aec_update.sensor_gain,
    new_aec_stats.aec_update.total_drc_gain,
    new_aec_stats.aec_update.gtm_ratio,
    new_aec_stats.aec_update.ltm_ratio,
    new_aec_stats.aec_update.la_ratio,
    new_aec_stats.aec_update.gamma_ratio,
    new_aec_stats.aec_update.color_drc_gain);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  module_enable = isp_sub_module->submod_enable;

  gtm->hdr_sensitivity_ratio = new_aec_stats.aec_update.hdr_sensitivity_ratio;

  gtm->enable_adrc =
    isp_sub_module_util_is_adrc_mod_enable(new_aec_stats.aec_update.gtm_ratio,
      new_aec_stats.aec_update.total_drc_gain);

  if (gtm->enable_adrc == TRUE) {
    if (!F_EQUAL(new_aec_stats.aec_update.total_drc_gain,
        gtm->aec_update.total_drc_gain) ||
        !F_EQUAL(gtm->aec_update.gtm_ratio, new_aec_stats.aec_update.gtm_ratio)) {
      gtm->aec_update = new_aec_stats.aec_update;
      isp_sub_module->trigger_update_pending = TRUE;
    }
    module_enable = TRUE;
  } else {
    if (!F_EQUAL(gtm->aec_update.lux_idx, new_aec_stats.aec_update.lux_idx)     ||
        !F_EQUAL(gtm->aec_update.real_gain, new_aec_stats.aec_update.real_gain) ||
        !F_EQUAL(gtm->aec_update.hdr_sensitivity_ratio,
               new_aec_stats.aec_update.hdr_sensitivity_ratio)) {
      gtm->aec_update = new_aec_stats.aec_update;
      isp_sub_module->trigger_update_pending = TRUE;
      gtm->is_aec_update_valid = 1;
      module_enable = TRUE;
    }
    if (!isp_sub_module->chromatix_module_enable) {
      module_enable = FALSE;
    }
  }

  if (isp_sub_module->submod_enable != module_enable) {
    isp_sub_module->submod_enable = module_enable;
    isp_sub_module->update_module_bit = TRUE;
  }

  ret = TRUE;
error:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** gtm46_algo_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function handles the algo update event
 *  by saving the result to the module internal storage.
 *
 **/
boolean gtm46_algo_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  gtm46_t  *mod = NULL;
  isp_saved_gtm_params_t *algo_result;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  algo_result =
    (isp_saved_gtm_params_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(algo_result);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  mod->algo_output = *algo_result;
  mod->algo_update_valid  = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}


/** gtm46_fetch_gtm_data:
 *
 *  @module: mct module
 *  @event: mct event
 *  @isp_sub_module: isp_sub_module_t
 *  @isp_sub_module:
 *  This function Fills the blacklevel offset
 *  Return: TRUE on success
 */
boolean gtm46_fetch_gtm_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  gtm46_t            *gtm = NULL;
  isp_private_event_t        *private_event = NULL;
  gtm_data_t                *gtm_data = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (!isp_sub_module->private_data ||
      !event->u.module_event.module_event_data) {
    ISP_ERR("failed: mod %p module event data %p", isp_sub_module->private_data,
      event->u.module_event.module_event_data);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  gtm = (gtm46_t *)isp_sub_module->private_data;
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;

  if (private_event->data_size != sizeof(gtm_data_t)) {
    /* size mismatch */
    ISP_ERR("size mismatch, expecting = %zu, received = %u",
      sizeof(uint32_t), private_event->data_size);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  gtm_data = (gtm_data_t *)private_event->data;
  if (!gtm_data) {
    ISP_ERR("failed: gtm_data %p", gtm_data);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  memcpy(gtm_data, &gtm->gtm_data, sizeof(gtm_data_t));
  //ISP_DBG("offset %d", *blklvl_offset); //TBD
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}


/** gtm46_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration during first
 *  stream ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gtm46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  gtm46_t               *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("gtm enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* gtm46_config */

/** gtm46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gtm46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  gtm46_t *mod = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  gtm46_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* gtm46_streamoff */

/** gtm46_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gtm46_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  gtm46_t             *gtm = NULL;
  modulesChromatix_t *chromatix_ptrs = NULL;
  chromatix_parms_type       *chromatix_param = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  gtm = (gtm46_t *)isp_sub_module->private_data;
  if (!gtm) {
    ISP_ERR("failed: mod %p", gtm);
    return FALSE;
  }
  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  chromatix_param =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_param) {
    ISP_ERR("Null pointer! %p", chromatix_param);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  ret = gtm46_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
    isp_sub_module->submod_enable = FALSE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

#ifdef ZZHDR_CHROMATIX_EXTN
  gtm->zzhdr_chromatix_extn = &zzHDR_chromatix;
#endif
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* gtm44_set_chromatix_ptr */

/** gtm46_set_hdr_mode:
*
*  @module:
*  @isp_sub_module: isp sub module handle
*  @event: module event data
*
*  This function sets hdr mode.
*
*  Return: TRUE on success and FALSE on failure
**/
boolean gtm46_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_event_module_t *module_event = NULL;
  isp_hdr_mode_t     *isp_hdr_mode = NULL;
  gtm46_t            *gtm = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  module_event = &event->u.module_event;

  if (!module_event) {
    ISP_ERR("failed: module_event %p", module_event);
    return FALSE;
  }
  gtm = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gtm);


  isp_hdr_mode = ((isp_hdr_mode_t *)module_event->module_event_data);
  ISP_DBG("hdr_mode %d ", isp_hdr_mode->hdr_mode);
  RETURN_IF_NULL(isp_hdr_mode);
  isp_sub_module->hdr_mode = isp_hdr_mode->hdr_mode;

  gtm->sensor_hdr_mode = isp_hdr_mode->hdr_mode;
  gtm->use_gtm_fix_curve = USE_GTM46_FIXED_CURVE;
  if (isp_hdr_mode->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_hdr_mode->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    isp_sub_module->submod_enable = TRUE;
    gtm->use_gtm_fix_curve = isp_hdr_mode->use_gtm_fix_curve;
  }

  ISP_DBG("set_hdr_fix curve %d ; ", gtm->use_gtm_fix_curve);

  return TRUE;
}


/** gtm_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the gtm module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean gtm46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  gtm46_t *gtm = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);

  gtm = (gtm46_t *)CAM_MALLOC(sizeof(gtm46_t));
  if (!gtm) {
    ISP_ERR("failed: gtm %p", gtm);
    return FALSE;
  }

  gtm46_reset(gtm);
  isp_sub_module->private_data = (void *)gtm;

  FILL_FUNC_TABLE__HDR_GTM(gtm);
  FILL_FUNC_TABLE_GTM(gtm);
  return TRUE;
}/* gtm_init */

/** gtm46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void gtm46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (isp_sub_module && isp_sub_module->private_data) {
    free(isp_sub_module->private_data);
    isp_sub_module->private_data = NULL;
  }
} /* gtm46_destroy */


static ext_override_gtm_hdr_func gtm_override_hdr_func = {
  .gtm_trigger_params = NULL,
};

/** gtm46_fill_func_table_hdr:
 *
 *  @gtm: gtm module handle
 *  fill function table for overiding functions.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_fill_func_table_hdr(gtm46_t *gtm)
{
  gtm->ext_func_hdr_table = &gtm_override_hdr_func;
  return TRUE;
} /* gtm46_fill_func_table_hdr */

#if !OVERRIDE_FUNC
static ext_override_gtm_func gtm_override_func = {
  .gtm_process_adj_regions = gtm46_v2_process_adj_regions,
  .gtm_apply_hdr_effects = gtm46_apply_hdr_effects,
  .gtm_apply_hdr_dmi = gtm46_2d_apply_dmi,
};

/** gtm46_fill_func_table:
 *
 *  @gtm: gtm module handle
 *  fill function table for overiding functions.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_fill_func_table(gtm46_t *gtm)
{
  gtm->ext_func_table = &gtm_override_func;
  return TRUE;
} /* gtm46_fill_func_table */
#endif
