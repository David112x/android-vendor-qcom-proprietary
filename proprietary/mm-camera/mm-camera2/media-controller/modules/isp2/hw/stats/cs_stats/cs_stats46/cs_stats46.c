/* cs_stats46.c
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "cs_stats46.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CS_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CS_STATS, fmt, ##args)


#define CS_INPUT_DEPTH  12
#define CS_OUTPUT_DEPTH 16

/** cs_stats46_debug:
 *
 *  @cs_pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void cs_stats46_debug(ISP_StatsCs_CfgType *cs_pcmd, ISP_Stats_CfgType *init_pcmd)
{
  if (!cs_pcmd || !init_pcmd) {
    return;
  }
  ISP_DBG("CS Stats Configurations");
  ISP_DBG("rgnHNum = %d", cs_pcmd->rgnHNum);
  ISP_DBG("rgnVNum = %d", cs_pcmd->rgnVNum);
  ISP_DBG("rgnWidth = %d", cs_pcmd->rgnWidth);
  ISP_DBG("rgnHeight = %d", cs_pcmd->rgnHeight);
  ISP_DBG("rgnHOffset = %d", cs_pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset = %d", cs_pcmd->rgnVOffset);
  ISP_DBG("shiftBits = %d", init_pcmd->csShiftBits);
}

/** cs_stats46_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @cs_stats: bg stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean cs_stats46_store_hw_update(isp_sub_module_t *isp_sub_module,
  cs_stats46_t *cs_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsCs_CfgType      *cs_pcmd = NULL;
  ISP_Stats_CfgType        *init_pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsCs_CfgType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_Stats_CfgType   cfg_mask;
  uint32_t            num_reg_cfg_cmd = 2;

  if (!isp_sub_module || !cs_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, cs_stats);
    return FALSE;
  }

  cs_pcmd = &cs_stats->cs_pcmd;
  init_pcmd = &cs_stats->init_pcmd;
  cs_stats46_debug(cs_pcmd,init_pcmd);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd) * num_reg_cfg_cmd);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * num_reg_cfg_cmd);

  copy_cmd = (ISP_StatsCs_CfgType *)CAM_MALLOC(sizeof(*copy_cmd));
  if (!copy_cmd) {
    ISP_ERR("failed: copy_cmd %p", copy_cmd);
    goto ERROR_COPY_CMD;
  }
  memset(copy_cmd, 0, sizeof(*copy_cmd));

  *copy_cmd = *cs_pcmd;

  cfg_mask.csShiftBits  |= ~0;

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)copy_cmd;
  cfg_cmd->cmd_len = sizeof(*copy_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = num_reg_cfg_cmd;

  reg_cfg_cmd[0].u.mask_info.reg_offset = STATS_CFG_OFF;
  reg_cfg_cmd[0].u.mask_info.mask = cfg_mask.mask;
  reg_cfg_cmd[0].u.mask_info.val = init_pcmd->mask;
  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = CS_STATS_OFF;
  reg_cfg_cmd[1].u.rw_info.len = CS_STATS_LEN * sizeof(uint32_t);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(copy_cmd);
ERROR_COPY_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
}


/** cs_stats46_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean cs_stats46_split_config(cs_stats46_t *cs_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset,
  uint32_t single_rgn_width)
{
  ISP_StatsCs_CfgType *cs_pcmd = NULL;
  uint32_t                   left_stripe_stats_end = 0;
  uint32_t                   overlap = 0;
  isp_out_info_t            *isp_out = NULL;
  uint32_t                   target_remainder = 0;
  uint32_t                   current_remainder = 0;
  if (!cs_stats) {
    ISP_ERR("failed: NULL pointer cs_stats %p", cs_stats);
    return FALSE;
  }

  overlap = cs_stats->ispif_out_info.overlap;
  cs_pcmd = &cs_stats->cs_pcmd;
  isp_out = &cs_stats->isp_out_info;

  if (isp_out->is_split == TRUE) {
    /*find left VFE stats end bound*/
    left_stripe_stats_end =
      single_rgnHOffset + (single_rgn_width * single_rgnHNum);
    if (left_stripe_stats_end > isp_out->right_stripe_offset + overlap)
      left_stripe_stats_end = isp_out->right_stripe_offset + overlap;

    /* if offset go over left bound, no rgn required in left VFE */
    cs_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_end) ?
      0 : (left_stripe_stats_end - single_rgnHOffset) / single_rgn_width;

    /* adjust (num_left_rgns-1) according to the mod 8 remainder within 4 to
       7 requirement, its Hw limitation,
       so convert the rgn num to HW format then calculate */
    if (cs_stats->num_left_rgns) {
      target_remainder = (single_rgnHNum - 1) % 16 / 2;
      current_remainder = (cs_stats->num_left_rgns - 1) % 8;
      cs_stats->num_left_rgns -= (current_remainder >= target_remainder) ?
        current_remainder - target_remainder : current_remainder + 8 -
        target_remainder;
    }
    cs_stats->num_right_rgns = single_rgnHNum - cs_stats->num_left_rgns;

    /*decide dual vfe stats horizontal config*/
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      cs_pcmd->rgnHNum = (cs_stats->num_left_rgns > 0) ?
        cs_stats->num_left_rgns - 1 : 0;
      cs_pcmd->rgnHOffset = (cs_stats->num_left_rgns > 0) ?
        single_rgnHOffset : 0;
    } else {
      /*Right stats rgn num*/
      cs_pcmd->rgnHNum = (cs_stats->num_right_rgns > 0) ?
        cs_stats->num_right_rgns - 1 : 0;
      /*Right stats offset*/
      cs_pcmd->rgnHOffset = single_rgnHOffset +
        (cs_stats->num_left_rgns * (cs_pcmd->rgnWidth + 1)) -
        isp_out->right_stripe_offset;
      cs_pcmd->rgnHOffset = (cs_stats->num_right_rgns > 0) ?
         cs_pcmd->rgnHOffset : 0;
    }
  }

  return TRUE;
}

