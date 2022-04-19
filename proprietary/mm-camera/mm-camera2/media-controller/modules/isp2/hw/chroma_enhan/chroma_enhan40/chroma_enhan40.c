/* chroma_enhan40.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_chroma_enhan40.h"
#include "chroma_enhan40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "chroma_enhan_algo.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)

#define SET_SAT_MATRIX(IN, S) ({\
  IN[0][0] = IN[1][1] = S; \
  IN[0][1] = IN[1][0] = 0; \
  })

#define SET_HUE_MATRIX(IN, H) ({\
  IN[0][0] = cos(H); \
  IN[0][1] = -sin(H); \
  IN[1][0] = sin(H); \
  IN[1][1] = cos(H); \
  })

static boolean chroma_enhan40_fill_vfe_diag_data(chroma_enhan40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output);

/** chroma_enhan40_debug:
 *
 *  @cmd: configuration command
 *  @update: is it update flag
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps chroma enhance configuration
 *
 *  Return: None
 **/
static void chroma_enhan40_debug(void *cmd)
{
  ISP_DBG("VFE_Chroma_enhance40/ color conversion update");

  ISP_Chroma_Enhance_CfgCmdType* pcmd = (ISP_Chroma_Enhance_CfgCmdType *)cmd;

  ISP_DBG("RGBtoYConversionV0 = %d", pcmd->RGBtoYConversionV0);
  ISP_DBG("RGBtoYConversionV1 = %d", pcmd->RGBtoYConversionV1);
  ISP_DBG("RGBtoYConversionV2 = %d", pcmd->RGBtoYConversionV2);
  ISP_DBG("RGBtoYConversionOffset = %d", pcmd->RGBtoYConversionOffset);
  ISP_DBG("ap = %d", pcmd->ap);
  ISP_DBG("am = %d", pcmd->am);
  ISP_DBG("bp = %d", pcmd->bp);
  ISP_DBG("bm = %d", pcmd->bm);
  ISP_DBG("cp = %d", pcmd->cp);
  ISP_DBG("cm = %d", pcmd->cm);
  ISP_DBG("dp = %d", pcmd->dp);
  ISP_DBG("dm = %d", pcmd->dm);
  ISP_DBG("kcb = %d", pcmd->kcb);
  ISP_DBG("kcr = %d", pcmd->kcr);

}

/** chroma_enhan40_copy_cv_data
 *
 *  @dst_cv_data: Local struct to strore CV
 *  @src_cv_data: Chromaric CC data
 *
 *  This function do a copy to local struct
 *
 *  Return: None
 **/

void chroma_enhan40_copy_cv_data(
   color_conversion_type_t         *dst_cv_data,
   chromatix_color_conversion_type *src_cv_data)
{
  dst_cv_data->chroma.a_m  = src_cv_data->chroma.a_m;
  dst_cv_data->chroma.a_p  = src_cv_data->chroma.a_p;
  dst_cv_data->chroma.b_m  = src_cv_data->chroma.b_m;
  dst_cv_data->chroma.b_p  = src_cv_data->chroma.b_p;
  dst_cv_data->chroma.c_m  = src_cv_data->chroma.c_m;
  dst_cv_data->chroma.c_p  = src_cv_data->chroma.c_p;
  dst_cv_data->chroma.d_m  = src_cv_data->chroma.d_m;
  dst_cv_data->chroma.d_p  = src_cv_data->chroma.d_p;
  dst_cv_data->chroma.k_cb = (float)src_cv_data->chroma.k_cb;
  dst_cv_data->chroma.k_cr = (float)src_cv_data->chroma.k_cr;
  dst_cv_data->luma.v0     = src_cv_data->luma.v0;
  dst_cv_data->luma.v1     = src_cv_data->luma.v1;
  dst_cv_data->luma.v2     = src_cv_data->luma.v2;
  dst_cv_data->luma.k      = (float)src_cv_data->luma.k;

}

/** chroma_enhan40_util_append_cfg:
 *
 *  @hw_update_list: hw update list
 *  @hw_update: hw update cmd to be enqueued
 *
 *  Enqueue hw_update in hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean chroma_enhan40_util_append_cfg(
  struct msm_vfe_cfg_cmd_list **hw_update_list,
  struct msm_vfe_cfg_cmd_list *hw_update)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd_list *list = NULL;

  if (!hw_update) {
    ISP_ERR("failed: %p", hw_update);
    return FALSE;
  }

  list = *hw_update_list;
  if (!list) {
    *hw_update_list = hw_update;
  } else {
    while (list->next) {
      list = list->next;
    }
    list->next = hw_update;
  }
  return ret;
}

/** chroma_enhan40_do_hw_update:
 *
 *  @chroma enhance_mod: chroma enhancement module instance
 *  @hw_update_list: hw update list handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   0 - Success
 *           -1 - configuration error
 **/
