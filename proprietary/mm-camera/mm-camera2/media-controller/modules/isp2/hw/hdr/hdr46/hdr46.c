/* hdr46.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"
#include "camera_dbg.h"

/* isp headers */
#include "module_hdr46.h"
#include "hdr46.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_HDR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_HDR, fmt, ##args)

/** hdr46_debug:
 *
 *  @cmd: configuration command
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps hdr mac configuration
 *
 *  Return: None
 **/
static void hdr46_debug(hdr_reg_cmd_t *RegCmd)
{
  if (!RegCmd) {
    ISP_ERR("failed: NULL pointer");
    return;
  }

  /* HDR reg*/
  ISP_DBG("HDR_CFG WB Rg gain %d ", RegCmd->hdrCfgCmd.RgWbGainRatio);
  ISP_DBG("HDR_CFG WB Bg gain %d ", RegCmd->hdrCfgCmd.BgWbGainRatio);
  ISP_DBG("HDR_CFG WB Gb gain %d ", RegCmd->hdrCfgCmd.GbWbGainRatio);
  ISP_DBG("HDR_CFG WB Gr gain %d", RegCmd->hdrCfgCmd.GrWbGainRatio);
  ISP_DBG("HDR_CFG exp ratio(Q10) %d", RegCmd->hdrCfgCmd.ExpRatio);
  ISP_DBG("HDR_CFG recon first field %d ", RegCmd->hdrCfgCmd.ReconFirstFiled );

  /* HDR RECON reg*/
  ISP_DBG("Recon_Cfg ReconLinearMode %d", RegCmd->ReconCfgCmd.ReconLinearMode);
  ISP_DBG("Recon_Cfg ReconHedgeTH1 %d", RegCmd->ReconCfgCmd.ReconHedgeTH1);
  ISP_DBG("Recon_Cfg ReconHedgeDthLog2 %d", RegCmd->ReconCfgCmd.ReconHedgeDthLog2);
  ISP_DBG("Recon_Cfg ReconMotionTH1 %d", RegCmd->ReconCfgCmd.ReconMotionTH1);
  ISP_DBG("Recon_Cfg ReconMotionDthLog2 %d", RegCmd->ReconCfgCmd.ReconMotionDthLog2);
  ISP_DBG("Recon_Cfg ReconDarkTH1 %d", RegCmd->ReconCfgCmd.ReconDarkTH1);
  ISP_DBG("Recon_Cfg ReconDarkDthLog2 %d", RegCmd->ReconCfgCmd.ReconDarkDthLog2);
  ISP_DBG("Recon_Cfg ReconFlatRegionTH %d", RegCmd->ReconCfgCmd.ReconFlatRegionTH);
  ISP_DBG("Recon_Cfg ReconEdgeLpfTap0 %d", RegCmd->ReconCfgCmd.ReconEdgeLpfTap0);
  ISP_DBG("Recon_Cfg ReconMinFactor %d", RegCmd->ReconCfgCmd.ReconMinFactor);

  /* HDR MAC reg*/
  ISP_DBG("MAC_cfg MacMotion0TH1 %d",RegCmd->MacCfgCmd.MacMotion0TH1);
  ISP_DBG("MAC_cfg MacMotion0TH2 %d",RegCmd->MacCfgCmd.MacMotion0TH2);
  ISP_DBG("MAC_cfg MacSqrtAnalogGain %d",RegCmd->MacCfgCmd.MacSqrtAnalogGain);
  ISP_DBG("MAC_cfg MacDilation    %d",RegCmd->MacCfgCmd.MacDilation);
  ISP_DBG("MAC_cfg MacMotion0Dt0  %d",RegCmd->MacCfgCmd.MacMotion0Dt0);
  ISP_DBG("MAC_cfg MacMotionStength %d",RegCmd->MacCfgCmd.MacMotionStength);
  ISP_DBG("MAC_cfg MacLowLightTH1 %d",RegCmd->MacCfgCmd.MacLowLightTH1);
  ISP_DBG("MAC_cfg MacLowLightDthLo %d",RegCmd->MacCfgCmd.MacLowLightDthLog2);
  ISP_DBG("MAC_cfg MacLowLightStren %d",RegCmd->MacCfgCmd.MacLowLightStrength_H);
  ISP_DBG("MAC_cfg MacHighLightDthL %d",RegCmd->MacCfgCmd.MacHighLightDthLog2);
  ISP_DBG("MAC_cfg MacHighLightTH1 %d",RegCmd->MacCfgCmd.MacHighLightTH1);
  ISP_DBG("MAC_cfg MacSmoothTH1   %d",RegCmd->MacCfgCmd.MacSmoothTH1);
  ISP_DBG("MAC_cfg MacSmoothEnable %d",RegCmd->MacCfgCmd.MacSmoothEnable);
  ISP_DBG("MAC_cfg MacSmoothDthLog2 %d",RegCmd->MacCfgCmd.MacSmoothDthLog2);
  ISP_DBG("MAC_cfg MacSmoothTap0  %d",RegCmd->MacCfgCmd.MacSmoothTap0);
  ISP_DBG("MAC_cfg ExpRatioRecip  %d",RegCmd->MacCfgCmd.ExpRatioRecip);
  ISP_DBG("MAC_cfg MsbAligned     %d",RegCmd->MacCfgCmd.MsbAligned);
  ISP_DBG("MAC_cfg MacLinearMode  %d",RegCmd->MacCfgCmd.MacLinearMode);
}

