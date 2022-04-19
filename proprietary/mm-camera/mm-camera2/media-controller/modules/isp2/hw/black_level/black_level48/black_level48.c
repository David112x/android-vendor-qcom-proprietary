/* black_level48.c
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "black_level48.h"


#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BLSS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BLSS, fmt, ##args)



/** black_level48_cmd_debug:
 *
 *  @cmd: black_level config cmd
 *
 *  This function dumps the black_level module configuration set to hw
 *
 *  Returns nothing
 **/
static void black_level48_cmd_debug(ISP_black_level_reg_t *cmd)
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
} /* black_level48_cmd_debug */

/** black_level48_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @black_level: black_level struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean black_level48_store_hw_update(isp_sub_module_t *isp_sub_module,
  black_level48_t *black_level)
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

  black_level48_cmd_debug(RegCmd);

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
} /* black_level48_do_hw_update */

#if !OVERRIDE_FUNC
/** black_level48_update_params:
 *  @black_level: black level related info
 *
 *  This function checks for HDR status and computes
 *  the multiplication factors. The black level
 *  data is updated accordingly.
 **/
void black_level48_update_params(void *data1,
  float BLSS_offset, int BLSS_is_v2, cam_sensor_hdr_type_t hdr_mode)
{
  black_level48_t *black_level = NULL;
  float multifactor = 1.0f, stats_multifactor = 1.0f;
  float scale = 1.0f;

  black_level = (black_level48_t *)data1;

  if (BLSS_is_v2) {
    switch (hdr_mode) {
    case CAM_SENSOR_HDR_IN_SENSOR:
      multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - HDR_OUTPUT_BIT_WIDTH));
      stats_multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - HDR_OUTPUT_BIT_WIDTH));
      break;
    case CAM_SENSOR_HDR_ZIGZAG:
      /* For HDR convert 14bit to 10bit */
      multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - HDR_OUTPUT_BIT_WIDTH));
      stats_multifactor = 1.0f;
      break;
    case CAM_SENSOR_HDR_OFF:
    default:
    /* For Non-HDR mode, v2 version, the chromatix is already 14 bit */
       multifactor = 1.0f;
       stats_multifactor = 1.0f;
      break;
     }
     // 14 bit to 12 bit offset for rest of the modules
     black_level->pipe_cfg_blss_offset = ((Round(BLSS_offset)+
       ((1 << ISP_CHROMATIX_BIT_DIFF) - 1)) >> ISP_CHROMATIX_BIT_DIFF);
  } else {
    /* when HDR mode, adjust the multilplication factor
       otherwise keep 12 bit header for backward compatible.
       Stats BLSS block is always non-HDR */
    switch (hdr_mode) {
    case CAM_SENSOR_HDR_IN_SENSOR:
      multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
      stats_multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
      break;
    case CAM_SENSOR_HDR_ZIGZAG:
      multifactor = 1.0f/(float)(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
      stats_multifactor = NON_HDR_MUL_FACTOR;
      break;
    case CAM_SENSOR_HDR_OFF:
    default:
      multifactor = NON_HDR_MUL_FACTOR;
      stats_multifactor = NON_HDR_MUL_FACTOR;
      break;
    }
    /* BLSS offset for rest of the modules */
    black_level->pipe_cfg_blss_offset = Round(BLSS_offset);
  }

  /* Stats BLSS block is always non-HDR */
  black_level->Reg.black_level_cfg.offset =
    CLAMP(Round(multifactor * BLSS_offset), 0, SET_FIRST_N_BITS(12));
  black_level->Reg.black_level_stats_cfg.offset =
    CLAMP(Round(stats_multifactor * BLSS_offset), 0, SET_FIRST_N_BITS(12));
  scale = (float)(1<<ISP_PIPELINE_WIDTH)/((float)((1 << ISP_PIPELINE_WIDTH)
    - black_level->Reg.black_level_cfg.offset));
  if(scale >= 2.0f) {
    ISP_ERR("Scale in BLSS can't be more then 2.0f");
    scale = 1.9999f;
  }
  black_level->Reg.black_level_cfg.scale =
    CLAMP(FLOAT_TO_Q(11, scale), 0, SET_FIRST_N_BITS(12));
  black_level->Reg.black_level_stats_cfg.scale =
    black_level->Reg.black_level_cfg.scale;
}
#endif

