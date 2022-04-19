/* black_level47.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "black_level47.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BLSS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BLSS, fmt, ##args)

#define CLAMP_BLK_LVL(x1,v1) ((x1 > v1) ? v1 : x1 )
#define SET_FIRST_N_BITS(N) ((1 << N) - 1)

/** black_level47_cmd_debug:
 *
 *  @cmd: black_level config cmd
 *
 *  This function dumps the black_level module configuration set to hw
 *
 *  Returns nothing
 **/
static void black_level47_cmd_debug(ISP_black_level_reg_t *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG("cmd->black_level_cfg.offset = %d", cmd->black_level_cfg.offset);
  ISP_DBG("cmd->black_level_cfg.scale = %d", cmd->black_level_cfg.scale);
  ISP_DBG("cmd->black_level_stats_cfg.offset %d", cmd->black_level_stats_cfg.offset);
  ISP_DBG("cmd->black_level_stats_cfg.scale = %d"
          ,cmd->black_level_stats_cfg.scale);
} /* black_level47_cmd_debug */

/** black_level47_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @black_level: black_level struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean black_level47_store_hw_update(isp_sub_module_t *isp_sub_module,
  black_level47_t *black_level)
{

  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_black_level_reg_t       *RegCmd = NULL;

  if (!isp_sub_module || !black_level) {
    ISP_ERR("failed: %p %p", isp_sub_module, black_level);
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
    * ISP_BLACK_LEVEL_NUM_REG);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * ISP_BLACK_LEVEL_NUM_REG);

  RegCmd = (ISP_black_level_reg_t *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BLACK_LEVEL_CMD;
  }
  *RegCmd = black_level->Reg;
  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  /* write ISP_Black_LevelCfg register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[0], 0,
    sizeof(RegCmd->black_level_cfg), VFE_WRITE, ISP_BLACK_LEVEL_CFG_OFF);

  /* write ISP_Black_Level_StatsCfg register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1],
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len,
    sizeof(RegCmd->black_level_stats_cfg), VFE_WRITE,
    ISP_STATS_BLACK_LEVEL_CFG_OFF);

  black_level47_cmd_debug(RegCmd);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(RegCmd);
ERROR_BLACK_LEVEL_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* black_level47_do_hw_update */

/** black_level47_hw_config:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @black_level: black lavel modue data
 *
 *  Write the Registers with data from aec update after
 *  interpolation
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean black_level47_hw_config(isp_sub_module_t *isp_sub_mod,
  black_level47_t *black_level)
{
  chromatix_VFE_common_type             *chromatix_common = NULL;
  chromatix_parms_type                  *chromatix_ptr = NULL;
  Chromatix_blk_subtract_scale_type     *chromatix_BLSS = NULL;
  chromatix_videoHDR_type               *chromatix_VHDR = NULL;
  float                                  aec_ratio = 0, BLSS_offset = 0;
  uint32_t                               BLSS_offset_lowlight = 0,
                                         BLSS_offset_normal = 0;
  float                                  multifactor = 0.0f;
  boolean                                lsb_allignment = FALSE;

  if (!isp_sub_mod || !black_level) {
    ISP_ERR("failed isp_sub_mod %p black_level %p", isp_sub_mod, black_level);
    return FALSE;
  }

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_mod->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_mod->chromatix_ptrs.chromatixPtr;
  if (!chromatix_common || !chromatix_ptr) {
    ISP_ERR("failed: chromatix_common %p chromatix_ptr %p",
      chromatix_common, chromatix_ptr);
    return FALSE;
  }

  chromatix_BLSS = &chromatix_common->Chromatix_BLSS_data;
  BLSS_offset_lowlight =
    chromatix_BLSS->black_level_lowlight.black_level_offset;
  BLSS_offset_normal = chromatix_BLSS->black_level_normal.black_level_offset;
  aec_ratio = black_level->aec_ratio;

  BLSS_offset = (float)LINEAR_INTERPOLATION((float)BLSS_offset_normal,
    (float)BLSS_offset_lowlight, aec_ratio);

  BLSS_offset *= black_level->gain.green_even;
  /*when HDR mode, adjust the multilplication factor*/
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    multifactor = 1.0f * (float)(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  } else {
    multifactor = 1;
  }
  if (isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }

  if (lsb_allignment){
    multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  }

  /* Stats BLSS block is always non-HDR so need to multiply by 4 always */
  black_level->Reg.black_level_stats_cfg.offset =
    CLAMP_BLK_LVL(Round(BLSS_offset * NON_HDR_MUL_FACTOR), SET_FIRST_N_BITS(10));
  black_level->Reg.black_level_cfg.offset =
    CLAMP_BLK_LVL(Round(BLSS_offset * multifactor), SET_FIRST_N_BITS(10));
  black_level->Reg.black_level_cfg.scale =
    CLAMP_BLK_LVL(FLOAT_TO_Q(11, 1),
    SET_FIRST_N_BITS(12));
  black_level->Reg.black_level_stats_cfg.scale =
    black_level->Reg.black_level_cfg.scale;

  return TRUE;
}/* end black_level47_hw_config*/

