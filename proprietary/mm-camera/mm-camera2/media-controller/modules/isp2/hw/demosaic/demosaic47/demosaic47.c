/* demosaic47.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_demosaic47.h"
#include "demosaic47.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_DEMOSAIC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_DEMOSAIC, fmt, ##args)

#define DEMOSAIC_DEFAULT_IDX 1

static void demosaic47_ez_isp_update(isp_sub_module_t *isp_sub_module,
  demosaic47_t *demosaic_module, demosaic3_t *demosaicDiag);

/** demosaic47_debug:
 *
 *  @cmd: configuration command
 *  @update: is it update flag
 *
 *  This function dumps demosaic configuration
 *
 *  Return: None
 **/
static void demosaic47_debug(void *cmd)
{
  int i;

  ISP_DemosaicConfigCmdType* pcmd = (ISP_DemosaicConfigCmdType *)cmd;

  /* Left WB CFG */
  ISP_DBG("ISP_Demosaic47CmdType leftWBCfg0.g_gain %d",
      pcmd->leftWBCfg0.g_gain);
  ISP_DBG("ISP_Demosaic47CmdType leftWBCfg0.b_gain %d",
      pcmd->leftWBCfg0.b_gain);
  ISP_DBG("ISP_Demosaic47CmdType leftWBCfg1.r_gain %d",
      pcmd->leftWBCfg1.r_gain);
  ISP_DBG("ISP_Demosaic47CmdType leftWBOffset0.g_offset %d",
      pcmd->leftWBOffsetCfg0.g_offset);
  ISP_DBG("ISP_Demosaic47CmdType leftWBOffset0.b_offset %d",
      pcmd->leftWBOffsetCfg0.b_offset);
  ISP_DBG("ISP_Demosaic47CmdType leftWBOffset1.r_offset %d",
      pcmd->leftWBOffsetCfg1.r_offset);

  /* Right WB CFG */
  ISP_DBG("ISP_Demosaic47CmdType rightWBCfg0.g_gain %d",
      pcmd->rightWBCfg0.g_gain);
  ISP_DBG("ISP_Demosaic47CmdType rightWBCfg0.b_gain %d",
      pcmd->rightWBCfg0.b_gain);
  ISP_DBG("ISP_Demosaic47CmdType rightWBCfg1.r_gain %d",
      pcmd->rightWBCfg1.r_gain);
  ISP_DBG("ISP_Demosaic47CmdType rightWBOffset0.g_offset %d",
      pcmd->rightWBOffsetCfg0.g_offset);
  ISP_DBG("ISP_Demosaic47CmdType rightWBOffset0.b_offset %d",
      pcmd->rightWBOffsetCfg0.b_offset);
  ISP_DBG("ISP_Demosaic47CmdType rightWBOffset1.r_offset %d",
      pcmd->rightWBOffsetCfg1.r_offset);

  ISP_DBG("ISP_Demosaic47CmdType interpCoeffCfg.wgr1 %d",
      pcmd->interpCoeffCfg.wgr1);
  ISP_DBG("ISP_Demosaic47CmdType interpCoeffCfg.wgr2 %d",
      pcmd->interpCoeffCfg.wgr2);
  ISP_DBG("ISP_Demosaic47CmdType interpCoeffCfg.lambda_g %d",
      pcmd->interpCoeffCfg.lambda_g);
  ISP_DBG("ISP_Demosaic47CmdType interpCoeffCfg.lambda_rb %d",
      pcmd->interpCoeffCfg.lambda_rb);

  ISP_DBG("dirGInterpDisable %d , dirRBInterpDisable %d",
    pcmd->demoCfg.dirGInterpDisable, pcmd->demoCfg.dirRBInterpDisable);
  ISP_DBG("dynGClampEnable %d , dynRBClampEnable %d",
    pcmd->demoCfg.dynGClampEnable, pcmd->demoCfg.dynRBClampEnable);
  ISP_DBG("dynGClampCDisable %d , dynRBClampCDisable %d",
    pcmd->demoCfg.dynGClampCDisable, pcmd->demoCfg.dynRBClampCDisable);

  for(i = 0; i < ISP_DEMOSAIC_CLASSIFIER_CNT; i++)
    ISP_DBG("ISP_Demosaic47CmdType [%d] w_n %d, t_n %d, a_n %d", i,
        pcmd->interpClassifier[i].w_n,
        pcmd->interpClassifier[i].t_n,
        pcmd->interpClassifier[i].a_n);

  ISP_DBG("ISP_Demosaic47CmdType interpNoiseCfg.noise_level_g %d",
      pcmd->interpNoiseCfg.noise_level_g);
  ISP_DBG("ISP_Demosaic47CmdType interpNoiseCfg.noise_level_rb %d",
      pcmd->interpNoiseCfg.noise_level_rb);

}/*demosaic47_debug*/

