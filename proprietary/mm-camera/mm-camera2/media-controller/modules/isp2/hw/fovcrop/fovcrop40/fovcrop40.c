/* fovcrop40.c
 *
 * Copyright (c) 2012-2014, 2016-2017,2019 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_fovcrop40.h"
#include "port_fovcrop40.h"
#include "fovcrop40_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_FOVCROP, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_FOVCROP, fmt, ##args)

/*#define USE_FOVCROP_FOR_SMALL_SCALE_FACTOR*/
#define MIN_DOWNSCALE_FACTOR 1.03125
#define PREVIEW_FORMAT CAMERA_YUV_420_NV12

#define CHECK_ASPECT_RATIO(x) ((1.3 <= x) && (x <= 1.34))
#define CHECK_DEFAULT_CROP_FACTOR(x) (x == (1<<12))
#define CHECK_DOWNSCALE_FACTOR(x) ((1.0 < x) && (x < MIN_DOWNSCALE_FACTOR))

static boolean fovcrop40_get_module_enable_mask(fovcrop40_t *fovcrop,
  isp_module_enable_info_t *enable_bit_info);

typedef struct {
  uint32_t width;
  uint32_t height;
} dim_t;

typedef struct {
  dim_t y;
  dim_t cbcr;
} img_desc_t;

typedef struct {
  uint32_t left;
  uint32_t right;
  uint32_t top;
  uint32_t bottom;
} crop_desc_t;

/** fovcrop40_cmd_debug
 *    @cmd: bcc config cmd
 *    @index: pix path index
 *
 * This function dumps the bcc module register settings set to
 * hw
 *
 * Return: nothing
 **/
static void fovcrop40_cmd_debug(ISP_FOV_CropConfigCmdType cmd ,int index)
{
  ISP_DBG(" FOV[%d](%d-Enc/%d-View/%d-Video/%d-Ideal)", index,
    ISP_HW_STREAM_ENCODER, ISP_HW_STREAM_VIEWFINDER,
    ISP_HW_STREAM_VIDEO, ISP_HW_STREAM_IDEAL_RAW);

  ISP_DBG(" FOV_CROP[%d] Y firstPixel %d",
     index, cmd.y_crop_cfg.firstPixel );
  ISP_DBG(" FOV_CROP[%d] Y lastPixel %d",
     index, cmd.y_crop_cfg.lastPixel );
  ISP_DBG(" FOV_CROP[%d] Y firstLine %d",
     index, cmd.y_crop_cfg.firstLine );
  ISP_DBG(" FOV_CROP[%d] Y lastLine %d",
     index, cmd.y_crop_cfg.lastLine );
  ISP_DBG(" FOV_CROP[%d] CbCr firstPixel %d",
     index, cmd.cbcr_crop_cfg.firstPixel );
  ISP_DBG(" FOV_CROP[%d] CbCr lastPixel %d",
     index, cmd.cbcr_crop_cfg.lastPixel );
  ISP_DBG(" FOV_CROP[%d] CbCr firstLine %d",
     index, cmd.cbcr_crop_cfg.firstLine );
  ISP_DBG(" FOV_CROP[%d] CbCr lastLine %d",
     index, cmd.cbcr_crop_cfg.lastLine );
} /* fovcrop40_cmd_debug */

/** fovcrop40_validate_config
 *    @fovcrop:       fov module control
 *
 * validate config
 *
 * Return: TRUE on success and FALSE on failure
 **/
static boolean fovcrop40_validate_config(fovcrop40_entry_t *entry)
{
  /* validate Y plane config to have more info and prevent violation
     also can throw useful error to help preven no buf done issue*/
  if(entry->reg_cmd.y_crop_cfg.firstPixel >
     entry->reg_cmd.y_crop_cfg.lastPixel) {
    ISP_ERR("invalid Y width, first pix %d, last pix %d",
      entry->reg_cmd.y_crop_cfg.firstPixel,
      entry->reg_cmd.y_crop_cfg.lastPixel);
    return FALSE;
  }

  if(entry->reg_cmd.y_crop_cfg.firstLine >
     entry->reg_cmd.y_crop_cfg.lastLine) {
    ISP_ERR("invalid Y Height, first line %d, last line %d",
      entry->reg_cmd.y_crop_cfg.firstLine,
      entry->reg_cmd.y_crop_cfg.lastLine);
    return FALSE;
  }

  if(entry->reg_cmd.cbcr_crop_cfg.firstPixel >
     entry->reg_cmd.cbcr_crop_cfg.lastPixel) {
    ISP_ERR("invalid CbCr width, first pix %d, last pix %d",
      entry->reg_cmd.cbcr_crop_cfg.firstPixel,
      entry->reg_cmd.cbcr_crop_cfg.lastPixel);
    return FALSE;
  }

  if(entry->reg_cmd.cbcr_crop_cfg.firstLine >
     entry->reg_cmd.cbcr_crop_cfg.lastLine) {
    ISP_ERR("invalid CbCr Height, first line %d, last line %d",
      entry->reg_cmd.cbcr_crop_cfg.firstLine,
      entry->reg_cmd.cbcr_crop_cfg.lastLine);
    return FALSE;
  }

  return TRUE;
}

/** fovcrop40_split_config
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: TRUE on success and FALSE on failure
 **/
