/* ihist_stats44.c
 *
 * Copyright (c) 2013-2014, 2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "ihist_stats44.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_IHIST_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_IHIST_STATS, fmt, ##args)

#define IHIST_RGN_WIDTH 2
#define IHIST_RGN_HEIGHT 2

/** ihist_stats44_debug:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void ihist_stats44_debug(ISP_StatsIhist_CfgType *pcmd)
{
  if (!pcmd) {
    return;
  }
  ISP_DBG("Image histogram Stats Configurations");
  ISP_DBG("rgnHOffset    %d", pcmd->rgnHOffset);
  ISP_DBG("rgnVOffset    %d", pcmd->rgnVOffset);
  ISP_DBG("rgnHNum       %d", pcmd->rgnHNum);
  ISP_DBG("rgnVNum       %d", pcmd->rgnVNum);
  ISP_DBG("channelSelect %d", pcmd->channelSelect);
  ISP_DBG("shiftBits     %d", pcmd->shiftBits);
  ISP_DBG("siteSelect    %d", pcmd->siteSelect);
}

/** ihist_stats44_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @ihist_stats: bf stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean ihist_stats44_store_hw_update(isp_sub_module_t *isp_sub_module,
  ihist_stats44_t *ihist_stats)
{
  boolean                      ret = TRUE;
  ISP_StatsIhist_CfgType      *pcmd = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_StatsIhist_CfgType      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !ihist_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, ihist_stats);
    return FALSE;
  }

  pcmd = &ihist_stats->pcmd;
  ihist_stats44_debug(pcmd);

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

  copy_cmd = (ISP_StatsIhist_CfgType *)CAM_MALLOC(sizeof(*copy_cmd));
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
  reg_cfg_cmd[0].u.rw_info.reg_offset = IHIST_STATS_OFF;
  reg_cfg_cmd[0].u.rw_info.len = IHIST_STATS_LEN * sizeof(uint32_t);

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

/** ihist_stats44_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean ihist_stats44_split_config(ihist_stats44_t *ihist_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset)
{
  ISP_StatsIhist_CfgType *pcmd = NULL;
  isp_out_info_t         *isp_out = NULL;
  uint32_t                left_stripe_stats_end = 0;
  uint32_t                overlap = 0;

  if (!ihist_stats) {
    ISP_ERR("NULL pointer! ihist_stats %p", ihist_stats);
    return FALSE;
  }

  pcmd = &ihist_stats->pcmd;
  overlap = ihist_stats->ispif_out_info.overlap;
  isp_out = &ihist_stats->isp_out_info;

  /*find out left stripe stats boundry*/
  left_stripe_stats_end =
    single_rgnHOffset + IHIST_RGN_WIDTH * (single_rgnHNum);
  if (left_stripe_stats_end > isp_out->right_stripe_offset + overlap)
    left_stripe_stats_end = isp_out->right_stripe_offset + overlap;

  /*determine left_rgn_num and right_rgn_num
    if rgn offset go over left end, then completely fall on right VFE*/
  ihist_stats->num_left_rgns = (single_rgnHOffset > left_stripe_stats_end) ?
     0 : (left_stripe_stats_end - single_rgnHOffset) / IHIST_RGN_WIDTH;
  ihist_stats->num_right_rgns =
    single_rgnHNum - ihist_stats->num_left_rgns;

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
        /*check with system team*/
      pcmd->rgnHNum = (ihist_stats->num_left_rgns > 1) ?
        ihist_stats->num_left_rgns - 1 : 1;
      /* if still rgn in left then we config left VFE */
      pcmd->rgnHOffset = (ihist_stats->num_left_rgns > 0) ?
        single_rgnHOffset : 0;
    } else {
       /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (ihist_stats->num_right_rgns > 1) ?
        ihist_stats->num_right_rgns - 1 : 1;
      /*Right rgn offset*/
      pcmd->rgnHOffset = single_rgnHOffset +
        (ihist_stats->num_left_rgns * IHIST_RGN_WIDTH)
        - isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (ihist_stats->num_right_rgns > 0) ?
        pcmd->rgnHOffset : 0;
    }

    return TRUE;
}

