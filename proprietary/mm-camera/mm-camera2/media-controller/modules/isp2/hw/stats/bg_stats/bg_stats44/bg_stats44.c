/* bg_stats44.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "bg_stats44.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BG_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BG_STATS, fmt, ##args)

#define MIN_RGN_WIDTH  4
#define MIN_RGN_HEIGHT 2

/** bg_stats44_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void bg_stats44_debug(ISP_StatsBg_CfgCmdType *pcmd)
{
  if (!pcmd) {
    return;
  }
  ISP_DBG("Bayer Grid Stats Configurations");
  ISP_DBG("rgnHOffset %d", pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset %d", pcmd->rgnVOffset);
  ISP_DBG("rgnWidth   %d", pcmd->rgnWidth);
  ISP_DBG("rgnHeight  %d", pcmd->rgnHeight);
  ISP_DBG("rgnHNum    %d", pcmd->rgnHNum);
  ISP_DBG("rgnVNum    %d", pcmd->rgnVNum);
  ISP_DBG("gbMax      %d", pcmd->gbMax);
  ISP_DBG("grMax      %d", pcmd->grMax);
  ISP_DBG("rMax       %d", pcmd->rMax);
  ISP_DBG("bMax       %d", pcmd->bMax);
}

/** bg_stats44_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bg_stats: bg stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bg_stats44_store_hw_update(isp_sub_module_t *isp_sub_module,
  bg_stats44_t *bg_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsBg_CfgCmdType      *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsBg_CfgCmdType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !bg_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bg_stats);
    return FALSE;
  }

  pcmd = &bg_stats->pcmd;
  bg_stats44_debug(pcmd);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  copy_cmd = (ISP_StatsBg_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
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
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = BG_STATS_OFF;
  reg_cfg_cmd[0].u.rw_info.len = BG_STATS_LEN * sizeof(uint32_t);

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

/** bg_stats44_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean bg_stats44_split_config(bg_stats44_t *bg_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset, uint32_t single_rgnWidth)
{
   ISP_StatsBg_CfgCmdType *pcmd = NULL;
   uint32_t                left_stripe_stats_bound = 0;
   uint32_t                overlap = 0;
   isp_out_info_t          *isp_out = NULL;

  if (!bg_stats) {
    ISP_ERR("NULL pointer! bg %p", bg_stats);
    return FALSE;
  }

  pcmd = &bg_stats->pcmd;
  overlap = bg_stats->ispif_out_info.overlap;
  isp_out = &bg_stats->isp_out_info;

  /*using determined single VFE config to further determine dual vfe config*/
    /*find out left stripe stats boundry*/
  left_stripe_stats_bound =
    single_rgnHOffset + (single_rgnWidth * single_rgnHNum);

  if (left_stripe_stats_bound > isp_out->right_stripe_offset + overlap)
    left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

  /*if single VFE request stats rgn go over left end,
    then no rgn required in left VFE,
    or we need to calculate the rgn num for left VFE*/
  bg_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_bound) ?
     0: (left_stripe_stats_bound - single_rgnHOffset) / single_rgnWidth;

  /*calculate righ num rgns based on left VFE rgn*/
  bg_stats->num_right_rgns = single_rgnHNum - bg_stats->num_left_rgns;
  /* Check if the stat region satisfies the crucial assumption: there is
     at least one grid line within the overlap area if the whole region
     spans across both stripes */
  if (single_rgnHOffset + (bg_stats->num_left_rgns * single_rgnWidth) <
    isp_out->right_stripe_offset && bg_stats->num_right_rgns > 0) {
    ISP_ERR("Unable to support such stats region in dual-ISP mode");
    return FALSE;
  }

  if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
     /* programe 0 means 1 already*/
     pcmd->rgnHNum  = (bg_stats->num_left_rgns > 0) ?
       bg_stats->num_left_rgns - 1: 0;
   /* limitation is even number*/
     pcmd->rgnHOffset = (bg_stats->num_left_rgns > 0) ?
       FLOOR2(single_rgnHOffset) : 0;
  } else {
      /* programe 0 means 1 already*/
    pcmd->rgnHNum = (bg_stats->num_right_rgns > 0) ?
       bg_stats->num_right_rgns - 1 : 0;

    /*right offset from left stats rgn info,
      HW limit: even number*/
    pcmd->rgnHOffset = single_rgnHOffset +
      (bg_stats->num_left_rgns * single_rgnWidth) -
      isp_out->right_stripe_offset;
    pcmd->rgnHOffset = (bg_stats->num_right_rgns > 0) ?
      FLOOR2(pcmd->rgnHOffset) : 0;
  }


  ISP_DBG("<dual_dbg num_left_rgn %d, num_rught_rgn %d>",
    bg_stats->num_left_rgns, bg_stats->num_right_rgns);

  return TRUE;
}