static boolean fovcrop40_split_config(fovcrop40_t *fovcrop,
  float cbcr_scale)
{
  fovcrop40_entry_t *entry = NULL;
  int y_left_crop = 0, y_right_crop = 0, y_total_crop = 0;
  int cbcr_left_crop = 0, cbcr_right_crop = 0, cbcr_total_crop = 0;

  entry = &fovcrop->fov_entry;

  /*Y Cfg*/
  y_total_crop = fovcrop->scaler_output.width - fovcrop->hw_stream_info.width;
  y_left_crop = y_total_crop / 2;
  y_right_crop = y_total_crop - y_left_crop;

  if (fovcrop->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
    entry->reg_cmd.y_crop_cfg.firstPixel = y_left_crop;
    entry->reg_cmd.y_crop_cfg.lastPixel =
      entry->reg_cmd.y_crop_cfg.firstPixel +
      fovcrop->isp_out_info.left_output_width - 1;
  } else if (fovcrop->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    entry->reg_cmd.y_crop_cfg.lastPixel =
      fovcrop->scaler_output.right_split_y_width - y_right_crop - 1;
    entry->reg_cmd.y_crop_cfg.firstPixel =
      entry->reg_cmd.y_crop_cfg.lastPixel -
      fovcrop->isp_out_info.right_output_width + 1;
  }

  /*CbCr*/
  cbcr_total_crop = (fovcrop->scaler_output.width -
    fovcrop->hw_stream_info.width) / cbcr_scale;
  cbcr_left_crop = cbcr_total_crop / 2;
  cbcr_right_crop = cbcr_total_crop - cbcr_left_crop;

  if (fovcrop->isp_out_info.stripe_id == ISP_STRIPE_LEFT) {
    entry->reg_cmd.cbcr_crop_cfg.firstPixel = cbcr_left_crop;
    entry->reg_cmd.cbcr_crop_cfg.lastPixel =
      entry->reg_cmd.cbcr_crop_cfg.firstPixel +
      (fovcrop->isp_out_info.left_output_width / cbcr_scale) - 1;
  } else if (fovcrop->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    entry->reg_cmd.cbcr_crop_cfg.lastPixel =
      fovcrop->scaler_output.right_split_cbcr_width - cbcr_right_crop - 1;
    entry->reg_cmd.cbcr_crop_cfg.firstPixel =
      entry->reg_cmd.cbcr_crop_cfg.lastPixel -
      (fovcrop->isp_out_info.right_output_width / cbcr_scale) + 1;
  }

  /*input to split config*/
  ISP_DBG("======= SPLIT CONFIG: stripe id %d(0- LEFT, 1- RIGHT)==========",
    fovcrop->isp_out_info.stripe_id);
  ISP_DBG("split_input: hw_stream_info.width %d idx %d",
    fovcrop->hw_stream_info.width, fovcrop->entry_idx );
  ISP_DBG("split_input: SCALER: total out %d right_out_Y %d, right_out_CBCR %d",
    fovcrop->scaler_output.width,
    fovcrop->scaler_output.right_split_y_width,
    fovcrop->scaler_output.right_split_cbcr_width);
  ISP_DBG("split_input: FOVCROP: left out %d",
    fovcrop->isp_out_info.left_output_width);

  /*output for split config*/
  ISP_DBG(" split_output: Y: total_crop %d, lef_cropt %d righ_cropt %d",
    y_total_crop, y_left_crop, y_right_crop);
  ISP_DBG(" split_output: CbCr: total_crop %d, left_crop %d right_crop %d",
    cbcr_total_crop, cbcr_left_crop, cbcr_right_crop);

  return TRUE;
}

/** fovcrop40_compute_min_split_width_left:
 *
 *  @input_width: input width
 *  @outptu_width: output width
 *
 *  Calculage min left input width based on input and output
 *  width
 *
 *  Return 0 on success and negative error on failure
 **/
static uint32_t fovcrop40_compute_min_split_width_left(
  uint32_t input_width, uint32_t output_width, boolean cds)
{
  uint32_t split_pt = 0;
  /* the output to AXI need to be allign
     Left output width in dual VFE case needs to be padded for buffer alignment.
     Normally the padding is 16, with extra chroma subsampling requirement,
     it becomes 32 */
  if (cds) {
    split_pt = PAD_TO_SIZE(output_width / 4, 16) * 2 * input_width / output_width;
  } else {
    split_pt = PAD_TO_SIZE(output_width / 2, 16) * input_width / output_width;
  }
  return split_pt;
}

/** fovcrop40_update_stripe_limit
 *    @fovcrop: fovcrop module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  scaler module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_update_stripe_limit(fovcrop40_t *fovcrop,
  isp_stripe_limit_info_t *stripe_limit_info)
{
  int32_t       rc = 0;
  uint32_t      sensor_output_width = 0;

  if (!stripe_limit_info || !fovcrop ||
    (0 == fovcrop->hw_stream_info.width)) {
    ISP_ERR("Invalid params, stripe_limit_info %p, fovcrop %p",
      stripe_limit_info, fovcrop);
    return -1;
  }

  sensor_output_width =
    fovcrop->sensor_out_info.request_crop.last_pixel -
    fovcrop->sensor_out_info.request_crop.first_pixel + 1;

  /*find out what is minimum split left,
    based on stream output + CDS + allignment*/
  fovcrop->min_left_input = EVEN_CEIL(fovcrop40_compute_min_split_width_left(
    sensor_output_width, fovcrop->hw_stream_info.width, TRUE));
  if (fovcrop->min_left_input > stripe_limit_info->min_left_split) {
    stripe_limit_info->min_left_split = fovcrop->min_left_input;
  }

  if (!stripe_limit_info->smallest_split_point ||
    (fovcrop->min_left_input < stripe_limit_info->smallest_split_point)) {
    stripe_limit_info->smallest_split_point = fovcrop->min_left_input;
  }

  return 0;
}

/** fovcrop40_handle_set_chromatix_ptr
 *
 *  @fovcrop: fovcrop ctrl handle
 *
 *  @event: set chromatix ptr event to be handled
 *
 *  Store chromatix ptr to be used later for interpolation
 *
 *  Return TRUE on success and FALS on failure
 **/