static boolean chroma_enhan40_store_hw_update(isp_sub_module_t *isp_sub_module,
  chroma_enhan40_t *chroma_enhan_mod)
{
  boolean ret = TRUE;
  int i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  ISP_Chroma_Enhance_CfgCmdType *reg_cmd = NULL;

  if (!chroma_enhan_mod) {
    ISP_ERR("failed: %p", chroma_enhan_mod);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, sizeof(struct msm_vfe_reg_cfg_cmd));

  reg_cmd = (ISP_Chroma_Enhance_CfgCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = chroma_enhan_mod->RegCmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CC40_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_CC40_LEN * sizeof(uint32_t);

  chroma_enhan40_debug(&chroma_enhan_mod->RegCmd);
  chroma_enhan_mod->applied_RegCmd = chroma_enhan_mod->RegCmd;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma enhance40_util_append_cfg");
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
}


/** chroma_enhan_prepare_hw_config:
 *
 *    @mod:
 *
 * copy from mod->threshold to reg cmd then configure
 *
 **/
static void chroma_enhan_prepare_hw_config(chroma_enhan40_t *mod)
{
  color_conversion_type_t *cc = &(mod->cv_data);
  double am_new, bm_new, cm_new, dm_new;
  double ap_new, bp_new, cp_new, dp_new;

  ISP_DBG("effects_matrix: %f, %f; %f, %f\n",
    mod->effects_matrix[0][0], mod->effects_matrix[0][1],
    mod->effects_matrix[1][0], mod->effects_matrix[1][1]);

  /*config 1st set of matrix for HW to select, am, bm, cm, dm*/
  am_new = cc->chroma.a_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[0][1];
  bm_new = cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[0][0] +
    cc->chroma.c_m * mod->effects_matrix[0][1];
  cm_new = cc->chroma.c_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * cc->chroma.b_m * mod->effects_matrix[1][0];
  dm_new = cc->chroma.c_m * cc->chroma.d_m * mod->effects_matrix[1][1] +
    cc->chroma.a_m * mod->effects_matrix[1][0];

  if (am_new)
    bm_new /= am_new;

  if (cm_new)
    dm_new /= cm_new;

  if (fabs(am_new) >= 4)
    ISP_ERR("error overflow a_m_new = %f\n", am_new);
  if (fabs(bm_new) >= 4)
    ISP_ERR("error overflow b_m_new = %f\n", bm_new);
  if (fabs(cm_new) >= 4)
    ISP_ERR("error overflow c_m_new = %f\n", cm_new);
  if (fabs(dm_new) >= 4)
    ISP_ERR("error overflow d_m_new = %f\n", dm_new);

  mod->RegCmd.am = FLOAT_TO_Q(8, am_new);
  mod->RegCmd.bm = FLOAT_TO_Q(8, bm_new);
  mod->RegCmd.cm = FLOAT_TO_Q(8, cm_new);
  mod->RegCmd.dm = FLOAT_TO_Q(8, dm_new);

  /*config 2nd set of matrix for HW to select, ap, bp, cp, dp*/
  ap_new = cc->chroma.a_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[0][1];
  bp_new = cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[0][0] +
    cc->chroma.c_p * mod->effects_matrix[0][1];
  cp_new = cc->chroma.c_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * cc->chroma.b_p * mod->effects_matrix[1][0];
  dp_new = cc->chroma.c_p * cc->chroma.d_p * mod->effects_matrix[1][1] +
    cc->chroma.a_p * mod->effects_matrix[1][0];

  if (ap_new)
    bp_new /= ap_new;

  if (cp_new)
    dp_new /= cp_new;

  if (fabs(ap_new) >= 4)
    ISP_ERR("error overflow a_p_new = %f\n", ap_new);
  if (fabs(bp_new) >= 4)
    ISP_ERR("error overflow b_p_new = %f\n", bp_new);
  if (fabs(cp_new) >= 4)
    ISP_ERR("error overflow c_p_new = %f\n", cp_new);
  if (fabs(dp_new) >= 4)
    ISP_ERR("error overflow d_p_new = %f\n", dp_new);

  mod->RegCmd.ap = FLOAT_TO_Q(8, ap_new);
  mod->RegCmd.bp = FLOAT_TO_Q(8, bp_new);
  mod->RegCmd.cp = FLOAT_TO_Q(8, cp_new);
  mod->RegCmd.dp = FLOAT_TO_Q(8, dp_new);

  /*constant offset for matrix conversion: Cb, Cr*/
  mod->RegCmd.kcb = Round(cc->chroma.k_cb);
  mod->RegCmd.kcr = Round(cc->chroma.k_cr);

  /*constant offset for matrix conversion: Y*/
  mod->RegCmd.RGBtoYConversionOffset =
    Round(cc->luma.k);

  /*Coeff for R, G, B for calculating Y*/
  mod->RegCmd.RGBtoYConversionV0 =
    FLOAT_TO_Q(8, cc->luma.v0);

  mod->RegCmd.RGBtoYConversionV1 =
    FLOAT_TO_Q(8, cc->luma.v1);

  mod->RegCmd.RGBtoYConversionV2 =
    FLOAT_TO_Q(8, cc->luma.v2);
}

/** chroma_enhan40_trigger_update:
 *
 *  @mod: chroma enhance module instance
 *  @hw_update_list: hw update list handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean chroma_enhan40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  isp_private_event_t * private_event = NULL;
  ISP_Chroma_Enhance_CfgCmdType *p_cmd = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CV_type       *chromatix_CV_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  chroma_enhan40_t         *chroma_enhan_mod = NULL;
  aec_update_t *aec_params = NULL;
  trigger_ratio_t trigger_ratio;
  isp_sub_module_output_t  *output = NULL;
  Chromatix_ADRC_ACE_adj_type *adrc_adj_data;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /* get chroma enhance mod private*/
  chroma_enhan_mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!chroma_enhan_mod) {
    ISP_ERR("failed: mod %p", chroma_enhan_mod);
    return FALSE;
  }

  /* get chromatix pointer*/
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }
  chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;
  chromatix_gamma =
    &chromatix_ptr->chromatix_VFE.chromatix_gamma;

  /*get chroa enhance hw update list*/
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  if (output->frame_meta) {
    output->frame_meta->bestshot_mode = chroma_enhan_mod->bestshot_mode;
    output->frame_meta->saturation= chroma_enhan_mod->saturation;
       if (chroma_enhan_mod->set_effect == TRUE)
         output->frame_meta->special_effect = chroma_enhan_mod->effect_mode;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (!isp_sub_module->submod_enable) {
      ISP_HIGH("skip trigger update: enb = %d",
        isp_sub_module->submod_enable);
      ret = TRUE;
      goto FILL_METADATA;
  }

  if (isp_sub_module->trigger_update_pending == TRUE &&
      isp_sub_module->submod_trigger_enable == TRUE) {
     /* do awb trigger update*/
    chroma_enhan40_trigger_update_awb(chroma_enhan_mod, chromatix_ptr);
     /* do aec trigger update*/
    chroma_enhan40_trigger_update_aec(chroma_enhan_mod, chromatix_ptr);
    if (chroma_enhan_mod->cur_flash_mode != CAM_FLASH_MODE_OFF) {
      /* do flash trigger update */
      chroma_enhan40_trigger_update_flash(chroma_enhan_mod, chromatix_ptr);
    }

    /* Adjust the CV based on ADRC. */
    adrc_adj_data = &chromatix_ptr->chromatix_VFE.chromatix_adrc_ace_adj_data;
    if (chroma_enhan_mod->enable_adrc &&
        adrc_adj_data->adrc_ace_adj_enable) {
      chroma_enhan_algo_adjust_cv_by_adrc(adrc_adj_data,&chroma_enhan_mod->cv_data.chroma,
        chroma_enhan_mod->stats_update.aec_update);
    }

    chroma_enhan_prepare_hw_config(chroma_enhan_mod);
    ret = chroma_enhan40_store_hw_update(isp_sub_module, chroma_enhan_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: chroma enhance40_do_hw_update");
    }
    /* Change trigger_update_pending to FALSE */
    isp_sub_module->trigger_update_pending = FALSE;

  } else if (isp_sub_module->trigger_update_pending == TRUE &&
      isp_sub_module->submod_trigger_enable == FALSE) {
    /* submod_trigger_enable is FALSE -> DO NOT perform interpolation.
     * Use cv_data that is already selected and prepare hw update list
     */
    chroma_enhan_prepare_hw_config(chroma_enhan_mod);
    ret = chroma_enhan40_store_hw_update(isp_sub_module, chroma_enhan_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: chroma enhance40_do_hw_update");
    }
    /* Change trigger_update_pending to FALSE */
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = chroma_enhan40_fill_vfe_diag_data(chroma_enhan_mod, isp_sub_module,
      output);
    if (ret == FALSE) {
      ISP_ERR("failed: demosaic44_fill_vfe_diag_data");
    }
  }

  if (output &&
      output->meta_dump_params &&
      output->metadata_dump_enable == 1) {
    output->meta_dump_params->frame_meta.adrc_info.color_drc_gain =
      chroma_enhan_mod->stats_update.aec_update.color_drc_gain;
    output->meta_dump_params->frame_meta.adrc_info.exposure_time_ratio =
      chroma_enhan_mod->stats_update.aec_update.hdr_exp_time_ratio;
    output->meta_dump_params->frame_meta.adrc_info.exposure_ratio =
      chroma_enhan_mod->stats_update.aec_update.hdr_sensitivity_ratio;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** chroma_enhan40_update_streaming_mode_mask:
 *
 *
 *  Return
 **/
void chroma_enhan40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask)
{
  chroma_enhan40_t *chroma_enhan = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  chroma_enhan = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!chroma_enhan) {
    ISP_ERR("failed: chroma enhance %p", chroma_enhan);
    return;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  chroma_enhan->streaming_mode_mask = streaming_mode_mask;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
}

/** chroma_enhan40_streamon:
 *
 *  @mod: chroma enhance module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial configuration of chroma enhance module
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean chroma_enhan40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  chroma_enhan40_t *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CV_type       *chromatix_CV_ptr = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &mod->streaming_mode_mask);

  chroma_enhan_prepare_hw_config(mod);
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** chroma_enhan40_streamoff:
 *
 *  @mod: chroma enahnce module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets chroma enahnce module
 *
 *  Return: None
 **/
boolean chroma_enhan40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  chroma_enhan40_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &mod->streaming_mode_mask);

  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  memset(&mod->aec_ratio, 0, sizeof(mod->aec_ratio));
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** chroma_enhan_apply_bestshot:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @bestshot_mode: bestshot mode
 *
 *  Set BestShot mode
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan_apply_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_scene_mode_type bestshot_mode)
{
  boolean ret = TRUE;
  float s = 0.0;
  ISP_Chroma_Enhance_CfgCmdType *reg_cmd = NULL;
  chroma_enhan40_t              *mod = NULL;
  chromatix_parms_type          *chromatix_ptr = NULL;
  chromatix_CV_type             *chromatix_CV_ptr = NULL;

  if (!module || !isp_sub_module || (bestshot_mode >= CAM_SCENE_MODE_MAX)) {
    ISP_ERR("failed: %p %p %d", module, isp_sub_module, bestshot_mode);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;
  ASD_VFE_struct_type     *ASD_algo_ptr = NULL;
  ASD_algo_ptr =        &chromatix_ptr->ASD_algo_data;

  if (!chromatix_CV_ptr || !ASD_algo_ptr) {
    ISP_ERR("failed: %p %p", chromatix_CV_ptr, ASD_algo_ptr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /*disable trigger when bestshot mode*/
  isp_sub_module->submod_trigger_enable = FALSE;

  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;

  mod->set_bestshot = TRUE;

  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  ISP_DBG("bestshot mode %d", bestshot_mode);
  switch (bestshot_mode) {
  case CAM_SCENE_MODE_THEATRE:
  case CAM_SCENE_MODE_CANDLELIGHT:
  case CAM_SCENE_MODE_SUNSET: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &ASD_algo_ptr->sunset_color_conversion);
  }
    break;

  case CAM_SCENE_MODE_FIREWORKS: {
    /* from WB need to check */
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->daylight_color_conversion);
  }
    break;

  case CAM_SCENE_MODE_FLOWERS:
  case CAM_SCENE_MODE_PARTY: {
    /* apply required saturation */
    s = chromatix_CV_ptr->saturated_color_conversion_factor;
    /* Just update color conversion matrix in this case */
    SET_UNITY_MATRIX(mod->effects_matrix, 2);
    SET_SAT_MATRIX(mod->effects_matrix, s);
    mod->set_bestshot = FALSE;

    if (mod->set_effect == FALSE) {
      isp_sub_module->submod_trigger_enable = TRUE;
    }
  }
    break;

  default: {
    mod->set_bestshot = FALSE;
    if (mod->set_effect == FALSE) {
      isp_sub_module->submod_trigger_enable = TRUE;
    }
  }
    break;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** chroma_enhan_set_bestshot:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: bestshot event to be handled
 *
 *  Set BestShot mode
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  float s = 0.0;
  ISP_Chroma_Enhance_CfgCmdType *reg_cmd = NULL;
  chroma_enhan40_t              *mod = NULL;
  chromatix_parms_type          *chromatix_ptr = NULL;
  chromatix_CV_type             *chromatix_CV_ptr = NULL;
  cam_scene_mode_type            bestshot_mode = CAM_SCENE_MODE_OFF;
  mct_event_control_parm_t      *param = NULL;
  ASD_VFE_struct_type           *ASD_algo_ptr = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  /*get best shot mode from mct event data*/
  param = event->u.ctrl_event.control_event_data;
  bestshot_mode = *((cam_scene_mode_type *)param->parm_data);
  if (bestshot_mode >= CAM_SCENE_MODE_MAX) {
    ISP_ERR("failed: bestshot_mode %d", bestshot_mode);
    return FALSE;
  }
  mod->bestshot_mode = bestshot_mode;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    /* Will be handled after set_chromatix_ptr is received */
    mod->set_bestshot = TRUE;
    return TRUE;
  }

  ret = chroma_enhan_apply_bestshot(module, isp_sub_module, mod->bestshot_mode);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma_enhan_apply_bestshot %d", mod->bestshot_mode);
  }

  return ret;
}

