/* wb46.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>
/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "wb46.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_WB, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_WB, fmt, ##args)

/** wb46_debug:
 *
 *  @p_cmd: config cmd to be printed
 *
 *  Print the values in config cmd
 *
 *  Return void
 **/
static void wb46_debug(ISP_WhiteBalanceConfig0_CmdType* p_cmd0, ISP_WhiteBalanceConfig1_CmdType* p_cmd1)
{
  ISP_DBG("ISP_WhiteBalanceCfgCmd.ch0Gain = %d\n",
    p_cmd0->ch0Gain);
  ISP_DBG("ISP_WhiteBalanceCfgCmd.ch1Gain = %d\n",
    p_cmd0->ch1Gain);
  ISP_DBG("ISP_WhiteBalanceCfgCmd.ch2Gain = %d\n",
    p_cmd1->ch2Gain);
}/*wb_debug*/

/** wb46_update_hw_gain_reg:
 *    @wb_mod: Pointer to wb module
 *
 *  Update the gain registers in mod
 *
 *
 *  Return void
 **/
static void wb46_update_hw_gain_reg(wb46_t *wb_mod)
{
  wb_mod->ISP_WhiteBalanceCfg0_Cmd.ch0Gain =
    WB_GAIN(wb_mod->awb_gain.g_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfg0_Cmd.ch1Gain =
    WB_GAIN(wb_mod->awb_gain.b_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfg1_Cmd.ch2Gain =
    WB_GAIN(wb_mod->awb_gain.r_gain * wb_mod->dig_gain);
}

/** wb46_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @wb_mod: wb module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean wb46_do_hw_update(isp_sub_module_t *isp_sub_module,
  wb46_t *wb_mod, isp_sub_module_output_t *output)
{
  boolean                        ret = TRUE;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  ISP_WhiteBalanceConfig0_CmdType *ISP_WhiteBalanceCfg0_Cmd = NULL;

  if (!isp_sub_module || !wb_mod || !output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, wb_mod, output);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)
    CAM_MALLOC(sizeof(struct msm_vfe_cfg_cmd_list));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd =
    (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd)* 2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  ISP_WhiteBalanceCfg0_Cmd = (ISP_WhiteBalanceConfig0_CmdType *)
    CAM_MALLOC(sizeof(ISP_WhiteBalanceConfig0_CmdType));
  if (!ISP_WhiteBalanceCfg0_Cmd) {
    ISP_ERR("failed: ISP_WhiteBalanceCfg0_Cmd %p", ISP_WhiteBalanceCfg0_Cmd);
    goto ERROR_FREE_REG_CFG_CMD;
  }
  memset(ISP_WhiteBalanceCfg0_Cmd, 0, sizeof(*ISP_WhiteBalanceCfg0_Cmd));


  *ISP_WhiteBalanceCfg0_Cmd = wb_mod->ISP_WhiteBalanceCfg0_Cmd;
  cfg_cmd->cfg_data = (void *)ISP_WhiteBalanceCfg0_Cmd;
  cfg_cmd->cmd_len = sizeof(*ISP_WhiteBalanceCfg0_Cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CLF_WB46_CFG_0;
  reg_cfg_cmd[0].u.rw_info.len = sizeof(uint32_t);

  reg_cfg_cmd[1].u.mask_info.reg_offset = ISP_CLF_WB46_CFG_1;
  reg_cfg_cmd[1].u.mask_info.mask = ISP46_CLF_WB_MASK;
  reg_cfg_cmd[1].u.mask_info.val = wb_mod->ISP_WhiteBalanceCfg1_Cmd.ch2Gain;
  reg_cfg_cmd[1].cmd_type = VFE_CFG_MASK;

  wb46_debug(&wb_mod->ISP_WhiteBalanceCfg0_Cmd, &wb_mod->ISP_WhiteBalanceCfg1_Cmd);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_FREE_WB_CFG_CMD;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;

ERROR_FREE_WB_CFG_CMD:
  free(ISP_WhiteBalanceCfg0_Cmd);
ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
  return FALSE;
}

/** wb46_manual_update:
 *
   @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  This function uses the manual CC gain values and update
 *  the HW
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean wb46_manual_update(
   isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  wb46_t                       *wb = NULL;
  boolean                       ret = TRUE;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
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

  wb = (wb46_t *)isp_sub_module->private_data;
  if (!wb) {
    ISP_ERR("failed: wb %p", wb);
    return FALSE;
  }

  wb->dig_gain = 1.0;

  wb->awb_gain.r_gain = isp_sub_module->manual_ctrls.cc_gain.gains[0];
  /* Use the G-even and ignore G-odd */
  wb->awb_gain.g_gain = isp_sub_module->manual_ctrls.cc_gain.gains[1];
  wb->awb_gain.b_gain = isp_sub_module->manual_ctrls.cc_gain.gains[3];

  wb46_update_hw_gain_reg(wb);

  ret = wb46_do_hw_update(isp_sub_module, wb, output);
  if (ret == FALSE) {
    ISP_ERR("failed: wb_do_hw_update ret %d", ret);
  }

  return ret;
}

/** wb_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using awb_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean wb46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  wb46_t                       *wb = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
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

  wb = (wb46_t *)isp_sub_module->private_data;
  if (!wb) {
    ISP_ERR("failed: wb %p", wb);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
      !isp_sub_module->config_pending) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.cc_mode == CAM_COLOR_CORRECTION_TRANSFORM_MATRIX &&
      ((isp_sub_module->manual_ctrls.wb_mode == CAM_WB_MODE_OFF &&
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_AUTO) ||
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF)) {
        isp_sub_module->submod_trigger_enable = FALSE;

        if (isp_sub_module->manual_ctrls.update_cc_gain) {
          ret = wb46_manual_update(isp_sub_module, event);
          isp_sub_module->manual_ctrls.update_cc_gain = FALSE;
          goto FILL_METADATA;
        }
    } else {
      if(isp_sub_module->manual_ctrls.cc_mode !=
         CAM_COLOR_CORRECTION_TRANSFORM_MATRIX) {
        isp_sub_module->submod_trigger_enable = TRUE;
      }

      if (isp_sub_module->manual_ctrls.update_cc_gain){
        ISP_ERR("Manual CC GAIN not applied");
        goto FILL_METADATA;
      }
    }
  }

  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if ((isp_sub_module->submod_enable == FALSE)) {
    ISP_DBG("enable = %d",isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((isp_sub_module->submod_trigger_enable == FALSE) ||
      (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("trigger_enable = %d hw update pending %d",
           isp_sub_module->submod_trigger_enable,
           isp_sub_module->trigger_update_pending);
    goto FILL_METADATA;
  }

  wb46_update_hw_gain_reg(wb);

  ret = wb46_do_hw_update(isp_sub_module, wb, output);
  if (ret == FALSE) {
    ISP_ERR("failed: wb_do_hw_update ret %d", ret);
    goto FILL_METADATA;
  }
FILL_METADATA:

  if (output->meta_dump_params &&
    output->meta_dump_params->metadata_dump_enable == 1) {
    output->meta_dump_params->frame_meta.awb_gain.b_gain = wb->awb_gain.b_gain;
    output->meta_dump_params->frame_meta.awb_gain.r_gain = wb->awb_gain.r_gain;
    output->meta_dump_params->frame_meta.awb_gain.g_gain = wb->awb_gain.g_gain;
  }
  if (output->frame_meta) {
    output->frame_meta->awb_update = wb->awb_update;
    output->frame_meta->app_ctrl_mode =
      isp_sub_module->manual_ctrls.app_ctrl_mode;
    output->frame_meta->cc_gain.gains[0] = wb->awb_gain.r_gain;
    output->frame_meta->cc_gain.gains[1] = wb->awb_gain.g_gain;
    output->frame_meta->cc_gain.gains[2] = wb->awb_gain.g_gain;
    output->frame_meta->cc_gain.gains[3] = wb->awb_gain.b_gain;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* wb_trigger_update */

/** wb_reset:
 *
 *  @mod: Pointer to wb module
 *
 *  Perform reset of wb module
 *
 *  Return void
 **/
static void wb46_reset(wb46_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->dig_gain = 0.0;
  memset(&mod->ISP_WhiteBalanceCfg0_Cmd, 0,
    sizeof(mod->ISP_WhiteBalanceCfg0_Cmd));
  memset(&mod->ISP_WhiteBalanceCfg1_Cmd, 0,
    sizeof(mod->ISP_WhiteBalanceCfg1_Cmd));
  memset(&mod->ISP_WhiteBalanceRightCfg0_Cmd, 0,
    sizeof(mod->ISP_WhiteBalanceRightCfg0_Cmd));
  memset(&mod->ISP_WhiteBalanceRightCfg1_Cmd, 0,
    sizeof(mod->ISP_WhiteBalanceRightCfg1_Cmd));

  memset(&mod->awb_gain, 0, sizeof(mod->awb_gain));
}

/** wb_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to wb module
 *  @chroma_ptr: chromatix ptr
 *
 *  Configure the wb module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean wb46_config(isp_sub_module_t *isp_sub_module, wb46_t *mod,
  void *chroma_param_ptr)
{
  uint32_t                   i = 0;
  awb_MWB_type              *chromatix_MWB = NULL;
  awb_algo_struct_type      *AWB_bayer_algo_data = NULL;

  if (!isp_sub_module || !mod || !chroma_param_ptr) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, mod, chroma_param_ptr);
    return FALSE;
  }

  chromatix_3a_parms_type *chroma_ptr = chroma_param_ptr;

  chromatix_MWB = &chroma_ptr->AWB_bayer_algo_data.awb_MWB;
  AWB_bayer_algo_data = &chroma_ptr->AWB_bayer_algo_data;

  mod->dig_gain= 1.0; //no apply aec gain for now, could be used in the future

  wb46_update_hw_gain_reg(mod);
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}

/** wb46_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean wb46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  modulesChromatix_t          *module_chromatix_ptrs = NULL;
  chromatix_parms_type        *chroma_ptr = NULL;
  awb_MWB_type                *chromatix_MWB = NULL;
  awb_algo_struct_type        *AWB_bayer_algo_data = NULL;
  wb46_t                      *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  module_chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!module_chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", module_chromatix_ptrs);
    return FALSE;
  }

  mod = (wb46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *module_chromatix_ptrs;
  isp_sub_module->trigger_update_pending = TRUE;

  if (F_EQUAL(mod->awb_gain.r_gain, 0.0f) && F_EQUAL(mod->awb_gain.g_gain, 0.0f)
    && F_EQUAL(mod->awb_gain.b_gain, 0.0f)) {

    chromatix_3a_parms_type *chroma_ptr =
      isp_sub_module->chromatix_ptrs.chromatix3APtr;

    chromatix_MWB = &chroma_ptr->AWB_bayer_algo_data.awb_MWB;
    AWB_bayer_algo_data = &chroma_ptr->AWB_bayer_algo_data;

    mod->awb_gain.b_gain = chromatix_MWB->MWB_tl84.b_gain *
      AWB_bayer_algo_data->awb_basic_tuning.awb_adjust_gain_table.
        gain_adj_normallight[AGW_AWB_WARM_FLO].blue_gain_adj;
    mod->awb_gain.r_gain = chromatix_MWB->MWB_tl84.r_gain *
      AWB_bayer_algo_data->awb_basic_tuning.awb_adjust_gain_table.
        gain_adj_normallight[AGW_AWB_WARM_FLO].red_gain_adj;
    mod->awb_gain.g_gain = chromatix_MWB->MWB_tl84.g_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** wb46_stats_awb_update:
 *
 *  @module: demosaic module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean wb46_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t                      *stats_update = NULL;
  chromatix_manual_white_balance_type *cur_awb_gain = NULL;
  awb_gain_t                          *new_awb_gain = NULL;
  wb46_t                              *wb = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  wb = (wb46_t *)isp_sub_module->private_data;
  if (!wb) {
    ISP_ERR("failed: wb %p", wb);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_awb_gain = &wb->awb_gain;
  new_awb_gain = &stats_update->awb_update.gain;

  if (isp_sub_module->trigger_update_pending == FALSE) {
   if (!WB_GAIN_EQUAL(new_awb_gain, wb->awb_gain) &&
       !WB_GAIN_EQ_ZERO(new_awb_gain)) {
      isp_sub_module->trigger_update_pending = TRUE;
    }
  }

  ISP_DBG("old gain g=%f b=%f r=%f new g=%f b=%f r=%f",
    cur_awb_gain->g_gain, cur_awb_gain->b_gain,
    cur_awb_gain->r_gain, new_awb_gain->g_gain,
    new_awb_gain->b_gain, new_awb_gain->r_gain);

  /* Store AWB update in module private */
  cur_awb_gain->g_gain = new_awb_gain->g_gain;
  cur_awb_gain->b_gain = new_awb_gain->b_gain;
  cur_awb_gain->r_gain = new_awb_gain->r_gain;
  wb->awb_update = stats_update->awb_update;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** wb46_streamon:
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
boolean wb46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  wb46_t                *mod = NULL;
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

  mod = (wb46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("wb enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = wb46_config(isp_sub_module, mod,
    isp_sub_module->chromatix_ptrs.chromatix3APtr);
  if (ret == FALSE) {
    ISP_ERR("failed: wb46_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* wb46_config */

/** wb46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean wb46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  wb46_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (wb46_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  wb46_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.update_cc_gain = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* wb46_streamoff */

/** wb_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the wb module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean wb46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  wb46_t *wb = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  wb = (wb46_t *)CAM_MALLOC(sizeof(wb46_t));
  if (!wb) {
    ISP_ERR("failed: wb %p", wb);
    return FALSE;
  }

  memset(wb, 0, sizeof(*wb));
  wb46_reset(wb);

  isp_sub_module->private_data = (void *)wb;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.update_cc_gain = FALSE;

  return TRUE;
}/* wb_init */

/** wb46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void wb46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* wb46_destroy */
