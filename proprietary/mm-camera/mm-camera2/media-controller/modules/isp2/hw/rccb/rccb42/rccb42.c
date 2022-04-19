/* rccb42.c
 *
 * Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "rccb_reg.h"
#include "rccb42.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_RCCB, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_RCCB, fmt, ##args)

/** rccb42_debug:
 *
 *  @p_cmd: config cmd to be printed
 *
 *  Print the values in config cmd
 *
 *  Return void
 **/
static void rccb42_debug(rccb42_t *mod)
{
  ISP_DBG("R gain = %d R offset %d\n",
    mod->r_gain.gain, mod->r_offset.offset);
  ISP_DBG("G gain = %d G offset %d\n",
    mod->g_gain.gain, mod->g_offset.offset);
  ISP_DBG("B gain = %d B offset %d\n",
    mod->b_gain.gain, mod->b_offset.offset);
}/*rccb42_debug*/

/** rccb42_is_rcc_sensor
 *
 * @fmt : Sensor output format
 *
 * Return whether sensor is RCCB or Not
 *
 **/
boolean rccb42_is_rccb_sensor(cam_format_t fmt)
{
   boolean is_rccb = TRUE;
   switch (fmt) {
     case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
     case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
     case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
     case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
     case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
     case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
     case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
     case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
     case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
     case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
     case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
     case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
     case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
     case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
     case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
     case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
     case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
     case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
     case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
     case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
     case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
     case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
     case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
     case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
       is_rccb = FALSE;
       break;
     default:
       break;
   }
   return is_rccb;
}
/** rccb42_update_hw_gain_reg:
 *  @mod: Pointer to rccb module
 *
 *  Update the gain registers in mod
 *
 *
 *  Return void
 **/
static void rccb42_update_hw_gain_reg(rccb42_t *mod)
{
  mod->g_gain.gain =
    WB_GAIN(mod->awb_gain.g_gain * mod->dig_gain);
  mod->b_gain.gain =
    WB_GAIN(mod->awb_gain.b_gain * mod->dig_gain);
  mod->r_gain.gain =
    WB_GAIN(mod->awb_gain.r_gain * mod->dig_gain);
  mod->r_offset.offset = 0;
  mod->g_offset.offset = 0;
  mod->b_offset.offset = 0;
}

/** rccb42_set_unity_gain
 * @mod: Pointer t rccb module
 *
 * Update the gain registers in mod
 *
 *
 * Return Void
 **/
static void rccb42_set_unity_gain(rccb42_t *mod)
{
  mod->g_gain.gain =
    WB_GAIN(1.0);
  mod->b_gain.gain =
    WB_GAIN(1.0);
  mod->r_gain.gain =
    WB_GAIN(1.0);
  mod->r_offset.offset = 0;
  mod->g_offset.offset = 0;
  mod->b_offset.offset = 0;
}

/** rccb42_store_hw_update
 *
 *  @isp_sub_module: sub module
 *  @rccb_mod: pointer to rcb module
 *
 *  @output: ouput list
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 */
static boolean rccb42_store_hw_update(isp_sub_module_t *isp_sub_module,
  rccb42_t *mod, isp_sub_module_output_t *output)
{
  boolean                        ret = TRUE;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;

  RETURN_IF_NULL(mod);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(output);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 6);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd) * 6));
  cfg_cmd->cfg_data = NULL;
  cfg_cmd->cmd_len = 0;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 6;

  reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_RCCB_WB_B_GAIN;
  reg_cfg_cmd[0].u.mask_info.mask = ISP_RCCB_WB_GAIN_MASK;
  reg_cfg_cmd[0].u.mask_info.val = mod->b_gain.reg;
  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[1].u.mask_info.reg_offset = ISP_RCCB_WB_R_GAIN;
  reg_cfg_cmd[1].u.mask_info.mask = ISP_RCCB_WB_GAIN_MASK;
  reg_cfg_cmd[1].u.mask_info.val = mod->r_gain.reg;
  reg_cfg_cmd[1].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[2].u.mask_info.reg_offset = ISP_RCCB_WB_G_GAIN;
  reg_cfg_cmd[2].u.mask_info.mask = ISP_RCCB_WB_GAIN_MASK;
  reg_cfg_cmd[2].u.mask_info.val = mod->g_gain.reg;
  reg_cfg_cmd[2].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[3].u.mask_info.reg_offset = ISP_RCCB_WB_B_OFFSET;
  reg_cfg_cmd[3].u.mask_info.mask = ISP_RCCB_WB_OFFSET_MASK;
  reg_cfg_cmd[3].u.mask_info.val = mod->b_offset.reg;
  reg_cfg_cmd[3].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[4].u.mask_info.reg_offset = ISP_RCCB_WB_R_OFFSET;
  reg_cfg_cmd[4].u.mask_info.mask = ISP_RCCB_WB_OFFSET_MASK;
  reg_cfg_cmd[4].u.mask_info.val = mod->r_offset.reg;
  reg_cfg_cmd[4].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[5].u.mask_info.reg_offset = ISP_RCCB_WB_G_OFFSET;
  reg_cfg_cmd[5].u.mask_info.mask = ISP_RCCB_WB_OFFSET_MASK;
  reg_cfg_cmd[5].u.mask_info.val = mod->g_offset.reg;
  reg_cfg_cmd[5].cmd_type = VFE_CFG_MASK;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_FREE_REG_CFG_CMD;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;

ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
  return FALSE;
}