/** chroma_enhan_set_effect:
 *
 *    @mod:
 *    @in_params:
 *    @in-param_size:
 *
 * Set effect
 *
 **/
boolean chroma_enhan_set_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  int i = 0, j = 0;
  boolean                  ret = TRUE;
  float                    hue_matrix[2][2];
  float                    sat_matrix[2][2];
  float                    s, hue_in_radian;
  float                    float_saturation = 0.0;
  int32_t                  *hal_stauration = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *ctrl_parm =NULL;
  chroma_enhan40_t         *mod = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  ctrl_event = &event->u.ctrl_event;

  if (!mod || !ctrl_event) {
    ISP_ERR("failed: mod %p %p", mod, ctrl_event);
    return FALSE;
  }

  ctrl_parm = ctrl_event->control_event_data;

  if(ctrl_parm->type == CAM_INTF_PARM_EFFECT) {
    ret = chroma_enhan_set_spl_effect(module, isp_sub_module, event);
    return ret;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if(ctrl_parm->type == CAM_INTF_PARM_SATURATION) {
    hal_stauration = (int32_t *)ctrl_parm->parm_data;
    float_saturation = (float)*hal_stauration / 10.0f;
    mod->saturation = *hal_stauration;

    ISP_DBG("CAM_INTF_PARM_SATURATION: hal parm %d, satuation = %f",
      *hal_stauration, float_saturation);
    s = 2.0 * float_saturation;
    SET_UNITY_MATRIX(mod->effects_matrix, 2);
    SET_SAT_MATRIX(sat_matrix, s);
    /* Enable trigger_update_pending */
    isp_sub_module->trigger_update_pending = TRUE;
  }

#ifdef ENABLE_CV_LOGGING
  PRINT_2D_MATRIX(2, 2, mod->effects_matrix);
#endif

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

END:
  return ret;
}