boolean fovcrop40_handle_set_chromatix_ptr(fovcrop40_t *fovcrop,
  mct_event_t *event)
{
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!fovcrop || !event) {
    ISP_ERR("failed: %p %p", fovcrop, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&fovcrop->mutex);
  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    goto ERROR;
  }

  fovcrop->chromatix_ptrs = *chromatix_ptrs;
  PTHREAD_MUTEX_UNLOCK(&fovcrop->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&fovcrop->mutex);
  return FALSE;
}

/** fovcrop40_calculate_scaler_output
 *    @M:       output
 *    @N:       input
 *    @offset:  offset
 *    @actual_input_width: actual input width
 *    @y_scaler: luma scaler
 *
 * bcc module disable,release reg settings and strcuts
 *
 * Return: Scaler putput value
 **/
uint32_t fovcrop40_calculate_scaler_output(uint32_t  M, uint32_t  N,
  uint32_t  offset, uint32_t  actual_input_width, boolean   y_scaler)
{
  uint32_t mn_init, phase_init;
  uint32_t ratio = N / M;
  uint32_t interp_reso = 3;
  if (ratio >= 16)     interp_reso = 0;
  else if (ratio >= 8) interp_reso = 1;
  else if (ratio >= 4) interp_reso = 2;

  mn_init = offset * M % N;
  phase_init = (mn_init << (13 + interp_reso)) / M;
  if (y_scaler && ((phase_init >> 13) != 0)) {
    mn_init = (offset + 1) * M % N;
    actual_input_width--;
  }
  return (mn_init + actual_input_width * M) / N;
}

/** fovcrop40_config_crop_window
 *    @fovcrop:  fov module control
 *
 * Mark the FOV mapping on the scalar window in case of VFE
 * zoom. In case of CPP zoom, update the co-ordinates to crop
 * and upscale
 *
 * Return: TRUE on success and FALSE on failure
 **/
