/* hdr_be_stats46.c
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "hdr_be_stats46.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_HDR_BE_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_HDR_BE_STATS, fmt, ##args)

#define HDR_BE_STATS46_MIN_WIDTH  6
#define HDR_BE_STATS46_MIN_HEIGHT 2
#define HDR_BE_STATS46_MAX_HNUM   160
#define HDR_BE_STATS46_MAX_VNUM   90
#define HDR_BE_STATS46_MIN_HVNUM 1

/** hdr_be_stats46_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void hdr_be_stats46_debug(ISP_StatsHdrBe_CfgCmdType *pcmd)
{
  if (!pcmd) {
    return;
  }
  ISP_DBG("%s:Bayer Exposure Stats Configurations\n", __func__);
  ISP_DBG("%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG("%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG("%s:rgnWidth   %d\n", __func__, pcmd->rgnWidth);
  ISP_DBG("%s:rgnHeight  %d\n", __func__, pcmd->rgnHeight);
  ISP_DBG("%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG("%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
  ISP_DBG("%s:r_max      %d\n", __func__, pcmd->rMax);
  ISP_DBG("%s:gr_max     %d\n", __func__, pcmd->grMax);
  ISP_DBG("%s:b_max      %d\n", __func__, pcmd->bMax);
  ISP_DBG("%s:gb_max     %d\n", __func__, pcmd->gbMax);
}


/** hdr_be_stats46_fill_stats_parser_params:
 *
 *  @isp_sub_module: isp sub module handle
 *  @be_stats: be stats params
 *  @sub_module_output: sub module output
 *  @camif_window_w: camif window width
 *  @camif_window_h: camif window height
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean hdr_be_stats46_fill_stats_parser_params(
  isp_sub_module_t *isp_sub_module, hdr_be_stats46_t *be_stats,
  isp_sub_module_output_t *sub_module_output, uint32_t camif_window_w,
  uint32_t camif_window_h)
{
  ISP_StatsHdrBe_CfgCmdType   *pcmd = NULL;
  isp_rgns_stats_param_t      *rgns_stats = NULL;
  isp_stats_roi_params_t      *roi_params = NULL;
  isp_stats_config_t          *stats_config = NULL;
  aec_be_config_t             *be_config = NULL;
  isp_tintless_stats_config_t *tintless_stats_config = NULL;
  isp_algo_params_t           *algo_params = NULL;
  isp_saved_stats_params_t    *stats_params = NULL;

  if (!isp_sub_module || !be_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, be_stats, sub_module_output);
    return FALSE;
  }

  if (MSM_ISP_STATS_HDR_BE == HW_STATS_TYPE) {
    be_config = &be_stats->aec_config.be_config;
  }
  else {
    be_config = &be_stats->aec_config.aec_bg_config;
  }
  pcmd = &be_stats->pcmd;
  stats_params = sub_module_output->stats_params;
  algo_params = sub_module_output->algo_params;

  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_HDR_BE);
  stats_config->aec_config = be_stats->aec_config;

  roi_params =
    &stats_params->stats_roi_params[MSM_ISP_STATS_HDR_BE];
  roi_params->rgnWidth = pcmd->rgnWidth;
  roi_params->rgnHeight = pcmd->rgnHeight;
  roi_params->rgnVOffset = pcmd->rgnVOffset;
  roi_params->rMax = be_config->r_Max;
  roi_params->bMax = be_config->b_Max;
  roi_params->grMax = be_config->gr_Max;
  roi_params->gbMax = be_config->gb_Max;
  if (be_stats->isp_out_info.is_split == FALSE)
    roi_params->rgnHOffset = pcmd->rgnHOffset;
  else
    roi_params->rgnHOffset = be_stats->left_stipe_rgn_offset;

  /*fill in parsing rgn info*/
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_HDR_BE];
  rgns_stats->is_valid = TRUE;
  if (be_stats->isp_out_info.is_split == TRUE) {
     /*DUAL VFE case*/
    if (be_stats->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
      rgns_stats->h_rgns_start = 0;
      rgns_stats->h_rgns_end = be_stats->num_left_rgns - 1;
      rgns_stats->h_rgns_total =
        be_stats->num_left_rgns + be_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    } else {
      rgns_stats->h_rgns_start = be_stats->num_left_rgns;
      rgns_stats->h_rgns_end =
        be_stats->num_left_rgns + be_stats->num_right_rgns - 1;
      rgns_stats->h_rgns_total =
        be_stats->num_left_rgns + be_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    }
  } else {
    /*SINGLE VFE case*/
    rgns_stats->h_rgns_start = 0;
    rgns_stats->h_rgns_end = pcmd->rgnHNum;
    rgns_stats->h_rgns_total = pcmd->rgnHNum + 1;
    rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
  }

  ISP_DBG("<dual_dbg>rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (isp_sub_module->tintless_enabled == TRUE &&
      HW_STATS_TYPE == MSM_ISP_STATS_HDR_BE) {
    tintless_stats_config =
      &algo_params->tintless_stats_config[MSM_ISP_STATS_HDR_BE];
    tintless_stats_config->is_valid = TRUE;
    tintless_stats_config->camif_win_w = camif_window_w;
    tintless_stats_config->camif_win_h = camif_window_h;
    tintless_stats_config->stat_elem_w =
      FLOOR2(camif_window_w / ISP_STATS_HDR_BE_GRID_H);
    tintless_stats_config->stat_elem_h =
      FLOOR2(camif_window_h / ISP_STATS_HDR_BE_GRID_V);
    tintless_stats_config->num_stat_elem_cols = ISP_STATS_HDR_BE_GRID_H;
    tintless_stats_config->num_stat_elem_rows = ISP_STATS_HDR_BE_GRID_V;
    tintless_stats_config->saturation_limit = 255 - PIX_VALUE_OFFSET;
    tintless_stats_config->stats_type = HW_STATS_TYPE;
  }

  return TRUE;
}

