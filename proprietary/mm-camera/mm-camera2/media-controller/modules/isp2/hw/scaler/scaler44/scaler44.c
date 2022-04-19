/* scaler44.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

/* isp headers */
#include "module_scaler44.h"
#include "scaler44_util.h"
#include "port_scaler44.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "scaler44.h"

static boolean scaler44_get_module_enable_mask(scaler44_t *scaler,
  isp_module_enable_info_t *enable_bit_info);

/** scaler44_cmd_debug
 *    @cmd: bcc config cmd
 *    @index: pix path index
 *
 * This function dumps the scaler module register settings set
 * to hw
 *
 * Return: nothing
 **/
static void scaler44_cmd_debug(ISP_ScaleCfgCmdType* cmd, uint8_t index)
{
  ISP_HIGH(" scaler[%d](%d-Enc/%d-View/%d-Video/%d-Ideal)", index,
    ISP_HW_STREAM_ENCODER, ISP_HW_STREAM_VIEWFINDER,
    ISP_HW_STREAM_VIDEO, ISP_HW_STREAM_IDEAL_RAW);

  /* Y config */
  ISP_HIGH("  scaler[%d] Y horiz_input = %d",
    index, cmd->Y_ScaleCfg.hIn);
  ISP_HIGH("scaler[%d] Y horiz_output = %d",
    index, cmd->Y_ScaleCfg.hOut);
  ISP_HIGH("  scaler[%d] Y vert_input = %d",
    index, cmd->Y_ScaleCfg.vIn);
  ISP_HIGH("  scaler[%d] Y vert_output = %d",
    index, cmd->Y_ScaleCfg.vOut);
  ISP_HIGH(" scaler[%d] Y horiz_interp_Resolution = %d",
     index, cmd->Y_ScaleCfg.horizInterResolution);
  ISP_HIGH(" scaler[%d] Y horiz_phase_Multipli_Factor = %u",
     index, cmd->Y_ScaleCfg.horizPhaseMult);
  ISP_HIGH(" scaler[%d] Y vert_interp_Resolution = %d",
     index, cmd->Y_ScaleCfg.vertInterResolution);
  ISP_HIGH(" scaler[%d] Y vert_phase_Multipli_Factor = %u",
     index, cmd->Y_ScaleCfg.vertPhaseMult);
  ISP_HIGH(" scaler[%d] Y horiz_MNInit = %d",
     index, cmd->Y_ScaleCfg.horizMNInit);
  ISP_HIGH(" scaler[%d] Y horiz_PhaseInit = %d",
     index, cmd->Y_ScaleCfg.horizPhaseInit);
  ISP_HIGH(" scaler[%d] Y vert_MNInit = %d",
     index, cmd->Y_ScaleCfg.vertMNInit);
  ISP_HIGH(" scaler[%d] Y vert_PhaseInit = %d",
     index, cmd->Y_ScaleCfg.vertPhaseInit);
  ISP_DBG("%s: scaler[%d] Y scale_YInWidth = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.scaleYInWidth);
  ISP_DBG("%s: scaler[%d] Y horiz_SkipCount = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.hSkipCount);
  ISP_DBG("%s: scaler[%d] Y right_pad_en = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.rightPadEnable);
  ISP_HIGH("%s: scaler[%d] Y scale_YInHeight = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.scaleYInHeight);
  ISP_DBG("%s: scaler[%d] Y vert_SkipCount = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.vSkipCount);
  ISP_DBG("%s: scaler[%d] Y bottom_pad_en = %d\n",
    __func__, index, cmd->Y_ScalePadCfg.bottomPadEnable);

  /* cbcr config*/
  ISP_HIGH(" scaler[%d] CbCr horiz_input = %d",
     index, cmd->CbCr_ScaleCfg.hIn);
  ISP_HIGH(" scaler[%d] CbCr horiz_output = %d",
     index, cmd->CbCr_ScaleCfg.hOut);
  ISP_HIGH(" scaler[%d] CbCr vert_input = %d",
     index, cmd->CbCr_ScaleCfg.vIn);
  ISP_HIGH(" scaler[%d] CbCr vert_output = %d",
     index, cmd->CbCr_ScaleCfg.vOut);
  ISP_HIGH(" scaler[%d] CbCr horiz_interp_Resolution = %d",
     index, cmd->CbCr_ScaleCfg.horizInterResolution);
  ISP_HIGH(" scaler[%d] CbCr horiz_phase_Multipli_Factor = %u",
     index, cmd->CbCr_ScaleCfg.horizPhaseMult);
  ISP_HIGH(" scaler[%d] CbCr vert_interp_Resolution = %d",
     index, cmd->CbCr_ScaleCfg.vertInterResolution );
  ISP_HIGH(" scaler[%d] CbCr vert_phase_Multipli_Factor = %u",
      index, cmd->CbCr_ScaleCfg.vertPhaseMult );
  ISP_HIGH(" scaler[%d] CbCr horiz_MNInit = %d",
     index, cmd->CbCr_ScaleCfg.horizMNInit);
  ISP_HIGH(" scaler[%d] CbCr horiz_PhaseInit = %d",
     index, cmd->CbCr_ScaleCfg.horizPhaseInit);
  ISP_HIGH(" scaler[%d] CbCr vert_MNInit = %d",
     index, cmd->CbCr_ScaleCfg.vertMNInit);
  ISP_HIGH(" scaler[%d] CbCr vert_PhaseInit = %d",
     index, cmd->CbCr_ScaleCfg.vertPhaseInit);
  ISP_HIGH(" scaler[%d] CbCr CbCr_In_Width = %d",
     index, cmd->CbCr_ScaleCfg.scaleCbCrInWidth);
  ISP_HIGH(" scaler[%d] CbCr Horiz_Skip_Count = %d",
     index, cmd->CbCr_ScaleCfg.hSkipCount);
  ISP_HIGH(" scaler[%d] CbCr Right_Pad_Enable = %d",
     index, cmd->CbCr_ScaleCfg.rightPadEnable);
  ISP_HIGH(" scaler[%d] CbCr CbCr_In_Height = %d",
     index, cmd->CbCr_ScaleCfg.scaleCbCrInHeight);
  ISP_HIGH(" scaler[%d] CbCr Vert_Skip_Count = %d",
     index, cmd->CbCr_ScaleCfg.vSkipCount);
  ISP_HIGH(" scaler[%d] CbCr Bottom_Pad_Enable = %d",
     index, cmd->CbCr_ScaleCfg.bottomPadEnable);

} /* scaler44_cmd_debug */

/** scaler44_handle_set_chromatix_ptr
 *
 *  @scaler: scaler ctrl handle
 *  @event: set chromatix ptr event to be handled
 *
 *  Store chromatix ptr to be used later for interpolation
 *
 *  Return TRUE on success and FALS on failure
 **/
boolean scaler44_handle_set_chromatix_ptr(scaler44_t *scaler,
  mct_event_t *event)
{
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!scaler || !event) {
    ISP_ERR("failed: %p %p", scaler, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    goto ERROR;
  }

  scaler->chromatix_ptrs = *chromatix_ptrs;
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return FALSE;
}

/** calculate_scaler44_factor
 *    @scale_factor_horiz: horizontal scale factor
 *    @scale_factor_vert:  vertical scale factor
 *    @hFactor: output
 *    @vFactor: output
 *
 * calculate scaler44 factor
 *
 * Return: TRUE
 **/
static boolean calculate_scaler44_factor(int scale_factor_horiz,
  int scale_factor_vert, uint32_t *h_inter_resol, uint32_t *v_inter_resol)
{

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(" Output1 scale larger than camsensor FOV, set scale_factor=1");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    *h_inter_resol = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    *h_inter_resol = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    *h_inter_resol = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    *h_inter_resol = 0;
  } else {
    ISP_DBG("scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.");
  }

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    *v_inter_resol = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    *v_inter_resol = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    *v_inter_resol = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    *v_inter_resol = 0;
  } else {
    ISP_DBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.");
  }

  return TRUE;
}/* calculate_scaler44_factor */

/** scaler44_is_stripe_offset_on_rollover_point
 *
 * DESCRIPTION:
 *
 **/
boolean scaler44_is_stripe_offset_rollover_point(
  uint32_t M, uint32_t N, uint32_t offset, uint32_t *mn_init_output)
{
  uint32_t mn_init_1 = 0;
  uint32_t mn_init_2 = 0;

  mn_init_1 = offset * M % N;
  mn_init_2 = (offset + 1) * M % N;


  if (mn_init_1 == 0) {
    *mn_init_output = 0;
    return TRUE;
  }

  if (mn_init_2 < M && mn_init_2 != 0) {
     *mn_init_output = mn_init_2;
     return TRUE;
  }

  return FALSE;
}

/** scale44_fetch_engine_adjust
 *  @scaler: scaler module control struct
 *  @stripeoffset : pointer to stripe offset
 *
 *
 *
 * Adjust the stripe offset for FE read
 **/
void scale44_fetch_engine_adjust(scaler44_t *scaler, uint32_t *stripe_offset)
{
  if (!stripe_offset || !scaler) {
    ISP_ERR("NULL pointer, stripe_offset %p, scaler %p",
      stripe_offset, scaler);
    return;
  }

  switch (scaler->sensor_out_info.fmt) {
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      (*stripe_offset) -= (*stripe_offset) % 32;
      break;
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      (*stripe_offset) -= (*stripe_offset) % 12;
      break;
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
      (*stripe_offset) -= (*stripe_offset) % 4;
      break;
    default:
      break;
  }
}

/** scaler44_compute_stripe_offset
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler44_compute_stripe_offset(scaler44_t *scaler,
  isp_stripe_request_t *stripe_request_info)
{
  int32_t       rc = 0;
  uint32_t      mid_point = 0;
  uint32_t      sensor_output_width = 0;
  uint32_t      split_point = 0;
  uint32_t      stripe_offset = 0;
  uint32_t      min_overlap_half = 0;
  uint32_t      rolloff_grid_width = 0;
  uint32_t      rolloff_min_dist_from_grid = 0;
  uint32_t      distance_from_grid = 0;
  uint32_t      rolloff_subgrid_width = 0;
  uint32_t      rolloff_min_dist_from_subgrid = 0;
  uint32_t      distance_from_subgrid = 0;
  uint32_t      left_stripe = 0;
  uint32_t      right_stripe = 0;
  isp_stripe_limit_info_t *limits;

  if (!stripe_request_info || !scaler) {
    ISP_ERR("NULL pointer, ispif_out_info %p, scaler %p",
      stripe_request_info, scaler);
    return -1;
  }

  sensor_output_width =
    scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;

  limits = &stripe_request_info->stripe_limit;

  /* decide split point */
  mid_point = sensor_output_width / 2;
  split_point = MAX(limits->min_left_split, mid_point);

  rolloff_grid_width = limits->rolloff_grid_width;
  rolloff_min_dist_from_grid = limits->rolloff_min_dist_from_grid;
  rolloff_subgrid_width = limits->rolloff_subgrid_width;
  rolloff_min_dist_from_subgrid = limits->rolloff_min_dist_from_subgrid;

  left_stripe = split_point + limits->min_overlap_right;
  right_stripe = sensor_output_width - limits->smallest_split_point +
    limits->min_overlap_left;

  /* Move split point if does not fit in one of ISPs limitations*/
  if (left_stripe > limits->max_left_split) {
    split_point -= left_stripe - limits->max_left_split;
    split_point -= rolloff_min_dist_from_grid;
  } else if (right_stripe > limits->max_right_split) {
    split_point += right_stripe - limits->max_right_split;
    split_point += rolloff_min_dist_from_grid;
  }
  if (limits->min_left_split > split_point) {
    ISP_ERR("failed: cannot fit in ISP limitations");
    ISP_ERR("left stripe %d split_point %d",left_stripe, split_point);
  }
  /*decide initial stripe offset by split point and overlap*/
  stripe_offset = limits->smallest_split_point - limits->min_overlap_left;

  /* check if the init stripe offset go over max stripe offset*/
   if ((stripe_request_info->stripe_limit.max_stripe_offset != 0) &&
       (stripe_offset > stripe_request_info->stripe_limit.max_stripe_offset)) {
     stripe_offset = stripe_request_info->stripe_limit.max_stripe_offset;
   }

  /* Adjust the stripe offset for rolloff grid line - 36 requirement */
  if (rolloff_grid_width != 0) {
    distance_from_grid = (mid_point - stripe_offset) % rolloff_grid_width;
    if (distance_from_grid < rolloff_min_dist_from_grid) {
      stripe_offset -= rolloff_min_dist_from_grid - distance_from_grid;
    }

    /*claculate new distance to grid*/
    /* and to calculate distance to subgrid*/
    if (rolloff_grid_width != 0) {
	distance_from_grid = (mid_point - stripe_offset) % rolloff_grid_width;
    }
    if (rolloff_subgrid_width != 0) {
	distance_from_subgrid = distance_from_grid % rolloff_subgrid_width;
    }
    if (distance_from_subgrid < rolloff_min_dist_from_subgrid) {
      stripe_offset -= rolloff_min_dist_from_subgrid - distance_from_subgrid;
    }
  }

  ISP_DBG("sensor_output_width %d, mid_point %d, split_point %d, "
        "stripe_offset %d",
        sensor_output_width, mid_point,split_point, stripe_offset);

  if (stripe_request_info->offline_mode) {
    scale44_fetch_engine_adjust(scaler, &stripe_offset);
  }
  stripe_offset = FLOOR2(stripe_offset);

  /*scaler 44 dont have stripe offset limitation
    only scaler 40 have limitation which need to be on rollover point*/
  stripe_request_info->ispif_out_info.overlap = limits->min_left_split -
    stripe_offset + limits->min_overlap_right;
  stripe_request_info->ispif_out_info.overlap =
    FLOOR2(stripe_request_info->ispif_out_info.overlap);


  /*ispif stripe offset need to consider ispif*/
  stripe_request_info->ispif_out_info.right_stripe_offset =
    stripe_offset + scaler->sensor_out_info.request_crop.first_pixel;

  /*store mid poing between both ISPs*/
  stripe_request_info->ispif_out_info.split_point =
    ((split_point - limits->smallest_split_point) / 2) +
    limits->smallest_split_point;


  ISP_DBG("mid_point %d, split_point %d, stripe_offset %d",
           mid_point,split_point, stripe_offset);

  if ((stripe_offset + stripe_request_info->ispif_out_info.overlap) >
       limits->max_left_split) {
    ISP_ERR("failed: Cannot fit in left ISP. stripe %d max %d ",
      stripe_offset + stripe_request_info->ispif_out_info.overlap,
      limits->max_left_split);
    return -1;
  }
  if ((sensor_output_width - stripe_offset) > limits->max_right_split) {
    ISP_ERR("failed: Cannot fit in right ISP. stripe %d max %d ",
      sensor_output_width - stripe_offset, limits->max_right_split);
    return -1;
  }

  ISP_DBG("<stripe_dbg> ispif_out_info: is_split = %d, overlap = %d,"\
    "right_stripe_offset = %d split_point = %d",
    stripe_request_info->ispif_out_info.is_split,
    stripe_request_info->ispif_out_info.overlap,
    stripe_request_info->ispif_out_info.right_stripe_offset,
    stripe_request_info->ispif_out_info.split_point);

  return 0;
}

/** scaler44_trigger44_enable
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler44_trigger_enable(scaler44_t *scaler,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    ISP_ERR(" size mismatch, expecting = %zu, received = %u",
          sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  scaler->trigger_enable = enable->enable_mask;
  return 0;
}

/** scaler44_enable
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input struct size
 *
 *  scaler module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler44_enable(scaler44_t *scaler, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  int i;

  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    ISP_ERR(" size mismatch, expecting = %zu, received = %u",
          sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  scaler->enable = enable->enable_mask;
   /* set all scaler entries to not used */
  scaler->scalers.hw_update_pending = FALSE;
  scaler->scalers.is_used = enable->enable_mask;/* now we enable both scalers */

  return 0;
}

/** scaler44_calculate_zoom_scaling
 *    @pix_setting: pix path settings
 *    @entry_idx: pix path index
 *    @y_cfg: output config
 *    @crop_factor: crop factor
 *
 *  Based on input/output aspect ratio, calculate width, height
 *
 * Return: nothing
 **/
static void scaler44_calculate_zoom_scaling(scaler44_t *scaler,
  ISP_Y_ScaleCfgCmdType* y_cfg, uint32_t crop_factor)
{
  uint32_t hout_factor = y_cfg->hOut * crop_factor;
  uint32_t vout_factor = y_cfg->vOut * crop_factor;
  uint32_t tmp1 = y_cfg->hIn *
    scaler->hw_stream_info.height;
  uint32_t tmp2 = y_cfg->vIn *
    scaler->hw_stream_info.width;
  uint32_t hOut, vOut;

  ISP_DBG("hout_factor %d vout_factor %d", hout_factor, vout_factor);
  ISP_DBG("tmp1 %d tmp2 %d", tmp1, tmp2);

  if (tmp1 == tmp2) {
    /* same aspect ratio */
    hOut = hout_factor / Q12;
    vOut = vout_factor / Q12;
    /* bounded by camif box */
    if (hOut > y_cfg->hIn || vOut > y_cfg->vIn) {
      hOut = y_cfg->hIn;
      vOut = y_cfg->vIn;
    }
  } else if (tmp1 < tmp2) {
    /* input aspect < output aspect ratio: output width bigger */
    hOut = hout_factor / Q12;
    if (hOut > y_cfg->hIn)
      hOut = y_cfg->hIn;
    vOut = hOut * y_cfg->vIn / y_cfg->hIn;
  } else {
    /* input aspect ratio > output aspect ratio: output width shorted */
    vOut = vout_factor / Q12;
    if (vOut > y_cfg->vIn)
      vOut = y_cfg->vIn;
    hOut = vOut * y_cfg->hIn / y_cfg->vIn;
  }
  y_cfg->hOut = hOut;
  y_cfg->vOut = vOut;
  ISP_DBG("hOut %d vOut %d", hOut, vOut);
  ISP_DBG("y_cfg->hOut %d y_cfg->vOut %d", y_cfg->hOut, y_cfg->vOut);
}

/** scaler44_check_hw_limit
 *    @entry_idx: pix path index
 *    @y_cfg:
 *    @in_aspect_ratio:
 *
 * Check of output dimensions are withing the limit, if not
 * clamp to maximum
 *
 * Return: nothing
 **/
static void scaler44_check_hw_limit(scaler44_t *scaler,
  ISP_Y_ScaleCfgCmdType* y_cfg, int in_aspect_ratio)
{
  uint32_t scaler_output_width = 0;
  float scale_factor = 0.0;
  float max_scale_factor = 0.0;
  uint32_t actual_camif_width = 0;
  uint32_t entry_idx = 0;
  int  down_scaling_factor = 0;

  entry_idx = scaler->entry_idx;

  scale_factor = (float)y_cfg->hOut / (float)y_cfg->hIn;

  if (scaler->rtb_mode){
    ISP_HIGH("RTB mode is set can not downscale\n");
    scaler->request_crop = 1;
    return;
  }

  if ((scale_factor > scaler->max_scale_factor) &&
    ((y_cfg->hIn * scaler->max_scale_factor) >=
    scaler->hw_stream_info.width)) {

    /* Restrict scaler output to max */
    y_cfg->hOut = y_cfg->hIn * scaler->max_scale_factor;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
    /*report crop request to downstream component*/
    scaler->request_crop = 1;
  }

  /* check the output against the hw stream output */
  if (y_cfg->hOut < scaler->hw_stream_info.width ||
      y_cfg->vOut < scaler->hw_stream_info.height) {
    y_cfg->vOut = y_cfg->vIn;
    y_cfg->hOut = y_cfg->hIn;
    ISP_HIGH("Can not do downscale\n");
    scaler->request_crop = 1;
    return;
  }

  /* this limitation not related to pixel numbers,
     but related to scale ratio only*/
  if ((y_cfg->hOut * ISP_SCALER44_MAX_SCALER_FACTOR) < y_cfg->hIn) {
    ISP_ERR(" hOut / hIn < 1/16");
    y_cfg->hOut = (y_cfg->hIn + ISP_SCALER44_MAX_SCALER_FACTOR -1) /
      ISP_SCALER44_MAX_SCALER_FACTOR;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
  }

  if ((y_cfg->vOut * ISP_SCALER44_MAX_SCALER_FACTOR) < y_cfg->vIn) {
    ISP_ERR(" vOut / vIn < 1/16");
    y_cfg->vOut = (y_cfg->vIn + ISP_SCALER44_MAX_SCALER_FACTOR -1)/
      ISP_SCALER44_MAX_SCALER_FACTOR;
    y_cfg->hOut = y_cfg->vOut * in_aspect_ratio / Q12;
  }

  down_scaling_factor = (y_cfg->hIn * 100) / y_cfg->hOut;
  if (down_scaling_factor < SCALE_RATIO_LIMIT) {

    if (scaler->crop_factor_applied != Q12) {
      y_cfg->hOut = (y_cfg->hIn * 100) / SCALE_RATIO_LIMIT;
      y_cfg->vOut = (y_cfg->vIn * 100) / SCALE_RATIO_LIMIT;

      if(y_cfg->hOut < scaler->hw_stream_info.width ||
        y_cfg->vOut < scaler->hw_stream_info.height) {
        y_cfg->hOut = y_cfg->hIn;
        y_cfg->vOut = y_cfg->vIn;
      }
    } else {
      y_cfg->hOut = y_cfg->hIn;
      y_cfg->vOut = y_cfg->vIn;
    }
    scaler->request_crop = 1;
    ISP_DBG("Entry %d y_cfg->hOut %d y_cfg->vOut %d",
      entry_idx ,y_cfg->hOut, y_cfg->vOut);
  }

  return;
}

/** scaler44_get_camif_sizes
 *    @scaler:       fov module control
 *    @camif_w:      width after camif
 *    @camif_h:      height after camif
 *
 * Extracts camif dimensions from sensor crop data.
 *
 * Return: None
 **/
static void scaler44_get_camif_sizes(scaler44_t *scaler, int32_t *camif_w,
    int32_t *camif_h)
{
  /* Derive the scaling factor, before that adjust the aspect ratio
     of crop_window to required output */
  /* 1. Modify the crop window according to the output */
  if (scaler->is_bayer_sensor)
    *camif_w = scaler->sensor_out_info.request_crop.last_pixel -
      scaler->sensor_out_info.request_crop.first_pixel + 1;
  else
    *camif_w = (scaler->sensor_out_info.request_crop.last_pixel -
       scaler->sensor_out_info.request_crop.first_pixel + 1) >> 1;

  *camif_h = scaler->sensor_out_info.request_crop.last_line -
      scaler->sensor_out_info.request_crop.first_line + 1;
}

/** scaler44_calculate_dis
 *    @scaler:       fov module control
 *
 * Update dimensions of scaler depending on DIS
 *
 * Return: None
 **/
void scaler44_calculate_dis(scaler44_t *scaler)
{
  int32_t camif_w, camif_h;

  if (scaler->dis_enable) {
    scaler44_get_camif_sizes(scaler, &camif_w, &camif_h);
    if(scaler->hw_stream_info.width > camif_w)
      scaler->hw_stream_info.width = camif_w;
    if(scaler->hw_stream_info.height > camif_h)
      scaler->hw_stream_info.height = camif_h;
    scaler44_config(scaler);
  }
  scaler->hw_stream_info.dis_enabled = scaler->dis_enable;
}

static uint32_t is_rollover_point(uint32_t M, uint32_t N, uint32_t offset,
  uint32_t *mn_init)
{
  uint32_t mn_init_1 = offset * M % N;
  uint32_t mn_init_2 = (offset + 1) * M % N;

  if (mn_init_1 == 0) {
    *mn_init = 0;
    return 1;
  }
  if (mn_init_2 < M && mn_init_2 != 0) {
    *mn_init = mn_init_2;
    return 1;
  }
  return 0;
}

static void scaler_calculate_phase(uint32_t  M, uint32_t  N, uint32_t  offset,
  uint32_t interp_reso, uint32_t *mn_init, uint32_t *phase_init,
  uint32_t *h_skip_cnt)
{

  *h_skip_cnt = 0;
  while (!is_rollover_point(M, N, offset, mn_init)) {
    offset++;
    (*h_skip_cnt)++;
  }
  *phase_init = (*mn_init << (13 + interp_reso)) / M;
}

/** scaler44_config
 *    @scaler:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: TRUE on success and FALSE on failure
 **/
boolean scaler44_config(scaler44_t *scaler)
{
  boolean  rc              = TRUE;
  float    zoom_scaling    = 0.0;
  uint32_t interp_reso     = 0;
  uint32_t mn_init         = 0;
  uint32_t phase_init      = 0;
  uint32_t h_skip_cnt      = 0;
  float    scale_w         = 0;
  float    scale_h         = 0;
  uint32_t cbcr_hFactor    = 0;
  uint32_t cbcr_vFactor    = 0;
  uint32_t y_hFactor       = 0;
  uint32_t y_vFactor       = 0;
  int      input_ratio     = 0;
  int      output_ratio    = 0;
  int      phase_increment = 0;

  unsigned int cbcr_scale_factor_horiz = 0;
  unsigned int cbcr_scale_factor_vert  = 0;
  unsigned int y_scale_factor_horiz    = 0;
  unsigned int y_scale_factor_vert     = 0;

  isp_scaler44_entry_t*     entry     = &scaler->scalers;
  ISP_Y_ScaleCfgCmdType*    y_cfg     = &entry->reg_cmd.Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType* cbcr_cfg  = &entry->reg_cmd.CbCr_ScaleCfg;
  ISP_Y_ScalePadCfgCmdType* y_pad_cfg = &entry->reg_cmd.Y_ScalePadCfg;

  /* TODO: need to add cropping into scaler */
  if (scaler->hw_stream_info.width == 0) {
    entry->is_used = 0;
    entry->hw_update_pending = FALSE;
    return TRUE;
  }
  entry->is_used = 1;
  scaler->request_crop = 0;

  if (!scaler->sensor_out_info.request_crop.last_pixel ||
    !scaler->sensor_out_info.request_crop.last_line) {
    return TRUE;
  }

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;

  if (scaler->is_bayer_sensor)
    y_cfg->hIn = scaler->sensor_out_info.request_crop.last_pixel -
      scaler->sensor_out_info.request_crop.first_pixel + 1;
  else
    y_cfg->hIn =
      (scaler->sensor_out_info.request_crop.last_pixel -
       scaler->sensor_out_info.request_crop.first_pixel + 1) >> 1;

  y_cfg->vIn =
    scaler->sensor_out_info.request_crop.last_line -
      scaler->sensor_out_info.request_crop.first_line + 1;

  input_ratio = y_cfg->hIn * Q12 / y_cfg->vIn;

  output_ratio = scaler->hw_stream_info.width * Q12 /
    scaler->hw_stream_info.height;

  if (output_ratio == input_ratio) {
    y_cfg->hOut = scaler->hw_stream_info.width;
    y_cfg->vOut = scaler->hw_stream_info.height;
  } else if (output_ratio > input_ratio) {
    y_cfg->hOut = scaler->hw_stream_info.width;
    y_cfg->vOut = scaler->hw_stream_info.width *
      Q12 / input_ratio;
  } else {
    y_cfg->hOut = scaler->hw_stream_info.height *
      input_ratio / Q12;
    y_cfg->vOut = scaler->hw_stream_info.height;
  }

  ISP_DBG("scaler->crop_factor_applied %d", scaler->crop_factor_applied);
  /* In RTB mode, RTB algo need 1x frames even we zoom,
   * to support this feature ISP will disable zooming feature
   */
  if (!scaler->rtb_mode){
     scaler44_calculate_zoom_scaling(scaler, y_cfg, scaler->crop_factor_applied);
  }
  scaler44_check_hw_limit(scaler, y_cfg, input_ratio);

  /* make sure its even number so CbCr will match Y instead of rounding down */
  y_cfg->hOut = CEILING2(y_cfg->hOut);
  y_cfg->vOut = CEILING2(y_cfg->vOut);

  if (output_ratio > input_ratio)
    entry->scaling_factor = (float)y_cfg->hIn / (float)y_cfg->hOut;
  else
    entry->scaling_factor = (float)y_cfg->vIn / (float)y_cfg->vOut;

  scaler->scaling_factor_update = TRUE;
  y_scale_factor_horiz = y_cfg->hIn / y_cfg->hOut;
  y_scale_factor_vert = y_cfg->vIn / y_cfg->vOut;

  rc = calculate_scaler44_factor(y_scale_factor_horiz,
    y_scale_factor_vert, &y_hFactor, &y_vFactor);

  y_pad_cfg->hSkipCount = h_skip_cnt;

  phase_increment = 13 + y_hFactor;
  y_cfg->horizInterResolution = y_hFactor;
  y_cfg->horizPhaseMult = (y_cfg->hIn << phase_increment) / y_cfg->hOut;
  y_cfg->horizMNInit = 0;
  y_cfg->horizPhaseInit = 0;
  y_cfg->horizPhaseInitMsb = 0;

  phase_increment = 13 + y_vFactor;
  y_cfg->vertInterResolution = y_vFactor;
  y_cfg->vertPhaseMult = (y_cfg->vIn << phase_increment) / y_cfg->vOut;
  y_cfg->vertMNInit = 0;
  y_cfg->vertPhaseInit = 0;
  y_cfg->vertPhaseInitMsb = 0;

  y_pad_cfg->scaleYInWidth    = y_cfg->hIn;
  y_pad_cfg->scaleYInHeight   = y_cfg->vIn;
  y_pad_cfg->hSkipCount       = 0;
  y_pad_cfg->vSkipCount       = 0;
  y_pad_cfg->rightPadEnable   = 0;
  y_pad_cfg->bottomPadEnable  = 0;

  cbcr_cfg->hEnable  = TRUE;
  cbcr_cfg->vEnable = TRUE;
  cbcr_cfg->hIn = y_cfg->hIn;
  cbcr_cfg->vIn = y_cfg->vIn;

  isp_sub_module_getscalefactor(scaler->hw_stream_info.fmt, &scale_w, &scale_h);

  cbcr_cfg->hOut = y_cfg->hOut / scale_w;
  cbcr_cfg->vOut = y_cfg->vOut / scale_h;

  if (scaler->hw_stream_info.need_uv_subsample) {
    cbcr_cfg->vOut /= 2;
    cbcr_cfg->hOut /= 2;
  }

  cbcr_scale_factor_horiz = cbcr_cfg->hIn / cbcr_cfg->hOut;
  cbcr_scale_factor_vert = cbcr_cfg->vIn / cbcr_cfg->vOut;

  rc = calculate_scaler44_factor(cbcr_scale_factor_horiz,
    cbcr_scale_factor_vert, &cbcr_hFactor, &cbcr_vFactor);

  phase_increment = 13 + cbcr_hFactor;
  cbcr_cfg->horizInterResolution = cbcr_hFactor;
  cbcr_cfg->horizPhaseMult = (cbcr_cfg->hIn << phase_increment) /
    cbcr_cfg->hOut;
  cbcr_cfg->horizMNInit = 0;
  cbcr_cfg->horizPhaseInit = 0;

  phase_increment = 13 + cbcr_vFactor;
  cbcr_cfg->vertInterResolution = cbcr_vFactor;
  cbcr_cfg->vertPhaseMult = (cbcr_cfg->vIn << phase_increment) / cbcr_cfg->vOut;
  cbcr_cfg->vertMNInit = 0;
  cbcr_cfg->vertPhaseInit = 0;

  cbcr_cfg->rightPadEnable = 0;
  cbcr_cfg->hSkipCount = 0;
  cbcr_cfg->scaleCbCrInWidth = cbcr_cfg->hIn;
  cbcr_cfg->bottomPadEnable = 0;
  cbcr_cfg->vSkipCount = 0;
  cbcr_cfg->scaleCbCrInHeight = cbcr_cfg->vIn;

  if ((scaler->ispif_out_info.is_split == TRUE) &&
    (scaler->isp_out_info.stripe_id == ISP_STRIPE_RIGHT)) {
    /* This scaler is for right VFE. calculate actual scaler output
     * coming out of right VFE. this info will be used by fovcrop to
     * configure its register
     */
    scaler_calculate_phase(y_cfg->hOut, y_cfg->hIn,
      scaler->isp_out_info.right_stripe_offset, y_hFactor, &mn_init,
      &phase_init, &h_skip_cnt);
    y_cfg->horizMNInit = mn_init;
    y_pad_cfg->hSkipCount = h_skip_cnt;
    y_cfg->horizPhaseInit = phase_init;
    y_pad_cfg->rightPadEnable = 1;
    y_pad_cfg->bottomPadEnable = 1;

    scaler_calculate_phase(cbcr_cfg->hOut, cbcr_cfg->hIn,
      scaler->isp_out_info.right_stripe_offset, cbcr_hFactor, &mn_init,
      &phase_init, &h_skip_cnt);
    cbcr_cfg->horizMNInit = mn_init;
    cbcr_cfg->hSkipCount = h_skip_cnt;
    cbcr_cfg->rightPadEnable = 1;
    cbcr_cfg->bottomPadEnable = 1;
    cbcr_cfg->horizPhaseInit = phase_init;
  }

  ISP_DBG("y %d %d %d %d cbcr %d %d %d %d", y_cfg->hIn,
    y_cfg->hOut, y_cfg->vIn, y_cfg->vOut, cbcr_cfg->hIn, cbcr_cfg->hOut,
    cbcr_cfg->vIn, cbcr_cfg->vOut);
  if ((y_cfg->hIn == y_cfg->hOut) && (y_cfg->vIn == y_cfg->vOut)) {
    y_cfg->hEnable = 0;
    y_cfg->vEnable = 0;
  }

  if ((cbcr_cfg->hIn == cbcr_cfg->hOut) && (cbcr_cfg->vIn == cbcr_cfg->vOut)) {
    cbcr_cfg->hEnable = 0;
    cbcr_cfg->vEnable = 0;
  }

  entry->hw_update_pending = TRUE;
  return rc;
}

/** scaler444update_zoom_params
 *    @scaler: Scaler module struct data
 *    @upd_zoom_params: zoom params to update
 *    @identity: stream identity
 *
 * update zoom params from scaler data;
 *
 * Return:   0 - success
 *          -1 - NULL parameter supplied
 **/
static int scaler44_update_zoom_params(scaler44_t *scaler,
  isp_zoom_params_t *upd_zoom_params, uint32_t identity)
{
  int i,j;
  isp_zoom_params_t *zoom_params;

  if(!scaler || !upd_zoom_params) {
    ISP_ERR("failed: scaler %p upd_zoom_params %p", scaler, upd_zoom_params);
    return -1;
  }

  if (!scaler->session_based_identity) {
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      zoom_params = &upd_zoom_params[i];
      if (zoom_params->identity) {
        continue;
      }
      zoom_params->identity = identity;
      zoom_params->camif_output.width = scaler->scalers.reg_cmd.Y_ScaleCfg.hIn;
      zoom_params->camif_output.height = scaler->scalers.reg_cmd.Y_ScaleCfg.vIn;
      zoom_params->scaling_ratio = scaler->scalers.scaling_factor;
      zoom_params->scaler_output.width =
        scaler->scalers.reg_cmd.Y_ScaleCfg.hOut;
      zoom_params->scaler_output.height =
        scaler->scalers.reg_cmd.Y_ScaleCfg.vOut;
      ISP_DBG("i %d ide %x scale %f out %d %d", i, identity,
        zoom_params->scaling_ratio, zoom_params->scaler_output.width,
        zoom_params->scaler_output.height);
      break;
    }
  } else {
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if (scaler->identity[j] &&
        (scaler->identity[j] != scaler->session_based_identity)) {
        for (i = 0; i < ISP_MAX_STREAMS; i++) {
          zoom_params = &upd_zoom_params[i];
          if (zoom_params->identity) {
            continue;
          }
          zoom_params->identity = scaler->identity[j];
          zoom_params->camif_output.width =
            scaler->scalers.reg_cmd.Y_ScaleCfg.hIn;
          zoom_params->camif_output.height =
            scaler->scalers.reg_cmd.Y_ScaleCfg.vIn;
          zoom_params->scaling_ratio = scaler->scalers.scaling_factor;
          zoom_params->scaler_output.width =
            scaler->scalers.reg_cmd.Y_ScaleCfg.hOut;
          zoom_params->scaler_output.height =
            scaler->scalers.reg_cmd.Y_ScaleCfg.vOut;
          ISP_DBG("i %d ide %x scale %f out %d %d", i, scaler->identity[j],
            zoom_params->scaling_ratio, zoom_params->scaler_output.width,
            zoom_params->scaler_output.height);
          break;
        }
      }
    }
  }

  return 0;
}

