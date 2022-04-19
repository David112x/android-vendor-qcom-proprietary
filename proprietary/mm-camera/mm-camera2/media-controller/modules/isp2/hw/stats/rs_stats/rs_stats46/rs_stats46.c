/* rs_stats46.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "rs_stats46.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_RS_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_RS_STATS, fmt, ##args)

#define RS_INPUT_DEPTH  12
#define RS_OUTPUT_DEPTH 16

/** rs_stats46_debug:
 *
 *  @rs_pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void rs_stats46_debug(ISP_StatsRs_CfgType *rs_pcmd, ISP_Stats_CfgType *init_pcmd)
{
  if (!rs_pcmd || !init_pcmd) {
    return;
  }
  ISP_DBG("RS Stats Configurations");
  ISP_DBG("rgnHNum = %d", rs_pcmd->rgnHNum);
  ISP_DBG("rgnVNum = %d", rs_pcmd->rgnVNum);
  ISP_DBG("rgnWidth = %d", rs_pcmd->rgnWidth);
  ISP_DBG("rgnHeight = %d", rs_pcmd->rgnHeight);
  ISP_DBG("rgnHOffset = %d", rs_pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset = %d", rs_pcmd->rgnVOffset);
  ISP_DBG("shiftBits = %d", init_pcmd->rsShiftBits);
}

/** rs_stats46_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rs_stats46_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rs_stats46_t   *rs_stats = NULL;
  rs_config_t    *rs_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  rs_config = (rs_config_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (rs_stats->rs_config.max_algo_support_h_rgn !=
    rs_config->max_algo_support_h_rgn ||
    rs_stats->rs_config.max_algo_support_v_rgn !=
    rs_config->max_algo_support_v_rgn) {
    rs_stats->rs_config = *rs_config;
    ISP_HIGH("max h rgn supported  by AFD algo %d max v rgn supported %d",
      rs_stats->rs_config.max_algo_support_h_rgn,
      rs_stats->rs_config.max_algo_support_v_rgn);
    if (rs_stats->rs_config.max_algo_support_h_rgn >
      rs_stats->rs_caps.max_Hnum ||
      !rs_stats->rs_config.max_algo_support_h_rgn) {
      ISP_ERR("error: max_support_h_rgn invalid %d",
         rs_stats->rs_config.max_algo_support_h_rgn);
      /* Use RS_MAX_H_REGIONS instead of rs_caps.max_Hnum as we dont know
       * if stats algo support rs_caps.max_Hnum in dual vfe case */
       rs_stats->rs_config.max_algo_support_h_rgn = RS_MAX_H_REGIONS;
    }
    if (rs_stats->rs_config.max_algo_support_v_rgn >
        rs_stats->rs_caps.max_Vnum ||
        !rs_stats->rs_config.max_algo_support_v_rgn)
      rs_stats->rs_config.max_algo_support_v_rgn = RS_MAX_V_REGIONS;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** rs_stats46_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @rs_stats: bg stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean rs_stats46_store_hw_update(isp_sub_module_t *isp_sub_module,
  rs_stats46_t *rs_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsRs_CfgType         *rs_pcmd = NULL;
  ISP_Stats_CfgType           *init_pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsRs_CfgType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_Stats_CfgType           cfg_mask;
  uint32_t                    num_reg_cfg_cmd = 2;

  if (!isp_sub_module || !rs_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, rs_stats);
    return FALSE;
  }

  init_pcmd = &rs_stats->init_pcmd;
  rs_pcmd = &rs_stats->rs_pcmd;
  rs_stats46_debug(rs_pcmd, init_pcmd);

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

  copy_cmd = (ISP_StatsRs_CfgType *)CAM_MALLOC(sizeof(*copy_cmd));
  if (!copy_cmd) {
    ISP_ERR("failed: copy_cmd %p", copy_cmd);
    goto ERROR_COPY_CMD;
  }
  memset(copy_cmd, 0, sizeof(*copy_cmd));

  *copy_cmd = *rs_pcmd;

  cfg_mask.rsShiftBits  |= ~0;

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
  reg_cfg_cmd[1].u.rw_info.reg_offset = RS_STATS_OFF;
  reg_cfg_cmd[1].u.rw_info.len = RS_STATS_LEN * sizeof(uint32_t);

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

