/* aec_bg_stats47.c
 *
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "aec_bg_stats47.h"
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

/* #define AEC_BG_STATS47_DEBUG */
#ifdef AEC_BG_STATS47_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR

#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

#define AEC_BG_STATS47_MIN_WIDTH 6
#define AEC_BG_STATS47_MIN_HEIGHT 2

/** aec_bg_stats47_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void aec_bg_stats47_debug(ISP_StatsAecBg_CfgCmdType *pcmd)
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
  ISP_DBG("gbMax      %d", pcmd->gbMin);
  ISP_DBG("grMax      %d", pcmd->grMin);
  ISP_DBG("rMax       %d", pcmd->rMin);
  ISP_DBG("bMax       %d", pcmd->bMin);
}

/** aec_bg_stats47_stats_config_validate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_stats_config_validate_be(aec_bg_stats47_t *bg_stats,
    aec_be_config_t *bg_config )

{
    int32_t                 camif_window_w_t = 0;
    int32_t                 camif_window_h_t = 0;
    sensor_out_info_t       *sensor_out_info = NULL;

    RETURN_IF_NULL(bg_stats);
    RETURN_IF_NULL(bg_config);
    sensor_out_info = &bg_stats->sensor_out_info;


    if ((bg_config->grid_info.h_num == 0) ||
      (bg_config->grid_info.v_num == 0)) {
      ISP_HIGH("Invalid BG from 3A h_num = %d, v_num = %d\n",
        bg_config->grid_info.h_num, bg_config->grid_info.v_num);
      goto ERROR;
    }

    camif_window_w_t = sensor_out_info->request_crop.last_pixel -
       sensor_out_info->request_crop.first_pixel + 1;

    camif_window_h_t = sensor_out_info->request_crop.last_line -
       sensor_out_info->request_crop.first_line + 1;

    if((bg_config->roi.top + bg_config->roi.height > camif_window_h_t) ||
      (bg_config->roi.left + bg_config->roi.width > camif_window_w_t)){
      ISP_ERR("Invalid BG ROI from 3A %d %d %d %d", bg_config->roi.left,
          bg_config->roi.top, bg_config->roi.width, bg_config->roi.height);
      goto ERROR;
    }

    return TRUE;

ERROR:
    return FALSE;
}

/** aec_bg_stats47_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @aec_bg_stats: bg stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean aec_bg_stats47_store_hw_update(isp_sub_module_t *isp_sub_module,
  aec_bg_stats47_t *aec_bg_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsAecBg_CfgCmdType      *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsAecBg_CfgCmdType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                     num_reg_cfg_cmd = 1+AEC_BG_QUAD_SYNC_ENABLE;

  if (!isp_sub_module || !aec_bg_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, aec_bg_stats);
    return FALSE;
  }

  pcmd = &aec_bg_stats->pcmd;
  aec_bg_stats47_debug(pcmd);

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

  copy_cmd = (ISP_StatsAecBg_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
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

  if (AEC_BG_QUAD_SYNC_ENABLE) {
    reg_cfg_cmd->u.mask_info.reg_offset = STATS_CFG_OFF_AEC_BG;
    reg_cfg_cmd->u.mask_info.mask       = STATS_CFG_MASK_AEC_BG;
    reg_cfg_cmd->u.mask_info.val        = STATS_CFG_VAL_AEC_BG;
    reg_cfg_cmd->cmd_type               = VFE_CFG_MASK;
    ++reg_cfg_cmd;
  }

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = AEC_BG_STATS_OFF;
  reg_cfg_cmd->u.rw_info.len = AEC_BG_STATS_LEN * sizeof(uint32_t);

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

/** aec_bg_stats47_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean aec_bg_stats47_split_config(aec_bg_stats47_t *aec_bg_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset, uint32_t single_rgnWidth)
{
   ISP_StatsAecBg_CfgCmdType *pcmd = NULL;
   uint32_t                left_stripe_stats_bound = 0;
   uint32_t                overlap = 0;
   isp_out_info_t          *isp_out = NULL;

  if (!aec_bg_stats) {
    ISP_ERR("NULL pointer! bg %p", aec_bg_stats);
    return FALSE;
  }

  pcmd = &aec_bg_stats->pcmd;
  overlap = aec_bg_stats->ispif_out_info.overlap;
  isp_out = &aec_bg_stats->isp_out_info;

  /*using determined single VFE config to further determine dual vfe config*/
    /*find out left stripe stats boundry*/
  left_stripe_stats_bound =
    single_rgnHOffset + (single_rgnWidth * single_rgnHNum);

  if (left_stripe_stats_bound > isp_out->right_stripe_offset + overlap)
    left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

  /*if single VFE request stats rgn go over left end,
    then no rgn required in left VFE,
    or we need to calculate the rgn num for left VFE*/
  aec_bg_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_bound) ?
     0: (left_stripe_stats_bound - single_rgnHOffset) / single_rgnWidth;

  /*calculate righ num rgns based on left VFE rgn*/
  aec_bg_stats->num_right_rgns = single_rgnHNum - aec_bg_stats->num_left_rgns;
  /* Check if the stat region satisfies the crucial assumption: there is
     at least one grid line within the overlap area if the whole region
     spans across both stripes */
  if (single_rgnHOffset + (aec_bg_stats->num_left_rgns * single_rgnWidth) <
    isp_out->right_stripe_offset && aec_bg_stats->num_right_rgns > 0) {
    ISP_ERR("Unable to support such stats region in dual-ISP mode");
    return FALSE;
  }

  if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
     /* programe 0 means 1 already*/
     pcmd->rgnHNum  = (aec_bg_stats->num_left_rgns > 0) ?
       aec_bg_stats->num_left_rgns - 1: 0;
   /* limitation is even number*/
     if (aec_bg_stats->num_left_rgns > 0)
       pcmd->rgnHOffset = (aec_bg_stats->num_left_rgns > 0) ?
         FLOOR2(single_rgnHOffset) : 0;
  } else {
      /* programe 0 means 1 already*/
    pcmd->rgnHNum = (aec_bg_stats->num_right_rgns > 0) ?
       aec_bg_stats->num_right_rgns - 1 : 0;

    /*right offset from left stats rgn info,
      HW limit: even number*/
    pcmd->rgnHOffset = single_rgnHOffset +
      (aec_bg_stats->num_left_rgns * single_rgnWidth) -
      isp_out->right_stripe_offset;
    pcmd->rgnHOffset = (aec_bg_stats->num_right_rgns > 0) ?
      FLOOR2(pcmd->rgnHOffset) : 0;
  }


  ISP_DBG("<dual_dbg num_left_rgn %d, num_rught_rgn %d>",
    aec_bg_stats->num_left_rgns, aec_bg_stats->num_right_rgns);

  return TRUE;
}