/** hdr_be_stats46_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @be_stats: be stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean hdr_be_stats46_store_hw_update(isp_sub_module_t *isp_sub_module,
  hdr_be_stats46_t *be_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsHdrBe_CfgCmdType   *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsHdrBe_CfgCmdType   *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                     num_reg_cmd = 2;
  ISP_Stats_CfgType            stats_cfg_mask, stats_cfg_val;

  if (!isp_sub_module || !be_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, be_stats);
    return FALSE;
  }

  pcmd = &be_stats->pcmd;
  hdr_be_stats46_debug(pcmd);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)*num_reg_cmd);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd)*num_reg_cmd);

  copy_cmd = (ISP_StatsHdrBe_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
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
  cfg_cmd->num_cfg = num_reg_cmd;

  /* Initialize mask and val */
  stats_cfg_mask.mask = 0;
  stats_cfg_val.mask = 0;

  /* Set both bits in mask */
  stats_cfg_mask.hdr_be_field_sel = 0x3;
  stats_cfg_mask.hdr_stats_site_sel = 1;

  /* Fill val */
  stats_cfg_val.hdr_be_field_sel = 0x0; /* All lines */
  if (isp_sub_module->stats_tap_location == ISP_STATS_TAP_DEFAULT ||
    isp_sub_module->stats_tap_location == ISP_STATS_TAP_AFTER_LENS_ROLLOFF) {
    stats_cfg_val.hdr_stats_site_sel = 0x1; /* Before Demosaic */
  } else {
    stats_cfg_val.hdr_stats_site_sel = 0x0; /* Before HDR */
  }

  reg_cfg_cmd[0].u.mask_info.reg_offset = STATS_CFG_OFF;
  reg_cfg_cmd[0].u.mask_info.mask       = STATS_CFG_MASK;
  reg_cfg_cmd[0].u.mask_info.val        = STATS_CFG_VAL;
  reg_cfg_cmd[0].cmd_type               = VFE_CFG_MASK;

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = HDR_BE_STATS_OFF;
  reg_cfg_cmd[1].u.rw_info.len = HDR_BE_STATS_LEN * sizeof(uint32_t);

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