boolean fovcrop40_config_crop_window(fovcrop40_t *fovcrop)
{
  fovcrop40_entry_t *entry = &fovcrop->fov_entry;
  cam_format_t format = fovcrop->hw_stream_info.fmt;
  float scale_w;
  float scale_h;
  uint32_t camif_w, camif_h,output_width, output_height;
  uint32_t x,y,left,top,crop_w,crop_h, temp, split_point;
  uint32_t out_w,out_h,scalar_w,scalar_h;
  uint32_t fov_x,fov_y,fov_w,fov_h;
  uint32_t left_crop = 0, right_crop = 0, total_crop = 0;
  boolean ret = TRUE;

  if (fovcrop->hw_stream_info.width == 0) {
    entry->is_used = 0;
    ISP_HIGH("FOV entry not used");
    return FALSE;
  }

  entry->is_used = 1;

  /*first pixel , first line , last pix, last line*/
  if (fovcrop->is_bayer_sensor)
    camif_w = fovcrop->sensor_out_info.request_crop.last_pixel -
      fovcrop->sensor_out_info.request_crop.first_pixel + 1;
  else
    camif_w = (fovcrop->sensor_out_info.request_crop.last_pixel -
       fovcrop->sensor_out_info.request_crop.first_pixel + 1) >> 1;

  camif_h = fovcrop->sensor_out_info.request_crop.last_line -
      fovcrop->sensor_out_info.request_crop.first_line + 1;

  out_w = fovcrop->hw_stream_info.width;
  out_h = fovcrop->hw_stream_info.height;

  scalar_w = fovcrop->scaler_output.width;
  scalar_h = fovcrop->scaler_output.height;

  ISP_DBG("scaler: W*H %d * %d, Output W*H %d*%d, CAMIF W*H %d*%d\n",
    scalar_w,scalar_h,out_w,out_h,camif_w,camif_h);

  /* Crop window on Scalar Output */
  left = (fovcrop->scaler_output.modified_crop_window.left *
       scalar_w) / camif_w;
  top = (fovcrop->scaler_output.modified_crop_window.top *
       scalar_h) / camif_h;

  crop_w = fovcrop->scaler_output.modified_crop_window.width *
       ((float)scalar_w/camif_w);

  crop_h = fovcrop->scaler_output.modified_crop_window.height *
       ((float)scalar_h/camif_h);

  split_point = fovcrop->ispif_out_info.split_point *
       ((float)scalar_w/camif_w);

  /* check if crop area is part of only one of ISPs */
  if (fovcrop->ispif_out_info.is_split &&
      left + crop_w < split_point && left > split_point) {
    ISP_ERR("FIX ME: Can not support non centric zoom in dual ISP");
    return TRUE;
  }

  x = FLOOR2((scalar_w - out_w)>>1);
  y = FLOOR2((scalar_h - out_h)>>1);

  ISP_DBG("VFE path %d FOV window [%d,%d,%d,%d], Scalar %d * %d\n",
    fovcrop->entry_idx, left, top, crop_w,crop_h, scalar_w, scalar_h);

  /* Figure out the master window that is >= Crop window and == to output
     and more importantly should contain the crop region*/

  /* FOV (X,Y) cal for Analog zoom case */
  if (crop_w >= out_w) {
    entry->reg_cmd.y_crop_cfg.firstPixel = FLOOR2(left + (crop_w - out_w)/2);
    entry->reg_cmd.y_crop_cfg.lastPixel =
      entry->reg_cmd.y_crop_cfg.firstPixel + out_w - 1;
    /* Within ISP scaling range no extra cropping required */
    entry->crop_window.x = 0;
    entry->crop_window.crop_out_x = out_w;
  } else {
    /* Digital Zoom case, */
    /* Minimum to make sure FOV falls within the output of FOV module*/
    fov_x = MIN(x,left);
    /* If the FOV is beyond the proposed FOV output window,
       align the FOV output to cover FOV */
    if ((fov_x + out_w) < (left + crop_w)) {
       temp = scalar_w - (left + crop_w);
      if (x > temp) {
       fov_x = fov_x + (x - temp);
      }
    }

    entry->reg_cmd.y_crop_cfg.firstPixel = FLOOR2(fov_x);
    entry->reg_cmd.y_crop_cfg.lastPixel =
      entry->reg_cmd.y_crop_cfg.firstPixel + out_w - 1;

    /* Beyond ISP scaling range PPROC cropping needed for zoom */
    entry->crop_window.x = left - entry->reg_cmd.y_crop_cfg.firstPixel;
    entry->crop_window.crop_out_x = crop_w;
  }

  /* FOV (X,Y) cal for Analog zoom case */
  if (crop_h >= out_h) {
    entry->reg_cmd.y_crop_cfg.firstLine = FLOOR2(top + (crop_h - out_h)/2);
    /* Within ISP scaling range no extra cropping required */
    entry->crop_window.y = 0;
    entry->crop_window.crop_out_y = out_h;
  } else {
    /* Digital Zoom case, */
    /* Minimum to make sure FOV falls within the output of FOV module*/
    fov_y = MIN(y,top);
    /* If the FOV is beyond the proposed FOV output window,
       align the FOV output to cover FOV */
    if ((fov_y + out_h) < (top + crop_h)) {
       temp = scalar_h - (top + crop_h);
     if (y > temp) {
       fov_y = fov_y + (y - temp);
     }
    }
    entry->reg_cmd.y_crop_cfg.firstLine = FLOOR2(fov_y);
    /* Beyond ISP scaling range PPROC cropping needed for zoom */
    entry->crop_window.y = top - entry->reg_cmd.y_crop_cfg.firstLine;
    entry->crop_window.crop_out_y = crop_h;

  }
  entry->reg_cmd.y_crop_cfg.lastLine = entry->reg_cmd.y_crop_cfg.firstLine +
    out_h - 1;

  ISP_DBG("Entry %d FOV window [1st_pix, 1st_line,Last_pix,last_Line] [%d,%d,%d,%d]\n",
     fovcrop->entry_idx, entry->reg_cmd.y_crop_cfg.firstPixel,
     entry->reg_cmd.y_crop_cfg.firstLine, entry->reg_cmd.y_crop_cfg.lastPixel,
     entry->reg_cmd.y_crop_cfg.lastLine);

  ISP_DBG("Entry %d, Crop window for digital Zoom [x,y,w,h] [%d,%d,%d,%d]\n",
     fovcrop->entry_idx, entry->crop_window.x, entry->crop_window.y,
     entry->crop_window.crop_out_x, entry->crop_window.crop_out_y);

  isp_sub_module_getscalefactor(format, &scale_w, &scale_h);

  /* TODO: uv_subsampling changes */
  if (fovcrop->hw_stream_info.need_uv_subsample) {
    scale_w *= 2;
    scale_h *= 2;
  }

  entry->reg_cmd.cbcr_crop_cfg.firstPixel =
    entry->reg_cmd.y_crop_cfg.firstPixel / scale_w;

  entry->reg_cmd.cbcr_crop_cfg.lastPixel =
    entry->reg_cmd.cbcr_crop_cfg.firstPixel +
      (fovcrop->hw_stream_info.width / scale_w) -1;

  entry->reg_cmd.cbcr_crop_cfg.firstLine =
    entry->reg_cmd.y_crop_cfg.firstLine / scale_h;
  entry->reg_cmd.cbcr_crop_cfg.lastLine =
    entry->reg_cmd.cbcr_crop_cfg.firstLine +
      (fovcrop->hw_stream_info.height / scale_h) -1;
  entry->hw_update_pending = TRUE;

  /* dual VFE split config after determine singel VFE config*/
  if (fovcrop->ispif_out_info.is_split == TRUE) {
    ret = fovcrop40_split_config(fovcrop, scale_w);
    if (ret == FALSE) {
      ISP_ERR("fovcrop40_split_config failed!");
      return FALSE;
    }
  }

  ret = fovcrop40_validate_config(entry);
  if (ret == FALSE) {
    ISP_ERR("fovcrop failed!!!");
    return FALSE;
  }

  entry->hw_update_pending = TRUE;

  ISP_DBG("Y firstPixel %d, lastPixel %d firstline %d lastLine %d",
    entry->reg_cmd.y_crop_cfg.firstPixel,
    entry->reg_cmd.y_crop_cfg.lastPixel,
    entry->reg_cmd.y_crop_cfg.firstLine,
    entry->reg_cmd.y_crop_cfg.lastLine);
  ISP_DBG("CBCr firstPixel %d, lastPixel %d firstline %d lastLine %d",
    entry->reg_cmd.cbcr_crop_cfg.firstPixel,
    entry->reg_cmd.cbcr_crop_cfg.lastPixel,
    entry->reg_cmd.cbcr_crop_cfg.firstLine,
    entry->reg_cmd.cbcr_crop_cfg.lastLine);
  ISP_DBG("entry->hw_update_pending = TRUE");

  return TRUE;
}

/** fovcrop40_config
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: TRUE on success and FALSE on failure
 **/
