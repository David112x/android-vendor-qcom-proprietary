/* gic48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>
#include "isp_defs.h"

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gic48.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GIC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GIC, fmt, ##args)

#define NORMAL_LIGHT_INDEX 4
#define CLAMP(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))
#define INTERPOLATE(v1, v2, ratio, q_factor) \
 FLOAT_TO_Q(q_factor, ((v1) * (1.0f - (ratio)) + (v2) * (ratio)))

static void gic48_param_debug(gic48_t *mod)
{
  int i = 0;
  ISP_DBG("noiseOffset = %u", mod->ISP_GICCfgCmd.noiseOffset);
  ISP_DBG("filterStrength = %u", mod->ISP_GICCfgCmd.filterStrength);
  ISP_DBG("GIC noiseSTDTable Values");
  for (i = 0; i < GIC_NOISE_STD_LUT_SIZE_CHROMATIX; i++) {
      ISP_DBG("0x%x", mod->noiseStdTable[i]);
  }
}

static void gic48_pack_lut(
  uint32_t *tbl_in, uint32_t *tbl_out, uint32_t num_entries,
  uint32_t delta_shift)
{
  uint32_t i;
  int32_t base = 0, delta = 0;
  for (i = 0; i < num_entries; i++) {
    base = tbl_in[i];
    delta = tbl_in[i+1] - base;
    delta = CLAMP(delta, GIC2_NOISESTDLUT_DELTA_MIN, GIC2_NOISESTDLUT_DELTA_MAX);
    tbl_out[i] = (delta << delta_shift) | base;
  }
}

/** gic_ez_isp_update
 *
 *  @mod: gic module handle
 *  @gicDiag: gic Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void gic_ez_isp_update(gic48_t *mod,
  gicdiag_t  *gicDiag)
{
  ISP_GIC_CfgCmdType *gicCfg = &(mod->ISP_GICCfgCmd);

  if (gicCfg != NULL) {
    gicDiag->FilterStrength = gicCfg->filterStrength;
    gicDiag->NoiseOffset = gicCfg->noiseOffset;

    memcpy(&gicDiag->NoiseStd2LUTLevel0[0], &mod->noiseStdTable[0],
      GIC_NOISE_STD2_LUT_SIZE);
  }
}/* gic_ez_isp_update */

/** gic48_fill_vfe_diag_data:
 *
 *  @gic: gic module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean gic48_fill_vfe_diag_data(gic48_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean  ret = TRUE;
  gicdiag_t  *gicDiag = NULL;
  vfe_diagnostics_t  *vfe_diag = NULL;

  if (sub_module_output && sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    gicDiag = &(vfe_diag->prev_gicdiag);

    gic_ez_isp_update(mod, gicDiag);
  }

  return ret;
}/* gic48_fill_vfe_diag_data */

/** gic48_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @gic_mod: gic module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gic48_do_hw_update(isp_sub_module_t *isp_sub_module,
  gic48_t *gic_mod, isp_sub_module_output_t *output)
{
  boolean                      ret           = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd       = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd   = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update     = NULL;
  ISP_GIC_CfgCmdType          *ISP_GICCfgCmd = NULL;

  if (GIC_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GIC_CGC_OVERRIDE_REGISTER, GIC_CGC_OVERRIDE_BIT, TRUE);
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

  ISP_GICCfgCmd = (ISP_GIC_CfgCmdType *)
    CAM_MALLOC(sizeof(ISP_GIC_CfgCmdType));
  if (!ISP_GICCfgCmd) {
    ISP_ERR("failed allocating for ISP_GICCfgCmd");
    goto ERROR_FREE_REG_CFG_CMD;
  }
  memset(ISP_GICCfgCmd, 0, sizeof(ISP_GIC_CfgCmdType));

  *ISP_GICCfgCmd = gic_mod->ISP_GICCfgCmd;
  cfg_cmd->cfg_data = (void *)ISP_GICCfgCmd;
  cfg_cmd->cmd_len = sizeof(*ISP_GICCfgCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_GIC_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_GIC_LEN * sizeof(uint32_t);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_FREE_PACKED_LUT;
  }

  gic48_pack_lut(gic_mod->noiseStdTable, gic_mod->packed_noiseStdTable,
    GIC_NOISE_STD2_LUT_SIZE, 14);
  ret = isp_sub_module_util_write_dmi(gic_mod->packed_noiseStdTable,
    GIC_NOISE_STD2_LUT_SIZE * sizeof(uint32_t),
    GIC_NOISE_STD2_LUT_BANK0 + gic_mod->ISP_GICCfgCmd.lutBankSel,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi for noiseStd2Table");
    goto ERROR_FREE_PACKED_LUT;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR_FREE_PACKED_LUT;
  }

  gic_mod->ISP_GICCfgCmd.lutBankSel ^= 1;

  if (GIC_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      GIC_CGC_OVERRIDE_REGISTER, GIC_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;
  gic_mod->apply_hdr_effects = 0;
  return TRUE;

ERROR_FREE_PACKED_LUT:
ERROR_FREE_GIC_CFG_CMD:
  free(ISP_GICCfgCmd);
ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
ERROR_NO_FREE:
  return FALSE;
}

/** gic_config_internal:
 *
 *    @mod: Pointer to gic module
 *    @ratio: interpolation ratio
 *    @gicChromatixPtr: chromatix gic type
 **/
