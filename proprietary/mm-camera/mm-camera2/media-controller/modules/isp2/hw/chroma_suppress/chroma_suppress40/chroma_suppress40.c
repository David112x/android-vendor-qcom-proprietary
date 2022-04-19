/* chroma_suppress40.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "chroma_suppress40.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_SUPPRESS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_SUPPRESS, fmt, ##args)

/* #define CHROMA_SUPPRESS40_DEBUG */
#ifdef CHROMA_SUPPRESS40_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_HIGH
#endif

#define CHROMA_SUPPRESS_DIFF_MIN 4
#define CHROMA_SUPPRESS_DIFF_MAX 127

/** chroma_suppress40_cmd_debug:
 *
 *  @cmd: chroma_suppress config cmd
 *  @mix1_cmd: mix1 cmd
 *  @mix2_cmd: mix2 cmd
 *
 *  This function dumps the chroma_suppress module configuration set to hw
 *
 *  Returns nothing
 **/
static void chroma_suppress40_cmd_debug(ISP_ChromaSuppress_ConfigCmdType *cmd,
  ISP_ChromaSuppress_Mix1_ConfigCmdType *mix1_cmd,
  ISP_ChromaSuppress_Mix2_ConfigCmdType *mix2_cmd)
{
  if (!cmd || !mix1_cmd || !mix2_cmd) {
    ISP_ERR("failed: cmd %p mix1_cmd %p mix2_cmd %p", cmd, mix1_cmd,
      mix2_cmd);
    return;
  }
  ISP_DBG("ySup1 : %d ySup2 : %d ySup3 : %d ySup4 : %d",
    cmd->ySup1, cmd->ySup2, cmd->ySup3, cmd->ySup4);

  ISP_DBG("ySupM1 : %d ySupM3 : %d ySupS1 : %d ySupS3 : %d",
    mix1_cmd->ySupM1, mix1_cmd->ySupM3, mix1_cmd->ySupS1, mix1_cmd->ySupS3);

  ISP_DBG("chromaSuppressEn : %d", mix1_cmd->chromaSuppressEn);
  ISP_DBG("cSup1 : %d cSup2 : %d cSupM1 : %d cSupS1 : %d",
    mix2_cmd->cSup1, mix2_cmd->cSup2, mix2_cmd->cSupM1, mix2_cmd->cSupS1);
} /* chroma_suppress40_cmd_debug */

/** chroma_suppress40_ez_isp_update
 *
 *  @mod: chroma_suppress module handle
 *  @chroma_suppressDiag: chroma suppressionDiag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void chroma_suppress40_ez_isp_update(chroma_suppress40_t *mod,
  chromasuppression_t *chromasupp)
{
  chromasupp->ysup1  = mod->reg_cmd.ySup1;
  chromasupp->ysup2  = mod->reg_cmd.ySup2;
  chromasupp->ysup3  = mod->reg_cmd.ySup3;
  chromasupp->ysup4  = mod->reg_cmd.ySup4;
  chromasupp->ysupM1 = mod->reg_mix_cmd_1.reg_cmd.ySupM1;
  chromasupp->ysupM3 = mod->reg_mix_cmd_1.reg_cmd.ySupM3;
  chromasupp->ysupS1 = mod->reg_mix_cmd_1.reg_cmd.ySupS1;
  chromasupp->ysupS3 = mod->reg_mix_cmd_1.reg_cmd.ySupS3;
  chromasupp->csup1  = mod->reg_mix_cmd_2.reg_cmd.cSup1;
  chromasupp->csup2  = mod->reg_mix_cmd_2.reg_cmd.cSup2;
  chromasupp->csupM1 = mod->reg_mix_cmd_2.reg_cmd.cSupM1;
  chromasupp->csupS1 = mod->reg_mix_cmd_2.reg_cmd.cSupS1;

}/* chroma_suppress40_ez_isp_update */

