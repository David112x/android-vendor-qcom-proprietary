/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "pdaf48.h"
#include "isp_zzhdr_tune_def.h"

isp_zzhdr_2d_lut_type zzhdr_pdpc_tuning_data =
    #include "isp_zzhdr_tune_param.h"
;

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
 ISP_DBG_MOD(ISP_LOG_PDAF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
 ISP_HIGH_MOD(ISP_LOG_PDAF, fmt, ##args)

/** pdaf8_cmd_debug:
 *
 *  @cmd: pdaf config cmd
 *
 *  This function dumps the pdaf module configuration set to hw
 *
 *  Returns nothing
 **/
static void pdaf48_cmd_debug(ISP_PDAF_Reg_t *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }

  ISP_DBG("cmd->pdaf_config.pdaf_pdpc_en = %x",
          cmd->pdaf_config.pdaf_pdpc_en);
  ISP_DBG("cmd->pdaf_config.pdaf_dsbpc_en = %x",
          cmd->pdaf_config.pdaf_dsbpc_en);
  ISP_DBG("cmd->pdaf_config.blk_lvl = %x", cmd->pdaf_config.blk_lvl);

  ISP_DBG("cmd->hdr_config.hdr_exp_ratio.exp_ratio = %x",
          cmd->hdr_config.hdr_exp_ratio.exp_ratio);
  ISP_DBG("cmd->hdr_config.hdr_exp_ratio.exp_ratio_recip = %x",
          cmd->hdr_config.hdr_exp_ratio.exp_ratio_recip);

  ISP_DBG("cmd->bp_config.bp_th.fmax = %x",
          cmd->bp_config.bp_th.fmax);
  ISP_DBG("cmd->bp_config.bp_th.fmin = %x",
          cmd->bp_config.bp_th.fmin);

  ISP_DBG("cmd->bp_config.bp_offset.offset_g_pixel = %x",
          cmd->bp_config.bp_offset.offset_g_pixel);
  ISP_DBG("cmd->bp_config.bp_offset.offset_rb_pixel = %x",
          cmd->bp_config.bp_offset.offset_rb_pixel);

  ISP_DBG("cmd->bp_config.bp_offset_t2.offset_g_pixel = %x",
          cmd->bp_config.bp_offset_t2.offset_g_pixel);
  ISP_DBG("cmd->bp_config.bp_offset_t2.offset_rb_pixel = %x",
          cmd->bp_config.bp_offset_t2.offset_rb_pixel);

  ISP_DBG("cmd->wb_gain_config.rg_wb_gain.rg_wb_gain = %x",
          cmd->wb_gain_config.rg_wb_gain.rg_wb_gain);

  ISP_DBG("cmd->wb_gain_config.bg_wb_gain.bg_wb_gain = %x",
          cmd->wb_gain_config.bg_wb_gain.bg_wb_gain);

  ISP_DBG("cmd->wb_gain_config.gr_wb_gain.gr_wb_gain = %x",
          cmd->wb_gain_config.gr_wb_gain.gr_wb_gain);

  ISP_DBG("cmd->wb_gain_config.gb_wb_gain.gb_wb_gain = %x",
          cmd->wb_gain_config.gb_wb_gain.gb_wb_gain);

  ISP_DBG("cmd->loc_config.loc_offset_cfg.x_offset = %x",
          cmd->loc_config.loc_offset_cfg.x_offset);
  ISP_DBG("cmd->loc_config.loc_offset_cfg.y_offset = %x",
          cmd->loc_config.loc_offset_cfg.y_offset);

  ISP_DBG("cmd->loc_config.loc_end_cfg.x_end = %x",
          cmd->loc_config.loc_end_cfg.x_end);
  ISP_DBG("cmd->loc_config.loc_end_cfg.y_end = %x",
          cmd->loc_config.loc_end_cfg.y_end);

} /* pdaf48_cmd_debug */

/** pdaf48_write_lut_to_dmi:
 *
 *  @isp_sub_module: isp base module
 *  @pdaf: module data
 *
 *  writes pdaf LUT table to DMI
 *
 **/
boolean pdaf48_write_lut_to_dmi(isp_sub_module_t *isp_sub_module, pdaf48_t *pdaf)
{
  boolean   ret = TRUE;
  uint32_t  lut_size = sizeof(int) * PDAF_LUT_COUNT;
  ret = isp_sub_module_util_write_dmi(
    (void*)&pdaf->packed_lut, lut_size,
    PDAF_LUT_RAM_BANK,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: pdaf48 isp_sub_module_util_write_dmi");
  }
  return ret;
}

/** pdaf48_fetch_blkLvl_offset:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Black Level offset
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean pdaf48_fetch_blkLvl_offset(mct_module_t *module,
  uint32_t identity, uint32_t *BlkLvl_offset)
{
  boolean                               ret = TRUE;
  mct_event_t                           mct_event;
  isp_private_event_t                   private_event;
  uint32_t                              blklvl;

  if (!module || !BlkLvl_offset) {
    ISP_ERR("failed: %p %p", module, BlkLvl_offset);
    return FALSE;
  }

  /*Get black level offset info */
  ISP_DBG("E: Get black level offset info");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_BLKLVL_OFFSET;
  private_event.data = (void *)&blklvl;
  private_event.data_size = sizeof(uint32_t);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  ret = isp_sub_mod_util_send_event(module, &mct_event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_mod_util_send_event");
    return ret;
  }
  *BlkLvl_offset = blklvl;
  ISP_DBG("Black Level offset %d", *BlkLvl_offset);
  return ret;
}