/** rs_stats46_get_stats_capabilities:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  @data: handle to mct_stats_info_t
 *
 *  Get stats caps
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean rs_stats46_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                     ret = TRUE;
  mct_stats_info_t           *stats_info = NULL;
  isp_private_event_t        *private_event = NULL;
  rs_stats46_t               *rs_stats = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p data %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  stats_info = (mct_stats_info_t *)private_event->data;
  if (!stats_info) {
    ISP_ERR("failed: stats_info %p", stats_info);
    return FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memcpy(&stats_info->rs_caps, &rs_stats->rs_caps, sizeof(stats_info->rs_caps));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}
/** rs_stats46_fill_stats_parser_params:
 *
 *  @rs_stats: rs stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean rs_stats46_fill_stats_parser_params(rs_stats46_t *rs_stats,
  isp_saved_stats_params_t *stats_params)
{
  ISP_StatsRs_CfgType    *rs_pcmd = NULL;
  ISP_Stats_CfgType      *init_pcmd = NULL;
  isp_rgns_stats_param_t *rgns_stats = NULL;

  if (!rs_stats || !stats_params ) {
    ISP_ERR("failed: %p %p", rs_stats, stats_params);
    return FALSE;
  }

  init_pcmd = &rs_stats->init_pcmd;
  rs_pcmd = &rs_stats->rs_pcmd;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_RS];

  stats_params->rs_shift_bits = init_pcmd->rsShiftBits;
  rgns_stats->is_valid = TRUE;

  if (rs_stats->ispif_out_info.is_split == TRUE) {
     /*both rgn =0 means they are both HALF and sharing the same Horizontal rgn*/
     if ((rs_stats->num_left_rgns == 0) &&
          (rs_stats->num_right_rgns == 0)) {
        rgns_stats->h_rgns_start = 0;
        rgns_stats->h_rgns_end = 0; /*single rgn - 1 = 0*/
        rgns_stats->h_rgns_total = 1;
        rgns_stats->v_rgns_total = rs_pcmd->rgnVNum + 1;
     } else {
        if (rs_stats->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
          rgns_stats->h_rgns_start = 0;
          rgns_stats->h_rgns_end = rs_stats->num_left_rgns - 1;
          rgns_stats->h_rgns_total =
            rs_stats->num_left_rgns + rs_stats->num_right_rgns;
          rgns_stats->v_rgns_total = rs_pcmd->rgnVNum + 1;
        } else {
          rgns_stats->h_rgns_start = rs_stats->num_left_rgns;
          rgns_stats->h_rgns_end =
            rs_stats->num_left_rgns + rs_stats->num_right_rgns - 1;
          rgns_stats->h_rgns_total =
            rs_stats->num_left_rgns + rs_stats->num_right_rgns;
          rgns_stats->v_rgns_total = rs_pcmd->rgnVNum + 1;
        }
     }
  } else {
    /*Single VFE case*/
    rgns_stats->h_rgns_start = 0;
    rgns_stats->h_rgns_end = rs_pcmd->rgnHNum;
    rgns_stats->h_rgns_total = rs_pcmd->rgnHNum + 1;
    rgns_stats->v_rgns_total = rs_pcmd->rgnVNum + 1;
  }
  ISP_DBG("rs rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  return TRUE;
}

/** rs_stats46_fetch_rs_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Fetches RS number of rows
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rs_stats46_fetch_rs_info(isp_sub_module_t *isp_sub_module,
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

  cs_rs_stats_info->num_rows = RS_MAX_V_REGIONS;

  return TRUE;
}

/** rs_stats46_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean rs_stats46_split_config(rs_stats46_t *rs_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset,
  uint32_t single_rgn_width)
{
  ISP_StatsRs_CfgType        *rs_pcmd = NULL;
  uint32_t                   left_stripe_stats_end = 0;
  isp_out_info_t            *isp_out = NULL;
  boolean                    is_split_rgn = FALSE;

  if (!rs_stats) {
    ISP_ERR("failed, NULL pointer RS %p", rs_stats);
    return FALSE;
  }

  rs_pcmd = &rs_stats->rs_pcmd;
  isp_out = &rs_stats->isp_out_info;

  /* There are two ways in which we can configure dual vfe case
   * We can either configure single_rgnHNum into both left and right vfe
   * and still the total number of HNum is within the algo limit
   * max_algo_support_h_rgn or if the 2* single_rgnHNum exceeds algo limit
   * then we split single_rgnHNum into left and right vfe */
  is_split_rgn = ((single_rgnHNum * 2) >
    rs_stats->rs_config.max_algo_support_h_rgn) ? TRUE : FALSE;
  if (is_split_rgn) {
    /* For RS it is assumed that the ROI is centered. */
    if (single_rgnHNum == 1) {
      /* special case:
         both rgn = 0 means they are both HALF and sharing the same Horiz rgn
         needs to combine two regions into one at the end */
      rs_stats->num_left_rgns = 0;
      rs_stats->num_right_rgns = 0;
      rs_pcmd->rgnWidth = (single_rgn_width / 2) - 1;
      if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
        rs_pcmd->rgnHOffset = single_rgnHOffset;
      } else {
        rs_pcmd->rgnHOffset = (single_rgnHOffset - isp_out->right_stripe_offset)
          + single_rgn_width / 2;
      }
    } else if (IF_EVEN(single_rgnHNum) &&
      (single_rgnHNum <=rs_stats->rs_caps.max_Hnum)) {
      rs_stats->num_left_rgns = single_rgnHNum / 2;
      rs_stats->num_right_rgns = single_rgnHNum / 2;
      rs_pcmd->rgnHNum = (single_rgnHNum / 2) - 1;
      if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
        rs_pcmd->rgnHOffset = single_rgnHOffset;
      } else {
        rs_pcmd->rgnHOffset = (single_rgnHOffset - isp_out->right_stripe_offset)
          + (single_rgn_width * rs_stats->num_left_rgns);
      }
    } else {
      ISP_ERR("Does not support %d horizontal regions for row sum stats in"
        "dual VFE mode\n", single_rgnHNum);
      return FALSE;
    }
  } else {
    rs_stats->num_left_rgns = single_rgnHNum;
    rs_stats->num_right_rgns = single_rgnHNum;
    rs_pcmd->rgnHNum = single_rgnHNum - 1;
    /* keep the same rgn number, rgn width will reduce.
       camif size is about half of of orginal*/
    rs_pcmd->rgnWidth = (single_rgn_width / 2) - 1;

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      rs_pcmd->rgnHOffset = single_rgnHOffset;
    } else {
      rs_pcmd->rgnHOffset = (single_rgnHOffset - isp_out->right_stripe_offset)
        + ((single_rgn_width / 2) * rs_stats->num_left_rgns);
    }
  }

  return TRUE;
}

