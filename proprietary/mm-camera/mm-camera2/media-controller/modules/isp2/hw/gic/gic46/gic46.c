/* gic46.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gic46.h"
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

static void gic46_pack_lut(
  uint32_t *tbl_in, uint32_t *tbl_out, uint32_t num_entries,
  uint32_t delta_shift)
{
  uint32_t i;
  int32_t base = 0, delta = 0;
  for (i = 0; i < num_entries; i++) {
    base = tbl_in[i];
    delta = tbl_in[i+1] - base;
    tbl_out[i] = (delta << delta_shift) | base;
  }
}

static void gic46_pack_signal2_lut(
  uint32_t *tbl_in, uint32_t *tbl_out, uint32_t num_entries,
  uint32_t delta_shift)
{
  uint32_t i;
  int32_t base = 0, delta = 0;
  for (i = 0; i < num_entries; i++) {
    base = tbl_in[i];
    delta = tbl_in[i+1] - base;
    tbl_out[i] = ((uint32_t)Round(delta/2.0) << delta_shift) | base;
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
static void gic_ez_isp_update(gic46_t *mod,
  gicdiag_t  *gicDiag)
{
  ISP_GIC_CfgCmdType *gicCfg = &(mod->ISP_GICCfgCmd);

  if (gicCfg != NULL) {
    gicDiag->SoftThreshNoiseScale = gicCfg->softThNoiseScale;
    gicDiag->SoftThreshNoiseShift = gicCfg->softThNoiseShift;
    gicDiag->FilterStrength = gicCfg->filterStrength;
    gicDiag->NoiseScale0 = gicCfg->noiseScale0;
    gicDiag->NoiseScale1 = gicCfg->noiseScale1;
    gicDiag->NoiseOffset = gicCfg->noiseOffset;

    memcpy(&gicDiag->NoiseStd2LUTLevel0[0], &mod->noiseStd2Table[0],
                                                GIC_NOISE_STD2_LUT_SIZE);
    memcpy(&gicDiag->Signal2LUTLevel0[0], &mod->signalSqrTable[0],
                                                GIC_SIGNAL_SQR_LUT_SIZE);
  }
}/* gic_ez_isp_update */

/** gic46_fill_vfe_diag_data:
 *
 *  @gic: gic module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean gic46_fill_vfe_diag_data(gic46_t *mod,
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
}/* gic46_fill_vfe_diag_data */

