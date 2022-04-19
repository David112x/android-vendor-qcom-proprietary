/* bpc40.c
 *
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BPC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BPC, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "bpc40.h"
#include "isp_pipeline_reg.h"

#define DBPC_RCCB_GAIN(x) FLOAT_TO_Q(7, (x))

/*forward declartions */
static void bpc40_interpolation_width_update( bpc_4_offset_type *input_offset );

/** bpc40_cmd_debug:
 *
 *  @cmd: bpc config cmd
 *
 *  This function dumps the bpc module configuration set to hw
 *
 *  Returns nothing
 **/
static void bpc40_cmd_debug(ISP_DemosaicDBPC_CmdType *cmd)
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
} /* bpc40_cmd_debug */

/** bpc40_ez_isp_update
 *
 *  @mod: bpc module handle
 *  @bpcDiag: bpc Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void bpc40_ez_isp_update(bpc40_t *mod,
  badcorrection_t *bpcDiag)
{
  ISP_DemosaicDBPC_CmdType *bpcCfg = &(mod->RegCmd);

  bpcDiag->fminThreshold = bpcCfg->fminThreshold;
  bpcDiag->fmaxThreshold = bpcCfg->fmaxThreshold;
  bpcDiag->gbOffsetLo = bpcCfg->gbOffsetLo;
  bpcDiag->gbOffsetHi = bpcCfg->gbOffsetHi;
  bpcDiag->grOffsetLo = bpcCfg->grOffsetLo;
  bpcDiag->grOffsetHi = bpcCfg->grOffsetHi;
  bpcDiag->rOffsetLo = bpcCfg->rOffsetLo;
  bpcDiag->rOffsetHi = bpcCfg->rOffsetHi;
  bpcDiag->bOffsetLo = bpcCfg->bOffsetLo;
  bpcDiag->bOffsetHi = bpcCfg->bOffsetHi;
}/* bpc40_ez_isp_update */

/** bpc40_fill_vfe_diag_data:
 *
 *  @bpc: bpc module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean bpc40_fill_vfe_diag_data(bpc40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  badcorrection_t     *bpcDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    bpcDiag = &(vfe_diag->prev_bpc);

    bpc40_ez_isp_update(mod, bpcDiag);
  }
  return ret;
}

/** bpc40_cmd_config:
 *
 *  @mod: bpc module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void bpc40_cmd_config(isp_sub_module_t *isp_sub_module, bpc40_t *bpc)
{
  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return;
  }
  if (!bpc->p_params.p_input_offset) {
    ISP_ERR("failed: mod->p_params.p_input_offset %p",
      bpc->p_params.p_input_offset);
    return;
  }
  bpc->RegCfgCmd.enable = (isp_sub_module->submod_enable == TRUE) ? 1 : 0;
  bpc->RegCmd.fminThreshold = *bpc->p_params.p_Fmin;
  bpc->RegCmd.fmaxThreshold = *bpc->p_params.p_Fmax;
  bpc->RegCmd.gain = DBPC_RCCB_GAIN(1);
  bpc->RegCmd.rOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_r_hi;
  bpc->RegCmd.rOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_r_lo;
  bpc->RegCmd.bOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_b_hi;
  bpc->RegCmd.bOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_b_lo;
  bpc->RegCmd.grOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_gr_lo;
  bpc->RegCmd.grOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_gr_hi;
  bpc->RegCmd.gbOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_gb_lo;
  bpc->RegCmd.gbOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* bpc40_cmd_config */

/** bpc40_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc40_store_hw_update(isp_sub_module_t *isp_sub_module,
  bpc40_t *bpc)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_DemosaicDBPC_CmdType    *RegCmd = NULL;

  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
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

  RegCmd = (ISP_DemosaicDBPC_CmdType *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BPC_CMD;
  }
  memset(RegCmd, 0, sizeof(*RegCmd));
  *RegCmd = bpc->RegCmd;

  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = ISP_DBPC_CFG_OFF;
  reg_cfg_cmd->u.rw_info.len = ISP_DBPC_LEN * sizeof(uint32_t);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(RegCmd);
ERROR_BPC_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* bpc40_do_hw_update */

/** bpc40_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc handle
 *
 *  BPC module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bpc40_config(isp_sub_module_t *isp_sub_module, bpc40_t *bpc)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;

  if (!isp_sub_module, !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;

  if (bpc->ext_func_table && bpc->ext_func_table->get_init_min_max) {
    bpc->ext_func_table->get_init_min_max(bpc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    return FALSE;
  }

  return TRUE;
} /* bpc40_config */