/** scaler44_do_hw_update
 * @scaler: scaler module struct data
 * @hw_update_list: linked list ent from pipeline layer which
 *      scaler44 will populate with register address and data info
 * @identity: event identity
 *
 * update scaler module register to kernel
 *
 * Return: nothing
 **/
static boolean scaler44_do_hw_update(scaler44_t *scaler,
  isp_sub_module_output_t *output, uint32_t identity)
{
  boolean ret = TRUE;
  int rc;
  ISP_ScaleCfgCmdType *reg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  uint32_t *data = NULL;
  uint32_t len = 0;
  isp_zoom_params_t         *zoom_params = NULL;
  uint32_t                   i = 0, j = 0;

  if (!scaler || !output || !output->stats_params) {
    ISP_ERR("failed: scaler %p output %p", scaler, output);
    return FALSE;
  }

  ISP_DBG("HW_update, scaler[%d] = %d\n",
       scaler->entry_idx, scaler->scalers.hw_update_pending);

  if (scaler->update_enable_bit == TRUE) {
    scaler44_get_module_enable_mask(scaler,
      &output->stats_params->module_enable_info);
    scaler->update_enable_bit = FALSE;
  }

  /* pass cds update flag if this hw update list has cds updating going on */
  if (scaler->is_cds_update == TRUE) {
    *(output->is_cds_update) = scaler->is_cds_update;
    scaler->is_cds_update = FALSE;
  }

  if (scaler->scalers.hw_update_pending == TRUE) {
    scaler44_cmd_debug(&scaler->scalers.reg_cmd, scaler->entry_idx);

    reg_cmd = (ISP_ScaleCfgCmdType *)CAM_MALLOC(sizeof(ISP_ScaleCfgCmdType));
    if (!reg_cmd) {
      ISP_ERR("failed: no memory ISP_ScaleCfgCmdType");
      return FALSE;
    }
    memset(reg_cmd, 0, sizeof(*reg_cmd));
    memcpy(reg_cmd, &scaler->scalers.reg_cmd, sizeof(*reg_cmd));

    hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
    if (!hw_update) {
      ISP_ERR("failed: hw_update %p", hw_update);
      free(reg_cmd);
      return FALSE;
    }
    memset(hw_update, 0, sizeof(*hw_update));

    reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
      CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 2);
    if (!reg_cfg_cmd) {
      ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
      free(hw_update);
      free(reg_cmd);
      return FALSE;
    }
    memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

    cfg_cmd = &hw_update->cfg_cmd;
    cfg_cmd->cfg_data = (void *)reg_cmd;
    cfg_cmd->cmd_len = sizeof(*reg_cmd);
    cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd->num_cfg = 2;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = scaler->reg_off_0;
    reg_cfg_cmd[0].u.rw_info.len = scaler->reg_len_0 * sizeof(uint32_t);

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset = reg_cfg_cmd[0].u.rw_info.len;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE;
    reg_cfg_cmd[1].u.rw_info.reg_offset = scaler->reg_off_1;
    reg_cfg_cmd[1].u.rw_info.len = scaler->reg_len_1 * sizeof(uint32_t);

    ISP_HIGH("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
      reg_cfg_cmd[0].u.rw_info.len);
    data = (uint32_t *)cfg_cmd->cfg_data;
    for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
      ISP_DBG("data[%d] %x", len, data[len]);
    }
    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = scaler44_util_append_cfg(&output->hw_update_list, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_util_append_cfg");
      free(reg_cfg_cmd);
      free(hw_update);
      free(reg_cmd);
      return FALSE;
    }
    scaler->scalers.hw_update_pending = FALSE;
  }
  output->stats_params->zoom = scaler->zoom_value;
  ISP_DBG("ZOOMDEBUG user zoom %d stream type %d",
    scaler->zoom_value.user_zoom, scaler->stream_type);
  rc = scaler44_update_zoom_params(scaler, output->stats_params->zoom_params,
    identity);
  if (rc < 0) {
    ISP_ERR("failed: scaler44_update_zoom_params");
    return FALSE;
  }

  if (output->metadata_dump_enable == 1) {
    output->meta_dump_params->frame_meta.addlinfo.CDS_enable =
      scaler->hw_stream_info.need_uv_subsample;
  }
  if (output->frame_meta) {
      output->frame_meta->crop_window = scaler->applied_crop_window;
      output->frame_meta->zoom_value  = scaler->zoom_value;
  }
  return ret;
}