/** chroma_enhan_set_manual_wb:
 *
 *    @mod:
 *    @in_params:
 *    @in-param_size:
 *
 *
 **/
boolean chroma_enhan_set_manual_wb(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  chromatix_parms_type        *chromatix_ptr = NULL;
  chromatix_CV_type           *chromatix_CV_ptr = NULL;
  cam_wb_mode_type            *wb_mode = NULL;
  chroma_enhan40_t            *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  wb_mode = (cam_wb_mode_type *)event->u.ctrl_event.control_event_data;
  mod->wb_mode = *wb_mode;
  ISP_DBG("wb_mode = %d", mod->wb_mode);

  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;

  switch(mod->wb_mode) {
  case CAM_WB_MODE_INCANDESCENT: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->A_color_conversion);
  }
    break;

  case CAM_WB_MODE_CLOUDY_DAYLIGHT:
  case CAM_WB_MODE_DAYLIGHT: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->daylight_color_conversion);
  }
    break;

  default:
    break;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* color_conversion_set_manual_wb */

/** chroma_enhan_apply_spl_effect:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @effects: effect to be applied
 *
 * Set special effect
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan_apply_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_effect_mode_type effects)
{
  boolean ret = TRUE;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_CV_type        *chromatix_CV_ptr = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  chroma_enhan40_t         *mod = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !isp_sub_module || (effects >= CAM_EFFECT_MODE_MAX)) {
    ISP_ERR("failed: %p %p %d", module, isp_sub_module, effects);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }

  chromatix_CV_ptr =
      &chromatix_ptr->chromatix_VFE.chromatix_CV;

  ISP_HIGH(" apply  speciall effect %d", effects);

  /* reset hue/saturation */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);

  /* disable trigger enable when apply special effect*/
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* disable trigger when bestshot mode */
  isp_sub_module->submod_trigger_enable = FALSE;

  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;

  mod->set_effect = TRUE;

  switch(effects) {
  case CAM_EFFECT_MODE_EMBOSS:
  case CAM_EFFECT_MODE_SKETCH:
  case CAM_EFFECT_MODE_MONO: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->mono_color_conversion);
  }
    break;

  case CAM_EFFECT_MODE_NEGATIVE: {
   chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->negative_color_conversion);
  }
    break;

  case CAM_EFFECT_MODE_SEPIA: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->sepia_color_conversion);
  }
    break;

  case CAM_EFFECT_MODE_AQUA: {
    chroma_enhan40_copy_cv_data(&mod->cv_data,
      &chromatix_CV_ptr->aqua_color_conversion);
  }
    break;

  default: {
    if (mod->set_bestshot == FALSE) {
      isp_sub_module->submod_trigger_enable = TRUE;
    }
    mod->set_effect = FALSE;
  }
    break;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** chroma_enhan_set_spl_effect:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: effect event to be applied
 *
 * Set special effect
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_CV_type        *chromatix_CV_ptr = NULL;
  cam_effect_mode_type      effects = CAM_EFFECT_MODE_OFF;
  mct_event_control_t      *ctrl_event = NULL;
  chroma_enhan40_t         *mod = NULL;
  mct_event_control_parm_t *param = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  ctrl_event = &event->u.ctrl_event;
  param = ctrl_event->control_event_data;
  effects = *(int32_t *)param->parm_data;

  if (effects >= CAM_EFFECT_MODE_MAX) {
    ISP_ERR("failed: effects %d", effects);
    return FALSE;
  }
  ISP_HIGH(" apply  speciall effect %d", effects);
  mod->effect_mode = effects;

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    /* Will be handled after set_chromatix_ptr is received */
    mod->set_effect = TRUE;
    return TRUE;
  }

  ret = chroma_enhan_apply_spl_effect(module, isp_sub_module, mod->effect_mode);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma_enhan_apply_spl_effect %d", mod->effect_mode);
  }

  return ret;
}