static void gic_config_internal(gic48_t *mod,
  chromatix_GIC_v2_type *chromatix_gic, float multifactor)
{
  uint32_t i = 0;
  float ratio = mod->aec_ratio;
  //float noise_offset, filter_str, denoise_scale;
  float tmp_val = 0.0f;
  chromatix_gic_v2_core_type *gic, *gic_start, *gic_end;

  if (ratio > 0.0f) {
    gic_start = &chromatix_gic->gic_config[mod->trigger_index];
    gic_end   = &chromatix_gic->gic_config[mod->trigger_index + 1];

    mod->noise_offset = INTERPOLATE(
      ((float)gic_start->thin_line_noise_offset * multifactor),
      ((float)gic_end->thin_line_noise_offset * multifactor), ratio, 0);

    mod->filter_str  = (float)LINEAR_INTERPOLATION(gic_start->filter_strength,
      (float)gic_end->filter_strength, (float)(1 - ratio));
    mod->filter_str *= 256;


    mod->denoise_scale =(float)LINEAR_INTERPOLATION((float)gic_start->denoise_scale,
     (float)gic_end->denoise_scale, (float)(1 - ratio));
     mod->denoise_scale *= 256;

    if ((mod->ext_func_table && mod->ext_func_table->apply_2d_interpolation)
      && (mod->apply_hdr_effects)) {
      mod->ext_func_table->apply_2d_interpolation(mod);
    }

    for (i = 0; i < GIC_NOISE_STD_LUT_SIZE_CHROMATIX; i++) {
      tmp_val = (float)
        LINEAR_INTERPOLATION(
          (float)gic_start->noise_std_lut_level0[i],
          (float)gic_end->noise_std_lut_level0[i], (float)(1 - ratio));
      tmp_val = CLAMP(tmp_val, 0.0f, GIC2_NOISE_OFFSET_MAX_VAL_CHROMATIX);
      /* 16 * tmp * denoise_scale / 256 */
      tmp_val = (tmp_val * mod->denoise_scale) / 16.0f;
      tmp_val = Round(tmp_val);
      mod->noiseStdTable[i] = CLAMP(tmp_val, GIC2_NOISE_STD_LUT_LEVEL0_MIN,
        GIC2_NOISE_STD_LUT_LEVEL0_MAX);
    }
  } else {
    gic = &chromatix_gic->gic_config[mod->trigger_index];
    mod->noise_offset = (float)gic->thin_line_noise_offset * multifactor;
    mod->filter_str  = gic->filter_strength;
    mod->denoise_scale =  gic->denoise_scale;
    if ((mod->ext_func_table && mod->ext_func_table->apply_2d_interpolation)
      && (mod->apply_hdr_effects)) {
      mod->ext_func_table->apply_2d_interpolation(mod);
    }

    mod->filter_str  *= 256;
    mod->denoise_scale *= 256;
    for (i = 0; i < GIC_NOISE_STD_LUT_SIZE_CHROMATIX; i++) {
      tmp_val = (float)gic->noise_std_lut_level0[i];
      tmp_val = CLAMP(tmp_val, 0.0f, GIC2_NOISE_OFFSET_MAX_VAL_CHROMATIX);
      tmp_val = (tmp_val * mod->denoise_scale) / 16.0f;
      mod->noiseStdTable[i] = CLAMP(roundf(tmp_val), GIC2_NOISE_STD_LUT_LEVEL0_MIN,
        GIC2_NOISE_STD_LUT_LEVEL0_MAX);
    }
  }

  mod->noise_offset = MIN(mod->noise_offset, GIC2_THINLINE_OFF_MAX);
  mod->ISP_GICCfgCmd.noiseOffset = FLOAT_TO_Q(0, mod->noise_offset);

  mod->filter_str = MIN(mod->filter_str, GIC2_FILTER_STRENGTH_MAX);
  /* Flip filter strength to have same effect like ABF/wavelet */
  mod->ISP_GICCfgCmd.filterStrength = FLOAT_TO_Q(0, GIC2_FILTER_STRENGTH_MAX - mod->filter_str);

  gic48_param_debug(mod);
}

