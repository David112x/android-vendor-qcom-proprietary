/* clf46.c
 *
 * Copyright (c)2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */

#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CLF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CLF, fmt, ##args)

/* isp headers */
#include "module_clf46.h"
#include "clf46.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"


/** clf46_debug:
 *
 *  @p_cmd: configuration command
 *
 *  This function dumps cac configuration
 *
 *  Return: None
 **/
static void clf46_debug(ISP_CLF_CmdType* p_cmd)
{
  ISP_DBG("edge_hv_nz_flag = 0x%x",
    *(uint32_t *)(&p_cmd->edge_hv_nz_flag));
  ISP_DBG("edge_diag_nz_flag = 0x%x",
    *(uint32_t *)(&p_cmd->edge_diag_nz_flag));
  ISP_DBG("edge_hv_coeff coeff0 = %d coeff1 = %d coeff2 = %d",
    p_cmd->edge_hv_coeff.coeff0,
    p_cmd->edge_hv_coeff.coeff1,
    p_cmd->edge_hv_coeff.coeff2);
  ISP_DBG("edge_hv_coeff coeff3 = %d coeff4 = %d coeff5 = %d",
    p_cmd->edge_hv_coeff.coeff3,
    p_cmd->edge_hv_coeff.coeff4,
    p_cmd->edge_hv_coeff.coeff5);
  ISP_DBG("edge_hv_coeff coeff6 = %d coeff7 = %d coeff8 = %d",
    p_cmd->edge_hv_coeff.coeff6,
    p_cmd->edge_hv_coeff.coeff7,
    p_cmd->edge_hv_coeff.coeff8);
  ISP_DBG("edge_diag_coeff coeff0 = %d coeff1 = %d coeff2 = %d",
    p_cmd->edge_diag_coeff.coeff0,
    p_cmd->edge_diag_coeff.coeff1,
    p_cmd->edge_diag_coeff.coeff2);
  ISP_DBG("edge_diag_coeff coeff3 = %d coeff4 = %d coeff5 = %d",
    p_cmd->edge_diag_coeff.coeff3,
    p_cmd->edge_diag_coeff.coeff4,
    p_cmd->edge_diag_coeff.coeff5);
  ISP_DBG("edge_diag_coeff coeff6 = %d coeff7 = %d coeff8 = %d",
    p_cmd->edge_diag_coeff.coeff6,
    p_cmd->edge_diag_coeff.coeff7,
    p_cmd->edge_diag_coeff.coeff8);
  ISP_DBG("threshold edge = %d sat = %d",
    p_cmd->threshold.edge, p_cmd->threshold.sat);
  ISP_DBG("threshold rg_hi = %d rg_low = %d",
    p_cmd->threshold.rg_hi, p_cmd->threshold.rg_low);
  ISP_DBG("threshold bg_hi = %d bg_low = %d",
    p_cmd->threshold.bg_hi, p_cmd->threshold.bg_low);
  ISP_DBG("interp_slope rg_inv = %d bg_inv = %d",
    p_cmd->interp_slope.rg_inv, p_cmd->interp_slope.bg_inv);
  ISP_DBG("interp_slope rg_inv_qfactor = %d bg_inv_qfactor = %d",
    p_cmd->interp_slope.rg_inv_qfactor, p_cmd->interp_slope.bg_inv_qfactor);
  ISP_DBG("interp_slope rg_hi = %d rg_low = %d bg_hi = %d bg_low = %d",
    p_cmd->interp_thresh.rg_hi, p_cmd->interp_thresh.rg_low,
    p_cmd->interp_thresh.bg_hi, p_cmd->interp_thresh.bg_low);
} /* clf46_debug */

/** clf46_init_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: clf module private data
 *
 *  This function makes initial configuration of clf module
 *
 *  Return:   TRUE - Success
 **/