/** pdaf48_store_hw_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @pdaf: pdaf handle
 *
 *  Create hw update list and store in isp sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean pdaf48_store_hw_update(isp_sub_module_t *isp_sub_module,
  pdaf48_t *pdaf)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_PDAF_Reg_t              *RegCmd = NULL;
  uint32_t                    *val = NULL;

  if (!isp_sub_module || !pdaf) {
    ISP_ERR("failed: %p %p", isp_sub_module, pdaf);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)
    * PDAF_TOTAL_NUM_REG);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * PDAF_TOTAL_NUM_REG);

  RegCmd = (ISP_PDAF_Reg_t *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_PDAF_CMD;
  }
  *RegCmd = pdaf->pdaf_reg;

  if (PDAF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      PDAF_CGC_OVERRIDE_REGISTER, PDAF_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }
  /* write LUT for first time */
  if (pdaf->one_time_config_done == FALSE) {
    pdaf48_write_lut_to_dmi(isp_sub_module, pdaf);
    pdaf->one_time_config_done = TRUE;
  }

  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = PDAF_TOTAL_NUM_REG;

  /* write ISP_pdaf_config register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[0],
    0, sizeof(RegCmd->pdaf_config), VFE_WRITE, PDAF_OFFSET);

  /* write ISP_pdaf_hdr_config register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1],
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len,
    sizeof(RegCmd->hdr_config), VFE_WRITE, PDAF_HDR_OFFSET);

  /* write ISP_pdaf_bp_config registers (3 registers) */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[2],
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + reg_cfg_cmd[1].u.rw_info.len,
    sizeof(RegCmd->bp_config), VFE_WRITE, PDAF_BP_OFFSET);

  /* write ISP_pdaf_wb_gain_config registers (4 registers) */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[5],
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset + reg_cfg_cmd[2].u.rw_info.len,
    sizeof(RegCmd->wb_gain_config), VFE_WRITE, PDAF_RG_OFFSET);

  /* write ISP_pdaf_loc_config registers (2 registers) */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[9],
    reg_cfg_cmd[5].u.rw_info.cmd_data_offset + reg_cfg_cmd[5].u.rw_info.len,
    sizeof(RegCmd->loc_config), VFE_WRITE, PDAF_LOC_OFFSET);

  pdaf48_cmd_debug(RegCmd);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  if (PDAF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      PDAF_CGC_OVERRIDE_REGISTER, PDAF_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }
  return TRUE;

ERROR_APPEND:
  free(RegCmd);
ERROR_PDAF_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* pdaf48_store_hw_update */