#if !OVERRIDE_FUNC
/** black_level48_hw_config:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @black_level: black lavel modue data
 *
 *  Write the Registers with data from aec update after
 *  interpolation
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean black_level48_hw_config(void *data1,
  void *data2)
{
  isp_sub_module_t *isp_sub_mod;
  black_level48_t *black_level;
  chromatix_VFE_common_type             *chromatix_common = NULL;
  chromatix_parms_type                  *chromatix_ptr = NULL;
  Chromatix_blk_subtract_scale_type     *chromatix_BLSS = NULL;
  Chromatix_blk_subtract_scale_v2_type  *chromatix_BLSS_v2 = NULL;
  chromatix_videoHDR_type               *chromatix_VHDR = NULL;
  float                                  aec_ratio = 1.0, BLSS_offset = 0.0;
  uint32_t                               BLSS_offset_lowlight = 0,
                                         BLSS_offset_normal = 0;
  trigger_point_type                    *BLSS_trigger = NULL;
  uint8_t                                trigger_index = NUM_BLK_REGIONS + 1;
  float                                  aec_reference = 0.0;
  float                                  start = 0.0;
  float                                  end   = 0.0;
  int                                    i     = 0;

  if (!data1 || !data2) {
    ISP_ERR("failed isp_sub_mod %p black_level %p", data1, data2);
    return FALSE;
  }

  isp_sub_mod = (isp_sub_module_t *)data1;
  black_level = (black_level48_t *)data2;
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
  chromatix_BLSS_v2 = &chromatix_common->Chromatix_BLSS_v2_data;
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  if (chromatix_BLSS_v2 && chromatix_BLSS_v2->BLSS_v2_enable) {
    for (i = 0; i < NUM_BLK_REGIONS; i++) {
      start = 0;
      end = 0;
      BLSS_trigger = &(chromatix_BLSS_v2->black_level_data[i].blss_v2_trigger);
      trigger_index = i;

      if (i == NUM_BLK_REGIONS - 1) {
        /* falls within region 6 but we do not use trigger points in the region */
        aec_ratio = 1.0;
        break;
      }

      if (chromatix_BLSS_v2->control_BLSS == 0) {
        /* Lux index based */
        aec_reference = black_level->aec_update.lux_idx;
        start = BLSS_trigger->lux_index_start;
        end   = BLSS_trigger->lux_index_end;
      } else {
        /* Gain based */
        aec_reference = black_level->aec_update.sensor_gain;
        start = BLSS_trigger->gain_start;
        end   = BLSS_trigger->gain_end;
      }

      /* index is within interpolation range, find ratio */
      if ((aec_reference > start) &&
          ((aec_reference < end) || (F_EQUAL(aec_reference, end)))) {
        aec_ratio = (end - aec_reference)/(end - start);
        ISP_DBG("%s [%f - %f - %f] = %f, trigger_idx:%d", __func__, start, aec_reference, end,
          aec_ratio, trigger_index);
        break;
      }

      /* already scanned past the lux index */
      if ((aec_reference < end) || (F_EQUAL(aec_reference, end))) {
        break;
      }
    }

    if (trigger_index >= NUM_BLK_REGIONS) {
      ISP_ERR("no trigger match for black_level trigger value: %f\n", aec_reference);
      goto ERROR;
    }

    if (trigger_index == (NUM_BLK_REGIONS - 1)) {
      /* Last available region */
      BLSS_offset = (float)chromatix_BLSS_v2->black_level_data[trigger_index].
        black_level_offset;
    } else {
        BLSS_offset = (float)LINEAR_INTERPOLATION(
          (float)chromatix_BLSS_v2->black_level_data[trigger_index].black_level_offset,
          (float)chromatix_BLSS_v2->black_level_data[trigger_index + 1].black_level_offset,
          aec_ratio);
    }
  } else {
      BLSS_offset_lowlight =
        chromatix_BLSS->black_level_lowlight.black_level_offset;
      BLSS_offset_normal = chromatix_BLSS->black_level_normal.black_level_offset;

      aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
        chromatix_BLSS->control_BLSS, &chromatix_BLSS->BLSS_low_light_trigger,
        &black_level->aec_update, 1);

      BLSS_offset = (float)LINEAR_INTERPOLATION((float)BLSS_offset_normal,
        (float)BLSS_offset_lowlight, aec_ratio);
  }

  if (black_level->ext_func_table && black_level->ext_func_table->update_params) {
    black_level->ext_func_table->update_params(black_level, BLSS_offset,
    chromatix_BLSS_v2->BLSS_v2_enable, isp_sub_mod->hdr_mode);
  }

  return TRUE;

ERROR:
  return FALSE;
}/* end black_level48_hw_config*/
#endif

/** black_level48_fetch_blklvl_offset:
 *
 *  @module: mct module
 *  @event: mct event
 *  @isp_sub_module: isp_sub_module_t
 *  @isp_sub_module:
 *  This function Fills the blacklevel offset
 *  Return: TRUE on success
 */
boolean black_level48_fetch_blklvl_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  black_level48_t            *blacklvl = NULL;
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

  blacklvl = (black_level48_t *)isp_sub_module->private_data;
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
  *blklvl_offset = blacklvl->pipe_cfg_blss_offset;
  ISP_DBG("offset %d", *blklvl_offset);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}