/** chroma_enhan_interpolate:
 *
 *    @in1:
 *    @in2:
 *    @out:
 *    @ratio:
 *
 **/
void chroma_enhan_interpolate(
  color_conversion_type_t* in1, color_conversion_type_t *in2,
  color_conversion_type_t* out, float ratio)
{
  if (!in1 || !in2 || !out) {
    ISP_ERR("%s:%d failed %p %p %p\n", __func__, __LINE__, in1, in2, out);
    return;
  }

  out->chroma.a_p = LINEAR_INTERPOLATION(in1->chroma.a_p,
    in2->chroma.a_p, ratio);
  out->chroma.a_m = LINEAR_INTERPOLATION(in1->chroma.a_m,
    in2->chroma.a_m, ratio);
  out->chroma.b_p = LINEAR_INTERPOLATION(in1->chroma.b_p,
    in2->chroma.b_p, ratio);
  out->chroma.b_m = LINEAR_INTERPOLATION(in1->chroma.b_m,
    in2->chroma.b_m, ratio);
  out->chroma.c_p = LINEAR_INTERPOLATION(in1->chroma.c_p,
    in2->chroma.c_p, ratio);
  out->chroma.c_m = LINEAR_INTERPOLATION(in1->chroma.c_m,
    in2->chroma.c_m, ratio);
  out->chroma.d_p = LINEAR_INTERPOLATION(in1->chroma.d_p,
    in2->chroma.d_p, ratio);
  out->chroma.d_m = LINEAR_INTERPOLATION(in1->chroma.d_m,
    in2->chroma.d_m, ratio);
  out->chroma.k_cb = LINEAR_INTERPOLATION(in1->chroma.k_cb,
    in2->chroma.k_cb, ratio);
  out->chroma.k_cr = LINEAR_INTERPOLATION(in1->chroma.k_cr,
    in2->chroma.k_cr, ratio);
  out->luma.k = LINEAR_INTERPOLATION(in1->luma.k, in2->luma.k, ratio);
  out->luma.v0 = LINEAR_INTERPOLATION(in1->luma.v0, in2->luma.v0, ratio);
  out->luma.v1 = LINEAR_INTERPOLATION(in1->luma.v1, in2->luma.v1, ratio);
  out->luma.v2 = LINEAR_INTERPOLATION(in1->luma.v2, in2->luma.v2, ratio);
} /* util_color_conversion_interpolate */