/** pdaf48_interpolate:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @pdaf: pdaf struct data
 *
 * interpolate pdaf state with the aec ratio
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean pdaf48_interpolate(isp_sub_module_t *isp_sub_mod, pdaf48_t *pdaf)
{
  chromatix_PDPC_DSBPC_core         *pdaf_data_start = NULL;
  chromatix_PDPC_DSBPC_core         *pdaf_data_end = NULL;
  chromatix_parms_type              *chromatix_ptr = NULL;
  chromatix_PDPC_DSBPC_type         *chromatix_pdpc_dsbpc = NULL;

  float                             aec_ratio_pdaf = 0;
  float                             hdr_exp_ratio = 0;
  awb_gain_t                        *cur_awb_gain = NULL;

  ISP_PDAF_Reg_t                    *pdaf_params = NULL;
  float                             pipelinebitadjfactor = 1.0f;
  pdaf_block_pattern_t              *pdaf_pattern = NULL;
  unsigned int                      x_end;
  unsigned int                      y_end;
  boolean ret = TRUE;

  if (!isp_sub_mod || !pdaf) {
    ISP_ERR("failed isp_sub_mod %p pdaf %p", isp_sub_mod, pdaf);
    return FALSE;
  }

  pdaf_params = &pdaf->pdaf_reg;
  aec_ratio_pdaf = pdaf->aec_ratio;
  hdr_exp_ratio  = pdaf->hdr_exp_ratio;
  chromatix_ptr = isp_sub_mod->chromatix_ptrs.chromatixPtr;

  if (!chromatix_ptr) {
    ISP_ERR("failed chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_pdpc_dsbpc = &chromatix_ptr->chromatix_VFE.chromatix_PDPC_data;

  if (!chromatix_pdpc_dsbpc) {
    ISP_ERR("failed chromatix_pdpc_dsbpc %p", chromatix_pdpc_dsbpc);
    return FALSE;
  }
  pdaf_pattern = &pdaf->pdaf_pattern;

  if (pdaf->trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("trigger_index is invalid %d\n", pdaf->trigger_index);
    return FALSE;
  }
  pdaf_data_start = &chromatix_pdpc_dsbpc->PDPC_config[pdaf->trigger_index];

  if (isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR){
      pipelinebitadjfactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  }
  else{
      pipelinebitadjfactor = 1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH);
  }

  pdaf_params->pdaf_config.blk_lvl = Round(pdaf->BLSS_offset * pipelinebitadjfactor);

  if ((aec_ratio_pdaf > 0) &&
      ((pdaf->trigger_index + 1) <= MAX_LIGHT_TYPES_FOR_SPATIAL)) {
    pdaf_data_end = &chromatix_pdpc_dsbpc->PDPC_config[pdaf->trigger_index + 1];

    pdaf->fmax = (LINEAR_INTERPOLATION((float)pdaf_data_end->Fmax,
     (float)pdaf_data_start->Fmax, aec_ratio_pdaf));
    pdaf_params->bp_config.bp_th.fmax = roundf(pdaf->fmax);

    pdaf_params->bp_config.bp_th.fmin = roundf(LINEAR_INTERPOLATION((float)pdaf_data_end->Fmin,
     (float)pdaf_data_start->Fmin, aec_ratio_pdaf));

    pdaf->offset_rb_pixel = (LINEAR_INTERPOLATION((((float)pdaf_data_end->bp_offset_rb)*pipelinebitadjfactor),
     (((float)pdaf_data_start->bp_offset_rb)*pipelinebitadjfactor), aec_ratio_pdaf));
    pdaf_params->bp_config.bp_offset.offset_rb_pixel = CLAMP(roundf(pdaf->offset_rb_pixel), PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);

    pdaf->offset_g_pixel = (LINEAR_INTERPOLATION((((float)pdaf_data_end->bp_offset_g)*pipelinebitadjfactor),
     (((float)pdaf_data_start->bp_offset_g)*pipelinebitadjfactor), aec_ratio_pdaf));
    pdaf_params->bp_config.bp_offset.offset_g_pixel = CLAMP(roundf(pdaf->offset_g_pixel), PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);

    pdaf->offset_rb_pixel_t2 = (LINEAR_INTERPOLATION((((float)pdaf_data_end->bp_offset_rb_T2)*pipelinebitadjfactor),
     (((float)pdaf_data_start->bp_offset_rb_T2)*pipelinebitadjfactor), aec_ratio_pdaf));
    pdaf_params->bp_config.bp_offset_t2.offset_rb_pixel = CLAMP(roundf(pdaf->offset_rb_pixel_t2), PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);

    pdaf->offset_g_pixel_t2 = (LINEAR_INTERPOLATION((((float)pdaf_data_end->bp_offset_g_T2)*pipelinebitadjfactor),
     (((float)pdaf_data_start->bp_offset_g_T2)*pipelinebitadjfactor), aec_ratio_pdaf));
    pdaf_params->bp_config.bp_offset_t2.offset_g_pixel = CLAMP(roundf(pdaf->offset_g_pixel_t2), PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);

  } else {
    pdaf_params->bp_config.bp_th.fmax                   = pdaf_data_start->Fmax;
    pdaf_params->bp_config.bp_th.fmin                   = pdaf_data_start->Fmin;
    pdaf_params->bp_config.bp_offset.offset_rb_pixel    = CLAMP((pdaf_data_start->bp_offset_rb)*pipelinebitadjfactor, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
    pdaf_params->bp_config.bp_offset.offset_g_pixel     = CLAMP((pdaf_data_start->bp_offset_g)*pipelinebitadjfactor, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
    pdaf_params->bp_config.bp_offset_t2.offset_rb_pixel = CLAMP((pdaf_data_start->bp_offset_rb_T2)*pipelinebitadjfactor, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
    pdaf_params->bp_config.bp_offset_t2.offset_g_pixel  = CLAMP((pdaf_data_start->bp_offset_g_T2)*pipelinebitadjfactor, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
    pdaf->fmax = pdaf_params->bp_config.bp_th.fmax;
    pdaf->offset_g_pixel = pdaf_params->bp_config.bp_offset.offset_g_pixel;
    pdaf->offset_rb_pixel = pdaf_params->bp_config.bp_offset.offset_rb_pixel;
    pdaf->offset_g_pixel_t2 = pdaf_params->bp_config.bp_offset_t2.offset_g_pixel;
    pdaf->offset_rb_pixel_t2 = pdaf_params->bp_config.bp_offset_t2.offset_rb_pixel;
  }

  if ((pdaf->ext_func_table && pdaf->ext_func_table->apply_2d_interpolation)
    && (pdaf->apply_hdr_effects)) {
    pdaf->ext_func_table->apply_2d_interpolation(pdaf);
  }

  pdaf_params->hdr_config.hdr_exp_ratio.exp_ratio        = CLAMP(FLOAT_TO_Q(10, hdr_exp_ratio), PDAF_MIN_HDR_EXP_RATIO, PDAF_MAX_HDR_EXP_RATIO);

  /* Calculate the reciprocal in place */
  hdr_exp_ratio = 1/hdr_exp_ratio;
  pdaf_params->hdr_config.hdr_exp_ratio.exp_ratio_recip  = CLAMP(FLOAT_TO_Q(8, hdr_exp_ratio), PDAF_MIN_HDR_EXP_RATIO_RECIP, PDAF_MAX_HDR_EXP_RATIO_RECIP);

  cur_awb_gain = &pdaf->curr_awb_gain;

  pdaf_params->wb_gain_config.rg_wb_gain.rg_wb_gain    = FLOAT_TO_Q(12, cur_awb_gain->r_gain/cur_awb_gain->g_gain);
  pdaf_params->wb_gain_config.bg_wb_gain.bg_wb_gain    = FLOAT_TO_Q(12, cur_awb_gain->b_gain/cur_awb_gain->g_gain);
  pdaf_params->wb_gain_config.gr_wb_gain.gr_wb_gain    = FLOAT_TO_Q(12, cur_awb_gain->g_gain/cur_awb_gain->r_gain);
  pdaf_params->wb_gain_config.gb_wb_gain.gb_wb_gain    = FLOAT_TO_Q(12, cur_awb_gain->g_gain/cur_awb_gain->b_gain);

  pdaf_params->loc_config.loc_offset_cfg.x_offset      = CLAMP(pdaf_pattern->pd_offset_horizontal,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);
  pdaf_params->loc_config.loc_offset_cfg.y_offset      = CLAMP(pdaf_pattern->pd_offset_vertical,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);

  x_end = ((pdaf_pattern->block_dim.width * pdaf->block_count_horizontal) + pdaf_pattern->pd_offset_horizontal);
  y_end = ((pdaf_pattern->block_dim.height * pdaf->block_count_vertical) + pdaf_pattern->pd_offset_vertical);

  pdaf_params->loc_config.loc_end_cfg.x_end            = CLAMP(x_end, PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);
  pdaf_params->loc_config.loc_end_cfg.y_end            = CLAMP(y_end, PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);

  return TRUE;
}