/** hdr_be_stats46_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean hdr_be_stats46_split_config(hdr_be_stats46_t *be_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset, uint32_t single_rgnWidth)
{
   ISP_StatsHdrBe_CfgCmdType *pcmd = NULL;
   uint32_t                   left_stripe_stats_bound = 0;
   uint32_t                   overlap = 0;
   isp_out_info_t            *isp_out = NULL;

  if (!be_stats) {
    ISP_ERR("NULL pointer! be %p", be_stats);
    return FALSE;
  }

  pcmd = &be_stats->pcmd;
  overlap = be_stats->ispif_out_info.overlap;
  isp_out = &be_stats->isp_out_info;

  /*using determined single VFE config to further determine dual vfe config*/
    /*find out left stripe stats boundry*/
  left_stripe_stats_bound = pcmd->rgnHOffset +
    (single_rgnWidth * single_rgnHNum);
  if (left_stripe_stats_bound > isp_out->right_stripe_offset + overlap)
    left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

  be_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_bound) ?
    0: (left_stripe_stats_bound - single_rgnHOffset) / single_rgnWidth;
  be_stats->num_right_rgns = single_rgnHNum - be_stats->num_left_rgns;
  /* Check if the stat region satisfies the crucial assumption: there is
     at least one grid line within the overlap area if the whole region spans across
     both stripes */
  if (single_rgnHOffset + (be_stats->num_left_rgns * single_rgnWidth)
      < isp_out->right_stripe_offset && be_stats->num_right_rgns > 0) {
    ISP_ERR("Unable to support such stats region in dual-ISP mode\n");
    return -1;
  }

  be_stats->left_stipe_rgn_offset = pcmd->rgnHOffset;
  if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
    /* even no rgn in left, we still need irq to get buffer done*/
    pcmd->rgnHNum = (be_stats->num_left_rgns > 0) ?
      be_stats->num_left_rgns - 1 : 0;
    pcmd->rgnHOffset = (be_stats->num_left_rgns > 0) ?
      FLOOR2(pcmd->rgnHOffset) : 0;
  } else {
    pcmd->rgnHNum = (be_stats->num_right_rgns > 0) ?
      be_stats->num_right_rgns - 1 : 0;
    pcmd->rgnHOffset = single_rgnHOffset +
      (be_stats->num_left_rgns * single_rgnWidth)
      - isp_out->right_stripe_offset;
    pcmd->rgnHOffset = (be_stats->num_right_rgns > 0) ?
      FLOOR2(pcmd->rgnHOffset) : 0;
  }

  ISP_DBG("<dual_dbg> num_left_rgn %d, num_rught_rgn %d",
    be_stats->num_left_rgns, be_stats->num_right_rgns);

  return TRUE;
}

