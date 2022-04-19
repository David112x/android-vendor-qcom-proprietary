/* scaler40.c
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_scaler40.h"
#include "scaler40_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

static boolean scaler40_get_module_enable_mask(scaler40_t *scaler,
  isp_module_enable_info_t *enable_bit_info);

/** scaler40_cmd_debug
 *    @cmd: bcc config cmd
 *    @index: pix path index
 *
 * This function dumps the scaler module register settings set
 * to hw
 *
 * Return: nothing
 **/
static void scaler40_cmd_debug(ISP_ScaleCfgCmdType* cmd, uint8_t index)
{
  ISP_HIGH(" scaler[%d](%d-Enc/%d-View/%d-Video/%d-Ideal)", index,
    ISP_HW_STREAM_ENCODER, ISP_HW_STREAM_VIEWFINDER,
    ISP_HW_STREAM_VIDEO, ISP_HW_STREAM_IDEAL_RAW);

  ISP_HIGH(" scaler[%d]",  index);
  /* Y config */
  ISP_HIGH(" scaler[%d] Y horiz_input = %d",
    index, cmd->Y_ScaleCfg.hIn);
  ISP_HIGH(" scaler[%d] Y horiz_output = %d",
    index, cmd->Y_ScaleCfg.hOut);
  ISP_HIGH(" scaler[%d] Y vert_input = %d",
    index, cmd->Y_ScaleCfg.vIn);
  ISP_HIGH(" scaler[%d] Y vert_output = %d",
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
     index, cmd->CbCr_ScaleCfg.ScaleCbCrInWidth);
  ISP_HIGH(" scaler[%d] CbCr Horiz_Skip_Count = %d",
     index, cmd->CbCr_ScaleCfg.HSkipCount);
  ISP_HIGH(" scaler[%d] CbCr Right_Pad_Enable = %d",
     index, cmd->CbCr_ScaleCfg.RightPadEnable);
  ISP_HIGH(" scaler[%d] CbCr CbCr_In_Height = %d",
     index, cmd->CbCr_ScaleCfg.ScaleCbCrInHeight);
  ISP_HIGH(" scaler[%d] CbCr Vert_Skip_Count = %d",
     index, cmd->CbCr_ScaleCfg.VSkipCount);
  ISP_HIGH(" scaler[%d] CbCr Bottom_Pad_Enable = %d",
     index, cmd->CbCr_ScaleCfg.BottomPadEnable);

} /* scaler40_cmd_debug */

/** scaler40_handle_set_chromatix_ptr
 *
 *  @scaler: scaler ctrl handle
 *  @event: set chromatix ptr event to be handled
 *
 *  Store chromatix ptr to be used later for interpolation
 *
 *  Return TRUE on success and FALS on failure
 **/