boolean fovcrop40_config(fovcrop40_t *fovcrop)
{
  boolean ret = TRUE;
  isp_win_t fov_win_y;
  isp_win_t fov_win_cbcr;
  float scale_w;
  float scale_h;
  uint32_t camif_w, camif_h;
  cam_format_t format = fovcrop->hw_stream_info.fmt;
  fovcrop40_entry_t *entry = &fovcrop->fov_entry;
  uint32_t left_crop = 0, right_crop = 0, total_crop = 0;
  uint32_t crop_width;
  uint32_t crop_height;
  uint32_t h_factor;
  uint32_t v_factor;
  uint32_t scaler_factor;

  if (fovcrop->hw_stream_info.width == 0) {
    entry->is_used = 0;
    ISP_HIGH(" FOV entry not used");
    return FALSE;
  }

  ISP_DBG(" fov_entry hw stream info scaler: width %d, height %d",
     fovcrop->hw_stream_info.width,
       fovcrop->hw_stream_info.height);
  ISP_DBG(" fov_entry scaler_output: width %d, height %d",
     fovcrop->scaler_output.width,
       fovcrop->scaler_output.height);

  entry->is_used = 1;

  /*first pixel , first line , last pix, last line*/
  entry->reg_cmd.y_crop_cfg.firstPixel =
    (fovcrop->scaler_output.width -
    fovcrop->hw_stream_info.width) / 2;
  entry->reg_cmd.y_crop_cfg.lastPixel =
    entry->reg_cmd.y_crop_cfg.firstPixel +
    fovcrop->hw_stream_info.width - 1;

  entry->reg_cmd.y_crop_cfg.firstLine =
    (fovcrop->scaler_output.height -
    fovcrop->hw_stream_info.height) / 2;
  entry->reg_cmd.y_crop_cfg.lastLine =
    entry->reg_cmd.y_crop_cfg.firstLine +
    fovcrop->hw_stream_info.height - 1;

  /* ISP hits scaling limitation needs extra cropping for zoom */
  crop_width = fovcrop->hw_stream_info.width;
  crop_height = fovcrop->hw_stream_info.height;
  h_factor = fovcrop->scaler_output.width * Q12;
  v_factor = fovcrop->scaler_output.height * Q12;

  h_factor /= crop_width;
  v_factor /= crop_height;
  scaler_factor = (h_factor > v_factor)? v_factor : h_factor;

  entry->crop_window.crop_out_x = crop_width * scaler_factor /
    fovcrop->crop_factor.crop_applied;
  if (entry->crop_window.crop_out_x > crop_width)
    entry->crop_window.crop_out_x = crop_width;

  entry->crop_window.crop_out_y = crop_height * scaler_factor /
    fovcrop->crop_factor.crop_applied;
  if (entry->crop_window.crop_out_y > crop_height)
    entry->crop_window.crop_out_y = crop_height;

  entry->crop_window.x = (crop_width - entry->crop_window.crop_out_x) >> 1;
  entry->crop_window.y = (crop_height - entry->crop_window.crop_out_y) >> 1;

  /* Differentiate applied crop vs calculated
   * this is to support per stream scaler */
  entry->crop_window_calculated.crop_out_x = crop_width * scaler_factor /
    fovcrop->crop_factor.crop_to_calculate;
  if (entry->crop_window_calculated.crop_out_x > crop_width)
    entry->crop_window_calculated.crop_out_x = crop_width;

  entry->crop_window_calculated.crop_out_y = crop_height * scaler_factor /
    fovcrop->crop_factor.crop_to_calculate;
  if (entry->crop_window_calculated.crop_out_y > crop_height)
    entry->crop_window_calculated.crop_out_y = crop_height;

  entry->crop_window_calculated.x =
    (crop_width - entry->crop_window_calculated.crop_out_x) >> 1;
  entry->crop_window_calculated.y =
    (crop_height - entry->crop_window_calculated.crop_out_y) >> 1;

  ISP_DBG("crop x %d y %d dx %d dy %d", entry->crop_window.x,
    entry->crop_window.y, entry->crop_window.crop_out_x,
    entry->crop_window.crop_out_y);

  isp_sub_module_getscalefactor(format, &scale_w, &scale_h);

  if (fovcrop->hw_stream_info.need_uv_subsample) {
    scale_w *= 2;
    scale_h *= 2;
  }

  entry->reg_cmd.cbcr_crop_cfg.firstPixel =
    entry->reg_cmd.y_crop_cfg.firstPixel / scale_w;
  entry->reg_cmd.cbcr_crop_cfg.lastPixel =
    entry->reg_cmd.cbcr_crop_cfg.firstPixel +
      (fovcrop->hw_stream_info.width / scale_w) -1;

  entry->reg_cmd.cbcr_crop_cfg.firstLine =
    entry->reg_cmd.y_crop_cfg.firstLine / scale_h;
  entry->reg_cmd.cbcr_crop_cfg.lastLine =
    entry->reg_cmd.cbcr_crop_cfg.firstLine +
      (fovcrop->hw_stream_info.height / scale_h) -1;

  /* dual VFE split config after determine singel VFE config*/
  if (fovcrop->ispif_out_info.is_split == TRUE){
    ret = fovcrop40_split_config(fovcrop, scale_w);
    if (ret == FALSE) {
      ISP_ERR("fovcrop40_split_config failed!");
      return FALSE;
    }
  }

  ret = fovcrop40_validate_config(entry);
  if (ret == FALSE) {
    ISP_ERR("fovcrop failed!!!");
    return FALSE;
  }

  entry->hw_update_pending = TRUE;

  return TRUE;
} /* fovcrop40_config_entry */

