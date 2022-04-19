/* bf_stats40.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "bf_stats40.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"

#define MIN_RGN_H_OFFSET 20
#define MIN_RGN_V_OFFSET  2
#define MIN_GRID_WIDTH    8
#define MAX_GRID_WIDTH_DIVIDER    2
#define MIN_GRID_HEIGHT   2
#define MAX_GRID_HEIGHT_DIVIDER    2
#define MIN_H_NUM         1
#define MAX_H_NUM         18
#define MIN_V_NUM         1
#define MAX_V_NUM         14

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BF_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BF_STATS, fmt, ##args)

/* bitwidth of fv_min configured from outside */
#define FV_MIN_BITWIDTH_CFG  8
#define FV_MIN_SHIFT   \
  (FV_MIN_BITWIDTH_HW - FV_MIN_BITWIDTH_CFG)

/** bf_stats40_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void bf_stats40_debug(ISP_StatsBf_CfgCmdType *pcmd)
{
  if (!pcmd) {
    ISP_ERR("pcmd is null");
    return;
  }
  ISP_DBG("Bayer Focus Stats Configurations");
  ISP_DBG("rgnHOffset %d", pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset %d", pcmd->rgnVOffset);
  ISP_DBG("rgnWidth   %d", pcmd->rgnWidth);
  ISP_DBG("rgnHeight  %d", pcmd->rgnHeight);
  ISP_DBG("rgnHNum    %d", pcmd->rgnHNum);
  ISP_DBG("rgnVNum    %d", pcmd->rgnVNum);
  ISP_DBG("r_fv_min   %d", pcmd->r_fv_min);
  ISP_DBG("gr_fv_min  %d", pcmd->gr_fv_min);
  ISP_DBG("b_fv_min   %d", pcmd->b_fv_min);
  ISP_DBG("gb_fv_min  %d", pcmd->gb_fv_min);
  ISP_DBG("a00        %d", pcmd->a00);
  ISP_DBG("a01        %d", pcmd->a01);
  ISP_DBG("a02        %d", pcmd->a02);
  ISP_DBG("a03        %d", pcmd->a03);
  ISP_DBG("a04        %d", pcmd->a04);

  ISP_DBG("a10        %d", pcmd->a10);
  ISP_DBG("a11        %d", pcmd->a11);
  ISP_DBG("a12        %d", pcmd->a12);
  ISP_DBG("a13        %d", pcmd->a13);
  ISP_DBG("a14        %d", pcmd->a14);

}

/** bf_stats40_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_stats: bf stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats40_store_hw_update(isp_sub_module_t *isp_sub_module,
  bf_stats40_t *bf_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsBf_CfgCmdType      *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsBf_CfgCmdType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                     num_reg_cfg_cmd = 2;

  if (!isp_sub_module || !bf_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_stats);
    return FALSE;
  }

  pcmd = &bf_stats->pcmd;
  bf_stats40_debug(pcmd);

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

  copy_cmd = (ISP_StatsBf_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
  if (!copy_cmd) {
    ISP_ERR("failed: copy_cmd %p", copy_cmd);
    goto ERROR_COPY_CMD;
  }
  memset(copy_cmd, 0, sizeof(*copy_cmd));

  *copy_cmd = *pcmd;
  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)copy_cmd;
  cfg_cmd->cmd_len = sizeof(*copy_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = num_reg_cfg_cmd;

  /* writing to STATS_CFG_OFF wont have any effect on vfe44
   * mask is 0 for vfe 40.
   * this change is made to reuse the code for vfe 46.
   */
  reg_cfg_cmd[0].u.mask_info.reg_offset = STATS_CFG_OFF;
  reg_cfg_cmd[0].u.mask_info.mask       = STATS_CFG_MASK;
  reg_cfg_cmd[0].u.mask_info.val        = STATS_CFG_VAL;
  reg_cfg_cmd[0].cmd_type               = VFE_CFG_MASK;

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = BF_STATS_OFF_0;
  reg_cfg_cmd[1].u.rw_info.len = BF_STATS_LEN_0 * sizeof(uint32_t);