boolean scaler40_handle_set_chromatix_ptr(scaler40_t *scaler,
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

/** calculate_scaler40_factor
 *    @scale_factor_horiz: horizontal scale factor
 *    @scale_factor_vert:  vertical scale factor
 *    @hFactor: output
 *    @vFactor: output
 *
 * calculate scaler40 factor
 *
 * Return: Scaler putput value
 **/
static  int calculate_scaler40_factor(int scale_factor_horiz,
  int scale_factor_vert, int* hFactor, int* vFactor)
{
  int rc = 0;

  if (scale_factor_horiz < 1 || scale_factor_vert < 1) {
    ISP_DBG(" Output1 scale larger than camsensor FOV, set scale_factor=1");
    scale_factor_horiz = 1;
    scale_factor_vert = 1;
  }

  if (scale_factor_horiz >= 1 && scale_factor_horiz < 4) {
    *hFactor = 3;
  } else if (scale_factor_horiz >= 4 && scale_factor_horiz < 8) {
    *hFactor = 2;
  } else if (scale_factor_horiz >= 8 && scale_factor_horiz < 16) {
    *hFactor = 1;
  } else if (scale_factor_horiz >= 16 && scale_factor_horiz < 32) {
    *hFactor = 0;
  } else {
    ISP_DBG("scale_factor_horiz is greater than 32, which is more than "
      "the supported maximum scale factor.");
  }

  if (scale_factor_vert >= 1 && scale_factor_vert < 4) {
    *vFactor = 3;
  } else if (scale_factor_vert >= 4 && scale_factor_vert < 8) {
    *vFactor = 2;
  } else if (scale_factor_vert >= 8 && scale_factor_vert < 16) {
    *vFactor = 1;
  } else if (scale_factor_vert >= 16 && scale_factor_vert < 32) {
    *vFactor = 0;
  } else {
    ISP_DBG("scale_factor_vert is greater than 32, which is more than "
      "the supported maximum scale factor.");
  }

  return rc;
}/* calculate_scaler40_factor */

/** scaler40_compute_stripe_offset
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_compute_stripe_offset(scaler40_t *scaler,
  isp_stripe_request_t *stripe_request_info)
{
  int32_t       rc = 0;
  uint32_t      min_left_input = 0;
  uint32_t      mid_point = 0;
  uint32_t      sensor_output_width = 0;
  uint32_t      split_point = 0;
  uint32_t      stripe_offset = 0;
  uint32_t      min_overlap_half = 0;
  uint32_t      rolloff_grid_width = 0;
  uint32_t      rolloff_min_dist_from_grid = 0;
  uint32_t      distance_from_grid = 0;
  isp_stripe_limit_info_t *limits;

  if (!stripe_request_info || !scaler) {
    ISP_ERR("NULL pointer, ispif_out_info %p, scaler %p",
      stripe_request_info, scaler);
    return -1;
  }

  sensor_output_width =
    scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;

  /*decide split point*/
  mid_point = sensor_output_width / 2;
  split_point =
    MAX(stripe_request_info->stripe_limit.min_left_split, mid_point);

  rolloff_grid_width = stripe_request_info->stripe_limit.rolloff_grid_width;
  rolloff_min_dist_from_grid =
    stripe_request_info->stripe_limit.rolloff_min_dist_from_grid;

  /*decide initial stripe offset by split point and overlap*/
  stripe_offset = split_point - limits->min_overlap_left -
       (limits->min_left_split - limits->smallest_split_point);

  /* check if the init stripe offset go over max stripe offset*/
   if ((stripe_request_info->stripe_limit.max_stripe_offset != 0) &&
       (stripe_offset > stripe_request_info->stripe_limit.max_stripe_offset)) {
     stripe_offset = stripe_request_info->stripe_limit.max_stripe_offset;
   }

  if (rolloff_grid_width != 0) {
    /* Adjust the stripe offset for rolloff grid line - 36 requirement */
    distance_from_grid = (mid_point - stripe_offset) % rolloff_grid_width;
    if (distance_from_grid < rolloff_min_dist_from_grid) {
      stripe_offset = stripe_offset -
        (rolloff_min_dist_from_grid - distance_from_grid);
    }
  }

  ISP_DBG("sensor_output_width %d, mid_point %d, split_point %d, stripe_offset %d",
        sensor_output_width, mid_point,split_point, stripe_offset);

  stripe_offset = FLOOR2(stripe_offset);

  stripe_request_info->ispif_out_info.overlap = limits->min_left_split -
    stripe_offset + limits->min_overlap_right;

  /*ispif stripe offset need to consider ispif*/
  stripe_request_info->ispif_out_info.right_stripe_offset =
    stripe_offset + scaler->sensor_out_info.request_crop.first_pixel;

  ISP_DBG("<stripe_dbg> ispif_out_info: is_split = %d, overlap = %d,"\
    "right_stripe_offset = %d", stripe_request_info->ispif_out_info.is_split,
    stripe_request_info->ispif_out_info.overlap,
    stripe_request_info->ispif_out_info.right_stripe_offset);

  return 0;
}

/** scaler40_trigger40_enable
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_trigger_enable(scaler40_t *scaler,
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

/** scaler40_enable
 *    @scaler: scaler module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input struct size
 *
 *  scaler module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_enable(scaler40_t *scaler, isp_mod_set_enable_t *enable,
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

/** scaler40_calculate_zoom_scaling
 *    @pix_setting: pix path settings
 *    @entry_idx: pix path index
 *    @y_cfg: output config
 *    @crop_factor: crop factor
 *
 *  Based on input/output aspect ratio, calculate width, height
 *
 * Return: nothing
 **/
static void scaler40_calculate_zoom_scaling(scaler40_t *scaler,
  ISP_Y_ScaleCfgCmdType* y_cfg, uint32_t crop_factor)
{
  uint32_t hout_factor = y_cfg->hOut * crop_factor;
  uint32_t vout_factor = y_cfg->vOut * crop_factor;
  uint32_t tmp1 = y_cfg->hIn *
    scaler->hw_stream_info.height;
  uint32_t tmp2 = y_cfg->vIn *
    scaler->hw_stream_info.width;
  uint32_t hOut, vOut;

  ISP_HIGH("hout_factor %d vout_factor %d", hout_factor, vout_factor);
  ISP_HIGH("tmp1 %d tmp2 %d", tmp1, tmp2);
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
  ISP_HIGH("hOut %d vOut %d", hOut, vOut);
  ISP_HIGH("y_cfg->hOut %d y_cfg->vOut %d", y_cfg->hOut, y_cfg->vOut);
}

/** scaler40_calculate_phase
 *    @M:       output
 *    @N:       input
 *    @offset:  offset
 *    @interp_reso: actual input width
 *    @mn_init:
 *    @phase_init:
 *    @phase_mult:
 *    @y_scaler: luma scaler
 *
 * TODO
 *
 * Return: nothing
 **/