/** black_level47_fetch_demux_gain:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Demux gain
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean black_level47_fetch_demux_gain(mct_module_t *module,
  uint32_t identity, chromatix_channel_balance_gains_type *gain)
{
  boolean                               ret = TRUE;
  mct_event_t                           mct_event;
  isp_private_event_t                   private_event;
  chromatix_channel_balance_gains_type  cb_gain;

  if (!module || !gain) {
    ISP_ERR("failed: %p %p", module, gain);
    return FALSE;
  }

  /*Get demux gain info */
  ISP_DBG("E: Get demux gain info");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_DEMUX_GAIN;
  private_event.data = (void *)&cb_gain;
  private_event.data_size = sizeof(chromatix_channel_balance_gains_type);

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
  *gain = cb_gain;
  ISP_DBG("Demux green even %f", gain->green_even);
  return ret;
}

/** black_level47_fetch_blklvl_offset:
 *
 *  @module: mct module
 *  @event: mct event
 *  @isp_sub_module: isp_sub_module_t
 *  @isp_sub_module:
 *  This function Fills the blacklevel offset
 *  Return: TRUE on success
 */
boolean black_level47_fetch_blklvl_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  black_level47_t            *blacklvl = NULL;
  uint32_t                   *blklvl_offset = NULL;
  isp_private_event_t        *private_event = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (!isp_sub_module->private_data ||
      !event->u.module_event.module_event_data) {
    ISP_ERR("failed: mod %p module event data %p", isp_sub_module->private_data,
      event->u.module_event.module_event_data);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  blacklvl = (black_level47_t *)isp_sub_module->private_data;
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;

  if (private_event->data_size != sizeof(uint32_t)) {
    /* size mismatch */
    ISP_ERR("size mismatch, expecting = %zu, received = %u",
      sizeof(uint32_t), private_event->data_size);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  blklvl_offset = (uint32_t *)private_event->data;
  if (!blklvl_offset) {
    ISP_ERR("failed: offset %p", blklvl_offset);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  *blklvl_offset = blacklvl->Reg.black_level_cfg.offset;
  ISP_DBG("offset %d", *blklvl_offset);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}
/** black_level47_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean black_level47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  black_level47_t              *black_level = NULL;
  isp_sub_module_output_t      *output = NULL;
  isp_private_event_t          *private_event = NULL;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  private_event = event->u.module_event.module_event_data;
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

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    goto ERROR;
  }

  /* TODO: Need to propagate black level lock to meta data 'every frame' */
  if (isp_sub_module->manual_ctrls.manual_update &&
      !isp_sub_module->config_pending) {
    isp_sub_module->submod_trigger_enable =
      (isp_sub_module->manual_ctrls.black_level_lock ==
      CAM_BLACK_LEVEL_LOCK_ON)? FALSE: TRUE;
    isp_sub_module->manual_ctrls.manual_update = FALSE;
  }

  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger update enable %d, trig_enable %d\n",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  memset(&black_level->gain, 0, sizeof(black_level->gain));

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ret = black_level47_fetch_demux_gain(module, event->identity,
      &black_level->gain);
    if (ret == FALSE) {
      ISP_ERR("failed: black_level47_fetch_demux_gain");
      goto ERROR;
    }
    ret = black_level47_hw_config(isp_sub_module, black_level);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc47_config");
      goto ERROR;
    }
    isp_sub_module->trigger_update_pending = FALSE;
    black_level47_store_hw_update(isp_sub_module, black_level);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:
  if (output->frame_meta)
    output->frame_meta->black_level_lock =
      isp_sub_module->manual_ctrls.black_level_lock;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** black_level47_manual_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to aec_manual_update_t
 *
 * Handle manual AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level47_manual_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_manual_update_t                  *manual_stats_update = NULL;
  black_level47_t                      *black_level = NULL;
  float                                 aec_ratio = 0;
  chromatix_VFE_common_type            *chromatix_common = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_BLSS = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  manual_stats_update = (aec_manual_update_t *)data;
  if (!manual_stats_update) {
    ISP_ERR("failed: manual_stats_update %p", manual_stats_update);
    return FALSE;
  }

  black_level->aec_update.sensor_gain = manual_stats_update->sensor_gain;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_common) {
    ISP_ERR("failed: chromatix_common %p", chromatix_common);
    goto ERROR;
  }

  chromatix_BLSS = &chromatix_common->Chromatix_BLSS_data;

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_BLSS->control_BLSS, &chromatix_BLSS->BLSS_low_light_trigger,
    &black_level->aec_update, 1);

  if (!F_EQUAL(black_level->aec_ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
    black_level->aec_ratio = aec_ratio;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}


/** black_level47_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level47_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t                       *stats_update = NULL;
  black_level47_t                      *black_level = NULL;
  float                                 aec_ratio = 0;
  chromatix_VFE_common_type            *chromatix_common = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_BLSS = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_common) {
    ISP_ERR("failed: chromatix_common %p", chromatix_common);
    goto ERROR;
  }

  chromatix_BLSS = &chromatix_common->Chromatix_BLSS_data;

  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_BLSS->control_BLSS, &chromatix_BLSS->BLSS_low_light_trigger,
    &stats_update->aec_update, 1);

  if (!F_EQUAL(black_level->aec_ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
    black_level->aec_ratio = aec_ratio;
    black_level->aec_update = stats_update->aec_update;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** black_level47_init_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @black_level: black_level data instance
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean black_level47_init_config(isp_sub_module_t *isp_sub_module,
  black_level47_t *black_level)
{
  ISP_black_level_reg_t                   *Reg = NULL;
  chromatix_parms_type                    *chromatix_ptr = NULL;
  chromatix_VFE_common_type               *chromatix_common = NULL;
  Chromatix_blk_subtract_scale_type       *chromatix_BLSS = NULL;
  chromatix_videoHDR_type                 *chromatix_VHDR = NULL;
  int                                      multifactor = 0;
  boolean                                  is_lsb_alligned = FALSE;

  if (!isp_sub_module || !black_level) {
    ISP_ERR("failed: %p %p", isp_sub_module, black_level);
    return FALSE;
  }
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_ptr || !chromatix_common) {
    ISP_ERR("failed: chromatix_ptr %p chromatix_common %p", chromatix_ptr,
      chromatix_common);
    return FALSE;
  }

  chromatix_BLSS = &chromatix_common->Chromatix_BLSS_data;
  if (chromatix_BLSS->BLSS_enable) {
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->update_module_bit = TRUE;
  } else {
    isp_sub_module->submod_enable = FALSE;
    isp_sub_module->update_module_bit = FALSE;
  }
  /*when HDR mode, adjest the multilplication factor*/
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  } else {
    multifactor = 0;
  }
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
    is_lsb_alligned = TRUE;
  }
  Reg = &black_level->Reg;
  /* initialize default values from Lowlight */
  if (is_lsb_alligned){
    Reg->black_level_cfg.offset =
      CLAMP_BLK_LVL(chromatix_BLSS->black_level_lowlight.black_level_offset >>
      multifactor, SET_FIRST_N_BITS(10));
  } else {
    Reg->black_level_cfg.offset =
      CLAMP_BLK_LVL(chromatix_BLSS->black_level_lowlight.black_level_offset <<
      multifactor, SET_FIRST_N_BITS(10));
  }
  /* Stats BLSS block is always non-HDR so need to multiply by 4 always */
  Reg->black_level_stats_cfg.offset =
    CLAMP_BLK_LVL(chromatix_BLSS->black_level_lowlight.black_level_offset *
    NON_HDR_MUL_FACTOR, SET_FIRST_N_BITS(10));
  /* initialize scale to 1 */
  Reg->black_level_cfg.scale = CLAMP_BLK_LVL(FLOAT_TO_Q(11, 1),
    SET_FIRST_N_BITS(12));

  return TRUE;
}

