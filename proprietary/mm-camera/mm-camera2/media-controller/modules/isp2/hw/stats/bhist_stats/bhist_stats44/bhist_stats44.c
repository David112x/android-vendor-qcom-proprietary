/* bhist_stats44.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "bhist_stats44.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BHIST_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BHIST_STATS, fmt, ##args)

#define BHIST_RGN_WIDTH 2
#define BHIST_RGN_HEIGHT 2

/** bhist_stats44_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void bhist_stats44_debug(ISP_StatsBhist_CfgCmdType *pcmd)
{
  if (!pcmd) {
    return;
  }
  ISP_DBG("BHIST Stats Configurations");
  ISP_DBG("rgnHOffset = %d", pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset = %d", pcmd->rgnVOffset);
  ISP_DBG("rgnHNum = %d", pcmd->rgnHNum);
  ISP_DBG("rgnVNum = %d", pcmd->rgnVNum);
}

/** bhist_stats44_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bhist_stats: bg stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bhist_stats44_store_hw_update(isp_sub_module_t *isp_sub_module,
  bhist_stats44_t *bhist_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsBhist_CfgCmdType   *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsBhist_CfgCmdType   *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !bhist_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bhist_stats);
    return FALSE;
  }

  pcmd = &bhist_stats->pcmd;
  bhist_stats44_debug(pcmd);

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

  copy_cmd = (ISP_StatsBhist_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
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
  reg_cfg_cmd[0].u.rw_info.reg_offset = BHIST_STATS_OFF;
  reg_cfg_cmd[0].u.rw_info.len = BHIST_STATS_LEN * sizeof(uint32_t);

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


/** bhist_stats44_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean bhist_stats44_split_config(bhist_stats44_t *bhist_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset)
{
  uint32_t                   left_stripe_stats_end = 0;
  uint32_t                   overlap = 0;
  ISP_StatsBhist_CfgCmdType  *pcmd = NULL;
  isp_out_info_t            *isp_out = NULL;

  if (!bhist_stats) {
    ISP_ERR("failed, NULL pointer bhist %p", bhist_stats);
    return FALSE;
  }

  overlap = bhist_stats->ispif_out_info.overlap;
  pcmd = &bhist_stats->pcmd;
  isp_out = &bhist_stats->isp_out_info;

  /*get left VFE stats end bound*/
  left_stripe_stats_end =
    single_rgnHOffset + (BHIST_RGN_WIDTH * single_rgnHNum);
  if (left_stripe_stats_end > isp_out->right_stripe_offset + overlap)
    left_stripe_stats_end = isp_out->right_stripe_offset + overlap;

  /*if rgn offset over end bound,
     means stats rgn fully fall on right stripe*/
   bhist_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_end) ?
     0 : (left_stripe_stats_end - single_rgnHOffset) / BHIST_RGN_WIDTH;

   bhist_stats->num_right_rgns =
     single_rgnHNum - bhist_stats->num_left_rgns;

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /* LEFT STRIPE, rgnHNum minimum program value 1 */
      pcmd->rgnHNum = (bhist_stats->num_left_rgns > 1) ?
        bhist_stats->num_left_rgns - 1: 1;

      pcmd->rgnHOffset = (bhist_stats->num_left_rgns > 0) ?
        FLOOR2(single_rgnHOffset) : 2; /* Default offset for BHIST STATS */
    } else {
       /* ISP_STRIPE_RIGHT, rgnHNum minimum program value 1 */
      pcmd->rgnHNum = (bhist_stats->num_right_rgns > 1) ?
        bhist_stats->num_right_rgns - 1 : 1;

       /*Right stripe RGN OFFSET*/
       pcmd->rgnHOffset = single_rgnHOffset + (bhist_stats->num_left_rgns * 2) -
         isp_out->right_stripe_offset;
       pcmd->rgnHOffset = (bhist_stats->num_right_rgns > 0) ?
         FLOOR2(pcmd->rgnHOffset) : 2; /* Default offset for BHIST STATS */
     }

    return TRUE;
}