/** demosaic47_fill_vfe_diag_data:
 *
 *  @demosaic: demosaic module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean demosaic47_fill_vfe_diag_data(demosaic47_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  demosaic3_t         *demosaicDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    demosaicDiag = &(vfe_diag->prev_demosaic);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      demosaicDiag = &(vfe_diag->snap_demosaic);
    }
    demosaic47_ez_isp_update(isp_sub_module, mod, demosaicDiag);
  }
  return ret;
}

/** demosaic47_set_chromatix_params:
 *
 *  @demosaic: demosaic module instance
 *  @pix_setting: PIX settings
 *
 *  This function prepares demosaic configuration based on
 *  chromatix parameters
 *
 *  Return: None
 **/
static void demosaic47_set_chromatix_params(
    demosaic47_t         *demosaic,
    isp_sub_module_t   *isp_sub_module,
    chromatix_parms_type *chromatix_ptr)
{
  int i    = 0;
  int temp = 0;

  Chromatix_demosaic4_type *chromatix_demosaic4 = NULL;
  Demosaic4_reserve        *reserve             = NULL;
  Demosaic4_core           *data                = NULL;
  boolean                   lsb_allignment = FALSE;

  ISP_DemosaicConfigCmdType *p_cmd   = NULL;
  ISP_DemosaicConfigType    *cfg_cmd = NULL;
  uint32_t  multi_factor = (ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH);
  uint32_t  ak_multi_factor = (ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH);

  if (!demosaic || !chromatix_ptr || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p ", demosaic, chromatix_ptr,
      isp_sub_module);
    return;
  }
  p_cmd = &demosaic->reg_cmd;
  cfg_cmd = &demosaic->reg_cmd.demoCfg;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }

  if (lsb_allignment){
      multi_factor = 0;
      ak_multi_factor = (ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH);
  }

  chromatix_demosaic4 = &chromatix_ptr->chromatix_VFE.chromatix_demosaic4;
  /* TODO use correct chromatix demosaic field */
  reserve = (Demosaic4_reserve*)&(chromatix_demosaic4->demosaic4_reserve);
  data = (Demosaic4_core*)&(chromatix_demosaic4->demosaic4_data[DEMOSAIC_DEFAULT_IDX]);

  cfg_cmd->cositedRgbEnable = FALSE;

  /* Config */
  cfg_cmd->dirGInterpDisable = data->dis_directional_G;
  cfg_cmd->dirRBInterpDisable = data->dis_directional_RB;
  cfg_cmd->dynGClampEnable = reserve->en_dyna_clamp_G;
  cfg_cmd->dynRBClampEnable = reserve->en_dyna_clamp_RB;
  cfg_cmd->dynGClampCDisable = reserve->dis_dclamp_cntr_G;
  cfg_cmd->dynRBClampCDisable = reserve->dis_dclamp_cntr_RB;

  /* Classifier */
  for (i = 0; i < ISP_DEMOSAIC_CLASSIFIER_CNT; i++) {
    p_cmd->interpClassifier[i].w_n = FLOAT_TO_Q(10,data->wk[i]);
    p_cmd->interpClassifier[i].t_n =
      data->tk[i];
    if (lsb_allignment){
      p_cmd->interpClassifier[i].a_n = data->ak[i] >> ak_multi_factor;
    } else {
      p_cmd->interpClassifier[i].a_n = data->ak[i] << ak_multi_factor;
    }
  }

  /* Interp WB */
  p_cmd->interpCoeffCfg.wgr1 = reserve->non_dir_weight_1;
  p_cmd->interpCoeffCfg.wgr2 = reserve->non_dir_weight_2;

  /* lambda values need to multiply by Q8 */
  p_cmd->interpCoeffCfg.lambda_g = data->low_freq_weight_G;
  p_cmd->interpCoeffCfg.lambda_rb = data->low_freq_weight_RB;

  /* Interp G */
  p_cmd->interpNoiseCfg.noise_level_g = data->noise_level_G
                    << multi_factor;
  p_cmd->interpNoiseCfg.noise_level_rb = data->noise_level_RB
                    << multi_factor;

} /* demosaic47_set_cfg_params */

