/* bpc48.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "bpc48.h"
#include "isp_pipeline_reg.h"
#include "isp_zzhdr_tune_def.h"

isp_zzhdr_2d_lut_type zzhdr_bpc_tuning_data =
    #include "isp_zzhdr_tune_param.h"
;

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BPC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BPC, fmt, ##args)

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

/** bpc48_cmd_debug:
 *
 *  @cmd: bpc config cmd
 *
 *  This function dumps the bpc module configuration set to hw
 *
 *  Returns nothing
 **/
static void bpc48_cmd_debug(ISP_BPC_Reg_t *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }

  ISP_DBG("cmd->bpc_cfg0.BlkLevel = %d", cmd->bpc_regcfg.bpc_cfg0.BlkLevel);
  ISP_DBG("cmd->bpc_cfg0.SameChRecover = %d", cmd->bpc_regcfg.bpc_cfg0.SameChRecover);
  ISP_DBG("cmd->bpc_cfg0.ColdPixCor_Disable = %d",
          cmd->bpc_regcfg.bpc_cfg0.ColdPixCor_Disable);
  ISP_DBG("cmd->bpc_cfg0.HotPixCor_Disable = %d",
          cmd->bpc_regcfg.bpc_cfg0.HotPixCor_Disable);

  ISP_DBG("cmd->bpc_cfg1.RG_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg1.RG_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg2.BG_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg2.BG_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg3.GR_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg3.GR_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg4.GB_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg4.GB_WB_GainRatio);

  ISP_DBG("cmd->bpc_cfg5.BCC_Offset = %d",
    cmd->bpc_regcfg.bpc_cfg5.BCC_Offset);
  ISP_DBG("cmd->bpc_cfg5.BPC_Offset = %d",
    cmd->bpc_regcfg.bpc_cfg5.BPC_Offset);

  ISP_DBG("cmd->bpc_cfg6.CorrectThreshold = %d",
    cmd->bpc_regcfg.bpc_cfg6.CorrectThreshold);
  ISP_DBG("cmd->bpc_cfg6.Fmin = %d", cmd->bpc_regcfg.bpc_cfg6.Fmin);
  ISP_DBG("cmd->bpc_cfg6.Fmax = %d", cmd->bpc_regcfg.bpc_cfg6.Fmax);

} /* bpc48_cmd_debug */


/** bpc48_convert_to_int:
 *
 *  @val: input signed char variable to be converted to int
 *  Convert char to int
 *
 *  Returns converted int value
 **/
int bpc48_convert_to_int(char val)
{
  int num = val;
  if (num > 127) {
    num = -(256 - num);
  }
  return num;
}
/** bpc48_clamp_min_max:
 *
 *  @val: input char variable to be clamped
 *  @min: minimum value
 *  @max: maximum value
 *  @num_bits: number of bits of the final clamped value
 *
 *  CLAMP the input to min/max and adjust the final bits to the
 *  desired bits in num_bits
 *  Returns clamped value
 **/
uint32_t bpc48_clamp_min_max(char val, int min, int max, int num_bits)
{
  int num = 0;
  num = bpc48_convert_to_int(val);
  num = CLAMP(num, min, max);
  if (num < 0) {
    num = -num;
    num = (1 << num_bits) - num;
  }
  return num;
}