void scaler40_calculate_phase(uint32_t  M, uint32_t  N, uint32_t  offset,
  uint32_t *interp_reso, uint32_t *mn_init, uint32_t *phase_init,
  uint32_t *phase_mult, boolean   y_scaler)
{
  uint32_t ratio = N / M;
  *interp_reso = 3;
  if (ratio >= 16)     *interp_reso = 0;
  else if (ratio >= 8) *interp_reso = 1;
  else if (ratio >= 4) *interp_reso = 2;

  *mn_init = offset * M % N;
  *phase_init = (*mn_init << (13 + *interp_reso)) / M;
  if (y_scaler && ((*phase_init >> 13) != 0)) {
    *mn_init = (offset + 1) * M % N;
    *phase_init = (*mn_init << (13 + *interp_reso)) / M;
  }
  *phase_mult = (N << (13 + *interp_reso)) / M;
}

/** scaler40_check_hw_limit
 *    @entry_idx: pix path index
 *    @y_cfg:
 *    @in_aspect_ratio:
 *
 * Check of output dimensions are withing the limit, if not
 * clamp to maximum
 *
 * Return: nothing
 **/
static void scaler40_check_hw_limit(scaler40_t *scaler,
  ISP_Y_ScaleCfgCmdType* y_cfg, int in_aspect_ratio)
{
  uint32_t scaler_output_width = 0;
  float scale_factor = 0.0;
  uint32_t actual_camif_width = 0;
  uint32_t sensor_out_w = 0;
  uint32_t entry_idx = 0;
  int  down_scaling_factor = 0;

  entry_idx = scaler->entry_idx;

  scale_factor = (float)y_cfg->hOut / (float)y_cfg->hIn;
  sensor_out_w = scaler->sensor_out_info.request_crop.last_pixel -
         scaler->sensor_out_info.request_crop.first_pixel + 1;

  /*check dual vfe case, what is pixels output from scaler*/
  if (scaler->isp_out_info.is_split) {
    if (scaler->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
      actual_camif_width = scaler->isp_out_info.right_stripe_offset
        + scaler->ispif_out_info.overlap;
      scaler_output_width =  actual_camif_width * scale_factor;
    } else {
      actual_camif_width = sensor_out_w -
        scaler->isp_out_info.right_stripe_offset;
      scaler_output_width = actual_camif_width * scale_factor;
    }
  } else {
    actual_camif_width = sensor_out_w;
    scaler_output_width = y_cfg->hOut;
  }

  if (y_cfg->hOut > scaler->hw_limits.max_width) {
    ISP_ERR(" hw_limit hout = %d", scaler->hw_limits.max_width);
    y_cfg->hOut = scaler->hw_limits.max_width;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
  }

  /* this limitation not related to pixel numbers,
   but related to scale ratio only*/
  if ((y_cfg->hOut * ISP_SCALER40_MAX_SCALER_FACTOR) < y_cfg->hIn) {
    ISP_ERR(" hOut / hIn < 1/16");
    y_cfg->hOut = (y_cfg->hIn + ISP_SCALER40_LIMIT_SCALER_FACTOR -1) /
      ISP_SCALER40_LIMIT_SCALER_FACTOR;
    y_cfg->vOut = y_cfg->hOut * Q12 / in_aspect_ratio;
  }

  if ((y_cfg->vOut * ISP_SCALER40_MAX_SCALER_FACTOR) < y_cfg->vIn) {
    ISP_ERR(" vOut / vIn < 1/16");
    y_cfg->vOut = (y_cfg->vIn + ISP_SCALER40_LIMIT_SCALER_FACTOR -1)/
      ISP_SCALER40_LIMIT_SCALER_FACTOR;
    y_cfg->hOut = y_cfg->vOut * in_aspect_ratio / Q12;
  }

  down_scaling_factor = (y_cfg->hIn * 100) / y_cfg->hOut;
  ISP_HIGH("down_scaling_factor %d, SCALE_RATIO_LIMIT %d", down_scaling_factor,
    SCALE_RATIO_LIMIT);
  if (down_scaling_factor < SCALE_RATIO_LIMIT) {
    if (scaler->crop_factor_applied!= Q12) {
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
    ISP_HIGH("y_cfg->hOut %d y_cfg->vOut %d", y_cfg->hOut, y_cfg->vOut);
  }

  return;
}

/** scaler40_get_camif_sizes
 *    @scaler:       fov module control
 *    @camif_w:      width after camif
 *    @camif_h:      height after camif
 *
 * Extracts camif dimensions from sensor crop data.
 *
 * Return: None
 **/
static void scaler40_get_camif_sizes(scaler40_t *scaler, int32_t *camif_w,
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



/** scaler40_calculate_dis
 *    @scaler:       scaler module control
 *
 * Update dimensions of scaler depending on DIS
 *
 * Return: None
 **/
void scaler40_calculate_dis(scaler40_t *scaler)
{
  int32_t camif_w, camif_h;

  if (scaler->dis_enable) {
    scaler40_get_camif_sizes(scaler, &camif_w, &camif_h);
    if(scaler->hw_stream_info.width > camif_w)
      scaler->hw_stream_info.width = camif_w;
    if(scaler->hw_stream_info.height > camif_h)
      scaler->hw_stream_info.height = camif_h;
  }
  scaler->hw_stream_info.dis_enabled = scaler->dis_enable;
}

/** scaler40_config_entry
 *    @scaler:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_config_entry(scaler40_t *scaler)
{
  int rc = 0;
  isp_scaler40_entry_t *entry = &scaler->scalers;
  ISP_Y_ScaleCfgCmdType* y_cfg = &entry->reg_cmd.Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType* cbcr_cfg = &entry->reg_cmd.CbCr_ScaleCfg;

  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;
  float zoom_scaling = 0.0;
  int phase_increment;

  /* TODO: need to add cropping into scaler */
  if (scaler->hw_stream_info.width == 0) {
    entry->is_used = 0;
    entry->hw_update_pending = FALSE;
    return 0;
  }
  entry->is_used = 1;
  scaler->request_crop = 0;

  if (!scaler->sensor_out_info.request_crop.last_pixel ||
    !scaler->sensor_out_info.request_crop.last_line) {
    return 0;
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

  ISP_DBG("y_cfg->hIn %d", y_cfg->hIn);
  y_cfg->vIn =
    scaler->sensor_out_info.request_crop.last_line -
      scaler->sensor_out_info.request_crop.first_line + 1;
  ISP_DBG("y_cfg->vIn %d", y_cfg->vIn);

  input_ratio = y_cfg->hIn * Q12 / y_cfg->vIn;
  ISP_DBG("input_ratio %f", (float)input_ratio / Q12);
  output_ratio = scaler->hw_stream_info.width * Q12 /
    scaler->hw_stream_info.height;
  ISP_DBG("output_ratio %f", (float)output_ratio / Q12);
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
  ISP_DBG("y_cfg->hOut %d y_cfg->vOut %d", y_cfg->hOut, y_cfg->vOut);

  ISP_DBG("scaler->crop_factor_applied %d", scaler->crop_factor_applied);
  scaler40_calculate_zoom_scaling(scaler, y_cfg, scaler->crop_factor_applied);
  scaler40_check_hw_limit(scaler, y_cfg, input_ratio);

  /* make sure its even number so CbCr will match Y instead of rounding donw*/
  y_cfg->hOut = (y_cfg->hOut >> 1) << 1;
  y_cfg->vOut = (y_cfg->vOut >> 1) << 1;
  ISP_DBG("y_cfg->hOut %d y_cfg->vOut %d", y_cfg->hOut, y_cfg->vOut);

  if (output_ratio > input_ratio)
    entry->scaling_factor = (float)y_cfg->hIn / (float)y_cfg->hOut;
  else
    entry->scaling_factor = (float)y_cfg->vIn / (float)y_cfg->vOut;

  y_scale_factor_horiz = y_cfg->hIn / y_cfg->hOut;
  y_scale_factor_vert = y_cfg->vIn / y_cfg->vOut;
  ISP_DBG("entry->scaling_factor %f", entry->scaling_factor);
  ISP_DBG("y_scale_factor_horiz %d", y_scale_factor_horiz);
  ISP_DBG("y_scale_factor_vert %d", y_scale_factor_vert);

  rc = calculate_scaler40_factor(y_scale_factor_horiz,
    y_scale_factor_vert, &y_hFactor, &y_vFactor);
  ISP_DBG("y_hFactor %d y_vFactor %d", y_hFactor, y_vFactor);

  phase_increment = 13 + y_hFactor;
  y_cfg->horizInterResolution = y_hFactor;
  y_cfg->horizPhaseMult = (y_cfg->hIn << phase_increment) / y_cfg->hOut;
  y_cfg->horizMNInit = 0;
  y_cfg->horizPhaseInit = 0;

  phase_increment = 13 + y_vFactor;
  y_cfg->vertInterResolution = y_vFactor;
  y_cfg->vertPhaseMult = (y_cfg->vIn << phase_increment) / y_cfg->vOut;
  y_cfg->vertMNInit = 0;
  y_cfg->vertPhaseInit = 0;

  cbcr_cfg->hEnable  = TRUE;
  cbcr_cfg->vEnable = TRUE;
  cbcr_cfg->hIn = y_cfg->hIn;
  cbcr_cfg->vIn = y_cfg->vIn;

  switch (scaler->hw_stream_info.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    cbcr_cfg->vOut = y_cfg->vOut;
    is_cosited = 1;
  }
    break;

  default: {
    cbcr_cfg->vOut = y_cfg->vOut / 2;
    cbcr_cfg->hOut = y_cfg->hOut / 2;
    is_cosited = 0;
  }
    break;
  }

  if (scaler->hw_stream_info.need_uv_subsample) {
    cbcr_cfg->vOut /= 2;
    cbcr_cfg->hOut /= 2;
  }

  cbcr_scale_factor_horiz = cbcr_cfg->hIn / cbcr_cfg->hOut;
  cbcr_scale_factor_vert = cbcr_cfg->vIn / cbcr_cfg->vOut;

  rc = calculate_scaler40_factor(cbcr_scale_factor_horiz,
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

  cbcr_cfg->RightPadEnable = 0;
  cbcr_cfg->HSkipCount = 0;
  cbcr_cfg->ScaleCbCrInWidth = cbcr_cfg->hIn;
  cbcr_cfg->BottomPadEnable = 0;
  cbcr_cfg->VSkipCount = 0;
  cbcr_cfg->ScaleCbCrInHeight = cbcr_cfg->vIn;

  ISP_DBG("cds_dbg y %d %d %d %d cbcr %d %d %d %d", y_cfg->hIn,
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

/** fov_config_entry_split
 *    @scaler:       fov module control
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_config_entry_split(scaler40_t *scaler)
{
  int rc = 0;
  isp_scaler40_entry_t *entry = &scaler->scalers;
  ISP_Y_ScaleCfgCmdType* y_cfg = &entry->reg_cmd.Y_ScaleCfg;
  ISP_CbCr_ScaleCfgCmdType* cbcr_cfg = &entry->reg_cmd.CbCr_ScaleCfg;

  unsigned int cbcr_scale_factor_horiz, cbcr_scale_factor_vert;
  unsigned int y_scale_factor_horiz, y_scale_factor_vert;
  int cbcr_hFactor, cbcr_vFactor, y_hFactor, y_vFactor;
  int input_ratio, output_ratio;
  int is_cosited = 0;
  float zoom_scaling = 0.0;
  uint32_t interp_reso, mn_init, phase_init, phase_mult;

  isp_out_info_t* isp_out_info = &scaler->isp_out_info;
  uint32_t offset = (isp_out_info->stripe_id == ISP_STRIPE_RIGHT) ?
    isp_out_info->right_stripe_offset : 0;

  /* TODO: need to add cropping into scaler */
  if (scaler->hw_stream_info.width == 0) {
    entry->is_used = 0;
    entry->hw_update_pending = FALSE;
    ISP_DBG(" Scaler entry %d not used",  scaler->entry_idx);
    return 0;
  }

  entry->is_used = 1;
  scaler->request_crop = 0;
  ISP_DBG(" scaler %d, start Y config ",  scaler->entry_idx);

  y_cfg->hEnable = TRUE;
  y_cfg->vEnable = TRUE;
  cbcr_cfg->hEnable = TRUE;
  cbcr_cfg->vEnable = TRUE;

  /* dual vfe scaler need use sensor output window to calculate config*/
  y_cfg->hIn =scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;
  y_cfg->vIn = scaler->sensor_out_info.request_crop.last_line -
    scaler->sensor_out_info.request_crop.first_line + 1;

  if (!scaler->is_bayer_sensor)
    y_cfg->hIn /= 2;

  /* from now on, hIn and vIn are adjusted, it's imaginary as if
     FOV crop is performed before to alter the input to the scaler */
  /* adjust hIn, vIn based on zoom level (crop_factor) */
  y_cfg->hIn    = y_cfg->hIn * Q12 / /* scaler->crop_factor_applied */ 1;
  y_cfg->vIn    = y_cfg->vIn * Q12 / /* scaler->crop_factor_applied */ 1;
  y_cfg->hOut   = scaler->hw_stream_info.width;
  y_cfg->vOut   = scaler->hw_stream_info.height;

  /* adjust to match input to output aspect ratio */
  /* This might not seem easy to understand why we set horizontal
      and vertical input and output to be the same initially.
      But due to luma scaler implementation where right stripe needs to start
      at roll-over points, the crop factor is chosen very carefully earlier so
      that the resulting scaling ratio ensures right stripe starts at a
      roll-over point. The crop factor was chosen based on which dimension is
      the non-cropping side, therefore, the non-cropping side M and N should be
      used precisely on both dimensions. Please do not change the value of hIn
      hOut vIn and vOut without understanding why it is set this way. */
  if (y_cfg->hOut * y_cfg->vIn < y_cfg->vOut * y_cfg->hIn) {
    y_cfg->hIn  = y_cfg->vIn;
    y_cfg->hOut = y_cfg->vOut;
  }
  else {
    y_cfg->vIn  = y_cfg->hIn;
    y_cfg->vOut = y_cfg->hOut;
  }

  /* maximum zoom by ISP reached */
  if (y_cfg->hIn < y_cfg->hOut) {
    y_cfg->hIn = y_cfg->hOut;
    y_cfg->vIn = y_cfg->vOut;
  }

  /* derive cbcr information based on y */
  cbcr_cfg->hIn  = y_cfg->hIn;
  cbcr_cfg->vIn  = y_cfg->vIn;

  switch (scaler->hw_stream_info.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    cbcr_cfg->hOut = y_cfg->hOut >>1;
    cbcr_cfg->vOut = y_cfg->vOut;
    is_cosited = 1;
  }
    break;

  default: {
    cbcr_cfg->vOut = y_cfg->vOut >> 1;
    cbcr_cfg->hOut = y_cfg->hOut >> 1;
    is_cosited = 0;
  }
    break;
  }

  if (scaler->hw_stream_info.need_uv_subsample) {
    cbcr_cfg->vOut /= 2;
    cbcr_cfg->hOut /= 2;
  }

  entry->scaling_factor = (float) y_cfg->hIn/ (float)y_cfg->hOut;

  /* calculate phase related values */
  scaler40_calculate_phase(y_cfg->hOut, y_cfg->hIn, offset,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 1);
  y_cfg->horizInterResolution = interp_reso;
  y_cfg->horizMNInit          = mn_init;
  y_cfg->horizPhaseInit       = phase_init & 0xffff;


  y_cfg->horizPhaseMult       = phase_mult;
  scaler40_calculate_phase(y_cfg->vOut, y_cfg->vIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 1);
  y_cfg->vertInterResolution  = interp_reso;
  y_cfg->vertMNInit           = mn_init;
  y_cfg->vertPhaseInit        = phase_init & 0xffff;
  y_cfg->vertPhaseMult        = phase_mult;

  scaler40_calculate_phase(cbcr_cfg->hOut, cbcr_cfg->hIn, offset,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 0);
  cbcr_cfg->horizInterResolution = interp_reso;
  cbcr_cfg->horizMNInit          = mn_init;
  cbcr_cfg->horizPhaseInit       = phase_init;
  cbcr_cfg->horizPhaseMult       = phase_mult;

  scaler40_calculate_phase(cbcr_cfg->vOut, cbcr_cfg->vIn, 0,
    &interp_reso, &mn_init, &phase_init, &phase_mult, 0);
  cbcr_cfg->vertInterResolution  = interp_reso;
  cbcr_cfg->vertMNInit           = mn_init;
  cbcr_cfg->vertPhaseInit        = phase_init;
  cbcr_cfg->vertPhaseMult        = phase_mult;

  cbcr_cfg->ScaleCbCrInWidth  = cbcr_cfg->hIn;
  cbcr_cfg->ScaleCbCrInHeight = cbcr_cfg->vIn;
  cbcr_cfg->HSkipCount        = 0;
  cbcr_cfg->VSkipCount        = 0;
  cbcr_cfg->RightPadEnable    = 0;
  cbcr_cfg->BottomPadEnable   = 0;

  entry->hw_update_pending = TRUE;
  return rc;
}

/** scaler40_update_zoom_params
 *    @scaler: Scaler module struct data
 *    @upd_zoom_params: zoom params to update
 *    @identity: stream identity
 *
 * update zoom params from scaler data;
 *
 * Return:   0 - success
 *          -1 - NULL parameter supplied
 **/
static int scaler40_update_zoom_params(scaler40_t *scaler,
  isp_zoom_params_t *upd_zoom_params, uint32_t identity)
{
  isp_zoom_params_t         *zoom_params = NULL;
  uint32_t                   i = 0, j = 0;

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
      zoom_params->scaler_output.width = scaler->scalers.reg_cmd.Y_ScaleCfg.hOut;
      zoom_params->scaler_output.height = scaler->scalers.reg_cmd.Y_ScaleCfg.vOut;
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
          ISP_DBG("i %d ide %x scale %f out %d %d", i, identity,
            zoom_params->scaling_ratio, zoom_params->scaler_output.width,
            zoom_params->scaler_output.height);
          break;
        }
      }
    }
  }

  return 0;
}