/** hdr_be_stats46_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for be_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                    ret = TRUE;
  ISP_StatsHdrBe_CfgCmdType *pcmd = NULL;
  hdr_be_stats46_t          *be_stats = NULL;
  isp_private_event_t       *private_event = NULL;
  sensor_out_info_t         *sensor_out_info = NULL;
  isp_sub_module_output_t   *sub_module_output = NULL;
  isp_stats_config_t        *stats_config = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  uint32_t                   camif_window_w,
                             camif_window_h,
                             diff,Maxwidth,
                             Maxheight,
                             rgnwidth, rgnheight;
  int32_t                    tmp;
  isp_stats_roi_params_t    *roi_params = NULL;
  aec_be_config_t           *be_config = NULL;

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

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    goto ERROR;
  }

  pcmd = &be_stats->pcmd;

  if (MSM_ISP_STATS_HDR_BE == HW_STATS_TYPE)
    be_config = &be_stats->aec_config.be_config;
  else
    be_config = &be_stats->aec_config.aec_bg_config;

  sensor_out_info = &be_stats->sensor_out_info;

  camif_window_w = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  camif_window_h = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("BE not enabled / trigger update pending false");

    /* Fill be_config */
    if (sub_module_output->stats_params) {
      ret = hdr_be_stats46_fill_stats_parser_params(isp_sub_module, be_stats,
        sub_module_output, camif_window_w, camif_window_h);
      if (ret == FALSE) {
        ISP_ERR("failed: hdr_be_stats46_fill_stats_parser_params");
      }
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((be_config->roi.left + be_config->roi.width) > (int)camif_window_w) {
    diff = (be_config->roi.left + be_config->roi.width) - camif_window_w;
    tmp = be_config->roi.width - diff;
    if (tmp < 0) {
      goto ERROR;
    }
    be_config->roi.width = tmp;
    ISP_ERR("Be stats ROI width > sensor width %u", camif_window_w);
  }
  if ((be_config->roi.top + be_config->roi.height) > (int)camif_window_h) {
    diff = (be_config->roi.top + be_config->roi.height) - camif_window_h;
    tmp = be_config->roi.height - diff;
    if (tmp < 0) {
      goto ERROR;
    }
    be_config->roi.height = tmp;
    ISP_ERR("Be stats ROI width > sensor height %u", camif_window_h);
  }

  if ((be_config->grid_info.h_num != 0) && (be_config->grid_info.v_num != 0)) {
    rgnwidth = FLOOR2(be_config->roi.width / be_config->grid_info.h_num);
    Maxwidth = FLOOR2(camif_window_w/be_config->grid_info.h_num);
    rgnwidth = Clamp(rgnwidth, HDR_BE_STATS46_MIN_WIDTH, Maxwidth);

    rgnheight = FLOOR2(be_config->roi.height / be_config->grid_info.v_num);
    Maxheight = FLOOR2(camif_window_h/be_config->grid_info.v_num);
    rgnheight = Clamp(rgnheight,HDR_BE_STATS46_MIN_HEIGHT, Maxheight);

    pcmd->rgnWidth = rgnwidth - 1;
    pcmd->rgnHeight = rgnheight - 1;
    pcmd->rgnHOffset = FLOOR2(be_config->roi.left);
    pcmd->rgnVOffset = FLOOR2(be_config->roi.top);
    pcmd->rgnHNum    = Clamp(be_config->grid_info.h_num, HDR_BE_STATS46_MIN_HVNUM,
      HDR_BE_STATS46_MAX_HNUM) - 1;
    pcmd->rgnVNum    = Clamp(be_config->grid_info.v_num, HDR_BE_STATS46_MIN_HVNUM,
      HDR_BE_STATS46_MAX_VNUM) - 1;

    pcmd->rMax       = be_config->r_Max;
    pcmd->grMax      = be_config->gr_Max;
    pcmd->bMax       = be_config->b_Max;
    pcmd->gbMax      = be_config->gb_Max;

    /* update be_config what we applied */
    be_config->roi.width = (rgnwidth) * be_config->grid_info.h_num;
    be_config->roi.height = (rgnheight)* be_config->grid_info.h_num ;
    be_config->roi.left = pcmd->rgnHOffset;
    be_config->roi.top = pcmd->rgnVOffset;
    be_config->grid_info.h_num = pcmd->rgnHNum + 1;
    be_config->grid_info.v_num = pcmd->rgnVNum + 1;
  } else {
    ISP_DBG("failed Denomitor is zero hnum %d vnum %d",
      be_config->grid_info.h_num, be_config->grid_info.v_num);

  /* Default configuration */
    pcmd->rgnWidth   = FLOOR2(camif_window_w / ISP_STATS_HDR_BE_GRID_H) - 1;
    pcmd->rgnHeight  = FLOOR2(camif_window_h / ISP_STATS_HDR_BE_GRID_V) - 1;
    pcmd->rgnHOffset = FLOOR2((camif_window_w - ISP_STATS_HDR_BE_GRID_H *
      (pcmd->rgnWidth + 1)) / 2);
    pcmd->rgnVOffset = FLOOR2((camif_window_h - ISP_STATS_HDR_BE_GRID_V *
      (pcmd->rgnHeight + 1)) / 2);
    pcmd->rgnHNum    = ISP_STATS_HDR_BE_GRID_H - 1;
    pcmd->rgnVNum    = ISP_STATS_HDR_BE_GRID_V - 1;
    pcmd->rMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->grMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->bMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->gbMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);

    /* update be_config what we applied */
    be_config->roi.width = (pcmd->rgnWidth + 1) * ISP_STATS_HDR_BE_GRID_H;
    be_config->roi.height = (pcmd->rgnHeight + 1) * ISP_STATS_HDR_BE_GRID_V;
    be_config->roi.left = pcmd->rgnHOffset;
    be_config->roi.top = pcmd->rgnVOffset;
    be_config->grid_info.h_num = pcmd->rgnHNum + 1;
    be_config->grid_info.v_num = pcmd->rgnVNum + 1;
    be_config->r_Max = pcmd->rMax;
    be_config->gr_Max = pcmd->rMax;
    be_config->b_Max = pcmd->rMax;
    be_config->gb_Max = pcmd->rMax;
  }
  pcmd->rMin       = 0;
  pcmd->grMin      = 0;
  pcmd->bMin       = 0;
  pcmd->gbMin      = 0;

  if (be_stats->isp_out_info.is_split == TRUE) {
     ret =  hdr_be_stats46_split_config(
       be_stats, pcmd->rgnHNum + 1, pcmd->rgnHOffset, pcmd->rgnWidth + 1);
     if (ret == FALSE) {
       ISP_ERR("failed hdr_be_stats46_split_config");
       goto ERROR;
     }
  }

  if (sub_module_output->stats_params) {
    ret = hdr_be_stats46_fill_stats_parser_params(isp_sub_module, be_stats,
      sub_module_output, camif_window_w, camif_window_h);
    if (ret == FALSE) {
      ISP_ERR("failed: hdr_be_stats46_fill_stats_parser_params");
      goto ERROR;
    }
  }

  ret = hdr_be_stats46_store_hw_update(isp_sub_module, be_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: hdr_be_stats46_store_hw_update");
    goto ERROR;
  }

  if (isp_sub_module->config_pending == TRUE) {
    ret = hdr_be_stats_ext46_hw_update(isp_sub_module, be_stats);
    if (ret == FALSE) {
      ISP_ERR("failed: hdr_be_stats_ext46_hw_update");
    }
    isp_sub_module->config_pending = FALSE;
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
} /* hdr_be_stats46_trigger_update */

