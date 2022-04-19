/* bcc44.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BCC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BCC, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "bcc44.h"
#include "isp_pipeline_reg.h"

/* forward declartions */
static void bcc44_interpolation_width_update( bpc_4_offset_type *input_offset,
  uint32_t chromatix_bitwidth, boolean is_lsb_alligned);

/** bcc44_cmd_debug:
 *
 *  @cmd: bcc config cmd
 *
 *  This function dumps the bcc module configuration set to hw
 *
 *  Returns nothing
 **/
static void bcc44_cmd_debug(ISP_DemosaicDBCC_CmdType *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG("cmd->fminThreshold = %d", cmd->fminThreshold);
  ISP_DBG("cmd->fmaxThreshold = %d", cmd->fmaxThreshold);
  ISP_DBG("cmd->rOffsetHi = %d", cmd->rOffsetHi);
  ISP_DBG("cmd->rOffsetLo = %d", cmd->rOffsetLo);
  ISP_DBG("cmd->bOffsetHi = %d", cmd->bOffsetHi);
  ISP_DBG("cmd->bOffsetLo = %d", cmd->bOffsetLo);
  ISP_DBG("cmd->grOffsetHi = %d", cmd->grOffsetHi);
  ISP_DBG("cmd->grOffsetLo = %d", cmd->grOffsetLo);
  ISP_DBG("cmd->gbOffsetHi = %d", cmd->gbOffsetHi);
  ISP_DBG("cmd->gbOffsetLo = %d", cmd->gbOffsetLo);
} /* bcc44_cmd_debug */

/** bcc44_ez_isp_update
 *
 *  @mod: bpc module handle
 *  @bccDiag: bpc Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void bcc44_ez_isp_update(bcc44_t *mod,
  badcorrection_t *bccDiag)
{
  ISP_DemosaicDBCC_CmdType *bccCfg = &(mod->RegCmd);

  bccDiag->fminThreshold = bccCfg->fminThreshold;
  bccDiag->fmaxThreshold = bccCfg->fmaxThreshold;
  bccDiag->gbOffsetLo = bccCfg->gbOffsetLo;
  bccDiag->gbOffsetHi = bccCfg->gbOffsetHi;
  bccDiag->grOffsetLo = bccCfg->grOffsetLo;
  bccDiag->grOffsetHi = bccCfg->grOffsetHi;
  bccDiag->rOffsetLo = bccCfg->rOffsetLo;
  bccDiag->rOffsetHi = bccCfg->rOffsetHi;
  bccDiag->bOffsetLo = bccCfg->bOffsetLo;
  bccDiag->bOffsetHi = bccCfg->bOffsetHi;
}/* bcc44_ez_isp_update */

/** bcc44_fill_vfe_diag_data:
 *
 *  @bcc: bcc module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean bcc44_fill_vfe_diag_data(bcc44_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  badcorrection_t     *bccDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    bccDiag = &(vfe_diag->prev_bcc);

    bcc44_ez_isp_update(mod, bccDiag);
  }
  return ret;
}

/** bcc44_fill_data:
 *
 *  @mod: bcc module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void bcc44_fill_data(isp_sub_module_t *isp_sub_module, bcc44_t *bcc)
{
  if (!isp_sub_module || !bcc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bcc);
    return;
  }
  if (!bcc->p_params.p_input_offset) {
    ISP_ERR("failed: mod->p_params.p_input_offset %p",
      bcc->p_params.p_input_offset);
    return;
  }
  bcc->RegCmd.fminThreshold = bcc->p_params.Fmin;
  bcc->RegCmd.fmaxThreshold = bcc->p_params.Fmax;
  bcc->RegCmd.rOffsetHi = bcc->p_params.p_input_offset->bpc_4_offset_r_hi;
  bcc->RegCmd.rOffsetLo = bcc->p_params.p_input_offset->bpc_4_offset_r_lo;
  bcc->RegCmd.bOffsetHi = bcc->p_params.p_input_offset->bpc_4_offset_b_hi;
  bcc->RegCmd.bOffsetLo = bcc->p_params.p_input_offset->bpc_4_offset_b_lo;
  bcc->RegCmd.grOffsetLo = bcc->p_params.p_input_offset->bpc_4_offset_gr_lo;
  bcc->RegCmd.grOffsetHi = bcc->p_params.p_input_offset->bpc_4_offset_gr_hi;
  bcc->RegCmd.gbOffsetLo = bcc->p_params.p_input_offset->bpc_4_offset_gb_lo;
  bcc->RegCmd.gbOffsetHi = bcc->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* bcc44_fill_data */