/** fovcrop40_trigger_update
 *    @mod: fov module control struct
 *    @trigger_input: contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @in_param_size: input params struct size
 *
 *  fov module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_trigger_update(fovcrop40_t *fovcrop)
{
  int rc = 0;

  int is_burst = IS_BURST_STREAMING(fovcrop->streaming_mode_mask);
  camera_flash_type new_flash_mode;

  if (!fovcrop->fov_enable) {
    ISP_DBG(" Pca Rolloff is disabled. Skip the trigger.");
    return 0;
  }

  if (!fovcrop->trigger_enable) {
    ISP_DBG(" Trigger is disable. Skip the trigger update.");
    return 0;
  }

  //rc = fovcrop40_config(fovcrop, &trigger_input->cfg, sizeof(trigger_input->cfg));

  return rc;
} /* fovcrop40_trigger_update */

/** fovcrop40_set_zoom_ratio
 *    @fov: bcc module control struct
 *    @pix_setting: contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @in_param_size: input params struct size
 *
 *  Configure Fov module absed on zoom ratio
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_set_zoom_ratio(fovcrop40_t *fovcrop,
  uint32_t in_param_size)
{
  int rc = 0;
  //rc = fovcrop40_config(fovcrop, pix_setting, in_param_size);

  return rc;
}

/** fovcrop40_reset
 *      @mod: bcc module struct data
 *
 * fov module disable hw updates,release reg settings and
 * structs
 *
 * Return: nothing
 **/
void fovcrop40_reset(fovcrop40_t *fovcrop)
{
  ISP_DBG("E");
  fovcrop->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&fovcrop->scaler_output, 0, sizeof(fovcrop->scaler_output));
  memset(&fovcrop->fov_entry,  0, sizeof(fovcrop->fov_entry));
  memset(&fovcrop->isp_out_info,0, sizeof(fovcrop->isp_out_info));
  memset(&fovcrop->ispif_out_info,0, sizeof(fovcrop->ispif_out_info));

  fovcrop->trigger_enable = 1;
  fovcrop->fov_enable = 1;
  fovcrop->crop_window_based = FALSE;
}


/** fovcrop40_enable
 *    @mod: bcc module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input param struct size
 *
 *  fov module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_enable(fovcrop40_t *fovcrop, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  int i;
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  ISP_ERR(" size mismatch, expecting = %zu, received = %u",
          sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  fovcrop->fov_enable = enable->enable_mask;
  if (!fovcrop->fov_enable) {
    /* set all scaler entries to not used */
    fovcrop->fov_entry.hw_update_pending = 0;
    fovcrop->fov_entry.is_used = 0;
  }
  return 0;
}

/** fovcrop40_trigger_enable
 *    @mod: fov module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  fov module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_trigger_enable(fovcrop40_t *fovcrop,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    ISP_ERR(" size mismatch, expecting = %zu, received = %u",
          sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  fovcrop->trigger_enable = enable->enable_mask;

  return 0;
}

/** fovcrop40_get_fov_crop
 *    @mod: fov module control struct
 *    @pix_setting : contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @hw_zoom_param: zoom information
 *
 *  Get/store information about crop window, width, height etc.
 *
 * Return: 0 - success and negative value - failure
 **/
static int fovcrop40_get_fov_crop(fovcrop40_t *fovcrop,
  isp_hw_zoom_param_t *hw_zoom_param)
{
  int i;
  fovcrop40_entry_t *entry;

  memset(hw_zoom_param,  0,  sizeof(isp_hw_zoom_param_t));
  entry = &fovcrop->fov_entry;
  if (fovcrop->hw_stream_info.width) {
    /* the output is used, fill in stream info */
    hw_zoom_param->entry.dim.width =
      fovcrop->hw_stream_info.width;
    hw_zoom_param->entry.dim.height =
      fovcrop->hw_stream_info.height;
    hw_zoom_param->entry.stream_id =
      ISP_GET_STREAM_ID(fovcrop->hw_stream_info.identity);
    /* get fov first pix, first line, delta_x and delta_y*/
    hw_zoom_param->entry.crop_win = entry->crop_window;
    hw_zoom_param->num++;
  }
  return 0;
}

/** fovcrop40_destroy
 *    @mod_ctrl: bcc module control strcut
 *
 *  Close fov module
 *
 * Return: 0 always
 **/
static int fovcrop40_destroy (void *mod_ctrl)
{
  fovcrop40_t *fovcrop = mod_ctrl;

  memset(fovcrop,  0,  sizeof(fovcrop40_t));
  free(fovcrop);
  return 0;
}

/** fovcrop40_update_zoom_params
 *    @fov_mod: FOV module struct data
 *    @upd_zoom_params: zoom params to update
 *    @identity: stream identity
 *
 * update zoom params from FOV data;
 *
 * Return:   0 - success
 *          -1 - NULL parameter supplied
 **/