/** hdr46_store_hw_update:
 *
 *  @isp_sub_mod: sub module handle
 *  @hdr_mac_mod: module private
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:  boolean
 **/
static boolean hdr46_store_hw_update(
  isp_sub_module_t *isp_sub_module,
  hdr46_t *hdr_mod)
{
  boolean                      ret = TRUE;
  int                          i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  hdr_reg_cmd_t               *reg_cmd = NULL;

  if (!hdr_mod || !isp_sub_module) {
    ISP_ERR("failed: %p %p", hdr_mod, isp_sub_module);
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
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 3);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)) * 3);

  reg_cmd = (hdr_reg_cmd_t *)CAM_MALLOC(sizeof(hdr_reg_cmd_t));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(hdr_reg_cmd_t));

  *reg_cmd = hdr_mod->RegCmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 3;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_HDR_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_HDR_LEN * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + sizeof(ISP_HdrCfgCmdType);
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_HDR_RECON_OFF;
  reg_cfg_cmd[1].u.rw_info.len = ISP_HDR_RECON_LEN * sizeof(uint32_t);

  reg_cfg_cmd[2].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + sizeof(ISP_HdrReconCfgCmdType) ;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_HDR_MAC_OFF;
  reg_cfg_cmd[2].u.rw_info.len = ISP_HDR_MAC_LEN * sizeof(uint32_t);

  hdr46_debug(&hdr_mod->RegCmd);
  hdr_mod->applied_RegCmd = hdr_mod->RegCmd;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: hdr46_util_append_cfg");
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

