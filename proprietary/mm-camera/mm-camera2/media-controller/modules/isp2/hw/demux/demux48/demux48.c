/* demux48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "modules.h"

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"
#include "module_demux48.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_DEMUX, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_DEMUX, fmt, ##args)

/* #define DMUX_DEBUG */
#ifdef DMUX_DEBUIG
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#endif

/* TODO: need to clean this, 3D used.*/
#define VFE_CMD_DEMUX_R_CHANNEL_GAIN_CONFIG 200

#define MAX_DEMUX_GAIN 31.9  /* max 32x, should we use 32? */
#define DEMUX_BLK_LVL_MAX ((1<<12)-1)
#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

#define INIT_GAIN(p_gain, val) ( {\
  p_gain.blue = val; \
  p_gain.red = val; \
  p_gain.green_even = val; \
  p_gain.green_odd = val; \
})

/** demux48_debug:
 *    @pcmd: configuration command
 *
 *  This function runs in ISP HW thread context.
 *  This function dumps demux configuration
 *
 *  Return: None
 **/
static void demux48_debug(ISP_DemuxConfigCmdType* pcmd)
{
  ISP_DBG("ISP_DemuxConfigCmd.ch0OddGain = %d",
    pcmd->ch0OddGain);
  ISP_DBG("ISP_DemuxConfigCmd.ch0EvenGain = %d",
    pcmd->ch0EvenGain);
  ISP_DBG("ISP_DemuxConfigCmd.ch1Gain = %d",
    pcmd->ch1Gain);
  ISP_DBG("ISP_DemuxConfigCmd.ch2Gain = %d",
    pcmd->ch2Gain);
  ISP_DBG("ISP_DemuxConfigCmd.period = %d",
    pcmd->period);
  ISP_DBG("ISP_DemuxConfigCmd.blk_in = %d",
    pcmd->blk_in);
  ISP_DBG("ISP_DemuxConfigCmd.blk_out = %d",
    pcmd->blk_out);
  ISP_DBG("ISP_DemuxConfigCmd.evenCfg = %d",
    pcmd->evenCfg);
  ISP_DBG("ISP_DemuxConfigCmd.oddCfg = %d",
    pcmd->oddCfg);
} /* demux48_debug */

/** demux48_gain_debug:
 *    @pcmd: configuration command
 *
 *  This function runs in ISP HW thread context.
 *  This function dumps demux gain configuration
 *
 *  Return: None
 **/
static void demux48_gain_debug(ISP_DemuxGainCfgCmdType* pcmd)
{
  ISP_DBG("ISP_DemuxGainCfgCmdType.ch0OddGain = %d",
    pcmd->ch0OddGain);
  ISP_DBG("ISP_DemuxGainCfgCmdType.ch0EvenGain = %d",
    pcmd->ch0EvenGain);
  ISP_DBG("ISP_DemuxGainCfgCmdType.ch1Gain = %d",
    pcmd->ch1Gain);
  ISP_DBG("ISP_DemuxGainCfgCmdType.ch2Gain = %d",
    pcmd->ch2Gain);
} /* demux48_gain_debug */

/** demux48_stats_aec_update:
 *
 * @mod: demux module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean demux48_set_digital_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demux48_t            *demux = NULL;
  float                 new_dig_gain= 0.0f;
  float                *cur_dig_gain;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  demux = (demux48_t *)isp_sub_module->private_data;
  if (!demux) {
    ISP_ERR("failed: mod %p", demux);
    return FALSE;
  }

  new_dig_gain = *(float *)event->u.module_event.module_event_data;
  ISP_DBG(": dig gain %5.3f", new_dig_gain);
  if (new_dig_gain < 1.0)
    new_dig_gain = 1.0;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_dig_gain = &demux->dig_gain;

  if (isp_sub_module->trigger_update_pending == FALSE) {
    if (!F_EQUAL(*cur_dig_gain, new_dig_gain)) {
      isp_sub_module->trigger_update_pending = TRUE;
    }
  }
  /* Store Digital gain in module private */
  *cur_dig_gain = new_dig_gain;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** demux48_get_global_gain:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  global gain changes
 *
 *  Return none
 **/