/** demosaic47_calc_interpolation_weight:
 *
 *  @value: value
 *  @start: start
 *  @end: end
 *
 *  Interpolate based on start and end
 *
 *  Return value after interpolation
 **/
static float demosaic47_calc_interpolation_weight(float value,
  float start, float end)
{
  /* return value is a ratio to the start point,
    "start" point is always the smaller gain/lux one.
    thus,
    "start" could be lowlight trigger start, and bright light trigger end*/
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    ISP_HIGH("Trigger Warning: same value %f\n", start);
    return 0.0;
  }
} /* demosaic47_calc_interpolation_weight */

/** demosaic47_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @demosaic_mod: demosaic module instance
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean demosaic47_store_hw_update(isp_sub_module_t *isp_sub_module,
  demosaic47_t *demosaic_mod)
{
  boolean                      ret = TRUE;
  int                          i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_DemosaicConfigCmdType *reg_cmd = NULL;

  if (!isp_sub_module || !demosaic_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, demosaic_mod);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  /*regular hw write for Demosaic*/
  demosaic47_debug(&demosaic_mod->reg_cmd);

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 5);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd) * 5));

  reg_cmd = (ISP_DemosaicConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));
  *reg_cmd = demosaic_mod->reg_cmd;
  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;

  if (demosaic_mod->classifier_cfg_done == 0)
    cfg_cmd->num_cfg = 5;
  else
    cfg_cmd->num_cfg = 4;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DEMOSAIC_CFG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_DEMOSAIC_CFG_LEN * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset +
    reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DEMOSAIC_WB_OFF;
  reg_cfg_cmd[1].u.rw_info.len = ISP_DEMOSAIC_WB_LEN *
    sizeof(uint32_t);

  reg_cfg_cmd[2].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset +
    reg_cfg_cmd[1].u.rw_info.len;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_DEMOSAIC_INTERP_COEFF_OFF;
  reg_cfg_cmd[2].u.rw_info.len = ISP_DEMOSAIC_INTERP_COEFF_LEN *
    sizeof(uint32_t);

  /*now even class fier need to interpolate*/
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset +
    reg_cfg_cmd[2].u.rw_info.len;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE;
  reg_cfg_cmd[3].u.rw_info.reg_offset = ISP_DEMOSAIC_CLASSIFIER_OFF;
  reg_cfg_cmd[3].u.rw_info.len = ISP_DEMOSAIC_CLASSIFIER_LEN *
    sizeof(uint32_t);

  reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset +
    reg_cfg_cmd[3].u.rw_info.len;
  reg_cfg_cmd[4].cmd_type = VFE_WRITE;
  reg_cfg_cmd[4].u.rw_info.reg_offset = ISP_DEMOSAIC_INTERP_NOISE_OFF;
  reg_cfg_cmd[4].u.rw_info.len = ISP_DEMOSAIC_INTERP_NOISE_LEN *
    sizeof(uint32_t);

  ISP_LOG_LIST("%s: hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: demosaic47_util_append_cfg");
    goto ERROR_APPEND;

  }

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);
  return FALSE;
} /* demosaic47_do_hw_update */


