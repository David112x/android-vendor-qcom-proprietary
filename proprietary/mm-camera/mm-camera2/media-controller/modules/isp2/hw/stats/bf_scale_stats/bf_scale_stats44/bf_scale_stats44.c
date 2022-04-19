/* bf_scale_stats44.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BF_SCALE_STATS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BF_SCALE_STATS, fmt, ##args)

/* isp headers */
#include "bf_scale_stats44.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"

#define MIN_RGN_H_OFFSET 20
#define MIN_RGN_V_OFFSET  2

//#define BF_SCALE_STATS44_DEBUG
#ifdef BF_SCALE_STATS44_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#endif

//#define BF_SCALE_DUMP
#ifdef BF_SCALE_DUMP
#undef ISP_DUMP
#define ISP_DUMP ISP_ERR
#endif

/** bf_scale_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
 **/
static void bf_scale_stats44_debug(ISP_StatsBf_Scale_MixCfgCmdType *pcmd)
{
  ISP_DBG("Bayer Focus Stats Scale Configurations");
  ISP_DBG("rgnHOffset %d", pcmd->cfg1.rgnHOffset);
  ISP_DBG("rgnVOffset %d", pcmd->cfg1.rgnVOffset);
  ISP_DBG("rgnWidth   %d", pcmd->cfg1.rgnWidth);
  ISP_DBG("rgnHeight  %d", pcmd->cfg1.rgnHeight);
  ISP_DBG("rgnHNum    %d", pcmd->cfg1.rgnHNum);
  ISP_DBG("rgnVNum    %d", pcmd->cfg1.rgnVNum);
  ISP_DBG("r_fv_min   %d", pcmd->cfg1.r_fv_min);
  ISP_DBG("gr_fv_min  %d", pcmd->cfg1.gr_fv_min);
  ISP_DBG("b_fv_min   %d", pcmd->cfg1.b_fv_min);
  ISP_DBG("gb_fv_min  %d", pcmd->cfg1.gb_fv_min);
  ISP_DBG("b00        %d", pcmd->cfg1.b00);
  ISP_DBG("b01        %d", pcmd->cfg1.b01);
  ISP_DBG("b02        %d", pcmd->cfg1.b02);
  ISP_DBG("b03        %d", pcmd->cfg1.b03);
  ISP_DBG("b04        %d", pcmd->cfg1.b04);
  ISP_DBG("b05        %d", pcmd->cfg1.b05);
  ISP_DBG("b06        %d", pcmd->cfg1.b06);
  ISP_DBG("b07        %d", pcmd->cfg1.b07);
  ISP_DBG("b08        %d", pcmd->cfg1.b08);
  ISP_DBG("b09        %d", pcmd->cfg1.b09);
  ISP_DBG("b10        %d", pcmd->cfg1.b10);
  ISP_DBG("b11        %d", pcmd->cfg1.b11);
  ISP_DBG("b12        %d", pcmd->cfg1.b12);
  ISP_DBG("b13        %d", pcmd->cfg1.b13);
  ISP_DBG("b14        %d", pcmd->cfg1.b14);
  ISP_DBG("b15        %d", pcmd->cfg1.b15);
  ISP_DBG("b16        %d", pcmd->cfg1.b16);
  ISP_DBG("b17        %d", pcmd->cfg1.b17);
  ISP_DBG("b18        %d", pcmd->cfg1.b18);
  ISP_DBG("b19        %d", pcmd->cfg1.b19);
  ISP_DBG("b20        %d", pcmd->cfg1.b20);
  ISP_DBG("b21        %d", pcmd->cfg1.b21);
}

/** bf_scale_stats_calculate_phase
 *    @M:       output width
 *    @N:       input width
 *    @offset:  offset
 *    @interp_reso: actual input width
 *    @mn_init: offset of given mn ratio
 *    @phase_init: initial phase
 *    @phase_mult: phase scaling
 *
 * This function calculates initial phase for scaler
 *
 * Return: nothing
 **/