boolean scaler44_parm_crop_region(mct_module_t *module,
  scaler44_t *scaler, cam_crop_region_t *crop_window, uint32_t identity)
{
  boolean ret = TRUE;
  int32_t camif_w, camif_h,output_width, output_height, temp1, temp2;
  float aspect_ratio1,aspect_ratio2,scale_factor;
  cam_crop_region_t tmp_crop_window;
  boolean is_stream_on = FALSE;

  if (!module || !scaler || !crop_window) {
    ISP_ERR("failed: %p %p %p", module, scaler, crop_window);
    return FALSE;
  }

  scaler44_get_camif_sizes(scaler, &camif_w, &camif_h);

  tmp_crop_window = *crop_window;

  if ((tmp_crop_window.width  + tmp_crop_window.left) > camif_w ||
      (tmp_crop_window.height + tmp_crop_window.top) > camif_h) {
       ISP_ERR("Invalid crop window, ignore");
       return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  output_width = scaler->hw_stream_info.width;
  output_height = scaler->hw_stream_info.height;

  aspect_ratio1 = (float)output_width / output_height;
  aspect_ratio2 = (float)tmp_crop_window.width/tmp_crop_window.height;

  if (fabs(aspect_ratio1 - aspect_ratio2) > ASPECT_TOLERANCE) {
      if (aspect_ratio1 < aspect_ratio2) {
      /* O/P aspect ratio < crop window aspect ratio */
        scaler->modified_crop_window.width =
          tmp_crop_window.height * aspect_ratio1;
        scaler->modified_crop_window.width =
          FLOOR16(scaler->modified_crop_window.width);

        scaler->modified_crop_window.left = tmp_crop_window.left +
          ((tmp_crop_window.width - scaler->modified_crop_window.width)>>1);

        scaler->modified_crop_window.height = tmp_crop_window.height;
        scaler->modified_crop_window.top = tmp_crop_window.top;

      } else if(aspect_ratio1 > aspect_ratio2){
      /* O/P aspect ratio > crop window aspect ratio */
        scaler->modified_crop_window.height =
          (float)tmp_crop_window.width/aspect_ratio1;
        scaler->modified_crop_window.height =
          FLOOR16(scaler->modified_crop_window.height);

        scaler->modified_crop_window.top = tmp_crop_window.top +
          ((tmp_crop_window.height - scaler->modified_crop_window.height)>>1);

        scaler->modified_crop_window.width = tmp_crop_window.width;
        scaler->modified_crop_window.left = tmp_crop_window.left;
      }
  } else {

      scaler->modified_crop_window = tmp_crop_window;
  }

  scaler->applied_crop_window = *crop_window;

  ISP_DBG("entry_idx %d, Crop Window Actual [%d,%d,%d,%d], Modified[%d,%d,%d,%d]\n",
    scaler->entry_idx,
    crop_window->left,crop_window->top,crop_window->width,crop_window->height,
    scaler->modified_crop_window.left,
    scaler->modified_crop_window.top,
    scaler->modified_crop_window.width,
    scaler->modified_crop_window.height);

  /* 2. Calculate the scaling factor, based on the crop window and
     Sensor output. Every time the crop_window decreases the crop_factor_applied increases
     (Scalar output increases)*/
  temp1 = (camif_w * Q12) /
      scaler->modified_crop_window.width;

  temp2 = (camif_h * Q12) /
    scaler->modified_crop_window.height;

  scaler->crop_factor_applied = MIN(temp1, temp2);
  scaler44_config(scaler);
  scaler->hw_update_pending = TRUE;

  is_stream_on = (scaler->stream_cnt > 0) ? TRUE : FALSE;

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);

  if (is_stream_on == TRUE) {
    ret = module_scaler44_pass_scale_factor(module, scaler, is_stream_on, identity);
  }

  return ret;
}