static int fovcrop40_update_zoom_params(fovcrop40_t *fov_mod,
  isp_zoom_params_t *upd_zoom_params, uint32_t identity)
{
  int i,j;
  isp_zoom_params_t *zoom_params;

  if(!fov_mod || !upd_zoom_params) {
    ISP_ERR("failed: fov_mod %p upd_zoom_params %p", fov_mod, upd_zoom_params);
    return -1;
  }

  if (!fov_mod->session_based_identity) {
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      zoom_params = &upd_zoom_params[i];
      if (zoom_params->identity == identity) {
        zoom_params->isp_output.width = fov_mod->hw_stream_info.width;
        zoom_params->isp_output.height = fov_mod->hw_stream_info.height;
        zoom_params->crop_window = fov_mod->fov_entry.crop_window;
        zoom_params->crop_window_calculated = fov_mod->fov_entry.crop_window_calculated;
        zoom_params->fov_output.x =
          (fov_mod->scaler_output.width - fov_mod->hw_stream_info.width) >> 1;
        zoom_params->fov_output.y =
          (fov_mod->scaler_output.height - fov_mod->hw_stream_info.height) >> 1;
        zoom_params->fov_output.crop_out_x = fov_mod->hw_stream_info.width;
        zoom_params->fov_output.crop_out_y = fov_mod->hw_stream_info.height;
        ISP_DBG("CAFDEBUG i %d ide %x crop %d %d %d %d", i, identity,
          zoom_params->crop_window.x, zoom_params->crop_window.y,
          zoom_params->crop_window.crop_out_x,
          zoom_params->crop_window.crop_out_y);
        break;
      }
    }
  } else {
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if (fov_mod->identity[j] &&
        (fov_mod->identity[j] != fov_mod->session_based_identity)) {
        for (i = 0; i < ISP_MAX_STREAMS; i++) {
          zoom_params = &upd_zoom_params[i];
          if (zoom_params->identity == fov_mod->identity[j]) {
            zoom_params->isp_output.width = fov_mod->hw_stream_info.width;
            zoom_params->isp_output.height = fov_mod->hw_stream_info.height;
            zoom_params->crop_window = fov_mod->fov_entry.crop_window;
            zoom_params->crop_window_calculated = fov_mod->fov_entry.crop_window_calculated;
            zoom_params->fov_output.x =
              (fov_mod->scaler_output.width -
              fov_mod->hw_stream_info.width) >> 1;
            zoom_params->fov_output.y =
              (fov_mod->scaler_output.height -
              fov_mod->hw_stream_info.height) >> 1;
            zoom_params->fov_output.crop_out_x = fov_mod->hw_stream_info.width;
            zoom_params->fov_output.crop_out_y = fov_mod->hw_stream_info.height;
            ISP_DBG("i %d ide %x crop %d %d %d %d", i,
              fov_mod->identity[j],
              zoom_params->crop_window.x, zoom_params->crop_window.y,
              zoom_params->crop_window.crop_out_x,
              zoom_params->crop_window.crop_out_y);
            break;
          }
        }
      }
    }
  }

  return 0;
}

/** fovcrop40_do_hw_update
 *    @fov_mod: bcc module struct data
 *    @output: isp sub module output params
 *
 * update fov module register to kernel
 *
 * Return: nothing
 **/
static int fovcrop40_do_hw_update(fovcrop40_t *fov_mod,
  isp_sub_module_output_t *output, uint32_t identity)
{
  int rc = 0;
  ISP_FOV_CropConfigCmdType *reg_cmd = NULL;
  isp_zoom_params_t         *zoom_params = NULL;
  uint32_t                   i = 0, j = 0;

  if (!fov_mod || !output) {
    ISP_ERR("failed: %p %p", fov_mod, output);
    return -EINVAL;
  }

  if (!output->stats_params) {
    ISP_ERR("failed: stats_params %p", output->stats_params);
    return -EINVAL;
  }

  ISP_DBG(" HW_update, FOV[%d] = %d",
    fov_mod->entry_idx, fov_mod->fov_entry.hw_update_pending);

  if ((fov_mod->update_enable_bit == TRUE) && output->stats_params) {
    fovcrop40_get_module_enable_mask(fov_mod,
      &output->stats_params->module_enable_info);
    fov_mod->update_enable_bit = FALSE;
  }

  if (fov_mod->fov_entry.hw_update_pending == TRUE) {
    reg_cmd = (ISP_FOV_CropConfigCmdType *)
      CAM_MALLOC(sizeof(ISP_FOV_CropConfigCmdType));
    if (!reg_cmd) {
      ISP_ERR("failed: reg_cmd %p", reg_cmd);
      return FALSE;
    }
    memset(reg_cmd, 0, sizeof(*reg_cmd));
    *reg_cmd = fov_mod->fov_entry.reg_cmd;
    fovcrop40_cmd_debug(fov_mod->fov_entry.reg_cmd, fov_mod->entry_idx);

    if (fov_mod->entry_idx == ISP_HW_STREAM_ENCODER) {
      rc = fovcrop40_util_single_HW_write(
        (void *)reg_cmd, sizeof(*reg_cmd), ISP_FOV40_ENC_OFF,
        ISP_FOV40_ENC_LEN, VFE_WRITE, &output->hw_update_list);
    } else {
      rc = fovcrop40_util_single_HW_write(
        (void *)reg_cmd, sizeof(*reg_cmd), ISP_FOV40_VIEW_OFF,
        ISP_FOV40_VIEW_LEN, VFE_WRITE, &output->hw_update_list);
    }
    fov_mod->fov_entry.hw_update_pending = FALSE;
  }

  rc = fovcrop40_update_zoom_params(fov_mod, output->stats_params->zoom_params,
    identity);

  return rc;
}