/** bpc40_interpolate
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc handle
 *
 *  BPC module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean bpc40_interpolate(isp_sub_module_t *isp_sub_module, bpc40_t *bpc)
{
  boolean               ret = TRUE;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  float                 aec_ratio = 0;
  uint8_t               Fmin = 0,
                        Fmin_lowlight = 0,
                        Fmax = 0,
                        Fmax_lowlight = 0;
  bpc_4_offset_type    bpc_normal_input_offset;
  bpc_4_offset_type    bpc_lowlight_input_offset;

  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
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

  ISP_DBG("Calculate table with AEC");
  if (bpc->ext_func_table && bpc->ext_func_table->get_min_max) {
    bpc->ext_func_table->get_min_max(bpc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    return FALSE;
  }

  Fmin = bpc->Fmin;
  Fmax = bpc->Fmax;
  Fmin_lowlight = bpc->Fmin_lowlight;
  Fmax_lowlight = bpc->Fmax_lowlight;
  bpc_normal_input_offset = bpc->bpc_normal_input_offset;
  bpc_lowlight_input_offset = bpc->bpc_lowlight_input_offset;

  bpc40_interpolation_width_update( &bpc_normal_input_offset );
  bpc40_interpolation_width_update( &bpc_lowlight_input_offset );

  aec_ratio = bpc->aec_ratio;
  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG("%s:Low Light",__func__);
    bpc->p_params.p_input_offset = &bpc_lowlight_input_offset;
    bpc->p_params.p_Fmin = &(Fmin_lowlight);
    bpc->p_params.p_Fmax = &(Fmax_lowlight);
    bpc40_cmd_config(isp_sub_module, bpc);
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG("%s:Normal Light",__func__);
    bpc->p_params.p_input_offset = &bpc_normal_input_offset;
    bpc->p_params.p_Fmin = &(Fmin);
    bpc->p_params.p_Fmax = &(Fmax);
    bpc40_cmd_config(isp_sub_module, bpc);
  } else {
    ISP_DBG("%s:Interpolate between Nomal and Low Light",__func__);
    /* Directly configure reg cmd.*/
    Fmin = (uint8_t)LINEAR_INTERPOLATION_BET(Fmin, Fmin_lowlight, aec_ratio);
    bpc->RegCmd.fminThreshold = Fmin;

    Fmax = (uint8_t)LINEAR_INTERPOLATION_BET(Fmax, Fmax_lowlight, aec_ratio);
    bpc->RegCmd.fmaxThreshold = Fmax;

    bpc->RegCmd.gain = DBPC_RCCB_GAIN(1);
    bpc->RegCmd.rOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_r_hi,
      bpc_lowlight_input_offset.bpc_4_offset_r_hi, aec_ratio);
    bpc->RegCmd.rOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_r_lo,
      bpc_lowlight_input_offset.bpc_4_offset_r_lo, aec_ratio);

    bpc->RegCmd.bOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_b_hi,
      bpc_lowlight_input_offset.bpc_4_offset_b_hi, aec_ratio);
    bpc->RegCmd.bOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_b_lo,
      bpc_lowlight_input_offset.bpc_4_offset_b_lo, aec_ratio);

    bpc->RegCmd.grOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset.bpc_4_offset_gr_hi, aec_ratio);
    bpc->RegCmd.grOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset.bpc_4_offset_gr_lo, aec_ratio);

    bpc->RegCmd.gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset.bpc_4_offset_gb_hi, aec_ratio);
    bpc->RegCmd.gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset.bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset.bpc_4_offset_gb_lo, aec_ratio);
  }
  bpc->aec_ratio = aec_ratio;

  bpc40_cmd_debug(&(bpc->RegCmd));

  ret = bpc40_store_hw_update(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("%s:failed: bpc40_store_hw_update",__func__);
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;
} /* bpc40_interpolate */

/** bpc40_interpolate_width_update
 *
 * input_offset - data from chromatix
 * 8994 - Pipeline bit width is 14
 * 8084 - Pipeline bitwidth is 12
 * 8916 - Pipeline width is 12
 * performs corresponding update on input data
 **/
void bpc40_interpolation_width_update( bpc_4_offset_type *input_offset )
{
  if (ISP_PIPELINE_WIDTH >= CHROMATIX_BITWIDTH)
  {
     input_offset->bpc_4_offset_r_hi  <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_r_lo  <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_b_hi  <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_b_lo  <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_gb_hi <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_gb_lo <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_gr_hi <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
     input_offset->bpc_4_offset_gr_lo <<= (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH );
  }
}