boolean scaler44_parm_zoom(mct_module_t *module, scaler44_t *scaler,
  cam_zoom_info_t *zoom_val, uint32_t identity)
{
  boolean ret = TRUE;
  boolean is_stream_on = FALSE;
  int i = 0;

  if (!module || !scaler || !zoom_val) {
    ISP_ERR("failed: %p %p %p", module, scaler, zoom_val);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);

  scaler->zoom_value = *zoom_val;

  if (!zoom_val->is_stream_zoom_info_valid) {
    ret = isp_zoom_get_crop_factor(&scaler->zoom, scaler->zoom_value.user_zoom,
      &scaler->crop_factor_applied);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_zoom_get_crop_factor");
    }
    scaler->crop_factor_calculate_crop = scaler->crop_factor_applied;
  } else {
    for (i = 0; i < scaler->zoom_value.num_streams; i++) {
      /* For Dual camera cases specifically, we need to support per stream
       * zoom control. The limitation is that all different stream zoom are
       * expected on mapped on different HW stream. This today is specific
       * to preview/snapshot which are guaranteed to be mapped to different
       * HW stream */
      if (scaler->stream_type == zoom_val->stream_zoom_info[i].stream_type) {
        ret = isp_zoom_get_crop_factor(&scaler->zoom,
        scaler->zoom_value.stream_zoom_info[i].isp_zoom,
          &scaler->crop_factor_applied);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_zoom_get_crop_factor");
        }
        ret = isp_zoom_get_crop_factor(&scaler->zoom,
        scaler->zoom_value.stream_zoom_info[i].stream_zoom,
          &scaler->crop_factor_calculate_crop);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_zoom_get_crop_factor");
        }
      }
    }
  }

  ISP_DBG("ZOOMDEBUG zoom_val %d crop factor %d",
    scaler->crop_factor_calculate_crop,
    scaler->crop_factor_applied);

  scaler44_config(scaler);

  scaler->hw_update_pending = TRUE;

  is_stream_on = (scaler->stream_cnt > 0) ? TRUE : FALSE;

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);

  if (ret && (is_stream_on == TRUE)) {
    ret = module_scaler44_pass_scale_factor(module, scaler, is_stream_on, identity);
  }

  return ret;
}