/** rs_stats46_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for rs_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean rs_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  ISP_Stats_CfgType       *init_pcmd = NULL;
  ISP_StatsRs_CfgType     *rs_pcmd = NULL;
  rs_stats46_t            *rs_stats = NULL;
  isp_private_event_t     *private_event = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  uint32_t                 camif_width = 0,
                           camif_height = 0,
                           rgn_h_num = 0,
                           rgn_v_num = 0,
                           rgn_width = 0,
                           rgn_height = 0,
                           max_algo_support_h_rgn = 0;

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

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    goto ERROR;
  }

  init_pcmd = &rs_stats->init_pcmd;
  rs_pcmd = &rs_stats->rs_pcmd;
  sensor_out_info = &rs_stats->sensor_out_info;

  if ((isp_sub_module->submod_enable == FALSE) &&
    (sub_module_output->stats_params)) {
    sub_module_output->stats_params->rgns_stats[MSM_ISP_STATS_RS].is_valid =
      FALSE;
  }

  if (isp_sub_module->trigger_update_pending == FALSE) {
    ISP_DBG("RS not enabled / trigger update pending false");

    /* Fill rs_config */
    if (sub_module_output->stats_params) {
      ret = rs_stats46_fill_stats_parser_params(rs_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: rs_stats46_fill_stats_parser_params");
      }
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  camif_width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  camif_height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  max_algo_support_h_rgn =
    rs_stats->rs_config.max_algo_support_h_rgn;
  if (max_algo_support_h_rgn < rs_stats->rs_caps.max_Hnum &&
      max_algo_support_h_rgn != 0) {
    rgn_h_num = max_algo_support_h_rgn;
  } else {
    ISP_ERR("error: max_algo_support_h_rgn %d invalid", max_algo_support_h_rgn);
    /* Use RS_MAX_H_REGIONS instead of rs_caps.max_Hnum as we dont know
     * if stats algo support rs_caps.max_Hnum in dual vfe case */
    rgn_h_num = RS_MAX_H_REGIONS;
  }
  if (rs_stats->rs_config.max_algo_support_v_rgn < RS_MAX_V_REGIONS &&
      rs_stats->rs_config.max_algo_support_v_rgn != 0) {
    rgn_v_num = rs_stats->rs_config.max_algo_support_v_rgn;
  } else {
    rgn_v_num = RS_MAX_V_REGIONS;
  }

  /*1. calculate region size, modify height to get less offset*/
  rgn_width = camif_width / rgn_h_num;
  rgn_height = (camif_height + rgn_v_num - 1) / rgn_v_num;

  /*1. config rgn height and width
    2. check hw limitation*/
  rgn_width = MAX(1, rgn_width);
  rgn_height = MAX(1, rgn_height);
  rgn_height = MIN(RS_MAX_RGN_HEIGHT, rgn_height);
  rs_pcmd->rgnWidth = rgn_width - 1;
  rs_pcmd->rgnHeight = rgn_height - 1;

  /*calculate region num, check hw limitation
    modify according to final rgn_width &height */
  rgn_h_num = camif_width / rgn_width;
  rgn_v_num = camif_height / rgn_height;
  rgn_h_num = MIN(RS_MAX_H_REGIONS, rgn_h_num);
  rgn_v_num = MIN(RS_MAX_V_REGIONS, rgn_v_num);
  rs_pcmd->rgnHNum = rgn_h_num - 1;
  rs_pcmd->rgnVNum = rgn_v_num - 1;

  /*config offset: rs_rgn_offset + (cmd->rs_rgn_num + 1) *
    (cmd->rs_rgn_width + 1) <= image_width*/
  rs_pcmd->rgnHOffset = (camif_width % (rgn_width * rgn_h_num)) / 2;
  rs_pcmd->rgnVOffset = (camif_height % (rgn_height * rgn_v_num)) / 2;

  if (rs_stats->ispif_out_info.is_split) {
    ret = rs_stats46_split_config(rs_stats,
      rs_pcmd->rgnHNum + 1, rs_pcmd->rgnHOffset, rs_pcmd->rgnWidth +1);
    if (ret == FALSE) {
      ISP_ERR("rs_stats46_split_config failed");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  init_pcmd->rsShiftBits = isp_sub_module_util_calculate_shift_bits(
    ((rs_pcmd->rgnWidth + 1) * rgn_height), RS_INPUT_DEPTH, RS_OUTPUT_DEPTH);

  ret = rs_stats46_store_hw_update(isp_sub_module, rs_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: rs_stats46_store_hw_update");
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
    ret = rs_stats46_fill_stats_parser_params(rs_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: rs_stats46_fill_stats_parser_params");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* rs_stats46_trigger_update */


/** rs_stats46_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rs_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rs_stats46_t            *rs_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  rs_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** rs_stats46_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rs_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rs_stats46_t            *rs_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  rs_stats->isp_out_info = *isp_split_out_info;
  if (isp_split_out_info->is_split) {
    rs_stats->rs_caps.max_Hnum = 2 * RS_MAX_H_REGIONS;
  } else {
    rs_stats->rs_caps.max_Vnum = RS_MAX_H_REGIONS;
  }
  rs_stats->rs_caps.is_valid = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** rs_stats46_update_min_stripe_overlap:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Update min overlap/max stripe offset constraints
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rs_stats46_update_min_stripe_overlap(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rs_stats46_t              *rs_stats = NULL;
  isp_private_event_t    *private_event = NULL;
  isp_stripe_limit_info_t  *stripe_limit = NULL;
  uint32_t                     sensor_output_width = 0;
  uint32_t                     mid_point  = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
    ISP_ERR("failed: stripe_limit %p", stripe_limit);
    return FALSE;
  }

  sensor_output_width = rs_stats->sensor_out_info.request_crop.last_pixel
    - rs_stats->sensor_out_info.request_crop.first_pixel + 1;
  mid_point = sensor_output_width / 2;

  if (!stripe_limit->max_stripe_offset ||
    stripe_limit->max_stripe_offset > mid_point) {
    stripe_limit->max_stripe_offset = mid_point;
  }
  stripe_limit->max_stripe_offset = FLOOR2(stripe_limit->max_stripe_offset);
  ISP_DBG("max_stripe_offset %u mid pt %u", stripe_limit->max_stripe_offset, mid_point);

  return TRUE;
}

/** rs_stats46_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rs_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  rs_stats46_t            *rs_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  ISP_StatsRs_CfgType     *rs_pcmd = NULL;
  aec_bg_config_t         *bg_config;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  rs_stats->sensor_out_info = *sensor_out_info;
  /* Default regions supported is mapped to VFE's default limit */
  rs_stats->rs_config.max_algo_support_h_rgn = RS_MAX_H_REGIONS;
  rs_stats->rs_config.max_algo_support_v_rgn = RS_MAX_V_REGIONS;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* rs_stats46_set_stream_config */

/** rs_stats46_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean rs_stats46_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  rs_stats46_t *rs_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)isp_sub_module->private_data;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(rs_stats, 0, sizeof(*rs_stats));
  rs_stats->rs_caps.max_Hnum = RS_MAX_H_REGIONS;
  rs_stats->rs_caps.max_Vnum = RS_MAX_V_REGIONS;
  rs_stats->rs_caps.is_valid = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* rs_stats46_streamoff */

/** rs_stats46_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the rs_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean rs_stats46_init(isp_sub_module_t *isp_sub_module)
{
  rs_stats46_t *rs_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  rs_stats = (rs_stats46_t *)CAM_MALLOC(sizeof(rs_stats46_t));
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats %p", rs_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(rs_stats, 0, sizeof(*rs_stats));
  /*initialize to Max regions for single VFE */
  rs_stats->rs_caps.max_Hnum = RS_MAX_H_REGIONS;
  rs_stats->rs_caps.max_Vnum = RS_MAX_V_REGIONS;
  rs_stats->rs_caps.is_valid = TRUE;

  isp_sub_module->private_data = (void *)rs_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* rs_stats46_init */

/** rs_stats46_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void rs_stats46_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* rs_stats46_destroy */