/** pdaf48_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  boolean                       ret = TRUE;
  pdaf48_t                      *pdaf = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_output_t       *output = NULL;
  isp_meta_entry_t              *pdpc_dmi_info = NULL;
  uint32_t              BlkLvl_offset = 0;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pdaf = (pdaf48_t *)isp_sub_module->private_data;

  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  private_event = (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d\n",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  pdaf48_fetch_blkLvl_offset(module, event->identity,
    &BlkLvl_offset);
  pdaf->BLSS_offset = BlkLvl_offset;

/* TODO - implement this
   pdaf48_decide_hysterisis(isp_sub_module, pdaf->aec_reference);
*/

  if (((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) ||
    (pdaf->apply_hdr_effects)) {
    ret = pdaf48_interpolate(isp_sub_module, pdaf);
    if (ret == FALSE) {
      ISP_ERR("failed: pdaf48_interpolate");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  /* Update the offset for dual ISP */
  if (pdaf->isp_out_info.is_split == TRUE) {
     if (pdaf->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
        pdaf->pdaf_reg.loc_config.loc_offset_cfg.x_offset = 0;
        ISP_DBG("%s: pdaf %p x_offset %d\n",__func__,pdaf,
           pdaf->pdaf_reg.loc_config.loc_offset_cfg.x_offset);
      }
    }

    isp_sub_module->trigger_update_pending = FALSE;
    pdaf->apply_hdr_effects = 0;

    pdaf48_store_hw_update(isp_sub_module, pdaf);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

FILL_METADATA:
  if (output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    pdpc_dmi_info = &output->
      meta_dump_params->meta_entry[ISP_META_PDPC_TBL];
    /*dmi table length*/
    pdpc_dmi_info->len = sizeof(pdaf->packed_lut);
    /*dmi type */
    pdpc_dmi_info->dump_type = ISP_META_PDPC_TBL;
    pdpc_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;
    ISP_DBG("<new_dmi_debug> pdpc read length %d", pdpc_dmi_info->len);
    memcpy(pdpc_dmi_info->isp_meta_dump, &pdaf->packed_lut,
      pdpc_dmi_info->len);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* pdaf48_trigger_update */


/** pdaf_config
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  config default params
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean pdaf48_config(isp_sub_module_t *isp_sub_module, pdaf48_t *pdaf)
{
  ISP_PDAF_Reg_t                    *Reg = NULL;
  chromatix_parms_type              *chromatix_ptr = NULL;
  chromatix_PDPC_DSBPC_type         *chromatix_pdpc_dsbpc = NULL;
  chromatix_PDPC_DSBPC_core         *pdpc_core_data = NULL;
  pdaf_block_pattern_t              *pdaf_pattern = NULL;

  if (!isp_sub_module || !pdaf) {
    ISP_ERR("failed: %p %p", isp_sub_module, pdaf);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_pdpc_dsbpc = &chromatix_ptr->chromatix_VFE.chromatix_PDPC_data;

  pdpc_core_data    = &chromatix_pdpc_dsbpc->PDPC_config[DEFAULT_CONFIG_INDEX];

  Reg = &pdaf->pdaf_reg;
  pdaf_pattern = &pdaf->pdaf_pattern;

  Reg->pdaf_config.pdaf_pdpc_en                = chromatix_pdpc_dsbpc->pdaf_pdpc_en;
  Reg->pdaf_config.pdaf_dsbpc_en               = chromatix_pdpc_dsbpc->pdaf_dsbpc_en;

  /* TODO - Need to initialize black_level */
  Reg->pdaf_config.blk_lvl                     = 0;

  /* Initialize HDR exp ratio */
  Reg->hdr_config.hdr_exp_ratio.exp_ratio        = FLOAT_TO_Q(10, 1.0f);
  Reg->hdr_config.hdr_exp_ratio.exp_ratio_recip  = FLOAT_TO_Q(8, 1.0f);

  /* Initialize Bad Pixel registers */
  Reg->bp_config.bp_th.fmax                    = CLAMP(pdpc_core_data->Fmax, PDAF_MIN_FMAX, PDAF_MAX_FMAX);
  Reg->bp_config.bp_th.fmin                    = CLAMP(pdpc_core_data->Fmin, PDAF_MIN_FMIN, PDAF_MAX_FMIN);
  Reg->bp_config.bp_offset.offset_rb_pixel     = CLAMP(pdpc_core_data->bp_offset_rb, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
  Reg->bp_config.bp_offset.offset_g_pixel      = CLAMP(pdpc_core_data->bp_offset_g, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
  Reg->bp_config.bp_offset_t2.offset_rb_pixel  = CLAMP(pdpc_core_data->bp_offset_rb_T2, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);
  Reg->bp_config.bp_offset_t2.offset_g_pixel   = CLAMP(pdpc_core_data->bp_offset_g_T2, PDAF_MIN_OFFSET, PDAF_MAX_OFFSET);

  /* Initialize default values for WB gain */
  Reg->wb_gain_config.rg_wb_gain.rg_wb_gain    = FLOAT_TO_Q(12, 1.0f);
  Reg->wb_gain_config.bg_wb_gain.bg_wb_gain    = FLOAT_TO_Q(12, 1.0f);
  Reg->wb_gain_config.gr_wb_gain.gr_wb_gain    = FLOAT_TO_Q(12, 1.0f);
  Reg->wb_gain_config.gb_wb_gain.gb_wb_gain    = FLOAT_TO_Q(12, 1.0f);

  /* TODO - - Need to initialize default values for LOC regs*/
  Reg->loc_config.loc_offset_cfg.x_offset      = CLAMP(pdaf_pattern->pd_offset_horizontal,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);
  Reg->loc_config.loc_offset_cfg.y_offset      = CLAMP(pdaf_pattern->pd_offset_vertical,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);
  Reg->loc_config.loc_end_cfg.x_end            = CLAMP(pdaf_pattern->block_dim.width,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);
  Reg->loc_config.loc_end_cfg.y_end            = CLAMP(pdaf_pattern->block_dim.height,
                                                       PDAF_MIN_LOC_OFFSET, PDAF_MAX_LOC_OFFSET);

  return TRUE;
} /* pdaf48_config */

/** pdaf48_stats_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t             *stats_update = NULL;
  pdaf48_t                   *pdaf = NULL;
  chromatix_parms_type       *chromatix_ptr = NULL;
  chromatix_PDPC_DSBPC_type  *chromatix_pdaf = NULL;
  uint8_t                    trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL;
  float                      ratio = 0;
  float                      hdr_exp_ratio = 0;
  int                        i = 0;
  boolean ret = TRUE;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (chromatix_ptr == NULL) {
    ISP_ERR("chromatix is NULL");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  chromatix_pdaf = &chromatix_ptr->chromatix_VFE.chromatix_PDPC_data;
  pdaf->aec_gain_reference = stats_update->aec_update.real_gain;
  pdaf->aec_lux_reference = stats_update->aec_update.lux_idx;

  if (chromatix_pdaf->control_PDPC) { /* gain controlled */
    float aec_reference = stats_update->aec_update.sensor_gain;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *pdaf_trigger =
        &(chromatix_pdaf->PDPC_config[i].PDPC_trigger);
      float gain_start = pdaf_trigger->gain_start;
      float gain_end   = pdaf_trigger->gain_end;
      trigger_index    = i;

      if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
        /* falls within region 6 but we do not use trigger points in the region */
        ratio = 0;
        break;
      }
      /* gain is within interpolation range, find ratio */
      if (aec_reference > gain_start && aec_reference < gain_end)
        ratio = (aec_reference - gain_start) / (gain_end - gain_start);
      if (aec_reference < gain_end)
        break;
    }
  } else { /* lux index controlled */
    float aec_reference = stats_update->aec_update.lux_idx;
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      trigger_point_type *pdaf_trigger =
        &(chromatix_pdaf->PDPC_config[i].PDPC_trigger);
      float lux_start = (float)pdaf_trigger->lux_index_start;
      float lux_end   = (float)pdaf_trigger->lux_index_end;
      trigger_index   = i;

      /* lux index is within interpolation range, find ratio */
      if (aec_reference > lux_start && aec_reference < lux_end)
        ratio = (aec_reference - lux_start) / (lux_end - lux_start);
      if (aec_reference < lux_end)
        break;
    }
  }
  hdr_exp_ratio = stats_update->aec_update.hdr_sensitivity_ratio;
  pdaf->hdr_sensitivity_ratio = stats_update->aec_update.hdr_exp_time_ratio;

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for PDPC trigger ratio: %f\n", ratio);
    goto ERROR;
  }

  if ((trigger_index != pdaf->trigger_index) ||
     (!F_EQUAL(ratio, pdaf->aec_ratio)) ||
     (!F_EQUAL(hdr_exp_ratio, pdaf->hdr_exp_ratio))) {
    pdaf->trigger_index = trigger_index;
    pdaf->aec_ratio     = ratio;
    pdaf->hdr_exp_ratio = hdr_exp_ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if (pdaf->ext_func_table && pdaf->ext_func_table->get_2d_interpolation) {
     ret = pdaf->ext_func_table->get_2d_interpolation(isp_sub_module, pdaf);
     if (ret == FALSE) {
       ISP_ERR("failed: get_2d_interpolation");
       goto ERROR;
     }
   }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* pdaf48_stats_aec_update */

/** pdaf48_stats_awb_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t          *stats_update = NULL;
  awb_gain_t              *cur_awb_gain = NULL;
  awb_gain_t              *new_awb_gain = NULL;
  pdaf48_t                *pdaf = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  new_awb_gain = &stats_update->awb_update.gain;
  cur_awb_gain = &pdaf->curr_awb_gain;

  if (!AWB_GAIN_EQUAL(new_awb_gain, cur_awb_gain) &&
       !AWB_GAIN_EQ_ZERO(new_awb_gain)) {
    isp_sub_module->trigger_update_pending = TRUE;
    *cur_awb_gain = *new_awb_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* pdaf48_stats_awb_update */

/** pdaf48_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                   ret = TRUE;
  pdaf48_t                  *pdaf = NULL;
  chromatix_parms_type      *chromatix_ptr = NULL;
  modulesChromatix_t        *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }
  chromatix_ptrs = (modulesChromatix_t *)data;
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->update_module_bit = TRUE;
  isp_sub_module->submod_enable = FALSE;

  if ((chromatix_ptr->chromatix_VFE.chromatix_PDPC_data.pdaf_pdpc_en > 0) ||
     (chromatix_ptr->chromatix_VFE.chromatix_PDPC_data.pdaf_dsbpc_en > 0)) {

     isp_sub_module->submod_enable = TRUE;
  }

  if(isp_sub_module->submod_enable == TRUE)
  {
    ret = pdaf48_config(isp_sub_module, pdaf);
    if (ret == FALSE) {
      ISP_ERR("failed: pdaf_config ret %d", ret);
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* pdaf48_set_chromatix_ptr */

/** pdaf48_set_pdaf_pixel:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Set PDAF pixel pattern
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_set_pdaf_pixel(isp_sub_module_t *isp_sub_module,
  void *data)
{
  pdaf48_t              *pdaf = NULL;
  pdaf_block_pattern_t  *pdaf_pattern = NULL;
  unsigned int          i = 0, m = 0, n = 0;
  unsigned int          hOffset = 0;
  unsigned int          vOffset = 0;
  unsigned int          block_width = 0;
  unsigned int          block_height = 0;
  unsigned int          scale_X = 0;
  unsigned int          scale_Y = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pdaf_pattern = (pdaf_block_pattern_t*)(&(((pdaf_sensor_native_info_t *)data)->block_pattern));

  pdaf->block_count_horizontal = ((pdaf_sensor_native_info_t *)data)->block_count_horizontal;
  pdaf->block_count_vertical = ((pdaf_sensor_native_info_t *)data)->block_count_vertical;
  
  hOffset = pdaf_pattern->pd_offset_horizontal;
  vOffset = pdaf_pattern->pd_offset_vertical;

  block_width  = pdaf_pattern->block_dim.width;
  block_height = pdaf_pattern->block_dim.height;

  scale_X = PDAF_LUT_COUNT / block_width;
  scale_Y = PDAF_LUT_COUNT / block_height;

  for(m=0; m < scale_X; m++) {
    for(n=0; n < scale_Y; n++) {
      for(i=0; i < pdaf_pattern->pix_count; i++) {
        (pdaf->packed_lut.lut)[((pdaf_pattern->pix_coords)[i]).y - vOffset + block_height*n] |=
        1 << (31 - ((((pdaf_pattern->pix_coords)[i]).x - hOffset + block_width*m)/2));
      }
    }
  }
  pdaf->pdaf_pattern.pd_offset_horizontal = hOffset;
  pdaf->pdaf_pattern.pd_offset_vertical = vOffset;
  pdaf->pdaf_pattern.block_dim.width = block_width;
  pdaf->pdaf_pattern.block_dim.height = block_height;

  ISP_DBG("<stripe_dbg> sub_grid_width %d", block_width);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* pdaf48_set_pdaf_pixel */

/** pdaf48_update_offset
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pdaf48_update_offset(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                       ret = TRUE;
  pdaf48_t                      *pdaf = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_stripe_limit_info_t       *stripe_limit = NULL;


  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, pdaf enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pdaf = (pdaf48_t *)isp_sub_module->private_data;

  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
     ISP_ERR("failed: stripe_limit %p", stripe_limit);
     PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
     return FALSE;
   }

   stripe_limit->pdaf_enable = isp_sub_module->submod_enable;
   stripe_limit->pdaf_hoffset = pdaf->pdaf_pattern.pd_offset_horizontal;
   stripe_limit->pdaf_blockwidth = pdaf->pdaf_pattern.block_dim.width;

   ISP_DBG("<stripe_dbg> pdaf_enable %d pdaf_hoffset %d pdaf_blockwidth %d ",
    stripe_limit->pdaf_enable,
    stripe_limit->pdaf_hoffset,
    stripe_limit->pdaf_blockwidth);
   PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
   return TRUE;
}

/** pdaf48_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pdaf48_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                       ret = TRUE;
  pdaf48_t                      *pdaf = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  pdaf->isp_out_info = *isp_split_out_info;

  ISP_DBG("%s: pdaf %p set_pdaf id %d",__func__,pdaf, pdaf->isp_out_info.stripe_id);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** pdaf48_streamon:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function makes initial configuration during first
 *  stream ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pdaf48_streamon(isp_sub_module_t *isp_sub_module, void *data)
{
  pdaf48_t                   *pdaf = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: , pdaf) %p", pdaf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* PDAF LUT config is only done once per session.
     Reset the flag if it is 1st streamon */
  if (isp_sub_module->stream_on_count == 1) {
    pdaf->one_time_config_done = FALSE;
  }

  ISP_DBG("alloc");
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

} /* pdaf48_streamon */