/** scaler44_reset
 *      @scaler: scaler module struct data
 *
 * Scaler module disable hw updates,release reg settings and
 * structs
 *
 * Return: nothing
 **/
void scaler44_reset(scaler44_t *scaler)
{
  /* Enable module and trigger enable by default */
  scaler->trigger_enable = 1;
  scaler->enable = 1;
  scaler->applied_crop_factor = 0;
  scaler->is_cds_update = FALSE;
  scaler->scalers.hw_update_pending = FALSE;
  memset(&scaler->scalers.reg_cmd, 0, sizeof(scaler->scalers.reg_cmd));
  memset(&scaler->isp_out_info,0, sizeof(scaler->isp_out_info));
  memset(&scaler->ispif_out_info,0, sizeof(scaler->ispif_out_info));
  scaler->scalers.is_used = 0;

  /* Init the Crop window to active pixel array */
  scaler->modified_crop_window.left = 0;
  scaler->modified_crop_window.top = 0;

  scaler->modified_crop_window.width =
    scaler->sensor_out_info.dim_output.width;

  scaler->modified_crop_window.height =
    scaler->sensor_out_info.dim_output.height;

  scaler->applied_crop_window = scaler->modified_crop_window;
  /* In RTB mode isp zoom will disabled */
  scaler->rtb_mode = 0;

}