/** aec_bg_stats47_fill_stats_parser_params:
 *
 *  @aec_bg_stats: bg stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean aec_bg_stats47_fill_stats_parser_params(
  isp_sub_module_t *isp_sub_module, aec_bg_stats47_t *aec_bg_stats,
  isp_sub_module_output_t *sub_module_output, uint32_t camif_window_w,
  uint32_t camif_window_h)
{
  ISP_StatsAecBg_CfgCmdType   *pcmd = NULL;
  isp_rgns_stats_param_t      *rgns_stats = NULL;
  isp_stats_roi_params_t      *roi_params = NULL;
  aec_bg_config_t             *aec_bg_config = NULL;
  isp_stats_config_t          *stats_config = NULL;
  isp_tintless_stats_config_t *tintless_stats_config = NULL;
  isp_algo_params_t           *algo_params = NULL;
  isp_saved_stats_params_t    *stats_params = NULL;

  if (!isp_sub_module || !aec_bg_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, aec_bg_stats, sub_module_output);
    return FALSE;
  }

  pcmd = &aec_bg_stats->pcmd;
  algo_params = sub_module_output->algo_params;
  stats_params = sub_module_output->stats_params;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_AEC_BG];
  aec_bg_config = &aec_bg_stats->aec_bg_config;

  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_AEC_BG);
  stats_config->aec_config.aec_bg_config = aec_bg_stats->aec_bg_config;

  /*not chqanged between single vfe and dual vfe*/
  roi_params = &stats_params->stats_roi_params[MSM_ISP_STATS_AEC_BG];
  roi_params->rgnHeight = pcmd->rgnHeight;
  roi_params->rgnWidth = pcmd->rgnWidth;
  roi_params->rMax = aec_bg_config->r_Max;
  roi_params->bMax = aec_bg_config->b_Max;
  roi_params->grMax = aec_bg_config->gr_Max;
  roi_params->gbMax = aec_bg_config->gb_Max;

  /* Fill ROI parm and parsing parm*/
  rgns_stats->is_valid = TRUE;
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_AEC_BG];
  if (aec_bg_stats->isp_out_info.is_split == TRUE) {
     /*DUAL VFE case*/
    if (aec_bg_stats->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
      rgns_stats->h_rgns_start = 0;
      rgns_stats->h_rgns_end = aec_bg_stats->num_left_rgns - 1;
      rgns_stats->h_rgns_total =
        aec_bg_stats->num_left_rgns + aec_bg_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->rgnVNum + 1;
    } else {
      rgns_stats->h_rgns_start = aec_bg_stats->num_left_rgns;
      rgns_stats->h_rgns_end =
        aec_bg_stats->num_left_rgns + aec_bg_stats->num_right_rgns - 1;
      rgns_stats->h_rgns_total =
        aec_bg_stats->num_left_rgns + aec_bg_stats->num_right_rgns;
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
      FLOOR2(camif_window_w / ISP_STATS_AEC_BG_GRID_H);
    tintless_stats_config->stat_elem_h =
      FLOOR2(camif_window_h / ISP_STATS_AEC_BG_GRID_V);
    tintless_stats_config->num_stat_elem_cols = ISP_STATS_AEC_BG_GRID_H;
    tintless_stats_config->num_stat_elem_rows = ISP_STATS_AEC_BG_GRID_V;
    tintless_stats_config->saturation_limit = 255 - PIX_VALUE_OFFSET;
    tintless_stats_config->stats_type = HW_STATS_TYPE;
  }

  return TRUE;
}