/** cs_stats46_fill_stats_parser_params:
 *
 *  @cs_stats: rs stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean cs_stats46_fill_stats_parser_params(cs_stats46_t *cs_stats,
  isp_saved_stats_params_t *stats_params)
{
  ISP_StatsCs_CfgType    *cs_pcmd = NULL;
  ISP_Stats_CfgType      *init_pcmd = NULL;
  isp_rgns_stats_param_t *rgns_stats = NULL;

  if (!cs_stats || !stats_params) {
    ISP_ERR("failed: %p %p", cs_stats, stats_params);
    return FALSE;
  }

  cs_pcmd = &cs_stats->cs_pcmd;
  init_pcmd = &cs_stats->init_pcmd;
  init_pcmd = &cs_stats->init_pcmd;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_CS];

  stats_params->cs_shift_bits = init_pcmd->csShiftBits;
  rgns_stats->is_valid = TRUE;

  if (cs_stats->isp_out_info.is_split == TRUE) {
     if (cs_stats->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
       rgns_stats->h_rgns_start = 0;
       rgns_stats->h_rgns_end = cs_stats->num_left_rgns - 1;
       rgns_stats->h_rgns_total =
         cs_stats->num_left_rgns + cs_stats->num_right_rgns;
       rgns_stats->v_rgns_total = cs_pcmd->rgnVNum + 1;
     } else {
       rgns_stats->h_rgns_start = cs_stats->num_left_rgns;
       rgns_stats->h_rgns_end =
         cs_stats->num_left_rgns + cs_stats->num_right_rgns - 1;
       rgns_stats->h_rgns_total =
         cs_stats->num_left_rgns + cs_stats->num_right_rgns;
       rgns_stats->v_rgns_total = cs_pcmd->rgnVNum + 1;
     }
  } else {
    rgns_stats->h_rgns_start = 0;
    rgns_stats->h_rgns_end = cs_pcmd->rgnHNum;
    rgns_stats->h_rgns_total = cs_pcmd->rgnHNum + 1;
    rgns_stats->v_rgns_total = cs_pcmd->rgnVNum + 1;
  }
  ISP_DBG("rs rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  return TRUE;
}

/** cs_stats_get_h_num_adjustment:
 *
 *
 *  @h_num: h num
 *  @divider: divider
 *  @min_remainder: min remainder
 *
 *  Programmed rgnHNum has a requirement to fulfill: when it is divided by 8,
 *  the remainder needs to lie within 4 to 7 inclusively. In dual-vfe mode,
 *  we need to adjust it so that when it's divided by 16, the remainder lies
 *  within 9 and 15 so that the remainder is big enough to be
 *  shared by both vfe's. This function returns the adjustment
 *  needed on the hNum to fulfill the requirement.
 *
 *
 *  Return remainder
 **/