/** pdaf48_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pdaf48_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  pdaf48_t               *pdaf = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  pdaf = (pdaf48_t *)isp_sub_module->private_data;
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  memset(pdaf, 0, sizeof(*pdaf));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  FILL_FUNC_TABLE(pdaf);
  return TRUE;
} /* pdaf48_streamoff */

/** pdaf48_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the pdaf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean pdaf48_init(isp_sub_module_t *isp_sub_module)
{
  pdaf48_t *pdaf = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  pdaf = (pdaf48_t *)CAM_MALLOC(sizeof(pdaf48_t));
  if (!pdaf) {
    ISP_ERR("failed: pdaf %p", pdaf);
    return FALSE;
  }

  memset(pdaf, 0, sizeof(*pdaf));
  isp_sub_module->private_data = (void *)pdaf;
  FILL_FUNC_TABLE(pdaf);
  return TRUE;
}/* pdaf48_init */

/** pdaf48_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void pdaf48_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* pdaf48_destroy */

#if !OVERRIDE_FUNC
static void pdaf48_apply_2d_interpolation(void *data) {

  pdaf48_t   *pdaf;
  pdaf = (pdaf48_t*)data;

  pdaf->offset_g_pixel_t2 *= pdaf->pdaf48_ext_params.pdpc_bp_offset_G_adj;
  pdaf->offset_g_pixel_t2 = Round(pdaf->offset_g_pixel_t2);
  pdaf->pdaf_reg.bp_config.bp_offset_t2.offset_g_pixel = CLAMP(pdaf->offset_g_pixel_t2, PDAF_MIN_OFFSET,
    PDAF_MAX_OFFSET);
  pdaf->offset_rb_pixel_t2 *= pdaf->pdaf48_ext_params.pdpc_bp_offset_rb_adj;
  pdaf->offset_rb_pixel_t2 = Round(pdaf->offset_rb_pixel_t2);
  pdaf->pdaf_reg.bp_config.bp_offset_t2.offset_rb_pixel = CLAMP(pdaf->offset_rb_pixel_t2, PDAF_MIN_OFFSET,
    PDAF_MAX_OFFSET);
  pdaf->fmax *= pdaf->pdaf48_ext_params.pdpc_Fmax_adj;
  pdaf->pdaf_reg.bp_config.bp_th.fmax = Round(pdaf->fmax);
}