/** aec_bg_stats47_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for aec_bg_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  ISP_StatsAecBg_CfgCmdType  *pcmd = NULL;
  aec_bg_config_t         *aec_bg_config = NULL;
  uint32_t                 bg_rgn_width = 0,
                           bg_rgn_height = 0;
  aec_bg_stats47_t            *aec_bg_stats = NULL;
  isp_private_event_t     *private_event = NULL;
  int32_t                 camif_window_w_t = 0;
  int32_t                 camif_window_h_t = 0;
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

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    goto ERROR;
  }

  pcmd = &aec_bg_stats->pcmd;
  aec_bg_config = &aec_bg_stats->aec_bg_config;
  sensor_out_info = &aec_bg_stats->sensor_out_info;

  camif_window_w_t = sensor_out_info->request_crop.last_pixel -
     sensor_out_info->request_crop.first_pixel + 1;

  camif_window_h_t = sensor_out_info->request_crop.last_line -
     sensor_out_info->request_crop.first_line + 1;

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
    ((!pcmd->rgnHeight) && (!pcmd->rgnWidth))) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("BG not enabled / trigger update pending false");

    /* Fill aec_aec_bg_config */
    if (sub_module_output->stats_params) {
      ret = aec_bg_stats47_fill_stats_parser_params(isp_sub_module, aec_bg_stats,
        sub_module_output, camif_window_w_t, camif_window_h_t);
      if (ret == FALSE) {
        ISP_ERR("failed: aec_bg_stats47_fill_stats_parser_params");
      }
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((aec_bg_config->grid_info.h_num != 0) && (aec_bg_config->grid_info.v_num != 0)) {

    if((aec_bg_config->roi.top + aec_bg_config->roi.height > camif_window_h_t) ||
       (aec_bg_config->roi.left + aec_bg_config->roi.width > camif_window_w_t)||
       (aec_bg_config->roi.width == 0) || ( aec_bg_config->roi.height == 0) ){
        ISP_ERR("Invalid BG ROI %d %d %d %d", aec_bg_config->roi.left,
          aec_bg_config->roi.top, aec_bg_config->roi.width, aec_bg_config->roi.height);
        goto ERROR;
    }

    if ( aec_bg_config->roi.width / aec_bg_config->grid_info.h_num <
      AEC_BG_STATS47_MIN_WIDTH) {
      uint32_t new_h_num = 0;
      bg_rgn_width = AEC_BG_STATS47_MIN_WIDTH;
      new_h_num = aec_bg_config->roi.width /bg_rgn_width;

      ISP_ERR("Invalid ROI and GRID setting: roi.width %d grid_info.h_num %d",
        aec_bg_config->roi.width, aec_bg_config->grid_info.h_num);

      if (new_h_num > 0) {
        aec_bg_config->grid_info.h_num = new_h_num;
      } else {
        ISP_ERR("Invalid ROI and GRID setting, force h_num to 1");
        aec_bg_config->grid_info.h_num = 1;
      }
    } else {
       bg_rgn_width =
         aec_bg_config->roi.width / aec_bg_config->grid_info.h_num;
    }

    if ( aec_bg_config->roi.height / aec_bg_config->grid_info.v_num <
      AEC_BG_STATS47_MIN_HEIGHT) {
      uint32_t new_v_num = 0;
      bg_rgn_height = AEC_BG_STATS47_MIN_HEIGHT;
      new_v_num = aec_bg_config->roi.height /bg_rgn_height;

      ISP_ERR("Invalid ROI and GRID setting: roi.height %d grid_info.v_num %d",
        aec_bg_config->roi.height, aec_bg_config->grid_info.v_num);

      if (new_v_num > 0) {
        aec_bg_config->grid_info.v_num = new_v_num;
      } else {
        ISP_ERR("Invalid ROI and GRID setting, force v_num to 1");
        aec_bg_config->grid_info.v_num = 1;
      }
    } else {
       bg_rgn_height =
         aec_bg_config->roi.height / aec_bg_config->grid_info.v_num;
    }

    ISP_DBG("bg_rgn_width %d bg_rgn_height %d", bg_rgn_width, bg_rgn_height);

    pcmd->rgnHOffset = FLOOR2(aec_bg_config->roi.left);
    pcmd->rgnVOffset = FLOOR2(aec_bg_config->roi.top);
    pcmd->rgnWidth = FLOOR2(bg_rgn_width) - 1;
    pcmd->rgnHeight = FLOOR2(bg_rgn_height) - 1;
    pcmd->rgnHNum = MAX((aec_bg_config->grid_info.h_num - 1) , 1);
    pcmd->rgnVNum = MAX((aec_bg_config->grid_info.v_num - 1), 1);
    pcmd->rMin  = 0;
    pcmd->grMin = 0;
    pcmd->bMin  = 0;
    pcmd->gbMin = 0;
    pcmd->rMax  = aec_bg_config->r_Max;
    pcmd->grMax = aec_bg_config->gr_Max;
    pcmd->bMax  = aec_bg_config->b_Max;
    pcmd->gbMax = aec_bg_config->gb_Max;

    /* update aecConfig to reflect the new config    *
       It will be sent to 3A in STATS_NOTIFY event  */
    aec_bg_config->roi.left = pcmd->rgnHOffset;
    aec_bg_config->roi.top = pcmd->rgnVOffset;
    aec_bg_config->roi.width = (pcmd->rgnWidth + 1) * aec_bg_config->grid_info.h_num;
    aec_bg_config->roi.height = (pcmd->rgnHeight + 1) * aec_bg_config->grid_info.v_num;

    if (aec_bg_stats->isp_out_info.is_split == TRUE) {
       ret =  aec_bg_stats47_split_config(
         aec_bg_stats, pcmd->rgnHNum + 1, pcmd->rgnHOffset, pcmd->rgnWidth + 1);
       if (ret == FALSE) {
         ISP_ERR("failed aec_bg_stats47_split_config");
         goto ERROR;
       }
    }
  } else {

    pcmd->rgnWidth   = FLOOR2(camif_window_w_t / ISP_STATS_AEC_BG_GRID_H) - 1;
    pcmd->rgnHeight  = FLOOR2(camif_window_h_t / ISP_STATS_AEC_BG_GRID_V) - 1;
    pcmd->rgnHOffset = FLOOR2((camif_window_w_t - ISP_STATS_AEC_BG_GRID_H *
      (pcmd->rgnWidth + 1)) / 2);
    pcmd->rgnVOffset = FLOOR2((camif_window_h_t - ISP_STATS_AEC_BG_GRID_V *
      (pcmd->rgnHeight + 1)) / 2);
    pcmd->rgnHNum    = ISP_STATS_AEC_BG_GRID_H - 1;
    pcmd->rgnVNum    = ISP_STATS_AEC_BG_GRID_V - 1;
    pcmd->rMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->grMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->bMax       = ((1 << ISP_PIPELINE_WIDTH) - 1);
    pcmd->gbMax      = ((1 << ISP_PIPELINE_WIDTH) - 1);

    /* update be_config what we applied */
    aec_bg_config->roi.width = (pcmd->rgnWidth + 1) * ISP_STATS_AEC_BG_GRID_H;
    aec_bg_config->roi.height = (pcmd->rgnHeight + 1) * ISP_STATS_AEC_BG_GRID_V;
    aec_bg_config->roi.left = pcmd->rgnHOffset;
    aec_bg_config->roi.top = pcmd->rgnVOffset;
    aec_bg_config->grid_info.h_num = pcmd->rgnHNum + 1;
    aec_bg_config->grid_info.v_num = pcmd->rgnVNum + 1;
    aec_bg_config->r_Max = pcmd->rMax;
    aec_bg_config->gr_Max = pcmd->rMax;
    aec_bg_config->b_Max = pcmd->rMax;
    aec_bg_config->gb_Max = pcmd->rMax;
  }

  if (sub_module_output->stats_params) {
    ret = aec_bg_stats47_fill_stats_parser_params(isp_sub_module, aec_bg_stats,
      sub_module_output, camif_window_w_t, camif_window_h_t);
    if (ret == FALSE) {
      ISP_ERR("failed: aec_bg_stats47_fill_stats_parser_params");
    }
  }

  ret = aec_bg_stats47_store_hw_update(isp_sub_module, aec_bg_stats);
  if (ret == FALSE) {
     ISP_ERR("failed: aec_bg_stats47_store_hw_update");
     goto ERROR;
  }

  if (isp_sub_module->config_pending == TRUE) {
    ret = aec_bg_stats_ext47_hw_update(isp_sub_module, aec_bg_stats);
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
} /* aec_bg_stats47_trigger_update */

/** aec_bg_stats47_stats_config_validate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_stats_config_validate(aec_bg_stats47_t *bg_stats,
    aec_bg_config_t *bg_config )

{
    int32_t                 camif_window_w_t = 0;
    int32_t                 camif_window_h_t = 0;
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
      (bg_config->roi.left + bg_config->roi.width > camif_window_w_t)){
      ISP_ERR("Invalid BG ROI from 3A %d %d %d %d", bg_config->roi.left,
          bg_config->roi.top, bg_config->roi.width, bg_config->roi.height);
      goto ERROR;
    }

    return TRUE;

ERROR:
    return FALSE;
}

/** aec_bg_stats47_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_bg_stats47_t   *aec_bg_stats = NULL;
  aec_config_t       *aec_config = NULL;
  aec_bg_config_t    *aec_bg_config;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  aec_config = (aec_config_t *)data;
  if (!aec_config) {
    return TRUE;
  }

  if (!aec_config->aec_bg_config.is_valid) {
    ISP_DBG("AEC BG stats is not valid, dont update");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (HW_STATS_TYPE == MSM_ISP_STATS_AEC_BG) {
    if (memcmp(&aec_config->aec_bg_config, &aec_bg_stats->aec_bg_config,
      sizeof(aec_bg_config_t))) {
        if (aec_bg_stats47_stats_config_validate(aec_bg_stats,
             &aec_config->aec_bg_config)) {
          aec_bg_stats->aec_bg_config = aec_config->aec_bg_config;
          isp_sub_module->trigger_update_pending = TRUE;
        } else {
          /* Received invalid ROI from 3A, use default */
          ISP_HIGH("Use default BG h_num & v_num from 3A, h_num = %d, v_num = %d\n",
            aec_bg_stats->grid_info.h_num,
            aec_bg_stats->grid_info.v_num);
          ISP_HIGH("Using default BG ROI %d %d %d %d",
            aec_bg_stats->roi.left,
            aec_bg_stats->roi.top,
            aec_bg_stats->roi.width,
            aec_bg_stats->roi.height);
        }
      }
   } else if (HW_STATS_TYPE == MSM_ISP_STATS_HDR_BE) {
       if (memcmp(&aec_config->be_config, &aec_bg_stats->aec_bg_config,
        sizeof(aec_bg_config_t))) {
        if ( aec_config->be_config.is_valid &&
             aec_bg_stats47_stats_config_validate_be(aec_bg_stats,
             &aec_config->be_config)) {
          memcpy(&aec_bg_stats->aec_bg_config ,&aec_config->be_config, sizeof(aec_bg_config_t));
          isp_sub_module->trigger_update_pending = TRUE;
        } else {
            /* Received invalid ROI from 3A, use default */
          ISP_DBG("Use default BG h_num & v_num from 3A, h_num = %d, v_num = %d\n",
            aec_bg_stats->grid_info.h_num,
            aec_bg_stats->grid_info.v_num);
          ISP_DBG("Using default BG ROI %d %d %d %d",
            aec_bg_stats->roi.left,
            aec_bg_stats->roi.top,
            aec_bg_stats->roi.width,
            aec_bg_stats->roi.height);
        }
      }
    } else
      ISP_ERR("Should not be here");

  aec_bg_config = &aec_bg_stats->aec_bg_config;
  ISP_DBG("r_max %u, roi height %u roi width %u",aec_bg_config->r_Max,
          aec_bg_config->roi.height, aec_bg_config->roi.width);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** aec_bg_stats47_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_bg_stats47_t        *aec_bg_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  ISP_StatsAecBg_CfgCmdType  *pcmd = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  aec_bg_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** aec_bg_stats47_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_bg_stats47_t        *aec_bg_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  ISP_StatsAecBg_CfgCmdType  *pcmd = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  aec_bg_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** aec_bg_stats47_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  aec_bg_stats47_t        *aec_bg_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  ISP_StatsAecBg_CfgCmdType  *pcmd = NULL;
  aec_bg_config_t         *aec_bg_config;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  aec_bg_stats->sensor_out_info = *sensor_out_info;

  aec_bg_config = &aec_bg_stats->aec_bg_config;
  aec_bg_config->grid_info.h_num = ISP_STATS_AEC_BG_GRID_H;
  aec_bg_config->grid_info.v_num = ISP_STATS_AEC_BG_GRID_V;
  aec_bg_config->roi.left = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1) % 64;
  aec_bg_config->roi.top = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1) % 48;

  aec_bg_config->roi.width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1) - aec_bg_config->roi.left;

  aec_bg_config->roi.height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1) - aec_bg_config->roi.top;

  aec_bg_config->roi.left = FLOOR2(aec_bg_config->roi.left / 2);
  aec_bg_config->roi.top = FLOOR2(aec_bg_config->roi.top / 2);
  aec_bg_config->r_Max  = ((1 << ISP_PIPELINE_WIDTH) - 1);
  aec_bg_config->gr_Max = ((1 << ISP_PIPELINE_WIDTH) - 1);
  aec_bg_config->b_Max  = ((1 << ISP_PIPELINE_WIDTH) - 1);
  aec_bg_config->gb_Max = ((1 << ISP_PIPELINE_WIDTH) - 1);

  pcmd = &aec_bg_stats->pcmd;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = ISP_STATS_AEC_BG_GRID_H - 1;
  pcmd->rgnVNum    = ISP_STATS_AEC_BG_GRID_V - 1;
  pcmd->rMax       = aec_bg_config->r_Max;
  pcmd->grMax      = aec_bg_config->gr_Max;
  pcmd->bMax       = aec_bg_config->b_Max;
  pcmd->gbMax      = aec_bg_config->gb_Max;
  pcmd->rMin       = 0;
  pcmd->grMin      = 0;
  pcmd->bMin       = 0;
  pcmd->gbMin      = 0;

  ISP_DBG("h_num %d v_num %d", aec_bg_config->grid_info.h_num,
    aec_bg_config->grid_info.v_num);
  ISP_DBG("roi %d %d %d %d", aec_bg_config->roi.left, aec_bg_config->roi.top,
    aec_bg_config->roi.width, aec_bg_config->roi.height);
  ISP_DBG("r_Max %d gr_Max %d b_Max %d gb_Max %d", aec_bg_config->r_Max,
    aec_bg_config->gr_Max, aec_bg_config->b_Max, aec_bg_config->gb_Max);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* aec_bg_stats47_set_stream_config */

/** aec_bg_stats47_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean aec_bg_stats47_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  aec_bg_stats47_t *aec_bg_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)isp_sub_module->private_data;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(aec_bg_stats, 0, sizeof(*aec_bg_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* aec_bg_stats47_streamoff */

/** aec_bg_stats47_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the aec_bg_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean aec_bg_stats47_init(isp_sub_module_t *isp_sub_module)
{
  aec_bg_stats47_t *aec_bg_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  aec_bg_stats = (aec_bg_stats47_t *)CAM_MALLOC(sizeof(aec_bg_stats47_t));
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats %p", aec_bg_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(aec_bg_stats, 0, sizeof(*aec_bg_stats));

  isp_sub_module->private_data = (void *)aec_bg_stats;
  isp_sub_module->hw_stats_type = HW_STATS_TYPE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* aec_bg_stats47_init */

/** aec_bg_stats47_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void aec_bg_stats47_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* aec_bg_stats47_destroy */