/** gic46_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @gic_mod: gic module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gic46_do_hw_update(isp_sub_module_t *isp_sub_module,
  gic46_t *gic_mod, isp_sub_module_output_t *output)
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

  /* pack LUT for writing to DMI */
  gic46_pack_signal2_lut(gic_mod->signalSqrTable, gic_mod->packed_signalSqrTable,
    GIC_SIGNAL_SQR_LUT_SIZE, 16);
  ret = isp_sub_module_util_write_dmi(gic_mod->packed_signalSqrTable,
    GIC_SIGNAL_SQR_LUT_SIZE * sizeof(uint32_t),
    GIC_SIGNAL_SQR_LUT_BANK0 + gic_mod->ISP_GICCfgCmd.lutBankSel,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi for signalSqrTable");
    goto ERROR_FREE_PACKED_LUT;
  }

  gic46_pack_lut(gic_mod->noiseStd2Table, gic_mod->packed_noiseStd2Table,
    GIC_NOISE_STD2_LUT_SIZE, 16);
  ret = isp_sub_module_util_write_dmi(gic_mod->packed_noiseStd2Table,
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
static void gic_config_internal(gic46_t *mod,
  chromatix_GIC_type *chromatix_gic)
{
  uint32_t i = 0;
  float ratio = mod->aec_ratio;
  uint32_t noise_scale, noise_shift, filter_str;

  if (ratio > 0.0f) {
    chromatix_gic_core_type *gic_start, *gic_end;
    gic_start = &chromatix_gic->gic_config[mod->trigger_index];
    gic_end   = &chromatix_gic->gic_config[mod->trigger_index + 1];

    noise_scale = INTERPOLATE(gic_start->softthld_noise_scale,
                              gic_end->softthld_noise_scale, ratio, 8);
    noise_shift = INTERPOLATE(gic_start->softthld_noise_shift,
                              gic_end->softthld_noise_shift, ratio, 0);
    filter_str  = INTERPOLATE(gic_start->filter_strength,
                              gic_end->filter_strength, ratio, 8);

    for (i = 0; i < GIC_NOISE_STD2_LUT_SIZE_CHROMATIX; i++) {
      mod->noiseStd2Table[i] =
        INTERPOLATE(
          gic_start->noise_std2_lut_level0[i],
          gic_end->noise_std2_lut_level0[i], ratio, 0);
    }
  } else {
    chromatix_gic_core_type *gic;
    gic = &chromatix_gic->gic_config[mod->trigger_index];

    noise_scale = FLOAT_TO_Q(8, gic->softthld_noise_scale);
    noise_shift = gic->softthld_noise_shift;
    filter_str  = FLOAT_TO_Q(8, gic->filter_strength);

    for (i = 0; i < GIC_NOISE_STD2_LUT_SIZE_CHROMATIX; i++) {
      mod->noiseStd2Table[i] = gic->noise_std2_lut_level0[i];
    }
  }

  mod->ISP_GICCfgCmd.softThNoiseScale = MIN(noise_scale, 0xfff);
  mod->ISP_GICCfgCmd.softThNoiseShift = CLAMP(noise_shift, 5, 10);
  mod->ISP_GICCfgCmd.filterStrength = 0x100 - MIN(filter_str, 0x100);
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
boolean gic46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret           = TRUE;
  gic46_t                 *gic           = NULL;
  isp_private_event_t     *private_event = NULL;
  isp_sub_module_output_t *output        = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  isp_meta_entry_t        *gic_dmi_info  = NULL;
  chromatix_GIC_type      *chromatix_gic = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  gic = (gic46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gic);

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(output);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  RETURN_IF_NULL(chromatix_ptr);

  chromatix_gic = &chromatix_ptr->chromatix_VFE.chromatix_GIC;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d hw update pending %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable,
      isp_sub_module->trigger_update_pending);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((isp_sub_module->submod_trigger_enable == FALSE) ||
      (isp_sub_module->trigger_update_pending == FALSE) ) {
     goto FILL_METADATA;
  }
  gic_config_internal(gic, chromatix_gic);

  ret = gic46_do_hw_update(isp_sub_module, gic, output);
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
   gic_dmi_info->len =
   sizeof(gic->packed_noiseStd2Table) + sizeof(gic->packed_signalSqrTable);
   ISP_DBG("<new_dmi_debug> gic read length %d", gic_dmi_info->len);
   /*dmi type */
   gic_dmi_info->dump_type  = ISP_META_GIC_TBL;
   gic_dmi_info->start_addr = 0;
   output->meta_dump_params->frame_meta.num_entry++;
   memcpy(gic_dmi_info->isp_meta_dump,
      gic->packed_noiseStd2Table, sizeof(gic->packed_noiseStd2Table));
   memcpy((uint32_t*)gic_dmi_info->isp_meta_dump +
     sizeof(gic->packed_noiseStd2Table),
     gic->packed_signalSqrTable,
     sizeof(gic->packed_signalSqrTable));
 }

  if (output && isp_sub_module->vfe_diag_enable) {
    ret = gic46_fill_vfe_diag_data(gic, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: gic46_fill_vfe_diag_data");
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
static void gic46_reset(gic46_t *mod)
{
  memset(mod, 0, sizeof(gic46_t));
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
static boolean gic46_config(isp_sub_module_t *isp_sub_module, gic46_t *mod,
  chromatix_parms_type *chroma_ptr)
{
  uint32_t i;
  chromatix_GIC_type *chromatix_gic = &chroma_ptr->chromatix_VFE.chromatix_GIC;
  chromatix_GIC_reserve *gic_reserve = &chromatix_gic->gic_reserved_data;
  int  multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  boolean lsb_allignment = FALSE;

  /* configure the reserve data */
  /* noiseScale0 always divide by 4 */
  mod->ISP_GICCfgCmd.noiseScale0 = MIN(0xFFF,(gic_reserve->shot_noise_scale
                                                            >> multifactor));
  mod->ISP_GICCfgCmd.noiseScale1 = gic_reserve->fpn_noise_scale;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }

  /* Depend on HDR enabled or not */
   if (!lsb_allignment) {
     //MSB alligned data
     mod->ISP_GICCfgCmd.noiseOffset = gic_reserve->noise_offset << multifactor;
   } else {
       //LSB alligned data
     mod->ISP_GICCfgCmd.noiseOffset = gic_reserve->noise_offset >> multifactor;
   }
  for (i = 0; i < GIC_SIGNAL_SQR_LUT_SIZE_CHROMATIX; i++)
    mod->signalSqrTable[i] = gic_reserve->signal2_lut_level0[i];

  mod->aec_ratio = 0.0f;
  mod->trigger_index = NORMAL_LIGHT_INDEX;

  gic_config_internal(mod, chromatix_gic);

  isp_sub_module->trigger_update_pending = TRUE;
  return TRUE;
}

/** gic46_stats_aec_update:
 *
 *  @module: demosaic module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gic46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  uint8_t i, trigger_index;
  float ratio = 0.0f;
  gic46_t        *gic = NULL;
  stats_update_t *stats_update = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_GIC_type   *chromatix_gic = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  gic = (gic46_t *)isp_sub_module->private_data;
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

  chromatix_gic = &chromatix_ptr->chromatix_VFE.chromatix_GIC;

  if (chromatix_gic->control_gic) { /* gain controlled */
    float gain = stats_update->aec_update.sensor_gain;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *gic_trigger =
        &(chromatix_gic->gic_config[i].GIC_trigger);
      float gain_start = gic_trigger->gain_start;
      float gain_end   = gic_trigger->gain_end;
      trigger_index    = i;

      /* gain is within interpolation range, find ratio */
      if (gain > gain_start && gain < gain_end)
        ratio = (gain - gain_start) / (gain_end - gain_start);
      if (gain < gain_end)
        break;
    }
  } else { /* lux index controlled */
    float lux_index = stats_update->aec_update.lux_idx;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *gic_trigger =
        &(chromatix_gic->gic_config[i].GIC_trigger);
      float lux_start = (float)gic_trigger->lux_index_start;
      float lux_end   = (float)gic_trigger->lux_index_end;
      trigger_index   = i;

      /* lux index is within interpolation range, find ratio */
      if (lux_index > lux_start && lux_index < lux_end)
        ratio = (lux_index - lux_start) / (lux_end - lux_start);
      if (lux_index < lux_end)
        break;
    }
  }

  /* The last trigger set */
  if (trigger_index == MAX_LIGHT_TYPES_FOR_SPATIAL - 1)
    ratio = 0.0f;

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

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** gic46_streamon:
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
boolean gic46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  gic46_t                *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (gic46_t *)isp_sub_module->private_data;
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

  ret = gic46_config(isp_sub_module, mod,
    isp_sub_module->chromatix_ptrs.chromatixPtr);
  if (ret == FALSE) {
    ISP_ERR("failed: gic46_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* gic46_config */

/** gic46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean gic46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  gic46_t *mod = NULL;
  RETURN_IF_NULL(module);
  RETURN_IF_NULL(event);
  RETURN_IF_NULL(isp_sub_module);

  mod = (gic46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  gic46_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* gic46_streamoff */

/** gic_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the gic module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean gic46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  gic46_t *gic = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);

  gic = (gic46_t *)CAM_MALLOC(sizeof(gic46_t));
  RETURN_IF_NULL(gic);

  gic46_reset(gic);

  isp_sub_module->private_data = (void *)gic;

  return TRUE;
} /* gic_init */

/** gic46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void gic46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* gic46_destroy */