/** bcc44_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bcc: bcc struct data
 *
 *  Create hw update list and store it in isp sub module struct
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bcc44_store_hw_update(isp_sub_module_t *isp_sub_module,
  bcc44_t *bcc)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_DemosaicDBCC_CmdType    *RegCmd = NULL;

  if (!isp_sub_module || !bcc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bcc);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  RegCmd = (ISP_DemosaicDBCC_CmdType *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BCC_CMD;
  }
  memset(RegCmd, 0, sizeof(*RegCmd));
  *RegCmd = bcc->RegCmd;

  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = ISP_DBCC_OFF;
  reg_cfg_cmd->u.rw_info.len = ISP_DBCC_LEN * sizeof(uint32_t);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(RegCmd);
ERROR_BCC_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* bcc44_store_hw_update */

/** bcc44_init_data
 *
 *  @isp_sub_module: isp sub module handle
 *  @bcc: bcc handle
 *
 *  BCC module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bcc44_init_data(isp_sub_module_t *isp_sub_module, bcc44_t *bcc)
{
  if (!isp_sub_module, !bcc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bcc);
    return FALSE;
  }

  if (bcc->ext_func_table && bcc->ext_func_table->get_init_min_max) {
    bcc->ext_func_table->get_init_min_max(bcc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bcc->ext_func_table);
    return FALSE;
  }

  return TRUE;
} /* bcc44_init_data */

/** bcc44_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @bcc: bcc handle
 *
 *  Interpolate if trigger_update_pending is TRUE, create bcc
 *  module structure with interpolated params
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean bcc44_interpolate(isp_sub_module_t *isp_sub_module, bcc44_t *bcc)
{
  boolean                     ret = TRUE;
  chromatix_parms_type       *chromatix_ptr = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_BPC_type         *chromatix_BPC = NULL;
  float                       aec_ratio = 0;
  uint8_t                     Fmin = 0,
                              Fmin_lowlight = 0,
                              Fmax = 0,
                              Fmax_lowlight = 0;
  bpc_4_offset_type          *bcc_normal_input_offset = NULL;
  bpc_4_offset_type          *bcc_lowlight_input_offset = NULL;
  int32_t                    mutifactor =  0;
  boolean                    is_lsb_alligned = FALSE;

  if (!isp_sub_module || !bcc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bcc);
    return FALSE;
  }

  if (isp_sub_module->trigger_update_pending == FALSE) {
    return TRUE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  /*when HDR mode, chromatix tuned by 14 bit,
    otherwise keep 12 bit header for backward compatible*/
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    mutifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  } else {
    mutifactor = 0;
  }

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ) {
    mutifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
    is_lsb_alligned = TRUE;
  }

  ISP_DBG("Calculate table with AEC");

  if (bcc->ext_func_table && bcc->ext_func_table->get_min_max) {
    bcc->ext_func_table->get_min_max(bcc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bcc->ext_func_table);
    return FALSE;
  }

  Fmin = bcc->Fmin;
  Fmax = bcc->Fmax;
  Fmin_lowlight = bcc->Fmin_lowlight;
  Fmax_lowlight = bcc->Fmax_lowlight;
  bcc_normal_input_offset = &bcc->bcc_normal_input_offset;
  bcc_lowlight_input_offset = &bcc->bcc_lowlight_input_offset;

  bcc44_interpolation_width_update(bcc_normal_input_offset, mutifactor,
    is_lsb_alligned);
  bcc44_interpolation_width_update(bcc_lowlight_input_offset, mutifactor,
    is_lsb_alligned);

  aec_ratio = bcc->aec_ratio;
  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG("Low Light");
    bcc->p_params.p_input_offset = bcc_lowlight_input_offset;
    bcc->p_params.Fmin = Fmin_lowlight;
    bcc->p_params.Fmax = Fmax_lowlight;
    bcc44_fill_data(isp_sub_module, bcc);
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG("Normal Light");
    bcc->p_params.p_input_offset = bcc_normal_input_offset;
    bcc->p_params.Fmin = Fmin;
    bcc->p_params.Fmax = Fmax;
    bcc44_fill_data(isp_sub_module, bcc);
  } else {
    ISP_DBG("Interpolate between Nomal and Low Light");
    /* Directly configure reg cmd.*/
    Fmin = (uint8_t)LINEAR_INTERPOLATION_BET(Fmin, Fmin_lowlight, aec_ratio);
    bcc->RegCmd.fminThreshold = Fmin;

    Fmax = (uint8_t)LINEAR_INTERPOLATION_BET(Fmax, Fmax_lowlight, aec_ratio);
    bcc->RegCmd.fmaxThreshold = Fmax;

    bcc->RegCmd.rOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_r_hi,
      bcc_lowlight_input_offset->bpc_4_offset_r_hi, aec_ratio);
    bcc->RegCmd.rOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_r_lo,
      bcc_lowlight_input_offset->bpc_4_offset_r_lo, aec_ratio);

    bcc->RegCmd.bOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_b_hi,
      bcc_lowlight_input_offset->bpc_4_offset_b_hi, aec_ratio);
    bcc->RegCmd.bOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_b_lo,
      bcc_lowlight_input_offset->bpc_4_offset_b_lo, aec_ratio);

    bcc->RegCmd.grOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_gr_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gr_hi, aec_ratio);
    bcc->RegCmd.grOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_gr_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gr_lo, aec_ratio);

    bcc->RegCmd.gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_gb_hi,
      bcc_lowlight_input_offset->bpc_4_offset_gb_hi, aec_ratio);
    bcc->RegCmd.gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bcc_normal_input_offset->bpc_4_offset_gb_lo,
      bcc_lowlight_input_offset->bpc_4_offset_gb_lo, aec_ratio);
  }

  bcc->aec_ratio = aec_ratio;

  bcc44_cmd_debug(&(bcc->RegCmd));

  ret = bcc44_store_hw_update(isp_sub_module, bcc);
  if (ret == FALSE) {
    ISP_ERR("failed: bcc44_store_hw_update");
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;
} /* bcc44_trigger_update */