static float demux48_get_global_gain(demux48_t *demux,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *output)
{
  float                    global_gain = 0.0f;
  chromatix_parms_type    *chromatix_ptr = NULL;
  exposure_struct_type    *exposure_data = NULL;
  boolean                  ret = TRUE;

  if (!demux|| !isp_sub_module || !output) {
    ISP_ERR("failed: %p %p %p", demux, isp_sub_module, output);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  exposure_data = &chromatix_ptr->exposure_data;

  if (output->algo_params) {
    if (demux->pix_pattern == ISP_BAYER_RGRGRG ||
        demux->pix_pattern == ISP_BAYER_GRGRGR) {
        demux->st_data_r_gain =  output->algo_params->st_gain.r_gain;
        demux->st_data_b_gain = output->algo_params->st_gain.b_gain;
        demux->st_data_geven_gain = output->algo_params->st_gain.gr_gain;
        demux->st_data_godd_gain = output->algo_params->st_gain.gb_gain;
    } else if(demux->pix_pattern == ISP_BAYER_BGBGBG ||
              demux->pix_pattern == ISP_BAYER_GBGBGB) {
        demux->st_data_r_gain =  output->algo_params->st_gain.r_gain;
        demux->st_data_b_gain = output->algo_params->st_gain.b_gain;
        demux->st_data_geven_gain = output->algo_params->st_gain.gb_gain;
        demux->st_data_godd_gain = output->algo_params->st_gain.gr_gain;
    } else {
        /* YUV */
        demux->st_data_r_gain =  0.0f;
        demux->st_data_b_gain = 0.0f;
        demux->st_data_geven_gain = 0.0f;
        demux->st_data_godd_gain = 0.0f;
    }
  }
  ISP_DBG("Stretching Gains r:%f gr:%f gb:%f b:%f", demux->st_data_r_gain,
          demux->st_data_geven_gain, demux->st_data_godd_gain,
          demux->st_data_b_gain);

  global_gain = exposure_data->color_correction_global_gain;
  return global_gain;
}

/** demux48_do_hw_update:
 *    @demux_mod: demux module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   0 - Success
 *           -1 - configuration error
 **/
static int demux48_store_hw_update(isp_sub_module_t *isp_sub_module,
  demux48_t *demux)
{
  boolean ret = TRUE;
  int i, rc = 0;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  ISP_DemuxConfigCmdType        *reg_cmd = NULL;

#ifdef PRINT_REG_VAL_SET
  uint32_t *data = NULL;
  uint32_t len = 0;
#endif

  if (!demux) {
    ISP_ERR("failed: %p", demux);
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
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)));

  reg_cmd = (ISP_DemuxConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));
  *reg_cmd = demux->ISP_DemuxConfigCmd;

  cfg_cmd->cfg_data = (void *) reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DEMUX48_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_DEMUX48_LEN * sizeof(uint32_t);

  demux48_debug(&demux->ISP_DemuxConfigCmd);

#ifdef PRINT_REG_VAL_SET
  ISP_HIGH("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cfg_cmd->cfg_data;
  for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
    ISP_HIGH("data[%d] %x", len, data[len]);
  }
#endif

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_APPEND;
  }

  isp_sub_module->trigger_update_pending = FALSE;
  demux->applied_cmd = demux->ISP_DemuxConfigCmd;

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return ret;
} /* demux48_do_hw_update */

/** demux48_r_image_gain_update:
 *    @demux: demux module instance
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps demux gain configuration
 *
 *  Return: 0
 **/
static int demux48_r_image_gain_update(demux48_t *demux)
{
  ISP_DemuxGainCfgCmdType* p_gaincmd = &demux->ISP_RImageGainConfigCmd;
  p_gaincmd->ch0OddGain = DEMUX_GAIN(demux->r_gain.green_odd);
  p_gaincmd->ch0EvenGain = DEMUX_GAIN(demux->r_gain.green_even);
  p_gaincmd->ch1Gain = DEMUX_GAIN(demux->r_gain.blue);
  p_gaincmd->ch2Gain = DEMUX_GAIN(demux->r_gain.red);

  return 0;
} /* demux48_r_image_gain_update */

/** demux48_set_cfg_params:
 *    @pcmd: configuration commands
 *    @fmt: pixel pattern
 *
 *  This function runs in ISP HW thread context.
 *  This function prepares demux configuration parameters
 *
 *  Return: TRUE  - Success
 *          FALSE - Unsupported pixel pattern
 **/