/** fovcrop40_get_module_enable_mask:
 *
 *  @scaler: fov crop handle
 *  @module_enable: handle to mask module enable bit
 *
 *  Enable / disable this module based on module enable flag
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean fovcrop40_get_module_enable_mask(fovcrop40_t *fovcrop,
  isp_module_enable_info_t *enable_bit_info)
{
  if (!fovcrop || !enable_bit_info) {
    ISP_ERR("failed: %p %p", fovcrop, enable_bit_info);
    return FALSE;
  }

  enable_bit_info->reconfig_needed = TRUE;
  enable_bit_info->submod_mask[fovcrop->hw_module_id] = 1;
  if (fovcrop->fov_enable) {
    enable_bit_info->submod_enable[fovcrop->hw_module_id] = 1;
  } else {
    enable_bit_info->submod_enable[fovcrop->hw_module_id] = 0;
  }

  return TRUE;
}

static boolean fovcrop40_loop_saved_events(mct_module_t *module,
  fovcrop40_t *fovcrop, isp_saved_events_t *saved_events, uint32_t identity)
{
  mct_event_control_parm_t *hal_param = NULL;

  if (saved_events->set_params[ISP_SET_PARM_ZOOM] &&
    (saved_events->set_params_valid[ISP_SET_PARM_ZOOM] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_ZOOM]->u.ctrl_event.
      control_event_data;
    port_fovcrop40_handle_hal_set_parm(module, fovcrop, identity, hal_param);
  }

  if (saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION] &&
    (saved_events->set_params_valid[ISP_SET_META_SCALER_CROP_REGION] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION]->u.ctrl_event.
      control_event_data;
    port_fovcrop40_handle_hal_set_parm(module, fovcrop, identity, hal_param);
  }

  if (saved_events->set_params[ISP_SET_PARM_UPDATE_DEBUG_LEVEL] &&
    (saved_events->set_params_valid[ISP_SET_PARM_UPDATE_DEBUG_LEVEL] == TRUE)) {
    hal_param =
      saved_events->set_params[ISP_SET_PARM_UPDATE_DEBUG_LEVEL]->u.ctrl_event.
      control_event_data;
    port_fovcrop40_handle_hal_set_parm(module, fovcrop, identity, hal_param);
  }

  /* Handle module event */
  /* only CDS event is used by scaler in the list of module events */
  if (saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST] &&
    (saved_events->module_events_valid[ISP_MODULE_EVENT_CDS_REQUEST] == TRUE)) {
    port_fovcrop40_handle_cds_request(module, fovcrop,
      saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST]);
  }
  return TRUE;
}

/** fovcrop40_handle_isp_private_event:
 *
 *  @fovcrop: fovcrop private
 *  @data: module event data
 *  Handle ISP private event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean fovcrop40_handle_isp_private_event(mct_module_t *module,
  fovcrop40_t *fovcrop, void *data, uint32_t identity)
{
  boolean              ret = TRUE;
  int                  rc = 0;
  isp_private_event_t *private_event = (isp_private_event_t *)data;

  if (!module || !fovcrop || !data) {
    ISP_ERR("failed: module %p fovcrop40 %p data %p", module, fovcrop, data);
    return FALSE;
  }

  switch (private_event->type) {
  case ISP_PRIVATE_REQUEST_STRIPE_LIMITATION:
    rc = fovcrop40_update_stripe_limit(fovcrop, private_event->data);
    if (rc < 0) {
      ISP_ERR("failed: fovcrop rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_STRIPE_INFO: {
    ispif_out_info_t *ispif_stripe_info =
      (ispif_out_info_t *)private_event->data;
    fovcrop->ispif_out_info = *ispif_stripe_info;
  }
    break;

  case ISP_PRIVATE_SET_MOD_ENABLE:
    rc = fovcrop40_enable(fovcrop,
      (isp_mod_set_enable_t *)private_event->data, private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: fovcrop40_enable rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_TRIGGER_ENABLE:
    rc = fovcrop40_trigger_enable(fovcrop, private_event->data,
      private_event->data_size);
    if (rc < 0) {
      ISP_ERR("failed: fovcrop40_config rc %d", rc);
      ret = FALSE;
    }
    break;

  case ISP_PRIVATE_SET_TRIGGER_UPDATE: {
    isp_sub_module_output_t *output = private_event->data;
    if (output->saved_events) {
      fovcrop40_loop_saved_events(module, fovcrop, output->saved_events,
        identity);
    }
    ISP_DBG("ISP_PRIVATE_SET_TRIGGER_UPDATE");
    PTHREAD_MUTEX_LOCK(&fovcrop->mutex);
    rc = fovcrop40_do_hw_update(fovcrop,
      (isp_sub_module_output_t *)private_event->data, identity);
    if (rc < 0) {
      ISP_ERR("failed: fovcrop40_do_hw_update rc %d", rc);
      ret = FALSE;
    }
    PTHREAD_MUTEX_UNLOCK(&fovcrop->mutex);
  }
    break;

  case ISP_PRIVATE_ACTION_RESET:
    fovcrop40_reset(fovcrop);
    break;

  case ISP_PRIVATE_REQUEST_ZOOM_PARAMS:
    rc = fovcrop40_update_zoom_params(fovcrop,
        (isp_zoom_params_t *)private_event->data, identity);
    if (rc < 0) {
      ISP_ERR("failed: fovcrop40_update_zoom_params");
      ret = FALSE;
    }
    break;


  default:
    break;
  }
  return ret;
}

/** fovcrop40_open
 *    @version: hw version
 *
 *  fov 40 module open and create func table
 *
 * Return: fov module ops struct pointer
 **/
boolean fovcrop40_open(fovcrop40_t *fovcrop)
{
  uint8_t i = 0;
  ISP_HIGH(" E");
  if (!fovcrop) {
    /* no memory */
    ISP_ERR(" no mem");
    return FALSE;
  }
  memset(&fovcrop->fov_entry.reg_cmd,  0,  sizeof(fovcrop->fov_entry.reg_cmd));
  fovcrop->trigger_enable = 1; /* enable trigger update feature flag from PIX */
  fovcrop->skip_trigger = 0;
  fovcrop->fov_enable = 1;         /* enable flag from PIX */
  fovcrop40_reset(fovcrop);

  return TRUE;
}

/** fov_init
 *    @mod_ctrl: fov module control strcut
 *    @in_params: fov hw module init params
 *    @notify_ops: fn pointer to notify other modules
 *
 *  fov module data struct initialization
 *
 * Return: 0 always
 **/
boolean fovcrop40_init (void *mod_ctrl, void *in_params)
{
  fovcrop40_t *fov = mod_ctrl;
//  isp_hw_mod_init_params_t *init_params = in_params;

  fov->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  fovcrop40_reset(fov);
  return TRUE;
}/* fov_init */