/** chroma_enhan40_save_aec_param:
 *
 * @mod: chroma enhance module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  stats_update_t          *stats_update = NULL;
  chroma_enhan40_t        *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CV_type       *chromatix_CV_ptr = NULL;
  chromatix_gamma_type    *chromatix_gamma = NULL;
  trigger_ratio_t         trigger_ratio;
  trigger_point_type      outdoor_trigger;
  float                   color_drc_gain = 0.0;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }
  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;
  chromatix_gamma = &chromatix_ptr->chromatix_VFE.chromatix_gamma;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  outdoor_trigger.gain_start = GAMMA_OUTDOOR_TRIGGER(chromatix_gamma).gain_start;
  outdoor_trigger.gain_end = GAMMA_OUTDOOR_TRIGGER(chromatix_gamma).gain_end;
  outdoor_trigger.lux_index_start = GAMMA_OUTDOOR_TRIGGER(chromatix_gamma).lux_index_start;
  outdoor_trigger.lux_index_end = GAMMA_OUTDOOR_TRIGGER(chromatix_gamma).lux_index_end;

  /* Decide the trigger ratio for current lighting condition,
     fill in trigger ratio*/
  ret = isp_sub_module_util_get_aec_ratio_bright_low(
         chromatix_CV_ptr->control_cv,
         &(outdoor_trigger),
         &(chromatix_CV_ptr->cv_trigger), &stats_update->aec_update,
         &trigger_ratio, 0);
  if (ret != TRUE) {
    ISP_DBG(": get aec ratio error ");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  color_drc_gain = stats_update->aec_update.color_drc_gain;
  mod->enable_adrc = isp_sub_module_util_is_adrc_gain_set(color_drc_gain,
                       stats_update->aec_update.total_drc_gain);


  if (!(F_EQUAL(trigger_ratio.ratio, mod->aec_ratio.ratio)
       && (trigger_ratio.lighting == mod->aec_ratio.lighting)) ||
      (mod->enable_adrc &&
       !F_EQUAL(color_drc_gain, mod->prev_color_drc_gain))) {
    isp_sub_module->trigger_update_pending = TRUE;
  } else {
    ISP_DBG("skip aec trigger upadate, "
      "old: ratio = %f, lighting = %d, new ratio = %f, lighting = %d",
      mod->aec_ratio.ratio, mod->aec_ratio.lighting,
      trigger_ratio.ratio, trigger_ratio.lighting);
  }

  /* need trigger update, Store AEC update in module private */
  mod->stats_update.aec_update = stats_update->aec_update;
  mod->aec_ratio = trigger_ratio;
  mod->prev_color_drc_gain = color_drc_gain;

  if (stats_update->aec_update.dual_led_setting.is_valid) {
    mod->dual_led_setting.led1_low_setting  = stats_update->aec_update.dual_led_setting.led1_low_setting;
    mod->dual_led_setting.led2_low_setting  = stats_update->aec_update.dual_led_setting.led2_low_setting;
    mod->dual_led_setting.led1_high_setting = stats_update->aec_update.dual_led_setting.led1_high_setting;
    mod->dual_led_setting.led2_high_setting = stats_update->aec_update.dual_led_setting.led2_high_setting;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** chroma_enhan40_trigger_update_aec:
 *
 * @mod: chroma enhance module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_trigger_update_aec(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr)
{
  chromatix_CV_type       *chromatix_CV_ptr = NULL;

  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;
  color_conversion_type_t cv_table_lowlight, cv_table_outdoor;

  /*input awc interpolation result and then come out the final cv data*/
  switch (mod->aec_ratio.lighting) {
  case TRIGGER_LOWLIGHT: {
    chroma_enhan40_copy_cv_data(&cv_table_lowlight,
      &chromatix_CV_ptr->lowlight_color_conversion);
    chroma_enhan_interpolate(&mod->cv_data,
     &cv_table_lowlight,
     &mod->cv_data, mod->aec_ratio.ratio);
  }
    break;

  case TRIGGER_OUTDOOR: {
    chroma_enhan40_copy_cv_data(&cv_table_outdoor,
      &chromatix_CV_ptr->outdoor_color_conversion);
    chroma_enhan_interpolate(&mod->cv_data,
     &cv_table_outdoor,
     &mod->cv_data, mod->aec_ratio.ratio);
  }
    break;

  default:
  case TRIGGER_NORMAL: {
    ISP_DBG("LIGHTING = TRIGGER_NORMAL, no need to interpolate aec data");
  }
    break;
  }

  return TRUE;
}

/** chroma_enhan40_save_awb_params:
 *
 * @mod: chroma enhance module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  float              ratio = 0.0;
  stats_update_t     *stats_update = NULL;
  chroma_enhan40_t   *mod = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
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

  if (mod->color_temp == stats_update->awb_update.color_temp ||
      stats_update->awb_update.color_temp == 0) {
    ISP_DBG("skip awb trigger update, colort temp old = %d, new = %d",
    mod->color_temp, stats_update->awb_update.color_temp);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* need awb trigger update, save the color temp*/
  mod->color_temp = stats_update->awb_update.color_temp;

  if (stats_update->awb_update.dual_led_setting.is_valid) {
    mod->dual_led_setting.led1_low_setting  = stats_update->awb_update.dual_led_setting.led1_low_setting;
    mod->dual_led_setting.led2_low_setting  = stats_update->awb_update.dual_led_setting.led2_low_setting;
    mod->dual_led_setting.led1_high_setting = stats_update->awb_update.dual_led_setting.led1_high_setting;
    mod->dual_led_setting.led2_high_setting = stats_update->awb_update.dual_led_setting.led2_high_setting;
  }
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** chroma_enhan40_trigger_update_awb:
 *
 * @mod: chroma enhance module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_trigger_update_awb(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr)
{
  float ratio = 0.0;
  color_conversion_type_t cv_table_H, cv_table_D50,
    cv_table_A, cv_table_TL84, cv_table_D65;
  cct_trigger_info   trigger_info;
  awb_cct_type       cct_type;
  chromatix_CV_type  *chromatix_CV_ptr = NULL;
  float color_temp = 0.0f;
  ASD_VFE_struct_type   *ASD_algo_ptr = NULL;

  if (!mod ||  !chromatix_ptr) {
    ISP_ERR("%s:%d failed: %p %p\n", __func__, __LINE__, mod, chromatix_ptr);
    return FALSE;
  }

  chromatix_CV_ptr = &chromatix_ptr->chromatix_VFE.chromatix_CV;
  ASD_algo_ptr = &chromatix_ptr->ASD_algo_data;

  if(mod->wb_mode != CAM_WB_MODE_AUTO) {
     ISP_ERR("skip awb trigger update, in Manual WB mode %d", mod->wb_mode);
     return TRUE;
  }

  color_temp = mod->color_temp;
  /* fill in trigger_info by saved color temp*/
  if (mod->color_temp == 0) {
    ISP_ERR("failed: mod->color_temp = 0, return FALSE!");
    return FALSE;
  }

  memset(&trigger_info, 0, sizeof(trigger_info));
  MIRED(mod->color_temp, trigger_info.mired_color_temp);
  if (chromatix_CV_ptr->CV_Daylight_trigger.CCT_start &&
    chromatix_CV_ptr->CV_Daylight_trigger.CCT_end) {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
      chromatix_CV_ptr->CV_Daylight_trigger);
  }
  if (chromatix_CV_ptr->CV_A_trigger.CCT_start &&
    chromatix_CV_ptr->CV_A_trigger.CCT_end) {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
      chromatix_CV_ptr->CV_A_trigger);
  }

  if (chromatix_CV_ptr->CV_H_trigger.CCT_start &&
    chromatix_CV_ptr->CV_H_trigger.CCT_end) {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_H,
      chromatix_CV_ptr->CV_H_trigger);
  }

  if (chromatix_CV_ptr->CV_D50_trigger.CCT_start &&
    chromatix_CV_ptr->CV_D50_trigger.CCT_end) {
    CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d50,
      chromatix_CV_ptr->CV_D50_trigger);
  }

  /*use differnt color transform matrix for portrait mode*/
  chroma_enhan40_copy_cv_data(&cv_table_H,
    &chromatix_CV_ptr->H_color_conversion);
  chroma_enhan40_copy_cv_data(&cv_table_A,
    &chromatix_CV_ptr->A_color_conversion);
  chroma_enhan40_copy_cv_data(&cv_table_TL84,
    &chromatix_CV_ptr->TL84_color_conversion);
  chroma_enhan40_copy_cv_data(&cv_table_D65,
    &chromatix_CV_ptr->daylight_color_conversion);
  chroma_enhan40_copy_cv_data(&cv_table_D50,
    &chromatix_CV_ptr->D50_color_conversion);


  /* get cct type by color temp*/
  cct_type = isp_sub_module_util_get_awb_ext_h_cct_type(&trigger_info,
    chromatix_ptr);

  switch (cct_type) {
  case AWB_CCT_TYPE_A: {
    mod->cv_data = cv_table_A;
  }
    break;

  case AWB_CCT_TYPE_D50: {
    mod->cv_data = cv_table_D50;
  }
    break;

  case AWB_CCT_TYPE_H: {
    mod->cv_data = cv_table_H;
  }
    break;

  case AWB_CCT_TYPE_A_H: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_H_trigger.CCT_start,
      1.0f / (float)chromatix_CV_ptr->CV_H_trigger.CCT_end);

    chroma_enhan_interpolate(&cv_table_A, &cv_table_H,
      &mod->cv_data, ratio);
  }
    break;

  case AWB_CCT_TYPE_D50_TL84: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_D50_trigger.CCT_end,
      1.0f / (float)chromatix_CV_ptr->CV_D50_trigger.CCT_start);

    chroma_enhan_interpolate(&cv_table_D50, &cv_table_TL84,
      &mod->cv_data, ratio);
  }
    break;

  case AWB_CCT_TYPE_TL84_A: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_A_trigger.CCT_start,
      1.0f / (float)chromatix_CV_ptr->CV_A_trigger.CCT_end);
    chroma_enhan_interpolate(&cv_table_TL84, &cv_table_A, &mod->cv_data,
      ratio);
  }
    break;

  case AWB_CCT_TYPE_D50_D65: {
    ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
      1.0f / (float)chromatix_CV_ptr->CV_Daylight_trigger.CCT_end,
      1.0f / (float)chromatix_CV_ptr->CV_Daylight_trigger.CCT_start);
    chroma_enhan_interpolate(&cv_table_D65, &cv_table_D50, &mod->cv_data,
      ratio);
  }
    break;

  case AWB_CCT_TYPE_D65: {
    mod->cv_data = cv_table_D65;
  }
    break;

  default:
  case AWB_CCT_TYPE_TL84: {
    mod->cv_data = cv_table_TL84;
  }
    break;
  }

  return TRUE;
}