static boolean demux48_set_cfg_params(ISP_DemuxConfigCmdType* pcmd,
  cam_format_t fmt, enum ISP_START_PIXEL_PATTERN *pix_pattern)
{
  boolean ret = TRUE;

  /* Configure VFE input format, Send Input format command */
  *pix_pattern = isp_sub_module_fmt_to_pix_pattern(fmt);
  ISP_DBG("format %d pix_pattern %d",fmt, *pix_pattern);

  switch (*pix_pattern) {
  /* bayer patterns */
  case ISP_BAYER_GBGBGB:
    pcmd->period = 1;
    pcmd->evenCfg = 0xAC;
    pcmd->oddCfg = 0xC9;
    break;

  case ISP_BAYER_BGBGBG:
    pcmd->period = 1;
    pcmd->evenCfg = 0xCA;
    pcmd->oddCfg = 0x9C;
    break;

  case ISP_BAYER_GRGRGR:
    pcmd->period = 1;
    pcmd->evenCfg = 0x9C;
    pcmd->oddCfg = 0xCA;
    break;

  case ISP_BAYER_RGRGRG:
    pcmd->period = 1;
    pcmd->evenCfg = 0xC9;
    pcmd->oddCfg = 0xAC;
    break;

    /* YCbCr Patterns */
  case ISP_YUV_YCbYCr:
    pcmd->period = 3;
    pcmd->evenCfg = 0x9CAC;
    pcmd->oddCfg = 0x9CAC;
    break;

  case ISP_YUV_YCrYCb:
    pcmd->period = 3;
    pcmd->evenCfg = 0xAC9C;
    pcmd->oddCfg = 0xAC9C;
    break;

  case ISP_YUV_CbYCrY:
    pcmd->period = 3;
    pcmd->evenCfg =0xC9CA;
    pcmd->oddCfg = 0xC9CA;
    break;

  case ISP_YUV_CrYCbY:
    pcmd->period = 3;
    pcmd->evenCfg =0xCAC9;
    pcmd->oddCfg =0xCAC9;
    break;

  default:
    ISP_ERR("Error ISP input not configured!!!");
    ret = FALSE;
    break;
  }

  return ret;
}/* demux48_set_cfg_params */

static void demux48_ez_isp_update(demux48_t *demux_module,
  demuxchannelgain_t *gain)
{
  ISP_DemuxConfigCmdType *demuxchgainCfg;
  demuxchgainCfg = &(demux_module->applied_cmd);
  gain->greenEvenRow = demuxchgainCfg->ch0EvenGain;
  gain->greenOddRow = demuxchgainCfg->ch0OddGain;
  gain->blue = demuxchgainCfg->ch1Gain;
  gain->red = demuxchgainCfg->ch2Gain;
}

/** demux48_fill_vfe_diag_data:
 *
 *  @mod: demux module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean demux48_fill_vfe_diag_data(demux48_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  demuxchannelgain_t  *gain= NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    gain = &vfe_diag->prev_demuxchannelgain;
    demux48_ez_isp_update(mod, gain);
  }
  return ret;
}

/** demux48_fetch_demux_gain:
 *
 *  @module: mct module
 *  @event: mct event
 *  @isp_sub_module: isp_sub_module_t
 *  @isp_sub_module:
 *  This function Fills the demux gain
 *  Return: TRUE success
 */