/** bpc40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bpc40_trigger_update(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                       ret = TRUE;
  bpc40_t                      *bpc = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  uint8_t                      module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc = (bpc40_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
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

  if (isp_sub_module->manual_ctrls.manual_update) {
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

  if (bpc->ext_func_table && bpc->ext_func_table->check_enable_idx)
    bpc->ext_func_table->check_enable_idx(bpc, isp_sub_module, output);

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  ret = bpc40_interpolate(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("%s:failed: bpc40_interpolate",__func__);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = bpc40_fill_vfe_diag_data(bpc, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc40_fill_vfe_diag_data");
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

/** bpc40_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc40_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t       *stats_update = NULL;
  bpc40_t              *bpc = NULL;
  float                 aec_ratio = 0;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  boolean              ret = TRUE;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc40_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bpc->aec_update = stats_update->aec_update;
  chromatix_ptr = (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (bpc->ext_func_table && bpc->ext_func_table->get_trigger_ratio) {
    ret = bpc->ext_func_table->get_trigger_ratio(bpc, isp_sub_module, NULL);
  }  else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    ret =  FALSE;
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

/** bpc40_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  bpc40_t            *bpc = NULL;
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc40_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: mod %p", bpc);
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

  ret = bpc40_config(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc40_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* bpc40_set_chromatix_ptr */

/** bpc40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc40_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bpc40_t *bpc = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc40_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bpc, 0, sizeof(*bpc));
  FILL_FUNC_TABLE(bpc);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bpc40_streamoff */

/** bpc40_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bpc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bpc40_init(isp_sub_module_t *isp_sub_module)
{
  bpc40_t *bpc = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  bpc = (bpc40_t *)CAM_MALLOC(sizeof(bpc40_t));
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  memset(bpc, 0, sizeof(*bpc));

  isp_sub_module->private_data = (void *)bpc;
  FILL_FUNC_TABLE(bpc);
  isp_sub_module->manual_ctrls.hot_pix_mode = CAM_HOTPIXEL_MODE_FAST;
  return TRUE;
}/* bpc40_init */

/** bpc40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void bpc40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* bpc40_destroy */

#if !OVERRIDE_FUNC
/** bpc40_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
  if ((chromatix_BPC->bcc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bcc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d\n",
      chromatix_BPC->bcc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bcc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return FALSE;
  }

  bpc->Fmin = chromatix_BPC->bpc_Fmin;
  bpc->Fmax = chromatix_BPC->bpc_Fmax;
  bpc->Fmin_lowlight = chromatix_BPC->bpc_Fmin_lowlight;
  bpc->Fmax_lowlight = chromatix_BPC->bpc_Fmax_lowlight;
  bpc->bpc_normal_input_offset =
    chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT];
  bpc->bpc_lowlight_input_offset =
    chromatix_BPC->bpc_4_offset[BPC_LOW_LIGHT];
  return TRUE;
}

/** bpc4_get_init_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_init_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
  if ((chromatix_BPC->bpc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bpc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d",
      chromatix_BPC->bpc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bpc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return FALSE;
  }

  bpc->p_params.p_input_offset =
    &(chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT]);
  bpc->p_params.p_Fmin = &(chromatix_BPC->bpc_Fmin);
  bpc->p_params.p_Fmax = &(chromatix_BPC->bpc_Fmax);
  return TRUE;
}

/** bpc40_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_trigger_ratio(void *data1,
  void *data2, void *data3)
{
  trigger_ratio_t trigger_ratio;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  float                 aec_ratio = 0.0;
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
    chromatix_BPC->control_bpc, &chromatix_BPC->bpc_lowlight_trigger,
    &bpc->aec_update, 1);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
       !F_EQUAL(bpc->aec_ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  bpc->aec_ratio = aec_ratio;
  return TRUE;
}

static ext_override_func bpc_override_func = {
  .check_enable_idx  = NULL,
  .get_trigger_ratio = bpc40_get_trigger_ratio,
  .get_init_min_max  = bpc40_get_init_min_max,
  .get_min_max       = bpc40_get_min_max,
};

boolean bpc40_fill_func_table(bpc40_t *bpc)
{
  bpc->ext_func_table = &bpc_override_func;
  return TRUE;
} /* bpc40_fill_func_table */

#endif