/** chroma_suppress40_fill_vfe_diag_data:
 *
 *  @mod: chroma suppression module instance
 *  @isp_sub_module: base module
 *  @sub_module_output: output ptr to update vfe diag
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean chroma_suppress40_fill_vfe_diag_data(chroma_suppress40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  chromasuppression_t *chroma_suppressDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    /* We can overwrite this since this is called only when vfe_diag is  enabled */
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    chroma_suppressDiag = &(vfe_diag->prev_chromasupp);

    chroma_suppress40_ez_isp_update(mod, chroma_suppressDiag);
  }
  return ret;
}


/** chroma_suppress40_cmd_config:
 *
 *  @chroma_suppress: chroma_suppress module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void chroma_suppress40_cmd_config(chroma_suppress40_t *chroma_suppress)
{
  int Q_slope;
  int Diff;

  if (!chroma_suppress) {
    ISP_ERR("failed: chroma_suppress %p", chroma_suppress);
    return;
  }
  /* copy from local thres to cmd */
  chroma_suppress->reg_cmd.ySup1 =
    Round(chroma_suppress->thresholds.cs_luma_threshold1);
  chroma_suppress->reg_cmd.ySup2 =
    Round(chroma_suppress->thresholds.cs_luma_threshold2);
  chroma_suppress->reg_cmd.ySup3 =
    Round(chroma_suppress->thresholds.cs_luma_threshold3);
  chroma_suppress->reg_cmd.ySup4 =
    Round(chroma_suppress->thresholds.cs_luma_threshold4);
  chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup1 =
    Round(chroma_suppress->thresholds.cs_chroma_threshold1);
  chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup2 =
    Round(chroma_suppress->thresholds.cs_chroma_threshold2);

  Diff = chroma_suppress->reg_cmd.ySup2 -
           chroma_suppress->reg_cmd.ySup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  chroma_suppress->reg_cmd.ySup2 = chroma_suppress->reg_cmd.ySup1 + Diff;
  Q_slope = (int) ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_suppress->reg_mix_cmd_1.reg_cmd.ySupM1 = (1 << Q_slope) / Diff;

  chroma_suppress->reg_mix_cmd_1.reg_cmd.ySupS1 = Q_slope - 7;

  Diff = chroma_suppress->reg_cmd.ySup4 -
           chroma_suppress->reg_cmd.ySup3;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  chroma_suppress->reg_cmd.ySup3 = chroma_suppress->reg_cmd.ySup4 - Diff;
  Q_slope = (int) ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_suppress->reg_mix_cmd_1.reg_cmd.ySupM3 = (1 << Q_slope) / Diff;
  chroma_suppress->reg_mix_cmd_1.reg_cmd.ySupS3 = Q_slope - 7;
  chroma_suppress->reg_mix_cmd_1.hw_mask = 0x777f7f;

  Diff = chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup2 -
           chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup1;
  Diff = Clamp(Diff, CHROMA_SUPPRESS_DIFF_MIN, CHROMA_SUPPRESS_DIFF_MAX);
  chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup2 =
    chroma_suppress->reg_mix_cmd_2.reg_cmd.cSup1 + Diff;


  Q_slope = (int)ceil(log((double)Diff) / log(2.0)) + 6;
  chroma_suppress->reg_mix_cmd_2.reg_cmd.cSupM1 = (1 << Q_slope) / Diff;
  chroma_suppress->reg_mix_cmd_2.reg_cmd.cSupS1 = Q_slope - 7;
  chroma_suppress->reg_mix_cmd_2.hw_mask = 0x77fffff;
} /* chroma_suppress40_cmd_config */