boolean clf46_init_config(isp_sub_module_t *isp_sub_module,
  clf46_t *mod)
{
  boolean  ret = TRUE;
  uint32_t q_factor, delta_th, i;

  ISP_DBG(" E");

  /* V3 configuration */
  chromatix_parms_type *chroma_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_chroma_aliasing_correction_type *cac =
    &chroma_ptr->chromatix_post_processing.chromatix_chroma_aliasing_correction;
  chromatix_CAC_reserve *reserve = &cac->reserve;

  for (i = 0; i < 8; i++)
    if (reserve->NZ_HV[i] > 1 || reserve->NZ_HV[i] < -1 ||
        reserve->NZ_D[i] > 1 || reserve->NZ_D[i] < -1) {
      ISP_ERR("invalid NZ_HV or NZ_D");
    }

  mod->reg_cmd.edge_hv_nz_flag.nz_flag_0 = MAP_NZ(reserve->NZ_HV[0]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_1 = MAP_NZ(reserve->NZ_HV[1]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_2 = MAP_NZ(reserve->NZ_HV[2]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_3 = MAP_NZ(reserve->NZ_HV[3]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_4 = MAP_NZ(reserve->NZ_HV[4]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_5 = MAP_NZ(reserve->NZ_HV[5]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_6 = MAP_NZ(reserve->NZ_HV[6]);
  mod->reg_cmd.edge_hv_nz_flag.nz_flag_7 = MAP_NZ(reserve->NZ_HV[7]);

  mod->reg_cmd.edge_diag_nz_flag.nz_flag_0 = MAP_NZ(reserve->NZ_D[0]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_1 = MAP_NZ(reserve->NZ_D[1]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_2 = MAP_NZ(reserve->NZ_D[2]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_3 = MAP_NZ(reserve->NZ_D[3]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_4 = MAP_NZ(reserve->NZ_D[4]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_5 = MAP_NZ(reserve->NZ_D[5]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_6 = MAP_NZ(reserve->NZ_D[6]);
  mod->reg_cmd.edge_diag_nz_flag.nz_flag_7 = MAP_NZ(reserve->NZ_D[7]);

  mod->reg_cmd.edge_hv_coeff.coeff0 = reserve->Fx[0];
  mod->reg_cmd.edge_hv_coeff.coeff1 = reserve->Fx[1];
  mod->reg_cmd.edge_hv_coeff.coeff2 = reserve->Fx[2];
  mod->reg_cmd.edge_hv_coeff.coeff3 = reserve->Fx[3];
  mod->reg_cmd.edge_hv_coeff.coeff4 = reserve->Fx[4];
  mod->reg_cmd.edge_hv_coeff.coeff5 = reserve->Fx[5];
  mod->reg_cmd.edge_hv_coeff.coeff6 = reserve->Fx[6];
  mod->reg_cmd.edge_hv_coeff.coeff7 = reserve->Fx[7];
  mod->reg_cmd.edge_hv_coeff.coeff8 = reserve->Fx[8];

  mod->reg_cmd.edge_diag_coeff = mod->reg_cmd.edge_hv_coeff;

  mod->reg_cmd.threshold.edge = cac->ThEdge;
  mod->reg_cmd.threshold.sat = cac->ThSaturation;
  mod->reg_cmd.threshold.rg_hi  =
    FLOAT_TO_Q(7, cac->PossibleCAPixelRGHighThreshold);
  mod->reg_cmd.threshold.rg_low =
    FLOAT_TO_Q(7, cac->PossibleCAPixelRGLowThreshold);
  mod->reg_cmd.threshold.bg_hi  =
    FLOAT_TO_Q(7, cac->PossibleCAPixelBGHighThreshold);
  mod->reg_cmd.threshold.bg_low =
    FLOAT_TO_Q(7, cac->PossibleCAPixelBGLowThreshold);
  mod->reg_cmd.interp_thresh.rg_hi  =
    FLOAT_TO_Q(7, cac->RGDifferenceHighThreshold);
  mod->reg_cmd.interp_thresh.rg_low =
    FLOAT_TO_Q(7, cac->RGDifferenceLowThreshold);
  mod->reg_cmd.interp_thresh.bg_hi  =
    FLOAT_TO_Q(7, cac->BGDifferenceHighThreshold);
  mod->reg_cmd.interp_thresh.bg_low =
    FLOAT_TO_Q(7, cac->BGDifferenceLowThreshold);


  delta_th = mod->reg_cmd.interp_thresh.rg_hi -
    mod->reg_cmd.interp_thresh.rg_low;
  q_factor = 0;
  while (((1 << q_factor) / delta_th) < 1024)
    q_factor++;

  q_factor--;
  mod->reg_cmd.interp_slope.rg_inv = (1 << q_factor) / delta_th;
  mod->reg_cmd.interp_slope.rg_inv_qfactor = q_factor;

  delta_th = mod->reg_cmd.interp_thresh.bg_hi -
    mod->reg_cmd.interp_thresh.bg_low;
  q_factor = 0;
  while (((1 << q_factor) / delta_th) < 1024)
    q_factor++;

  q_factor--;
  mod->reg_cmd.interp_slope.bg_inv = (1 << q_factor) / delta_th;
  mod->reg_cmd.interp_slope.bg_inv_qfactor = q_factor;

  isp_sub_module->trigger_update_pending = TRUE;

  return ret;
} /* clf46_init_config */

/** clf46_get_vfe_diag_info_user:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function populates vfe diag data
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clf46_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ISP_ERR("TODO: not implemented yet");
  return TRUE;
}

/** clf46_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration using default values from
 *  chromatix ptr
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clf46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  clf46_t               *mod = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;

  ISP_DBG(" E");
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
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

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  mod = (clf46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("clf enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  ret = clf46_init_config(isp_sub_module, mod);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** clf46_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @clf_mod: clf module instance
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean clf46_store_hw_update(isp_sub_module_t *isp_sub_module,
  clf46_t *clf_mod)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_CLF_CmdType             *reg_cmd = NULL;
  uint32_t                    *data = NULL;
  uint32_t                     len = 0;
  chromatix_parms_type        *chroma_ptr = NULL;
  chromatix_videoHDR_type     *chromatix_VHDR = NULL;
  uint32_t                    multifactor =  0;
  boolean                     is_lsb_alligned = FALSE;

  if (!isp_sub_module || !clf_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, clf_mod);
    return FALSE;
  }

  chroma_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chroma_ptr) {
    ISP_ERR("failed: chroma_ptr %p", chroma_ptr);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  chromatix_VHDR =
    &chroma_ptr->chromatix_post_processing.chromatix_video_HDR;

  /*when HDR mode, chromatix tuned by 14 bit,
    otherwise keep 12 bit header for backward compatible*/
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  } else {
    multifactor = 0;
  }
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ) {
      multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
      is_lsb_alligned = TRUE;
  }

  if (is_lsb_alligned){
      clf_mod->reg_cmd.threshold.edge >>= multifactor;
      clf_mod->reg_cmd.threshold.sat  >>= multifactor;
  } else {
      clf_mod->reg_cmd.threshold.edge <<= multifactor;
      clf_mod->reg_cmd.threshold.sat  <<= multifactor;
  }

  /*regular hw write for clf*/
  clf46_debug(&clf_mod->reg_cmd);

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) );
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, sizeof(struct msm_vfe_reg_cfg_cmd) );

  reg_cmd = (ISP_CLF_CmdType *)CAM_MALLOC(sizeof(ISP_CLF_CmdType));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(ISP_CLF_CmdType));
  *reg_cmd = clf_mod->reg_cmd;
  cfg_cmd->cfg_data = (void *) reg_cmd;
  cfg_cmd->cmd_len = sizeof(clf_mod->reg_cmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CLF46_HV_NZ_FLAG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_CLF46_LEN * sizeof(uint32_t);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
#ifdef PRINT_REG_VAL_SET
  ISP_DBG("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cfg_cmd->cfg_data;
  for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
    ISP_DBG("data[%d] %x", len, data[len]);
  }
#endif

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: clf46_util_append_cfg");
    goto ERROR_APPEND;
  }
  isp_sub_module->trigger_update_pending = FALSE;
  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);
  return FALSE;
} /* clf46_do_hw_update */

/** clf46_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module instance
 *  @event: mct event for trigger update
 *
 *  This function checks and initiates triger update of module
 *
 *  Return TRUE on Success and FALSE Parameters size mismatch
 **/
boolean clf46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  clf46_t                      *mod = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  int8_t                       module_enable;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (clf46_t *)isp_sub_module->private_data;
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

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    module_enable = ((isp_sub_module->manual_ctrls.cac_mode ==
      CAM_COLOR_CORRECTION_ABERRATION_FAST) ||
      (isp_sub_module->manual_ctrls.cac_mode ==
       CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY))? TRUE : FALSE;

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
    ISP_DBG("No trigger update for cac: enable = %d, trigger_en = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  if (isp_sub_module->trigger_update_pending == TRUE) {
    ret = clf46_store_hw_update(isp_sub_module, mod);
    if (ret == FALSE) {
      ISP_ERR("failed: clf46_do_hw_update");
    }
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* clf46_trigger_update */

/** clf46_streamon:
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
boolean clf46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  clf46_t          *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_DBG("");
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (clf46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* clf46_streamon */

/** clf46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clf46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  clf46_t *mod = NULL;

  ISP_DBG("");
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (clf46_t *)isp_sub_module->private_data;
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
  isp_sub_module->trigger_update_pending = FALSE;
  isp_sub_module->manual_ctrls.cac_mode = CAM_COLOR_CORRECTION_ABERRATION_FAST;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* clf46_streamoff */

/** clf46_reset:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets clf module
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clf46_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  clf46_t *mod = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }
  ISP_DBG(" E");
  mod = (clf46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));

  isp_sub_module->trigger_update_pending  = FALSE;
  isp_sub_module->submod_trigger_enable   = TRUE;
  isp_sub_module->submod_enable           = TRUE;
  isp_sub_module->manual_ctrls.cac_mode = CAM_COLOR_CORRECTION_ABERRATION_FAST;
  return TRUE;
} /* clf46_reset */



/** clf46_query_cap:
 *
 *    @module: mct module instance
 *    @query_buf: query caps buffer handle
 *
 *  @sessionid: session id
 *  Returns TRUE
 *
 **/
boolean clf46_query_cap(mct_module_t *module,
  void *query_buf)
{
  mct_pipeline_cap_t *cap_buf;
  mct_pipeline_isp_cap_t *mod_cap = NULL;

  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->aberration_modes_count = 2;
  mod_cap->aberration_modes[0] = CAM_COLOR_CORRECTION_ABERRATION_OFF;
  mod_cap->aberration_modes[1] = CAM_COLOR_CORRECTION_ABERRATION_FAST;
  mod_cap->aberration_modes[2] = CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY;
  return TRUE;
}

/** clf46_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  This function instantiates a clf module
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean clf46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  clf46_t *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (clf46_t *)CAM_MALLOC(sizeof(clf46_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  isp_sub_module->private_data = (void *)mod;
  clf46_reset(module, isp_sub_module, NULL);
  return TRUE;
} /* clf4_init */

/** clf46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void clf46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* clf46_destroy */