/*
  reg_cfg_cmd[2].u.rw_info.cmd_data_offset = reg_cfg_cmd[1].u.rw_info.len;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;
  reg_cfg_cmd[2].u.rw_info.reg_offset = BF_STATS_OFF_1;
  reg_cfg_cmd[2].u.rw_info.len = BF_STATS_LEN_1 * sizeof(uint32_t);
*/
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

/** bf_stats40_update_min_stripe_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats40_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  bf_stats40_t                   *bf_stats = NULL;
  isp_private_event_t             *private_event = NULL;
  int                            rc = 0;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  uint32_t                       bf_grid_width = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, rolloff enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
    ISP_ERR("failed: stripe_limit %p", stripe_limit);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bf_grid_width = bf_stats->camif.width /
    bf_stats->af_config.common.grid_info.h_num;
  /*whole overlap need to > bf grid width*/
  if ((stripe_limit->min_overlap_left * 2) < bf_grid_width) {
    stripe_limit->min_overlap_left = (bf_grid_width + 1) / 2;
  }
  if ((stripe_limit->min_overlap_right * 2) < bf_grid_width) {
    stripe_limit->min_overlap_right = (bf_grid_width + 1) / 2;
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** bf_stats40_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean bf_stats40_split_config(bf_stats40_t *bf_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset,
  uint32_t single_rgnWidth)
{
  ISP_StatsBf_CfgCmdType *pcmd = NULL;
  uint32_t                left_stripe_stats_bound = 0;
  uint32_t                overlap = 0;
  isp_out_info_t          *isp_out = NULL;

  if (!bf_stats) {
    ISP_ERR("NULL pointer! bf %p", bf_stats);
    return FALSE;
  }

  pcmd = &bf_stats->pcmd;
  overlap = bf_stats->ispif_out_info.overlap;
  isp_out = &bf_stats->isp_out_info;

  if (isp_out->is_split == TRUE) {

    /*find out left stripe stats boundry*/
    left_stripe_stats_bound = pcmd->rgnHOffset +
       single_rgnWidth * single_rgnHNum;
    if (left_stripe_stats_bound > isp_out->right_stripe_offset + overlap)
      left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

    /*if single VFE request stats rgn go over left end,
      then no rgn required in left VFE,
      or we need to calculate the rgn num for left VFE*/
    bf_stats->num_left_rgns = (pcmd->rgnHOffset > left_stripe_stats_bound) ?
      0 : (left_stripe_stats_bound - pcmd->rgnHOffset) / single_rgnWidth;
    bf_stats->num_right_rgns = single_rgnHNum - bf_stats->num_left_rgns;

    /* Check if the stat region satisfies the crucial assumption: there is
       at least one grid line within the overlap area if the whole
       region spans across both stripes */
    if (single_rgnHOffset + (bf_stats->num_left_rgns * single_rgnWidth) <
       isp_out->right_stripe_offset && bf_stats->num_right_rgns > 0) {
      ISP_ERR("Unable to support such stats region in dual-ISP mode");
      return FALSE;
    }

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /*check with system team, hw limit should be minimum 0 only
        do we still need to propgram at least one as 8974?*/
      pcmd->rgnHNum =  (bf_stats->num_left_rgns > 0) ?
        bf_stats->num_left_rgns - 1 : 0;
      pcmd->rgnHOffset = (bf_stats->num_left_rgns > 0) ?
        FLOOR2(single_rgnHOffset) : MIN_RGN_H_OFFSET;
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (bf_stats->num_right_rgns > 0) ?
        bf_stats->num_right_rgns - 1 : 0;
      pcmd->rgnHOffset = single_rgnHOffset +
        (bf_stats->num_left_rgns * single_rgnWidth) -
        isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (bf_stats->num_right_rgns > 0) ?
        FLOOR2(pcmd->rgnHOffset) : MIN_RGN_H_OFFSET;
    }
    pcmd->rgnHOffset = (pcmd->rgnHOffset >= MIN_RGN_H_OFFSET) ?
      pcmd->rgnHOffset : MIN_RGN_H_OFFSET;
  }

  return TRUE;
}