void bf_scale_stats_calculate_phase(uint32_t  M, uint32_t  N,
  uint32_t  offset, uint32_t *interp_reso, uint32_t *mn_init,
  uint32_t *phase_init, uint32_t *phase_mult)
{
  uint32_t ratio = 0;
  *interp_reso = 3;

  if (M == 0) {
    ISP_ERR("failed, devide by 0 check!  h_out/v_out = %d", M);
    return;
  }

  ratio = N / M;
  if (ratio >= 16)     *interp_reso = 0;
  else if (ratio >= 8) *interp_reso = 1;
  else if (ratio >= 4) *interp_reso = 2;

  *mn_init = offset * M % N;
  *phase_init = (*mn_init << (13 + *interp_reso)) / M;
  *phase_mult = (N << (13 + *interp_reso)) / M;
}

/** bf_scale_stats44_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_scale_stats: bf stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static int bf_scale_stats44_store_hw_update(isp_sub_module_t *isp_sub_module,
  bf_scale_stats44_t *bf_scale_stats)
{
  boolean                           ret = TRUE;
  ISP_StatsBf_Scale_MixCfgCmdType  *pcmd = NULL;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  ISP_StatsBf_Scale_MixCfgCmdType  *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  uint32_t                         *data = NULL;
  uint32_t                          len = 0;

  if (!isp_sub_module || !bf_scale_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_scale_stats);
    return FALSE;
  }
  pcmd = &bf_scale_stats->pcmd;
  bf_scale_stats44_debug(pcmd);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)*2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd)*2);

  copy_cmd = (ISP_StatsBf_Scale_MixCfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
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
  cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = BF_SCALE_STATS_OFF_0;
  reg_cfg_cmd[0].u.rw_info.len = BF_SCALE_STATS_LEN_0 * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = BF_SCALE_STATS_OFF_1;
  reg_cfg_cmd[1].u.rw_info.len = BF_SCALE_STATS_LEN_1 * sizeof(uint32_t);

#ifdef PRINT_REG_VAL_SET
  ISP_ERR("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cfg_cmd->cfg_data;
  for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
    ISP_ERR("data[%d] %x", len, data[len]);
  }
#endif

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

/** bf_scale_stats44_update_min_stripe_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                        ret = TRUE;
  bf_scale_stats44_t                   *bf_scale_stats = NULL;
  isp_private_event_t             *private_event = NULL;
  int                            rc = 0;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  uint32_t                       bf_grid_width = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, bf_scale_stats enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
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
  bf_grid_width = bf_scale_stats->camif.width /
    bf_scale_stats->af_config.common.grid_info.h_num;
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

/** bf_scale_stats44_split_config:
 *
 *  @pcmd: Pointer to the reg_cmd struct that needs to be
 *        dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static boolean bf_scale_stats44_split_config(bf_scale_stats44_t *bf_scale_stats,
  uint32_t single_rgnHNum, uint32_t single_rgnHOffset,
  uint32_t single_rgnWidth, uint32_t scale_ratio)
{
  ISP_StatsBf_Scale_MixCfgCmdType *pcmd = NULL;
  uint32_t                left_stripe_stats_bound = 0;
  uint32_t                overlap = 0;
  isp_out_info_t          *isp_out = NULL;
  uint32_t                upscale_single_rgn_width = 0;
  uint32_t                upscale_single_rgn_HOffset = 0;


  if (!bf_scale_stats) {
    ISP_ERR("NULL pointer! bf %p", bf_scale_stats);
    return FALSE;
  }

  pcmd = &bf_scale_stats->pcmd;
  overlap = bf_scale_stats->ispif_out_info.overlap;
  isp_out = &bf_scale_stats->isp_out_info;

  /*to calculate dual vfe, need to upscale back to camif size*/
  upscale_single_rgn_width = single_rgnWidth * scale_ratio;
  upscale_single_rgn_HOffset = single_rgnHOffset * scale_ratio;


  if (isp_out->is_split == TRUE) {
    /*find out left stripe stats boundry*/
    left_stripe_stats_bound = upscale_single_rgn_HOffset +
       upscale_single_rgn_width * single_rgnHNum;
    if (left_stripe_stats_bound > isp_out->right_stripe_offset + overlap)
      left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

    /*if single VFE request stats rgn go over left end,
      then no rgn required in left VFE,
      or we need to calculate the rgn num for left VFE*/
    bf_scale_stats->num_left_rgns =
      (upscale_single_rgn_HOffset > left_stripe_stats_bound) ?
      0 : (left_stripe_stats_bound - upscale_single_rgn_HOffset) /
      upscale_single_rgn_width;
    bf_scale_stats->num_right_rgns = single_rgnHNum - bf_scale_stats->num_left_rgns;

    /* Check if the stat region satisfies the crucial assumption: there is
       at least one grid line within the overlap area if the whole
       region spans across both stripes */
    if (upscale_single_rgn_HOffset + (bf_scale_stats->num_left_rgns * upscale_single_rgn_width) <
       isp_out->right_stripe_offset && bf_scale_stats->num_right_rgns > 0) {
      ISP_ERR("failed: Unable to support such stats region in dual-ISP mode");
      return FALSE;
    }

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /*check with system team, hw limit should be minimum 0 only
        do we still need to propgram at least one as 8974?*/
      pcmd->cfg1.rgnHNum =  (bf_scale_stats->num_left_rgns > 0) ?
        bf_scale_stats->num_left_rgns - 1 : 0;
      pcmd->cfg1.rgnHOffset = (bf_scale_stats->num_left_rgns > 0) ?
        FLOOR2(single_rgnHOffset) : MIN_RGN_H_OFFSET;
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->cfg1.rgnHNum = (bf_scale_stats->num_right_rgns > 0) ?
        bf_scale_stats->num_right_rgns - 1 : 0;
      pcmd->cfg1.rgnHOffset = (upscale_single_rgn_HOffset +
        (bf_scale_stats->num_left_rgns * upscale_single_rgn_width) -
        isp_out->right_stripe_offset) / scale_ratio;
      pcmd->cfg1.rgnHOffset = (bf_scale_stats->num_right_rgns > 0) ?
        FLOOR2(pcmd->cfg1.rgnHOffset) : MIN_RGN_H_OFFSET;
    }
    pcmd->cfg1.rgnHOffset = (pcmd->cfg1.rgnHOffset >= MIN_RGN_H_OFFSET) ?
      pcmd->cfg1.rgnHOffset : MIN_RGN_H_OFFSET;
  }

  return TRUE;
}