/** scaler40_do_hw_update
 * @scaler: scaler module struct data
 * @hw_update_list: linked list ent from pipeline layer which
 *      scaler40 will populate with register address and data info
 * @identity: event identity
 *
 * update scaler module register to kernel
 *
 * Return: nothing
 **/
static boolean scaler40_do_hw_update(scaler40_t *scaler,
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

  if (!scaler || !output) {
    ISP_ERR("failed: scaler %p output %p", scaler, output);
    return FALSE;
  }

  ISP_DBG("HW_update, scaler[%d] = %d\n",
       scaler->entry_idx, scaler->scalers.hw_update_pending);

  if ((scaler->update_enable_bit == TRUE) && output->stats_params) {
    scaler40_get_module_enable_mask(scaler,
      &output->stats_params->module_enable_info);
    scaler->update_enable_bit = FALSE;
  }

  if (scaler->scalers.hw_update_pending == TRUE) {
    scaler40_cmd_debug(&scaler->scalers.reg_cmd, scaler->entry_idx);

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
      CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
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
    cfg_cmd->num_cfg = 1;

    reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd->cmd_type = VFE_WRITE;
    reg_cfg_cmd->u.rw_info.reg_offset = scaler->reg_off;
    reg_cfg_cmd->u.rw_info.len = scaler->reg_len * sizeof(uint32_t);

    ISP_HIGH("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
      reg_cfg_cmd[0].u.rw_info.len);
    data = (uint32_t *)cfg_cmd->cfg_data;
    for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
      ISP_HIGH("data[%d] %x", len, data[len]);
    }
    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = scaler40_util_append_cfg(&output->hw_update_list, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler40_util_append_cfg");
      free(reg_cfg_cmd);
      free(hw_update);
      free(reg_cmd);
      return FALSE;
    }
    scaler->scalers.hw_update_pending = FALSE;
  }

  rc = scaler40_update_zoom_params(scaler, output->stats_params->zoom_params,
    identity);
  if (rc < 0) {
    ISP_ERR("failed: scaler40_update_zoom_params");
    return FALSE;
  }
  if (output->frame_meta) {
      output->frame_meta->zoom_value  = scaler->zoom_value;
  }

  return ret;
}