static inline uint32_t cs_stats_get_h_num_adjustment(uint32_t h_num,
  uint32_t divider, uint32_t min_remainder)
{
  uint32_t remainder = h_num % divider;
  if (remainder < min_remainder)
    return remainder + 1;
  return 0;
}

/** cs_stats46_fetch_cs_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Fetches CS number of columns
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean cs_stats46_fetch_cs_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  isp_private_event_t       *private_event;
  isp_hw_rs_cs_stats_info_t *cs_rs_stats_info;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  cs_rs_stats_info = (isp_hw_rs_cs_stats_info_t *)private_event->data;
  if (!cs_rs_stats_info) {
    ISP_ERR("failed: output");
    return FALSE;
  }

  cs_rs_stats_info->num_cols = CS_MAX_H_REGIONS;

  return TRUE;
}

/** cs_stats46_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for cs_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean cs_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  ISP_StatsCs_CfgType     *cs_pcmd = NULL;
  ISP_Stats_CfgType       *init_pcmd = NULL;
  cs_stats46_t            *cs_stats = NULL;
  boolean                  is_split = FALSE;
  ispif_out_info_t        *ispif_out_info = NULL;
  isp_out_info_t          *isp_out = NULL;
  isp_private_event_t     *private_event = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  uint32_t                 camif_width = 0,
                           camif_height = 0,
                           rgn_h_num = 0,
                           rgn_v_num = 0,
                           rgn_width = 0,
                           rgn_height = 0;
  uint32_t                 overlap = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  cs_stats = (cs_stats46_t *)isp_sub_module->private_data;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    goto ERROR;
  }

  cs_pcmd = &cs_stats->cs_pcmd;
  init_pcmd = &cs_stats->init_pcmd;
  ispif_out_info = &cs_stats->ispif_out_info;
  isp_out = &cs_stats->isp_out_info;
  sensor_out_info = &cs_stats->sensor_out_info;
  is_split = ispif_out_info->is_split;
  overlap = ispif_out_info->overlap;

  if ((isp_sub_module->submod_enable == FALSE) &&
    (sub_module_output->stats_params)) {
    sub_module_output->stats_params->rgns_stats[MSM_ISP_STATS_CS].is_valid =
      FALSE;
  }

  if (isp_sub_module->trigger_update_pending == FALSE) {
    ISP_DBG("RS not enabled / trigger update pending false");

    /* Fill rs_config */
    if (sub_module_output->stats_params) {
      ret = cs_stats46_fill_stats_parser_params(cs_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: cs_stats46_fill_stats_parser_params");
      }
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  camif_width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  camif_height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  rgn_h_num = CS_MAX_H_REGIONS;
  rgn_v_num = CS_MAX_V_REGIONS;

  /*config region size, check hw limitation*/
  rgn_width = (camif_width + rgn_h_num - 1) / rgn_h_num;
  rgn_height = camif_height / rgn_v_num;

  /*1. config rgn height and width
   2. check hw limitation*/
  rgn_width = MAX(CS_MIN_RGN_WIDTH, rgn_width);
  rgn_width = MIN(CS_MAX_RGN_WIDTH, rgn_width);
  rgn_height = MAX(1, rgn_height);
  cs_pcmd->rgnWidth = rgn_width - 1;
  cs_pcmd->rgnHeight = rgn_height - 1;

  /* 1.config region num
     2.modify according to final rgn_width &height
     3.check hw limitation*/
  rgn_h_num = camif_width / rgn_width;
  rgn_v_num = camif_height / rgn_height;
  rgn_h_num = MIN(CS_MAX_H_REGIONS, rgn_h_num);
  rgn_v_num = MIN(CS_MAX_V_REGIONS, rgn_v_num);
  cs_pcmd->rgnHNum = rgn_h_num - 1;
  cs_pcmd->rgnVNum = rgn_v_num - 1;

  /* Adjust rgnHNum to satisfy the hardware limitation: the programmed rgnHNum
      when divided by 8, needs to have a remainder within 4 and 7
      inclusively. In dual-vfe mode, we need to adjust it so that when it's
      divided by 16, the remainder lies within 9 and 15 so that the remainder
      is enough to be shared by both vfe's */
  cs_pcmd->rgnHNum -= (is_split == TRUE) ?
    cs_stats_get_h_num_adjustment(cs_pcmd->rgnHNum, 16, 9) :
    cs_stats_get_h_num_adjustment(cs_pcmd->rgnHNum, 8, 4);
  rgn_h_num = cs_pcmd->rgnHNum + 1;
  /*config offset: cs_rgn_offset + (cs_rgn_num + 1) *
    (rs_rgn_width + 1) <= image_width*/
  cs_pcmd->rgnHOffset = (camif_width % (rgn_width * rgn_h_num)) / 2;
  cs_pcmd->rgnVOffset = (camif_height % (rgn_height * rgn_v_num)) / 2;

  init_pcmd->csShiftBits =
    isp_sub_module_util_calculate_shift_bits(rgn_height,
      CS_INPUT_DEPTH, CS_OUTPUT_DEPTH);

  if (cs_stats->isp_out_info.is_split == TRUE) {
    ret = cs_stats46_split_config(cs_stats,
      cs_pcmd->rgnHNum +1, cs_pcmd->rgnHOffset, cs_pcmd->rgnWidth +1);
    if (ret == FALSE) {
      ISP_ERR("faield: cs_stats46_split_config failed");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  ret = cs_stats46_store_hw_update(isp_sub_module, cs_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: cs_stats46_store_hw_update");
    goto ERROR;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  /* Fill rs_config */
  if (sub_module_output->stats_params) {
    ret = cs_stats46_fill_stats_parser_params(cs_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: cs_stats46_fill_stats_parser_params");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* cs_stats46_trigger_update */

/** cs_stats46_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean cs_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  cs_stats46_t            *cs_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cs_stats = (cs_stats46_t *)isp_sub_module->private_data;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  cs_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** cs_stats46_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean cs_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  cs_stats46_t            *cs_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cs_stats = (cs_stats46_t *)isp_sub_module->private_data;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  cs_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** cs_stats46_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean cs_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  cs_stats46_t            *cs_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  ISP_StatsCs_CfgType  *cs_pcmd = NULL;
  aec_bg_config_t         *bg_config;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cs_stats = (cs_stats46_t *)isp_sub_module->private_data;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  cs_stats->sensor_out_info = *sensor_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* cs_stats46_set_stream_config */

/** cs_stats46_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean cs_stats46_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  cs_stats46_t *cs_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cs_stats = (cs_stats46_t *)isp_sub_module->private_data;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(cs_stats, 0, sizeof(*cs_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* cs_stats46_streamoff */

/** cs_stats46_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the cs_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean cs_stats46_init(isp_sub_module_t *isp_sub_module)
{
  cs_stats46_t *cs_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cs_stats = (cs_stats46_t *)CAM_MALLOC(sizeof(cs_stats46_t));
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats %p", cs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(cs_stats, 0, sizeof(*cs_stats));

  isp_sub_module->private_data = (void *)cs_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* cs_stats46_init */

/** cs_stats46_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void cs_stats46_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* cs_stats46_destroy */