/** bf_scale_stats44_fill_stats_parser_params:
 *
 *  @bf_scale_stats: bf stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_scale_stats44_fill_stats_parser_params(bf_scale_stats44_t *bf_scale_stats,
  isp_saved_stats_params_t *stats_params)
{
  ISP_StatsBf_Scale_MixCfgCmdType *pcmd = NULL;
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_stats_roi_params_t    *roi_params = NULL;
  isp_stats_config_t        *stats_config = NULL;
  isp_out_info_t            *isp_out = NULL;
  if (!bf_scale_stats || !stats_params) {
    ISP_ERR("failed: %p %p", bf_scale_stats, stats_params);
    return FALSE;
  }

  pcmd = &bf_scale_stats->pcmd;
  isp_out = &bf_scale_stats->isp_out_info;

  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_BF_SCALE);
  stats_config->af_config = bf_scale_stats->af_config;

  /*fill in parse info*/
  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BF_SCALE];
  rgns_stats->is_valid = TRUE;
  if (isp_out->is_split == TRUE) {
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /*DUAL VFE case*/
      rgns_stats->h_rgns_start = 0;
      rgns_stats->h_rgns_end = bf_scale_stats->num_left_rgns - 1;
      rgns_stats->h_rgns_total =
        bf_scale_stats->num_left_rgns + bf_scale_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->cfg1.rgnVNum + 1;
    } else {
      rgns_stats->h_rgns_start = bf_scale_stats->num_left_rgns;
      rgns_stats->h_rgns_end =
        bf_scale_stats->num_left_rgns + bf_scale_stats->num_right_rgns - 1;
      rgns_stats->h_rgns_total =
        bf_scale_stats->num_left_rgns + bf_scale_stats->num_right_rgns;
      rgns_stats->v_rgns_total = pcmd->cfg1.rgnVNum + 1;
    }
  } else {
    /*SINGLE VFE case */
    rgns_stats->h_rgns_start = 0;
    rgns_stats->h_rgns_end = pcmd->cfg1.rgnHNum;
    rgns_stats->h_rgns_total = pcmd->cfg1.rgnHNum + 1;
    rgns_stats->v_rgns_total = pcmd->cfg1.rgnVNum + 1;
  }

  ISP_DBG("rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  return TRUE;
}