/** bhist_stats44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for bhist_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                    ret = TRUE;
  ISP_StatsBhist_CfgCmdType *pcmd = NULL;
  bhist_stats44_t           *bhist_stats = NULL;
  isp_private_event_t       *private_event = NULL;
  sensor_out_info_t         *sensor_out_info = NULL;
  isp_sub_module_output_t   *sub_module_output = NULL;
  stats_grid_t              *out_grid_info = NULL;
  cam_rect_t                *roi = NULL,
                            *out_roi = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_saved_stats_params_t  *stats_params = NULL;
  unsigned int               i = 0;

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
  stats_params = sub_module_output->stats_params;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    goto ERROR;
  }

  pcmd = &bhist_stats->pcmd;

  if (isp_sub_module->submod_enable == FALSE) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  stats_params->bhist_meta_enable = bhist_stats->meta_enable;

  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BHIST];
  rgns_stats->is_valid = TRUE;

  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_HDR_BHIST];
  rgns_stats->is_valid = TRUE;

  if (isp_sub_module->trigger_update_pending == TRUE) {
    isp_sub_module->trigger_update_pending = FALSE;
    roi = &bhist_stats->roi;
    out_grid_info =
      &stats_params->stats_config.aec_config.bhist_config.grid_info;
    out_roi =
      &stats_params->stats_config.aec_config.bhist_config.roi;
    ISP_DBG("roi: left, top, wd, ht :[%d, %d, %d, %d]",
      roi->left, roi->top, roi->width, roi->height);

    if ((roi->left + roi->width) > bhist_stats->camif_window_width ||
      (roi->top + roi->height) > bhist_stats->camif_window_height) {
      ISP_ERR("Invalid roi: left top wd ht: [%d %d %d %d], camif [%d %d]",
        roi->left, roi->top, roi->width, roi->height,
        bhist_stats->camif_window_width,
        bhist_stats->camif_window_height);
        goto ERROR;
    }

    pcmd->rgnHOffset = FLOOR2(roi->left);
    pcmd->rgnVOffset = FLOOR2(roi->top);
    pcmd->rgnHNum    = (((roi->width / BHIST_RGN_WIDTH) - 1) < 0 ) ?
      0 : ((roi->width / BHIST_RGN_WIDTH) - 1);
    pcmd->rgnVNum    = (((roi->height / BHIST_RGN_HEIGHT) - 1) < 0) ?
      0 : ((roi->height / BHIST_RGN_HEIGHT) - 1);
    bhist_stats44_debug(pcmd);

    /* update aecConfig to reflect the new config   *
       It will be sent to 3A in STATS_NOTIFY event  */
    out_roi->left = pcmd->rgnHOffset;
    out_roi->top = pcmd->rgnVOffset;
    out_roi->width = (pcmd->rgnHNum + 1) * BHIST_RGN_WIDTH;
    out_roi->height = (pcmd->rgnVNum + 1) * BHIST_RGN_HEIGHT;
    out_grid_info->h_num = pcmd->rgnHNum + 1;
    out_grid_info->v_num = pcmd->rgnVNum + 1;

    /*if stream split to dual VFE, then do split config*/
    if (bhist_stats->isp_out_info.is_split == TRUE) {
      ret =  bhist_stats44_split_config(bhist_stats,
        pcmd->rgnHNum + 1, pcmd->rgnHOffset);
      if (ret == FALSE) {
        ISP_ERR("bhist_stats44_split_config failed!");
        goto ERROR;
      }
    }

    if (BHIST_CGC_OVERRIDE == TRUE) {
      ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
        BHIST_CGC_OVERRIDE_REGISTER, BHIST_CGC_OVERRIDE_BIT, TRUE);
      if (ret == FALSE) {
        ISP_ERR("failed: enable cgc");
      }
    }

    for (i = 0; i < NUM_DMI; ++i) {
      ret = isp_sub_module_util_reset_hist_dmi(isp_sub_module, DMI_RAM_START+i,
        ISP_DMI_CFG_DEFAULT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_util_reset_hist_dmi RAM0");
        goto ERROR;
      }
    }

    ret = bhist_stats44_store_hw_update(isp_sub_module, bhist_stats);
    if (ret == FALSE) {
      ISP_ERR("failed: bhist_stats44_store_hw_update");
      goto ERROR;
    }

    ret = bhist_stats_ext44_hw_update(isp_sub_module, bhist_stats);
    if (ret == FALSE) {
      ISP_ERR("failed: bhist_stats_ext44_hw_update");
    }

    if (BHIST_CGC_OVERRIDE == TRUE) {
      ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
        BHIST_CGC_OVERRIDE_REGISTER, BHIST_CGC_OVERRIDE_BIT, FALSE);
      if (ret == FALSE) {
        ISP_ERR("failed: enable cgc");
      }
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  /* Fill parser config */
  sub_module_output->stats_params->stats_config.bhist_config =
    bhist_stats->bhist_config;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  /* Fill parser config */
  sub_module_output->stats_params->stats_config.bhist_config =
    bhist_stats->bhist_config;

  return TRUE;

ERROR:

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return FALSE;
} /* bhist_stats44_trigger_update */