/** hdr46_streamon:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of hdr mac
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean hdr46_prepare_hw_config(
  isp_sub_module_t *isp_sub_module, hdr46_t *hdr)
{
  boolean                  ret = TRUE;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_videoHDR_type *chromatix_VHDR = NULL;
  chromatix_videoHDR_core  *chromatix_HDR_core = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;

  if (!hdr || !isp_sub_module) {
    ISP_ERR("failed: %p %p", hdr, isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x309)
  chromatix_HDR_core = &chromatix_VHDR->videoHDR_core_data[0];
#else
  chromatix_HDR_core = &chromatix_VHDR->videoHDR_core_data;
#endif
  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

  /*config HDR reg*/
  hdr->RegCmd.hdrCfgCmd.RgWbGainRatio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, hdr->wb_rg_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  hdr->RegCmd.hdrCfgCmd.BgWbGainRatio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, hdr->wb_bg_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  hdr->RegCmd.hdrCfgCmd.GbWbGainRatio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, hdr->wb_gb_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  hdr->RegCmd.hdrCfgCmd.GrWbGainRatio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, hdr->wb_gr_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);

  hdr->RegCmd.hdrCfgCmd.ExpRatio = CLAMP_LIMIT(
    FLOAT_TO_Q(10, hdr->exp_ratio), ISP_HDR_EXP_RATIO_MIN,
    ISP_HDR_EXP_RATIO_MAX);
  hdr->RegCmd.hdrCfgCmd.ReconFirstFiled = chromatix_HDR_reserve->recon_first_field;

  /*config HDR recon reg,
    Recon decide how to reconstruct from sensor frame to T1 and T2 frame*/
  hdr->RegCmd.ReconCfgCmd.ReconLinearMode =
    chromatix_HDR_reserve->recon_linear_mode;
  hdr->RegCmd.ReconCfgCmd.ReconHedgeTH1 =
    chromatix_HDR_core->recon_h_edge_th1;
  hdr->RegCmd.ReconCfgCmd.ReconHedgeDthLog2 =
    chromatix_HDR_reserve->recon_h_edge_dth_log2;
  hdr->RegCmd.ReconCfgCmd.ReconMotionTH1 =
    chromatix_HDR_core->recon_motion_th1;
  hdr->RegCmd.ReconCfgCmd.ReconMotionDthLog2 =
    chromatix_HDR_reserve->recon_motion_dth_log2;
  hdr->RegCmd.ReconCfgCmd.ReconDarkTH1 =
    chromatix_HDR_core->recon_dark_th1;
  hdr->RegCmd.ReconCfgCmd.ReconDarkDthLog2 =
    chromatix_HDR_reserve->recon_dark_dth_log2;
  hdr->RegCmd.ReconCfgCmd.ReconFlatRegionTH =
    chromatix_HDR_core->recon_flat_region_th;
  hdr->RegCmd.ReconCfgCmd.ReconEdgeLpfTap0 =
    chromatix_HDR_reserve->recon_edge_lpf_tap0;
  hdr->RegCmd.ReconCfgCmd.ReconMinFactor =
    chromatix_HDR_core->recon_min_factor;

  /*config HDR MAC reg
    MAC decide how to combine T1 and T2 two differnt Exp image*/
  hdr->RegCmd.MacCfgCmd.MacMotion0TH1 = chromatix_HDR_core->mac_motion0_th1;
  hdr->RegCmd.MacCfgCmd.MacMotion0TH2 = chromatix_HDR_core->mac_motion0_th2;
  hdr->RegCmd.MacCfgCmd.MacDilation = chromatix_HDR_reserve->mac_motion_dilation;
  hdr->RegCmd.MacCfgCmd.MacMotion0Dt0 = chromatix_HDR_reserve->mac_motion0_dt0;
  hdr->RegCmd.MacCfgCmd.MacMotionStength =
    chromatix_HDR_core->mac_motion_strength;
  hdr->RegCmd.MacCfgCmd.MacLowLightTH1 = chromatix_HDR_core->mac_low_light_th1;

  hdr->RegCmd.MacCfgCmd.MacLowLightDthLog2 = chromatix_HDR_reserve->mac_low_light_dth_log2;
  hdr->RegCmd.MacCfgCmd.MacLowLightStrength_H =
    chromatix_HDR_core->mac_low_light_strength;
  hdr->RegCmd.MacCfgCmd.MacHighLightDthLog2 = chromatix_HDR_core->mac_high_light_dth_log2;
  hdr->RegCmd.MacCfgCmd.MacHighLightTH1 = chromatix_HDR_core->mac_high_light_th1;

  hdr->RegCmd.MacCfgCmd.MacSmoothTH1 =
    chromatix_HDR_reserve->mac_smooth_th1;
  hdr->RegCmd.MacCfgCmd.MacSmoothEnable = chromatix_HDR_reserve->mac_smooth_enable;
  hdr->RegCmd.MacCfgCmd.MacSmoothDthLog2 = chromatix_HDR_reserve->mac_smooth_dth_log2;
  hdr->RegCmd.MacCfgCmd.MacSmoothTap0 = chromatix_HDR_reserve->mac_smooth_tap0;
  hdr->RegCmd.MacCfgCmd.ExpRatioRecip = CLAMP_LIMIT(
    FLOAT_TO_Q(8, (1.0 / hdr->exp_ratio)), ISP_HDR_EXP_RECIP_RATIO_MIN,
    ISP_HDR_EXP_RECIP_RATIO_MAX);

  /* Default its LSB, with LSB alligned,
     some other module(BPC/BCC/demo/LINEA/ABF....) config should not shift 2 bit
     if we set it to be MSB alligned, then those config can keep the same
     which is always shift 2 bit from 12 bit to 14 bit */
  hdr->RegCmd.MacCfgCmd.MsbAligned = 0;
  hdr->RegCmd.MacCfgCmd.MacLinearMode = chromatix_HDR_reserve->mac_linear_mode;

  /*analog gain is global gain from AEC*/
  hdr->RegCmd.MacCfgCmd.MacSqrtAnalogGain = CLAMP_LIMIT(
    FLOAT_TO_Q(4, sqrt(hdr->analog_gain)), ISP_HDR_MAC_SQRT_ANALOG_GAIN_MIN,
    ISP_HDR_MAC_SQRT_ANALOG_GAIN_MAX);

  /* bypass HW by exp ratio = 1 && linear mode, System team will suggest later*/