/** bf_stats40_fill_stats_parser_params:
 *
 *  @bf_stats: bf stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats40_fill_stats_parser_params(bf_stats40_t *bf_stats,
  isp_saved_stats_params_t *stats_params)
{
  ISP_StatsBf_CfgCmdType    *pcmd = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_stats_roi_params_t    *roi_params = NULL;
  isp_stats_config_t        *stats_config = NULL;
  isp_out_info_t            *isp_out = NULL;
  if (!bf_stats || !stats_params) {
    ISP_ERR("failed: %p %p", bf_stats, stats_params);
    return FALSE;
  }

  pcmd = &bf_stats->pcmd;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BF];
  isp_out = &bf_stats->isp_out_info;

  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_BF);
  stats_config->af_config = bf_stats->af_config;

  /*fill in parse info*/
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BF];
  rgns_stats->is_valid = TRUE;
  if (isp_out->is_split == TRUE) {
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /*DUAL VFE case*/
      rgns_stats->h_rgns_start = 0;
      rgns_stats->h_rgns_end = bf_stats->num_left_rgns - 1;
      rgns_stats->h_rgns_total =
        bf_stats->num_left_rgns + bf_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    } else {
      rgns_stats->h_rgns_start = bf_stats->num_left_rgns;
      rgns_stats->h_rgns_end =
        bf_stats->num_left_rgns + bf_stats->num_right_rgns - 1;
      rgns_stats->h_rgns_total =
        bf_stats->num_left_rgns + bf_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    }
  } else {
    /*SINGLE VFE case */
    rgns_stats->h_rgns_start = 0;
    rgns_stats->h_rgns_end = pcmd->rgnHNum;
    rgns_stats->h_rgns_total = pcmd->rgnHNum + 1;
    rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
  }

  ISP_DBG("rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  return TRUE;
}