/** black_level48_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean black_level48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  black_level48_t              *black_level = NULL;
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

  black_level = (black_level48_t *)isp_sub_module->private_data;
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

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    if (black_level->ext_func_table && black_level->ext_func_table->hw_config) {
      ret = black_level->ext_func_table->hw_config(isp_sub_module, black_level);
    }
    if (ret == FALSE) {
      ISP_ERR("failed: bpc48_config");
      goto ERROR;
    }
    isp_sub_module->trigger_update_pending = FALSE;
    black_level48_store_hw_update(isp_sub_module, black_level);
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

/** black_level48_manual_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to aec_manual_update_t
 *
 * Handle manual AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level48_manual_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_manual_update_t                  *manual_stats_update = NULL;
  black_level48_t                      *black_level = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level48_t *)isp_sub_module->private_data;
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  manual_stats_update = (aec_manual_update_t *)data;
  if (!manual_stats_update) {
    ISP_ERR("failed: manual_stats_update %p", manual_stats_update);
    return FALSE;
  }


  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!F_EQUAL(black_level->aec_update.real_gain, manual_stats_update->sensor_gain)) {
    isp_sub_module->trigger_update_pending = TRUE;
      black_level->aec_update.real_gain = manual_stats_update->sensor_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

}
/** black_level48_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t                       *stats_update = NULL;
  black_level48_t                      *black_level = NULL;
  float                                 aec_ratio = 0;
  chromatix_VFE_common_type            *chromatix_common = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_BLSS = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level48_t *)isp_sub_module->private_data;
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

  if (!F_EQUAL(black_level->aec_update.lux_idx, stats_update->aec_update.lux_idx) ||
    !F_EQUAL(black_level->aec_update.real_gain, stats_update->aec_update.real_gain)) {
    black_level->aec_update = stats_update->aec_update;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** black_level48_init_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @black_level: black_level data instance
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean black_level48_init_config(isp_sub_module_t *isp_sub_module,
  black_level48_t *black_level)
{
  ISP_black_level_reg_t                   *Reg = NULL;
  chromatix_parms_type                    *chromatix_ptr = NULL;
  chromatix_VFE_common_type               *chromatix_common = NULL;
  Chromatix_blk_subtract_scale_type       *chromatix_BLSS = NULL;
  Chromatix_blk_subtract_scale_v2_type    *chromatix_BLSS_v2 = NULL;
  chromatix_videoHDR_type                 *chromatix_VHDR = NULL;
  int                                      multifactor = 0;
  unsigned short                           blk_offset = 0;

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
  chromatix_BLSS_v2 = &chromatix_common->Chromatix_BLSS_v2_data;
  if (chromatix_BLSS->BLSS_enable || chromatix_BLSS_v2->BLSS_v2_enable) {
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->update_module_bit = TRUE;
  } else {
    isp_sub_module->submod_enable = FALSE;
    isp_sub_module->update_module_bit = FALSE;
  }

  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  if (chromatix_BLSS_v2->BLSS_v2_enable) {
      blk_offset = chromatix_BLSS_v2->black_level_data[2].black_level_offset;
  } else {
      blk_offset = chromatix_BLSS->black_level_normal.black_level_offset;
  }

  if (black_level->ext_func_table && black_level->ext_func_table->update_params) {
    black_level->ext_func_table->update_params(black_level, blk_offset,
    chromatix_BLSS_v2->BLSS_v2_enable, isp_sub_module->hdr_mode);
  }

  return TRUE;
}

/** black_level48_set_digital_gain:
 *
 * @mod: black_level module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean black_level48_set_digital_gain(isp_sub_module_t *isp_sub_module,
  float *new_dig_gain)
{
  black_level48_t      *black_level = NULL;
  float                *cur_dig_gain;

  if (!new_dig_gain || !isp_sub_module) {
    ISP_ERR("failed: %p %p", new_dig_gain, isp_sub_module);
    return FALSE;
  }

  black_level = (black_level48_t *)isp_sub_module->private_data;
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

/** black_level48_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean black_level48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                             ret = TRUE;
  black_level48_t                    *black_level = NULL;
  modulesChromatix_t                 *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level48_t *)isp_sub_module->private_data;
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

  ret = black_level48_init_config(isp_sub_module, black_level);
  if (ret == FALSE) {
    ISP_ERR("failed: black_level48_init_config ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return ret;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* black_level48_set_chromatix_ptr */

/** black_level48_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean black_level48_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  black_level48_t *black_level = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  black_level = (black_level48_t *)isp_sub_module->private_data;
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

  FILL_FUNC_TABLE(black_level);

  return TRUE;
} /* black_level48_streamoff */

/** black_level48_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the black_level module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean black_level48_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  black_level48_t *black_level = NULL;

  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p module %p", isp_sub_module, module);
    return FALSE;
  }

  black_level = (black_level48_t *)CAM_MALLOC(sizeof(black_level48_t));
  if (!black_level) {
    ISP_ERR("failed: black_level %p", black_level);
    return FALSE;
  }

  memset(black_level, 0, sizeof(*black_level));

  isp_sub_module->private_data = (void *)black_level;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  FILL_FUNC_TABLE(black_level);
  return TRUE;
}/* black_level48_init */

/** black_level48_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void black_level48_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p module %p", isp_sub_module, module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* black_level48_destroy */

#if !OVERRIDE_FUNC
static ext_override_func black_level_override_func = {
  .hw_config = black_level48_hw_config,
  .update_params = black_level48_update_params,
};

boolean black_level48_fill_func_table(black_level48_t *black_level)
{
  black_level->ext_func_table = &black_level_override_func;
  return TRUE;
} /* linearization40_fill_func_table */
#endif