/** bg_stats44_fill_stats_parser_params:
 *
 *  @bg_stats: bg stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bg_stats44_fill_stats_parser_params(bg_stats44_t *bg_stats,
  isp_saved_stats_params_t *stats_params)
{
  ISP_StatsBg_CfgCmdType    *pcmd = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_stats_roi_params_t    *roi_params = NULL;
  aec_bg_config_t           *bg_config = NULL;
  isp_stats_config_t        *stats_config = NULL;

  if (!bg_stats || !stats_params) {
    ISP_ERR("failed: %p %p", bg_stats, stats_params);
    return FALSE;
  }

  pcmd = &bg_stats->pcmd;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BG];
  bg_config = &bg_stats->bg_config;

  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_BG);
  stats_config->aec_config.bg_config = bg_stats->bg_config;

  /*not chqanged between single vfe and dual vfe*/
  roi_params = &stats_params->stats_roi_params[MSM_ISP_STATS_BG];
  roi_params->rgnHeight = pcmd->rgnHeight;
  roi_params->rgnWidth = pcmd->rgnWidth;
  roi_params->rMax = bg_config->r_Max;
  roi_params->bMax = bg_config->b_Max;
  roi_params->grMax = bg_config->gr_Max;
  roi_params->gbMax = bg_config->gb_Max;

  /* Fill ROI parm and parsing parm*/
  rgns_stats->is_valid = TRUE;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BG];
  if (bg_stats->isp_out_info.is_split == TRUE) {
     /*DUAL VFE case*/
    if (bg_stats->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
      rgns_stats->h_rgns_start = 0;
      rgns_stats->h_rgns_end = bg_stats->num_left_rgns - 1;
      rgns_stats->h_rgns_total =
        bg_stats->num_left_rgns + bg_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    } else {
      rgns_stats->h_rgns_start = bg_stats->num_left_rgns;
      rgns_stats->h_rgns_end =
        bg_stats->num_left_rgns + bg_stats->num_right_rgns - 1;
      rgns_stats->h_rgns_total =
        bg_stats->num_left_rgns + bg_stats->num_right_rgns;
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

  return TRUE;
}

/** bg_stats44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for bg_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bg_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  ISP_StatsBg_CfgCmdType  *pcmd = NULL;
  aec_bg_config_t         *bg_config = NULL;
  uint32_t                 bg_rgn_width = 0,
                           bg_rgn_height = 0;
  bg_stats44_t            *bg_stats = NULL;
  isp_private_event_t     *private_event = NULL;
  uint32_t                 camif_window_w_t = 0;
  uint32_t                 camif_window_h_t = 0;
  sensor_out_info_t       *sensor_out_info = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  isp_stats_config_t      *stats_config = NULL;
  isp_rgns_stats_param_t  *rgns_stats = NULL;
  isp_stats_roi_params_t  *roi_params = NULL;

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

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    goto ERROR;
  }

  pcmd = &bg_stats->pcmd;
  bg_config = &bg_stats->bg_config;
  sensor_out_info = &bg_stats->sensor_out_info;

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
    ((!pcmd->rgnHeight) && (!pcmd->rgnWidth))) {

    isp_sub_module->trigger_update_pending = TRUE;
  }

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("BG not enabled / trigger update pending false");

    /* Fill bg_config */
    if (sub_module_output->stats_params) {
      ret = bg_stats44_fill_stats_parser_params(bg_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: bg_stats44_fill_stats_parser_params");
      }
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((bg_config->grid_info.h_num == 0) || (bg_config->grid_info.v_num == 0)) {
    ISP_ERR("Invalid BG h_num & v_num from 3A, h_num = %d, v_num = %d\n",
      bg_config->grid_info.h_num, bg_config->grid_info.v_num);
    goto ERROR;
  }

  camif_window_w_t = sensor_out_info->request_crop.last_pixel -
     sensor_out_info->request_crop.first_pixel + 1;

  camif_window_h_t = sensor_out_info->request_crop.last_line -
     sensor_out_info->request_crop.first_line + 1;

  if((bg_config->roi.top + bg_config->roi.height > camif_window_h_t) ||
    (bg_config->roi.left + bg_config->roi.width > camif_window_w_t) ||
    (bg_config->roi.height == 0 && bg_config->roi.width == 0)){
    ISP_ERR("Invalid BG ROI %d %d %d %d", bg_config->roi.left,
        bg_config->roi.top, bg_config->roi.width, bg_config->roi.height);
    goto ERROR;
  }

  if (bg_config->roi.width / bg_config->grid_info.h_num <
       MIN_RGN_WIDTH){
    bg_rgn_width = MIN_RGN_WIDTH;
    bg_config->grid_info.h_num = bg_config->roi.width/bg_rgn_width ;
  } else {
    bg_rgn_width =
       bg_config->roi.width / bg_config->grid_info.h_num;
  }

  if (bg_config->roi.height / bg_config->grid_info.v_num <
        MIN_RGN_HEIGHT){
     bg_rgn_height = MIN_RGN_HEIGHT;
     bg_config->grid_info.v_num = bg_config->roi.height/bg_rgn_height;
  } else {
     bg_rgn_height =
        bg_config->roi.height / bg_config->grid_info.v_num;
  }

  ISP_DBG("bg_rgn_width %d bg_rgn_height %d", bg_rgn_width, bg_rgn_height);

  pcmd->rgnHOffset = FLOOR2(bg_config->roi.left);
  pcmd->rgnVOffset = FLOOR2(bg_config->roi.top);
  pcmd->rgnWidth = MAX((int)(FLOOR2(bg_rgn_width) - 1), 1);
  pcmd->rgnHeight = MAX((int)(FLOOR2(bg_rgn_height) - 1), 1);
  pcmd->rgnHNum = MAX((bg_config->grid_info.h_num - 1) , 1);
  pcmd->rgnVNum = MAX((bg_config->grid_info.v_num - 1), 1);
  pcmd->rMax = bg_config->r_Max;
  pcmd->grMax = bg_config->gr_Max;
  pcmd->bMax = bg_config->b_Max;
  pcmd->gbMax = bg_config->gb_Max;

  /* update aecConfig to reflect the new config    *
     It will be sent to 3A in STATS_NOTIFY event  */
  bg_config->roi.left = pcmd->rgnHOffset;
  bg_config->roi.top = pcmd->rgnVOffset;
  bg_config->roi.width = (pcmd->rgnWidth + 1) * bg_config->grid_info.h_num;
  bg_config->roi.height = (pcmd->rgnHeight + 1) * bg_config->grid_info.v_num;

  if (bg_stats->isp_out_info.is_split == TRUE) {
     ret =  bg_stats44_split_config(
       bg_stats, pcmd->rgnHNum + 1, pcmd->rgnHOffset, pcmd->rgnWidth + 1);
     if (ret == FALSE) {
       ISP_ERR("failed bg_stats44_split_config");
       goto ERROR;
     }
  }

  if (sub_module_output->stats_params) {
    ret = bg_stats44_fill_stats_parser_params(bg_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: bg_stats44_fill_stats_parser_params");
    }
  }

   ret = bg_stats44_store_hw_update(isp_sub_module, bg_stats);
   if (ret == FALSE) {
     ISP_ERR("failed: bg_stats44_store_hw_update");
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
} /* bg_stats44_trigger_update */

/** bg_stats44_stats_config_validate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bg_stats44_stats_config_validate(bg_stats44_t *bg_stats,
  aec_bg_config_t *bg_config)

{
    uint32_t                 camif_window_w_t = 0;
    uint32_t                 camif_window_h_t = 0;
    sensor_out_info_t       *sensor_out_info = NULL;

    RETURN_IF_NULL(bg_stats);
    RETURN_IF_NULL(bg_config);
    sensor_out_info = &bg_stats->sensor_out_info;

    if ((bg_config->grid_info.h_num == 0) ||
      (bg_config->grid_info.v_num == 0)) {
      ISP_ERR("Invalid BG from 3A h_num = %d, v_num = %d\n",
        bg_config->grid_info.h_num, bg_config->grid_info.v_num);
      goto ERROR;
    }

    camif_window_w_t = sensor_out_info->request_crop.last_pixel -
       sensor_out_info->request_crop.first_pixel + 1;

    camif_window_h_t = sensor_out_info->request_crop.last_line -
       sensor_out_info->request_crop.first_line + 1;

    if((bg_config->roi.top + bg_config->roi.height > camif_window_h_t) ||
      (bg_config->roi.left + bg_config->roi.width > camif_window_w_t) ||
      (bg_config->roi.width < MIN_RGN_WIDTH) ||
      (bg_config->roi.height < MIN_RGN_HEIGHT)){
      ISP_ERR("Invalid BG ROI from 3A %d %d %d %d", bg_config->roi.left,
          bg_config->roi.top, bg_config->roi.width, bg_config->roi.height);
      goto ERROR;
    }
    return TRUE;

ERROR:
    return FALSE;
}

/** bg_stats44_stats_awb_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bg_stats44_stats_awb_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bg_stats44_t   *bg_stats = NULL;
  awb_config_t   *awb_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  awb_config = (awb_config_t *)data;
  if (!awb_config->bg_config.is_valid) {
    ISP_HIGH("BG stats is not valid, dont update");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (memcmp(&awb_config->bg_config, &bg_stats->bg_config,
    sizeof(aec_bg_config_t))) {
    isp_sub_module->trigger_update_pending = TRUE;
    if (bg_stats44_stats_config_validate(bg_stats, &awb_config->bg_config)) {
      /* Copy only valid ROI from 3A */
      bg_stats->bg_config = awb_config->bg_config;
    } else {
        /* Received invalid ROI from 3A, use default */
      ISP_HIGH("Use default BG h_num & v_num from 3A, h_num = %d, v_num = %d\n",
        bg_stats->bg_config.grid_info.h_num,
        bg_stats->bg_config.grid_info.v_num);
      ISP_HIGH("Using default BG ROI %d %d %d %d",
        bg_stats->bg_config.roi.left,
        bg_stats->bg_config.roi.top,
        bg_stats->bg_config.roi.width,
        bg_stats->bg_config.roi.height);
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}
/** bg_stats44_stats_aec_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bg_stats44_stats_aec_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bg_stats44_t   *bg_stats = NULL;
  aec_config_t   *aec_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  aec_config = (aec_config_t *)data;

  if (!aec_config->bg_config.is_valid) {
    ISP_HIGH("BG stats is not valid, dont update");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (memcmp(&aec_config->bg_config, &bg_stats->bg_config,
    sizeof(aec_bg_config_t))) {
    isp_sub_module->trigger_update_pending = TRUE;
    if (bg_stats44_stats_config_validate(bg_stats, &aec_config->bg_config)) {
      /* Copy only valid ROI from 3A */
      bg_stats->bg_config = aec_config->bg_config;
    } else {
        /* Received invalid ROI from 3A, use default */
      ISP_HIGH("Use default BG h_num & v_num from 3A, h_num = %d, v_num = %d\n",
        bg_stats->bg_config.grid_info.h_num,
        bg_stats->bg_config.grid_info.v_num);
      ISP_HIGH("Using default BG ROI %d %d %d %d",
        bg_stats->bg_config.roi.left,
        bg_stats->bg_config.roi.top,
        bg_stats->bg_config.roi.width,
        bg_stats->bg_config.roi.height);
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bg_stats44_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bg_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bg_stats44_t            *bg_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  ISP_StatsBg_CfgCmdType  *pcmd = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bg_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bg_stats44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bg_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bg_stats44_t            *bg_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  ISP_StatsBg_CfgCmdType  *pcmd = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bg_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bg_stats44_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bg_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bg_stats44_t            *bg_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  ISP_StatsBg_CfgCmdType  *pcmd = NULL;
  aec_bg_config_t         *bg_config;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bg_stats->sensor_out_info = *sensor_out_info;

  bg_config = &bg_stats->bg_config;
  bg_config->grid_info.h_num = 64;
  bg_config->grid_info.v_num = 48;
  bg_config->roi.left = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1) % 64;
  bg_config->roi.top = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1) % 48;

  bg_config->roi.width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1) - bg_config->roi.left;

  bg_config->roi.height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1) - bg_config->roi.top;

   bg_config->roi.left = FLOOR2(bg_config->roi.left / 2);
   bg_config->roi.top = FLOOR2(bg_config->roi.top / 2);


  bg_config->r_Max  = ((1 << ISP_PIPELINE_WIDTH) - 1);
  bg_config->gr_Max = ((1 << ISP_PIPELINE_WIDTH) - 1);
  bg_config->b_Max  = ((1 << ISP_PIPELINE_WIDTH) - 1);
  bg_config->gb_Max = ((1 << ISP_PIPELINE_WIDTH) - 1);

  pcmd = &bg_stats->pcmd;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = 63;
  pcmd->rgnVNum    = 47;
  pcmd->rMax       = bg_config->r_Max;
  pcmd->grMax      = bg_config->gr_Max;
  pcmd->bMax       = bg_config->b_Max;
  pcmd->gbMax      = bg_config->gb_Max;

  ISP_DBG("h_num %d v_num %d", bg_config->grid_info.h_num,
    bg_config->grid_info.v_num);
  ISP_DBG("roi %d %d %d %d", bg_config->roi.left, bg_config->roi.top,
    bg_config->roi.width, bg_config->roi.height);
  ISP_DBG("r_Max %d gr_Max %d b_Max %d gb_Max %d", bg_config->r_Max,
    bg_config->gr_Max, bg_config->b_Max, bg_config->gb_Max);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bg_stats44_set_stream_config */

/** bg_stats44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bg_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bg_stats44_t *bg_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)isp_sub_module->private_data;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bg_stats, 0, sizeof(*bg_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bg_stats44_streamoff */

/** bg_stats44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bg_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bg_stats44_init(isp_sub_module_t *isp_sub_module)
{
  bg_stats44_t *bg_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bg_stats = (bg_stats44_t *)CAM_MALLOC(sizeof(bg_stats44_t));
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats %p", bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(bg_stats, 0, sizeof(*bg_stats));

  isp_sub_module->private_data = (void *)bg_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* bg_stats44_init */

/** bg_stats44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void bg_stats44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* bg_stats44_destroy */