/** scaler40_config
 *    @scaler: fov module struct data
 *    @pix_setting: hw pixel settings
 *    @size: input params struct size
 *
 * TODO
 *
 * Return: 0 - sucess and negative value - failure
 **/
boolean scaler40_config(scaler40_t *scaler)
{
  int i, rc = 0;
  ISP_HIGH("E: scaler: %p is_split %d", scaler, scaler->isp_out_info.is_split);
  if (!scaler) {
    ISP_ERR("failed: %p", scaler);
    return FALSE;
  }

  if (!scaler->enable) {
    /* not enabled no op */
    return TRUE;
  }
  /*TODO: fill in is_split */
  if (!scaler->ispif_out_info.is_split)
    rc = scaler40_config_entry(scaler);
  else
    rc = scaler40_config_entry_split(scaler);

  if (rc < 0) {
    /* scaler entry configuration error */
    ISP_ERR(" scaler_config_entry error, idx = %d, rc = %d",
                scaler->entry_idx, rc);
    return FALSE;
  }
  return TRUE;
}

/** scaler40_set_zoom_ratio
 *
 *    @scaler: scaler 40 pointer
 *  TODO
 *
 * Return: nothing
 **/
static int scaler40_set_zoom_ratio(scaler40_t *scaler)
{
  return scaler40_config(scaler);

}