/** gic48_decide_hysterisis:
 *
 *  @isp_sub_module: sub module handle
 *  @aec_ref: AEC reference
 *
 * Handle hysterisis
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean gic48_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref)
{
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_GIC_v2_type *chromatix_gic = NULL;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_gic =
    &chromatix_ptr->chromatix_VFE.chromatix_GIC2_data;

  return isp_sub_module_util_decide_hysterisis(isp_sub_module,
    aec_ref, chromatix_gic->gic_enable, FALSE,
    chromatix_gic->control_gic, &chromatix_gic->gic_hysteresis_pt);
}

/** gic_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using aec_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gic48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret           = TRUE;
  gic48_t                 *gic           = NULL;
  isp_private_event_t     *private_event = NULL;
  isp_sub_module_output_t *output        = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  isp_meta_entry_t        *gic_dmi_info  = NULL;
  chromatix_GIC_v2_type   *chromatix_gic = NULL;
  float                    multifactor    = 1.0f;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  gic = (gic48_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gic);

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(output);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(chromatix_ptr);

  chromatix_gic = &chromatix_ptr->chromatix_VFE.chromatix_GIC2_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);


  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d hw update pending %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable,
      isp_sub_module->trigger_update_pending);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (((isp_sub_module->submod_trigger_enable == FALSE) ||
      (isp_sub_module->trigger_update_pending == FALSE)) &&
	  (!gic->apply_hdr_effects)) {
     goto FILL_METADATA;
  }

  gic_config_internal(gic, chromatix_gic, gic->multifactor);

  ret = gic48_do_hw_update(isp_sub_module, gic, output);
  if (ret == FALSE) {
    ISP_ERR("failed: gic_do_hw_update ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

FILL_METADATA:
  if (output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    gic_dmi_info = &output->
    meta_dump_params->meta_entry[ISP_META_GIC_TBL];
    /*dmi table length*/
    gic_dmi_info->len = sizeof(gic->packed_noiseStdTable);
    /*dmi type */
    gic_dmi_info->dump_type = ISP_META_GIC_TBL;
    gic_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;
    memcpy(gic_dmi_info->isp_meta_dump,
      gic->packed_noiseStdTable, gic_dmi_info->len);
}

  if (output && isp_sub_module->vfe_diag_enable) {
    ret = gic48_fill_vfe_diag_data(gic, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: gic48_fill_vfe_diag_data");
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* gic_trigger_update */

/** gic_reset:
 *
 *  @mod: Pointer to gic module
 *
 *  Perform reset of gic module
 *
 *  Return void
 **/
static void gic48_reset(gic48_t *mod)
{
  memset(mod, 0, sizeof(gic48_t));
}

/** gic_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to gic module
 *  @chroma_ptr: chromatix ptr
 *
 *  Configure the gic module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gic48_config(isp_sub_module_t *isp_sub_module, gic48_t *mod,
  chromatix_parms_type *chroma_ptr)
{
  uint32_t i;
  chromatix_GIC_v2_type *chromatix_gic =
    &chroma_ptr->chromatix_VFE.chromatix_GIC2_data;

  /* when HDR mode, chromatix tuned by 14 bit,
     otherwise keep 12 bit header for backward compatible */
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    mod->multifactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  } else {
    mod->multifactor = 1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH);
  }

  mod->aec_ratio = 0.0f;
  mod->trigger_index = NORMAL_LIGHT_INDEX;

  gic_config_internal(mod, chromatix_gic, mod->multifactor);

  isp_sub_module->trigger_update_pending = TRUE;
  return TRUE;
}