/** chroma_enhan40_set_flash_mode:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to cam_flash_mode_t
 *
 *  Handle set flash mode event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  chroma_enhan40_t   *mod = NULL;
  cam_flash_mode_t   *flash_mode = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  flash_mode = (cam_flash_mode_t *)event->u.module_event.module_event_data;
  if (!flash_mode) {
    ISP_ERR("failed: flash_mode %p", flash_mode);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (mod->cur_flash_mode != *flash_mode)
    isp_sub_module->trigger_update_pending = TRUE;

  ISP_DBG("<flash_dbg> get new flash mode = %d", *flash_mode);
  mod->cur_flash_mode = *flash_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** chroma_enhan40_ez_isp_update
 *
 *  @chroma enhance_module: chroma enhance module handle
 *  @chroma enhanceDiag: chroma enhance Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void chroma_enhan40_ez_isp_update(isp_sub_module_t *isp_sub_module,
  chroma_enhan40_t *chroma_enhan_mod, chromaenhancement_t *chroma_enhan_Diag)
{
  int index;
  if (!isp_sub_module || !chroma_enhan_mod || !chroma_enhan_Diag) {
    ISP_ERR("failed: %p %p %p",
      isp_sub_module, chroma_enhan_mod, chroma_enhan_Diag);
    return;
  }

  chroma_enhan_Diag->param_ap = chroma_enhan_mod->applied_RegCmd.ap;
  chroma_enhan_Diag->param_am = chroma_enhan_mod->applied_RegCmd.am;
  chroma_enhan_Diag->param_bp = chroma_enhan_mod->applied_RegCmd.bp;
  chroma_enhan_Diag->param_bm = chroma_enhan_mod->applied_RegCmd.bm;
  chroma_enhan_Diag->param_cp = chroma_enhan_mod->applied_RegCmd.cp;
  chroma_enhan_Diag->param_cm = chroma_enhan_mod->applied_RegCmd.cm;
  chroma_enhan_Diag->param_dp = chroma_enhan_mod->applied_RegCmd.dp;
  chroma_enhan_Diag->param_dm = chroma_enhan_mod->applied_RegCmd.dm;
  chroma_enhan_Diag->param_kcb = chroma_enhan_mod->applied_RegCmd.kcb;
  chroma_enhan_Diag->param_kcr = chroma_enhan_mod->applied_RegCmd.kcr;
  chroma_enhan_Diag->param_rtoy =
    chroma_enhan_mod->applied_RegCmd.RGBtoYConversionV0;

  /*TODO: confirm below register values and vfe diag params are matching*/
  chroma_enhan_Diag->param_gtoy =
    chroma_enhan_mod->applied_RegCmd.RGBtoYConversionV1;
  chroma_enhan_Diag->param_btoy =
    chroma_enhan_mod->applied_RegCmd.RGBtoYConversionV2;
  chroma_enhan_Diag->param_yoffset =
    chroma_enhan_mod->applied_RegCmd.RGBtoYConversionOffset;
}