/** scaler40_trigger_update
 *    @scaler: scaler module control struct
 *
 *  scaler module modify reg settings as per new input params
 *  and trigger hw update
 *
 * Return: 0 - success and negative value - failure
 **/
static int scaler40_trigger_update(scaler40_t *scaler)
{
  int rc = 0;

  if (!scaler->trigger_enable || !scaler->enable) {
    ISP_DBG(" SCALER trigger update not enabled" );
    return 0;
  }
  rc = scaler40_config(scaler);
  scaler->hw_update_pending = TRUE;
  return rc;

}

boolean scaler40_parm_zoom(scaler40_t *scaler, cam_zoom_info_t *zoom_val)
{
  boolean ret = TRUE;
  int i = 0;

  if (!scaler || !zoom_val) {
    ISP_ERR("failed: %p %p", scaler, zoom_val);
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

  scaler40_config(scaler);

  scaler->hw_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return ret;
}

/** scaler40_destroy
 *    @mod_ctrl: scaler module control strcut
 *
 *  Close scaler module
 *
 * Return: 0 always
 **/
int scaler40_destroy (void *mod_ctrl)
{
  scaler40_t *scaler = mod_ctrl;

  memset(scaler,  0,  sizeof(scaler40_t));
  free(scaler);
  return 0;
}

/** scaler40_reset
 *      @scaler: scaler module struct data
 *
 * Scaler module disable hw updates,release reg settings and
 * structs
 *
 * Return: nothing
 **/
void scaler40_reset(scaler40_t *scaler)
{
  /* Enable module and trigger enable by default */
  scaler->trigger_enable = 1;
  scaler->enable = 1;
  scaler->applied_crop_factor = 0;
  scaler->scalers.hw_update_pending = FALSE;
  memset(&scaler->scalers.reg_cmd, 0, sizeof(scaler->scalers.reg_cmd));
  memset(&scaler->isp_out_info,0, sizeof(scaler->isp_out_info));
  memset(&scaler->ispif_out_info,0, sizeof(scaler->ispif_out_info));
  scaler->scalers.is_used = 0;
}

/** scaler40_init
 *    @mod_ctrl: scaler module control strcut
 *
 *  scaler module data struct initialization
 *
 * Return: 0 always
 **/
boolean scaler40_init(void *mod_ctrl)
{
  scaler40_t *scaler = mod_ctrl;

  scaler40_reset(scaler);
  scaler->crop_factor_applied = Q12;
  scaler->crop_factor_calculate_crop = Q12;
  return TRUE;
}

/** scaler40_get_module_enable_mask:
 *
 *  @scaler: fov crop handle
 *  @module_enable: handle to mask module enable bit
 *
 *  Enable / disable this module based on module enable flag
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler40_get_module_enable_mask(scaler40_t *scaler,
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

/** scaler40_get_module_enable_mask:
 *
 *  @scaler: fov crop handle
 *  @crop_factor: handle to get crop factor
 *
 *  Return crop factor
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler40_get_crop_factor(scaler40_t *scaler,
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

static uint32_t scaler40_calculate_right_split_width(uint32_t M, uint32_t N,
  uint32_t mn_init, uint32_t h_skip_cnt, uint32_t actual_input)
{
    // mn_init and h_skip_cnt are from ones used in scaler
    return (mn_init + (actual_input - h_skip_cnt) * M) / N;
}

/** scaler40_handle_isp_private_event:
 *
 *  @scaler: scaler private
 *  @data: module event data
 *  @identity: identity of event
 *
 *  Handle ISP private event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler40_handle_isp_private_event(scaler40_t *scaler,
  void *data, uint32_t identity)
{
  boolean              ret = TRUE;
  int                  rc = 0;
  isp_private_event_t *private_event = (isp_private_event_t *)data;

  if (!scaler || !data) {
    ISP_ERR("failed: scaler %p data %p", scaler, data);
    return FALSE;
  }

  switch (private_event->type) {
  case ISP_PRIVATE_GET_VFE_DIAG_INFO_USER:
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

  case ISP_PRIVATE_SET_MOD_ENABLE:
    rc = scaler40_enable(scaler,
      (isp_mod_set_enable_t *)private_event->data, private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: scaler40_enable rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_STRIPE_INFO: {
    ispif_out_info_t *ispif_stripe_info =
      (ispif_out_info_t *)private_event->data;
    scaler->ispif_out_info = *ispif_stripe_info;
   }
   break;

  case ISP_PRIVATE_REQUEST_STRIPE_OFFSET:
    rc = scaler40_compute_stripe_offset(scaler, private_event->data);
    if (rc < 0) {
      ISP_ERR("failed: scaler40_config rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_TRIGGER_ENABLE:
    rc = scaler40_trigger_enable(scaler, private_event->data,
      private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: scaler40_config rc %d", rc);
      ret = FALSE;
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

  case ISP_PRIVATE_ACTION_RESET:
    scaler40_reset(scaler);
    break;

  case ISP_PRIVATE_SET_TRIGGER_UPDATE:
    PTHREAD_MUTEX_LOCK(&scaler->mutex);
    ret = scaler40_do_hw_update(scaler,
      (isp_sub_module_output_t *)private_event->data, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler40_config ret %d", ret);
    }
    PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
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
      ISP_HIGH(" scaler_output[%d]: Width %d, Height %d, scalefactor %f",
        scaler->entry_idx, scaler_output->width, scaler_output->height,
        scaler_output->scaling_factor);

      if (scaler->ispif_out_info.is_split == TRUE) {
        Y_ScaleCfg = &scaler->scalers.reg_cmd.Y_ScaleCfg;
        CbCr_ScaleCfg = &scaler->scalers.reg_cmd.CbCr_ScaleCfg;

        scaler_input_width = scaler->sensor_out_info.request_crop.last_pixel -
          scaler->ispif_out_info.right_stripe_offset + 1;
        ISP_DBG("DUALVFE scaler_input_width %d right_stripe_offset %d",
          scaler_input_width, scaler->ispif_out_info.right_stripe_offset);

        scaler_output->right_split_y_width = scaler40_calculate_right_split_width(
          Y_ScaleCfg->hOut, Y_ScaleCfg->hIn, Y_ScaleCfg->horizMNInit,
          0, scaler_input_width);

        scaler_output->right_split_cbcr_width =
          scaler40_calculate_right_split_width(CbCr_ScaleCfg->hOut,
          CbCr_ScaleCfg->hIn, CbCr_ScaleCfg->horizMNInit,
          0, scaler_input_width);
        ISP_DBG("DUALVFE scaler output y %d cbcr %d",
          scaler_output->right_split_y_width,
          scaler_output->right_split_cbcr_width);
      }
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
    ISP_HIGH("hw stream info width %d height %d fmt %d identity %x",
      scaler->hw_stream_info.width, scaler->hw_stream_info.height,
      scaler->hw_stream_info.fmt, scaler->hw_stream_info.identity);
  }
    break;

  case ISP_PRIVATE_FETCH_CROP_FACTOR:
    ret = scaler40_get_crop_factor(scaler, (isp_crop_factors_t *)private_event->data);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler40_get_crop_factor");
    }
    break;

  case ISP_PRIVATE_REQUEST_ZOOM_PARAMS:
    rc = scaler40_update_zoom_params(scaler,
        (isp_zoom_params_t *)private_event->data, identity);
    if (rc < 0) {
      ISP_ERR("failed: scaler40_update_zoom_params rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_REQUEST_CDS_CAP: {
      uint32_t *cds_cap = (uint32_t *)private_event->data;
      *cds_cap = scaler->cds_cap[scaler->entry_idx];
  }
    break;

  default:
    ISP_ERR("failed: invalid private event %d", private_event->type);
    break;
  }
  return ret;
}

/** scaler40_open
 *    @version: hw version
 *
 *  scaler 40 module open and create func table
 *
 * Return: scaler module ops struct pointer
 **/
boolean scaler40_open(uint32_t version)
{
  scaler40_t *scaler = CAM_MALLOC(sizeof(scaler40_t));

  if (!scaler) {
    /* no memory */
    ISP_ERR(" no mem");
    return FALSE;
  }
  memset(scaler, 0, sizeof(scaler40_t));
  scaler->hw_stream_info.fmt = CAM_FORMAT_MAX;
  return TRUE;
}