/** bcc44_interpolate_width_update
 *
 * input_offset - data from chromatix
 * 8994 - Pipeline bit width is 14
 * 8084 - pipeline bitwidth is 12
 * performs corresponding update on input data
 **/
void bcc44_interpolation_width_update( bpc_4_offset_type *input_offset,
  int32_t multifactor, boolean is_lsb_alligned)
{
  if (is_lsb_alligned){
      input_offset->bpc_4_offset_r_hi  >>= multifactor;
      input_offset->bpc_4_offset_r_lo  >>= multifactor;
      input_offset->bpc_4_offset_b_hi  >>= multifactor;
      input_offset->bpc_4_offset_b_lo  >>= multifactor;
      input_offset->bpc_4_offset_gb_hi >>= multifactor;
      input_offset->bpc_4_offset_gb_lo >>= multifactor;
      input_offset->bpc_4_offset_gr_hi >>= multifactor;
      input_offset->bpc_4_offset_gr_lo >>= multifactor;
  }
  } else {
      input_offset->bpc_4_offset_r_hi  <<= multifactor;
      input_offset->bpc_4_offset_r_lo  <<= multifactor;
      input_offset->bpc_4_offset_b_hi  <<= multifactor;
      input_offset->bpc_4_offset_b_lo  <<= multifactor;
      input_offset->bpc_4_offset_gb_hi <<= multifactor;
      input_offset->bpc_4_offset_gb_lo <<= multifactor;
      input_offset->bpc_4_offset_gr_hi <<= multifactor;
      input_offset->bpc_4_offset_gr_lo <<= multifactor;
  }
}

/** bcc44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bcc44_trigger_update(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                       ret = TRUE;
  bcc44_t                      *bcc = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  uint8_t                      module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bcc = (bcc44_t *)isp_sub_module->private_data;
  if (!bcc) {
    ISP_ERR("failed: bcc %p", bcc);
    goto ERROR;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    goto ERROR;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.hot_pix_mode ==
                     CAM_HOTPIXEL_MODE_OFF) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->module_enable_info.reconfig_needed = TRUE;
      output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;
    }
  }

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  if (bcc->ext_func_table && bcc->ext_func_table->check_enable_idx)
    bcc->ext_func_table->check_enable_idx(bcc, isp_sub_module, output);

  ret = bcc44_interpolate(isp_sub_module, bcc);
  if (ret == FALSE) {
    ISP_ERR("failed: bcc44_interpolate");
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = bcc44_fill_vfe_diag_data(bcc, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: bcc44_fill_vfe_diag_data");
    }
  }

 if (output->frame_meta)
   output->frame_meta->hot_pix_mode =
     isp_sub_module->manual_ctrls.hot_pix_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** bcc44_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bcc44_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t       *stats_update = NULL;
  bcc44_t              *bcc = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  boolean               ret = TRUE;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bcc = (bcc44_t *)isp_sub_module->private_data;
  if (!bcc) {
    ISP_ERR("failed: bcc %p", bcc);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    goto ERROR;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  bcc->aec_update = stats_update->aec_update;

  if (bcc->ext_func_table && bcc->ext_func_table->get_trigger_ratio) {
    ret = bcc->ext_func_table->get_trigger_ratio(bcc, isp_sub_module, NULL);
  }  else {
    ISP_ERR("failed: %p", bcc->ext_func_table);
    ret = FALSE;
  }
  if (FALSE == ret) {
    ISP_ERR("failed");
    goto ERROR;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** bcc44_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bcc44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  bcc44_t            *bcc = NULL;
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bcc = (bcc44_t *)isp_sub_module->private_data;
  if (!bcc) {
    ISP_ERR("failed: mod %p", bcc);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  ret = bcc44_init_data(isp_sub_module, bcc);
  if (ret == FALSE) {
    ISP_ERR("failed: bcc44_init_data ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* bcc44_set_chromatix_ptr */