#if 0
    if  (F_EQUAL(hdr->exp_ratio, 1.0)) {
    /*if exp ratio = 1,
      we should by pass the HW block to further save power consumption.
      confirm with system team about exp ratio = 1 + normal/linear mode */
    hdr->RegCmd.MacCfgCmd.MacLinearMode = 1;
    hdr->RegCmd.ReconCfgCmd.ReconLinearMode = 1;
    hdr->RegCmd.hdrCfgCmd.ExpRatio = FLOAT_TO_Q(10, 1.0);
    hdr->RegCmd.MacCfgCmd.ExpRatioRecip = FLOAT_TO_Q(8, 1.0);
  }
#endif

  return ret;
}

/** hdr46_streamon:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of hdr mac
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean hdr46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  hdr46_t                 *hdr = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_videoHDR_type *chromatix_vhdr = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  hdr = (hdr46_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &hdr->streaming_mode_mask);

  hdr->wb_rg_ratio = 1.0;
  hdr->wb_bg_ratio = 1.0;
  hdr->wb_gr_ratio = 1.0;
  hdr->wb_gb_ratio = 1.0;
  hdr->aec_update.hdr_sensitivity_ratio = 1.0;

  ret = hdr46_prepare_hw_config(isp_sub_module, hdr);
  if (ret == FALSE) {
    ISP_ERR("prepare hw cfg failed!");
  }

  hdr->trigger_enable = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;
  hdr->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** hdr46_streamoff:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function resets hdr mac module
 *
 *  Return: boolean
 **/