/** ihist_stats44_stats_config_validate:
 *
 *  @ihist_statss: ihist stats handle
 *  @af_config: handle to af stats data
 *
 *  Validate af stats config event data
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_stats_config_validate(ihist_stats44_t *ihist_stats,
  af_config_t *af_config)

{
    uint32_t                 camif_window_w = 0;
    uint32_t                 camif_window_h = 0;
    sensor_out_info_t       *sensor_out_info = NULL;
    ihist_config_t          *ihist_config = NULL;

    RETURN_IF_NULL(ihist_stats);
    RETURN_IF_NULL(af_config);
    ihist_config = &af_config->ihist_config;

    sensor_out_info = (sensor_out_info_t *)&ihist_stats->sensor_out_info;
    if (!sensor_out_info) {
      ISP_ERR("failed: %p", sensor_out_info);
      return FALSE;
    }

    camif_window_w =
      sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
    camif_window_h =
      sensor_out_info->request_crop.last_line -
      sensor_out_info->request_crop.first_line + 1;

    if((ihist_config->roi.top + ihist_config->roi.height >
        (int)camif_window_h) ||
      (ihist_config->roi.left + ihist_config->roi.width >
        (int)camif_window_w) ||
      (ihist_config->roi.width == 0) ||
      (ihist_config->roi.height == 0)) {
      ISP_INFO("warning: Invalid IHIST ROI from 3A %d %d %d %d",
        ihist_config->roi.left, ihist_config->roi.top,
        ihist_config->roi.width, ihist_config->roi.height);
      goto ERROR;
    }
    return TRUE;

ERROR:
    return FALSE;
}

/** ihist_stats44_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: mct event handle for af stats
 *
 *  Handle af stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ihist_stats44_t   *ihist_stats = NULL;
  af_config_t      *af_config = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  af_config = (af_config_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  /* Check if bhist grid or roi has changed */
  if (af_config->ihist_config.is_valid &&
    memcmp(&af_config->ihist_config.roi, &ihist_stats->roi,
    sizeof(cam_rect_t))) {
      if (ihist_stats44_stats_config_validate(ihist_stats, af_config)) {
          /* Copy only valid ROI from 3A */
        ihist_stats->roi = af_config->ihist_config.roi;
        ISP_DBG("roi: wd x ht %d x %d, x, y %d x %d",
          ihist_stats->roi.width,ihist_stats->roi.height,
          ihist_stats->roi.left, ihist_stats->roi.top);
        isp_sub_module->trigger_update_pending = TRUE;
      } else {
          /* Received invalid ROI from 3A, use default */
        ISP_HIGH("Using default IHIST ROI %d %d %d %d",
          ihist_stats->roi.left,
          ihist_stats->roi.top,
          ihist_stats->roi.width,
          ihist_stats->roi.height);
      }
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** ihist_stats44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for ihist_stats
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                  ret = TRUE;
  int32_t                  i = 0;
  ihist_stats44_t         *ihist_stats = NULL;
  isp_stats_config_t      *stats_config = NULL;
  isp_private_event_t     *private_event = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  ISP_StatsIhist_CfgType  *pcmd = NULL;
  uint32_t                 total_pixels;
  int32_t                  shift_bits;
  ispif_out_info_t        *ispif_out_info = NULL;
  isp_out_info_t          *isp_out = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_saved_stats_params_t  *stats_params = NULL;

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

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
   goto ERROR;
  }

  /*pass ihist shift bit info to algo*/
  sub_module_output->algo_params->ihist_shift_bits =
    ihist_stats->pcmd.shiftBits;

  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_IHIST];
  rgns_stats->is_valid = TRUE;

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("IHIST not enabled %d / trigger update pending false %d",
      isp_sub_module->submod_enable,
      isp_sub_module->trigger_update_pending);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  pcmd = &ihist_stats->pcmd;
  ispif_out_info = &ihist_stats->ispif_out_info;
  isp_out = &ihist_stats->isp_out_info;

  pcmd->channelSelect = 0;
  pcmd->rgnHNum = FLOOR16(ihist_stats->roi.width/IHIST_RGN_WIDTH)-1;
  pcmd->rgnVNum = FLOOR16(ihist_stats->roi.height/IHIST_RGN_WIDTH)-1;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;

  /* calculate shift bits */
  total_pixels = (float)((pcmd->rgnHNum + 1) * (pcmd->rgnVNum + 1)) / 2.0;
  shift_bits = CEIL_LOG2(total_pixels);
  shift_bits -= 16;
  shift_bits = MAX(0, shift_bits);
  shift_bits = MIN(4, shift_bits);
  ISP_DBG("Tot %d shift %d", total_pixels, shift_bits);

  pcmd->shiftBits = shift_bits;
  pcmd->siteSelect = 0;

  if (ihist_stats->isp_out_info.is_split == TRUE) {
     ret = ihist_stats44_split_config(ihist_stats,
       pcmd->rgnHNum + 1, pcmd->rgnHOffset);
     if (ret == FALSE) {
       ISP_ERR("failed, ihist_stats44_split_config");
       PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
       return FALSE;
     }
  }

  if (IHIST_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      IHIST_CGC_OVERRIDE_REGISTER, IHIST_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  ret = isp_sub_module_util_reset_hist_dmi(isp_sub_module, STATS_IHIST_RAM,
    ISP_DMI_CFG_DEFAULT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_reset_hist_dmi");
    goto ERROR;
  }

  ret = ihist_stats44_store_hw_update(isp_sub_module, ihist_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: ihist_stats44_store_hw_update");
    goto ERROR;
  }

  if (IHIST_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      IHIST_CGC_OVERRIDE_REGISTER, IHIST_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
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
} /* ihist_stats44_trigger_update */


/** cs_stats44_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ihist_stats44_t         *ihist_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  ihist_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** ihist_stats44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ihist_stats44_t            *ihist_stats = NULL;
  isp_out_info_t             *isp_split_out_info = NULL;
  isp_private_event_t        *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  ihist_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** ihist_stats44_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  ihist_stats44_t         *ihist_stats = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;
  ISP_StatsIhist_CfgType  *pcmd = NULL;
  uint32_t                 window_w_t, window_h_t, total_pixels;
  int32_t                  shift_bits;
  ispif_out_info_t        *ispif_out_info = NULL;
  isp_out_info_t          *isp_out = NULL;
  boolean                  ret = FALSE;
  cam_rect_t              *roi = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  pcmd = &ihist_stats->pcmd;
  ihist_stats->sensor_out_info = *sensor_out_info;
  ispif_out_info = &ihist_stats->ispif_out_info;
  isp_out = &ihist_stats->isp_out_info;


  ISP_DBG("sensor output crop: last pix %d first pix %d"
          "last line %d first line %d",
          ihist_stats->sensor_out_info.request_crop.last_pixel,
          ihist_stats->sensor_out_info.request_crop.first_pixel,
          ihist_stats->sensor_out_info.request_crop.last_line,
          ihist_stats->sensor_out_info.request_crop.first_line);

  window_w_t = ihist_stats->sensor_out_info.request_crop.last_pixel -
    ihist_stats->sensor_out_info.request_crop.first_pixel + 1;
  window_h_t = ihist_stats->sensor_out_info.request_crop.last_line -
    ihist_stats->sensor_out_info.request_crop.first_line + 1;

  roi = &ihist_stats->roi;
  roi->top = 0;
  roi->left = 0;
  roi->width = window_w_t;
  roi->height = window_h_t;

  pcmd->channelSelect = 0;
  pcmd->rgnHNum = FLOOR16(roi->width/IHIST_RGN_WIDTH)-1;
  pcmd->rgnVNum = FLOOR16(roi->height/IHIST_RGN_WIDTH)-1;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;

  /* calculate shift bits */
  total_pixels = (float)((pcmd->rgnHNum + 1) * (pcmd->rgnVNum + 1)) / 2.0;
  shift_bits = CEIL_LOG2(total_pixels);
  shift_bits -= 16;
  shift_bits = MAX(0, shift_bits);
  shift_bits = MIN(4, shift_bits);
  ISP_DBG("Tot %d shift %d", total_pixels, shift_bits);

  pcmd->shiftBits = shift_bits;
  pcmd->siteSelect = 0;

  if (ihist_stats->isp_out_info.is_split == TRUE) {
     ret = ihist_stats44_split_config(ihist_stats,
       pcmd->rgnHNum + 1, pcmd->rgnHOffset);
     if (ret == FALSE) {
       ISP_ERR("failed, ihist_stats44_split_config");
       PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
       return FALSE;
     }
  }

  isp_sub_module->trigger_update_pending = TRUE;

  ISP_DBG("IHIST statsconfig shiftBits      %d", pcmd->shiftBits);
  ISP_DBG("IHIST statsconfig channelSelect  %d", pcmd->channelSelect);
  ISP_DBG("IHIST statsconfig siteSelect     %d", pcmd->siteSelect);
  ISP_DBG("IHIST statsconfig rgnHNum        %d", pcmd->rgnHNum);
  ISP_DBG("IHIST statsconfig rgnVNum        %d", pcmd->rgnVNum);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* ihist_stats44_set_stream_config */

/** ihist_stats44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean ihist_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  ihist_stats44_t *ihist_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)isp_sub_module->private_data;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(ihist_stats, 0, sizeof(*ihist_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* ihist_stats44_streamoff */

/** ihist_stats44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the ihist_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean ihist_stats44_init(isp_sub_module_t *isp_sub_module)
{
  ihist_stats44_t *ihist_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ihist_stats = (ihist_stats44_t *)CAM_MALLOC(sizeof(ihist_stats44_t));
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats %p", ihist_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(ihist_stats, 0, sizeof(*ihist_stats));

  isp_sub_module->private_data = (void *)ihist_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* ihist_stats44_init */

/** ihist_stats44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void ihist_stats44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  free(isp_sub_module->private_data);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* ihist_stats44_destroy */