/** black_level47_set_digital_gain:
 *
 * @mod: black_level module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level47_set_digital_gain(isp_sub_module_t *isp_sub_module,
  float *new_dig_gain)
{
  black_level47_t      *black_level = NULL;
  float                *cur_dig_gain;

  if (!new_dig_gain || !isp_sub_module) {
    ISP_ERR("failed: %p %p", new_dig_gain, isp_sub_module);
    return FALSE;
  }

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: mod %p", black_level);
    return FALSE;
  }

  ISP_DBG(": dig gain %5.3f", *new_dig_gain);
  if (*new_dig_gain < 1.0)
    *new_dig_gain = 1.0;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_dig_gain = &black_level->dig_gain;

  if (isp_sub_module->trigger_update_pending == FALSE) {
    if (!F_EQUAL(*cur_dig_gain, *new_dig_gain)) {
      isp_sub_module->trigger_update_pending = TRUE;
    }
  }
  /* Store Digital gain in module private */
  *cur_dig_gain = *new_dig_gain;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** black_level47_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean black_level47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                             ret = TRUE;
  black_level47_t                    *black_level = NULL;
  modulesChromatix_t                 *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: mod %p", black_level);
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

  ret = black_level47_init_config(isp_sub_module, black_level);
  if (ret == FALSE) {
    ISP_ERR("failed: black_level47_init_config ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return ret;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* black_level47_set_chromatix_ptr */

/** black_level47_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean black_level47_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  black_level47_t *black_level = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level47_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count > 0) {
    if (--isp_sub_module->stream_on_count) {
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return TRUE;
    }
  } else {
    ISP_ERR("failed Stream on count %u", isp_sub_module->stream_on_count);
  }

  memset(black_level, 0, sizeof(*black_level));
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* black_level47_streamoff */

/** black_level47_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the black_level module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean black_level47_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  black_level47_t *black_level = NULL;

  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p module %p", isp_sub_module, module);
    return FALSE;
  }

  black_level = (black_level47_t *)CAM_MALLOC(sizeof(black_level47_t));
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  memset(black_level, 0, sizeof(*black_level));

  isp_sub_module->private_data = (void *)black_level;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  return TRUE;
}/* black_level47_init */

/** black_level47_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void black_level47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p module %p", isp_sub_module, module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* black_level47_destroy */