/** bpc48_init_config:
 *
 *  @mod: bpc module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void bpc48_init_config(isp_sub_module_t *isp_sub_module, bpc48_t *bpc2)
{
  ISP_BPC_Reg_t                     *Reg = NULL;
  chromatix_parms_type              *chromatix_ptr = NULL;
  Chromatix_BPC2_type               *chromatix_BPC2 = NULL;
  BPC2_reserved_type                *bpc2_reserved_data= NULL;
  BPC2_core_type                    *bpc2_data = NULL;
  chromatix_VFE_common_type         *pchromatix_common = NULL;
  Chromatix_blk_subtract_scale_type *chromatix_blk_lvl = NULL;

  if (!isp_sub_module || !bpc2) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc2);
    return;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_ptr || !pchromatix_common) {
    ISP_ERR("failed: chromatix_ptr %p, pchromatix_common %p",
      chromatix_ptr, pchromatix_common);
    return;
  }

  bpc2_reserved_data =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data.BPC2_reserved_data;
  chromatix_BPC2 =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;

  chromatix_blk_lvl =
    &pchromatix_common->Chromatix_BLSS_data;

  bpc2_data = &chromatix_BPC2->BPC2_data[DEFAULT_CONFIG_INDEX];

  Reg = &bpc2->Reg;
  Reg->bpc_regcfg.bpc_cfg0.HotPixCor_Disable = CLAMP(
     bpc2_reserved_data->hot_pixel_correction_disable, 0, 1);
  Reg->bpc_regcfg.bpc_cfg0.ColdPixCor_Disable = CLAMP(
     bpc2_reserved_data->cold_pixel_correction_disable, 0, 1);
  Reg->bpc_regcfg.bpc_cfg0.SameChRecover = CLAMP(
     bpc2_data->same_channel_recovery, 0, 1);
  if (chromatix_blk_lvl->BLSS_enable) {
    Reg->bpc_regcfg.bpc_cfg0.BlkLevel = CLAMP(
      chromatix_blk_lvl->black_level_normal.black_level_offset,
      BPC_MIN_BLKLVL, BPC_MAX_BLKLVL);
  } else {
    Reg->bpc_regcfg.bpc_cfg0.BlkLevel = 0;
  }

  Reg->bpc_regcfg.bpc_cfg5.BPC_Offset =
    CLAMP(bpc2_data->BPC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  Reg->bpc_regcfg.bpc_cfg5.BCC_Offset =
    CLAMP(bpc2_data->BCC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);

  Reg->bpc_regcfg.bpc_cfg6.Fmax =
    CLAMP(bpc2_data->fmax, BPC_MIN_FMAX, BPC_MAX_FMAX);
  Reg->bpc_regcfg.bpc_cfg6.Fmin =
    CLAMP(bpc2_data->fmin, BPC_MIN_FMIN, BPC_MAX_FMIN);

  Reg->bpc_regcfg.bpc_cfg6.CorrectThreshold =
    CLAMP(bpc2_data->correction_threshold, BPC_MIN_COR_THR, BPC_MAX_COR_THR);

} /* bpc48_init_config */

/** bpc48_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc48_store_hw_update(isp_sub_module_t *isp_sub_module,
  bpc48_t *bpc2)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_BPCRegConfig            *RegCmd = NULL;
  uint32_t                    *val = NULL;

  if (!isp_sub_module || !bpc2) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc2);
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
    * ISP_BPC_TOTAL_NUM_REG);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * ISP_BPC_TOTAL_NUM_REG);

  RegCmd = (ISP_BPCRegConfig *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BPC_CMD;
  }
  *RegCmd = bpc2->Reg.bpc_regcfg;

  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = ISP_BPC_TOTAL_NUM_REG;

  /* write ISP_BPCCfg0 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[0],
    0, sizeof(RegCmd->bpc_cfg0), VFE_WRITE, ISP_BPC_CFG0_OFF);

  /* write ISP_BPCCfg1 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1],
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg1), VFE_WRITE, ISP_BPC_CFG1_OFF);

  /* write ISP_BPCCfg2 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[2],
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + reg_cfg_cmd[1].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg2), VFE_WRITE, ISP_BPC_CFG2_OFF);

  /* write ISP_BPCCfg3 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[3],
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset + reg_cfg_cmd[2].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg3), VFE_WRITE, ISP_BPC_CFG3_OFF);

  /* write ISP_BPCCfg4 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[4],
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset + reg_cfg_cmd[3].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg4), VFE_WRITE, ISP_BPC_CFG4_OFF);

  /* write ISP_BPCCfg5 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[5],
    reg_cfg_cmd[4].u.rw_info.cmd_data_offset + reg_cfg_cmd[4].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg5), VFE_WRITE, ISP_BPC_CFG5_OFF);

  /* write ISP_BPCCfg6 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[6],
    reg_cfg_cmd[5].u.rw_info.cmd_data_offset + reg_cfg_cmd[5].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg6), VFE_WRITE, ISP_BPC_CFG6_OFF);

  bpc48_cmd_debug(&bpc2->Reg);

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
} /* bpc48_store_hw_update */