boolean demux48_fetch_demux_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demux48_t                              *demux = NULL;
  chromatix_channel_balance_gains_type   *gain = NULL;
  isp_private_event_t                    *private_event = NULL;

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

  demux = (demux48_t *)isp_sub_module->private_data;
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;

  if (private_event->data_size != sizeof(chromatix_channel_balance_gains_type)) {
    /* size mismatch */
    ISP_ERR("size mismatch, expecting = %zu, received = %u",
      sizeof(chromatix_channel_balance_gains_type), private_event->data_size);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  gain = (chromatix_channel_balance_gains_type *)private_event->data;
  if (!gain) {
    ISP_ERR("failed: gain %p", gain);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  *gain = demux->gain;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** demux48_fetch_blkLvl_offset:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Black Level offset
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean demux48_fetch_blkLvl_offset(mct_module_t *module,
  uint32_t identity, uint32_t *BlkLvl_offset, float multifactor)
{
  boolean                               ret = TRUE;
  mct_event_t                           mct_event;
  isp_private_event_t                   private_event;
  uint32_t                              blklvl;
  float                                 f_blklvl;

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
  f_blklvl = (float)blklvl * multifactor;
  f_blklvl = Round(f_blklvl);
  blklvl = CLAMP(f_blklvl, 0, DEMUX_BLK_LVL_MAX);
  *BlkLvl_offset = blklvl;
  ISP_DBG("BLSS offset:%d multifactor:%f blk_demux:%f", *BlkLvl_offset,
    multifactor, f_blklvl);
  return ret;
}

/** demux48_trigger_update:
 *    @demux_mod: demux module instance
 *    @trigger_params: module trigger update params
 *    @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
boolean demux48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                               ret = TRUE;
  isp_private_event_t                  *private_event = NULL;
  isp_sub_module_priv_t                *isp_sub_module_priv = NULL;
  float                                 max_gain_final = 0.0;
  float                                 cc_global_gain = 0.0;
  float                                 gain = 0.0;
  float                                 new_dig_gain = 1.0;
  float                                 max_ch_gain = 0.0;
  demux48_t                            *demux = NULL;
  ISP_DemuxConfigCmdType               *p_cmd = NULL;
  isp_sub_module_output_t              *output = NULL;
  chromatix_parms_type                 *chromatix_ptr = NULL;
  chromatix_channel_balance_gains_type *chromatix_channel_balance_gains = NULL;
  chromatix_videoHDR_type              *chromatix_VHDR = NULL;
  demux_info_t                         *demux_info = NULL;
  uint32_t                              BlkLvl_offset = 0;
  float                                 multifactor = 1.0f;
  boolean                               update_demux = FALSE;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  demux = (demux48_t *)isp_sub_module->private_data;
  if (!demux) {
    ISP_ERR("failed: mod %p", demux);
    return FALSE;
  }
  p_cmd = &demux->ISP_DemuxConfigCmd;

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

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
  if (!chromatix_VHDR) {
    ISP_ERR("failed: chromatix_VHDR %p", chromatix_VHDR);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((isp_sub_module->submod_enable == FALSE) ||
      (isp_sub_module->submod_trigger_enable == FALSE)) {
    ISP_DBG("No trigger update for demux: enable = %d, trigger_en = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);

    ret = TRUE;
    goto FILL_METADATA;
  }

  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
    chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    multifactor = NON_HDR_MUL_FACTOR;
  }

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) {
    multifactor = 1.0f/(float)(1 << ISP_CHROMATIX_BIT_DIFF);
  } else if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    multifactor = NON_HDR_MUL_FACTOR;
  }

  ret = demux48_fetch_blkLvl_offset(module, event->identity,
    &BlkLvl_offset, multifactor);
  if (demux->blk_lvl_offset != BlkLvl_offset) {
    demux->blk_lvl_offset = BlkLvl_offset;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  /* set to unitity all frame before calculation*/
  demux->remaining_digital_gain = 1.0;
  chromatix_channel_balance_gains =
    &chromatix_ptr->chromatix_VFE.chromatix_channel_balance_gains;
  cc_global_gain = demux48_get_global_gain(
    demux, isp_sub_module, output);

  max_ch_gain = MAX(chromatix_channel_balance_gains->green_odd,
    MAX(chromatix_channel_balance_gains->green_even,
    MAX(chromatix_channel_balance_gains->red,
    chromatix_channel_balance_gains->blue)));


  if (isp_sub_module->manual_ctrls.manual_update &&
      (isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF ||
       isp_sub_module->manual_ctrls.aec_ctrl_mode == CAM_AE_MODE_OFF) &&
      isp_sub_module->manual_ctrls.isp_gain > 0.0f){
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    max_gain_final = (max_ch_gain *
      isp_sub_module->manual_ctrls.isp_gain * cc_global_gain);
    update_demux = TRUE;
  } else if (isp_sub_module->trigger_update_pending == TRUE) {
    max_gain_final = (max_ch_gain * demux->dig_gain * cc_global_gain);
    update_demux = TRUE;
  }

  if (update_demux) {
    if (max_gain_final > MAX_DEMUX_GAIN) {
      demux->remaining_digital_gain = max_gain_final / MAX_DEMUX_GAIN;
      max_gain_final = MAX_DEMUX_GAIN;
    }

    if (cc_global_gain && max_ch_gain) {
      new_dig_gain = (max_gain_final / (cc_global_gain * max_ch_gain));
      demux->applied_dig_gain = new_dig_gain;
    } else {
      ISP_ERR("invalid gain value %f %f, devide by 0!",
        cc_global_gain, max_ch_gain);
      goto FILL_METADATA;
    }

    gain = cc_global_gain * new_dig_gain;
    demux->gain.green_odd =
      gain * chromatix_channel_balance_gains->green_odd * demux->st_data_godd_gain;
    demux->gain.green_even =
      gain * chromatix_channel_balance_gains->green_even * demux->st_data_geven_gain;
    demux->gain.red =
      gain * chromatix_channel_balance_gains->red * demux->st_data_r_gain;
    demux->gain.blue =
      gain * chromatix_channel_balance_gains->blue * demux->st_data_b_gain;

    p_cmd->ch0EvenGain = DEMUX_GAIN(demux->gain.green_even);
    p_cmd->ch0OddGain = DEMUX_GAIN(demux->gain.green_odd);
    p_cmd->ch1Gain = DEMUX_GAIN(demux->gain.blue);
    p_cmd->ch2Gain = DEMUX_GAIN(demux->gain.red);
    /* The black level in/out offsets have same value */
    p_cmd->blk_in = demux->blk_lvl_offset;
    p_cmd->blk_out = demux->blk_lvl_offset;

    ISP_DBG("max_ch_gain %5.3f glob %5.3f max_gain_final %5.3f",
      max_ch_gain, cc_global_gain, max_gain_final);
    ISP_DBG("gain: gr_odd = %5.2f, gr_even = %5.2f, red = %5.2f blue = %5.2f blk:%d",
      demux->gain.green_odd, demux->gain.green_even, demux->gain.red,
      demux->gain.blue, demux->blk_lvl_offset);

    /* 3D is not supported. */
    if (demux->is_3d)
      demux48_r_image_gain_update(demux);
    ret = demux48_store_hw_update(isp_sub_module, demux);
    if (ret == FALSE) {
      ISP_ERR("failed: demux48_store_hw_update");
    }
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

FILL_METADATA:
  demux_info = &(output->stats_params->be_info.demux_info);
  demux_info->gain_g_odd = demux->gain.green_odd;
  demux_info->gain_g_even = demux->gain.green_even;
  demux_info->gain_b = demux->gain.blue;
  demux_info->gain_r = demux->gain.red;
  demux_info->sensor_format = demux->fmt;

  if (output && isp_sub_module->vfe_diag_enable) {
    demux48_fill_vfe_diag_data(demux, isp_sub_module, output);
  }
  output->frame_meta->isp_gain =
      (int32_t)(demux->applied_dig_gain * MULTIPLY_FACTOR_FOR_ISO);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}/*demux48_trigger_update*/

/** demux48_config:
 *    @demux: demux module instance
 *    @in_params: configuration parameters
 *    @size: configuration parameters size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial configuration of demux module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
static boolean demux48_config(isp_sub_module_t *isp_sub_module,
  demux48_t *demux, chromatix_parms_type *chromatix_ptr)
{
  boolean                         ret = TRUE;
  boolean                         is_bayer_sensor = TRUE;
  ISP_DemuxConfigCmdType         *p_cmd  = &demux->ISP_DemuxConfigCmd;
  float                           color_correct_global_gain = 0.0;

  if (!isp_sub_module || !demux || !chromatix_ptr) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, demux, chromatix_ptr);
    return FALSE;
  }

  color_correct_global_gain =
    chromatix_ptr->exposure_data.color_correction_global_gain;

  is_bayer_sensor = isp_sub_module_is_bayer_mono_fmt(demux->fmt);

  if (is_bayer_sensor) {
    INIT_GAIN(demux->gain, color_correct_global_gain);
  } else {
    INIT_GAIN(demux->gain, 1.0);
  }

  ret = demux48_set_cfg_params(p_cmd, demux->fmt, &demux->pix_pattern);
  if (ret == FALSE) {
    ISP_ERR("failed: to configure demux");
    return ret;
  }

  p_cmd->ch0EvenGain = DEMUX_GAIN(demux->gain.green_even);
  p_cmd->ch0OddGain = DEMUX_GAIN(demux->gain.green_odd);
  p_cmd->ch1Gain = DEMUX_GAIN(demux->gain.blue);
  p_cmd->ch2Gain = DEMUX_GAIN(demux->gain.red);
  p_cmd->blk_in = demux->blk_lvl_offset;
  p_cmd->blk_out = demux->blk_lvl_offset;

  /* 3D is not supported. */
  if (demux->is_3d) {
    demux48_r_image_gain_update(demux);
  }
  isp_sub_module->trigger_update_pending = TRUE;

  ISP_DBG("sensor input format : %d ", demux->fmt);
  ISP_DBG("gain: gr_odd = %5.2f, gr_even = %5.2f, red = %5.2f blue = %5.2f blk:%d",
    demux->gain.green_odd, demux->gain.green_even, demux->gain.red,
    demux->gain.blue, demux->blk_lvl_offset);

  return ret;
} /* demux48_config */

/** demux48_set_stream_config:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store sensor fmt in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demux48_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  sensor_out_info_t *sensor_out_info = NULL;
  demux48_t         *demux = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }

  demux = (demux48_t *)isp_sub_module->private_data;
  if (!demux) {
    ISP_ERR("failed: demux %p", demux);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  demux->fmt = sensor_out_info->fmt;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* demux48_set_stream_config */

/** demux_reset:
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets demux module
 *
 *  Return: None
 **/
boolean demux48_reset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demux48_t     *demux = NULL;
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  demux = (demux48_t *)isp_sub_module->private_data;
  if (!demux) {
    ISP_ERR("failed: mod %p", demux);
    return FALSE;
  }

  memset(&demux->ISP_RImageGainConfigCmd, 0,
    sizeof(demux->ISP_RImageGainConfigCmd));
  memset(&demux->ISP_DemuxConfigCmd, 0, sizeof(demux->ISP_DemuxConfigCmd));
  memset(&demux->gain, 0, sizeof(demux->gain));
  memset(&demux->r_gain, 0, sizeof(demux->r_gain));
  demux->trigger_enable = 0;
  demux->dig_gain = 1.0;
  demux->is_3d =0;
  demux->remaining_digital_gain = 0.0;
  demux->blk_lvl_offset = 0;
  demux->st_data_r_gain = 1.0f;
  demux->st_data_geven_gain = 1.0f;
  demux->st_data_godd_gain = 1.0f;
  demux->st_data_b_gain = 1.0f;
  demux->applied_dig_gain = 1.0;
  return TRUE;
} /* demux_reset */