static void pdaf_interpolate(pdaf48_ext_t *local_params,
  pdaf48_ext_t *local_params1,
  pdaf48_ext_t *local_params2, float ratio) {

  local_params->pdpc_bp_offset_G_adj = (LINEAR_INTERPOLATION(
    local_params1->pdpc_bp_offset_G_adj, local_params2->pdpc_bp_offset_G_adj,
    ratio));

  local_params->pdpc_bp_offset_rb_adj = (LINEAR_INTERPOLATION((float)
    local_params1->pdpc_bp_offset_rb_adj, local_params2->pdpc_bp_offset_rb_adj,
    ratio));

  local_params->pdpc_Fmax_adj = (LINEAR_INTERPOLATION((float)
    local_params1->pdpc_Fmax_adj, local_params2->pdpc_Fmax_adj,
    ratio));
  }

static void pdaf_interpolate_region_parameters(
  bpc_pdpc_hdr_2d_exp_type *pdaf_exp_data,
  uint32_t trigger_idx_start, uint32_t trigger_idx_end,
  pdaf48_ext_t *local_params, float ratio) {

    local_params->pdpc_bp_offset_G_adj = (LINEAR_INTERPOLATION((float)
      pdaf_exp_data[trigger_idx_start].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_bp_offset_G_adj,
      (float)pdaf_exp_data[trigger_idx_end].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_bp_offset_G_adj,
      ratio));

    local_params->pdpc_bp_offset_rb_adj = (LINEAR_INTERPOLATION((float)
      pdaf_exp_data[trigger_idx_start].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_bp_offset_rb_adj,
      (float)pdaf_exp_data[trigger_idx_end].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_bp_offset_rb_adj,
      ratio));

    local_params->pdpc_Fmax_adj = (LINEAR_INTERPOLATION((float)
      pdaf_exp_data[trigger_idx_start].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_Fmax_adj,
      (float)pdaf_exp_data[trigger_idx_end].bpc_pdpc_hdr_2d_reserve_data.
      pdpc_Fmax_adj,
      ratio));

  }