/** bpc48_interpolate:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @bpc2: bpc struct data
 *
 * interpolate bpc state with the aec ratio
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc48_interpolate(isp_sub_module_t *isp_sub_mod, bpc48_t *bpc2)
{
  BPC2_core_type           *bpc2_data_start = NULL;
  BPC2_core_type           *bpc2_data_end = NULL;
  BPC2_core_type           *bpc2_data = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  Chromatix_BPC2_type      *chromatix_bpc2 = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;
  chromatix_VFE_common_type *pchromatix_common = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_blk_lvl = NULL;
  float                     aec_ratio_bpc = 0;
  awb_gain_t               *awb_gain;
  bpc_params_t             *bpc_params = NULL;
  float                    multfactor = 1.0f;
  boolean ret = TRUE;

  if (!isp_sub_mod || !bpc2) {
    ISP_ERR("failed isp_sub_mod %p bpc2 %p", isp_sub_mod, bpc2);
    return FALSE;
  }

  awb_gain = &bpc2->curr_awb_gain;
  bpc_params = &bpc2->bpc_params;
  aec_ratio_bpc = bpc2->aec_ratio_bpc.ratio;
  chromatix_ptr = isp_sub_mod->chromatix_ptrs.chromatixPtr;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_mod->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_ptr || !pchromatix_common) {
    ISP_ERR("failed: chromatix_ptr %p, pchromatix_common %p",
      chromatix_ptr, pchromatix_common);
    return FALSE;
  }
  chromatix_bpc2 = &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;
  chromatix_blk_lvl =
    &pchromatix_common->Chromatix_BLSS_data;
  bpc2_data_start = &chromatix_bpc2->BPC2_data[bpc2->trigger_index];

  if ((aec_ratio_bpc > 0) &&
      ((bpc2->trigger_index + 1) <= MAX_LIGHT_TYPES_FOR_SPATIAL)) {
    bpc2_data_end = &chromatix_bpc2->BPC2_data[bpc2->trigger_index + 1];

    bpc_params->Fmax = (LINEAR_INTERPOLATION((float)bpc2_data_end->fmax,
      (float)bpc2_data_start->fmax, aec_ratio_bpc));
    bpc_params->Fmin = roundf(LINEAR_INTERPOLATION((float)bpc2_data_end->fmin,
      (float)bpc2_data_start->fmin, aec_ratio_bpc));
    bpc_params->BPC_offset = LINEAR_INTERPOLATION((float)bpc2_data_end->BPC_offset,
      (float)bpc2_data_start->BPC_offset, aec_ratio_bpc);
    bpc_params->BCC_offset = LINEAR_INTERPOLATION((float)bpc2_data_end->BCC_offset,
      (float)bpc2_data_start->BCC_offset, aec_ratio_bpc);
    bpc_params->correction_threshold = LINEAR_INTERPOLATION(
      (float)bpc2_data_end->correction_threshold,
      (float)bpc2_data_start->correction_threshold,
      aec_ratio_bpc);
  } else {
    bpc_params->Fmax = bpc2_data_start->fmax;
    bpc_params->Fmin = bpc2_data_start->fmin;
    bpc_params->BPC_offset = bpc2_data_start->BPC_offset;
    bpc_params->BCC_offset = bpc2_data_start->BCC_offset;
    bpc_params->correction_threshold = bpc2_data_start->correction_threshold;
  }

  if (isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    //LSB alligned data
      multfactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  }
  else{
      multfactor = 1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH);
  }
  if ((bpc2->ext_func_table && bpc2->ext_func_table->apply_2d_interpolation)
    && (bpc2->apply_hdr_effects)) {
    bpc2->ext_func_table->apply_2d_interpolation(bpc2);
  }


  bpc_params->BlkLvl_offset = Round((float)(bpc_params->BlkLvl_offset) * multfactor);
  bpc_params->BPC_offset = CLAMP(Round((bpc_params->BPC_offset * multfactor)),
    BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  bpc_params->BCC_offset = CLAMP(Round((bpc_params->BCC_offset * multfactor)),
    BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  bpc_params->correction_threshold = roundf(CLAMP(
    bpc_params->correction_threshold * multfactor, BPC_MIN_COR_THR,
    BPC_MAX_COR_THR));


  bpc_params->Fmax = CLAMP(Round(bpc_params->Fmax), BPC_MIN_FMAX, BPC_MAX_FMAX);
  bpc_params->Fmin = CLAMP(bpc_params->Fmin, BPC_MIN_FMIN, BPC_MAX_FMIN);
  bpc_params->SameChannelRecovery = CLAMP(bpc2_data_start->same_channel_recovery,
    0, 1);

  return TRUE;
}

/** bpc48_hw_config:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @bpc2: bpc struct data
 *
 *  Configure BPC2 hawrdware registers
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc48_hw_config(isp_sub_module_t *isp_sub_mod, bpc48_t *bpc2)
{
  awb_gain_t               *awb_gain;
  bpc_params_t             *bpc_params;
  if (!isp_sub_mod || !bpc2) {
    ISP_ERR("failed isp_sub_mod %p bpc2 %p", isp_sub_mod, bpc2);
    return FALSE;
  }
  bpc_params = &bpc2->bpc_params;
  awb_gain = &bpc2->curr_awb_gain;

  bpc2->Reg.bpc_regcfg.bpc_cfg0.BlkLevel =
    CLAMP( bpc_params->BlkLvl_offset, BPC_MIN_BLKLVL, BPC_MAX_BLKLVL);
  bpc2->Reg.bpc_regcfg.bpc_cfg0.SameChRecover =
    CLAMP(bpc_params->SameChannelRecovery, 0, 1);

  bpc2->Reg.bpc_regcfg.bpc_cfg1.RG_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->r_gain/awb_gain->g_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg2.BG_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->b_gain/awb_gain->g_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg3.GR_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->g_gain/awb_gain->r_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg4.GB_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->g_gain/awb_gain->b_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);

  bpc2->Reg.bpc_regcfg.bpc_cfg5.BPC_Offset =
    CLAMP(bpc_params->BPC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  bpc2->Reg.bpc_regcfg.bpc_cfg5.BCC_Offset =
    CLAMP(bpc_params->BCC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);

  bpc2->Reg.bpc_regcfg.bpc_cfg6.Fmax = CLAMP(bpc_params->Fmax, BPC_MIN_FMAX, BPC_MAX_FMAX);
  bpc2->Reg.bpc_regcfg.bpc_cfg6.Fmin = CLAMP(bpc_params->Fmin, BPC_MIN_FMIN, BPC_MAX_FMIN);
  bpc2->Reg.bpc_regcfg.bpc_cfg6.CorrectThreshold =
    CLAMP(bpc_params->correction_threshold, BPC_MIN_COR_THR, BPC_MAX_COR_THR);

  return TRUE;
}

/** bpc48_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function adds ref count for stream on flag and sets
 *  trigger_update_pending flag to TRUE for first STREAM ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  bpc48_t               *bpc2;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }
  bpc2 = (bpc48_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* bpc48_streamon */