/** demux48_streamon:
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
boolean demux48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  demux48_t                *mod = NULL;
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

  mod = (demux48_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  ISP_DBG("%s: Demux stream_on_count %d", __func__, isp_sub_module->stream_on_count);
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("demux enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ISP_DBG("%s: config demux\n", __func__);
  ret = demux48_config(isp_sub_module, mod,
    isp_sub_module->chromatix_ptrs.chromatixPtr);
  if (ret == FALSE) {
    ISP_ERR("failed: demux48_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* demux48_config */

/** demux48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demux48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demux48_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (demux48_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ISP_DBG("%s: Demux stream_on_count %d", __func__,
           isp_sub_module->stream_on_count);

  if (isp_sub_module->stream_on_count == 0 ||
      --isp_sub_module->stream_on_count > 0 ) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ISP_DBG("%s: reset demux\n", __func__);

  /* reset stream on count */
  isp_sub_module->stream_on_count = 0;

  demux48_reset(module, isp_sub_module, event);
  isp_sub_module->trigger_update_pending = FALSE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* demux48_streamoff */

/** demux48_query_cap:
 *
 *  @module: mct module handle
 *
 *  @query_buf: buffer pointer to fill query caps
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean demux48_query_cap(mct_module_t *module, void *query_buf)
{
  mct_pipeline_cap_t *cap_buf;
  mct_pipeline_isp_cap_t *mod_cap = NULL;

  if (!module || !query_buf) {
    ISP_ERR("failed: %p, %p", module, query_buf);
    return FALSE;
  }
  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->isp_sensitivity_range.min_sensitivity = 1 * MULTIPLY_FACTOR_FOR_ISO;
  /* Max gain 31.99902343, 15 bit and Q10 value(2^15 -1)/1024
    round(31.99902343 * 100) -> 3200 */

  mod_cap->isp_sensitivity_range.max_sensitivity = 3200;
  return TRUE;
}

/** demux_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the demux module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean demux48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  demux48_t *demux = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  demux = (demux48_t *)CAM_MALLOC(sizeof(demux48_t));
  if (!demux) {
    ISP_ERR("failed: demux %p", demux);
    return FALSE;
  }

  memset(demux, 0, sizeof(*demux));
  demux->fmt = CAM_FORMAT_MAX;
  demux->dig_gain = 1.0;
  demux->remaining_digital_gain = 1.0;
  demux->applied_dig_gain = 1.0;
  INIT_GAIN(demux->r_gain, 1.0);

  isp_sub_module->private_data = (void *)demux;
  return TRUE;
}/* demux_init */

/** demux48_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void demux48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* demux48_destroy */