/** bhist_stats44_stats_config_validate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_stats_config_validate(bhist_stats44_t *bhist_stats,
  aec_config_t *aec_config)

{
    uint32_t                 camif_window_w_t = 0;
    uint32_t                 camif_window_h_t = 0;
    sensor_out_info_t       *sensor_out_info = NULL;
    aec_bhist_config_t         *bhist_config = NULL;

    RETURN_IF_NULL(bhist_stats);
    RETURN_IF_NULL(aec_config);
    bhist_config = &aec_config->bhist_config;

    camif_window_w_t = bhist_stats->camif_window_width;

    camif_window_h_t = bhist_stats->camif_window_height;
    if((bhist_config->roi.top + bhist_config->roi.height > (int)camif_window_h_t) ||
      (bhist_config->roi.left + bhist_config->roi.width > (int)camif_window_w_t)){
      ISP_ERR("Invalid BG ROI from 3A %d %d %d %d", bhist_config->roi.left,
          bhist_config->roi.top, bhist_config->roi.width, bhist_config->roi.height);
      goto ERROR;
    }
    return TRUE;

ERROR:
    return FALSE;
}

/** bhist_stats44_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_config_t
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bhist_stats44_t   *bhist_stats = NULL;
  aec_config_t      *aec_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  aec_config = (aec_config_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Check if bhist grid or roi has changed */
  if (memcmp(&aec_config->bhist_config,
    &bhist_stats->bhist_config,
    sizeof(aec_bhist_config_t))) {
      if (bhist_stats44_stats_config_validate(bhist_stats, aec_config)) {
          /* Copy only valid ROI from 3A */
        bhist_stats->bhist_config = aec_config->bhist_config;
        bhist_stats->roi= aec_config->bhist_config.roi;
        ISP_DBG("roi: wd x ht %d x %d, x, y %d x %d",
          bhist_stats->roi.width,bhist_stats->roi.height,
          bhist_stats->roi.left, bhist_stats->roi.top);
        isp_sub_module->trigger_update_pending = TRUE;
      } else {
          /* Received invalid ROI from 3A, use default */
        ISP_HIGH("Use default BE h_num & v_num from 3A, h_num = %d, v_num = %d\n",
          bhist_stats->grid_info.h_num,
          bhist_stats->grid_info.v_num);
        ISP_HIGH("Using default BE ROI %d %d %d %d",
          bhist_stats->roi.left,
          bhist_stats->roi.top,
          bhist_stats->roi.width,
          bhist_stats->roi.height);
      }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}


/** bhist_stats44_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bhist_stats44_t            *bhist_stats = NULL;
  ispif_out_info_t           *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bhist_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bhist_stats44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bhist_stats44_t         *bhist_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bhist_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bhist_stats44_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bhist_stats44_t            *bhist_stats = NULL;
  sensor_out_info_t          *sensor_out_info = NULL;
  ISP_StatsBhist_CfgCmdType  *pcmd = NULL;
  isp_stats_grid_t           *grid_info = NULL;
  cam_rect_t                 *roi = NULL;
  uint32_t                    camif_window_width = 0;
  uint32_t                    camif_window_height = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;
  if (!sensor_out_info) {
    ISP_ERR("failed: %p", sensor_out_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  camif_window_width =
    sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  camif_window_height =
    sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  bhist_stats->camif_window_width = camif_window_width;
  bhist_stats->camif_window_height= camif_window_height;

  roi = &bhist_stats->roi;
  if ((roi->left + roi->width) > bhist_stats->camif_window_width ||
      (roi->top + roi->height) > bhist_stats->camif_window_height) {
    grid_info = &bhist_stats->grid_info;
    grid_info->h_num = 64;
    grid_info->v_num = 48;

    roi->left = 0;
    roi->top = 0;
    roi->width = camif_window_width;
    roi->height = camif_window_height;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bhist_stats44_set_stream_config */

/** bhist_stats44_set_meta_enable:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: effect event to be applied
 *
 * Save the bhist meta enable flag
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_set_meta_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_event_control_parm_t *param = NULL;
  bhist_stats44_t          *bhist_stats = NULL;
  int32_t bhist_meta_enable = FALSE;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    return FALSE;
  }

  param = event->u.ctrl_event.control_event_data;
  bhist_stats->meta_enable = *(int32_t *)param->parm_data;

  ISP_DBG("Send BHIST stats in Meta? %d", bhist_stats->meta_enable);

  return ret;
}

/** bhist_stats44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bhist_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bhist_stats44_t *bhist_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)isp_sub_module->private_data;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bhist_stats, 0, sizeof(*bhist_stats));
  bhist_stats->bhist_config.is_bin_uniform = 1;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bhist_stats44_streamoff */

/** bhist_stats44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bhist_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bhist_stats44_init(isp_sub_module_t *isp_sub_module)
{
  bhist_stats44_t *bhist_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bhist_stats = (bhist_stats44_t *)CAM_MALLOC(sizeof(bhist_stats44_t));
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats %p", bhist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(bhist_stats, 0, sizeof(*bhist_stats));

  isp_sub_module->private_data = (void *)bhist_stats;
  bhist_stats->bhist_config.is_bin_uniform = 1;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* bhist_stats44_init */

/** bhist_stats44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void bhist_stats44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* bhist_stats44_destroy */