/** chroma_suppress40_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @chroma_suppress: chroma_suppress struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean chroma_suppress40_store_hw_update(
  isp_sub_module_t *isp_sub_module, chroma_suppress40_t *chroma_suppress)
{
  boolean                           ret = TRUE;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  ISP_ChromaSuppress_ConfigCmdType *reg_cmd = NULL;

  if (!isp_sub_module || !chroma_suppress) {
    ISP_ERR("failed: %p %p", isp_sub_module, chroma_suppress);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(3 * sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  reg_cmd = (ISP_ChromaSuppress_ConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cmd %p", reg_cmd);
    goto ERROR_REG_CMD;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));
  *reg_cmd = chroma_suppress->reg_cmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 3;

  /*write value into mixed register 1*/
  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_CHROMA40_SUP_MIX_OFF_1;
  reg_cfg_cmd[0].u.mask_info.mask = chroma_suppress->reg_mix_cmd_1.hw_mask;
  reg_cfg_cmd[0].u.mask_info.val = chroma_suppress->reg_mix_cmd_1.reg_cmd.cfg;

  /*write value into mixed register 2*/
  reg_cfg_cmd[1].cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd[1].u.mask_info.reg_offset = ISP_CHROMA40_SUP_MIX_OFF_2;
  reg_cfg_cmd[1].u.mask_info.mask = chroma_suppress->reg_mix_cmd_2.hw_mask;
  reg_cfg_cmd[1].u.mask_info.val = chroma_suppress->reg_mix_cmd_2.reg_cmd.cfg;

  /*write regular chroma SS cmd*/
  reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_CHROMA40_SUP_OFF;
  reg_cfg_cmd[2].u.rw_info.len = ISP_CHROMA40_SUP_LEN * sizeof(uint32_t);

  chroma_suppress40_cmd_debug(&(chroma_suppress->reg_cmd),
    &(chroma_suppress->reg_mix_cmd_1.reg_cmd),
    &(chroma_suppress->reg_mix_cmd_2.reg_cmd));

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* chroma_suppress40_do_hw_update */

/** chroma_suppress40_init_default
 *
 *  @isp_sub_module: isp sub module handle
 *  @chroma_suppress: chroma_suppress handle
 *
 *  cs module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean chroma_suppress40_init_default(isp_sub_module_t *isp_sub_module,
  chroma_suppress40_t *chroma_suppress)
{
  chromatix_parms_type   *chromatix_ptr = NULL;
  chromatix_CS_MCE_type  *chromatix_CS_MCE = NULL;
  cs_luma_threshold_type *p_thresholds = NULL;

  if (!isp_sub_module || !chroma_suppress) {
    ISP_ERR("failed: %p %p", isp_sub_module, chroma_suppress);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;

  p_thresholds = &chromatix_CS_MCE->cs_normal;

  /* default values */
  chroma_suppress->thresholds.cs_luma_threshold1 =
    (float)p_thresholds->cs_luma_threshold1;
  chroma_suppress->thresholds.cs_luma_threshold2 =
    (float)p_thresholds->cs_luma_threshold2;
  chroma_suppress->thresholds.cs_luma_threshold3 =
    (float)p_thresholds->cs_luma_threshold3;
  chroma_suppress->thresholds.cs_luma_threshold4 =
    (float)p_thresholds->cs_luma_threshold4;
  chroma_suppress->thresholds.cs_chroma_threshold1 =
    (float)p_thresholds->cs_chroma_threshold1;
  chroma_suppress->thresholds.cs_chroma_threshold2 =
    (float)p_thresholds->cs_chroma_threshold2;

  chroma_suppress40_cmd_config(chroma_suppress);

  return TRUE;
} /* chroma_suppress40_config */