/** bpc48_ez_isp_update
 *
 *  @mod: bpc module handle
 *  @bpcDiag: bpc Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void bpc48_ez_isp_update(bpc48_t *mod,
  badcorrection_t *bpcDiag)
{
  ISP_BPC_Reg_t *bpcCfg = &(mod->Reg);

  /* TODO: Filling only known metadata in extune structure.
   * When the new eztune header file is released, need
   * to fill all the required metadata */
  bpcDiag->fminThreshold = bpcCfg->bpc_regcfg.bpc_cfg6.Fmin;
  bpcDiag->fmaxThreshold = bpcCfg->bpc_regcfg.bpc_cfg6.Fmax;
}/* bpc48_ez_isp_update */

/** bpc48_fill_vfe_diag_data:
 *
 *  @bpc: bpc module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean bpc48_fill_vfe_diag_data(bpc48_t *mod,
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

    bpc48_ez_isp_update(mod, bpcDiag);
  }
  return ret;
}
/** bpc48_fetch_blkLvl_offset:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Black Level offset
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bpc48_fetch_blkLvl_offset(mct_module_t *module,
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

/** bpc48_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bpc48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  bpc48_t                      *bpc2 = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  uint32_t                      BlkLvl_offset = 0;
  uint8_t                       module_enable;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc2 = (bpc48_t *)isp_sub_module->private_data;

  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  private_event = (isp_private_event_t *)
                    (event->u.module_event.module_event_data);
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

  if (bpc2->ext_func_table && bpc2->ext_func_table->check_enable_idx) {
    bpc2->ext_func_table->check_enable_idx(bpc2, isp_sub_module, output);
    if (isp_sub_module->submod_enable) {
      bpc48_decide_hysterisis(isp_sub_module, bpc2->aec_reference);
    }
  }
  else {
    bpc48_decide_hysterisis(isp_sub_module, bpc2->aec_reference);
  }

  private_event = (isp_private_event_t *)event->u.module_event.
    module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    goto FILL_METADATA;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
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

    ret = bpc48_fetch_blkLvl_offset(module, event->identity,
      &BlkLvl_offset);
    if (bpc2->bpc_params.BlkLvl_offset != BlkLvl_offset) {
      bpc2->bpc_params.BlkLvl_offset = BlkLvl_offset;
      isp_sub_module->trigger_update_pending = TRUE;
    }

  if (((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) ||
	(bpc2->apply_hdr_effects)){
    ret = bpc48_interpolate(isp_sub_module, bpc2);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc48_interpolate");
      goto FILL_METADATA;
    }
    ret = bpc48_hw_config(isp_sub_module, bpc2);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc48_hw_config");
      goto FILL_METADATA;
    }
    isp_sub_module->trigger_update_pending = FALSE;
	bpc2->apply_hdr_effects = 0;

    bpc48_store_hw_update(isp_sub_module, bpc2);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = bpc48_fill_vfe_diag_data(bpc2, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc44_fill_vfe_diag_data");
    }
  }
  if (output->frame_meta)
    output->frame_meta->hot_pix_mode =
      isp_sub_module->manual_ctrls.hot_pix_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** bpc48_stats_awb_update:
 *
 * @mod: bpc48 module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc48_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t                      *stats_update = NULL;
  awb_gain_t                          *cur_awb_gain = NULL;
  awb_gain_t                          *new_awb_gain = NULL;
  bpc48_t                             *bpc48;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }
  stats_update = (stats_update_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bpc48 = isp_sub_module->private_data;
  if (!bpc48) {
    ISP_ERR("failed %p", bpc48);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  new_awb_gain = &stats_update->awb_update.gain;
  cur_awb_gain = &bpc48->curr_awb_gain;

  if (!AWB_GAIN_EQUAL(new_awb_gain, cur_awb_gain) &&
       !AWB_GAIN_EQ_ZERO(new_awb_gain)) {
     isp_sub_module->trigger_update_pending = TRUE;

     ISP_DBG("old gain g=%f b=%f r=%f new g=%f b=%f r=%f",
       cur_awb_gain->g_gain, cur_awb_gain->b_gain,
       cur_awb_gain->r_gain, new_awb_gain->g_gain,
       new_awb_gain->b_gain, new_awb_gain->r_gain);
      *cur_awb_gain = *new_awb_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

}

/** bpc48_decide_hysterisis:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle hysterisis trigger
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc48_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref)
{
  chromatix_parms_type *chromatix_ptr  = NULL;
  Chromatix_BPC2_type  *chromatix_bpc  = NULL;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_bpc =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;

  return isp_sub_module_util_decide_hysterisis(isp_sub_module,
    aec_ref, chromatix_bpc->BPC2_en, chromatix_bpc->bpc2_snapshot_hyst_en_flag,
    chromatix_bpc->control_BPC2, &chromatix_bpc->bpc2_hysteresis_trigger);
}

/** bpc48_stats_aec_update:
 *
 * @mod: bpc48 module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc48_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t                       *stats_update = NULL;
  bpc48_t                              *bpc2 = NULL;
  chromatix_parms_type                 *chromatix_ptr = NULL;
  Chromatix_BPC2_type                  *chromatix_BPC2 = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_blk_lvl = NULL;
  chromatix_VFE_common_type            *pchromatix_common = NULL;
  uint8_t                               trigger_index =
                                        MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float                                 aec_reference;
  float                                 aec_ratio = 0;
  float                                 aec_ratio_blklvl = 0;
  float                                 ratio = 0;
  float                                 new_sensor_gain = 0;
  int                                   i = 0;
  float start = 0;
  float end = 0;
  boolean ret = TRUE;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc2 = (bpc48_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc %p", bpc2);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_ptr || !pchromatix_common) {
    ISP_ERR("failed: chromatix_ptr %p, pchromatix_common%p",
      chromatix_ptr,pchromatix_common);
    goto ERROR;
  }

  bpc2->aec_update = stats_update->aec_update;

  bpc2->aec_lux_reference = stats_update->aec_update.lux_idx;
  bpc2->aec_gain_reference = stats_update->aec_update.sensor_gain;
  bpc2->hdr_sensitivity_ratio = stats_update->aec_update.hdr_sensitivity_ratio;
  bpc2->hdr_exp_time_ratio = stats_update->aec_update.hdr_exp_time_ratio;

  chromatix_BPC2 =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_blk_lvl =
    &pchromatix_common->Chromatix_BLSS_data;

  if (chromatix_BPC2->control_BPC2 == 0) {
    /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
  } else {
    /* Gain based */
    aec_reference = stats_update->aec_update.sensor_gain;
  }

  ISP_DBG("Lux index %f Gain %f", stats_update->aec_update.lux_idx,
    stats_update->aec_update.sensor_gain);

  new_sensor_gain = stats_update->aec_update.sensor_gain;
  ISP_HIGH("aec_reference :%f", aec_reference);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    start = 0;
    end = 0;
    trigger_point_type *bpc2_trigger =
      &(chromatix_BPC2->BPC2_data[i].BPC2_trigger);
    trigger_index = i;

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_BPC2->control_BPC2 == 0) {
      start = bpc2_trigger->lux_index_start;
      end   = bpc2_trigger->lux_index_end;
    } else {
      start = bpc2_trigger->gain_start;
      end   = bpc2_trigger->gain_end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }

  bpc2->aec_reference = aec_reference;

  ISP_DBG("[%f - %f - %f] = %f index %d", start, aec_reference, end,
    ratio, i);

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for BPC2 trigger value: %f\n", aec_reference);
    goto ERROR;
  }
  if ((trigger_index != bpc2->trigger_index) ||
     (!F_EQUAL(ratio, bpc2->aec_ratio_bpc.ratio)) ||
     (!F_EQUAL(new_sensor_gain, bpc2->curr_sensor_gain))) {
    bpc2->trigger_index = trigger_index;
    bpc2->aec_ratio_bpc.ratio = ratio;
    bpc2->curr_sensor_gain = new_sensor_gain;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if (bpc2->ext_func_table && bpc2->ext_func_table->get_2d_interpolation) {
    ret = bpc2->ext_func_table->get_2d_interpolation(isp_sub_module, bpc2);
    if (ret == FALSE) {
      ISP_ERR("failed: get_2d_interpolation");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}/* bpc48_stats_aec_update*/

/** bpc48_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  bpc48_t            *bpc = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc48_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: mod %p", bpc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *(modulesChromatix_t *)data;
  isp_sub_module->trigger_update_pending = TRUE;
  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }
  bpc48_init_config(isp_sub_module, bpc);

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* bpc48_set_chromatix_ptr */

/** bpc48_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc48_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bpc48_t *bpc = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bpc48_streamoff */

/** bpc48_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bpc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bpc48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  bpc48_t *bpc = NULL;

  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  bpc = (bpc48_t *)CAM_MALLOC(sizeof(bpc48_t));
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  memset(bpc, 0, sizeof(*bpc));

  isp_sub_module->private_data = (void *)bpc;
  FILL_FUNC_TABLE(bpc);
  isp_sub_module->manual_ctrls.hot_pix_mode = CAM_HOTPIXEL_MODE_FAST;

  return TRUE;
}/* bpc48_init */

/** bpc48_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void bpc48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* bpc48_destroy */

#if !OVERRIDE_FUNC

void bpc48_apply_2d_interpolation(void *data) {

  bpc48_t   *bpc;
  bpc = (bpc48_t *)data;
  bpc->bpc_params.BPC_offset *= bpc->bpc48_ext_params.bpc_offset_adj;
  bpc->bpc_params.BCC_offset *= bpc->bpc48_ext_params.bpc_offset_adj;
  bpc->bpc_params.Fmax *= bpc->bpc48_ext_params.bpc_Fmax_adj;
  bpc->bpc_params.Fmax = roundf(bpc->bpc_params.Fmax);

}

void bpc48_2d_interpolate(bpc48_ext_t *local_params,
  bpc48_ext_t *local_params1,
  bpc48_ext_t *local_params2, float ratio) {

  local_params->bpc_offset_adj = (LINEAR_INTERPOLATION(
    local_params1->bpc_offset_adj, local_params2->bpc_offset_adj,
    ratio));

  local_params->bpc_Fmax_adj = (LINEAR_INTERPOLATION((float)
    local_params1->bpc_Fmax_adj, local_params2->bpc_Fmax_adj,
    ratio));

}

static void bpc_interpolate_region_parameters(
  bpc_pdpc_hdr_2d_exp_type *bpc_exp_data,
  uint32_t trigger_idx_start, uint32_t trigger_idx_end,
  bpc48_ext_t *local_params, float ratio) {

    local_params->bpc_offset_adj = (LINEAR_INTERPOLATION((float)
      bpc_exp_data[trigger_idx_start].bpc_pdpc_hdr_2d_reserve_data.
      bpc_offset_adj,
      (float)bpc_exp_data[trigger_idx_end].bpc_pdpc_hdr_2d_reserve_data.
      bpc_offset_adj,
      ratio));

    local_params->bpc_Fmax_adj = (LINEAR_INTERPOLATION((float)
      bpc_exp_data[trigger_idx_start].bpc_pdpc_hdr_2d_reserve_data.
      bpc_Fmax_adj,
      (float)bpc_exp_data[trigger_idx_end].bpc_pdpc_hdr_2d_reserve_data.
      bpc_Fmax_adj,
      ratio));

  }

static boolean bpc_interpolate_parameters(bpc_pdpc_2d_adj_type
  *chromatix_2d, bpc48_t *bpc, bpc_pdpc_hdr_2d_exp_type
  *bpc_exp_data, bpc48_ext_t *local_params) {

  trigger_point2_type      *bpc_trigger       = NULL;
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
      hdr_trigger_ref = bpc->hdr_sensitivity_ratio;
      bpc_trigger   = &(bpc_exp_data[i].aec_sensitivity_ratio);
      start         = bpc_trigger->start;
      end           = bpc_trigger->end;

      break;
    case  CONTROL_EXP_TIME_RATIO:
      /* exp time based */
      hdr_trigger_ref = bpc->hdr_exp_time_ratio;
      bpc_trigger   = &(bpc_exp_data[i].exp_time_trigger);
      start         = bpc_trigger->start;
      end           = bpc_trigger->end;
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

  bpc_interpolate_region_parameters(bpc_exp_data,
    trigger_idx_start, trigger_idx_end, local_params, ratio);
error:
  return ret;
}


static boolean bpc48_process_regions(bpc_pdpc_2d_adj_type
  *chromatix_2d,
  bpc48_t   *bpc, uint32_t *trigger_idx_start, uint32_t *trigger_idx_end,
  float *ratio, tuning_control_type control_type) {
  boolean                  ret               = TRUE;
  trigger_point_type      *bpc_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;


  if (!chromatix_2d || !bpc) {
    ISP_ERR("failed: %p %p", chromatix_2d, bpc);
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
      aec_reference = bpc->aec_lux_reference;
      bpc_trigger   = &(chromatix_2d->bpc_pdpc_hdr_rgn_data[i].trigger);
      start         = bpc_trigger->lux_index_start;
      end           = bpc_trigger->lux_index_end;
      ISP_ERR("lux base, lux idx %f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = bpc->aec_gain_reference;
      bpc_trigger   = &(chromatix_2d->bpc_pdpc_hdr_rgn_data[i].trigger);
      start         = bpc_trigger->gain_start;
      end           = bpc_trigger->gain_end;
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

/** bpc48_ext_2d_interpolation:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc48_2d_interpolation(void *data1,
  void *data2 )
{
  isp_sub_module_t *isp_sub_module;
  bpc48_t   *bpc = NULL;
  boolean   ret  = TRUE;
  chromatix_parms_type  *chromatix_ptr = NULL;
  bpc_pdpc_2d_adj_type *bpc_2d_adj_type = NULL;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float     ratio = 0.0f;
  bpc48_ext_t  bpc48_ext_params_1, bpc48_ext_params_2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  memset(&bpc48_ext_params_1, 0, sizeof(bpc48_ext_t));
  memset(&bpc48_ext_params_1, 0, sizeof(bpc48_ext_t));

  isp_sub_module = (isp_sub_module_t *)data1;
  bpc = (bpc48_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }

  bpc_2d_adj_type = &zzhdr_bpc_tuning_data.bpc_2d_data;

  if (bpc_2d_adj_type->bpc_adj_enable &&
    (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)) {
    bpc48_process_regions(bpc_2d_adj_type, bpc, &trigger_idx_start,
      &trigger_idx_end, &ratio, bpc_2d_adj_type->exp_control);

    if (trigger_idx_start != trigger_idx_end) {
      /* Compute the params based on exp_time_trigger/ aec_sensitivity_ratio */
      bpc_interpolate_parameters(bpc_2d_adj_type, bpc,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_start].
        bpc_pdpc_hdr_exp_data, &bpc48_ext_params_1);
      bpc_interpolate_parameters(bpc_2d_adj_type, bpc,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_end].
        bpc_pdpc_hdr_exp_data, &bpc48_ext_params_2);

      bpc48_2d_interpolate(&bpc->bpc48_ext_params, &bpc48_ext_params_1,
        &bpc48_ext_params_2, ratio);
    } else {
      bpc_interpolate_parameters(bpc_2d_adj_type, bpc,
        bpc_2d_adj_type->bpc_pdpc_hdr_rgn_data[trigger_idx_start].
        bpc_pdpc_hdr_exp_data, &bpc->bpc48_ext_params);
    }
    //Assign local_params to bpc parameters. One more function or do a staright copy
    bpc->apply_hdr_effects = 1;
  }

  return TRUE;
}

static ext_override_func bpc_override_func = {
  .check_enable_idx            = NULL,
  .get_2d_interpolation = bpc48_2d_interpolation,
  .apply_2d_interpolation = bpc48_apply_2d_interpolation,
};

boolean bpc48_fill_func_table(bpc48_t *bpc2)
{
  bpc2->ext_func_table = &bpc_override_func;
  return TRUE;
} /* bpc48_fill_func_table */
#endif