/** bf_scale_stats44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for bf_scale_stats using values passed
 *  in pix settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  ISP_StatsBf_Scale_MixCfgCmdType *pcmd = NULL;
  af_config_t                   *af_config = NULL;
  bf_scale_config_t             *bf_scale_config = NULL;
  uint32_t                       bf_scale_rgn_width = 0,
                                 bf_scale_rgn_height = 0;
  uint32_t                       interp_reso = 0,
                                 mn_init = 0,
                                 phase_init = 0,
                                 phase_mult = 0,
                                 scale_ratio = 1;
  int32_t                        i = 0;
  bf_scale_stats44_t            *bf_scale_stats = NULL;
  boolean                        is_split = FALSE;
  ispif_out_info_t              *ispif_out_info = NULL;
  isp_out_info_t                *isp_out = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_output_t       *sub_module_output = NULL;
  isp_stats_config_t            *stats_config = NULL;
  isp_rgns_stats_param_t        *rgns_stats = NULL;


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

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
   goto ERROR;
  }

  pcmd = &bf_scale_stats->pcmd;
  af_config = &bf_scale_stats->af_config;
  bf_scale_config = &bf_scale_stats->af_config.bf_scale;
  ispif_out_info = &bf_scale_stats->ispif_out_info;
  isp_out = &bf_scale_stats->isp_out_info;

  is_split = ispif_out_info->is_split;

  if ((isp_sub_module->submod_enable == FALSE) ||
    (isp_sub_module->trigger_update_pending == FALSE)) {
    ISP_DBG("BF not enabled / trigger update pending false");

    /* Fill af_config */
    if (sub_module_output->stats_params) {
      ret = bf_scale_stats44_fill_stats_parser_params(bf_scale_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_scale_stats44_fill_stats_parser_params");
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


  /* Read scaling ratio from bf_scale_config */
  if(bf_scale_config->scale != 0) {
    scale_ratio = bf_scale_config->scale;
  }
  else {
    scale_ratio = 1;
  }

  if(((bf_scale_stats->scale_roi.bf_scaled_top +
       bf_scale_stats->scale_roi.bf_scaled_height) >
       (bf_scale_stats->camif.height/scale_ratio)) ||
       ((bf_scale_stats->scale_roi.bf_scaled_left +
       bf_scale_stats->scale_roi.bf_scaled_width) >
        (bf_scale_stats->camif.width/scale_ratio))){
    ISP_ERR("Invalid BF stats ROI[%d,%d,%d,%d], CAMIF Dim [%d * %d]\n",
      bf_scale_stats->scale_roi.bf_scaled_left,
      bf_scale_stats->scale_roi.bf_scaled_top,
      bf_scale_stats->scale_roi.bf_scaled_width,
      bf_scale_stats->scale_roi.bf_scaled_height,
      bf_scale_stats->camif.width,bf_scale_stats->camif.height);
    goto ERROR;
  }

    bf_scale_rgn_width = bf_scale_stats->scale_roi.bf_scaled_width /
      af_config->common.grid_info.h_num;
    bf_scale_rgn_height = bf_scale_stats->scale_roi.bf_scaled_height /
     af_config->common.grid_info.v_num;


  /* scaler config */
  pcmd->cfg1.hEnable = 1;
  pcmd->cfg1.vEnable = 1;

  pcmd->cfg1.hIn = MIN(ISP_STATS_BF_SCALE_MAX_H_CFG, bf_scale_stats->camif.width);
  pcmd->cfg1.vIn = MIN(ISP_STATS_BF_SCALE_MAX_V_CFG, bf_scale_stats->camif.height);

  pcmd->cfg1.hOut = pcmd->cfg1.hIn / scale_ratio;
  pcmd->cfg1.vOut = pcmd->cfg1.vIn / scale_ratio;

  bf_scale_stats_calculate_phase(pcmd->cfg1.hOut, pcmd->cfg1.hIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult);
  pcmd->cfg1.horizInterResolution = interp_reso;
  pcmd->cfg2.horizMNInit = mn_init;
  pcmd->cfg2.horizPhaseInit = phase_init;
  pcmd->cfg1.horizPhaseMult = phase_mult;
  pcmd->cfg2.scaleYInWidth = pcmd->cfg1.hIn;
  pcmd->cfg2.hSkipCount = 0;
  bf_scale_stats_calculate_phase(pcmd->cfg1.vOut, pcmd->cfg1.vIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult);
  pcmd->cfg1.vertInterResolution = interp_reso;
  pcmd->cfg2.vertMNInit = mn_init;
  pcmd->cfg2.vertPhaseInit = phase_init;
  pcmd->cfg1.vertPhaseMult = phase_mult;
  pcmd->cfg2.scaleYInHeight = pcmd->cfg1.vIn;
  pcmd->cfg2.vSkipCount = 0;

  /* min of rgn_v_offset = 2, min of rgn_h_offset = 20
     rgn_v_offset = 2 + roi_top,  rgn_h_offset = 20 + roi_left */
  pcmd->cfg1.rgnHOffset = FLOOR2(bf_scale_stats->scale_roi.bf_scaled_left);
  pcmd->cfg1.rgnVOffset = FLOOR2(bf_scale_stats->scale_roi.bf_scaled_top);
  pcmd->cfg1.rgnWidth   = FLOOR2(bf_scale_rgn_width) - 1;
  pcmd->cfg1.rgnHeight  = FLOOR2(bf_scale_rgn_height) - 1;
  pcmd->cfg1.rgnHNum    = af_config->common.grid_info.h_num - 1;
  pcmd->cfg1.rgnVNum    = af_config->common.grid_info.v_num - 1;

  /* update bf_scale_config to reflect the new config
    It will be sent to 3A in STATS_NOTIFY event  */
  bf_scale_config->roi.left = pcmd->cfg1.rgnHOffset;
  bf_scale_config->roi.top = pcmd->cfg1.rgnVOffset;
  bf_scale_config->roi.width = (pcmd->cfg1.rgnWidth + 1) *
    af_config->common.grid_info.h_num;
  bf_scale_config->roi.height = (pcmd->cfg1.rgnHeight + 1) *
    af_config->common.grid_info.v_num;

  pcmd->cfg1.r_fv_min   = bf_scale_config->r_min;
  pcmd->cfg1.gr_fv_min  = bf_scale_config->gr_min;
  pcmd->cfg1.b_fv_min   = bf_scale_config->b_min;
  pcmd->cfg1.gb_fv_min  = bf_scale_config->gb_min;

  pcmd->cfg1.b00 = bf_scale_config->hpf[0];
  pcmd->cfg1.b01 = bf_scale_config->hpf[1];
  pcmd->cfg1.b02 = bf_scale_config->hpf[2];
  pcmd->cfg1.b03 = bf_scale_config->hpf[3];
  pcmd->cfg1.b04 = bf_scale_config->hpf[4];
  pcmd->cfg1.b05 = bf_scale_config->hpf[5];
  pcmd->cfg1.b06 = bf_scale_config->hpf[6];
  pcmd->cfg1.b07 = bf_scale_config->hpf[7];
  pcmd->cfg1.b08 = bf_scale_config->hpf[8];
  pcmd->cfg1.b09 = bf_scale_config->hpf[9];
  pcmd->cfg1.b10 = bf_scale_config->hpf[10];
  pcmd->cfg1.b11 = bf_scale_config->hpf[11];
  pcmd->cfg1.b12 = bf_scale_config->hpf[12];
  pcmd->cfg1.b13 = bf_scale_config->hpf[13];
  pcmd->cfg1.b14 = bf_scale_config->hpf[14];
  pcmd->cfg1.b15 = bf_scale_config->hpf[15];
  pcmd->cfg1.b16 = bf_scale_config->hpf[16];
  pcmd->cfg1.b17 = bf_scale_config->hpf[17];
  pcmd->cfg1.b18 = bf_scale_config->hpf[18];
  pcmd->cfg1.b19 = bf_scale_config->hpf[19];
  pcmd->cfg1.b20 = bf_scale_config->hpf[20];
  pcmd->cfg1.b21 = bf_scale_config->hpf[21];

  ret = bf_scale_stats44_split_config(bf_scale_stats, pcmd->cfg1.rgnHNum +1,
    pcmd->cfg1.rgnHOffset, pcmd->cfg1.rgnWidth + 1, scale_ratio);
  if (ret == FALSE) {
    ISP_ERR("bf_scale_stats44_split_config failed");
    goto ERROR;
  }

  /* rgnHOffset needs to be at least 20 and at most camif_window_width - 2 */
  if (pcmd->cfg1.rgnHOffset < MIN_RGN_H_OFFSET ||
    pcmd->cfg1.rgnHOffset >= bf_scale_stats->camif.width - 2) {
    ISP_ERR("Unsupported BF stats region config: invalid offset: %d",
      pcmd->cfg1.rgnHOffset);
    pcmd->cfg1.rgnHOffset = MIN_RGN_H_OFFSET;
  }

  /* Fill bf_scale_config */
  if (sub_module_output->stats_params) {
    ret = bf_scale_stats44_fill_stats_parser_params(bf_scale_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_scale_stats44_fill_stats_parser_params");
    }
  }

  ret = bf_scale_stats44_store_hw_update(isp_sub_module, bf_scale_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_scale_stats44_store_hw_update");
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
}

/** bf_scale_stats44_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: stats data with new config
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_scale_stats44_t   *bf_scale_stats = NULL;
  af_config_t          *af_config = NULL;
  bf_scale_config_t    *bf_scale_config = NULL;
  uint32_t             roi_w_adjust = 0;
  uint32_t             roi_h_adjust = 0;
  int32_t              bf_scaled_left = 0;
  int32_t              bf_scaled_top = 0;
  int32_t              bf_scaled_width = 0;
  int32_t              bf_scaled_height = 0;
  int32_t              scale_ratio = 1;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  af_config = (af_config_t *)data;
  /* If no af_config or scale bit in mask return */
  if (!(af_config->mask & MCT_EVENT_STATS_BF_SCALE)) {
    ISP_DBG(" BF_SCALE mask not set ");
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_scale_config = &af_config->bf_scale;

  /* Read scaling ratio from bf_scale_config */
  if(bf_scale_config->scale != 0) {
    scale_ratio = bf_scale_config->scale;
  }
  else {
    scale_ratio = 1;
  }

  bf_scaled_left = bf_scale_config->roi.left/scale_ratio;
  bf_scaled_width = bf_scale_config->roi.width/scale_ratio;
  bf_scaled_top = bf_scale_config->roi.top/scale_ratio;
  bf_scaled_height = bf_scale_config->roi.height/scale_ratio;

  /*if offset not fit limtation, then adjust offset.
    we need to adjust width so it wont go over camif boundary*/
  if (bf_scaled_left < MIN_RGN_H_OFFSET) {
    roi_w_adjust = MIN_RGN_H_OFFSET - bf_scaled_left;
    bf_scaled_left = MIN_RGN_H_OFFSET;
    bf_scaled_width = bf_scaled_width - roi_w_adjust;
  }

  if (bf_scaled_top < MIN_RGN_V_OFFSET) {
    roi_h_adjust = MIN_RGN_V_OFFSET - bf_scaled_top;
    bf_scaled_top = MIN_RGN_V_OFFSET;
    bf_scaled_height = bf_scaled_height - roi_h_adjust;
  }

  if((bf_scaled_top + bf_scaled_height > (bf_scale_stats->camif.height/scale_ratio)) ||
    (bf_scaled_left + bf_scaled_width > (bf_scale_stats->camif.width/scale_ratio))){
    ISP_ERR("Invalid BF Scale stats ROI[%d,%d,%d,%d], CAMIF Dim [%d * %d]\n",
    bf_scaled_left, bf_scaled_top,bf_scaled_width,bf_scaled_height,
    bf_scale_stats->camif.width,bf_scale_stats->camif.height);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  bf_scale_stats->af_config = *af_config;

  bf_scale_stats->scale_roi.bf_scaled_left  =  bf_scaled_left;
  bf_scale_stats->scale_roi.bf_scaled_width  = bf_scaled_width;
  bf_scale_stats->scale_roi.bf_scaled_top =  bf_scaled_top;
  bf_scale_stats->scale_roi.bf_scaled_height  = bf_scaled_height;

  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bf_scale_stats44_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_scale_stats44_t            *bf_scale_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_scale_stats->ispif_out_info = *ispif_stripe_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bf_scale_stats44_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_scale_stats44_t            *bf_scale_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bg_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_scale_stats->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bf_scale_stats44_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_scale_stats44_t               *bf_scale_stats = NULL;
  sensor_out_info_t                *sensor_out_info = NULL;
  af_config_t                      *af_init_cfg = NULL;
  ISP_StatsBf_Scale_MixCfgCmdType  *pcmd = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_scale_stats->sensor_out_info = *sensor_out_info;

  bf_scale_stats->camif.width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  bf_scale_stats->camif.height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  af_init_cfg = &bf_scale_stats->af_config;
  af_init_cfg->bf_scale.scale = 2;
  af_init_cfg->common.grid_info.h_num = 18;
  af_init_cfg->common.grid_info.v_num = 14;
  af_init_cfg->bf_scale.roi.left = bf_scale_stats->camif.width / 4;
  af_init_cfg->bf_scale.roi.top = bf_scale_stats->camif.height / 4;
  af_init_cfg->bf_scale.roi.width = bf_scale_stats->camif.width / 2;
  af_init_cfg->bf_scale.roi.height = bf_scale_stats->camif.height / 2;

  bf_scale_stats->scale_roi.bf_scaled_left  =  af_init_cfg->bf_scale.roi.left /
    af_init_cfg->bf_scale.scale;
  bf_scale_stats->scale_roi.bf_scaled_width  =
    af_init_cfg->bf_scale.roi.width / af_init_cfg->bf_scale.scale;
  bf_scale_stats->scale_roi.bf_scaled_top =  af_init_cfg->bf_scale.roi.top /
   af_init_cfg->bf_scale.scale;
  bf_scale_stats->scale_roi.bf_scaled_height  =
   af_init_cfg->bf_scale.roi.height / af_init_cfg->bf_scale.scale;

  pcmd = &bf_scale_stats->pcmd;
  memset(pcmd, 0, sizeof(ISP_StatsBf_Scale_MixCfgCmdType));

  pcmd->cfg1.rgnHNum    = 17;
  pcmd->cfg1.rgnVNum    = 13;
  pcmd->cfg1.r_fv_min   = 30;
  pcmd->cfg1.gr_fv_min  = 30;
  pcmd->cfg1.b_fv_min   = 30;
  pcmd->cfg1.gb_fv_min  = 30;
  pcmd->cfg1.b00        = -4;
  pcmd->cfg1.b01        = -4;
  pcmd->cfg1.b02        = 1;
  pcmd->cfg1.b03        = 2;
  pcmd->cfg1.b04        = 3;
  pcmd->cfg1.b05        = 4;
  pcmd->cfg1.b06        = 3;
  pcmd->cfg1.b07        = 2;
  pcmd->cfg1.b08        = 1;
  pcmd->cfg1.b09        = -4;
  pcmd->cfg1.b10        = -4;
  pcmd->cfg1.b11        = -4;
  pcmd->cfg1.b12        = -4;
  pcmd->cfg1.b13        = 1;
  pcmd->cfg1.b14        = 2;
  pcmd->cfg1.b15        = 3;
  pcmd->cfg1.b16        = 4;
  pcmd->cfg1.b17        = 3;
  pcmd->cfg1.b18        = 2;
  pcmd->cfg1.b19        = 1;
  pcmd->cfg1.b20        = -4;
  pcmd->cfg1.b21        = -4;

  ISP_DBG("roi %d %d %d %d", af_init_cfg->bf_scale.roi.left,
    af_init_cfg->bf_scale.roi.top,
    af_init_cfg->bf_scale.roi.width,
    af_init_cfg->bf_scale.roi.height);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bf_scale_stats44_set_stream_config */

/** bf_scale_stats44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_scale_stats44_streamoff(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_scale_stats44_t *bf_scale_stats = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_scale_stats = (bf_scale_stats44_t *)isp_sub_module->private_data;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bf_scale_stats, 0, sizeof(*bf_scale_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bf_scale_stats44_streamoff */

/** bf_scale_stats44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bf_scale_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bf_scale_stats44_init(isp_sub_module_t *isp_sub_module)
{
  bf_scale_stats44_t *bf_scale_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_scale_stats = (bf_scale_stats44_t *)CAM_MALLOC(sizeof(bf_scale_stats44_t));
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats %p", bf_scale_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  memset(bf_scale_stats, 0, sizeof(*bf_scale_stats));

  isp_sub_module->private_data = (void *)bf_scale_stats;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}/* bf_scale_stats44_init */

/** bf_scale_stats44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void bf_scale_stats44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  free(isp_sub_module->private_data);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* bf_scale_stats44_destroy */