/** gic48_stats_aec_update:
 *
 *  @module: demosaic module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gic48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  uint8_t i, trigger_index;
  float ratio = 0.0f, aec_reference = 0.0f;
  gic48_t        *gic = NULL;
  stats_update_t *stats_update = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_GIC_v2_type   *chromatix_gic = NULL;
  boolean ret = TRUE;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  gic = (gic48_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gic);

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(stats_update);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (chromatix_ptr == NULL) {
    ISP_ERR("chromatix is NULL");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  chromatix_gic = &chromatix_ptr->chromatix_VFE.chromatix_GIC2_data;

  switch (chromatix_gic->control_gic) {
  case CONTROL_GAIN:
    aec_reference = stats_update->aec_update.sensor_gain;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *gic_trigger =
        &(chromatix_gic->gic_config[i].GIC_trigger);
      float gain_start = gic_trigger->gain_start;
      float gain_end   = gic_trigger->gain_end;
      trigger_index    = i;

      /* gain is within interpolation range, find ratio */
      if (aec_reference > gain_start && aec_reference < gain_end)
        ratio = (aec_reference - gain_start) / (gain_end - gain_start);
      if (aec_reference < gain_end)
        break;
    }
    break;
  case CONTROL_LUX_IDX:
    aec_reference = stats_update->aec_update.lux_idx;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *gic_trigger =
        &(chromatix_gic->gic_config[i].GIC_trigger);
      float lux_start = (float)gic_trigger->lux_index_start;
      float lux_end   = (float)gic_trigger->lux_index_end;
      trigger_index   = i;

      /* lux index is within interpolation range, find ratio */
      if (aec_reference > lux_start && aec_reference < lux_end)
        ratio = (aec_reference - lux_start) / (lux_end - lux_start);
      if (aec_reference < lux_end)
        break;
    }
    break;
  default:
    ISP_ERR("Incorrect control type%d", chromatix_gic->control_gic);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* The last trigger set */
  if (trigger_index == MAX_LIGHT_TYPES_FOR_SPATIAL - 1)
    ratio = 0.0f;

  gic->aec_reference = aec_reference;
  gic->aec_lux_reference = stats_update->aec_update.lux_idx;
  gic->aec_gain_reference = stats_update->aec_update.sensor_gain;
  gic->hdr_sen_trigger_ref = stats_update->aec_update.hdr_sensitivity_ratio;
  gic->hdr_time_trigger_ref = stats_update->aec_update.hdr_exp_time_ratio;

  if(!gic48_decide_hysterisis(isp_sub_module, aec_reference)){
    ISP_ERR("GIC Hysterisis failure");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (trigger_index + 1 >
      MAX_LIGHT_TYPES_FOR_SPATIAL + NUM_SCENE_MODES) {
    ISP_ERR("error in finding the correct region: trigger_index = %d\n",
            trigger_index);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (gic->trigger_index != trigger_index ||
      gic->aec_ratio != ratio)
    isp_sub_module->trigger_update_pending = TRUE;

  gic->aec_ratio = ratio;
  gic->trigger_index = trigger_index;
  
  /* when HDR mode, chromatix tuned by 14 bit,
     otherwise keep 12 bit header for backward compatible */
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    gic->multifactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  } else {
    gic->multifactor = 1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH);
  }  

  if (gic->ext_func_table && gic->ext_func_table->get_2d_interpolation) {
    ret = gic->ext_func_table->get_2d_interpolation(isp_sub_module, gic);
    if (ret == FALSE) {
      ISP_ERR("failed: get_2d_interpolation");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** gic48_streamon:
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
boolean gic48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  gic48_t                *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (gic48_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("gic enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if(!isp_sub_module->chromatix_ptrs.chromatixPtr) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    ISP_ERR("failed: NULL pointer isp_sub_module->chromatix_ptrs.chromatixPtr");
    return FALSE;
  }

  ret = gic48_config(isp_sub_module, mod,
    isp_sub_module->chromatix_ptrs.chromatixPtr);
  if (ret == FALSE) {
    ISP_ERR("failed: gic48_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* gic48_config */

/** gic48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gic48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  gic48_t *mod = NULL;
  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  mod = (gic48_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  gic48_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  FILL_FUNC_TABLE(mod);

  return TRUE;
} /* gic48_streamoff */

/** gic_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the gic module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean gic48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  gic48_t *gic = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);

  gic = (gic48_t *)CAM_MALLOC(sizeof(gic48_t));
  RETURN_IF_NULL(gic);

  gic48_reset(gic);

  isp_sub_module->private_data = (void *)gic;

  FILL_FUNC_TABLE(gic);
  return TRUE;
} /* gic_init */

/** gic48_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void gic48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* gic48_destroy */

#if !OVERRIDE_FUNC
static ext_override_func gic_override_func = {
  .get_2d_interpolation = NULL,
  .apply_2d_interpolation = NULL,
};

boolean gic48_fill_func_table(gic48_t *gic)
{
  gic->ext_func_table = &gic_override_func;
  return TRUE;
} /* abf48_fill_func_table */
#endif