boolean hdr46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  hdr46_t *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mod = (hdr46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &mod->streaming_mode_mask);

  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX */

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** hdr46_save_awb_params:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean hdr46_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  float              ratio = 0.0;
  stats_update_t     *stats_update = NULL;
  hdr46_t            *hdr = NULL;

  ISP_HIGH("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  hdr = (hdr46_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* need awb trigger update, save the color temp*/
  hdr->awb_update = stats_update->awb_update;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** hdr46_save_aec_param:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean hdr46_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  stats_update_t          *stats_update = NULL;
  hdr46_t                 *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_videoHDR_type *chromatix_vhdr = NULL;

  ISP_HIGH("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
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

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  mod = (hdr46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Store AEC update in module private*/
  mod->aec_update = stats_update->aec_update;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** hdr46_trigger_update:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean hdr46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  isp_private_event_t     *private_event = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  hdr46_t                 *hdr_mod = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  chromatix_videoHDR_type *chromatix_VHDR = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /* get chromatix pointer*/
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }

  /*get chroa enhance hw update list*/
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

  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  /* get hdr mod private*/
  hdr_mod = (hdr46_t *)isp_sub_module->private_data;
  if (!hdr_mod) {
    ISP_ERR("failed: mod %p", hdr_mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
      ISP_DBG("skip trigger update: enable = %d",
        isp_sub_module->submod_enable);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return ret;
  }

  if (isp_sub_module->trigger_update_pending == TRUE &&
      isp_sub_module->submod_trigger_enable == TRUE) {
     if (!F_EQUAL(hdr_mod->awb_update.gain.g_gain , 0.0) &&
         !F_EQUAL(hdr_mod->awb_update.gain.r_gain , 0.0) &&
         !F_EQUAL(hdr_mod->awb_update.gain.b_gain , 0.0)) {
       hdr_mod->wb_rg_ratio =
         hdr_mod->awb_update.gain.r_gain / hdr_mod->awb_update.gain.g_gain;
       hdr_mod->wb_bg_ratio =
         hdr_mod->awb_update.gain.b_gain / hdr_mod->awb_update.gain.g_gain;
       hdr_mod->wb_gr_ratio =
         hdr_mod->awb_update.gain.g_gain / hdr_mod->awb_update.gain.r_gain;
       hdr_mod->wb_gb_ratio =
         hdr_mod->awb_update.gain.g_gain / hdr_mod->awb_update.gain.b_gain;
     } else {
       hdr_mod->wb_rg_ratio = 1.0;
       hdr_mod->wb_bg_ratio = 1.0;
       hdr_mod->wb_gr_ratio = 1.0;
       hdr_mod->wb_gb_ratio = 1.0;
     }

    if (!F_EQUAL(hdr_mod->aec_update.hdr_sensitivity_ratio, 0.0)) {
       hdr_mod->exp_ratio = hdr_mod->aec_update.hdr_sensitivity_ratio;
    } else {
      hdr_mod->exp_ratio = 1.0;
    }
    hdr_mod->analog_gain = hdr_mod->aec_update.sensor_gain;

    ret = hdr46_prepare_hw_config(isp_sub_module, hdr_mod);
    if (ret == FALSE) {
      ISP_ERR("prepare hw cfg failed!");
    }
    ret = hdr46_store_hw_update(isp_sub_module, hdr_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: hdr46_do_hw_update");
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** hdr46_update_streaming_mode_mask:
 *
 *  @module: hdr mac module instance
 *  @isp_sub_module: sub module handle
 *  @streaming_mode_mask: streaming mode mask
 *
 *  update stream mask
 *  Return none
 **/
void hdr46_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask)
{
  hdr46_t *hdr_mac = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  hdr_mac = (hdr46_t *)isp_sub_module->private_data;
  if (!hdr_mac) {
    ISP_ERR("failed: hdr mac %p", hdr_mac);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return;
  }
  hdr_mac->streaming_mode_mask = streaming_mode_mask;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
}

/** hdr46_init:
 *
 *  @module: hdr mac module handle
 *  @isp_sub_module: submodule data
 *
 *  This function instantiates a hdr mac module
 *
 *  Return: boolean
 **/
boolean hdr46_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  hdr46_t *mod = NULL;

  ISP_HIGH("E");
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (hdr46_t *)CAM_MALLOC(sizeof(hdr46_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));

  /* enable trigger update feature flag from PIX */
  mod->trigger_enable = 1;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  isp_sub_module->private_data = (void *)mod;

  return TRUE;
}

/** hdr46_destroy:
 *
 *  @module: hdr mac module handle
 *  @isp_sub_module: submodule data
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void hdr46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  hdr46_t *mod = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
}