static boolean pdaf_interpolate_parameters(bpc_pdpc_2d_adj_type
  *chromatix_2d,
  pdaf48_t   *pdaf, bpc_pdpc_hdr_2d_exp_type *pdaf_exp_data,
  pdaf48_ext_t *local_params) {

  trigger_point2_type      *pdaf_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    hdr_trigger_ref     = 0.0;
  uint32_t                 i                 = 0;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float                ratio = 0.0f;
  boolean ret = TRUE;

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within last region but we do not have any more
         adjacent regions */
      ratio         = 0.0f;
      trigger_idx_start = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      trigger_idx_end = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      break;
    }

    switch (chromatix_2d->hdr_exp_control) {
    case  CONTROL_AEC_EXP_SENSITIVITY_RATIO:
        /* aec senstivity ratio  based */
      hdr_trigger_ref = pdaf->hdr_exp_ratio;
      pdaf_trigger   = &(pdaf_exp_data[i].aec_sensitivity_ratio);
      start         = pdaf_trigger->start;
      end           = pdaf_trigger->end;
      break;
    case  CONTROL_EXP_TIME_RATIO:
      /* exp time based */
      hdr_trigger_ref = pdaf->hdr_sensitivity_ratio;
      pdaf_trigger   = &(pdaf_exp_data[i].exp_time_trigger);
      start         = pdaf_trigger->start;
      end           = pdaf_trigger->end;
    break;
    default:
      ISP_ERR("Incorrect tuning control type %d", chromatix_2d->exp_control);
      ret = FALSE;
      goto error;
    }
    /* index is within interpolation range, find ratio */
    if (((hdr_trigger_ref > start) || (F_EQUAL(hdr_trigger_ref, start)))
      && hdr_trigger_ref < end) {
      ratio = (end - hdr_trigger_ref)/(end - start);
      trigger_idx_start = i;
      trigger_idx_end = i+1;
      break;
    }
    if (hdr_trigger_ref < start) {
      trigger_idx_start = i;
      trigger_idx_end = i;
      ratio = 0.0f;
      break;
    }
  }
  ISP_DBG("%s [%f - %f - %f] = %f, t_idx1=%d t_idx2=%d", __func__, start,
    hdr_trigger_ref, end, ratio, trigger_idx_start, trigger_idx_end);

  pdaf_interpolate_region_parameters(pdaf_exp_data,
    trigger_idx_start, trigger_idx_end, local_params, ratio);