/** scaler44_init
 *    @mod_ctrl: scaler module control strcut
 *
 *  scaler module data struct initialization
 *
 * Return: 0 always
 **/
boolean scaler44_init(scaler44_t *scaler)
{
  scaler44_reset(scaler);
  scaler->crop_factor_applied = Q12;
  scaler->crop_factor_calculate_crop = Q12;
  return TRUE;
}

/** scaler44_get_module_enable_mask:
 *
 *  @scaler: fov crop handle
 *  @module_enable: handle to mask module enable bit
 *
 *  Enable / disable this module based on module enable flag
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler44_get_module_enable_mask(scaler44_t *scaler,
  isp_module_enable_info_t *enable_bit_info)
{
  if (!scaler || !enable_bit_info) {
    ISP_ERR("failed: %p %p", scaler, enable_bit_info);
    return FALSE;
  }

  enable_bit_info->reconfig_needed = TRUE;
  enable_bit_info->submod_mask[scaler->hw_module_id] = 1;
  if (scaler->enable) {
    enable_bit_info->submod_enable[scaler->hw_module_id] = 1;
  } else {
    enable_bit_info->submod_enable[scaler->hw_module_id] = 0;
  }

  return TRUE;
}

/** scaler44_get_module_enable_mask:
 *
 *  @scaler: fov crop handle
 *  @crop_factor: handle to get crop factor
 *
 *  Return crop factor
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler44_get_crop_factor(scaler44_t *scaler,
  isp_crop_factors_t *crop_factor)
{
  if (!scaler || !crop_factor) {
    ISP_ERR("failed: %p %p", scaler, crop_factor);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  crop_factor->crop_applied = scaler->crop_factor_applied;
  crop_factor->crop_to_calculate = scaler->crop_factor_calculate_crop;
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);

  return TRUE;
}

static uint32_t scaler44_calculate_right_split_width(uint32_t M, uint32_t N,
  uint32_t mn_init, uint32_t h_skip_cnt, uint32_t actual_input)
{
    // mn_init and h_skip_cnt are from ones used in scaler
    return (mn_init + (actual_input - h_skip_cnt) * M) / N;
}

/** scaler44_set_max_scale_factor:
 *
 *  @scaler: scaler handle
 *
 *  Set max scaler factor for both vfe based by comparing max scaler
 *  factor supported by both
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler44_set_max_scale_factor(scaler44_t *scaler)
{
  uint32_t left_camif_width = 0, right_camif_width = 0, full_camif_width = 0;
  float left_max_scale_factor = 1.0f, right_max_scale_factor = 1.0f;

  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  if (!scaler->isp_out_info.is_split) {
    return TRUE;
  }

  /* Calculate full size camif width */
  full_camif_width = scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;

  /* Calculate left camif width and max scaler factor for left VFE */
  left_camif_width = scaler->isp_out_info.right_stripe_offset
    + scaler->ispif_out_info.overlap;
  if (left_camif_width > scaler->hw_limits.max_width) {
    left_max_scale_factor = (float)scaler->hw_limits.max_width /
      (float)left_camif_width;
  }

  /* Calculate right camif width and max scale factor for right VFE */
  right_camif_width = full_camif_width -
    scaler->isp_out_info.right_stripe_offset;
  if (right_camif_width > scaler->hw_limits.max_width) {
    right_max_scale_factor = (float)scaler->hw_limits.max_width /
      (float)right_camif_width;
  }

  /* In case of dual vfe + zoom => both VFE should apply same scaler factor,
   * else, it will cause dual vfe split. Find smaller max scaler factor
   * of both VFE and save it in internal data struct. Use this as
   * max scaler factor for both VFE during trigger_update. This is to
   * ensure that we apply same max scaler factor for both vfe even if one
   * vfe run beyond max supported scaler factor
   */
  scaler->max_scale_factor = MIN(left_max_scale_factor, right_max_scale_factor);

  return TRUE;
}