/** rccb42_manual_update:
 *
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  This function uses the manual CC gain values and update
 *  the HW
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rccb42_manual_update(
   isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  rccb42_t                     *rccb = NULL;
  boolean                      ret = TRUE;
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

  rccb = (rccb42_t *)isp_sub_module->private_data;
  if (!rccb) {
    ISP_ERR("failed: rccb %p", rccb);
    return FALSE;
  }

  rccb->dig_gain = 1.0;

  rccb->awb_gain.r_gain = isp_sub_module->manual_ctrls.cc_gain.gains[0];
  /* Use the G-even and ignore G-odd */
  rccb->awb_gain.g_gain = isp_sub_module->manual_ctrls.cc_gain.gains[1];
  rccb->awb_gain.b_gain = isp_sub_module->manual_ctrls.cc_gain.gains[3];

  if (!rccb->is_rccb)
    rccb42_update_hw_gain_reg(rccb);
  else
    rccb42_set_unity_gain(rccb);

  ret = rccb42_store_hw_update(isp_sub_module, rccb, output);
  if (ret == FALSE) {
    ISP_ERR("failed: rccb_do_hw_update ret %d", ret);
  }

  return ret;
}

/** rccb42_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using awb_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rccb42_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                      ret = TRUE;
  rccb42_t                     *rccb = NULL;
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
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rccb = (rccb42_t *)isp_sub_module->private_data;
  if (!rccb) {
    ISP_ERR("failed: rccb %p", rccb);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      !isp_sub_module->config_pending) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->manual_ctrls.cc_mode ==
          CAM_COLOR_CORRECTION_TRANSFORM_MATRIX &&
      ((isp_sub_module->manual_ctrls.wb_mode == CAM_WB_MODE_OFF &&
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_AUTO) ||
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF)) {
        isp_sub_module->submod_trigger_enable = FALSE;

        if (isp_sub_module->manual_ctrls.update_cc_gain) {
          ret = rccb42_manual_update(isp_sub_module, event);
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

  if ((isp_sub_module->submod_enable == FALSE) ||
      (isp_sub_module->submod_trigger_enable == FALSE) ||
      (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("enable = %d, trigger_enable = %d hw update pending %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable,
      isp_sub_module->trigger_update_pending);
    ret = TRUE;
    goto FILL_METADATA;
  }
  if (!rccb->is_rccb)
    rccb42_update_hw_gain_reg(rccb);
  else
    rccb42_set_unity_gain(rccb);

  ret = rccb42_store_hw_update(isp_sub_module, rccb, output);
  if (ret == FALSE) {
    ISP_ERR("failed: rccb_do_hw_update ret %d", ret);
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output->meta_dump_params &&
    output->metadata_dump_enable == 1) {
    output->meta_dump_params->frame_meta.awb_gain.b_gain =
      rccb->awb_gain.b_gain;
    output->meta_dump_params->frame_meta.awb_gain.r_gain =
      rccb->awb_gain.r_gain;
    output->meta_dump_params->frame_meta.awb_gain.g_gain =
      rccb->awb_gain.g_gain;
  }
  if (output->frame_meta) {
    output->frame_meta->awb_update = rccb->awb_update;
    output->frame_meta->app_ctrl_mode =
      isp_sub_module->manual_ctrls.app_ctrl_mode;
    output->frame_meta->cc_gain.gains[0] = rccb->awb_gain.r_gain;
    output->frame_meta->cc_gain.gains[1] = rccb->awb_gain.g_gain;
    output->frame_meta->cc_gain.gains[2] = rccb->awb_gain.g_gain;
    output->frame_meta->cc_gain.gains[3] = rccb->awb_gain.b_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* rccb42_trigger_update */