/** chroma_suppress40_interpolate
 *
 *  @isp_sub_module: isp sub module handle
 *  @chroma_suppress: chroma_suppress handle
 *
 *  cs module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean chroma_suppress40_interpolate(isp_sub_module_t *isp_sub_module,
  chroma_suppress40_t *chroma_suppress)
{
  boolean                 ret = TRUE;
  chromatix_parms_type   *chromatix_ptr = NULL;
  chromatix_CS_MCE_type  *chromatix_CS_MCE = NULL;
  float                   ratio = 0;
  cs_luma_threshold_type *cs_luma_threshold = NULL,
                         *cs_luma_threshold_lowlight = NULL;

  if (!isp_sub_module || !chroma_suppress) {
    ISP_ERR("failed: %p %p", isp_sub_module, chroma_suppress);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    return TRUE;
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

  chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;

  cs_luma_threshold = &(chromatix_CS_MCE->cs_normal);
  cs_luma_threshold_lowlight = &(chromatix_CS_MCE->cs_lowlight);
  ratio = chroma_suppress->aec_ratio.ratio;

  if (F_EQUAL(ratio, 0.0)) {
    ISP_DBG("Low Light");
    chroma_suppress->thresholds.cs_luma_threshold1 =
      (float)cs_luma_threshold_lowlight->cs_luma_threshold1;
    chroma_suppress->thresholds.cs_luma_threshold2 =
      (float)cs_luma_threshold_lowlight->cs_luma_threshold2;
    chroma_suppress->thresholds.cs_luma_threshold3 =
      (float)cs_luma_threshold_lowlight->cs_luma_threshold3;
    chroma_suppress->thresholds.cs_luma_threshold4 =
      (float)cs_luma_threshold_lowlight->cs_luma_threshold4;
    chroma_suppress->thresholds.cs_chroma_threshold1 =
      (float)cs_luma_threshold_lowlight->cs_chroma_threshold1;
    chroma_suppress->thresholds.cs_chroma_threshold2 =
      (float)cs_luma_threshold_lowlight->cs_chroma_threshold2;
  } else if (F_EQUAL(ratio, 1.0)) {
    ISP_DBG("Normal Light");
    chroma_suppress->thresholds.cs_luma_threshold1 =
      (float)cs_luma_threshold->cs_luma_threshold1;
    chroma_suppress->thresholds.cs_luma_threshold2 =
      (float)cs_luma_threshold->cs_luma_threshold2;
    chroma_suppress->thresholds.cs_luma_threshold3 =
      (float)cs_luma_threshold->cs_luma_threshold3;
    chroma_suppress->thresholds.cs_luma_threshold4 =
      (float)cs_luma_threshold->cs_luma_threshold4;
    chroma_suppress->thresholds.cs_chroma_threshold1 =
      (float)cs_luma_threshold->cs_chroma_threshold1;
    chroma_suppress->thresholds.cs_chroma_threshold2 =
      (float)cs_luma_threshold->cs_chroma_threshold2;
  } else {
    ISP_DBG("Interpolate between Normal and Low Light");
    chroma_suppress->thresholds.cs_luma_threshold1 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_luma_threshold1,
      (float)cs_luma_threshold_lowlight->cs_luma_threshold1, ratio);

    chroma_suppress->thresholds.cs_luma_threshold2 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_luma_threshold2,
      (float)cs_luma_threshold_lowlight->cs_luma_threshold2, ratio);

    chroma_suppress->thresholds.cs_luma_threshold3 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_luma_threshold3,
      (float)cs_luma_threshold_lowlight->cs_luma_threshold3, ratio);
    chroma_suppress->thresholds.cs_luma_threshold4 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_luma_threshold4,
      (float)cs_luma_threshold_lowlight->cs_luma_threshold4, ratio);

    chroma_suppress->thresholds.cs_chroma_threshold1 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_chroma_threshold1,
      (float)cs_luma_threshold_lowlight->cs_chroma_threshold1, ratio);

    chroma_suppress->thresholds.cs_chroma_threshold2 = (float)LINEAR_INTERPOLATION(
      (float)cs_luma_threshold->cs_chroma_threshold2,
      (float)cs_luma_threshold_lowlight->cs_chroma_threshold2, ratio);
  }

  chroma_suppress40_cmd_config(chroma_suppress);

  chroma_suppress->aec_ratio.ratio = ratio;

  ret = chroma_suppress40_store_hw_update(isp_sub_module, chroma_suppress);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma_suppress40_store_hw_update");
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;
} /* chroma_suppress40_interpolate */