static boolean scaler44_loop_saved_events(mct_module_t *module,
  scaler44_t *scaler, isp_saved_events_t *saved_events, uint32_t identity)
{
  mct_event_control_parm_t *hal_param = NULL;

  /* Handler set param */
  if (saved_events->set_params[ISP_SET_PARM_DIS_ENABLE] &&
    (saved_events->set_params_valid[ISP_SET_PARM_DIS_ENABLE] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_DIS_ENABLE]->u.ctrl_event.
      control_event_data;
    port_scaler44_handle_hal_set_parm(module, scaler, hal_param, identity);
  }

  if (saved_events->set_params[ISP_SET_PARM_ZOOM] &&
    (saved_events->set_params_valid[ISP_SET_PARM_ZOOM] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_ZOOM]->u.ctrl_event.
      control_event_data;
    port_scaler44_handle_hal_set_parm(module, scaler, hal_param, identity);
  }

  if (saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION] &&
    (saved_events->set_params_valid[ISP_SET_META_SCALER_CROP_REGION] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION]->u.ctrl_event.
      control_event_data;
    port_scaler44_handle_hal_set_parm(module, scaler, hal_param, identity);
  }

  if (saved_events->set_params[ISP_SET_PARM_UPDATE_DEBUG_LEVEL] &&
    (saved_events->set_params_valid[ISP_SET_PARM_UPDATE_DEBUG_LEVEL] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_UPDATE_DEBUG_LEVEL]->u.ctrl_event.
      control_event_data;
    port_scaler44_handle_hal_set_parm(module, scaler, hal_param, identity);
  }

  /* Handle module event */
  /* only CDS event is used by scaler in the list of module events */
  if (saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST] &&
    (saved_events->module_events_valid[ISP_MODULE_EVENT_CDS_REQUEST] == TRUE)) {
    port_scaler44_handle_cds_request(scaler,
      saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST]);
  }
  if (saved_events->set_params[ISP_SET_PARM_BOKEH_MODE] &&
    (saved_events->set_params_valid[ISP_SET_PARM_BOKEH_MODE] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_BOKEH_MODE]->u.ctrl_event.
      control_event_data;
    port_scaler44_handle_hal_set_parm(module, scaler, hal_param, identity);
  }
  return TRUE;
}