/** demosaic47_manual_update:
 *
   @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  This function uses the manual CC gain values and update
 *  the HW
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_manual_update(isp_sub_module_t *isp_sub_module)
{
  demosaic47_t                 *wb = NULL;
  boolean                       ret = TRUE;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  ISP_DemosaicConfigCmdType    *p_cmd = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  wb = (demosaic47_t *)isp_sub_module->private_data;
  if (!wb) {
    ISP_ERR("failed: wb %p", wb);
    return FALSE;
  }

  wb->gain.r_gain = isp_sub_module->manual_ctrls.cc_gain.gains[0];
  /* Use the G-even and ignore G-odd */
  wb->gain.g_gain = isp_sub_module->manual_ctrls.cc_gain.gains[1];
  wb->gain.b_gain = isp_sub_module->manual_ctrls.cc_gain.gains[3];
  ISP_DBG("its_debug gains %f %f %f", wb->gain.r_gain,
    wb->gain.g_gain, wb->gain.b_gain);

  return ret;
}


/** demosaic47_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module instance
 *  @event: mct event for trigger update
 *
 *  This function checks and initiates triger update of module
 *
 *  Return TRUE on Success and FALSE Parameters size mismatch
 **/
boolean demosaic47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  int                           temp = 0;
  float                         ratio = 0.0;
  Chromatix_demosaic4_type     *chromatix_demosaic4 = NULL;
  Demosaic4_reserve            *reserve = NULL;
  Demosaic4_core               *normal_data = NULL;
  Demosaic4_core               *lowlight_data = NULL;
  ISP_DemosaicConfigCmdType    *p_cmd = NULL;
  chromatix_parms_type         *chromatix_ptr = NULL;

  awb_gain_t                   *wb_gain = NULL;
  aec_update_t                 *aec_output = NULL;
  tuning_control_type          *tc = NULL;
  trigger_point_type           *tp = NULL;
  demosaic47_t                 *mod = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  float                         new_aG = 0.0, new_bL = 0.0;
  isp_sub_module_output_t      *sub_module_output = NULL;
  int                           i = 0;
  uint32_t  multi_factor = 1 << (ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH);
  float  ak_multi_factor = 1.0f;
  boolean lsb_allignment = FALSE;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }
  if (lsb_allignment){
      multi_factor = 1;
      ak_multi_factor =1.0f/(float)(1 <<(ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH));
  } else {
      multi_factor = 1 << (ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH);
      ak_multi_factor =1.0f * (float)(1 <<(ISP_PIPELINE_WIDTH- CHROMATIX_BITWIDTH));
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
      !isp_sub_module->config_pending) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.cc_mode ==
        CAM_COLOR_CORRECTION_TRANSFORM_MATRIX &&
      ((isp_sub_module->manual_ctrls.wb_mode == CAM_WB_MODE_OFF &&
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_AUTO) ||
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF)) {
        isp_sub_module->submod_trigger_enable = TRUE;

        if (isp_sub_module->manual_ctrls.update_cc_gain) {
          ret = demosaic47_manual_update(isp_sub_module);
          isp_sub_module->manual_ctrls.update_cc_gain = FALSE;
          isp_sub_module->trigger_update_pending = TRUE;
        }
    } else {
        isp_sub_module->submod_trigger_enable = TRUE;
        isp_sub_module->trigger_update_pending = TRUE;
      if (isp_sub_module->manual_ctrls.update_cc_gain){
        ISP_ERR("Manual CC GAIN not applied");
      }
    }
  }

  if (isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if ((isp_sub_module->submod_enable == FALSE) ||
      (isp_sub_module->submod_trigger_enable == FALSE)) {
    ISP_DBG("No trigger update for Demosaic: enable = %d, trigger_en = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }
  if (isp_sub_module->trigger_update_pending == TRUE) {
    p_cmd = &mod->reg_cmd;

    chromatix_demosaic4 = &chromatix_ptr->chromatix_VFE.chromatix_demosaic4;
    reserve = (Demosaic4_reserve*)&(chromatix_demosaic4->demosaic4_reserve);
    normal_data = (Demosaic4_core*)&(chromatix_demosaic4->demosaic4_data[NORMAL_LIGHT]);
    lowlight_data = (Demosaic4_core*)&(chromatix_demosaic4->demosaic4_data[LOW_LIGHT]);

    wb_gain = &mod->gain;
    ratio = mod->ratio.ratio;

    /* no interpolation, pick from start region,
       in demosaic case only have normal and low,
       so it always pick normal (start) till turn to pure lowligh*/
    if (ratio != 0.0) {
      p_cmd->demoCfg.dirGInterpDisable = normal_data->dis_directional_G;
      p_cmd->demoCfg.dirRBInterpDisable = normal_data->dis_directional_RB;
    } else {
      p_cmd->demoCfg.dirGInterpDisable =  lowlight_data->dis_directional_G;
      p_cmd->demoCfg.dirRBInterpDisable = lowlight_data->dis_directional_RB;
    }

    p_cmd->interpCoeffCfg.wgr1 = reserve->non_dir_weight_1;
    p_cmd->interpCoeffCfg.wgr2 = reserve->non_dir_weight_2;

    /* lambda values no Q conversion */
    p_cmd->interpCoeffCfg.lambda_g =
      LINEAR_INTERPOLATION_INT(normal_data->low_freq_weight_G,
      lowlight_data->low_freq_weight_G, ratio);

    p_cmd->interpCoeffCfg.lambda_rb =
      LINEAR_INTERPOLATION_INT(normal_data->low_freq_weight_RB,
      lowlight_data->low_freq_weight_RB, ratio);

    /* noise level need to times shift bit then round, so cant use interp int*/
    p_cmd->interpNoiseCfg.noise_level_g = roundf(LINEAR_INTERPOLATION(
      normal_data->noise_level_G, lowlight_data->noise_level_G, ratio)
      * multi_factor);
    p_cmd->interpNoiseCfg.noise_level_rb = roundf(LINEAR_INTERPOLATION(
      normal_data->noise_level_RB, lowlight_data->noise_level_RB, ratio)
      * multi_factor);

    /* update wb gains */
    ISP_DBG("gains r %f g %f b %f", wb_gain->r_gain, wb_gain->g_gain,
      wb_gain->b_gain);

    /* Classifier */
    for (i = 0; i < ISP_DEMOSAIC_CLASSIFIER_CNT; i++) {
      /* wk is float, can not use interpolation int,
         ak need to times shift bit then round, so cant use interp int*/
      p_cmd->interpClassifier[i].w_n = FLOAT_TO_Q(10,
        LINEAR_INTERPOLATION(normal_data->wk[i], lowlight_data->wk[i], ratio));
      p_cmd->interpClassifier[i].t_n = LINEAR_INTERPOLATION_INT(
        normal_data->tk[i], lowlight_data->tk[i], ratio);
      p_cmd->interpClassifier[i].a_n = roundf(LINEAR_INTERPOLATION(
        normal_data->ak[i], lowlight_data->ak[i], ratio) * ak_multi_factor);
    }
    p_cmd->leftWBCfg0.g_gain = FLOAT_TO_Q(7, (wb_gain->g_gain));
    p_cmd->leftWBCfg0.b_gain = FLOAT_TO_Q(7, (wb_gain->b_gain));
    p_cmd->leftWBCfg1.r_gain = FLOAT_TO_Q(7, (wb_gain->r_gain));

    p_cmd->leftWBOffsetCfg0.g_offset = 0;
    p_cmd->leftWBOffsetCfg0.b_offset = 0;
    p_cmd->leftWBOffsetCfg1.r_offset = 0;

    p_cmd->rightWBCfg0.g_gain = FLOAT_TO_Q(7, (wb_gain->g_gain));
    p_cmd->rightWBCfg0.b_gain = FLOAT_TO_Q(7, (wb_gain->b_gain));
    p_cmd->rightWBCfg1.r_gain = FLOAT_TO_Q(7, (wb_gain->r_gain));

    p_cmd->rightWBOffsetCfg0.g_offset = 0;
    p_cmd->rightWBOffsetCfg0.b_offset = 0;
    p_cmd->rightWBOffsetCfg1.r_offset = 0;

    ret = demosaic47_store_hw_update(isp_sub_module, mod);
    if (ret == FALSE) {
      ISP_ERR("failed: demosaic47_do_hw_update");
    }
    mod->applied_RegCmd = mod->reg_cmd;
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
  }

FILL_METADATA:
  if (sub_module_output && isp_sub_module->vfe_diag_enable) {
    ret = demosaic47_fill_vfe_diag_data(mod, isp_sub_module, sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: demosaic47_fill_vfe_diag_data");
    }
  }

  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {
    sub_module_output->meta_dump_params->frame_meta.awb_gain.b_gain =
      mod->gain.b_gain;
    sub_module_output->meta_dump_params->frame_meta.awb_gain.r_gain =
      mod->gain.r_gain;
    sub_module_output->meta_dump_params->frame_meta.awb_gain.g_gain =
      mod->gain.g_gain;
  }

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->awb_update =
      mod->awb_update;
    sub_module_output->frame_meta->app_ctrl_mode =
      isp_sub_module->manual_ctrls.app_ctrl_mode;
    sub_module_output->frame_meta->cc_gain.gains[0] =
      mod->gain.r_gain;
    sub_module_output->frame_meta->cc_gain.gains[1] =
      mod->gain.g_gain;
    sub_module_output->frame_meta->cc_gain.gains[2] =
      mod->gain.g_gain;
    sub_module_output->frame_meta->cc_gain.gains[3] =
      mod->gain.b_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* demosaic47_trigger_update */

/** demosaic47_streamon:
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
boolean demosaic47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  demosaic47_t          *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  demosaic47_set_chromatix_params(mod, isp_sub_module,
    isp_sub_module->chromatix_ptrs.chromatixPtr);

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* demosaic47_config */

/** demosaic47_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demosaic47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demosaic47_t *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  isp_sub_module->trigger_update_pending = FALSE;

  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* demosaic47_streamoff */

/** demosaic47_reset:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets demosaic module
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demosaic47_reset(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  demosaic47_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  return TRUE;
} /* demosaic47_reset */

/** demosaic47_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t            *stats_update = NULL;
  chromatix_parms_type      *chromatix_ptr = NULL;
  Chromatix_demosaic4_type  *chromatix_demosaic = NULL;
  demosaic47_t              *mod = NULL;
  tuning_control_type       tc;
  trigger_point_type        *tp = NULL;
  float                      ratio = 0.0;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    /* Put hw update flag to TRUE */
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_demosaic = &chromatix_ptr->chromatix_VFE.chromatix_demosaic4;
  if (chromatix_demosaic == NULL) {
    ISP_ERR("failed: null pointer %p", chromatix_demosaic);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(&tc, 0, sizeof(tuning_control_type));
  /* TODO use correct chromatix demosaic field */
  tc = chromatix_demosaic->control_demosaic4;
  tp = &chromatix_demosaic->demosaic4_trigger_lowlight;
  /*Do interpolation by the aec ratio*/
  ratio = isp_sub_module_util_get_aec_ratio_lowlight(tc, tp,
      &stats_update->aec_update, 1);
  ISP_DBG("aec ratio %f", ratio);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
      !F_EQUAL(mod->ratio.ratio, ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  mod->ratio.ratio = ratio;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** demosaic47_stats_awb_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t *stats_update = NULL;
  awb_gain_t     *cur_awb_gain = NULL;
  awb_gain_t     *new_awb_gain = NULL;
  demosaic47_t   *mod = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic47_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  // We need to get the updated awb from stats properly to demosaic module
  mod->awb_update = stats_update->awb_update;

  cur_awb_gain = &mod->gain;
  new_awb_gain = &stats_update->awb_update.gain;

  if (!F_EQUAL(cur_awb_gain->r_gain, new_awb_gain->r_gain) ||
      !F_EQUAL(cur_awb_gain->g_gain, new_awb_gain->g_gain) ||
      !F_EQUAL(cur_awb_gain->b_gain, new_awb_gain->b_gain)) {
    isp_sub_module->trigger_update_pending = TRUE;

    /* Store AWB update in module private */
    *cur_awb_gain = *new_awb_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** demosaic47_stats_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle set chromatix ptr event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  chromatix_parms_type      *chromatix_ptr = NULL;
  modulesChromatix_t        *chromatix_ptrs = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->submod_enable = FALSE;
  isp_sub_module->update_module_bit = TRUE;

  if (!isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed: %p updating module enable bit for hw %d", isp_sub_module,
      isp_sub_module->hw_module_id);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  if (chromatix_ptr->chromatix_VFE.chromatix_demosaic4.demosaic4_enable > 0) {
     isp_sub_module->submod_enable = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* demosaic47_set_chromatix_ptr */

/** demosaic47_ez_isp_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @demosaic_module: demosaic module handle
 *  @demosaicDiag: demosaic Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void demosaic47_ez_isp_update(isp_sub_module_t *isp_sub_module,
  demosaic47_t *demosaic_module, demosaic3_t *demosaicDiag)
{
  ISP_DemosaicConfigCmdType *demosaicCfg = NULL;
  int index;
  if (!isp_sub_module || !demosaic_module || !demosaicDiag) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, demosaic_module, demosaicDiag);
    return;
  }
  demosaicCfg = &(demosaic_module->reg_cmd);
  if (!demosaicCfg)
    return;

  for(index = 0; index < ISP_DEMOSAIC_CLASSIFIER_CNT; index++) {
    demosaicDiag->lut[index].wk = demosaicCfg->interpClassifier[index].w_n;
    demosaicDiag->lut[index].tk = demosaicCfg->interpClassifier[index].t_n;
    /* TODO ez tune support for chromatix 305
       demosaicDiag->lut[index].ak = demosaicCfg->interpClassifier[index].a_n;*/
  }
}

/** demosaic47_get_vfe_diag_info_user
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  demosaic47_t        *demosaic = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  demosaic3_t         *demosaicDiag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  demosaic = (demosaic47_t *)isp_sub_module->private_data;
  if (!demosaic) {
    ISP_ERR("failed: demosaic %p", demosaic);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  vfe_diag = (vfe_diagnostics_t *)private_event->data;
  if (!vfe_diag) {
    ISP_ERR("failed: vfe_diag %p", vfe_diag);
    return FALSE;
  }

  if (sizeof(vfe_diagnostics_t) != private_event->data_size) {
    ISP_ERR("failed: out_param_size mismatch, param_id = %d",
      private_event->type);
    return FALSE;
  }

  demosaicDiag = &(vfe_diag->prev_demosaic);
  if (demosaic->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
    demosaicDiag = &(vfe_diag->snap_demosaic);
  }

  demosaic47_ez_isp_update(isp_sub_module, demosaic, demosaicDiag);

  return TRUE;
}

/** demosaic47_open:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function instantiates a demosaic module
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  demosaic47_t *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic47_t *)CAM_MALLOC(sizeof(demosaic47_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  /* enable trigger update feature flag from PIX */

  isp_sub_module->private_data = (void *)mod;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;

  return TRUE;
} /* demosaic47_init */

/** demosaic47_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void demosaic47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* demosaic47_destroy */