/** chroma_suppress40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_suppress40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                       ret = TRUE;
  chroma_suppress40_t          *chroma_suppress = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  uint8_t                       module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  chroma_suppress = (chroma_suppress40_t *)isp_sub_module->private_data;
  if (!chroma_suppress) {
    ISP_ERR("failed: chroma_suppress %p", chroma_suppress);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      !isp_sub_module->config_pending &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.tonemap_mode ==
                     CAM_TONEMAP_MODE_CONTRAST_CURVE) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      output->stats_params->
        module_enable_info.submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.submod_mask[isp_sub_module->hw_module_id] = 1;

      if (!isp_sub_module->submod_enable) {
        PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
        return TRUE;
      }
    }
  }
  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if (isp_sub_module->submod_enable == FALSE ||
      isp_sub_module->submod_trigger_enable == FALSE) {
    ISP_DBG("enable = %d, trigger_enable = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  if(isp_sub_module->trigger_update_pending == TRUE &&
     isp_sub_module->submod_trigger_enable == TRUE) {

    ret = chroma_suppress40_interpolate(isp_sub_module, chroma_suppress);
    if (ret == FALSE) {
      ISP_ERR("failed: chroma_suppress40_interpolate");
    }
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret =chroma_suppress40_fill_vfe_diag_data(chroma_suppress,
      isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: chroma_suppress40_fill_vfe_diag_data");
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** chroma_suppress40_stats_aec_update:
 *
 * @mod: cs module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_suppress40_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t        *stats_update = NULL;
  chroma_suppress40_t   *chroma_suppress = NULL;
  float                  aec_ratio = 0;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_CS_MCE_type *chromatix_CS_MCE = NULL;
  trigger_point_type    *p_trigger_point = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chroma_suppress = (chroma_suppress40_t *)isp_sub_module->private_data;
  if (!chroma_suppress) {
    ISP_ERR("failed: chroma_suppress %p", chroma_suppress);
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
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_CS_MCE =
    &chromatix_ptr->chromatix_VFE.chromatix_CS_MCE;

  p_trigger_point = &chromatix_CS_MCE->cs_lowlight_trigger;

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_CS_MCE->control_cs, p_trigger_point, &stats_update->aec_update, 0);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
       !F_EQUAL(chroma_suppress->aec_ratio.ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  chroma_suppress->aec_ratio.ratio = aec_ratio;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** chroma_suppress40_streamon:
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
boolean chroma_suppress40_streamon(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  chroma_suppress40_t *chroma_suppress = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chroma_suppress = (chroma_suppress40_t *)isp_sub_module->private_data;
  if (!chroma_suppress) {
    ISP_ERR("failed: mod %p", chroma_suppress);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = chroma_suppress40_init_default(isp_sub_module, chroma_suppress);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma_suppress40_config ret %d", ret);
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* chroma_suppress40_streamon */

/** chroma_suppress40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean chroma_suppress40_streamoff(isp_sub_module_t *isp_sub_module,
  void *data)
{
  chroma_suppress40_t *chroma_suppress = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chroma_suppress = (chroma_suppress40_t *)isp_sub_module->private_data;
  if (!chroma_suppress) {
    ISP_ERR("failed: chroma_suppress %p", chroma_suppress);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(chroma_suppress, 0, sizeof(*chroma_suppress));
  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* chroma_suppress40_streamoff */

/** chroma_suppress40_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean chroma_suppress40_init(isp_sub_module_t *isp_sub_module)
{
  chroma_suppress40_t *chroma_suppress = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  chroma_suppress = (chroma_suppress40_t *)CAM_MALLOC(sizeof(chroma_suppress40_t));
  if (!chroma_suppress) {
    ISP_ERR("failed: chroma_suppress %p", chroma_suppress);
    return FALSE;
  }

  memset(chroma_suppress, 0, sizeof(*chroma_suppress));

  isp_sub_module->private_data = (void *)chroma_suppress;
  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;

  return TRUE;
}/* chroma_suppress40_init */

/** chroma_suppress40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void chroma_suppress40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* chroma_suppress40_destroy */