/** rccb42_reset:
 *
 *  @mod: Pointer to rccb module
 *
 *  Perform reset of rccb module
 *
 *  Return void
 **/
static void rccb42_reset(rccb42_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->dig_gain = 0.0;
  memset(&mod->r_gain, 0,
    sizeof(ISP_RccbWbGain));
  memset(&mod->b_gain, 0,
    sizeof(ISP_RccbWbGain));
  memset(&mod->g_gain, 0,
    sizeof(ISP_RccbWbGain));
  memset(&mod->r_offset, 0,
    sizeof(ISP_RccbWbOffset));
  memset(&mod->b_offset, 0,
    sizeof(ISP_RccbWbOffset));
  memset(&mod->g_offset, 0,
    sizeof(ISP_RccbWbOffset));
  memset(&mod->awb_gain, 0, sizeof(mod->awb_gain));
}

/** rccb42_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to rccb module
 *  @chroma_ptr: chromatix ptr
 *
 *  Configure the rccb module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean rccb42_config(isp_sub_module_t *isp_sub_module, rccb42_t *mod,
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

  mod->awb_gain.b_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->awb_basic_tuning.awb_adjust_gain_table.
      gain_adj_normallight[AGW_AWB_WARM_FLO].blue_gain_adj;
  mod->awb_gain.r_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->awb_basic_tuning.awb_adjust_gain_table.
      gain_adj_normallight[AGW_AWB_WARM_FLO].red_gain_adj;
  mod->awb_gain.g_gain = chromatix_MWB->MWB_tl84.b_gain;

  mod->dig_gain= 1.0; //no apply aec gain for now, could be used in the future
  if (!mod->is_rccb)
    rccb42_update_hw_gain_reg(mod);
  else
    rccb42_set_unity_gain(mod);
  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  return TRUE;
}

/** rccb42_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rccb42_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  modulesChromatix_t          *module_chromatix_ptrs = NULL;
  rccb42_t                      *mod = NULL;

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

  mod = (rccb42_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *module_chromatix_ptrs;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** rccb42_stats_awb_update:
 *
 *  @module: demosaic module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rccb42_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t                      *stats_update = NULL;
  chromatix_manual_white_balance_type *cur_awb_gain = NULL;
  awb_gain_t                          *new_awb_gain = NULL;
  rccb42_t                              *rccb = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  rccb = (rccb42_t *)isp_sub_module->private_data;
  if (!rccb) {
    ISP_ERR("failed: rccb %p", rccb);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_awb_gain = &rccb->awb_gain;
  new_awb_gain = &stats_update->awb_update.gain;

  if (isp_sub_module->trigger_update_pending == FALSE) {
   if (!WB_GAIN_EQUAL(new_awb_gain, rccb->awb_gain) &&
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
  rccb->awb_update = stats_update->awb_update;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** rccb42_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rccb42_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rccb42_t                  *rccb = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rccb = (rccb42_t *)isp_sub_module->private_data;
  if (!rccb) {
    ISP_ERR("failed: rccb %p", rccb);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  rccb->is_rccb = rccb42_is_rccb_sensor(sensor_out_info->fmt);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* rccb42_set_stream_config */

/** rccb42_streamon:
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
boolean rccb42_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  rccb42_t                *mod = NULL;
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

  mod = (rccb42_t *)isp_sub_module->private_data;
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
    ISP_DBG("rccb enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = rccb42_config(isp_sub_module, mod,
    isp_sub_module->chromatix_ptrs.chromatix3APtr);
  if (ret == FALSE) {
    ISP_ERR("failed: rccb42_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* rccb42_config */

/** rccb42_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rccb42_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  rccb42_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (rccb42_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  rccb42_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* rccb42_streamoff */

/** rccb42_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the rccb module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean rccb42_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  rccb42_t *rccb = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  rccb = (rccb42_t *)CAM_MALLOC(sizeof(rccb42_t));
  if (!rccb) {
    ISP_ERR("failed: rccb %p", rccb);
    return FALSE;
  }

  memset(rccb, 0, sizeof(*rccb));
  rccb42_reset(rccb);

  isp_sub_module->private_data = (void *)rccb;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;

  return TRUE;
}/* rccb42_init */

/** rccb42_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void rccb42_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* rccb42_destroy */