/** hdr_be_stats46_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  hdr_be_stats46_t           *be_stats = NULL;
  ispif_out_info_t           *ispif_stripe_info = NULL;
  ISP_StatsHdrBe_CfgCmdType  *pcmd = NULL;
  isp_private_event_t        *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  be_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** hdr_be_stats46_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  hdr_be_stats46_t            *be_stats = NULL;
  isp_out_info_t              *isp_split_out_info = NULL;
  ISP_StatsHdrBe_CfgCmdType   *pcmd = NULL;
  isp_private_event_t         *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  be_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** hdr_be_stats46_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  hdr_be_stats46_t   *be_stats = NULL;
  aec_config_t       *aec_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  aec_config = (aec_config_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (memcmp(aec_config, &be_stats->aec_config, sizeof(aec_config_t))) {
    be_stats->aec_config = *aec_config;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** hdr_be_stats46_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  hdr_be_stats46_t           *be_stats = NULL;
  sensor_out_info_t          *sensor_out_info = NULL;
  ISP_StatsHdrBe_CfgCmdType  *pcmd = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  be_stats->sensor_out_info = *sensor_out_info;

  pcmd = &be_stats->pcmd;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = ISP_STATS_HDR_BE_GRID_H - 1;
  pcmd->rgnVNum    = ISP_STATS_HDR_BE_GRID_V - 1;
  pcmd->rMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
  pcmd->grMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);
  pcmd->bMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
  pcmd->gbMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);
  pcmd->rMin       = 0;
  pcmd->grMin      = 0;
  pcmd->bMin       = 0;
  pcmd->gbMin      = 0;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* hdr_be_stats46_set_stream_config */

/** hdr_be_stats46_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr_be_stats46_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  hdr_be_stats46_t *be_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)isp_sub_module->private_data;
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(be_stats, 0, sizeof(*be_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* hdr_be_stats46_streamoff */

/** hdr_be_stats46_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the be_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean hdr_be_stats46_init(isp_sub_module_t *isp_sub_module)
{
  hdr_be_stats46_t *be_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  be_stats = (hdr_be_stats46_t *)CAM_MALLOC(sizeof(hdr_be_stats46_t));
  if (!be_stats) {
    ISP_ERR("failed: be_stats %p", be_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(be_stats, 0, sizeof(*be_stats));

  isp_sub_module->private_data = (void *)be_stats;
  isp_sub_module->hw_stats_type = HW_STATS_TYPE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* hdr_be_stats46_init */

/** hdr_be_stats46_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void hdr_be_stats46_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* hdr_be_stats46_destroy */