/** scaler44_handle_isp_private_event:
 *
 *  @scaler: scaler private
 *  @data: module event data
 *  @identity: identity of event
 *
 *  Handle ISP private event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_handle_isp_private_event(mct_module_t *module,
  scaler44_t *scaler, void *data, uint32_t identity)
{
  boolean              ret = TRUE;
  int                  rc = 0;
  isp_private_event_t *private_event = (isp_private_event_t *)data;

  if (!module || !scaler || !data) {
    ISP_ERR("failed: module %p scaler %p data %p", module, scaler, data);
    return FALSE;
  }

  switch (private_event->type) {
  case ISP_PRIVATE_GET_VFE_DIAG_INFO_USER:
    break;

  case ISP_PRIVATE_SET_MOD_ENABLE:
    rc = scaler44_enable(scaler,
      (isp_mod_set_enable_t *)private_event->data, private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: scaler44_enable rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_STRIPE_INFO: {
    ispif_out_info_t *ispif_stripe_info =
      (ispif_out_info_t *)private_event->data;
    scaler->ispif_out_info = *ispif_stripe_info;
  }
    break;

  case ISP_PRIVATE_SET_SENSOR_DIM: {
    sensor_set_dim_t *sensor_output_info =
        (sensor_set_dim_t *)private_event->data;
    scaler->sensor_out_info.dim_output.width =
      sensor_output_info->dim_output.width;
    scaler->sensor_out_info.dim_output.height =
      sensor_output_info->dim_output.height;
  }
  break;

  case ISP_PRIVATE_HW_LIMITATIONS: {
    isp_scaler_hw_limit_t *isp_hw_limit =
      (isp_scaler_hw_limit_t *)private_event->data;
    uint32_t i;
    for (i = 0; i < isp_hw_limit->num_hw_streams; i++) {
      if ((isp_hw_streamid_t)(scaler->entry_idx) ==
           isp_hw_limit->hw_limits[i].hw_stream_id) {
        scaler->hw_limits = isp_hw_limit->hw_limits[i];
        break;
      }
    }
  }
    break;

  case ISP_PRIVATE_REQUEST_STRIPE_OFFSET:
    rc = scaler44_compute_stripe_offset(scaler, private_event->data);
    if (rc < 0) {
      ISP_ERR("failed: scaler44_compute_stripe_offset rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_REQUEST_CDS_CAP: {
      uint32_t *cds_cap = (uint32_t *)private_event->data;
      *cds_cap = scaler->cds_cap[scaler->entry_idx];
  }
  break;

  case ISP_PRIVATE_SET_STREAM_SPLIT_INFO: {
    isp_out_info_t *isp_out_info =
      (isp_out_info_t *)private_event->data;
    scaler->isp_out_info = *isp_out_info;
    scaler44_set_max_scale_factor(scaler);
    scaler44_config(scaler);
  }
    break;

  case ISP_PRIVATE_SET_TRIGGER_ENABLE:
    rc = scaler44_trigger_enable(scaler, private_event->data,
      private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: scaler44_trigger_enable rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_ACTION_RESET:
    scaler44_reset(scaler);
    break;

  case ISP_PRIVATE_SET_TRIGGER_UPDATE: {
    isp_sub_module_output_t *output = private_event->data;
    if (output->saved_events) {
      scaler44_loop_saved_events(module, scaler, output->saved_events,
        identity);
    }
    PTHREAD_MUTEX_LOCK(&scaler->mutex);
    ret = scaler44_do_hw_update(scaler,
      (isp_sub_module_output_t *)private_event->data, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_do_hw_update ret %d", ret);
    }
    PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  }
    break;

  case ISP_PRIVATE_FETCH_SCALER_OUTPUT: {
    isp_pixel_window_info_t *scaler_output =
      (isp_pixel_window_info_t *)private_event->data;
    ISP_Y_ScaleCfgCmdType     *Y_ScaleCfg = NULL;
    ISP_CbCr_ScaleCfgCmdType  *CbCr_ScaleCfg = NULL;
    uint32_t scaler_input_width = 0;

    if (scaler->scalers.is_used) {
      scaler_output->height =
        scaler->scalers.reg_cmd.Y_ScaleCfg.vOut;
      scaler_output->width =
        scaler->scalers.reg_cmd.Y_ScaleCfg.hOut;
      scaler_output->scaling_factor = scaler->scalers.scaling_factor;
      ISP_DBG(" scaler_output[%d]: Width %d, Height %d, scalefactor %f",
        scaler->entry_idx, scaler_output->width, scaler_output->height,
        scaler_output->scaling_factor);

      if (scaler->ispif_out_info.is_split == TRUE) {
        Y_ScaleCfg = &scaler->scalers.reg_cmd.Y_ScaleCfg;
        CbCr_ScaleCfg = &scaler->scalers.reg_cmd.CbCr_ScaleCfg;

        scaler_input_width = scaler->sensor_out_info.request_crop.last_pixel -
          scaler->ispif_out_info.right_stripe_offset + 1;
        ISP_DBG("DUALVFE scaler_input_width %d right_stripe_offset %d",
          scaler_input_width, scaler->ispif_out_info.right_stripe_offset);

        scaler_output->right_split_y_width = scaler44_calculate_right_split_width(
          Y_ScaleCfg->hOut, Y_ScaleCfg->hIn, Y_ScaleCfg->horizMNInit,
          scaler->scalers.reg_cmd.Y_ScalePadCfg.hSkipCount, scaler_input_width);

        scaler_output->right_split_cbcr_width =
          scaler44_calculate_right_split_width(CbCr_ScaleCfg->hOut,
          CbCr_ScaleCfg->hIn, CbCr_ScaleCfg->horizMNInit,
          CbCr_ScaleCfg->hSkipCount, scaler_input_width);
        ISP_DBG("DUALVFE scaler output y %d cbcr %d",
          scaler_output->right_split_y_width,
          scaler_output->right_split_cbcr_width);
      }
      scaler_output->modified_crop_window = scaler->modified_crop_window;
    }
  }
    break;

  case ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO: {
    isp_hw_stream_info_t *hw_stream_info =
      (isp_hw_stream_info_t *)private_event->data;
    hw_stream_info->identity = scaler->hw_stream_info.identity;
    hw_stream_info->need_uv_subsample =
      scaler->hw_stream_info.need_uv_subsample;
    hw_stream_info->fmt = scaler->hw_stream_info.fmt;
    hw_stream_info->width = scaler->hw_stream_info.width;
    hw_stream_info->height = scaler->hw_stream_info.height;
    hw_stream_info->dis_enabled = scaler->hw_stream_info.dis_enabled;
    hw_stream_info->width_before_dis = scaler->hw_stream_info.width_before_dis;
    hw_stream_info->height_before_dis =
      scaler->hw_stream_info.height_before_dis;
    hw_stream_info->streaming_mode = scaler->hw_stream_info.streaming_mode;
    ISP_DBG("hw stream info width %d height %d fmt %d identity %x",
      scaler->hw_stream_info.width, scaler->hw_stream_info.height,
      scaler->hw_stream_info.fmt, scaler->hw_stream_info.identity);
  }
    break;

  case ISP_PRIVATE_FETCH_CROP_FACTOR:
    ret = scaler44_get_crop_factor(scaler,
      (isp_crop_factors_t *)private_event->data);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_get_crop_factor");
    }
    break;

  case ISP_PRIVATE_REQUEST_ZOOM_PARAMS:
    rc = scaler44_update_zoom_params(scaler,
        (isp_zoom_params_t *)private_event->data, identity);
    if (rc < 0) {
      ISP_ERR("failed: scaler44_update_zoom_params rc %d", rc);
      ret = FALSE;
    }
    break;

  default:
    ISP_DBG("failed: invalid private event %d", private_event->type);
    break;
  }
  return ret;
}

/** scaler44_open
 *    @version: hw version
 *
 *  scaler 44 module open and create func table
 *
 * Return: scaler module ops struct pointer
 **/
boolean scaler44_open(uint32_t version)
{
  scaler44_t *scaler = CAM_MALLOC(sizeof(scaler44_t));

  if (!scaler) {
    /* no memory */
    ISP_ERR(" no mem");
    return FALSE;
  }
  memset(scaler, 0, sizeof(scaler44_t));
  scaler->hw_stream_info.fmt = CAM_FORMAT_MAX;

  /* Initialize max scale factor */
  scaler->max_scale_factor = 1.0f;

  return TRUE;
}