/** bf_stats40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for bf_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  ISP_StatsBf_CfgCmdType  *pcmd = NULL;
  af_config_t             *af_config = NULL;
  bf_config_t             *bf_config = NULL;
  uint32_t                 bf_rgn_width = 0,
                           bf_rgn_height = 0;
  int32_t                  i = 0;
  bf_stats40_t            *bf_stats = NULL;
  boolean                  is_split = FALSE;
  ispif_out_info_t        *ispif_out_info = NULL;
  isp_out_info_t          *isp_out = NULL;
  isp_private_event_t     *private_event = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  isp_stats_config_t      *stats_config = NULL;
  isp_rgns_stats_param_t  *rgns_stats = NULL;

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

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
   goto ERROR;
  }

  pcmd = &bf_stats->pcmd;
  af_config = &bf_stats->af_config;
  bf_config = &bf_stats->af_config.bf;
  ispif_out_info = &bf_stats->ispif_out_info;
  isp_out = &bf_stats->isp_out_info;

  is_split = ispif_out_info->is_split;
  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("BF not enabled / trigger update pending false");
    /* Fill af_config */
    if (sub_module_output->stats_params) {
      ret = bf_stats40_fill_stats_parser_params(bf_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats44_fill_stats_parser_params");
      }
    }

    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((af_config->common.grid_info.h_num == 0) ||
    (af_config->common.grid_info.v_num == 0)) {
    ISP_ERR("Invalid BF h_num & v_num from 3A, h_num = %d, v_num = %d\n",
      af_config->common.grid_info.h_num, af_config->common.grid_info.v_num);
    goto ERROR;
  }

  if((bf_config->roi.top + bf_config->roi.height > bf_stats->camif.height) ||
    (bf_config->roi.left + bf_config->roi.width > bf_stats->camif.width)){
    ISP_ERR("Invalid BF stats ROI[%d,%d,%d,%d], CAMIF Dim [%d * %d]\n",
    bf_config->roi.left, bf_config->roi.top,
    bf_config->roi.width,bf_config->roi.height,
    bf_stats->camif.width,bf_stats->camif.height);
    goto ERROR;
  }

  bf_rgn_width =
    bf_config->roi.width / af_config->common.grid_info.h_num;
  bf_rgn_height =
    bf_config->roi.height / af_config->common.grid_info.v_num;

  ISP_ERR("af_rgn: width %d height %d, full roi: width = %d, height = %d",
    bf_rgn_width, bf_rgn_height, bf_config->roi.width, bf_config->roi.height);

  pcmd->r_fv_min   = bf_config->r_min  << FV_MIN_SHIFT;
  pcmd->gr_fv_min  = bf_config->gr_min << FV_MIN_SHIFT;
  pcmd->b_fv_min   = bf_config->b_min  << FV_MIN_SHIFT;
  pcmd->gb_fv_min  = bf_config->gb_min << FV_MIN_SHIFT;

  /* min of rgn_v_offset = 2, min of rgn_h_offset = 20
     rgn_v_offset = 2 + roi_top,  rgn_h_offset = 20 + roi_left */
  pcmd->rgnHOffset = FLOOR2(bf_config->roi.left);
  pcmd->rgnVOffset = FLOOR2(bf_config->roi.top);
  pcmd->rgnWidth   = FLOOR2(bf_rgn_width) - 1;
  pcmd->rgnHeight  = FLOOR2(bf_rgn_height) - 1;
  pcmd->rgnHNum    = af_config->common.grid_info.h_num - 1;
  pcmd->rgnVNum    = af_config->common.grid_info.v_num - 1;
  ISP_DBG("rgn %d %d %d %d %d %d", pcmd->rgnHOffset, pcmd->rgnVOffset,
    pcmd->rgnWidth, pcmd->rgnHeight, pcmd->rgnHNum, pcmd->rgnVNum);

  pcmd->a00        = bf_config->hpf[0];
  pcmd->a01        = bf_config->hpf[1];
  pcmd->a02        = bf_config->hpf[2];
  pcmd->a03        = bf_config->hpf[3];
  pcmd->a04        = bf_config->hpf[4];
  pcmd->a10        = bf_config->hpf[5];
  pcmd->a11        = bf_config->hpf[6];
  pcmd->a12        = bf_config->hpf[7];
  pcmd->a13        = bf_config->hpf[8];
  pcmd->a14        = bf_config->hpf[9];

  /* update bfConfig to reflect the new config
    It will be sent to 3A in STATS_NOTIFY event  */
  bf_config->roi.left = pcmd->rgnHOffset;
  bf_config->roi.top = pcmd->rgnVOffset;
  bf_config->roi.width =
    (pcmd->rgnWidth + 1) * af_config->common.grid_info.h_num;
  bf_config->roi.height =
    (pcmd->rgnHeight + 1) * af_config->common.grid_info.v_num;
  ISP_DBG("bf_config %d %d %d %d", bf_config->roi.left, bf_config->roi.top,
    bf_config->roi.width, bf_config->roi.height);

  ret = bf_stats40_split_config(bf_stats, pcmd->rgnHNum +1,
    pcmd->rgnHOffset, pcmd->rgnWidth + 1);
  if (ret == FALSE) {
    ISP_ERR("bf_stats40_split_config failed");
    goto ERROR;
  }


  /* rgnHOffset needs to be at least 20 and at most camif_window_width - 2 */
  if (pcmd->rgnHOffset < MIN_RGN_H_OFFSET ||
    pcmd->rgnHOffset >= bf_stats->camif.width - 2) {
    ISP_ERR("Unsupported BF stats region config: invalid offset: %d",
      pcmd->rgnHOffset);
    pcmd->rgnHOffset = MIN_RGN_H_OFFSET;
  }

  /* Fill af_config */
  if (sub_module_output->stats_params) {
    ret = bf_stats40_fill_stats_parser_params(bf_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats44_fill_stats_parser_params");
    }
  }

  ret = bf_stats40_store_hw_update(isp_sub_module, bf_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats44_store_hw_update");
    goto ERROR;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = FALSE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* bf_stats44_trigger_update */

/** bf_stats40_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats40_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats40_t   *bf_stats = NULL;
  af_config_t    *af_config = NULL;
  bf_config_t    *bf_config = NULL;
  uint32_t       roi_w_adjust = 0;
  uint32_t       roi_h_adjust = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  af_config = (af_config_t *)data;
  if (!(af_config->mask & MCT_EVENT_STATS_BF_SINGLE)) {
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_config = &af_config->bf;

  /*if offset not fit limtation, then adjust offset.
    we need to adjust width so it wont go over camif boundary*/
  if (bf_config->roi.left < MIN_RGN_H_OFFSET) {
    roi_w_adjust = MIN_RGN_H_OFFSET - bf_config->roi.left;
    bf_config->roi.left =  MIN_RGN_H_OFFSET;
    bf_config->roi.width  = bf_config->roi.width - roi_w_adjust;
  }

  if (bf_config->roi.top < MIN_RGN_V_OFFSET) {
    roi_h_adjust = MIN_RGN_V_OFFSET - bf_config->roi.top;
    bf_config->roi.top =  MIN_RGN_V_OFFSET;
    bf_config->roi.height  = bf_config->roi.height - roi_h_adjust;
  }

  if((bf_config->roi.top + bf_config->roi.height > bf_stats->camif.height) ||
    (bf_config->roi.left + bf_config->roi.width > bf_stats->camif.width)){
    ISP_ERR("Invalid BF stats ROI[%d,%d,%d,%d], CAMIF Dim [%d * %d]\n",
    bf_config->roi.left, bf_config->roi.top,
    bf_config->roi.width,bf_config->roi.height,
    bf_stats->camif.width,bf_stats->camif.height);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  bf_stats->af_config = *af_config;

  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bf_stats40_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats40_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats40_t            *bf_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bf_stats40_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats40_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats40_t            *bf_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bg_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bf_stats40_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats40_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats40_t            *bf_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  af_config_t             *af_init_cfg = NULL;
  bf_config_t             *bf_init_cfg = NULL;
  ISP_StatsBf_CfgCmdType  *pcmd = NULL;
  //int ret = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_stats->sensor_out_info = *sensor_out_info;

  bf_stats->camif.width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;

  bf_stats->camif.height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  af_init_cfg = &bf_stats->af_config;
  bf_init_cfg = &bf_stats->af_config.bf;
  af_init_cfg->common.grid_info.h_num = 18;
  af_init_cfg->common.grid_info.v_num = 14;
  bf_init_cfg->roi.left = bf_stats->camif.width / 4;
  bf_init_cfg->roi.top = bf_stats->camif.height / 4;
  bf_init_cfg->roi.width = bf_stats->camif.width / 2;
  bf_init_cfg->roi.height = bf_stats->camif.height / 2;
  pcmd = &bf_stats->pcmd;
  pcmd->rgnHOffset = 8;
  pcmd->rgnVOffset = 2;
  pcmd->rgnWidth   = bf_stats->camif.width;
  pcmd->rgnHeight  = bf_stats->camif.height;
  pcmd->rgnHNum    = 5;
  pcmd->rgnVNum    = 4;
  pcmd->r_fv_min   = 14;
  pcmd->gr_fv_min  = 14;
  pcmd->b_fv_min   = 14;
  pcmd->gb_fv_min  = 14;
  pcmd->a00        = -4;
  pcmd->a01        = 0;
  pcmd->a02        = -2;
  pcmd->a03        = 0;
  pcmd->a04        = -4;
  pcmd->a10        = -1;
  pcmd->a11        = -1;
  pcmd->a12        = 14;
  pcmd->a13        = -1;
  pcmd->a14        = -1;

  ISP_DBG("roi %d %d %d %d", bf_init_cfg->roi.left, bf_init_cfg->roi.top,
    bf_init_cfg->roi.width, bf_init_cfg->roi.height);
  bf_stats40_debug(pcmd);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bf_stats40_set_stream_config */

/** bf_stats40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats40_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bf_stats40_t *bf_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bf_stats, 0, sizeof(*bf_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bf_stats44_streamoff */

/** bf_stats40_get_stats_capabilities:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  @data: handle to mct_stats_info_t
 *
 *  Get stats caps
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bf_stats40_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  void *data)
{
  mct_stats_info_t        *stats_info = NULL;
  mct_stats_bf_roi_caps_t *bf_roi_caps = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  bf_stats40_t            *bf_stats = NULL;
  uint32_t                 camif_width = 0, camif_height = 0;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p data %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  sensor_out_info = &bf_stats->sensor_out_info;
  camif_width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1);
  camif_height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1);

  private_event = (isp_private_event_t *)data;
  stats_info = (mct_stats_info_t *)private_event->data;
  if (!stats_info) {
    ISP_ERR("failed: stats_info %p", stats_info);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  stats_info->bf_caps.bf_mask |=
    (MCT_EVENT_STATS_BF_SINGLE | MCT_EVENT_STATS_BF_SCALE);

  bf_roi_caps = &stats_info->bf_caps.bf_roi_caps;
  bf_roi_caps->is_valid = TRUE;
  bf_roi_caps->min_grid_width = MIN_GRID_WIDTH;
  if (MAX_GRID_WIDTH_DIVIDER) {
    bf_roi_caps->max_grid_width = camif_width / MAX_GRID_WIDTH_DIVIDER;
  } else {
    ISP_ERR("failed: MAX_GRID_WIDTH_DIVIDER %d", MAX_GRID_WIDTH_DIVIDER);
  }
  bf_roi_caps->max_grid_height = MIN_GRID_HEIGHT;
  if (MAX_GRID_HEIGHT_DIVIDER) {
    bf_roi_caps->max_grid_height = camif_height / MAX_GRID_HEIGHT_DIVIDER;
  } else {
    ISP_ERR("failed: MAX_GRID_HEIGHT_DIVIDER %d", MAX_GRID_HEIGHT_DIVIDER);
  }
  bf_roi_caps->min_hor_offset = MIN_RGN_H_OFFSET;
  bf_roi_caps->max_hor_offset = camif_width - MIN_GRID_WIDTH - MIN_RGN_H_OFFSET;
  bf_roi_caps->min_ver_offset = MIN_RGN_V_OFFSET;
  bf_roi_caps->max_ver_offset = camif_height - MIN_GRID_HEIGHT -
    MIN_RGN_V_OFFSET;
  bf_roi_caps->min_hor_grids = MIN_H_NUM;
  bf_roi_caps->max_hor_grids = MAX_H_NUM;
  bf_roi_caps->min_ver_grids = MIN_V_NUM;
  bf_roi_caps->max_ver_grids = MAX_V_NUM;
  bf_roi_caps->max_total_grids = bf_roi_caps->max_hor_grids *
    bf_roi_caps->max_ver_grids;

  ISP_DBG("%x %d %d %d %d %d %d %d %d %d %d %d %d", stats_info->bf_caps.bf_mask,
    bf_roi_caps->min_grid_width, bf_roi_caps->min_grid_height,
    bf_roi_caps->max_grid_width, bf_roi_caps->max_grid_height,
    bf_roi_caps->min_hor_offset, bf_roi_caps->max_hor_offset,
    bf_roi_caps->min_ver_offset, bf_roi_caps->max_ver_offset,
    bf_roi_caps->min_hor_grids, bf_roi_caps->max_hor_grids,
    bf_roi_caps->min_ver_grids, bf_roi_caps->max_ver_grids);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bf_stats40_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bf_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bf_stats40_init(isp_sub_module_t *isp_sub_module)
{
  bf_stats40_t *bf_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats40_t *)CAM_MALLOC(sizeof(bf_stats40_t));
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(bf_stats, 0, sizeof(*bf_stats));

  isp_sub_module->private_data = (void *)bf_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* bf_stats40_init */

/** bf_stats40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void bf_stats40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  free(isp_sub_module->private_data);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* bf_stats40_destroy */