/** bcc44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bcc44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bcc44_t *bcc = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bcc = (bcc44_t *)isp_sub_module->private_data;
  if (!bcc) {
    ISP_ERR("failed: bcc %p", bcc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bcc, 0, sizeof(*bcc));
  FILL_FUNC_TABLE(bcc);/* Keep func ptr table*/
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bcc44_streamoff */

/** bcc44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bcc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bcc44_init(isp_sub_module_t *isp_sub_module)
{
  bcc44_t *bcc = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  bcc = (bcc44_t *)CAM_MALLOC(sizeof(bcc44_t));
  if (!bcc) {
    ISP_ERR("failed: bcc %p", bcc);
    return FALSE;
  }

  memset(bcc, 0, sizeof(*bcc));
  isp_sub_module->private_data = (void *)bcc;
  FILL_FUNC_TABLE(bcc);
  isp_sub_module->manual_ctrls.hot_pix_mode = CAM_HOTPIXEL_MODE_FAST;

  return TRUE;
}/* bcc44_init */

/** bcc44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void bcc44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* bcc44_destroy */

#if !OVERRIDE_FUNC
/** bcc44_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc44_get_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bcc44_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  if ((chromatix_BPC->bcc_Fmin > chromatix_BPC->bcc_Fmax) ||
      (chromatix_BPC->bcc_Fmin_lowlight > chromatix_BPC->bcc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d\n",
      chromatix_BPC->bcc_Fmin, chromatix_BPC->bcc_Fmax,
      chromatix_BPC->bcc_Fmin_lowlight, chromatix_BPC->bcc_Fmax_lowlight);
    return FALSE;
  }

  bcc->Fmin = chromatix_BPC->bcc_Fmin;
  bcc->Fmax = chromatix_BPC->bcc_Fmax;
  bcc->Fmin_lowlight = chromatix_BPC->bcc_Fmin_lowlight;
  bcc->Fmax_lowlight = chromatix_BPC->bcc_Fmax_lowlight;
  bcc->bcc_normal_input_offset = chromatix_BPC->bcc_4_offset[BPC_NORMAL_LIGHT];
  bcc->bcc_lowlight_input_offset = chromatix_BPC->bcc_4_offset[BPC_LOW_LIGHT];
  return TRUE;
}

/** bcc44_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc44_get_init_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bcc44_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  if ((chromatix_BPC->bcc_Fmin > chromatix_BPC->bcc_Fmax) ||
      (chromatix_BPC->bcc_Fmin_lowlight > chromatix_BPC->bcc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d",
      chromatix_BPC->bcc_Fmin, chromatix_BPC->bcc_Fmax,
      chromatix_BPC->bcc_Fmin_lowlight, chromatix_BPC->bcc_Fmax_lowlight);
    return FALSE;
  }

  bcc->p_params.p_input_offset =
    &(chromatix_BPC->bcc_4_offset[BPC_NORMAL_LIGHT]);
  bcc->p_params.Fmin = chromatix_BPC->bcc_Fmin;
  bcc->p_params.Fmax = chromatix_BPC->bcc_Fmax;
  return TRUE;
}

/** bcc44_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc44_get_trigger_ratio(void *data1,
  void *data2, void*data3)
{
  trigger_ratio_t trigger_ratio;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  float                 aec_ratio = 0.0;
  bcc44_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_BPC->control_bcc, &chromatix_BPC->bcc_lowlight_trigger,
    &bcc->aec_update, 1);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
       !F_EQUAL(bcc->aec_ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  bcc->aec_ratio = aec_ratio;
  return TRUE;
}

static ext_override_func bcc_override_func = {
  .check_enable_idx  = NULL,
  .get_trigger_ratio = bcc44_get_trigger_ratio,
  .get_init_min_max  = bcc44_get_init_min_max,
  .get_min_max       = bcc44_get_min_max,
};

boolean bcc44_fill_func_table(bcc44_t *bcc)
{
  bcc->ext_func_table = &bcc_override_func;
  return TRUE;
} /* bcc44_fill_func_table */
#endif