/** chroma_enhan40_get_vfe_diag_info_user
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chroma_enhan40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  chroma_enhan40_t    *chroma_enhan_mod = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  chromaenhancement_t *chroma_enhan_Diag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  chroma_enhan_mod = (chroma_enhan40_t *)isp_sub_module->private_data;
  if (!chroma_enhan_mod) {
    ISP_ERR("failed: chroma_enhan_mod %p", chroma_enhan_mod);
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

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chroma_enhan_Diag = &(vfe_diag->colorconv);
  chroma_enhan40_ez_isp_update(isp_sub_module,
    chroma_enhan_mod, chroma_enhan_Diag);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** chroma_enhan40_fill_vfe_diag_data:
 *
 *  @mod: chroma enhan  module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean chroma_enhan40_fill_vfe_diag_data(chroma_enhan40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean                   ret = TRUE;
  chromaenhancement_t      *chroma_enhan_Diag = NULL;
  vfe_diagnostics_t        *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    chroma_enhan_Diag = &(vfe_diag->colorconv);

    chroma_enhan40_ez_isp_update(isp_sub_module,
      mod, chroma_enhan_Diag);
  }
  return ret;
}

/** chroma_enhan40_init:
 *
 *  @mod: demoule module handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function instantiates a chroma enhance module
 *
 *  Return:   NULL - not enough memory
 *            Otherwise handle to module instance
 **/
boolean chroma_enhan40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  chroma_enhan40_t *mod = NULL;

  ISP_HIGH("E");
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (chroma_enhan40_t *)CAM_MALLOC(sizeof(chroma_enhan40_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  memset(&mod->aec_ratio, 0, sizeof(mod->aec_ratio));

  /* enable trigger update feature flag from PIX */
  isp_sub_module->trigger_update_pending  = FALSE;
  isp_sub_module->submod_trigger_enable   = TRUE;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->color_temp = DEFAULT_COLOR_TEMP;
  mod->cur_flash_mode = CAM_FLASH_MODE_OFF;
  /* set old cfg to invalid value to trigger the first trigger update */
  SET_UNITY_MATRIX(mod->effects_matrix, 2);
  isp_sub_module->private_data = (void *)mod;

  return TRUE;
}

/** chroma_enhan40_destroy:
 *
 *  @mod: chroma enhance handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void chroma_enhan40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  chroma_enhan40_t *mod = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
}