error:
  return ret;
}


static boolean pdaf48_process_regions(bpc_pdpc_2d_adj_type
  *chromatix_2d,
  pdaf48_t   *pdaf, uint32_t *trigger_idx_start,
  uint32_t *trigger_idx_end,
  float *ratio, tuning_control_type control_type) {
  boolean                  ret               = TRUE;
  trigger_point_type      *pdaf_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;


  if (!chromatix_2d || !pdaf) {
    ISP_ERR("failed: %p %p", chromatix_2d, pdaf);
    return FALSE;
  }

  for (i = 0; i < MAX_SETS_FOR_2D_EXP_ADJ; i++) {
    if (i == MAX_SETS_FOR_2D_EXP_ADJ - 1) {
      /* falls within last region but we do not have any more
         adjacent regions */
      *ratio         = 0.0f;
      *trigger_idx_start = MAX_SETS_FOR_2D_EXP_ADJ - 1;
      *trigger_idx_end = MAX_SETS_FOR_2D_EXP_ADJ - 1;
      break;
    }

    switch (control_type) {
    case CONTROL_LUX_IDX:
        /* lux index based */
      aec_reference = pdaf->aec_lux_reference;
      pdaf_trigger   = &(chromatix_2d->bpc_pdpc_hdr_rgn_data[i].trigger);
      start         = pdaf_trigger->lux_index_start;
      end           = pdaf_trigger->lux_index_end;
      ISP_ERR("lux base, lux idx %f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = pdaf->aec_gain_reference;
      pdaf_trigger   = &(chromatix_2d->bpc_pdpc_hdr_rgn_data[i].trigger);
      start         = pdaf_trigger->gain_start;
      end           = pdaf_trigger->gain_end;
    break;
    default:
      ISP_ERR("Incorrect tuning control type %d", control_type);
      ret = FALSE;
      goto error;
    }
    /* index is within interpolation range, find ratio */
    if (((aec_reference > start) || (F_EQUAL(aec_reference, start)))
      && aec_reference < end) {
      *ratio = (end - aec_reference)/(end - start);
      *trigger_idx_start = i;
      *trigger_idx_end = i+1;
      break;
    }
    if (aec_reference < start) {
      *trigger_idx_start = i;
      *trigger_idx_end = i;
      *ratio = 0.0f;
      break;
    }
  }
  ISP_DBG("%s [%f - %f - %f] = %f, t_idx1=%d t_idx2=%d", __func__, start,
    aec_reference, end, *ratio, *trigger_idx_start, *trigger_idx_end);

error:
  return ret;
}

/** pdaf48_2d_interpolation:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean pdaf48_2d_interpolation(void *data1,
  void *data2 )
{
  isp_sub_module_t *isp_sub_module;
  pdaf48_t   *pdaf = NULL;
  boolean   ret  = TRUE;
  chromatix_parms_type  *chromatix_ptr = NULL;
  bpc_pdpc_2d_adj_type *bpc_2d_adj_type =  NULL;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float     ratio = 0.0f;
  pdaf48_ext_t  pdaf48_ext_params_1, pdaf48_ext_params_2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  memset(&pdaf48_ext_params_1, 0, sizeof(pdaf48_ext_t));
  memset(&pdaf48_ext_params_2, 0, sizeof(pdaf48_ext_t));
  isp_sub_module = (isp_sub_module_t *)data1;
  pdaf = (pdaf48_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }

  bpc_2d_adj_type = &zzhdr_pdpc_tuning_data.pdpc_2d_data;

  if (bpc_2d_adj_type->pdpc_adj_enable &&
    (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)){
    pdaf48_process_regions(bpc_2d_adj_type, pdaf, &trigger_idx_start,
      &trigger_idx_end, &ratio, bpc_2d_adj_type->exp_control);

    if (trigger_idx_start != trigger_idx_end) {
      /* Compute the params based on exp_time_trigger/ aec_sensitivity_ratio */
      pdaf_interpolate_parameters(bpc_2d_adj_type, pdaf,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_start].
        bpc_pdpc_hdr_exp_data, &pdaf48_ext_params_1);
      pdaf_interpolate_parameters(bpc_2d_adj_type, pdaf,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_end].
        bpc_pdpc_hdr_exp_data, &pdaf48_ext_params_2);

      pdaf_interpolate(&pdaf->pdaf48_ext_params, &pdaf48_ext_params_1,
        &pdaf48_ext_params_2, ratio);
    } else {
      pdaf_interpolate_parameters(bpc_2d_adj_type, pdaf,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_start].
        bpc_pdpc_hdr_exp_data, &pdaf->pdaf48_ext_params);
    }
    pdaf->apply_hdr_effects = 1;
	//Assign local_params to bpc parameters. One more function or do a staright copy
  }
  return TRUE;
}

static ext_override_func pdaf_override_func = {
  .get_2d_interpolation = pdaf48_2d_interpolation,
  .apply_2d_interpolation = pdaf48_apply_2d_interpolation,
};

boolean pdaf48_fill_func_table(pdaf48_t *pdaf)
{
  pdaf->ext_func_table = &pdaf_override_func;
  return TRUE;
} /* abf48_fill_func_table */
#endif
